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

#ifndef WL_EDITOR_UI_MENUS_PLAYER_TEAMS_MENU_H
#define WL_EDITOR_UI_MENUS_PLAYER_TEAMS_MENU_H

#include <cstdint>
#include <memory>
#include <vector>

#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

class EditorPlayerTeamsMenu : public EditorToolOptionsMenu {
public:
	EditorPlayerTeamsMenu(EditorInteractive&, EditorTool&, UI::UniqueWindow::Registry&);

	class PlayerRelationsPanel : public UI::Panel {
	public:
		PlayerRelationsPanel(UI::Panel* parent, EditorInteractive&, uint8_t);
		~PlayerRelationsPanel() override {
		}

		void draw(RenderTarget& r) override;
		bool handle_mousemove(uint8_t, int32_t x, int32_t y, int32_t, int32_t) override;
		bool handle_mousepress(uint8_t, int32_t x, int32_t y) override;

	private:
		EditorInteractive& eia_;
		uint8_t nr_players_;

		std::vector<std::unique_ptr<UI::Dropdown<uint8_t>>> teams_;
		std::vector<std::unique_ptr<UI::Button>> buttons_;

		int8_t player_at(int32_t x) const;
	};

private:
	PlayerRelationsPanel panel_;
};

// A long list of checkboxes which buildings a player may use originally.
// TODO(Nordfriese): A possibility to define custom buildings should go here
class EditorPlayerAllowedBuildingsWindow : public UI::UniqueWindow {
public:
	EditorPlayerAllowedBuildingsWindow(EditorInteractive*,
	                                   Widelands::PlayerNumber,
	                                   UI::UniqueWindow::Registry&);
	~EditorPlayerAllowedBuildingsWindow() override {
	}

private:
	UI::Box box_;
	std::vector<UI::Checkbox*> checkboxes_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_PLAYER_TEAMS_MENU_H
