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

int32_t ScenarioInfrastructureSettingsTool::handle_click_impl(const Widelands::World&,
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
		for (const ScenarioFlagSettingsWindow* w : open_windows_) {
			if (w->flag() == &imm) {
				return 0;
			}
		}
		open_windows_.insert(new ScenarioFlagSettingsWindow(parent, *this, dynamic_cast<Widelands::Flag&>(imm)));
	} else if (dynamic_cast<Widelands::Building*>(&imm)) {
		// This function uses UniqueWindow to ensure that the window doesn't open twice
		parent.show_building_window(center.node, true, false, true);
	}
	return 0;
}

ScenarioInfrastructureSettingsTool::~ScenarioInfrastructureSettingsTool() {
	for (ScenarioFlagSettingsWindow* w : open_windows_) {
		// Make sure the window won't try to notify us after we are destroyed
		w->unset_tool();
	}
}

void ScenarioInfrastructureSettingsTool::window_closing(ScenarioFlagSettingsWindow* w) {
	auto it = open_windows_.find(w);
	assert(it != open_windows_.end());
	open_windows_.erase(it);
}

