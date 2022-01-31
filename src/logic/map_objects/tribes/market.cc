/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "logic/map_objects/tribes/market.h"

#include <memory>

#include "base/i18n.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/player.h"

namespace Widelands {

MarketDescr::MarketDescr(const std::string& init_descname,
                         const LuaTable& table,
                         Descriptions& descriptions)
   : BuildingDescr(init_descname, MapObjectType::MARKET, table, descriptions) {

	DescriptionIndex const woi = descriptions.worker_index(table.get_string("carrier"));
	if (!descriptions.worker_exists(woi)) {
		throw wexception("The tribe does not define the worker in 'carrier'.");
	}
	carrier_ = woi;

	// TODO(sirver,trading): Add actual logic here.
}

Building& MarketDescr::create_object() const {
	return *new Market(*this);
}

int Market::TradeOrder::num_wares_per_batch() const {
	int sum = 0;
	for (const auto& item_pair : items) {
		sum += item_pair.second;
	}
	return sum;
}

bool Market::TradeOrder::fulfilled() const {
	return num_shipped_batches == initial_num_batches;
}

// TODO(sirver,trading): This needs to implement 'set_economy'. Maybe common code can be shared.
Market::Market(const MarketDescr& the_descr) : Building(the_descr) {
}

void Market::new_trade(const int trade_id,
                       const BillOfMaterials& items,
                       const int num_batches,
                       const Serial other_side) {
	trade_orders_[trade_id] = TradeOrder{items, num_batches, 0, other_side, 0, nullptr, {}};
	auto& trade_order = trade_orders_[trade_id];

	// Request one worker for each item in a batch.
	trade_order.worker_request.reset(
	   new Request(*this, descr().carrier(), Market::worker_arrived_callback, wwWORKER));
	trade_order.worker_request->set_count(trade_order.num_wares_per_batch());

	// Make sure we have a wares queue for each item in this.
	for (const auto& entry : items) {
		ensure_wares_queue_exists(entry.first);
	}
}

void Market::cancel_trade(const int trade_id) {
	// TODO(sirver,trading): Launch workers, release no longer required wares and delete now unneeded
	// 'WaresQueue's
	trade_orders_.erase(trade_id);
}

void Market::worker_arrived_callback(
   Game& game, Request& rq, DescriptionIndex /* widx */, Worker* const w, PlayerImmovable& target) {
	auto& market = dynamic_cast<Market&>(target);

	assert(w);
	assert(w->get_location(game) == &market);

	for (auto& trade_order_pair : market.trade_orders_) {
		auto& trade_order = trade_order_pair.second;
		if (trade_order.worker_request.get() != &rq) {
			continue;
		}

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
	NEVER_HERE();  // We should have found and handled a match by now.
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
			// TODO(sirver,trading): Can this even happen? Where is this function called from?
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

bool Market::is_ready_to_launch_batch(const int trade_id) const {
	const auto it = trade_orders_.find(trade_id);
	if (it == trade_orders_.end()) {
		return false;
	}
	const auto& trade_order = it->second;
	assert(!trade_order.fulfilled());

	// Do we have all necessary wares for a batch?
	for (const auto& item_pair : trade_order.items) {
		const auto wares_it = wares_queue_.find(item_pair.first);
		if (wares_it == wares_queue_.end()) {
			return false;
		}
		if (wares_it->second->get_filled() < item_pair.second) {
			return false;
		}
	}

	// Do we have enough people to carry wares?
	int num_available_carriers = 0;
	for (auto* worker : trade_order.workers) {
		num_available_carriers += worker->is_idle() ? 1 : 0;
	}
	return num_available_carriers == trade_order.num_wares_per_batch();
}

void Market::launch_batch(const int trade_id, Game* game) {
	assert(is_ready_to_launch_batch(trade_id));
	auto& trade_order = trade_orders_.at(trade_id);

	// Do we have all necessary wares for a batch?
	int worker_index = 0;
	for (const auto& item_pair : trade_order.items) {
		for (size_t i = 0; i < item_pair.second; ++i) {
			Worker* carrier = trade_order.workers.at(worker_index);
			++worker_index;
			assert(carrier->is_idle());

			// Give the carrier a ware.
			WareInstance* ware =
			   new WareInstance(item_pair.first, game->descriptions().get_ware_descr(item_pair.first));
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
			carrier->start_task_carry_trade_item(
			   *game, trade_id, ObjectPointer(game->objects().get_object(trade_order.other_side)));
		}
	}
}

void Market::ensure_wares_queue_exists(int ware_index) {
	if (wares_queue_.count(ware_index) > 0) {
		return;
	}
	wares_queue_[ware_index] =
	   std::unique_ptr<WaresQueue>(new WaresQueue(*this, ware_index, kMaxPerItemTradeBatchSize));
	wares_queue_[ware_index]->set_callback(Market::ware_arrived_callback, this);
}

InputQueue& Market::inputqueue(DescriptionIndex index, WareWorker ware_worker, const Request* r) {
	assert(ware_worker == wwWARE);
	auto it = wares_queue_.find(index);
	if (it != wares_queue_.end()) {
		return *it->second;
	}
	// The parent will throw an exception.
	return Building::inputqueue(index, ware_worker, r);
}

void Market::cleanup(EditorGameBase& egbase) {
	for (auto& pair : wares_queue_) {
		pair.second->cleanup();
	}
	Building::cleanup(egbase);
}

void Market::traded_ware_arrived(const int trade_id,
                                 const DescriptionIndex ware_index,
                                 Game* game) {
	auto& trade_order = trade_orders_.at(trade_id);

	WareInstance* ware =
	   new WareInstance(ware_index, game->descriptions().get_ware_descr(ware_index));
	ware->init(*game);

	// TODO(sirver,trading): This is a hack. We should have a worker that
	// carriers stuff out. At the moment this assumes this market is barbarians
	// (which is always correct right now), creates a carrier for each received
	// ware to drop it off. The carrier then leaves the building and goes home.
	const WorkerDescr& w_desc = *game->descriptions().get_worker_descr(
	   game->descriptions().worker_index("barbarians_carrier"));
	auto& worker = w_desc.create(*game, get_owner(), this, position_);
	worker.start_task_dropoff(*game, *ware);
	++trade_order.received_traded_wares_in_this_batch;
	get_owner()->ware_produced(ware_index);

	auto* other_market = dynamic_cast<Market*>(game->objects().get_object(trade_order.other_side));
	assert(other_market != nullptr);
	other_market->get_owner()->ware_consumed(ware_index, 1);
	auto& other_trade_order = other_market->trade_orders_.at(trade_id);
	if (trade_order.received_traded_wares_in_this_batch == other_trade_order.num_wares_per_batch() &&
	    other_trade_order.received_traded_wares_in_this_batch == trade_order.num_wares_per_batch()) {
		// This batch is completed.
		++trade_order.num_shipped_batches;
		trade_order.received_traded_wares_in_this_batch = 0;
		++other_trade_order.num_shipped_batches;
		other_trade_order.received_traded_wares_in_this_batch = 0;
		if (trade_order.fulfilled()) {
			assert(other_trade_order.fulfilled());
			// TODO(sirver,trading): This is not quite correct. This should for
			// example send a differnet message than actually canceling a trade.
			game->cancel_trade(trade_id);
		}
	}
}

}  // namespace Widelands
