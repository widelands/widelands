/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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
#include "map_io/map_packet_versions.h"

namespace Widelands {

ExpeditionBootstrap::ExpeditionBootstrap(PortDock* const portdock)
   : portdock_(portdock),
     ware_economy_(portdock->get_economy(wwWARE)),
     worker_economy_(portdock->get_economy(wwWORKER)) {
}

ExpeditionBootstrap::~ExpeditionBootstrap() {
	assert(queues_.empty());
}

void ExpeditionBootstrap::check_is_ready(Game& game) {
	for (auto& iq : queues_) {
		if (iq.first->get_max_size() != iq.first->get_filled() ||
		    iq.first->get_max_size() != iq.first->get_max_fill()) {
			return portdock_->set_expedition_bootstrap_complete(game, false);
		}
	}

	// If this point is reached, all needed wares and workers are stored and waiting for a ship
	portdock_->set_expedition_bootstrap_complete(game, true);
}

// static
void ExpeditionBootstrap::input_callback(
   Game& game, InputQueue* queue, DescriptionIndex, Worker*, void* data) {
	ExpeditionBootstrap* eb = static_cast<ExpeditionBootstrap*>(data);
	eb->check_is_ready(game);
	// If we ask for several additional items of the same type, it may happen that a
	// specific item was originally requested by queue B but is put into queue A. This
	// causes both queues to cancel their requests so that some transfers are
	// accidentally cancelled. The solution is to iterate ALL queues of this type and
	// check whether their count and transfers still match up.
	for (std::pair<std::unique_ptr<InputQueue>, bool>& pair : eb->queues_) {
		if (pair.first->get_type() == queue->get_type() &&
		    pair.first->get_index() == queue->get_index()) {
			pair.first->set_max_fill(pair.first->get_max_fill());  // calls update()
		}
	}
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
		queues_[i].first.reset(wq);
		queues_[i].second = false;
	}

	// Issue the request for the workers (so far only a builder).
	queues_[buildcost_size].first.reset(
	   new WorkersQueue(*warehouse, warehouse->owner().tribe().builder(), 1));
	queues_[buildcost_size].first->set_callback(input_callback, this);
	queues_[buildcost_size].second = false;

	// Update the user interface
	Notifications::publish(NoteBuilding(warehouse->serial(), NoteBuilding::Action::kChanged));
}

/**
 * Cancel the Expediton by putting back all wares and workers.
‚ */
void ExpeditionBootstrap::cancel(Game& game) {

	// Put all wares from the WaresQueues back into the warehouse
	Warehouse* const warehouse = portdock_->get_warehouse();
	for (auto& iq : queues_) {
		switch (iq.first->get_type()) {
		case wwWARE:
			warehouse->insert_wares(iq.first->get_index(), iq.first->get_filled());
			break;
		case wwWORKER:
			WorkersQueue& wq = dynamic_cast<WorkersQueue&>(*iq.first);
			while (wq.get_filled() > 0) {
				warehouse->incorporate_worker(game, wq.extract_worker());
			}
			break;
		}
		iq.first->cleanup();
	}
	queues_.clear();

	// Update the user interface
	Notifications::publish(NoteBuilding(warehouse->serial(), NoteBuilding::Action::kChanged));
	Notifications::publish(NoteExpeditionCanceled(this));
}

void ExpeditionBootstrap::demand_additional_item(Game& game,
                                                 WareWorker ww,
                                                 DescriptionIndex di,
                                                 bool add) {
	if (add) {
		InputQueue* wq;
		if (ww == wwWARE) {
			wq = new WaresQueue(*portdock_->get_warehouse(), di, 1);
		} else {
			wq = new WorkersQueue(*portdock_->get_warehouse(), di, 1);
		}
		wq->set_callback(input_callback, this);
		queues_.push_back(std::make_pair(std::unique_ptr<InputQueue>(wq), true));
		return check_is_ready(game);
	} else {
		// Remove the last matching additional queue
		for (auto it = queues_.end(); it != queues_.begin();) {
			--it;
			if (it->second && it->first->get_type() == ww && it->first->get_index() == di) {
				Warehouse* const warehouse = portdock_->get_warehouse();
				if (it->first->get_type() == wwWARE) {
					warehouse->insert_wares(it->first->get_index(), it->first->get_filled());
				} else {
					assert(it->first->get_type() == wwWORKER);
					WorkersQueue* wq = dynamic_cast<WorkersQueue*>(it->first.get());
					while (wq->get_filled() > 0) {
						warehouse->incorporate_worker(game, wq->extract_worker());
					}
				}
				it->first->cleanup();
				queues_.erase(it);
				return check_is_ready(game);
			}
		}
		NEVER_HERE();
	}
}

void ExpeditionBootstrap::cleanup(EditorGameBase& /* egbase */) {
	for (auto& iq : queues_) {
		iq.first->cleanup();
	}
	queues_.clear();
}

InputQueue&
ExpeditionBootstrap::inputqueue(DescriptionIndex index, WareWorker type, bool additional) const {
	for (const auto& iq : queues_) {
		if (iq.first->get_index() == index && iq.first->get_type() == type &&
		    iq.second == additional) {
			return *iq.first;
		}
	}
	NEVER_HERE();
}

InputQueue& ExpeditionBootstrap::inputqueue(const Request& r) const {
	for (const auto& iq : queues_) {
		if (iq.first->matches(r)) {
			return *iq.first;
		}
	}
	NEVER_HERE();
}

