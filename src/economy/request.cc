/*
 * Copyright (C) 2002-2004, 2006-2011, 2015 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "economy/request.h"

#include "base/macros.h"
#include "economy/economy.h"
#include "economy/portdock.h"
#include "economy/transfer.h"
#include "economy/ware_instance.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"


namespace Widelands {

/*
==============================================================================

Request IMPLEMENTATION

==============================================================================
*/

Request::Request
	(PlayerImmovable & init_target,
	 DescriptionIndex const index,
	 CallbackFn const cbfn,
	 WareWorker const w)
	:
	type_             (w),
	target_           (init_target),
	target_building_  (dynamic_cast<Building *>(&init_target)),
	target_productionsite_  (dynamic_cast<ProductionSite *>(&init_target)),
	target_warehouse_ (dynamic_cast<Warehouse *>(&init_target)),
	target_constructionsite_ (dynamic_cast<ConstructionSite *>(&init_target)),
	economy_          (init_target.get_economy()),
	index_            (index),
	count_            (1),
	callbackfn_       (cbfn),
	required_time_    (init_target.owner().egbase().get_gametime()),
	required_interval_(0),
	last_request_time_(required_time_)
{
	assert(type_ == wwWARE || type_ == wwWORKER);
	if (w == wwWARE && !init_target.owner().egbase().tribes().ware_exists(index))
		throw wexception
			("creating ware request with index %u, but the ware for this index doesn't exist",
			 index);
	if (w == wwWORKER && !init_target.owner().egbase().tribes().worker_exists(index))
		throw wexception
			("creating worker request with index %u, but the worker for this index doesn't exist",
			 index);
	if (economy_)
		economy_->add_request(*this);
}

Request::~Request()
{
	// Remove from the economy
	if (is_open() && economy_)
		economy_->remove_request(*this);

	// Cancel all ongoing transfers
	while (transfers_.size())
		cancel_transfer(0);
}

// Modified to allow Requirements and SoldierRequests
constexpr uint16_t kCurrentPacketVersion = 6;

/**
 * Read this request from a file
 *
 * it is most probably created by some init function,
 * so ad least target/economy is correct. Some Transports
 * might have been initialized. We have to kill them and replace
 * them through the data in the file
 */
void Request::read
	(FileRead & fr, Game & game, MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const TribeDescr& tribe = target_.owner().tribe();
			char const* const type_name = fr.c_string();
			DescriptionIndex const wai = tribe.ware_index(type_name);
			if (tribe.has_ware(wai)) {
				type_ = wwWARE;
				index_ = wai;
			} else {
				DescriptionIndex const woi = tribe.worker_index(type_name);
				if (tribe.has_worker(woi)) {
					type_ = wwWORKER;
					index_ = woi;
				} else {
					throw wexception("Request::read: unknown type '%s'.\n", type_name);
				}
			}
			count_             = fr.unsigned_32();
			required_time_     = fr.unsigned_32();
			required_interval_ = fr.unsigned_32();

			last_request_time_ = fr.unsigned_32();

			assert(transfers_.empty());

			uint16_t const nr_transfers = fr.unsigned_16();
			for (uint16_t i = 0; i < nr_transfers; ++i)
				try {
					MapObject* obj = &mol.get<MapObject>(fr.unsigned_32());
					Transfer* transfer;

					if (upcast(Worker, worker, obj)) {
						transfer = worker->get_transfer();
						if (type_ != wwWORKER || !worker->descr().can_act_as(index_)) {
							throw wexception("Request::read: incompatible transfer type");
						}
					} else if (upcast(WareInstance, ware, obj)) {
						transfer = ware->get_transfer();
						if (type_ != wwWARE || ware->descr_index() != index_) {
							throw wexception("Request::read: incompatible transfer type");
						}
					} else {
						throw wexception("transfer target %u is neither ware nor worker", obj->serial());
					}

					if (!transfer) {
						log("WARNING: loading request, transferred object %u has no transfer\n",
						    obj->serial());
					} else {
						transfer->set_request(this);
						transfers_.push_back(transfer);
					}
				} catch (const WException& e) {
				   throw wexception("transfer %u: %s", i, e.what());
				}
			requirements_.read (fr, game, mol);
			if (!is_open() && economy_)
				economy_->remove_request(*this);
		} else {
			throw UnhandledVersionError("Request", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw wexception("request: %s", e.what());
	}
}

/**
 * Write this request to a file
 */
