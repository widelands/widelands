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

#ifndef WL_WUI_GAME_MAIN_MENU_SAVE_GAME_H
#define WL_WUI_GAME_MAIN_MENU_SAVE_GAME_H

#include "base/i18n.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/load_or_save_game.h"

class InteractiveGameBase;

/// Displays a warning if the filename to be saved to already esists
struct SaveWarnMessageBox;

/// A window that lets the user save the current game and delete savegames.
struct GameMainMenuSaveGame : public UI::UniqueWindow {
	enum class Type { kSave, kLoad };

	friend struct SaveWarnMessageBox;
	GameMainMenuSaveGame(InteractiveGameBase&, UI::UniqueWindow::Registry& registry, Type);

	void fill_list();
	void select_by_name(const std::string& name);

protected:
	void die() override;
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	void layout() override;
	InteractiveGameBase& igbase();

	/// Update button status and game details and prefill the edibox.
	void entry_selected();

	/// Update buttons and table selection state
	void edit_box_changed();
	/// Resets the savegame's name in the editbox. If savegame name didn't change, die().
	void reset_editbox_or_die(const std::string& current_filename);

	/// Called when the OK button is clicked or the Return key pressed in the edit box.
	void ok();

	bool save_game(std::string filename, bool binary);

	/// Pause/unpause the game
	void pause_game(bool paused);

	// UI coordinates and spacers
	int32_t const padding_;  // Common padding between panels

	const Type type_;

	UI::Box main_box_;
	UI::Box info_box_;

	LoadOrSaveGame load_or_save_;

	UI::Box filename_box_;
	UI::Textarea filename_label_;
	UI::EditBox filename_editbox_;

	UI::Box buttons_box_;
	UI::Button cancel_, ok_;

	std::string curdir_;
	const std::string illegal_filename_tooltip_;
};

#endif  // end of include guard: WL_WUI_GAME_MAIN_MENU_SAVE_GAME_H
