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

#include "editor/ui_menus/scenario_tool_vision_options_menu.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"

inline EditorInteractive& ScenarioToolVisionOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

ScenarioToolVisionOptionsMenu::ScenarioToolVisionOptionsMenu(EditorInteractive& parent,
                                                             ScenarioVisionTool& tool,
                                                             UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 300, 30, _("Field Vision"), tool),
     tool_(tool),
     box_(this, 0, 0, UI::Box::Vertical),
     players_(&box_,
              "player",
              0,
              0,
              get_inner_w(),
              8,
              get_inner_h(),
              "",
              UI::DropdownType::kTextual,
              UI::PanelStyle::kWui,
              UI::ButtonStyle::kWuiSecondary),
     modes_(&box_,
            "mode",
            0,
            0,
            get_inner_w(),
            8,
            get_inner_h(),
            "",
            UI::DropdownType::kTextual,
            UI::PanelStyle::kWui,
            UI::ButtonStyle::kWuiSecondary) {

	players_.selected.connect([this]() { select_player(); });
	modes_.selected.connect([this]() { select_mode(); });
	const Widelands::Map& map = parent.egbase().map();
	const Widelands::PlayerNumber max = map.get_nrplayers();
	const Widelands::PlayerNumber sel = tool_.get_player();
	for (Widelands::PlayerNumber p = 1; p <= max; ++p) {
		const std::string& name = map.get_scenario_player_name(p);
		const std::string& tribe = map.get_scenario_player_tribe(p);
		players_.add(
		   (boost::format(_("Player %1$s (%2$s)")) % std::to_string(static_cast<int>(p)) % name)
		      .str(),
		   p,
		   g_gr->images().get(
		      Widelands::get_tribeinfo(eia().egbase().map().get_scenario_player_tribe(p)).icon),
		   sel == p);
	}
	modes_.add(_("Reveal"), Widelands::SeeUnseeNode::kVisible, nullptr,
	           tool_.get_mode() == Widelands::SeeUnseeNode::kVisible,
	           _("Make fields visible for this player"));
	modes_.add(_("Hide"), Widelands::SeeUnseeNode::kUnexplored, nullptr,
	           tool_.get_mode() == Widelands::SeeUnseeNode::kUnexplored,
	           _("Mark fields as never seen by the player"));
	modes_.add(_("Previously seen"), Widelands::SeeUnseeNode::kPreviouslySeen, nullptr,
	           tool_.get_mode() == Widelands::SeeUnseeNode::kPreviouslySeen,
	           _("Mark fields as previously revealed but not currently seen"));

	box_.add(&players_, UI::Box::Resizing::kFullSize);
	box_.add(&modes_, UI::Box::Resizing::kFullSize);
	set_center_panel(&box_);

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

ScenarioToolVisionOptionsMenu::~ScenarioToolVisionOptionsMenu() {
	eia().select_tool(eia().tools()->info, EditorTool::First);
}

void ScenarioToolVisionOptionsMenu::select_player() {
	const Widelands::PlayerNumber p = players_.get_selected();
	assert(p);
	assert(p <= eia().egbase().map().get_nrplayers());
	tool_.set_player(eia(), p);
	select_correct_tool();
}

void ScenarioToolVisionOptionsMenu::select_mode() {
	tool_.set_mode(modes_.get_selected());
	select_correct_tool();
}
