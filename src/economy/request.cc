/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "economy/request.h"

#include "base/log.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
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

Request::Request(PlayerImmovable& init_target,
                 DescriptionIndex const index,
                 CallbackFn const cbfn,
                 WareWorker const w)
   : type_(w),
     target_(init_target),
     target_building_(dynamic_cast<Building*>(&init_target)),
     target_productionsite_(dynamic_cast<ProductionSite*>(&init_target)),
     target_warehouse_(dynamic_cast<Warehouse*>(&init_target)),
     target_constructionsite_(dynamic_cast<ConstructionSite*>(&init_target)),
     economy_(init_target.get_economy(w)),
     index_(index),
     count_(1),
     exact_match_(false),
     callbackfn_(cbfn),
     required_time_(init_target.owner().egbase().get_gametime()),
     required_interval_(0),
     last_request_time_(required_time_) {
	assert(type_ == wwWARE || type_ == wwWORKER);
	if (w == wwWARE && !init_target.owner().egbase().descriptions().ware_exists(index)) {
		throw wexception(
		   "creating ware request with index %u, but the ware for this index doesn't exist", index);
	}
	if (w == wwWORKER && !init_target.owner().egbase().descriptions().worker_exists(index)) {
		throw wexception(
		   "creating worker request with index %u, but the worker for this index doesn't exist",
		   index);
	}
	if (economy_) {
		economy_->add_request(*this);
	}
}

Request::~Request() {
	// Remove from the economy
	if (is_open() && economy_) {
		economy_->remove_request(*this);
	}

	// Cancel all ongoing transfers
	while (!transfers_.empty()) {
		cancel_transfer(0);
	}
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
void Request::read(FileRead& fr, Game& game, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const std::string wareworker_name = fr.c_string();
			const std::pair<WareWorker, DescriptionIndex> wareworker =
			   game.descriptions().load_ware_or_worker(wareworker_name);
			type_ = wareworker.first;
			index_ = wareworker.second;
			// Check that the tribe uses the ware/worker
			switch (type_) {
			case WareWorker::wwWARE: {
				if (!target_.owner().tribe().has_ware(index_)) {
					throw GameDataError("Request::read: tribe '%s' does not use ware '%s'",
					                    target_.owner().tribe().name().c_str(), wareworker_name.c_str());
				}
			} break;
			case WareWorker::wwWORKER: {
				if (!target_.owner().tribe().has_worker(index_)) {
					throw GameDataError("Request::read: tribe '%s' does not use worker '%s'",
					                    target_.owner().tribe().name().c_str(), wareworker_name.c_str());
				}
			} break;
			}

			// Overwrite initial economy because our WareWorker type may have changed
			if (economy_) {
				economy_->remove_request(*this);
			}
			economy_ = target_.get_economy(type_);
			assert(economy_);

			count_ = fr.unsigned_32();
			required_time_ = Time(fr);
			required_interval_ = Duration(fr);

			last_request_time_ = Time(fr);

			assert(transfers_.empty());

			uint16_t const nr_transfers = fr.unsigned_16();
			for (uint16_t i = 0; i < nr_transfers; ++i) {
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
						log_warn(
						   "loading request, transferred object %u has no transfer\n", obj->serial());
					} else {
						transfer->set_request(this);
						transfers_.push_back(transfer);
					}
				} catch (const WException& e) {
					throw wexception("transfer %u: %s", i, e.what());
				}
			}
			requirements_.read(fr, game, mol);
			if (is_open()) {
				economy_->add_request(*this);
			}
		} else {
			throw UnhandledVersionError("Request", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("request: %s", e.what());
	}
}

/**
 * Write this request to a file
 */
void Request::write(FileWrite& fw, Game& game, MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);

	//  Target and economy should be set. Same is true for callback stuff.

	assert(type_ == wwWARE || type_ == wwWORKER);
	switch (type_) {
	case wwWARE:
		assert(game.descriptions().ware_exists(index_));
		fw.c_string(game.descriptions().get_ware_descr(index_)->name());
		break;
	case wwWORKER:
		assert(game.descriptions().worker_exists(index_));
		fw.c_string(game.descriptions().get_worker_descr(index_)->name());
		break;
	}

	fw.unsigned_32(count_);

	required_time_.save(fw);
	required_interval_.save(fw);

	last_request_time_.save(fw);

	fw.unsigned_16(transfers_.size());  //  Write number of current transfers.
	for (const Transfer* trans : transfers_) {
		if (trans->ware_) {  //  write ware/worker
			assert(mos.is_object_known(*trans->ware_));
			fw.unsigned_32(mos.get_object_file_index(*trans->ware_));
		} else if (trans->worker_) {
			assert(mos.is_object_known(*trans->worker_));
			fw.unsigned_32(mos.get_object_file_index(*trans->worker_));
		}
	}
	requirements_.write(fw, game, mos);
}

