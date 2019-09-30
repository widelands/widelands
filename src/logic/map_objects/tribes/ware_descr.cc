/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/map_objects/tribes/ware_descr.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"

namespace Widelands {

/**
 * The contents of 'table' are documented in
 * /data/tribes/wares/armor/init.lua
 */
WareDescr::WareDescr(const std::string& init_descname, const LuaTable& table)
   : MapObjectDescr(MapObjectType::WARE, table.get_string("name"), init_descname, table),
     ai_hints_(new WareHints(table.get_string("name"), *table.get_table("preciousness"))) {
	if (helptext_script().empty()) {
		throw GameDataError("Ware %s has no helptext script", name().c_str());
	}
	if (!is_animation_known("idle")) {
		throw GameDataError("Ware %s has no idle animation", name().c_str());
	}
	if (icon_filename().empty()) {
		throw GameDataError("Ware %s has no menu icon", name().c_str());
	}
	i18n::Textdomain td("tribes");

	std::unique_ptr<LuaTable> items_table = table.get_table("default_target_quantity");
	for (const std::string& key : items_table->keys<std::string>()) {
		default_target_quantities_.emplace(key, items_table->get_int(key));
	}
}

DescriptionIndex WareDescr::default_target_quantity(const std::string& tribename) const {
	if (default_target_quantities_.count(tribename) > 0) {
		return default_target_quantities_.at(tribename);
	}
	return kInvalidWare;
}

bool WareDescr::has_demand_check(const std::string& tribename) const {
	return default_target_quantity(tribename) != kInvalidWare;
}

void WareDescr::set_has_demand_check(const std::string& tribename) {
	if (default_target_quantities_.count(tribename) > 0 &&
	    default_target_quantities_.at(tribename) == kInvalidWare) {
		default_target_quantities_.at(tribename) = 1;
	}
}

void WareDescr::add_consumer(const DescriptionIndex& building_index) {
	consumers_.insert(building_index);
}

void WareDescr::add_producer(const DescriptionIndex& building_index) {
	producers_.insert(building_index);
}

const std::set<DescriptionIndex>& WareDescr::consumers() const {
	return consumers_;
}

const std::set<DescriptionIndex>& WareDescr::producers() const {
	return producers_;
}
}  // namespace Widelands