void Request::write
	(FileWrite & fw, Game & game, MapObjectSaver & mos) const
{
	fw.unsigned_16(kCurrentPacketVersion);

	//  Target and economy should be set. Same is true for callback stuff.

	assert(type_ == wwWARE || type_ == wwWORKER);
	if (type_ == wwWARE) {
		assert(game.tribes().ware_exists(index_));
		fw.c_string(game.tribes().get_ware_descr(index_)->name());
	} else if (type_ == wwWORKER) {
		assert(game.tribes().worker_exists(index_));
		fw.c_string(game.tribes().get_worker_descr(index_)->name());
	}

	fw.unsigned_32(count_);

	fw.unsigned_32(required_time_);
	fw.unsigned_32(required_interval_);

	fw.unsigned_32(last_request_time_);

	fw.unsigned_16(transfers_.size()); //  Write number of current transfers.
	for (uint32_t i = 0; i < transfers_.size(); ++i) {
		Transfer & trans = *transfers_[i];
		if (trans.ware_) { //  write ware/worker
			assert(mos.is_object_known(*trans.ware_));
			fw.unsigned_32(mos.get_object_file_index(*trans.ware_));
		} else if (trans.worker_) {
			assert(mos.is_object_known(*trans.worker_));
			fw.unsigned_32(mos.get_object_file_index(*trans.worker_));
		}
	}
	requirements_.write (fw, game, mos);
}

/**
 * Figure out the flag we need to deliver to.
*/
Flag & Request::target_flag() const
{
	return target().base_flag();
}

/**
 * Return the point in time at which we want the ware of the given number to
 * be delivered. nr is in the range [0..count_[
*/
int32_t Request::get_base_required_time
	(EditorGameBase & egbase, uint32_t const nr) const
{
	if (count_ <= nr) {
		if (!(count_ == 1 && nr == 1)) {
			log
				("Request::get_base_required_time: WARNING nr = %u but count is %u, "
				"which is not allowed according to the comment for this function\n",
				nr, count_);
		}
	}
	int32_t const curtime = egbase.get_gametime();

	if (!nr || !required_interval_)
		return required_time_;

	if ((curtime - required_time_) > (required_interval_ * 2)) {
		if (nr == 1)
			return required_time_ + (curtime - required_time_) / 2;

		assert(2 <= nr);
		return curtime + (nr - 2) * required_interval_;
	}

	return required_time_ + nr * required_interval_;
}

/**
 * Return the time when the requested ware is needed.
 * Can be in the past, indicating that we have been idling, waiting for the
 * ware.
*/
int32_t Request::get_required_time() const
{
	return
		get_base_required_time(economy_->owner().egbase(), transfers_.size());
}

//#define MAX_IDLE_PRIORITY           100
#define PRIORITY_MAX_COST         50000
#define COST_WEIGHT_IN_PRIORITY       1
#define WAITTIME_WEIGHT_IN_PRIORITY   2

/**
 * Return the request priority used to sort requests or -1 to skip request
 */
// TODO(sirver): this is pretty weird design: we ask the building for the
// priority it assigns to the ware, at the same time, we also adjust the
// priorities depending on the building type. Move all of this into the
// building code.
int32_t Request::get_priority (int32_t cost) const
{
	int MAX_IDLE_PRIORITY = 100;
	bool is_construction_site = false;
	int32_t modifier = DEFAULT_PRIORITY;

	if (target_building_) {
		modifier = target_building_->get_priority(get_type(), get_index());
		if (target_constructionsite_)
			is_construction_site = true;
		else if (target_warehouse_) {
			// If there is no expedition at this warehouse, use the default
			// warehouse calculation. Otherwise we use the default priority for
			// the ware.
			if
				(!target_warehouse_->get_portdock() ||
				 !target_warehouse_->get_portdock()->expedition_bootstrap())
			{
				modifier =
					std::max(1, MAX_IDLE_PRIORITY - cost * MAX_IDLE_PRIORITY / PRIORITY_MAX_COST);
			}
		}
	}

	if (cost > PRIORITY_MAX_COST)
		cost = PRIORITY_MAX_COST;

	// priority is higher if building waits for ware a long time
	// additional factor - cost to deliver, so nearer building
	// with same priority will get ware first
	//  make sure that idle request are lower
	return
		MAX_IDLE_PRIORITY
		+
		std::max
			(uint32_t(1),
			 ((economy_->owner().egbase().get_gametime() -
			   (is_construction_site ?
			    get_required_time() : get_last_request_time()))
			  *
			  WAITTIME_WEIGHT_IN_PRIORITY
			  +
			  (PRIORITY_MAX_COST - cost) * COST_WEIGHT_IN_PRIORITY)
			 *
			 modifier);
}


/**
 * Return the transfer priority, based on the priority set at the destination
 */
// TODO(sirver): Same comment as for Request::get_priority.
uint32_t Request::get_transfer_priority() const
{
	uint32_t pri = 0;

	if (target_building_) {
		pri = target_building_->get_priority(get_type(), get_index());
		if (target_constructionsite_)
			return pri + 3;
		else if (target_warehouse_)
			return pri - 2;
	}
	return pri;
}

