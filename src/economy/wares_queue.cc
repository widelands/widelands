/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

WaresQueue::WaresQueue(PlayerImmovable& init_owner,
                       DescriptionIndex const init_ware,
                       uint8_t const init_max_size)
   : InputQueue(init_owner, init_ware, init_max_size, wwWARE), filled_(0) {
	if (index_ != INVALID_INDEX) {
		update();
	}
}

void WaresQueue::cleanup() {
	assert(index_ != INVALID_INDEX);

	if (filled_ && owner_.get_economy(wwWARE)) {
		owner_.get_economy(wwWARE)->remove_wares_or_workers(index_, filled_);
	}

	filled_ = 0;
	max_size_ = 0;
	max_fill_ = 0;

	update();

	index_ = INVALID_INDEX;
}

void WaresQueue::entered(
#ifndef NDEBUG
   DescriptionIndex index, Worker* worker
#else
   DescriptionIndex, Worker*
#endif
) {

	assert(worker == nullptr);  // WaresQueue can't hold workers
	assert(filled_ < max_size_);
	assert(index_ == index);

	// Update
	set_filled(filled_ + 1);
}

void WaresQueue::remove_from_economy(Economy& e) {
	if (index_ != INVALID_INDEX) {
		e.remove_wares_or_workers(index_, filled_);
		if (request_) {
			request_->set_economy(nullptr);
		}
	}
}

void WaresQueue::add_to_economy(Economy& e) {
	if (index_ != INVALID_INDEX) {
		e.add_wares_or_workers(index_, filled_);
		if (request_) {
			request_->set_economy(&e);
		}
	}
}

void WaresQueue::set_filled(Quantity filled) {
	if (filled > max_size_) {
		filled = max_size_;
	}

	if (owner_.get_economy(wwWARE)) {
		if (filled > filled_) {
			owner_.get_economy(wwWARE)->add_wares_or_workers(index_, filled - filled_);
		} else if (filled < filled_) {
			owner_.get_economy(wwWARE)->remove_wares_or_workers(index_, filled_ - filled);
		}
	}

	filled_ = filled;

	update();
}

/**
 * Read and write
 */

constexpr uint16_t kCurrentPacketVersion = 3;

void WaresQueue::write_child(FileWrite& fw, Game& /*g*/, MapObjectSaver& /*s*/) {
	fw.unsigned_16(kCurrentPacketVersion);

	fw.signed_32(filled_);
}

void WaresQueue::read_child(FileRead& fr, Game& /*g*/, MapObjectLoader& /*mol*/) {
	uint16_t const packet_version = fr.unsigned_16();
	try {
		if (packet_version == kCurrentPacketVersion) {
			filled_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("WaresQueue", packet_version, kCurrentPacketVersion);
		}
	} catch (const GameDataError& e) {
		throw GameDataError("waresqueue: %s", e.what());
	}
}
}  // namespace Widelands
