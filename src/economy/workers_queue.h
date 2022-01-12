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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_ECONOMY_WORKERS_QUEUE_H
#define WL_ECONOMY_WORKERS_QUEUE_H

#include "economy/input_queue.h"
#include "logic/map_objects/immovable.h"

namespace Widelands {

/**
 * This micro storage room can hold any number of items of a fixed worker.
 */
class WorkersQueue : public InputQueue {
public:
	WorkersQueue(PlayerImmovable&, DescriptionIndex, uint8_t size);

#ifndef NDEBUG
	~WorkersQueue() override {
		assert(index_ == INVALID_INDEX);
	}
#endif

	Quantity get_filled() const override {
		return workers_.size();
	}

	void cleanup() override;

	void remove_from_economy(Economy&) override;
	void add_to_economy(Economy&) override;

	void set_filled(Quantity) override;

	void set_max_fill(Quantity q) override;

	/**
	 * Extracts the first worker from the queue and returns it
	 * without removing it from the game.
	 * Used by ExpeditionBootstrap.
	 * @return The first worker in stored in this list.
	 */
	Worker* extract_worker();

protected:
	void read_child(FileRead&, Game&, MapObjectLoader&) override;
	void write_child(FileWrite&, Game&, MapObjectSaver&) override;

	void entered(DescriptionIndex index, Worker* worker) override;

	/// The workers currently in the queue
	std::vector<Worker*> workers_;
};
}  // namespace Widelands

#endif  // WL_ECONOMY_WORKERS_QUEUE_H
