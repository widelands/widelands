/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#include "logic/map_objects/tribes/soldiercontrol.h"

#include <memory>

#include "base/i18n.h"
#include "base/string.h"
#include "graphic/style_manager.h"
#include "logic/map_objects/tribes/tribe_descr.h"

namespace {

std::string format_extra_soldiers(Widelands::Quantity has, Widelands::Quantity wants) {
	assert(has > wants);
	/** TRANSLATORS: %1% is the number of all soldiers in a warehouse, %2% is the desired
	                 number */
	return format(pgettext("warehouse", "%1% (%2%)"),
	              StyleManager::color_tag(
	                 as_string(has), g_style_manager->building_statistics_style().high_color()),
	              wants);
}

}  // namespace

namespace Widelands {

/**
===============
Status string with number of soldiers.
===============
*/
std::string SoldierControl::get_status_string(const TribeDescr& tribe,
                                              SoldierPreference pref) const {
	std::string rv;
	Quantity present = present_soldiers().size();
	Quantity stationed = stationed_soldiers().size();
	const UI::BuildingStatisticsStyleInfo& style = g_style_manager->building_statistics_style();

	// military capacity strings
	if (present == stationed) {
		if (soldier_capacity() > stationed) {  // Soldiers are lacking
			rv = format(
			   tribe.get_soldiers_format_string(TribeDescr::CapacityStringIndex::kLacking, stationed),
			   stationed,
			   StyleManager::color_tag(as_string(soldier_capacity() - stationed), style.low_color()));
		} else if (stationed > soldier_capacity()) {  // Port or HQ has extra soldiers in store
			rv = format(
			   tribe.get_soldiers_format_string(TribeDescr::CapacityStringIndex::kFull, stationed),
			   format_extra_soldiers(stationed, soldier_capacity()));
		} else {  // Soldiers filled to capacity
			rv = format(
			   tribe.get_soldiers_format_string(TribeDescr::CapacityStringIndex::kFull, stationed),
			   stationed);
		}
	} else {
		if (soldier_capacity() > stationed) {  // Soldiers are lacking; others are outside
			rv = format(
			   tribe.get_soldiers_format_string(
			      TribeDescr::CapacityStringIndex::kOutAndLacking, stationed),
			   present, StyleManager::color_tag(as_string(stationed - present), style.high_color()),
			   StyleManager::color_tag(as_string(soldier_capacity() - stationed), style.low_color()));
		} else if (present > soldier_capacity()) {  // Port or HQ has extra soldiers in store;
			                                         // some are outside
			                                         // (this is currently not possible, outside
			                                         //  soldiers are not tracked for warehouses)
			rv = format(
			   tribe.get_soldiers_format_string(TribeDescr::CapacityStringIndex::kOut, stationed),
			   format_extra_soldiers(present, soldier_capacity()),
			   StyleManager::color_tag(as_string(stationed - present), style.high_color()));
		} else {  // Soldiers filled to capacity; some are outside
			rv = format(
			   tribe.get_soldiers_format_string(TribeDescr::CapacityStringIndex::kOut, stationed),
			   present, StyleManager::color_tag(as_string(stationed - present), style.high_color()));
		}
	}

	return StyleManager::color_tag(
	   format("%s %s", soldier_preference_icon(pref), rv), style.medium_color());
}

}  // namespace Widelands
