/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "economy/expedition_bootstrap.h"

#include <memory>

#include "base/macros.h"
#include "base/wexception.h"
#include "economy/portdock.h"
#include "economy/wares_queue.h"
#include "economy/workers_queue.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

ExpeditionBootstrap::ExpeditionBootstrap(PortDock* const portdock)
   : portdock_(portdock), economy_(portdock->get_economy()) {
}

ExpeditionBootstrap::~ExpeditionBootstrap() {
	assert(queues_.empty());
}

void ExpeditionBootstrap::is_ready(Game& game) {
	for (std::unique_ptr<InputQueue>& iq : queues_) {
		if (iq->get_max_fill() != iq->get_filled())
			return;
	}

	// If this point is reached, all needed wares and workers are stored and waiting for a ship
	portdock_->expedition_bootstrap_complete(game);
}

// static
void ExpeditionBootstrap::input_callback(
   Game& game, InputQueue*, DescriptionIndex, Worker*, void* data) {
	ExpeditionBootstrap* eb = static_cast<ExpeditionBootstrap*>(data);
	eb->is_ready(game);
}

void ExpeditionBootstrap::start() {
	assert(queues_.empty());

	// Load the buildcosts for the port building + builder
	Warehouse* const warehouse = portdock_->get_warehouse();

	const std::map<DescriptionIndex, uint8_t>& buildcost = warehouse->descr().buildcost();
	size_t const buildcost_size = buildcost.size();

	// Issue request for wares for this expedition.
	// TODO(sirver): could try to get some of these directly form the warehouse.
	// But this is really a premature optimization and should probably be
	// handled in the economy code.
	queues_.resize(buildcost_size + 1);
	std::map<DescriptionIndex, uint8_t>::const_iterator it = buildcost.begin();
	for (size_t i = 0; i < buildcost_size; ++i, ++it) {
		WaresQueue* wq = new WaresQueue(*warehouse, it->first, it->second);
		wq->set_callback(input_callback, this);
		queues_[i].reset(wq);
	}

	// Issue the request for the workers (so far only a builder).
	queues_[buildcost_size].reset(
	   new WorkersQueue(*warehouse, warehouse->owner().tribe().builder(), 1));
	queues_[buildcost_size]->set_callback(input_callback, this);

	// Update the user interface
	if (upcast(InteractiveGameBase, igb, warehouse->owner().egbase().get_ibase()))
		warehouse->refresh_options(*igb);
}

/**
 * Cancel the Expediton by putting back all wares and workers.
‚ */
void ExpeditionBootstrap::cancel(Game& game) {

	// Put all wares from the WaresQueues back into the warehouse
	Warehouse* const warehouse = portdock_->get_warehouse();
	for (std::unique_ptr<InputQueue>& iq : queues_) {
		if (iq->get_type() == wwWARE) {
			warehouse->insert_wares(iq->get_index(), iq->get_filled());
		} else {
			assert(iq->get_type() == wwWORKER);
			WorkersQueue* wq = dynamic_cast<WorkersQueue*>(iq.get());
			while (iq->get_filled() > 0) {
				warehouse->incorporate_worker(game, wq->extract_worker());
			}
		}
		iq->cleanup();
	}
	queues_.clear();

	// Update the user interface
	if (upcast(InteractiveGameBase, igb, warehouse->owner().egbase().get_ibase())) {
		warehouse->refresh_options(*igb);
	}

	Notifications::publish(NoteExpeditionCanceled(this));
}

void ExpeditionBootstrap::cleanup(EditorGameBase& /* egbase */) {

	for (std::unique_ptr<InputQueue>& iq : queues_) {
		iq->cleanup();
	}
	queues_.clear();
}

InputQueue& ExpeditionBootstrap::inputqueue(DescriptionIndex index, WareWorker type) const {
	for (const std::unique_ptr<InputQueue>& iq : queues_) {
		if (iq->get_index() == index && iq->get_type() == type) {
			return *iq.get();
		}
	}
	NEVER_HERE();
}

std::vector<InputQueue*> ExpeditionBootstrap::queues() const {
	std::vector<InputQueue*> return_value;
	for (const std::unique_ptr<InputQueue>& iq : queues_) {
		return_value.emplace_back(iq.get());
	}
	return return_value;
}

