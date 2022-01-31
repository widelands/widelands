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

#ifndef WL_ECONOMY_WARES_QUEUE_H
#define WL_ECONOMY_WARES_QUEUE_H

#include "economy/input_queue.h"
#include "logic/map_objects/immovable.h"

namespace Widelands {

/**
 * This micro storage room can hold any number of items of a fixed ware.
 */
class WaresQueue : public InputQueue {
public:
	WaresQueue(PlayerImmovable&, DescriptionIndex, uint8_t size);

#ifndef NDEBUG
	~WaresQueue() override {
		assert(index_ == INVALID_INDEX);
	}
#endif

	Quantity get_filled() const override {
		return filled_;
	}

	void cleanup() override;

	void remove_from_economy(Economy&) override;
	void add_to_economy(Economy&) override;

	void set_filled(Quantity) override;

protected:
	void read_child(FileRead&, Game&, MapObjectLoader&) override;
	void write_child(FileWrite&, Game&, MapObjectSaver&) override;

	void entered(DescriptionIndex index, Worker* worker) override;

	/// Number of items that are currently in the queue
	Quantity filled_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_WARES_QUEUE_H
