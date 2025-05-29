/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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
#include "economy/flag.h"
#include "graphic/style_manager.h"
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

	carrier_request_.reset(
	   new Request(*this, descr().local_carrier, Market::carrier_callback, wwWORKER));

	if (upcast(Game, game, &egbase); game != nullptr) {
		send_message(*game, Message::Type::kTrading, descr().descname(), descr().icon_filename(),
		             descr().descname(), _("A new market was added to your economy."), true);
	}

	return true;
}

void Market::cleanup(EditorGameBase& egbase) {
	if (upcast(Game, game, &egbase); game != nullptr) {
		std::vector<TradeID> to_cancel;
		std::vector<TradeID> to_retract;
		for (const auto& pair : game->all_trade_agreements()) {
			if (pair.second.initiator == this || pair.second.receiver == this) {
				if (pair.second.state == TradeInstance::State::kProposed) {
					to_retract.push_back(pair.first);
				} else {
					to_cancel.push_back(pair.first);
				}
			}
		}
		for (TradeID id : to_cancel) {
			game->cancel_trade(id, false, get_owner());
		}
		for (TradeID id : to_retract) {
			game->retract_trade(id);
		}
	}

	assert(trade_orders_.empty());

	if (Worker* carrier = carrier_.get(egbase); carrier != nullptr) {
		carrier->set_location(nullptr);
	}

	Building::cleanup(egbase);
}

void Market::TradeOrder::cleanup() {
	upcast(Game, game, &market->get_owner()->egbase());

	while (carriers_queue_->get_filled() > 0) {
		Worker* worker = carriers_queue_->extract_worker();

		if (worker->get_state(Worker::taskCarryTradeItem) != nullptr) {
			worker->send_signal(*game, "trade");
		} else {
			worker->set_location(&market->base_flag());
			worker->reset_tasks(*game);
			worker->start_task_return(*game, true);
		}
	}

	carriers_queue_->cleanup();

	for (auto& pair : wares_queues_) {
		for (int i = pair.second->get_filled(); i > 0; --i) {
			market->pending_dropout_wares_.push_back(pair.second->get_index());
			market->get_economy(wwWARE)->add_wares_or_workers(pair.second->get_index(), 1);
		}

		pair.second->cleanup();
	}

	if (Worker* carrier = market->carrier_.get(*game); carrier != nullptr) {
		carrier->update_task_buildingwork(*game);
	}
}

void Market::kickout_worker_from_queue(Game& game, Worker& worker) {
	if (worker.get_state(Worker::taskCarryTradeItem) != nullptr) {
		worker.send_signal(game, "trade");
	} else {
		Building::kickout_worker_from_queue(game, worker);
	}
}

std::string Market::market_census_string() const {
	static const std::string fmt = "⇵&nbsp;%s&nbsp;⇵";
	return named_building_census_string(fmt, get_market_name());
}

void Market::update_statistics_string(std::string* str) {
	const unsigned nr_trades = trade_orders_.size();
	*str = StyleManager::color_tag(format(ngettext("%u trade", "%u trades", nr_trades), nr_trades),
	                               g_style_manager->building_statistics_style().neutral_color());
}

void Market::remove_worker(Worker& worker) {
	Building::remove_worker(worker);

	if (carrier_.serial() == worker.serial()) {
		carrier_ = nullptr;
		return;
	}

	for (auto& pair : trade_orders_) {
		if (pair.second->carriers_queue_->remove_if_present(worker)) {
			return;
		}
	}
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
	} else {
		if (carrier_request_ != nullptr) {
			carrier_request_->set_economy(e);
		}
	}

	for (auto& pair : trade_orders_) {
		if (type == wwWARE) {
			if (old != nullptr) {
				for (auto& q : pair.second->wares_queues_) {
					q.second->remove_from_economy(*old);
				}
			}

			if (e != nullptr) {
				for (auto& q : pair.second->wares_queues_) {
					q.second->add_to_economy(*e);
				}
			}
		} else {
			if (old != nullptr) {
				pair.second->carriers_queue_->remove_from_economy(*old);
			}
			if (e != nullptr) {
				pair.second->carriers_queue_->add_to_economy(*old);
			}
		}
	}
}