void ExpeditionBootstrap::set_economy(Economy* new_economy) {
	if (new_economy == economy_)
		return;

	// Transfer the wares and workers.
	for (std::unique_ptr<InputQueue>& iq : queues_) {
		if (economy_)
			iq->remove_from_economy(*economy_);
		if (new_economy)
			iq->add_to_economy(*new_economy);
	}

	economy_ = new_economy;
}

void ExpeditionBootstrap::get_waiting_workers_and_wares(Game& game,
                                                        const TribeDescr& tribe,
                                                        std::vector<Worker*>* return_workers,
                                                        std::vector<WareInstance*>* return_wares) {
	for (std::unique_ptr<InputQueue>& iq : queues_) {
		if (iq->get_type() == wwWARE) {
			const DescriptionIndex ware_index = iq->get_index();
			for (uint32_t j = 0; j < iq->get_filled(); ++j) {
				WareInstance* temp = new WareInstance(ware_index, tribe.get_ware_descr(ware_index));
				temp->init(game);
				temp->set_location(game, portdock_);
				return_wares->emplace_back(temp);
			}
		} else {
			assert(iq->get_type() == wwWORKER);
			WorkersQueue* wq = dynamic_cast<WorkersQueue*>(iq.get());
			while (iq->get_filled() > 0) {
				return_workers->emplace_back(wq->extract_worker());
			}
		}
	}

	cleanup(game);
}

void ExpeditionBootstrap::save(FileWrite& fw, Game& game, MapObjectSaver& mos) {
	uint8_t number_warequeues = 0;
	for (std::unique_ptr<InputQueue>& q : queues_) {
		if (q->get_type() == wwWARE) {
			number_warequeues++;
		}
	}
	fw.unsigned_8(queues_.size() - number_warequeues);
	for (std::unique_ptr<InputQueue>& q : queues_) {
		if (q->get_type() == wwWORKER) {
			q->write(fw, game, mos);
		}
	}
	fw.unsigned_8(number_warequeues);
	for (std::unique_ptr<InputQueue>& q : queues_) {
		if (q->get_type() == wwWARE) {
			q->write(fw, game, mos);
		}
	}
}

void ExpeditionBootstrap::load(
   Warehouse& warehouse, FileRead& fr, Game& game, MapObjectLoader& mol, uint16_t packet_version) {

	static const uint16_t kCurrentPacketVersion = 7;

	assert(queues_.empty());
	// Load worker queues
	std::vector<WorkersQueue*> wqs;
	try {
		if (packet_version <= 6) {
			// This code is actually quite broken/inflexible but it should work
			// If we are here, use the old loader for build 19 packets
			const uint8_t num_workers = fr.unsigned_8();
			WorkersQueue* wq = new WorkersQueue(warehouse, warehouse.owner().tribe().builder(), 1);
			wq->load_for_expedition(fr, game, mol, num_workers);
			wqs.push_back(wq);
		} else if (packet_version >= kCurrentPacketVersion) {
			uint8_t num_queues = fr.unsigned_8();
			for (uint8_t i = 0; i < num_queues; ++i) {
				WorkersQueue* wq = new WorkersQueue(warehouse, INVALID_INDEX, 0);
				wq->read(fr, game, mol);
				wq->set_callback(input_callback, this);

				if (wq->get_index() == INVALID_INDEX) {
					delete wq;
				} else {
					wqs.push_back(wq);
				}
			}
		} else {
			throw UnhandledVersionError("ExpeditionBootstrap", packet_version, kCurrentPacketVersion);
		}

		// Load ware queues
		// Same code for both versions
		uint8_t num_queues = fr.unsigned_8();
		for (uint8_t i = 0; i < num_queues; ++i) {
			WaresQueue* wq = new WaresQueue(warehouse, INVALID_INDEX, 0);
			wq->read(fr, game, mol);
			wq->set_callback(input_callback, this);

			if (wq->get_index() == INVALID_INDEX) {
				delete wq;
			} else {
				queues_.emplace_back(wq);
			}
		}
		// Append worker queues to the end
		for (WorkersQueue* wq : wqs) {
			queues_.emplace_back(wq);
		}
	} catch (const GameDataError& e) {
		throw GameDataError("loading ExpeditionBootstrap: %s", e.what());
	}
}

}  // namespace Widelands
