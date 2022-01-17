/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_SINGLEPLAYERDROPDOWN_H
#define WL_UI_FSMENU_SINGLEPLAYERDROPDOWN_H

#include <memory>
#include <string>

#include "logic/game_settings.h"
#include "ui_basic/dropdown.h"

namespace FsMenu {

class LaunchGame;

template <typename T> class SinglePlayerDropdown {
public:
	SinglePlayerDropdown(UI::Panel* parent,
	                     LaunchGame& lg,
	                     const std::string& name,
	                     int32_t x,
	                     int32_t y,
	                     uint32_t w,
	                     uint32_t max_list_items,
	                     int button_dimension,
	                     const std::string& label,
	                     const UI::DropdownType type,
	                     UI::PanelStyle style,
	                     UI::ButtonStyle button_style,
	                     GameSettingsProvider* const settings,
	                     PlayerSlot id)
	   : dropdown_(parent,
	               name,
	               x,
	               y,
	               w,
	               max_list_items,
	               button_dimension,
	               label,
	               type,
	               style,
	               button_style),
	     launch_game_(lg),
	     settings_(settings),
	     id_(id),
	     selection_locked_(false) {
		dropdown_.set_disable_style(UI::ButtonDisableStyle::kFlat);
		dropdown_.selected.connect([this]() { on_selection_changed(); });
	}
	virtual ~SinglePlayerDropdown() {
	}

	UI::Panel* get_dropdown() {
		return &dropdown_;
	}
	virtual void selection_action() {
	}

	virtual void rebuild() = 0;

	void set_visible(bool visible) {
		dropdown_.set_visible(visible);
	}
	void set_enabled(bool enable) {
		dropdown_.set_enabled(enable);
	}
	void set_desired_size(int w, int h) {
		dropdown_.set_desired_size(w, h);
	}

protected:
	UI::Dropdown<T> dropdown_;
	LaunchGame& launch_game_;
	GameSettingsProvider* const settings_;
	PlayerSlot const id_;
	bool selection_locked_;

private:
	void on_selection_changed() {
		selection_locked_ = true;
		selection_action();
		selection_locked_ = false;
	}
};

class SinglePlayerTribeDropdown : public SinglePlayerDropdown<std::string> {
public:
	SinglePlayerTribeDropdown(UI::Panel* parent,
	                          LaunchGame& lg,
	                          const std::string& name,
	                          int32_t x,
	                          int32_t y,
	                          uint32_t w,
	                          int button_dimension,
	                          GameSettingsProvider* const settings,
	                          PlayerSlot id);
	void rebuild() override;

private:
	void selection_action() override;
};

class SinglePlayerPlayerTypeDropdown : public SinglePlayerDropdown<std::string> {
public:
	SinglePlayerPlayerTypeDropdown(UI::Panel* parent,
	                               LaunchGame& lg,
	                               const std::string& name,
	                               int32_t x,
	                               int32_t y,
	                               uint32_t w,
	                               int button_dimension,
	                               GameSettingsProvider* const settings,
	                               PlayerSlot id);
	void rebuild() override;

private:
	void fill();
	void select_entry();
	void selection_action() override;
};

class SinglePlayerStartTypeDropdown : public SinglePlayerDropdown<uintptr_t> {
public:
	SinglePlayerStartTypeDropdown(UI::Panel* parent,
	                              LaunchGame& lg,
	                              const std::string& name,
	                              int32_t x,
	                              int32_t y,
	                              uint32_t w,
	                              int button_dimension,
	                              GameSettingsProvider* const settings,
	                              PlayerSlot id);

	void rebuild() override;

private:
	void fill();
	void selection_action() override;
};

class SinglePlayerTeamDropdown : public SinglePlayerDropdown<uintptr_t> {
public:
	SinglePlayerTeamDropdown(UI::Panel* parent,
	                         LaunchGame& lg,
	                         const std::string& name,
	                         int32_t x,
	                         int32_t y,
	                         uint32_t w,
	                         int button_dimension,
	                         GameSettingsProvider* const settings,
	                         PlayerSlot id);

	void rebuild() override;

private:
	void selection_action() override;
};
}  // namespace FsMenu

#endif  // WL_UI_FSMENU_SINGLEPLAYERDROPDOWN_H