/**
 * Figure out the flag we need to deliver to.
 */
Flag& Request::target_flag() const {
	return target().base_flag();
}

/**
 * Return the point in time at which we want the ware of the given number to
 * be delivered. nr is in the range [0..count_[
 */
Time Request::get_base_required_time(const EditorGameBase& egbase, uint32_t const nr) const {
	if (count_ <= nr) {
		if (!(count_ == 1 && nr == 1)) {
			log_warn_time(egbase.get_gametime(),
			              "Request::get_base_required_time: WARNING nr = %u but count is %u, "
			              "which is not allowed according to the comment for this function\n",
			              nr, count_);
		}
	}
	const Time& curtime = egbase.get_gametime();

	if (!nr || required_interval_.get() == 0) {
		return required_time_;
	}

	if (curtime >= required_time_ && (curtime - required_time_) > (required_interval_ * 2)) {
		if (nr == 1) {
			return required_time_ + (curtime - required_time_) / 2;
		}

		assert(2 <= nr);
		return curtime + required_interval_ * (nr - 2);
	}

	return required_time_ + required_interval_ * nr;
}

/**
 * Return the time when the requested ware is needed.
 * Can be in the past, indicating that we have been idling, waiting for the
 * ware.
 */
Time Request::get_required_time() const {
	return get_base_required_time(economy_->owner().egbase(), transfers_.size());
}

constexpr Duration kBlacklistDurationAfterEvict(18000);

/**
 * Return the request priority. Used only to sort requests from most to least important.
 */
uint32_t Request::get_priority(const int32_t cost) const {
	assert(cost >= 0);
	const WarePriority& priority =
	   (target_building_ ? target_building_->get_priority(get_type(), get_index()) :
                          WarePriority::kNormal);

	// Workaround for bug #4809 Kicking a worker let him go the building where he was kicked off
	const Time& cur_time = economy_->owner().egbase().get_gametime();
	if (target_building_ != nullptr && get_type() == wwWORKER &&
	    target_building_->get_worker_evicted().is_valid() &&
	    cur_time - target_building_->get_worker_evicted() < kBlacklistDurationAfterEvict) {
		return 0;
	}

	if (WarePriority::kVeryHigh <= priority) {
		// Always serve requests with the highest priority first,
		// even if other requests have to wait then.
		return std::numeric_limits<uint32_t>::max();
	} else if (priority <= WarePriority::kVeryLow) {
		// Requests with priority 0 are processed by the
		// Economy only if there are no other requests.
		return 0;
	}

	const uint32_t req_time =
	   (target_constructionsite_ ? get_required_time().get() : get_last_request_time().get());
	return
	   // Linear scaling of request priority depending on
	   // the building's user-specified ware priority.
	   priority.to_weighting_factor() *
	   // Linear scaling of request priority depending on the time
	   // since the request was last supplied (constructionsites)
	   // or when the next ware is due (productionsites)
	   (cur_time.get() > req_time ? cur_time.get() - req_time : 1) *
	   // Requests with higher costs are preferred to keep the average waiting
	   // times short. This is capped at an arbitrary max cost of 30 seconds
	   // gametime to not disadvantage close-by supplies too much.
	   std::max(1, 30000 - cost);
}

/**
 * Return the transfer priority, based on the priority set at the destination,
 * normalized on a scale from 0 (lowest) to Flag::kMaxTransferPriority (highest).
 */
uint32_t Request::get_normalized_transfer_priority() const {
	if (!target_building_) {
		return 0;
	}

	// Magic numbers for reasonable weighting. Results for the values at the time of writing:
	//  Priority     Weighting factor      Result of the
	//              (ware_priority.cc)   calculation below
	// VeryLow               0                  0
	// Low                   1                  2
	// Normal               64                  7
	// High               4096                 12
	// VeryHigh         2^32-1                 16

	const WarePriority& priority = target_building_->get_priority(get_type(), get_index());
	if (WarePriority::kVeryHigh <= priority) {
		return Flag::kMaxTransferPriority;
	} else if (priority <= WarePriority::kVeryLow) {
		return 0;
	}

	const uint32_t factor = std::log2(priority.to_weighting_factor());
	return factor + 2 - factor / 6;
}