void Market::set_market_name(const std::string& name) {
	market_name_ = name;
	get_owner()->reserve_warehousename(name);
}

void Market::notify_trade_extended(const TradeID trade_id, const int new_total_batches) {
	trade_orders_.at(trade_id)->initial_num_batches = new_total_batches;
}

void Market::new_trade(const TradeID trade_id,
                       const BillOfMaterials& items,
                       const int32_t num_batches,
                       OPtr<Market> other_side) {
	MutexLock m(MutexLock::ID::kObjects);

	assert(trade_orders_.count(trade_id) == 0);
	std::unique_ptr<TradeOrder>& trade_order = trade_orders_[trade_id];
	trade_order.reset(new TradeOrder);

	trade_order->market = this;
	trade_order->items = items;
	trade_order->initial_num_batches = num_batches;
	trade_order->other_side = other_side;

	trade_order->carriers_queue_.reset(
	   new WorkersQueue(*this, descr().trade_carrier, trade_order->num_wares_per_batch()));
	trade_order->carriers_queue_->set_callback(Market::ware_arrived_callback, this);
	for (const auto& entry : items) {
		auto& queue = trade_order->wares_queues_[entry.first];
		queue.reset(new WaresQueue(*this, entry.first, entry.second));
		queue->set_callback(Market::ware_arrived_callback, this);
	}

	molog(owner().egbase().get_gametime(), "Enqueuing new trade #%u with %d batches to %u", trade_id,
	      num_batches, other_side.serial());
	Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kChanged));
}

void Market::cancel_trade(Game& game,
                          const TradeID trade_id,
                          const bool reached_regular_end,
                          const bool send_msg) {
	MutexLock m(MutexLock::ID::kObjects);

	if (auto it = trade_orders_.find(trade_id); it != trade_orders_.end()) {
		molog(owner().egbase().get_gametime(),
		      reached_regular_end ? "Completed trade #%u" : "Cancelling trade #%u", trade_id);

		if (send_msg) {
			send_message(
			   game, Message::Type::kTrading,
			   reached_regular_end ? _("Trade Complete") : _("Trade Cancelled"),
			   descr().icon_filename(),
			   reached_regular_end ? _("Trade agreement complete") : _("Trade agreement cancelled"),
			   format(reached_regular_end ? _("Your trade agreement at %s has been completed.") :
			                                _("Your trade agreement at %s has been cancelled."),
			          get_market_name()),
			   false);
		}

		it->second->cleanup();
		trade_orders_.erase(trade_id);
		Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kChanged));
	} else {
		molog(owner().egbase().get_gametime(), "cancel_trade: trade #%u not found", trade_id);
	}
}

void Market::move_trade_to(Game& game, const TradeID trade_id, Market& dest) {
	const auto it = trade_orders_.find(trade_id);
	if (it == trade_orders_.end()) {
		return;  // Trade no longer exists
	}

	Market* other_market = it->second->other_side.get(game);
	if (other_market == nullptr) {
		return;  // Other side no longer exists
	}
	const auto other_it = other_market->trade_orders_.find(trade_id);
	if (other_it == other_market->trade_orders_.end() ||
	    other_it->second->other_side.serial() != serial()) {
		return;  // Trade no longer exists
	}

	// Clear the old queues and dismiss the trade's carriers
	it->second->cleanup();

	// Move the trade
	assert(dest.trade_orders_.count(trade_id) == 0);
	TradeOrder* order = it->second.release();
	trade_orders_.erase(it);
	order->market = &dest;
	dest.trade_orders_.emplace(trade_id, order);
	assert(dest.trade_orders_.count(trade_id) == 1);

	other_it->second->other_side = &dest;

	// Create the new queues
	order->carriers_queue_.reset(
	   new WorkersQueue(dest, descr().trade_carrier, order->num_wares_per_batch()));
	order->carriers_queue_->set_callback(Market::ware_arrived_callback, &dest);
	for (const auto& entry : order->items) {
		auto& queue = order->wares_queues_[entry.first];
		queue.reset(new WaresQueue(dest, entry.first, entry.second));
		queue->set_callback(Market::ware_arrived_callback, &dest);
	}
}

