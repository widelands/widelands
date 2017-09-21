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
	trade_orders_[trade_id] = TradeOrder{items, num_batches, 0, other_side, {}};
	auto& trade_order = trade_orders_[trade_id];

	// Request one worker for each item in a batch.
	int num_required_carriers = 0;
	for (size_t i = 0; i < items.size(); ++i) {
		num_required_carriers += items[i].second;
	}
	trade_order.worker_request.reset(
	   new Request(*this, descr().carrier(), Market::request_worker_callback, wwWORKER));
	trade_order.worker_request->set_count(num_required_carriers);
}

void Market::request_worker_callback(
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
		Notifications::publish(NoteBuilding(market.serial(), NoteBuilding::Action::kWorkersChanged));
		// NOCOM(#sirver): try launching a batch now.
		return;
	}
	NEVER_HERE(); // We should have found and handled a match by now.
}

}  // namespace Widelands
