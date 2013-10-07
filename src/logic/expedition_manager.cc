/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#include "logic/expedition_manager.h"

#include <boost/foreach.hpp>

#include "economy/portdock.h"
#include "log.h" // NOCOM(#sirver): remove again
#include "logic/player.h"
#include "logic/warehouse.h"
#include "upcast.h"
#include "wui/interactive_gamebase.h"


// NOCOM(#sirver): maybe rename just to Expedition.
// // NOCOM(#sirver): remove the priority buttons and stuff from the expedition window.

namespace Widelands {

struct ExpeditionManager::ExpeditionWorker {
	// Ownership is taken.
	ExpeditionWorker(Request * r) : request(r), worker(nullptr) {}

	std::unique_ptr<Request> request;
	std::unique_ptr<Worker> worker;
};

ExpeditionManager::ExpeditionManager(PortDock* const portdock)
	 : portdock_(portdock), economy_(portdock->get_warehouse()->get_economy()) {}

ExpeditionManager::~ExpeditionManager() {
	assert(workers_.empty());
	assert(wares_.empty());
}

void ExpeditionManager::is_expedition_ready(Game & game) {
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	BOOST_FOREACH(std::unique_ptr<WaresQueue>& wq, wares_) {
		log("#sirver wq->get_max_fill(): %u,wq->get_filled(): %u\n",
				wq->get_max_fill(), wq->get_filled());
		if (wq->get_max_fill() != wq->get_filled())
			return;
	}

	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	BOOST_FOREACH(std::unique_ptr<ExpeditionWorker>& ew, workers_) {
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		if (ew->request)
			return;
	}

	// If this point is reached, all needed wares and workers are stored and waiting for a ship
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	portdock_->expedition_is_ready(game);
}

// static
void ExpeditionManager::ware_callback(Game& game, WaresQueue*, Ware_Index, void* const data)
{
	ExpeditionManager* pd = static_cast<ExpeditionManager *>(data);
	pd->is_expedition_ready(game);
}

// static
void ExpeditionManager::worker_callback
	(Game& game, Request& r, Ware_Index, Worker* worker, PlayerImmovable& pd) {
	Warehouse* warehouse = static_cast<Warehouse *>(&pd);

	warehouse->get_portdock()->expedition_manager()->handle_worker_callback(game, r, worker);
}

void ExpeditionManager::handle_worker_callback(Game& game, Request& request, Worker* worker) {
	BOOST_FOREACH(std::unique_ptr<ExpeditionWorker>& ew, workers_) {
		if (ew->request.get() == &request) {
			ew->request.reset(nullptr);  // deletes &request.
			ew->worker.reset(worker);

			is_expedition_ready(game);
			return;
		}
	}
	// Never here, otherwise we would have a callback for a request we know
	// nothing about.
	assert(false);
}

void ExpeditionManager::start_expedition() {
	assert(workers_.empty());
	assert(wares_.empty());

	// Load the buildcosts for the port building + builder
	Warehouse* const warehouse = portdock_->get_warehouse();

	const std::map<Ware_Index, uint8_t>& buildcost = warehouse->descr().buildcost();
	size_t const buildcost_size = buildcost.size();

	// Issue request for wares for this expedition.
	// TODO(sirver): could try to get some of these directly form the warehouse.
	// But this is really a premature optimization and should probably be
	// handled in the economy code.
	wares_.resize(buildcost_size);
	std::map<Ware_Index, uint8_t>::const_iterator it = buildcost.begin();
	for (size_t i = 0; i < buildcost_size; ++i, ++it) {
		WaresQueue* wq = new WaresQueue(*warehouse, it->first, it->second);
		wq->set_callback(ware_callback, this);
		wares_[i].reset(wq);
	}

	// Issue the request for the workers (so far only a builder).
	workers_.emplace_back
		(new ExpeditionWorker
		 (new Request(*warehouse,
						  warehouse->owner().tribe().safe_worker_index("builder"),
						  ExpeditionManager::worker_callback, wwWORKER))
	);

	// Update the user interface
	if (upcast(Interactive_GameBase, igb, warehouse->owner().egbase().get_ibase()))
		warehouse->refresh_options(*igb);
}

void ExpeditionManager::cancel_expedition(Game& game) {
	// Put all wares from the WaresQueues back into the warehouse
	Warehouse* const warehouse = portdock_->get_warehouse();
	BOOST_FOREACH(std::unique_ptr<WaresQueue>& wq, wares_) {
		warehouse->insert_wares(wq->get_ware(), wq->get_filled());
		wq->cleanup();
	}
	wares_.clear();

	// Send all workers from the expedition list back inside the warehouse
	BOOST_FOREACH(std::unique_ptr<ExpeditionWorker>& ew, workers_) {
		if (ew->worker) {
			warehouse->incorporate_worker(game, *ew->worker.release());
		}
	}
	workers_.clear();

	// Update the user interface
	if (upcast(Interactive_GameBase, igb, warehouse->owner().egbase().get_ibase()))
		warehouse->refresh_options(*igb);
}

void ExpeditionManager::cleanup(Editor_Game_Base& egbase) {
	// Send all workers from the expedition list back inside the warehouse
	BOOST_FOREACH(std::unique_ptr<ExpeditionWorker>& ew, workers_) {
		if (ew->worker) {
			ew->worker->cleanup(egbase);
		}
	}
	workers_.clear();

	BOOST_FOREACH(std::unique_ptr<WaresQueue>& wq, wares_) {
		wq->cleanup();
	}
	wares_.clear();
}

WaresQueue& ExpeditionManager::waresqueue(Ware_Index index) const {
	BOOST_FOREACH(const std::unique_ptr<WaresQueue>& wq, wares_) {
		if (wq->get_ware() == index) {
			return *wq.get();
		}
	}
	assert(false); // Never here, otherwise we do not have a queue for this ware.
}

std::vector<WaresQueue*> ExpeditionManager::wares() const {
	std::vector<WaresQueue*> return_value;
	BOOST_FOREACH(const std::unique_ptr<WaresQueue>& wq, wares_) {
		return_value.emplace_back(wq.get());
	}
	log("#sirver return_values.size(): %u\n", return_value.size());
	return return_value;
}

void ExpeditionManager::set_economy(Economy* new_economy) {
	if (new_economy == economy_)
		return;

	// Transfer the wares.
	BOOST_FOREACH(std::unique_ptr<WaresQueue>& wq, wares_) {
		if (economy_)
			wq->remove_from_economy(*economy_);
		if (new_economy)
			wq->add_to_economy(*new_economy);
	}

	// Transfer the workers.
	BOOST_FOREACH(std::unique_ptr<ExpeditionWorker>& ew, workers_) {
		if (ew->request) {
			ew->request->set_economy(new_economy);
		}
		if (ew->worker)
			ew->worker->set_economy(new_economy);
	}

	economy_ = new_economy;
}

void ExpeditionManager::get_waiting_workers_and_wares
	(Game& game, const Tribe_Descr& tribe, std::vector<Worker*>* return_workers,
	 std::vector<WareInstance*>* return_wares)
{
	BOOST_FOREACH(std::unique_ptr<WaresQueue>& wq, wares_) {
		const Ware_Index ware_index = wq->get_ware();
		for (uint32_t j = 0; j < wq->get_filled(); ++j) {
			WareInstance* temp = new WareInstance(ware_index, tribe.get_ware_descr(ware_index));
			temp->init(game);
			temp->set_location(game, portdock_);
			return_wares->emplace_back(temp);
		}
		wq->set_filled(0);
		wq->set_max_fill(0);
	}

	BOOST_FOREACH(std::unique_ptr<ExpeditionWorker>& ew, workers_) {
		assert(ew->worker.get() != nullptr);
		assert(!ew->request);
		return_workers->emplace_back(ew->worker.release());
	}

	cleanup(game);
}

}  // namespace Widelands
