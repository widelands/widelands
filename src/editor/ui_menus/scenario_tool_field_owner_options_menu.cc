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

#include "editor/ui_menus/scenario_tool_field_owner_options_menu.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"

inline EditorInteractive& ScenarioToolFieldOwnerOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

ScenarioToolFieldOwnerOptionsMenu::ScenarioToolFieldOwnerOptionsMenu(
   EditorInteractive& parent, ScenarioFieldOwnerTool& tool, UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 300, 24, _("Set Field Owner"), tool),
     tool_(tool),
     list_(this,
           "player",
           0,
           0,
           get_inner_w(),
           8,
           get_inner_h(),
           "",
           UI::DropdownType::kTextual,
           UI::PanelStyle::kWui,
           UI::ButtonStyle::kWuiSecondary) {

	list_.selected.connect([this]() { select(); });
	const Widelands::Map& map = parent.egbase().map();
	const Widelands::PlayerNumber max = map.get_nrplayers();
	const Widelands::PlayerNumber sel = tool_.get_new_owner();
	list_.add(_("Unset Owner"), 0, nullptr, sel == 0 || sel > max, _("Mark fields as unowned"));
	for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
		const std::string& name = map.get_scenario_player_name(p);
		list_.add(
		   (boost::format(_("Player %1$s (%2$s)")) % std::to_string(static_cast<int>(p)) % name)
		      .str(),
		   p,
		   g_gr->images().get(
		      Widelands::get_tribeinfo(eia().egbase().map().get_scenario_player_tribe(p)).icon),
		   sel == p, (boost::format(_("Claim fields for %s")) % name).str());
	}

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void ScenarioToolFieldOwnerOptionsMenu::select() {
	const Widelands::PlayerNumber p = list_.get_selected();
	assert(p <= eia().egbase().map().get_nrplayers());
	tool_.set_new_owner(p);
	select_correct_tool();
}