/**
 * Change the Economy we belong to.
*/
void Request::set_economy(Economy * const e)
{
	if (economy_ != e) {
		if (economy_ && is_open())
			economy_->remove_request(*this);
		economy_ = e;
		if (economy_ && is_open())
			economy_->add_request(*this);
	}
}

/**
 * Change the number of wares we need.
*/
void Request::set_count(uint32_t const count)
{
	bool const wasopen = is_open();

	count_ = count;

	// Cancel unneeded transfers. This should be more clever about which
	// transfers to cancel. Then again, this loop shouldn't execute during
	// normal play anyway
	while (count_ < transfers_.size())
		cancel_transfer(transfers_.size() - 1);

	// Update the economy
	if (economy_) {
		if (wasopen && !is_open())
			economy_->remove_request(*this);
		else if (!wasopen && is_open())
			economy_->add_request(*this);
	}
}

/**
 * Change the time at which the first ware to be delivered is needed.
 * Default is the gametime of the Request creation.
*/
void Request::set_required_time(int32_t const time)
{
	required_time_ = time;
}

/**
 * Change the time between desired delivery of wares.
*/
void Request::set_required_interval(int32_t const interval)
{
	required_interval_ = interval;
}

/**
 * Begin transfer of the requested ware from the given supply.
 * This function does not take ownership of route, i.e. the caller is
 * responsible for its deletion.
*/
void Request::start_transfer(Game & game, Supply & supp)
{
	assert(is_open());

	::StreamWrite & ss = game.syncstream();
	ss.unsigned_32(0x01decafa); // appears as facade01 in sync stream
	ss.unsigned_32(target().serial());
	ss.unsigned_32(supp.get_position(game)->serial());

	Transfer * t;
	if (get_type() == wwWORKER) {
		//  Begin the transfer of a soldier or worker.
		//  launch_worker() creates or starts the worker
		Worker & s = supp.launch_worker(game, *this);
		ss.unsigned_32(s.serial());
		t = new Transfer(game, *this, s);
	} else {
		//  Begin the transfer of an ware. The ware itself is passive.
		//  launch_ware() ensures the WareInstance is transported out of the
		//  warehouse. Once it's on the flag, the flag code will decide what to
		//  do with it.
		WareInstance & ware = supp.launch_ware(game, *this);
		ss.unsigned_32(ware.serial());
		t = new Transfer(game, *this, ware);
	}

	transfers_.push_back(t);
	if (!is_open())
		economy_->remove_request(*this);
}

/**
 * Callback from ware/worker code that the requested ware has arrived.
 * This will call a callback function in the target, which is then responsible
 * for removing and deleting the request.
*/
void Request::transfer_finish(Game & game, Transfer & t)
{
	Worker * const w = t.worker_;

	if (t.ware_)
		t.ware_->destroy(game);

	t.worker_ = nullptr;
	t.ware_ = nullptr;

	remove_transfer(find_transfer(t));

	set_required_time(get_base_required_time(game, 1));
	--count_;

	// the callback functions are likely to delete us,
	// therefore we musn't access member variables behind this
	// point
	(*callbackfn_)(game, *this, index_, w, target_);
}

/**
 * Callback from ware/worker code that the scheduled transfer has failed.
 * The calling code has already dealt with the worker/ware.
 *
 * Re-open the request.
*/
void Request::transfer_fail(Game &, Transfer & t) {
	bool const wasopen = is_open();

	t.worker_ = nullptr;
	t.ware_ = nullptr;

	remove_transfer(find_transfer(t));

	if (!wasopen)
		economy_->add_request(*this);
}

/// Cancel the transfer with the given index.
///
/// \note This does *not* update whether the \ref Request is registered with
/// the \ref Economy or not.
void Request::cancel_transfer(uint32_t const idx)
{
	remove_transfer(idx);
}

/**
 * Remove and free the transfer with the given index.
 * This does not update the Transfer's worker or ware, and it does not update
 * whether the Request is registered with the Economy.
 */
void Request::remove_transfer(uint32_t const idx)
{
	Transfer * const t = transfers_[idx];

	transfers_.erase(transfers_.begin() + idx);

	delete t;
}

/**
 * Lookup a \ref Transfer in the transfers array.
 * \throw wexception if the \ref Transfer is not registered with us.
 */
uint32_t Request::find_transfer(Transfer & t)
{
	TransferList::const_iterator const it =
		std::find(transfers_.begin(), transfers_.end(), &t);

	if (it == transfers_.end())
		throw wexception("Request::find_transfer(): not found");

	return it - transfers_.begin();
}

}