InputQueue* ExpeditionBootstrap::inputqueue(size_t additional_index) const {
	for (const auto& iq : queues_) {
		if (iq.second) {
			if (additional_index == 0) {
				return iq.first.get();
			}
			--additional_index;
		}
	}
	return nullptr;
}

std::vector<InputQueue*> ExpeditionBootstrap::queues(bool all) const {
	std::vector<InputQueue*> return_value;
	for (const auto& iq : queues_) {
		if (all || !iq.second) {
			return_value.emplace_back(iq.first.get());
		}
	}
	return return_value;
}

size_t ExpeditionBootstrap::count_additional_queues() const {
	size_t i = 0;
	for (const auto& iq : queues_) {
		if (iq.second) {
			++i;
		}
	}
	return i;
}

void ExpeditionBootstrap::set_economy(Economy* new_economy, WareWorker type) {
	if (new_economy == (type == wwWARE ? ware_economy_ : worker_economy_)) {
		return;
	}

	// Transfer the wares and workers.
	for (auto& iq : queues_) {
		if (type != iq.first->get_type()) {
			continue;
		}
		if (Economy* e = type == wwWARE ? ware_economy_ : worker_economy_) {
			iq.first->remove_from_economy(*e);
		}
		if (new_economy) {
			iq.first->add_to_economy(*new_economy);
		}
	}

	(type == wwWARE ? ware_economy_ : worker_economy_) = new_economy;
}

void ExpeditionBootstrap::get_waiting_workers_and_wares(Game& game,
                                                        const TribeDescr& tribe,
                                                        std::vector<Worker*>* return_workers,
                                                        std::vector<WareInstance*>* return_wares) {
	for (auto& iq : queues_) {
		switch (iq.first->get_type()) {
		case wwWARE: {
			const DescriptionIndex ware_index = iq.first->get_index();
			for (uint32_t j = 0; j < iq.first->get_filled(); ++j) {
				WareInstance* temp = new WareInstance(ware_index, tribe.get_ware_descr(ware_index));
				temp->init(game);
				temp->set_location(game, portdock_);
				return_wares->emplace_back(temp);
			}
			break;
		}
		case wwWORKER: {
			WorkersQueue& wq = dynamic_cast<WorkersQueue&>(*iq.first);
			while (wq.get_filled() > 0) {
				return_workers->emplace_back(wq.extract_worker());
			}
			break;
		}
		}
	}

	cleanup(game);
}

void ExpeditionBootstrap::save(FileWrite& fw, Game& game, MapObjectSaver& mos) {
	uint8_t number_warequeues = 0;
	for (auto& q : queues_) {
		if (q.first->get_type() == wwWARE) {
			number_warequeues++;
		}
	}
	fw.unsigned_8(queues_.size() - number_warequeues);
	for (auto& q : queues_) {
		if (q.first->get_type() == wwWORKER) {
			q.first->write(fw, game, mos);
			fw.unsigned_8(q.second ? 1 : 0);
		}
	}
	fw.unsigned_8(number_warequeues);
	for (auto& q : queues_) {
		if (q.first->get_type() == wwWARE) {
			q.first->write(fw, game, mos);
			fw.unsigned_8(q.second ? 1 : 0);
		}
	}
}

void ExpeditionBootstrap::load(
   Warehouse& warehouse, FileRead& fr, Game& game, MapObjectLoader& mol, uint16_t packet_version) {

	assert(queues_.empty());
	// Load worker queues
	std::vector<WorkersQueue*> wqs;
	std::vector<InputQueue*> additional_queues;
	try {
		if (packet_version == kCurrentPacketVersionWarehouseAndExpedition) {
			uint8_t num_queues = fr.unsigned_8();
			for (uint8_t i = 0; i < num_queues; ++i) {
				WorkersQueue* wq = new WorkersQueue(warehouse, INVALID_INDEX, 0);
				wq->read(fr, game, mol);
				const bool removable = fr.unsigned_8();
				wq->set_callback(input_callback, this);

				if (wq->get_index() == INVALID_INDEX) {
					delete wq;
				} else if (removable) {
					additional_queues.push_back(wq);
				} else {
					wqs.push_back(wq);
				}
			}
		} else {
			throw UnhandledVersionError(
			   "ExpeditionBootstrap", packet_version, kCurrentPacketVersionWarehouseAndExpedition);
		}

		// Load ware queues
		uint8_t num_queues = fr.unsigned_8();
		for (uint8_t i = 0; i < num_queues; ++i) {
			WaresQueue* wq = new WaresQueue(warehouse, INVALID_INDEX, 0);
			wq->read(fr, game, mol);
			bool removable = packet_version >= 8 ? fr.unsigned_8() : false;
			wq->set_callback(input_callback, this);

			if (wq->get_index() == INVALID_INDEX) {
				delete wq;
			} else if (removable) {
				additional_queues.push_back(wq);
			} else {
				queues_.push_back(std::make_pair(std::unique_ptr<InputQueue>(wq), false));
			}
		}
		// Append worker queues to the end
		for (InputQueue* wq : wqs) {
			queues_.emplace_back(std::make_pair(std::unique_ptr<InputQueue>(wq), false));
		}
		for (InputQueue* wq : additional_queues) {
			assert(wq->get_max_size() == 1);
			queues_.emplace_back(std::make_pair(std::unique_ptr<InputQueue>(wq), true));
		}
	} catch (const GameDataError& e) {
		throw GameDataError("loading ExpeditionBootstrap: %s", e.what());
	}
}

}  // namespace Widelands
