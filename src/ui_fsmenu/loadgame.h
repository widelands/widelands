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

#ifndef WL_UI_FSMENU_LOADGAME_H
#define WL_UI_FSMENU_LOADGAME_H

#include "logic/game.h"
#include "logic/game_settings.h"
#include "ui_basic/checkbox.h"
#include "ui_fsmenu/menu.h"
#include "wui/load_or_save_game.h"

namespace FsMenu {

/// Select a Saved Game in Fullscreen Mode. It's a modal fullscreen menu.
class LoadGame : public TwoColumnsFullNavigationMenu {
public:
	LoadGame(
	   MenuCapsule&,
	   Widelands::Game&,
	   GameSettingsProvider& gsp,
	   bool take_ownership_of_game_and_settings,
	   bool is_replay,
	   const std::function<void(const std::string&)>& = [](const std::string&) {});
	~LoadGame() override;

	bool handle_key(bool down, SDL_Keysym code) override;
	void think() override;

protected:
	/// Sets the current selected filename and ends the modal screen with 'Ok' status.
	void clicked_ok() override;
	void layout() override;

	/// Update button status and game details
	void entry_selected();

	/// Fill load_or_save_'s table
	void fill_table();

private:
	Widelands::Game& game_;
	GameSettingsProvider& settings_;
	bool take_ownership_of_game_and_settings_;

	std::function<void(const std::string&)> callback_on_ok_;

	void toggle_filenames();

	LoadOrSaveGame load_or_save_;

	bool is_replay_;
	bool update_game_details_;

	UI::Checkbox* show_filenames_;
	bool showing_filenames_;
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_LOADGAME_H
