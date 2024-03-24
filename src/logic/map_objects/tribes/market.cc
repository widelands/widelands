/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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
#include "graphic/text_layout.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/player.h"

namespace Widelands {

MarketDescr::MarketDescr(const std::string& init_descname,
                         const LuaTable& table,
                         Descriptions& descriptions)
   : BuildingDescr(init_descname, MapObjectType::MARKET, table, descriptions),
   local_carrier(descriptions.worker_index(table.get_string("local_carrier"))),
   trade_carrier(descriptions.worker_index(table.get_string("trade_carrier"))) {
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

Market::Market(const MarketDescr& the_descr) : Building(the_descr) {
}

bool Market::init(EditorGameBase& egbase) {
	Building::init(egbase);

	market_name_ = get_owner()->pick_warehousename(Player::WarehouseNameType::kMarket);

	carrier_request_.reset(new Request(*this, descr().local_carrier, Market::carrier_callback, wwWORKER));

	return true;
}

void Market::cleanup(EditorGameBase& egbase) {
	for (auto& pair : trade_orders_) {
		pair.second.cleanup(*this);
	}

	if (Worker* carrier = carrier_.get(egbase); carrier != nullptr) {
		carrier->set_location(nullptr);
	}

	Building::cleanup(egbase);
}

void Market::TradeOrder::cleanup(Market& market) {
	for (auto& pair : wares_queues_) {
		for (int i = pair.second->get_filled(); i > 0; --i) {
			market.pending_dropout_wares_.push_back(pair.second->get_index());
			market.get_economy(wwWARE)->add_wares_or_workers(pair.second->get_index(), 1);
		}

		pair.second->cleanup();
	}

	if (upcast(Game, game, &market.get_owner()->egbase()); game != nullptr) {
		if (Worker* carrier = market.carrier_.get(*game); carrier != nullptr) {
			carrier->update_task_buildingwork(*game);
		}
	}
}

void Market::update_statistics_string(std::string* str) {
	*str = richtext_escape(get_market_name());
}

void Market::set_economy(Economy* const e, WareWorker type) {
	Economy* const old = get_economy(type);

	if (old == e) {
		return;
	}

	Building::set_economy(e, type);

	if (Worker* carrier = carrier_.get(get_owner()->egbase()); carrier != nullptr) {
		carrier->set_economy(e, type);
	}

	if (type == wwWARE) {
		for (DescriptionIndex ware : pending_dropout_wares_) {
			if (old != nullptr) {
				old->remove_wares_or_workers(ware, 1);
			}
			if (e != nullptr) {
				e->add_wares_or_workers(ware, 1);
			}
		}
	}

	for (auto& pair : trade_orders_) {
		for (Worker* worker : pair.second.workers) {
			worker->set_economy(e, type);
		}

		if (type == wwWORKER) {
			if (pair.second.worker_request != nullptr) {
				pair.second.worker_request->set_economy(e);
			}
		} else {
			if (old != nullptr) {
				for (auto& pair : pair.second.wares_queues_) {
					pair.second->remove_from_economy(*old);
				}
			}

			if (e != nullptr) {
				for (auto& pair : pair.second.wares_queues_) {
					pair.second->add_to_economy(*e);
				}
			}
		}
	}
}

void Market::set_market_name(const std::string& name) {
	market_name_ = name;
	get_owner()->reserve_warehousename(name);
}

void Market::new_trade(const TradeID trade_id,
                       const BillOfMaterials& items,
                       const int num_batches,
                       const Serial other_side) {
	assert(trade_orders_.count(trade_id) == 0);
	TradeOrder& trade_order = trade_orders_[trade_id];

	trade_order.items = items;
	trade_order.initial_num_batches = num_batches;
	trade_order.other_side = other_side;

	// Request one worker for each item in a batch.
	trade_order.worker_request.reset(
	   new Request(*this, descr().trade_carrier, Market::worker_arrived_callback, wwWORKER));
	trade_order.worker_request->set_count(trade_order.num_wares_per_batch());

	// Make sure we have a wares queue for each item in this.
	for (const auto& entry : items) {
		auto& queue = trade_order.wares_queues_[entry.first];
		queue.reset(new WaresQueue(*this, entry.first, entry.second));
		queue->set_callback(Market::ware_arrived_callback, this);
	}

	molog(owner().egbase().get_gametime(), "Enqueuing new trade #%u with %d batches to %u", trade_id, num_batches, other_side);
	Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kChanged));
}

