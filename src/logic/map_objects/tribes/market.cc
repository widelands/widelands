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

namespace Widelands {

// NOCOM(#sirver): forbid building of this in regular games for now.

MarketDescr::MarketDescr(const std::string& init_descname,
                         const LuaTable& table,
                         const EditorGameBase& egbase)
   : BuildingDescr(init_descname, MapObjectType::MARKET, table, egbase) {
	i18n::Textdomain td("tribes");

	parse_working_positions(egbase, table.get_table("working_positions").get(), &working_positions_);
	// NOCOM(#sirver): fill in
}

Building& MarketDescr::create_object() const {
	return *new Market(*this);
}

Market::Market(const MarketDescr& descr) : Building(descr) {}

Market::~Market() {}

}  // namespace Widelands
