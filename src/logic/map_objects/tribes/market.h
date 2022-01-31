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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H

#include <memory>

#include "economy/request.h"
#include "economy/wares_queue.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/map_objects/tribes/building.h"

namespace Widelands {

class MarketDescr : public BuildingDescr {
public:
	MarketDescr(const std::string& init_descname, const LuaTable& t, Descriptions& descriptions);
	~MarketDescr() override {
	}

	Building& create_object() const override;

	DescriptionIndex carrier() const {
		return carrier_;
	}

private:
	DescriptionIndex carrier_;
};

class Market : public Building {
	MO_DESCR(MarketDescr)
public:
	explicit Market(const MarketDescr& descr);
	~Market() override = default;

	void new_trade(int trade_id, const BillOfMaterials& items, int num_batches, Serial other_side);
	void cancel_trade(int trade_id);

	InputQueue& inputqueue(DescriptionIndex, WareWorker, const Request*) override;
	void cleanup(EditorGameBase&) override;

	void try_launching_batch(Game* game);
	void traded_ware_arrived(int trade_id, DescriptionIndex ware_index, Game* game);

private:
	struct TradeOrder {
		BillOfMaterials items;
		int initial_num_batches;
		int num_shipped_batches;
		Serial other_side;

		int received_traded_wares_in_this_batch;

		// The invariant here is that worker.size() + worker_request.get_count()
		// == 'num_wares_per_batch()'
		std::unique_ptr<Request> worker_request;
		std::vector<Worker*> workers;

		// The number of individual wares in 'items', i.e. the sum of all '.second's.
		int num_wares_per_batch() const;

		// True if the 'num_shipped_batches' equals the 'initial_num_batches'
		bool fulfilled() const;
	};

	static void
	worker_arrived_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);
	static void
	ware_arrived_callback(Game& g, InputQueue* q, DescriptionIndex ware, Worker* worker, void* data);

	void ensure_wares_queue_exists(int ware_index);
	bool is_ready_to_launch_batch(int trade_id) const;
	void launch_batch(int trade_id, Game* game);

	std::map<int, TradeOrder> trade_orders_;                  // Key is 'trade_id's.
	std::map<int, std::unique_ptr<WaresQueue>> wares_queue_;  // Key is 'ware_index'.

	DISALLOW_COPY_AND_ASSIGN(Market);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H