void Market::cancel_trade(Game& game, const TradeID trade_id, const bool reached_regular_end, const bool send_msg) {
	if (auto it = trade_orders_.find(trade_id); it != trade_orders_.end()) {
		molog(owner().egbase().get_gametime(), reached_regular_end ? "Completed trade #%u" : "Cancelling trade #%u", trade_id);

		if (send_msg) {
			send_message(game, reached_regular_end ? Message::Type::kTradeComplete : Message::Type::kTradeCancelled,
				reached_regular_end ? _("Trade Complete") : _("Trade Cancelled"),
				descr().icon_filename(),
				reached_regular_end ? _("Trade agreement complete") : _("Trade agreement cancelled"),
				format(reached_regular_end ? _("Your trade agreement at %s has been completed.") : _("Your trade agreement at %s has been cancelled."), get_market_name()),
				false);
		}

		it->second.cleanup(*this);
		trade_orders_.erase(trade_id);
		Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kChanged));
	} else {
		molog(owner().egbase().get_gametime(), "cancel_trade: trade #%u not found", trade_id);
	}
}

void Market::carrier_callback(Game& game, Request& /* rq */, DescriptionIndex /* widx */, Worker* carrier, PlayerImmovable& target) {
	Market& market = dynamic_cast<Market&>(target);
	market.carrier_request_.reset();
	market.carrier_ = carrier;
	assert(carrier != nullptr);
	assert(carrier->get_location(game) == &market);
	carrier->reset_tasks(game);
	carrier->start_task_buildingwork(game);
}

