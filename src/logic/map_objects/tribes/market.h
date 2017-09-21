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

#include "logic/map_objects/tribes/building.h"
#include "economy/request.h"

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

private:
	struct TradeOrder {
		BillOfMaterials items;
		int initial_num_batches;
		int shipped_batches;
		Serial other_side;

		std::unique_ptr<Request> worker_request;
	};

	static void
	request_worker_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

	std::map<int, TradeOrder> trade_orders_;

	DISALLOW_COPY_AND_ASSIGN(Market);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_MARKET_H
