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
std::string SoldierControl::get_status_string(const TribeDescr& tribe, SoldierPreference pref) {
	std::unique_ptr<i18n::GenericTextdomain> td(AddOns::create_textdomain_for_addon(
	   tribe.basic_info().addon, "tribes_encyclopedia"));
	std::string rv;
	Quantity present = present_soldiers().size();
	Quantity stationed = stationed_soldiers().size();
	const UI::BuildingStatisticsStyleInfo& style = g_style_manager->building_statistics_style();

	// military capacity strings
	if (present == stationed) {
		if (soldier_capacity() > stationed) {  // Soldiers are lacking
			rv = format(
			   npgettext(tribe.get_soldier_context_string().c_str(),
			             tribe.get_soldier_capacity_strings_sg()[0].c_str(),
			             tribe.get_soldier_capacity_strings_pl()[0].c_str(), stationed),
			   stationed,
			   StyleManager::color_tag(as_string(soldier_capacity() - stationed), style.low_color()));
		} else {  // Soldiers filled to capacity
			rv = format(
			   npgettext(tribe.get_soldier_context_string().c_str(),
			             tribe.get_soldier_capacity_strings_sg()[1].c_str(),
			             tribe.get_soldier_capacity_strings_pl()[1].c_str(), stationed),
			   stationed);
		}
	} else {
		if (soldier_capacity() > stationed) {  // Soldiers are lacking; others are outside
			rv = format(
			   npgettext(tribe.get_soldier_context_string().c_str(),
			             tribe.get_soldier_capacity_strings_sg()[2].c_str(),
			             tribe.get_soldier_capacity_strings_pl()[2].c_str(), stationed),
			   present, StyleManager::color_tag(as_string(stationed - present), style.high_color()),
			   StyleManager::color_tag(as_string(soldier_capacity() - stationed), style.low_color()));
		} else {  // Soldiers filled to capacity; some are outside
			rv = format(
			   npgettext(tribe.get_soldier_context_string().c_str(),
			             tribe.get_soldier_capacity_strings_sg()[3].c_str(),
			             tribe.get_soldier_capacity_strings_pl()[3].c_str(), stationed),
			   present, StyleManager::color_tag(as_string(stationed - present), style.high_color()));
		}
	}

	return StyleManager::color_tag(
	   format("%s %s", soldier_preference_icon(pref), rv), style.medium_color());
}

}  // namespace Widelands