void Market::worker_arrived_callback(Game& game, Request& rq, DescriptionIndex /* widx */, Worker* const w, PlayerImmovable& target) {
	Market& market = dynamic_cast<Market&>(target);

	assert(w != nullptr);
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

void Market::ware_arrived_callback(Game& g,
                                   InputQueue* /* queue */,
                                   DescriptionIndex /* index */,
                                   Worker* /* worker */,
                                   void* data) {
	Market& market = *static_cast<Market*>(data);
	market.try_launching_batch(&g);
}

void Market::try_launching_batch(Game* game) {
	for (auto& pair : trade_orders_) {
		if (!is_ready_to_launch_batch(pair.first)) {
			continue;
		}

		Market* other_market =
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

bool Market::is_ready_to_launch_batch(const TradeID trade_id) const {
	const auto it = trade_orders_.find(trade_id);
	if (it == trade_orders_.end()) {
		return false;
	}
	const auto& trade_order = it->second;
	assert(!trade_order.fulfilled());

	// Do we have all necessary wares for a batch?
	for (const auto& item_pair : trade_order.items) {
		const auto wares_it = trade_order.wares_queues_.find(item_pair.first);
		if (wares_it == trade_order.wares_queues_.end()) {
			return false;
		}
		if (wares_it->second->get_filled() < item_pair.second) {
			return false;
		}
	}

	// Do we have enough people to carry wares?
	int num_available_carriers = 0;
	for (Worker* worker : trade_order.workers) {
		num_available_carriers += worker->is_idle() ? 1 : 0;
	}
	return num_available_carriers == trade_order.num_wares_per_batch();
}

void Market::launch_batch(const TradeID trade_id, Game* game) {
	assert(is_ready_to_launch_batch(trade_id));
	TradeOrder& trade_order = trade_orders_.at(trade_id);
	molog(game->get_gametime(), "Launching batch for trade #%u", trade_id);

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
			trade_order.wares_queues_.at(item_pair.first)
			   ->set_filled(trade_order.wares_queues_.at(item_pair.first)->get_filled() - 1);

			// Send the carrier going.
			carrier->reset_tasks(*game);
			carrier->start_task_carry_trade_item(
			   *game, trade_id, ObjectPointer(game->objects().get_object(trade_order.other_side)));
		}
	}
}

InputQueue& Market::inputqueue(DescriptionIndex index, WareWorker ware_worker, const Request* r) {
	assert(ware_worker == wwWARE);

	for (auto& pair : trade_orders_) {
		auto it = pair.second.wares_queues_.find(index);
		if (it != pair.second.wares_queues_.end() && it->second->matches(*r)) {
			return *it->second;
		}
	}

	// The parent will throw an exception.
	return Building::inputqueue(index, ware_worker, r);
}

bool Market::fetch_from_flag(Game& game) {
	++fetchfromflag_;

	if (Worker* carrier = carrier_.get(game); carrier != nullptr) {
		carrier->update_task_buildingwork(game);
	}

	return true;
}

bool Market::get_building_work(Game& game, Worker& worker, bool /* success */) {
	if (fetchfromflag_ > 0) {
		--fetchfromflag_;
		worker.start_task_fetchfromflag(game);
		return true;
	}

	if (!pending_dropout_wares_.empty()) {
		get_economy(wwWARE)->remove_wares_or_workers(pending_dropout_wares_.front(), 1);
		WareInstance& ware = *new WareInstance(pending_dropout_wares_.front(), owner().tribe().get_ware_descr(pending_dropout_wares_.front()));
		ware.init(game);
		worker.start_task_dropoff(game, ware);
		pending_dropout_wares_.pop_front();
		return true;
	}

	return false;
}

void Market::traded_ware_arrived(const TradeID trade_id,
                                 const DescriptionIndex ware_index,
                                 Game* game) {
	TradeOrder& trade_order = trade_orders_.at(trade_id);

	pending_dropout_wares_.push_back(ware_index);
	get_economy(wwWARE)->add_wares_or_workers(ware_index, 1);
	++trade_order.received_traded_wares_in_this_batch;
	get_owner()->ware_produced(ware_index);

	if (Worker* carrier = carrier_.get(*game); carrier != nullptr) {
		carrier->update_task_buildingwork(*game);
	}

	Market* other_market = dynamic_cast<Market*>(game->objects().get_object(trade_order.other_side));
	assert(other_market != nullptr);
	other_market->get_owner()->ware_consumed(ware_index, 1);
	TradeOrder& other_trade_order = other_market->trade_orders_.at(trade_id);
	if (trade_order.received_traded_wares_in_this_batch == other_trade_order.num_wares_per_batch() &&
	    other_trade_order.received_traded_wares_in_this_batch == trade_order.num_wares_per_batch()) {
		// This batch is completed.
		++trade_order.num_shipped_batches;
		trade_order.received_traded_wares_in_this_batch = 0;
		++other_trade_order.num_shipped_batches;
		other_trade_order.received_traded_wares_in_this_batch = 0;
		if (trade_order.fulfilled()) {
			assert(other_trade_order.fulfilled());
			game->cancel_trade(trade_id, true);
		}
	}
}

void Market::log_general_info(const EditorGameBase& egbase) const {
	molog(egbase.get_gametime(), "Market '%s'", market_name_.c_str());
	Building::log_general_info(egbase);

	molog(egbase.get_gametime(), "%" PRIuS " trade orders", trade_orders_.size());
	for (const auto& pair : trade_orders_) {
		molog(egbase.get_gametime(), "  - #%6u: %3d/%3d to %6u, received %4d", pair.first, pair.second.num_shipped_batches, pair.second.initial_num_batches, pair.second.other_side, pair.second.received_traded_wares_in_this_batch);
		for (const auto& ware_amount : pair.second.items) {
			molog(egbase.get_gametime(), "    - %3u x %s", ware_amount.second, owner().tribe().get_ware_descr(ware_amount.first)->name().c_str());
		}
	}
}
}  // namespace Widelands
