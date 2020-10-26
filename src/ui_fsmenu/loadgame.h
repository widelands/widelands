/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/panel.h"
#include "ui_fsmenu/load_map_or_game.h"
#include "wui/load_or_save_game.h"

/// Select a Saved Game in Fullscreen Mode. It's a modal fullscreen menu.
class FullscreenMenuLoadGame : public FullscreenMenuLoadMapOrGame {
public:
	FullscreenMenuLoadGame(FullscreenMenuMain&,
	                       Widelands::Game&,
	                       GameSettingsProvider* gsp,
	                       bool is_replay = false);

	/// The currently selected filename
	const std::string& filename() const;

	bool handle_key(bool down, SDL_Keysym code) override;
	void think() override;

protected:
	/// Sets the current selected filename and ends the modal screen with 'Ok' status.
	void clicked_ok() override;

	/// Update button status and game details
	void entry_selected() override;

	/// Fill load_or_save_'s table
	void fill_table() override;

private:
	void layout() override;
	void toggle_filenames();

	UI::Box main_box_;
	UI::Box info_box_;

	LoadOrSaveGame load_or_save_;

	// TODO(GunChleoc): Get rid of this hack once everything is 100% box layout
	UI::Panel* button_spacer_;
	std::string filename_;

	bool is_replay_;
	bool update_game_details_;

	UI::Checkbox* show_filenames_;
	bool showing_filenames_;
};

#endif  // end of include guard: WL_UI_FSMENU_LOADGAME_H
