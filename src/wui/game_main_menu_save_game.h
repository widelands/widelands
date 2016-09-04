/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/load_or_save_game.h"

class InteractiveGameBase;

struct SaveWarnMessageBox;
struct GameMainMenuSaveGame : public UI::UniqueWindow {
	friend struct SaveWarnMessageBox;
	GameMainMenuSaveGame(InteractiveGameBase&, UI::UniqueWindow::Registry& registry);

	void fill_table();
	void select_by_name(std::string name);

protected:
	void die() override;

private:
	InteractiveGameBase& igbase();
	void entry_selected(uint32_t);
	void double_clicked(uint32_t);
	void edit_box_changed();
	void ok();
	void delete_clicked();

	bool save_game(std::string);
	void pause_game(bool paused);

	// UI coordinates and spacers
	int32_t const padding_;  // Common padding between panels
	int32_t const buth_;     // Button dimensions
	int32_t const tablex_, tabley_, tablew_, tableh_;
	int32_t const right_column_x_;
	int32_t const butw_;  // Button dimensions

	UI::Textarea editbox_label_;
	UI::EditBox editbox_;
	LoadOrSaveGame load_or_save_;

	UI::Button ok_, cancel_, delete_;
	std::string curdir_;
	std::string parentdir_;
	std::string filename_;
	bool overwrite_;
};

#endif  // end of include guard: WL_WUI_GAME_MAIN_MENU_SAVE_GAME_H