void Market::carrier_callback(Game& game,
                              Request& /* rq */,
                              DescriptionIndex /* widx */,
                              Worker* carrier,
                              PlayerImmovable& target) {
	Market& market = dynamic_cast<Market&>(target);
	market.carrier_request_.reset();
	market.carrier_ = carrier;
	assert(carrier != nullptr);
	assert(carrier->get_location(game) == &market);
	carrier->reset_tasks(game);
	carrier->start_task_buildingwork(game);
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

		Market* other_market = pair.second->other_side.get(*game);
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
	const TradeOrder& trade_order = *it->second;
	assert(!trade_order.fulfilled());

	if (trade_order.paused) {
		return false;
	}

	// Do we have all necessary carriers and wares for a batch?
	if (static_cast<int>(trade_order.carriers_queue_->get_filled()) <
	    trade_order.num_wares_per_batch()) {
		return false;
	}
	for (Worker* carrier : trade_order.carriers_queue_->workers_in_queue()) {
		if (carrier->get_position() != get_position() || !carrier->is_idle()) {
			return false;
		}
	}

	return std::all_of(trade_order.items.begin(), trade_order.items.end(),
	                   [&trade_order](const WareAmount& ware_amount) {
		                   const auto wares_it = trade_order.wares_queues_.find(ware_amount.first);
		                   return wares_it != trade_order.wares_queues_.end() &&
		                          wares_it->second->get_filled() >= ware_amount.second;
	                   });
}

void Market::launch_batch(const TradeID trade_id, Game* game) {
	assert(is_ready_to_launch_batch(trade_id));
	TradeOrder& trade_order = *trade_orders_.at(trade_id);
	molog(game->get_gametime(), "Launching batch for trade #%u", trade_id);

	// Do we have all necessary wares for a batch?
	Worker** next_carrier = trade_order.carriers_queue_->workers_in_queue().data();
	for (const auto& item_pair : trade_order.items) {
		for (size_t i = 0; i < item_pair.second; ++i) {
			Worker* carrier = *next_carrier++;

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
			carrier->start_task_carry_trade_item(*game, trade_id, trade_order.other_side.get(*game));
		}
	}
}

InputQueue& Market::inputqueue(const DescriptionIndex index,
                               const WareWorker ware_worker,
                               const Request* r,
                               const uint32_t disambiguator_id) {
	std::pair<InputQueue*, TradeID> pair = find_inputqueue(index, ware_worker, r, disambiguator_id);
	if (pair.first != nullptr) {
		return *pair.first;
	}

	// The parent will throw an exception.
	return Building::inputqueue(index, ware_worker, r, disambiguator_id);
}

std::pair<InputQueue*, TradeID> Market::find_inputqueue(const DescriptionIndex index,
                                                        const WareWorker ware_worker,
                                                        const Request* r,
                                                        const uint32_t disambiguator_id) {
	if (r != nullptr) {
		for (auto& pair : trade_orders_) {
			if (ware_worker == wwWARE) {
				auto it = pair.second->wares_queues_.find(index);
				if (it != pair.second->wares_queues_.end() && it->second->matches(*r)) {
					assert(it->second->get_index() == index);
					return {it->second.get(), pair.first};
				}
			} else if (pair.second->carriers_queue_->matches(*r)) {
				return {pair.second->carriers_queue_.get(), pair.first};
			}
		}
	} else {
		if (auto pair = trade_orders_.find(disambiguator_id); pair != trade_orders_.end()) {
			if (ware_worker == wwWARE) {
				auto it = pair->second->wares_queues_.find(index);
				if (it != pair->second->wares_queues_.end()) {
					assert(it->second->get_index() == index);
					return {it->second.get(), pair->first};
				}
			} else if (pair->second->carriers_queue_->get_index() == index) {
				return {pair->second->carriers_queue_.get(), pair->first};
			}
		}
	}

	return {nullptr, kInvalidTrade};
}

