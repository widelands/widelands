/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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

#include "economy/wares_queue.h"

#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/request.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/**
 * Pre-initialize a WaresQueue
*/
WaresQueue::WaresQueue
	(PlayerImmovable &       init_owner,
	 DescriptionIndex        const init_ware,
	 uint8_t           const init_max_size)
	:
	owner_           (init_owner),
	ware_            (init_ware),
	max_size_        (init_max_size),
	max_fill_        (init_max_size),
	filled_          (0),
	consume_interval_(0),
	request_         (nullptr),
	callback_fn_     (nullptr),
	callback_data_   (nullptr)
{
	if (ware_ != INVALID_INDEX)
		update();
}


/**
 * Clear the queue appropriately.
*/
void WaresQueue::cleanup() {
	assert(ware_ != INVALID_INDEX);

	if (filled_ && owner_.get_economy())
		owner_.get_economy()->remove_wares(ware_, filled_);

	filled_ = 0;
	max_size_ = 0;
	max_fill_ = 0;

	update();

	ware_ = INVALID_INDEX;
}

/**
 * Fix filled <= size and requests.
 * You must call this after every call to set_*()
*/
void WaresQueue::update() {
	assert(ware_ != INVALID_INDEX);

	if (filled_ > max_size_) {
		if (owner_.get_economy())
			owner_.get_economy()->remove_wares(ware_, filled_ - max_size_);
		filled_ = max_size_;
	}

	if (filled_ < max_fill_)
	{
		if (!request_)
			request_ =
				new Request
					(owner_,
					 ware_,
					 WaresQueue::request_callback,
					 wwWARE);

		request_->set_count(max_fill_ - filled_);
		request_->set_required_interval(consume_interval_);
	}
	else
	{
		delete request_;
		request_ = nullptr;
	}
}

/**
 * Set the callback function that is called when an item has arrived.
*/
void WaresQueue::set_callback(CallbackFn * const fn, void * const data)
{
	callback_fn_ = fn;
	callback_data_ = data;
}

/**
 * Called when an item arrives at the owning building.
*/
void WaresQueue::request_callback
	(Game            &       game,
	 Request         &,
	 DescriptionIndex        const ware,
#ifndef NDEBUG
	 Worker          * const w,
#else
	 Worker          *,
#endif
	 PlayerImmovable & target)
{
	WaresQueue & wq =
		dynamic_cast<Building&>(target).waresqueue(ware);

	assert(!w); // WaresQueue can't hold workers
	assert(wq.filled_ < wq.max_size_);
	assert(wq.ware_ == ware);

	// Update
	wq.set_filled(wq.filled_ + 1);

	if (wq.callback_fn_)
		(*wq.callback_fn_)(game, &wq, ware, wq.callback_data_);
}

/**
 * Remove the wares in this queue from the given economy (used in accounting).
*/
void WaresQueue::remove_from_economy(Economy & e)
{
	if (ware_ != INVALID_INDEX) {
		e.remove_wares(ware_, filled_);
		if (request_)
			request_->set_economy(nullptr);
	}
}

/**
 * Add the wares in this queue to the given economy (used in accounting)
*/
void WaresQueue::add_to_economy(Economy & e)
{
	if (ware_ != INVALID_INDEX) {
		e.add_wares(ware_, filled_);
		if (request_)
			request_->set_economy(&e);
	}
}

/**
 * Change size of the queue.
 */
void WaresQueue::set_max_size(const Quantity size)
{
	Quantity old_size = max_size_;
	max_size_ = size;

	// make sure that max fill is reduced as well if the max size is decreased
	// because this is very likely what the user wanted to only consume so
	// and so many wares in the first place. If it is increased, keep the
	// max fill fill as it was
	set_max_fill(std::min(max_fill_, max_fill_ - (old_size - max_size_)));

	update();
}

/**
 * Change the number of wares that should be available in this queue
 *
 * This is basically the same as setting the maximum size,
 * but if there are more wares than that in the queue, they will not get
 * lost (the building should drop them).
 */
void WaresQueue::set_max_fill(Quantity size)
{
	if (size > max_size_)
		size = max_size_;

	max_fill_ = size;

	update();
}

/**
 * Change fill status of the queue.
 */
void WaresQueue::set_filled(const Quantity filled) {
	if (owner_.get_economy()) {
		if (filled > filled_)
			owner_.get_economy()->add_wares(ware_, filled - filled_);
		else if (filled < filled_)
			owner_.get_economy()->remove_wares(ware_, filled_ - filled);
	}

	filled_ = filled;

	update();
}

/**
 * Set the time between consumption of items when the owning building
 * is consuming at full speed.
 *
 * This interval is merely a hint for the Supply/Request balancing code.
*/
void WaresQueue::set_consume_interval(const uint32_t time)
{
	consume_interval_ = time;

	update();
}

/**
 * Read and write
 */

constexpr uint16_t kCurrentPacketVersion = 2;

void WaresQueue::write(FileWrite & fw, Game & game, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersion);

	//  Owner and callback is not saved, but this should be obvious on load.
	fw.c_string
		(owner().tribe().get_ware_descr(ware_)->name().c_str());
	fw.signed_32(max_size_);
	fw.signed_32(max_fill_);
	fw.signed_32(filled_);
	fw.signed_32(consume_interval_);
	if (request_) {
		fw.unsigned_8(1);
		request_->write(fw, game, mos);
	} else
		fw.unsigned_8(0);
}


void WaresQueue::read(FileRead & fr, Game & game, MapObjectLoader & mol)
{
	uint16_t const packet_version = fr.unsigned_16();
	try {
		if (packet_version == kCurrentPacketVersion) {
			delete request_;
			ware_             = owner().tribe().ware_index(fr.c_string  ());
			max_size_         =                            fr.unsigned_32();
			max_fill_ = fr.signed_32();
			filled_           =                            fr.unsigned_32();
			consume_interval_ =                            fr.unsigned_32();
			if                                             (fr.unsigned_8 ()) {
				request_ =                          //  TODO(unknown): Change Request::read
					new Request                       //  to a constructor.
						(owner_,
						 0,
						 WaresQueue::request_callback,
						 wwWORKER);
				request_->read(fr, game, mol);
			} else
				request_ = nullptr;

			//  Now Economy stuff. We have to add our filled items to the economy.
			if (owner_.get_economy())
				add_to_economy(*owner_.get_economy());
		} else {
			throw UnhandledVersionError("WaresQueue", packet_version, kCurrentPacketVersion);
		}
	} catch (const GameDataError & e) {
		throw GameDataError("waresqueue: %s", e.what());
	}
}

}
