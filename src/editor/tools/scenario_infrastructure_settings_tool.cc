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

#include "editor/tools/scenario_infrastructure_settings_tool.h"

#include <cstdio>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "wui/soldierlist.h"

int32_t
ScenarioInfrastructureSettingsTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                      EditorInteractive& parent,
                                                      EditorActionArgs*,
                                                      Widelands::Map* map) {
	parent.stop_painting();
	Widelands::Field& f = (*map)[center.node];
	Widelands::BaseImmovable* imm = f.get_immovable();
	if (is_a(Widelands::Building, imm)) {
		parent.show_building_window(center.node, true, false);
	} else {
		for (Widelands::Bob* bob = f.get_first_bob(); bob; bob = bob->get_next_bob()) {
			if (upcast(Widelands::Ship, ship, bob)) {
				parent.show_ship_window(ship);
			} else if (upcast(Widelands::Soldier, soldier, bob)) {
				SoldierSettings s(parent, *soldier, false);
				// SoldierSettings is implemented as a modal dialog
				s.run<UI::Panel::Returncodes>();
			}
		}
		if (imm && imm->descr().type() == Widelands::MapObjectType::FLAG) {
			for (const auto& w : open_flag_windows_) {
				if (w->flag() == imm) {
					return 0;
				}
			}
			open_flag_windows_.insert(
			   new ScenarioFlagSettingsWindow(parent, *this, dynamic_cast<Widelands::Flag&>(*imm)));
		}
	}
	return 0;
}

ScenarioInfrastructureSettingsTool::~ScenarioInfrastructureSettingsTool() {
	// Make sure the windows won't try to notify us after we are destroyed
	for (auto& w : open_flag_windows_) {
		w->unset_tool();
	}
}

void ScenarioInfrastructureSettingsTool::window_closing(ScenarioFlagSettingsWindow* w) {
	auto it = open_flag_windows_.find(w);
	assert(it != open_flag_windows_.end());
	open_flag_windows_.erase(it);
}