bool Market::can_change_max_fill(const DescriptionIndex index,
                                 const WareWorker ware_worker,
                                 const Request* r,
                                 const uint32_t disambiguator_id) {
	const std::pair<InputQueue*, TradeID> pair =
	   find_inputqueue(index, ware_worker, r, disambiguator_id);
	const auto order = trade_orders_.find(pair.second);
	return order != trade_orders_.end() && order->second->paused &&
	       Building::can_change_max_fill(index, ware_worker, r, disambiguator_id);
}

uint32_t Market::get_priority_disambiguator_id(const Request* req) const {
	if (req != nullptr) {
		for (const auto& pair : trade_orders_) {
			for (const auto& queue : pair.second->wares_queues_) {
				if (queue.second->matches(*req)) {
					return pair.first;
				}
			}
			if (pair.second->carriers_queue_->matches(*req)) {
				return pair.first;
			}
		}
	}

	return Building::get_priority_disambiguator_id(req);
}

bool Market::is_paused(const TradeID id) const {
	const auto trade_order = trade_orders_.find(id);
	return trade_order != trade_orders_.end() && trade_order->second->paused;
}

bool Market::can_resume(const TradeID id) const {
	if (!is_paused(id)) {
		return false;
	}

	const TradeOrder& order = *trade_orders_.at(id);
	for (const auto& pair : order.wares_queues_) {
		if (pair.second->get_max_fill() < pair.second->get_max_size()) {
			return false;
		}
	}
	return order.carriers_queue_->get_max_fill() >= order.carriers_queue_->get_max_size();
}

void Market::set_paused(Game& game, const TradeID id, const bool pause) {
	auto trade_order = trade_orders_.find(id);
	if (trade_order == trade_orders_.end() || trade_order->second->paused == pause) {
		return;
	}

	if (!pause && !can_resume(id)) {
		molog(game.get_gametime(), "Attempt to resume but cannot resume");
		return;
	}

	trade_order->second->paused = pause;

	if (Market* other = trade_order->second->other_side.get(game); other != nullptr) {
		if (pause) {
			other->send_message(game, Message::Type::kTrading, _("Trade Paused"),
			                    other->descr().icon_filename(), _("Trade agreement paused"),
			                    format_l(_("%1$s paused the trade with you at %2$s."),
			                             other->owner().get_name(), other->get_market_name()),
			                    true);
		} else {
			other->send_message(game, Message::Type::kTrading, _("Trade Resumed"),
			                    other->descr().icon_filename(), _("Trade agreement resumed"),
			                    format_l(_("%1$s resumed the trade with you at %2$s."),
			                             other->owner().get_name(), other->get_market_name()),
			                    true);
		}
	}

	Notifications::publish(NoteTradeChanged(
	   id, pause ? NoteTradeChanged::Action::kPaused : NoteTradeChanged::Action::kUnpaused));

	if (!pause) {
		try_launching_batch(&game);
	}
}

InputQueue* Market::find_overfull_input_queue() {
	for (auto& order : trade_orders_) {
		for (auto& pair : order.second->wares_queues_) {
			if (pair.second->get_type() == wwWARE &&
			    pair.second->get_filled() > pair.second->get_max_fill()) {
				return pair.second.get();
			}
		}
	}
	return nullptr;
}

