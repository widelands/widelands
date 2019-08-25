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

#include "editor/tools/scenario_building_settings_tool.h"

#include <cstdio>
#include <vector>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"

int32_t ScenarioBuildingSettingsTool::handle_click_impl(const Widelands::World&,
                                          const Widelands::NodeAndTriangle<>& center,
                                          EditorInteractive& parent,
                                          EditorActionArgs*,
                                          Widelands::Map* map) {
	parent.stop_painting();
	Widelands::Field& f = (*map)[center.node];
	if (!f.get_immovable()) {
		return 0;
	}
	Widelands::BaseImmovable& imm = *f.get_immovable();
	if (imm.descr().type() == Widelands::MapObjectType::FLAG) {
		for (const ScenarioFlagSettingsWindow* w : open_windows_flag_) {
			if (w->flag() == &imm) {
				return 0;
			}
		}
		open_windows_flag_.insert(new ScenarioFlagSettingsWindow(parent, *this, dynamic_cast<Widelands::Flag&>(imm)));
	} /* else if (Widelands::Building* bld = dynamic_cast<Widelands::Building>(&imm)) {
		for (const ScenarioBuildingSettingsWindow* w : open_windows_building_) {
			if (w->building() == bld) {
				return 0;
			}
		}
		open_windows_building_.insert(new ScenarioBuildingSettingsWindow(parent, *bld));
	} */
	return 0;
}

/* void ScenarioBuildingSettingsTool::window_closing(const ScenarioBuildingSettingsWindow* w) {
	auto it = open_windows_building_.find(w);
	assert(it != open_windows_building_.end());
	open_windows_building_.erase(it);
} */

void ScenarioBuildingSettingsTool::window_closing(const ScenarioFlagSettingsWindow* w) {
	auto it = open_windows_flag_.find(w);
	assert(it != open_windows_flag_.end());
	open_windows_flag_.erase(it);
}

