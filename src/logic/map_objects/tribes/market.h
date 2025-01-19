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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H

#include <deque>
#include <map>
#include <memory>

#include "economy/request.h"
#include "economy/wares_queue.h"
#include "economy/workers_queue.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/trade_agreement.h"

namespace Widelands {

class MarketDescr : public BuildingDescr {
public:
	MarketDescr(const std::string& init_descname, const LuaTable& t, Descriptions& descriptions);
	~MarketDescr() override = default;

	[[nodiscard]] Building& create_object() const override;

	const DescriptionIndex local_carrier;
	const DescriptionIndex trade_carrier;
};

class Market : public Building {
	friend class MapBuildingdataPacket;
	MO_DESCR(MarketDescr)
public:
	struct TradeOrder {
		Market* market{nullptr};
		BillOfMaterials items;
		int initial_num_batches{0};
		int num_shipped_batches{0};
		OPtr<Market> other_side{nullptr};

		int received_traded_wares_in_this_batch{0};

		std::unique_ptr<WorkersQueue> carriers_queue_;
		std::map<DescriptionIndex, std::unique_ptr<WaresQueue>> wares_queues_;

		// The number of individual wares in 'items', i.e. the sum of all '.second's.
		[[nodiscard]] int num_wares_per_batch() const;

		// True if the 'num_shipped_batches' equals the 'initial_num_batches'
		[[nodiscard]] bool fulfilled() const;

		void cleanup();
	};

	explicit Market(const MarketDescr& descr);
	~Market() override = default;

	bool init(EditorGameBase&) override;
	void set_economy(Economy*, WareWorker) override;

	void update_statistics_string(std::string* str) override;

	bool fetch_from_flag(Game&) override;
	bool get_building_work(Game&, Worker&, bool success) override;
	void remove_worker(Worker&) override;

	void log_general_info(const EditorGameBase&) const override;

	[[nodiscard]] const std::string& get_market_name() const {
		return market_name_;
	}
	void set_market_name(const std::string& name);
	std::string market_census_string() const;

	[[nodiscard]] const std::deque<DescriptionIndex>& pending_dropout_wares() const {
		return pending_dropout_wares_;
	}

	void new_trade(TradeID trade_id,
	               const BillOfMaterials& items,
	               int num_batches,
	               OPtr<Market> other_side);
	void cancel_trade(Game& game, TradeID trade_id, bool reached_regular_end, bool send_msg);

	[[nodiscard]] InputQueue& inputqueue(DescriptionIndex, WareWorker, const Request*) override;
	void cleanup(EditorGameBase&) override;

	void try_launching_batch(Game* game);
	void traded_ware_arrived(TradeID trade_id, DescriptionIndex ware_index, Game* game);

	[[nodiscard]] const std::map<TradeID, TradeOrder>& trade_orders() const {
		return trade_orders_;
	}

	static void carrier_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);
	static void
	ware_arrived_callback(Game& g, InputQueue* q, DescriptionIndex ware, Worker* worker, void* data);

private:
	[[nodiscard]] bool is_ready_to_launch_batch(TradeID trade_id) const;
	void launch_batch(TradeID trade_id, Game* game);

	std::string market_name_;
	std::map<TradeID, TradeOrder> trade_orders_;
	std::deque<DescriptionIndex> pending_dropout_wares_;

	OPtr<Worker> carrier_;
	std::unique_ptr<Request> carrier_request_;
	uint32_t fetchfromflag_{0U};

	DISALLOW_COPY_AND_ASSIGN(Market);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H
