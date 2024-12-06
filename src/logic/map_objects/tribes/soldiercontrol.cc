/*
 * Copyright (C) 2023-2024 by the Widelands Development Team
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
	if (present == stationed || present >= soldier_capacity()) {
		if (soldier_capacity() > stationed) {  // Soldiers are lacking
			rv = format(
			   tribe.get_soldiers_format_string(TribeDescr::CapacityStringIndex::kLacking, stationed),
			   stationed,
			   StyleManager::color_tag(as_string(soldier_capacity() - stationed), style.low_color()));
		} else {  // Soldiers filled to capacity
			rv = format(tribe.get_soldiers_format_string(
			               TribeDescr::CapacityStringIndex::kFull, soldier_capacity()),
			            soldier_capacity());
		}
	} else {
		if (soldier_capacity() > stationed) {  // Soldiers are lacking; others are outside
			rv = format(
			   tribe.get_soldiers_format_string(
			      TribeDescr::CapacityStringIndex::kOutAndLacking, stationed),
			   present, StyleManager::color_tag(as_string(stationed - present), style.high_color()),
			   StyleManager::color_tag(as_string(soldier_capacity() - stationed), style.low_color()));
		} else {  // Soldiers filled to capacity; some are outside
			rv = format(
			   tribe.get_soldiers_format_string(
			      TribeDescr::CapacityStringIndex::kOut, soldier_capacity()),
			   present,
			   StyleManager::color_tag(as_string(soldier_capacity() - present), style.high_color()));
		}
	}

	return StyleManager::color_tag(
	   format("%s %s", soldier_preference_icon(pref), rv), style.medium_color());
}

}  // namespace Widelands
