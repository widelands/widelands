/*
 * Copyright (C) 2002-2004, 2006-2008, 2011-2012 by the Widelands Development Team
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

#include "logic/ware_descr.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "logic/tribes/tribe_descr.h"

namespace Widelands {

WareDescr::WareDescr(const std::string& init_descname, const LuaTable& table) :
	MapObjectDescr(MapObjectType::WARE, table.get_string("name"), init_descname, table),
	icon_fname_(table.get_string("icon")),
	icon_(g_gr->images().get("pics/but0.png")) {

	assert(is_animation_known("idle"));
	i18n::Textdomain td("tribes");

	directory_ = table.get_string("directory");

	std::unique_ptr<LuaTable> items_table = table.get_table("default_target_quantity");
	for (const std::string& key : items_table->keys<std::string>()) {
		default_target_quantities_.emplace(key, items_table->get_int(key));
	}

	items_table = table.get_table("preciousness");
	for (const std::string& key : items_table->keys<std::string>()) {
		preciousnesses_.emplace(key, items_table->get_int(key));
	}
}

int WareDescr::preciousness(const std::string& tribename) const {
	if (preciousnesses_.count(tribename) > 0) {
		return preciousnesses_.at(tribename);
	}
	return kInvalidWare;
}


WareIndex WareDescr::default_target_quantity(const std::string& tribename) const {
	if (default_target_quantities_.count(tribename) > 0) {
		return default_target_quantities_.at(tribename);
	}
	return kInvalidWare;
}


/**
 * Load all static graphics
 */
void WareDescr::load_graphics()
{
	// NOCOM shift to MapObject
	icon_ = g_gr->images().get(icon_fname_);
}

bool WareDescr::has_demand_check(const std::string& tribename) const {
	return default_target_quantity(tribename) != kInvalidWare;
}

void WareDescr::set_has_demand_check(const std::string& tribename) {
	if (default_target_quantities_.count(tribename) > 0
		 && default_target_quantities_.at(tribename) == kInvalidWare) {
		default_target_quantities_.at(tribename) = 1;
	}
}

void WareDescr::add_consumer(const BuildingIndex& building_index) {
	consumers_.emplace(building_index);
}

void WareDescr::add_producer(const BuildingIndex& building_index) {
	producers_.emplace(building_index);
}

const std::set<BuildingIndex>& WareDescr::consumers() const {
	return consumers_;
}

const std::set<BuildingIndex>& WareDescr::producers() const {
	return producers_;
}


}
