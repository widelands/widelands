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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H

#include <memory>

#include "economy/request.h"
#include "economy/wares_queue.h"
#include "logic/map_objects/tribes/building.h"

namespace Widelands {

class MarketDescr : public BuildingDescr {
public:
	MarketDescr(const std::string& init_descname, const LuaTable& t, const EditorGameBase& egbase);
	~MarketDescr() override {
	}

	Building& create_object() const override;

	DescriptionIndex carrier() const { return carrier_; }

private:
	DescriptionIndex carrier_;
};

class Market : public Building {
	MO_DESCR(MarketDescr)
public:
	explicit Market(const MarketDescr& descr);
	~Market() override;

	void new_trade(int trade_id, const BillOfMaterials& items, int num_batches, Serial other_side);

	InputQueue& inputqueue(DescriptionIndex, WareWorker) override;
	void cleanup(EditorGameBase&) override;

	void try_launching_batch(Game* game);
	void receive_ware(Game&, DescriptionIndex ware) override;

private:
	struct WareRequest {
		int index;
		std::unique_ptr<Request> request;
	};

	struct TradeOrder {
		BillOfMaterials items;
		int initial_num_batches;
		int shipped_batches;
		Serial other_side;

		// The invariant here is that worker.size() + worker_request.get_count()
		// equals the number of individual wares in 'items'.
		std::unique_ptr<Request> worker_request;
		std::vector<Worker*> workers;

		std::vector<WareRequest> ware_requests;
	};

	static void
	worker_arrived_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);
	static void
	ware_arrived_callback(Game& g, InputQueue* q, DescriptionIndex ware, Worker* worker, void* data);

	void ensure_wares_queue_exists(int ware_index);
	bool is_ready_to_launch_batch(int trade_id);
	void launch_batch(int trade_id, Game* game);

	std::map<int, TradeOrder> trade_orders_;  // Key is 'trade_id's.
	std::map<int, std::unique_ptr<WaresQueue>> wares_queue_; // Key is 'ware_index'.

	DISALLOW_COPY_AND_ASSIGN(Market);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H
