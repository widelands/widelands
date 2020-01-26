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

#ifndef WL_EDITOR_UI_MENUS_SCENARIO_INFRASTRUCTURE_SETTINGS_WINDOW_H
#define WL_EDITOR_UI_MENUS_SCENARIO_INFRASTRUCTURE_SETTINGS_WINDOW_H

#include <memory>
#include <vector>

#include "economy/flag.h"
#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/window.h"

class EditorInteractive;
struct ScenarioInfrastructureSettingsTool;

/// Allows placing wares on a flag and changing economy targets
struct ScenarioFlagSettingsWindow : public UI::Window {
	ScenarioFlagSettingsWindow(EditorInteractive&,
	                           ScenarioInfrastructureSettingsTool&,
	                           Widelands::Flag&);
	~ScenarioFlagSettingsWindow() override {
	}

	const Widelands::Flag* flag() const;

	void unset_tool();

protected:
	void die() override;
	void think() override;

private:
	EditorInteractive& eia();

	UI::Box main_box_, wares_box_;
	std::unique_ptr<std::unique_ptr<UI::Dropdown<Widelands::DescriptionIndex>>[]> dropdowns_;
	UI::Button economy_options_;

	std::unique_ptr<Widelands::DescriptionIndex[]> wares_on_flag_;

	void update();
	void select(uint32_t slot);
	void economy_options_clicked();

	ScenarioInfrastructureSettingsTool* tool_;
	Widelands::OPtr<Widelands::Flag> flag_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_SCENARIO_INFRASTRUCTURE_SETTINGS_WINDOW_H
