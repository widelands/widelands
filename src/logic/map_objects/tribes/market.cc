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

Market::Market(const MarketDescr& descr) : Building(descr) {}

Market::~Market() {}

void Market::new_trade(const int trade_id,
                       const BillOfMaterials& items,
                       const int num_batches,
                       const Serial other_side) {
	trade_orders_[trade_id] = TradeOrder{items, num_batches, 0, other_side, {}};
	auto& trade_order = trade_orders_[trade_id];

	// Request one worker for each item in a batch.
	for (size_t i = 0; i < items.size(); ++i) {
		trade_order.worker_requests.emplace_back(
		   new Request(*this, descr().carrier(), Market::request_worker_callback, wwWORKER));
	}
}

void Market::request_worker_callback(
   Game& game, Request& rq, DescriptionIndex /* widx */, Worker* const w, PlayerImmovable& target) {
	auto& market = dynamic_cast<Market&>(target);

	assert(w);
	assert(w->get_location(game) == &market);

	// NOCOM(#sirver): We have to go through all trade_orders and search for the pointer of the request to identify for which trade order we asked for the trade. It should never happen that a request call this after it was canceled/deleted, so we should always have a match.
	for (auto& 
	// NOCOM(#sirver): once this is implemented, we need to expose an interface to Lua to create trade agreements for testing.
	
	// If there is more than one working position, it's possible, that different level workers are
	// requested and therefor possible, that a higher qualified worker answers a request for a lower
	// leveled worker, although a worker with equal level (as the arrived worker has) is needed as
	// well.
	// Therefor, we first check whether the worker exactly fits the requested one. If yes, we place
	// the
	// worker and everything is fine, else we shuffle through the working positions, whether one of
	// them
	// needs a worker like the one just arrived. That way it is of course still possible, that the
	// worker is
	// placed on the slot that originally requested the arrived worker.
	bool worker_placed = false;
	DescriptionIndex idx = w->descr().worker_index();
	for (WorkingPosition* wp = market.working_positions_;; ++wp) {
		if (wp->worker_request == &rq) {
			if (wp->worker_request->get_index() == idx) {
				// Place worker
				delete &rq;
				*wp = WorkingPosition(nullptr, w);
				worker_placed = true;
			} else {
				// Set new request for this slot
				DescriptionIndex workerid = wp->worker_request->get_index();
				delete wp->worker_request;
				wp->worker_request = &market.request_worker(workerid);
			}
			break;
		}
	}
	while (!worker_placed) {
		{
			uint8_t nwp = market.descr().nr_working_positions();
			uint8_t pos = 0;
			WorkingPosition* wp = market.working_positions_;
			for (; pos < nwp; ++wp, ++pos) {
				// Find a fitting slot
				if (!wp->worker && !worker_placed)
					if (wp->worker_request->get_index() == idx) {
						delete wp->worker_request;
						*wp = WorkingPosition(nullptr, w);
						worker_placed = true;
						break;
					}
			}
		}
		if (!worker_placed) {
			// Find the next smaller version of this worker
			DescriptionIndex nuwo = game.tribes().nrworkers();
			DescriptionIndex current = 0;
			for (; current < nuwo; ++current) {
				WorkerDescr const* worker = game.tribes().get_worker_descr(current);
				if (worker->becomes() == idx) {
					idx = current;
					break;
				}
			}
			if (current == nuwo)
				throw wexception(
				   "Something went wrong! No fitting place for worker %s in %s at (%u, %u) found!",
				   w->descr().descname().c_str(), market.descr().descname().c_str(),
				   market.get_position().x, market.get_position().y);
		}
	}

	// It's always the first worker doing building work,
	// the others only idle. Still, we need to wake up the
	// primary worker if the worker that has just arrived is
	// the last one we need to start working.
	w->start_task_idle(game, 0, -1);
	market.try_start_working(game);
	Notifications::publish(NoteBuilding(market.serial(), NoteBuilding::Action::kWorkersChanged));

}

}  // namespace Widelands
