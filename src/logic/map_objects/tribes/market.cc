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

#include "logic/map_objects/tribes/market.h"

#include <memory>

#include "base/i18n.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/tribes.h"

namespace Widelands {

MarketDescr::MarketDescr(const std::string& init_descname,
                         const LuaTable& table,
                         const EditorGameBase& egbase)
   : BuildingDescr(init_descname, MapObjectType::MARKET, table, egbase) {
	i18n::Textdomain td("tribes");

	DescriptionIndex const woi = egbase.tribes().worker_index(table.get_string("carrier"));
	if (!egbase.tribes().worker_exists(woi)) {
		throw wexception("invalid");
	}
	carrier_ = woi;

	// TODO(sirver,trading): Add actual logic here.
}

Building& MarketDescr::create_object() const {
	return *new Market(*this);
}

Market::Market(const MarketDescr& the_descr) : Building(the_descr) {
}

Market::~Market() {
}

void Market::new_trade(const int trade_id,
                       const BillOfMaterials& items,
                       const int num_batches,
                       const Serial other_side) {
	trade_orders_[trade_id] = TradeOrder{items, num_batches, 0, other_side, nullptr, {}, {}};
	auto& trade_order = trade_orders_[trade_id];

	// Request one worker for each item in a batch.
	int num_required_carriers = 0;
	for (size_t i = 0; i < items.size(); ++i) {
		num_required_carriers += items[i].second;
	}
	trade_order.worker_request.reset(
	   new Request(*this, descr().carrier(), Market::worker_arrived_callback, wwWORKER));
	trade_order.worker_request->set_count(num_required_carriers);

	// Make sure we have a wares queue for each item in this.
	for (const auto& entry : items) {
		ensure_wares_queue_exists(entry.first);
	}
}

void Market::worker_arrived_callback(
   Game& game, Request& rq, DescriptionIndex /* widx */, Worker* const w, PlayerImmovable& target) {
	// NOCOM(#sirver): set_economy is not handled.
	auto& market = dynamic_cast<Market&>(target);

	assert(w);
	assert(w->get_location(game) == &market);

	for (auto& trade_order_pair : market.trade_orders_) {
		auto& trade_order = trade_order_pair.second;
		if (trade_order.worker_request.get() != &rq) {
			continue;
		}

		log("#sirver rq.get_count(): %d\n", rq.get_count());
		if (rq.get_count() == 0) {
			// Erase this request.
			trade_order.worker_request.reset();
		}
		w->start_task_idle(game, 0, -1);
		trade_order.workers.push_back(w);
		Notifications::publish(NoteBuilding(market.serial(), NoteBuilding::Action::kWorkersChanged));
		market.try_launching_batch(&game);
		return;
	}
	NEVER_HERE(); // We should have found and handled a match by now.
}

void Market::ware_arrived_callback(Game& g, InputQueue*, DescriptionIndex, Worker*, void* data) {
	Market& market = *static_cast<Market*>(data);
	market.try_launching_batch(&g);
}

void Market::try_launching_batch(Game* game) {
	for (auto& pair : trade_orders_) {
		if (!is_ready_to_launch_batch(pair.first)) {
			continue;
		}

		auto* other_market =
		   dynamic_cast<Market*>(game->objects().get_object(pair.second.other_side));
		if (other_market == nullptr) {
			// NOCOM(#sirver): can this even happen?
			// The other market seems to have vanished. The game tracks this and
			// should soon delete this trade request from us. We just ignore it.
			continue;
		}
		if (!other_market->is_ready_to_launch_batch(pair.first)) {
			continue;
		}
		launch_batch(pair.first, game);
		other_market->launch_batch(pair.first, game);
		break;
	}
}

bool Market::is_ready_to_launch_batch(const int trade_id) {
	log("#sirver trade_id: %d\n", trade_id);
	// NOCOM(#sirver): if a transfer is in progress, this should fail.
	const auto it = trade_orders_.find(trade_id);
	if (it == trade_orders_.end()) {
		return false;
	}
	auto& trade_order = it->second;

	// Do we have all necessary wares for a batch?
	int num_wares = 0;
	for (const auto& item_pair : trade_order.items) {
		log("#sirver item_pair.first: %d\n", item_pair.first);
		const auto wares_it = wares_queue_.find(item_pair.first);
		if (wares_it == wares_queue_.end()) {
			return false;
		}
		if (wares_it->second->get_filled() < item_pair.second) {
			return false;
		}
		num_wares += item_pair.second;
	}

	// Do we have enough people to carry wares?
	int num_available_carriers = 0;
	for (auto* worker : trade_order.workers) {
		num_available_carriers += worker->is_idle() ? 1 : 0;
	}
	return num_available_carriers == num_wares;
}

void Market::launch_batch(const int trade_id, Game* game) {
	log("#sirver LAUNCHING BATCH %s:%i\n", __FILE__, __LINE__);
	assert(is_ready_to_launch_batch(trade_id));
	auto& trade_order = trade_orders_.at(trade_id);

	// Do we have all necessary wares for a batch?
	for (const auto& item_pair : trade_order.items) {
		for (size_t i = 0; i < item_pair.second; ++i) {
			Worker* carrier = trade_order.workers.at(i);
			assert(carrier->is_idle());

			// Give the carrier a ware.
			WareInstance* ware =
			   new WareInstance(item_pair.first, game->tribes().get_ware_descr(item_pair.first));
			ware->init(*game);
			carrier->set_carried_ware(*game, ware);

			// We have to remove this item from our economy. Otherwise it would be
			// considered idle (since it has no transport associated with it) and
			// the engine would want to transfer it to the next warehouse.
			ware->set_economy(nullptr);
			wares_queue_.at(item_pair.first)
			   ->set_filled(wares_queue_.at(item_pair.first)->get_filled() - 1);

			// Send the carrier going.
			carrier->reset_tasks(*game);
			carrier->start_task_carry_trade_item(*game);
			// NOCOM(#sirver): pass the object
			carrier->top_state().objvar1 =
			   ObjectPointer(game->objects().get_object(trade_order.other_side));
		}
	}
	// NOCOM(#sirver): implement
}

void Market::ensure_wares_queue_exists(int ware_index) {
	if (wares_queue_.count(ware_index) > 0) {
		return;
	}
	wares_queue_[ware_index] =
	   std::unique_ptr<WaresQueue>(new WaresQueue(*this, ware_index, kMaxPerItemTradeBatchSize));
	wares_queue_[ware_index]->set_callback(Market::ware_arrived_callback, this);
}

InputQueue& Market::inputqueue(DescriptionIndex index, WareWorker ware_worker) {
	assert(ware_worker == wwWARE);
	auto it = wares_queue_.find(index);
	if (it != wares_queue_.end()) {
		return *it->second;
	}
	// The parent will throw an exception.
	return Building::inputqueue(index, ware_worker);
}

void Market::cleanup(EditorGameBase& egbase) {
	for (auto& pair : wares_queue_) {
		pair.second->cleanup();
	}
	Building::cleanup(egbase);
}

void Market::receive_ware(Game& game, DescriptionIndex ware_index) {
	WareInstance* ware = new WareInstance(ware_index, game.tribes().get_ware_descr(ware_index));
	ware->init(game);
	worker.start_task_dropoff(game, ware_index);
	owner().ware_produced(ware_index);
}

}  // namespace Widelands
