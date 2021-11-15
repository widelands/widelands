/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
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

#include "economy/workers_queue.h"

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
 * Pre-initialize a WorkersQueue
 */
WorkersQueue::WorkersQueue(PlayerImmovable& init_owner,
                           DescriptionIndex const init_ware,
                           uint8_t const init_max_size)
   : InputQueue(init_owner, init_ware, init_max_size, wwWORKER) {
	if (index_ != INVALID_INDEX) {
		update();
	}
}

/**
 * Clear the queue appropriately.
 */
void WorkersQueue::cleanup() {
	assert(index_ != INVALID_INDEX);

	// Seems like workers don't need to be removed (done by building?)
	workers_.clear();
	max_size_ = 0;
	max_fill_ = 0;

	update();

	index_ = INVALID_INDEX;
}

/**
 * Called when a worker arrives at the owning building.
 */
void WorkersQueue::entered(
#ifndef NDEBUG
   DescriptionIndex index,
#else
   DescriptionIndex,
#endif
   Worker* worker) {

	assert(worker != nullptr);
	assert(get_filled() < max_size_);
	assert(index_ == index);

	EditorGameBase& egbase = get_owner()->egbase();
	if (worker->get_location(egbase) != &(owner_)) {
		worker->set_location(&(owner_));
	}
	assert(worker->get_location(egbase) == &owner_);

	// Bind the worker into this house, hide him on the map
	if (upcast(Game, game, &egbase)) {
		worker->start_task_idle(*game, 0, -1);
	}

	// Add worker to queue
	workers_.push_back(worker);

	// Make sure the request count is reduced or the request is deleted.
	update();
}

void WorkersQueue::remove_from_economy(Economy&) {
	if (index_ != INVALID_INDEX) {
		if (request_) {
			request_.reset();
		}
		// Removal of workers from the economy is not required, this is done by the building (or so)
	}
}

void WorkersQueue::add_to_economy(Economy& e) {
	if (index_ != INVALID_INDEX) {
		update();
		if (request_) {
			request_->set_economy(&e);
		}
	}
}

/**
 * Change fill status of the queue.
 */
void WorkersQueue::set_filled(Quantity filled) {
	if (filled > max_size_) {
		filled = max_size_;
	}
	if (filled == get_filled()) {
		return;
	}

	// Now adjust them
	const TribeDescr& tribe = owner().tribe();
	const WorkerDescr* worker_descr = tribe.get_worker_descr(index_);
	EditorGameBase& egbase = get_owner()->egbase();
	upcast(Game, game, &egbase);
	assert(game != nullptr);

	// Add workers
	while (get_filled() < filled) {
		// Create new worker
		Worker& w =
		   worker_descr->create(egbase, get_owner(), &owner_, owner_.get_positions(egbase).front());
		assert(w.get_location(egbase) == &owner_);
		w.start_task_idle(*game, 0, -1);
		workers_.push_back(&w);
	}
	assert(get_filled() >= filled);

	// Remove workers
	// Note: This might be slow (removing from start) but we want to consume
	// the first worker in the queue first
	while (get_filled() > filled) {
		// Remove worker
		assert(!workers_.empty());
		Worker* w = workers_.front();
		assert(w->get_location(egbase) == &owner_);
		// Remove from game
		w->schedule_destroy(*game);
		// Remove reference from list
		workers_.erase(workers_.begin());
	}
	assert(get_filled() == filled);
	update();
}

void WorkersQueue::set_max_fill(Quantity q) {
	InputQueue::set_max_fill(q);

	// If requested, kick out workers
	upcast(Game, game, &get_owner()->egbase());
	while (workers_.size() > max_fill_) {
		workers_.back()->reset_tasks(*game);
		workers_.back()->start_task_leavebuilding(*game, true);
		workers_.pop_back();
	}
}

Worker* WorkersQueue::extract_worker() {
	assert(get_filled() > 0);
	assert(!workers_.empty());

	Worker* w = workers_.front();
	// Don't remove from game
	// Remove reference from list
	workers_.erase(workers_.begin());
	return w;
}

/**
 * Read and write
 */

constexpr uint16_t kCurrentPacketVersion = 3;

void WorkersQueue::write_child(FileWrite& fw, Game&, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersion);
	// Store references to the workers
	fw.unsigned_32(workers_.size());
	for (Worker* w : workers_) {
		assert(mos.is_object_known(*w));
		fw.unsigned_32(mos.get_object_file_index(*w));
	}
}

void WorkersQueue::read_child(FileRead& fr, Game&, MapObjectLoader& mol) {
	uint16_t const packet_version = fr.unsigned_16();
	try {
		if (packet_version == kCurrentPacketVersion) {
			size_t nr_workers = fr.unsigned_32();
			assert(nr_workers <= max_size_);
			assert(workers_.empty());
			for (size_t i = 0; i < nr_workers; ++i) {
				workers_.push_back(&mol.get<Worker>(fr.unsigned_32()));
			}
			assert(workers_.size() == nr_workers);
		} else {
			throw UnhandledVersionError("WorkersQueue", packet_version, kCurrentPacketVersion);
		}
	} catch (const GameDataError& e) {
		throw GameDataError("workersqueue: %s", e.what());
	}
}
}  // namespace Widelands