/**
 * Change the Economy we belong to.
 */
void Request::set_economy(Economy* const e) {
	if (economy_ != e) {
		if (economy_ && is_open()) {
			economy_->remove_request(*this);
		}
		economy_ = e;
		if (economy_ && is_open()) {
			economy_->add_request(*this);
		}
	}
}

/**
 * Change the number of wares we need.
 */
void Request::set_count(uint32_t const count) {
	bool const wasopen = is_open();

	count_ = count;

	// Cancel unneeded transfers. This should be more clever about which
	// transfers to cancel. Then again, this loop shouldn't execute during
	// normal play anyway
	while (count_ < transfers_.size()) {
		cancel_transfer(transfers_.size() - 1);
	}

	// Update the economy
	if (economy_) {
		if (wasopen && !is_open()) {
			economy_->remove_request(*this);
		} else if (!wasopen && is_open()) {
			economy_->add_request(*this);
		}
	}
}

/**
 * Sets whether a worker supply has to match exactly or if a can_act_as() comparison is good enough.
 */
void Request::set_exact_match(bool match) {
	exact_match_ = match;
}

/**
 * Change the time at which the first ware to be delivered is needed.
 * Default is the gametime of the Request creation.
 */
void Request::set_required_time(const Time& time) {
	required_time_ = time;
}

/**
 * Change the time between desired delivery of wares.
 */
void Request::set_required_interval(const Duration& interval) {
	required_interval_ = interval;
}

/**
 * Begin transfer of the requested ware from the given supply.
 * This function does not take ownership of route, i.e. the caller is
 * responsible for its deletion.
 */
void Request::start_transfer(Game& game, Supply& supp) {
	assert(is_open());

	::StreamWrite& ss = game.syncstream();
	ss.unsigned_8(SyncEntry::kStartTransfer);
	ss.unsigned_32(target().serial());
	ss.unsigned_32(supp.get_position(game)->serial());

	Transfer* t;
	switch (get_type()) {
	case wwWORKER: {
		//  Begin the transfer of a soldier or worker.
		//  launch_worker() creates or starts the worker
		Worker& s = supp.launch_worker(game, *this);
		ss.unsigned_32(s.serial());
		t = new Transfer(game, *this, s);
		break;
	}
	case wwWARE: {
		//  Begin the transfer of n ware. The ware itself is passive.
		//  launch_ware() ensures the WareInstance is transported out of the
		//  warehouse. Once it's on the flag, the flag code will decide what to
		//  do with it.
		WareInstance& ware = supp.launch_ware(game, *this);
		ss.unsigned_32(ware.serial());
		t = new Transfer(game, *this, ware);
		break;
	}
	}

	transfers_.push_back(t);
	if (!is_open()) {
		economy_->remove_request(*this);
	}
}

/**
 * Callback from ware/worker code that the requested ware has arrived.
 * This will call a callback function in the target, which is then responsible
 * for removing and deleting the request.
 */
void Request::transfer_finish(Game& game, Transfer& t) {
	Worker* const w = t.worker_;

	if (t.ware_) {
		t.ware_->destroy(game);
	}

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
void Request::transfer_fail(Game&, Transfer& t) {
	bool const wasopen = is_open();

	t.worker_ = nullptr;
	t.ware_ = nullptr;

	remove_transfer(find_transfer(t));

	if (!wasopen) {
		economy_->add_request(*this);
	}
}

/// Cancel the transfer with the given index.
///
/// \note This does *not* update whether the \ref Request is registered with
/// the \ref Economy or not.
void Request::cancel_transfer(uint32_t const idx) {
	remove_transfer(idx);
}

/**
 * Remove and free the transfer with the given index.
 * This does not update the Transfer's worker or ware, and it does not update
 * whether the Request is registered with the Economy.
 */
void Request::remove_transfer(uint32_t const idx) {
	Transfer* const t = transfers_[idx];

	transfers_.erase(transfers_.begin() + idx);

	delete t;
}

/**
 * Lookup a \ref Transfer in the transfers array.
 * \throw wexception if the \ref Transfer is not registered with us.
 */
uint32_t Request::find_transfer(Transfer& t) {
	TransferList::const_iterator const it = std::find(transfers_.begin(), transfers_.end(), &t);

	if (it == transfers_.end()) {
		throw wexception("Request::find_transfer(): not found");
	}

	return it - transfers_.begin();
}
}  // namespace Widelands