void Market::inputqueue_max_fill_changed() {
	Building::inputqueue_max_fill_changed();

	if (find_overfull_input_queue() != nullptr) {
		if (Worker* carrier = carrier_.get(get_owner()->egbase()); carrier != nullptr) {
			carrier->update_task_buildingwork(dynamic_cast<Game&>(get_owner()->egbase()));
		}
	}
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
		WareInstance& ware =
		   *new WareInstance(pending_dropout_wares_.front(),
		                     owner().tribe().get_ware_descr(pending_dropout_wares_.front()));
		ware.init(game);
		worker.start_task_dropoff(game, ware);
		pending_dropout_wares_.pop_front();
		return true;
	}

	if (InputQueue* queue = find_overfull_input_queue(); queue != nullptr) {
		queue->set_filled(queue->get_filled() - 1);
		const WareDescr& wd = *owner().tribe().get_ware_descr(queue->get_index());
		WareInstance& ware = *new WareInstance(queue->get_index(), &wd);
		ware.init(game);
		worker.start_task_dropoff(game, ware);
		return true;
	}

	return false;
}

void Market::traded_ware_arrived(const TradeID trade_id,
                                 const DescriptionIndex ware_index,
                                 Game* game) {
	auto trade_order = trade_orders_.find(trade_id);

	if (trade_order != trade_orders_.end()) {
		++trade_order->second->received_traded_wares_in_this_batch;
	}

	pending_dropout_wares_.push_back(ware_index);
	get_economy(wwWARE)->add_wares_or_workers(ware_index, 1);
	get_owner()->ware_produced(ware_index);

	if (Worker* carrier = carrier_.get(*game); carrier != nullptr) {
		carrier->update_task_buildingwork(*game);
	}

	if (trade_order != trade_orders_.end()) {
		if (Market* other_market = trade_order->second->other_side.get(*game);
		    other_market != nullptr) {
			other_market->get_owner()->ware_consumed(ware_index, 1);

			auto other_trade_order = other_market->trade_orders_.find(trade_id);
			if (other_trade_order != other_market->trade_orders_.end() &&
			    trade_order->second->received_traded_wares_in_this_batch ==
			       other_trade_order->second->num_wares_per_batch() &&
			    other_trade_order->second->received_traded_wares_in_this_batch ==
			       trade_order->second->num_wares_per_batch()) {
				// This batch is completed.
				++trade_order->second->num_shipped_batches;
				trade_order->second->received_traded_wares_in_this_batch = 0;
				++other_trade_order->second->num_shipped_batches;
				other_trade_order->second->received_traded_wares_in_this_batch = 0;
				if (trade_order->second->fulfilled()) {
					assert(other_trade_order->second->fulfilled());
					game->cancel_trade(trade_id, true, get_owner());
				}
			}
		}
	}

	Notifications::publish(NoteTradeChanged(trade_id, NoteTradeChanged::Action::kWareArrived));
}

void Market::log_general_info(const EditorGameBase& egbase) const {
	molog(egbase.get_gametime(), "Market '%s'", market_name_.c_str());
	Building::log_general_info(egbase);

	molog(egbase.get_gametime(), "%" PRIuS " trade orders", trade_orders_.size());
	for (const auto& pair : trade_orders_) {
		molog(egbase.get_gametime(), "  - #%6u: %3d/%3d to %6u, received %4d", pair.first,
		      pair.second->num_shipped_batches, pair.second->initial_num_batches,
		      pair.second->other_side.serial(), pair.second->received_traded_wares_in_this_batch);
		for (const auto& ware_amount : pair.second->items) {
			molog(egbase.get_gametime(), "    - %3u x %s", ware_amount.second,
			      owner().tribe().get_ware_descr(ware_amount.first)->name().c_str());
		}
	}
}

