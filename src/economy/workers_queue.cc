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

#include "economy/workers_queue.h"

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

WorkersQueue::WorkersQueue
    (PlayerImmovable &       init_owner,
	 DescriptionIndex        const init_worker,
	 uint8_t           const init_max_size)
    : owner_(init_owner),
	  worker_type_(init_worker),
	  max_capacity_(init_max_size),
	  capacity_(init_max_size),
	  request_(nullptr)
{
	if (worker_type_ != INVALID_INDEX)
		update_request();
}

void WorkersQueue::set_capacity(Quantity capacity) {
	assert(capacity <= max_capacity_);
	if (capacity_ != capacity) {
        capacity_ = capacity;
        update_request();
	}
}

void WorkersQueue::drop(Worker & worker) {
	Game & game = dynamic_cast<Game&>(owner().egbase());

	std::vector<Worker *>::iterator it =
		std::find(workers_.begin(), workers_.end(), &worker);
	if (it == workers_.end()) {
		return;
	}

	workers_.erase(it);

	worker.reset_tasks(game);
	worker.start_task_leavebuilding(game, true);

	update_request();
}

void WorkersQueue::remove_workers(Quantity amount) {
#warning TODO(Notabilis): Check if there are any ressources lost when removing workers
// Especially if there are any worker-memory-objects left or too much is removed

    assert(workers_.size() >= amount);

	Game & game = dynamic_cast<Game&>(owner().egbase());

    for (Quantity i = 0; i < amount; i++) {
        // TODO(Notabilis): Remove from economy (Not sure if required)
        // owner_.economy().remove_workers(worker_type_, amount);
        // Remove worker
		(*(workers_.begin()))->schedule_destroy(game);
        // Remove reference from list
        workers_.erase(workers_.begin());
    }

	update_request();
}

int WorkersQueue::incorporate_worker(EditorGameBase & egbase, Worker & w) {
	if (w.get_location(egbase) != &(owner_)) {
		if (workers_.size() + 1 > max_capacity_) {
			return -1;
        }
		w.set_location(&(owner_));
	}

	// Bind the worker into this house, hide him on the map
	if (upcast(Game, game, &egbase)) {
		w.start_task_idle(*game, 0, -1);
	}

    // Not quite sure about next line, the training sites are doing it inside add_worker().
    // But that method is not available for ware/worker-queues.
    // But anyway: Add worker to queue
	// TODO(Notabilis): Maybe only do when not already in the queue, not sure (on loading or so)
    workers_.push_back(&w);

	// Make sure the request count is reduced or the request is deleted.
	update_request();
	return 0;
}

void WorkersQueue::remove_from_economy(Economy &) {
	if (worker_type_ != INVALID_INDEX) {
        // Removal of workers is not required, this is done by the building (or so)
		if (request_) {
			request_->set_economy(nullptr);
		}
	}
}

void WorkersQueue::add_to_economy(Economy & e) {

	if (worker_type_ != INVALID_INDEX) {
		if (request_)
			request_->set_economy(&e);
	}
}

constexpr uint16_t kCurrentPacketVersion = 2;

void WorkersQueue::write(FileWrite & fw, Game & game, MapObjectSaver & mos) {
    // Adapted copy from WaresQueue
	fw.unsigned_16(kCurrentPacketVersion);

	//  Owner and callback is not saved, but this should be obvious on load.
	fw.c_string
		(owner().tribe().get_worker_descr(worker_type_)->name().c_str());
	fw.signed_32(max_capacity_);
	fw.signed_32(capacity_);
	if (request_) {
		fw.unsigned_8(1);
		request_->write(fw, game, mos);
	} else
		fw.unsigned_8(0);
    // Store references to the workers
    fw.unsigned_32(workers_.size());
    for (Worker * w : workers_) {
        assert(mos.is_object_known(*w));
        fw.unsigned_32(mos.get_object_file_index(*w));
    }
}


void WorkersQueue::read(FileRead & fr, Game & game, MapObjectLoader & mol) {
    // Adapted copy from WaresQueue
	uint16_t const packet_version = fr.unsigned_16();
	try {
		if (packet_version == kCurrentPacketVersion) {
			delete request_;
			worker_type_ = owner().tribe().worker_index(fr.c_string());
			max_capacity_ = fr.unsigned_32();
			capacity_ = fr.signed_32();
			assert(capacity_ <= max_capacity_);
			if (fr.unsigned_8 ()) {
				request_ =
					new Request
						(owner_,
						 0,
						 WorkersQueue::request_callback,
						 wwWORKER);
				request_->read(fr, game, mol);
			} else
				request_ = nullptr;
            size_t nr_workers = fr.unsigned_32();
            assert(nr_workers <= capacity_);
            assert(workers_.empty());
            for (size_t i = 0; i < nr_workers; ++i) {
                workers_.push_back(&mol.get<Worker>(fr.unsigned_32()));
            }
            assert(workers_.size() == nr_workers);
		} else {
			throw UnhandledVersionError("WorkersQueue", packet_version, kCurrentPacketVersion);
		}
	} catch (const GameDataError & e) {
		throw GameDataError("workersqueue: %s", e.what());
	}
}

void WorkersQueue::request_callback
	(Game            &       game,
	 Request         &,
	 DescriptionIndex        const,
	 Worker          * const worker,
	 PlayerImmovable & target)
{
	WorkersQueue & wq =
		dynamic_cast<Building&>(target).workersqueue(worker->descr().worker_index());

	assert(worker != nullptr);
	assert(wq.workers_.size() < wq.max_capacity_);
	assert(worker->descr().can_act_as(wq.worker_type_));

	assert(worker->get_location(game) == &target);

	// Update
	wq.incorporate_worker(game, *worker);
}

void WorkersQueue::update_request() {
	assert(worker_type_ != INVALID_INDEX);

	if (workers_.size() < capacity_) {
		if (!request_) {
			request_ =
				new Request
					(owner_,
					 worker_type_,
					 WorkersQueue::request_callback,
					 wwWORKER);
             // TODO(Notabilis): If it is possible to restrict the request to exactly the worker-type
             // of this queue, do so. Currently there are sometimes improved workers (e.g. Chief Miner)
             // coming to enter the building (where Miners are requested).
             // This happened after I kicked the Chief Miner from its mine.
             // The Master Miners in the headquarters however show no intentions of entering the building
		}

		request_->set_count(capacity_ - workers_.size());
	} else if (workers_.size() >= capacity_) {
		delete request_;
		request_ = nullptr;

		while (workers_.size() > capacity_) {
			drop(**workers_.rbegin());
		}
	}
}

}