std::string TradeInstance::format_richtext(const TradeID id,
                                           const EditorGameBase& egbase,
                                           const PlayerNumber iplayer,
                                           const bool can_act) const {
	constexpr int16_t kSpacing = 4;
	assert((iplayer == receiving_player) ^ (iplayer == sending_player));
	const bool is_receiver = iplayer == receiving_player;

	Widelands::Market* own_market = initiator.get(egbase);
	Widelands::Market* other_market = receiver.get(egbase);
	if (is_receiver) {
		std::swap(own_market, other_market);
	}

	std::string infotext("<rt><p>");
	if (state == State::kRunning) {  // Active trade
		if (own_market == nullptr || other_market == nullptr) {
			return std::string();
		}

		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelHeading,
		   format_l(
		      /** TRANSLATORS: "At" is the market's name, and "with" is the receiving player's name */
		      _("Trade agreement at %1$s with %2$s"), own_market->get_market_name(),
		      other_market->owner().get_name()));
	} else if (is_receiver) {  // Offered trade
		if (other_market == nullptr) {
			return std::string();
		}

		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelHeading,
		                        format_l(_("Trade offer from %s"), other_market->owner().get_name()));
	} else {  // Proposed trade
		if (own_market == nullptr) {
			return std::string();
		}

		infotext += as_font_tag(
		   UI::FontStyle::kWuiInfoPanelHeading,
		   format_l(
		      /** TRANSLATORS: "From" is the market's name, and "to" is the receiving player's name */
		      _("Trade offer from %1$s to %2$s"), own_market->get_market_name(),
		      egbase.player(receiving_player).get_name()));
	}

	infotext += format("</p><div padding_l=%d padding_b=%d><p>",
	                   g_style_manager->styled_size(UI::StyledSize::kUIDefaultIndent),
	                   g_style_manager->styled_size(UI::StyledSize::kWuiTextDefaultGap));
	if (num_batches != kInfiniteTrade) {
		infotext +=
		   as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph,
		               format_l(ngettext("%d batch", "%d batches", num_batches), num_batches));
	} else {
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph, _("Trade runs indefinitely"));
	}

	if (state == State::kRunning) {
		const auto trade = own_market->trade_orders().find(id);
		if (trade == own_market->trade_orders().end()) {
			return std::string();
		}

		infotext += "</p><p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph,
		                        format_l(ngettext("%d batch delivered", "%d batches delivered",
		                                          trade->second->num_shipped_batches),
		                                 trade->second->num_shipped_batches));

		if (num_batches != kInfiniteTrade) {
			infotext += "</p><p>";
			infotext +=
			   as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph,
			               format_l(ngettext("%d batch remaining", "%d batches remaining",
			                                 num_batches - trade->second->num_shipped_batches),
			                        num_batches - trade->second->num_shipped_batches));
		}

		bool other_paused = false;
		if (const auto other_trade = other_market->trade_orders().find(id);
		    other_trade != other_market->trade_orders().end()) {
			other_paused = other_trade->second->paused;
		}

		std::string paused_text;
		if (trade->second->paused) {
			if (other_paused) {
				paused_text = _("Paused by both players");
			} else {
				paused_text = can_act ? _("Paused by you") :
				                        format_l(_("Paused by %s"), own_market->owner().get_name());
			}
		} else {
			if (other_paused) {
				paused_text = format_l(_("Paused by %s"), other_market->owner().get_name());
			} else {
				paused_text = _("Active");
			}
		}

		infotext += "</p><p>";
		infotext += as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph, paused_text);
	}

	infotext += "</p>";
	infotext += as_vspace(kSpacing);
	infotext += "<p>";
	infotext += as_font_tag(
	   UI::FontStyle::kWuiInfoPanelHeading, can_act ? _("You send:") : _("Player sends:"));
	for (const auto& pair : is_receiver ? items_to_receive : items_to_send) {
		infotext +=
		   as_listitem(format_l(_("%1$i× %2$s"), pair.second,
		                        egbase.descriptions().get_ware_descr(pair.first)->descname()),
		               UI::FontStyle::kWuiInfoPanelParagraph);
	}

	infotext += "</p>";
	infotext += as_vspace(kSpacing);
	infotext += "<p>";
	infotext += as_font_tag(
	   UI::FontStyle::kWuiInfoPanelHeading, can_act ? _("You receive:") : _("Player receives:"));
	for (const auto& pair : is_receiver ? items_to_send : items_to_receive) {
		infotext +=
		   as_listitem(format_l(_("%1$i× %2$s"), pair.second,
		                        egbase.descriptions().get_ware_descr(pair.first)->descname()),
		               UI::FontStyle::kWuiInfoPanelParagraph);
	}
	infotext += "</p></div></rt>";

	return infotext;
}
}  // namespace Widelands
