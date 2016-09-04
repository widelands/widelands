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

#include "wui/game_main_menu_save_game.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "game_io/game_saver.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/playersmanager.h"
#include "wui/interactive_gamebase.h"

InteractiveGameBase& GameMainMenuSaveGame::igbase() {
	return dynamic_cast<InteractiveGameBase&>(*get_parent());
}

GameMainMenuSaveGame::GameMainMenuSaveGame(InteractiveGameBase& parent,
                                           UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      "save_game",
                      &registry,
                      parent.get_inner_w() - 40,
                      parent.get_inner_h() - 40,
                      _("Save Game")),
     // Values for alignment and size
     padding_(4),
     buth_(20),
     tablex_(padding_),
     tabley_(padding_),
     tablew_(get_inner_w() * 7 / 12),
     tableh_(get_inner_h() - tabley_ - 3 * buth_ - 2 * padding_),
     right_column_x_(tablew_ + 2 * padding_),
     butw_((get_inner_w() - right_column_x_ - 2 * padding_) / 2),
     editbox_label_(this,
                    padding_,
                    tabley_ + tableh_ + 3 * padding_,
                    butw_,
                    buth_,
                    _("Filename:"),
                    UI::Align::kLeft),
     editbox_(this,
              editbox_label_.get_w() + 2 * padding_,
              tabley_ + tableh_ + 3 * padding_,
              get_inner_w() - editbox_label_.get_w() - 3 * padding_,
              buth_,
              2,
              g_gr->images().get("images/ui_basic/but1.png")),
     load_or_save_(this,
                   igbase().game(),
                   tablex_,
                   tabley_,
                   tablew_,
                   tableh_,
                   padding_,
                   LoadOrSaveGame::FileType::kGame,
                   GameDetails::Style::kWui,
                   false),
     ok_(this,
         "ok",
         UI::g_fh1->fontset()->is_rtl() ? get_inner_w() / 2 - butw_ - padding_ :
                                          get_inner_w() / 2 + padding_,
         get_inner_h() - padding_ - buth_,
         butw_,
         buth_,
         g_gr->images().get("images/ui_basic/but5.png"),
         _("OK")),
     cancel_(this,
             "cancel",
             UI::g_fh1->fontset()->is_rtl() ? get_inner_w() / 2 + padding_ :
                                              get_inner_w() / 2 - butw_ - padding_,
             get_inner_h() - padding_ - buth_,
             butw_,
             buth_,
             g_gr->images().get("images/ui_basic/but1.png"),
             _("Cancel")),
     delete_(this,
             "delete",
             right_column_x_,
             tabley_ + tableh_ - buth_,
             get_inner_w() - right_column_x_ - padding_,
             buth_,
             g_gr->images().get("images/ui_basic/but1.png"),
             _("Delete")),
     curdir_(SaveHandler::get_base_dir()) {
	editbox_.changed.connect(boost::bind(&GameMainMenuSaveGame::edit_box_changed, this));
	editbox_.ok.connect(boost::bind(&GameMainMenuSaveGame::ok, this));

	ok_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::ok, this));
	cancel_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::die, this));
	delete_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::delete_clicked, this));

	load_or_save_.table().selected.connect(
	   boost::bind(&GameMainMenuSaveGame::entry_selected, this, _1));
	load_or_save_.table().double_clicked.connect(
	   boost::bind(&GameMainMenuSaveGame::double_clicked, this, _1));

	fill_table();

	center_to_parent();
	move_to_top();

	std::string cur_filename = parent.game().save_handler().get_cur_filename();
	if (!cur_filename.empty()) {
		load_or_save_.select_by_name(cur_filename);
	}
	editbox_.focus();
	pause_game(true);
}

/**
 * called when a item is selected
 */
void GameMainMenuSaveGame::entry_selected(uint32_t) {
	if (load_or_save_.has_selection()) {
		const SavegameData& gamedata = *load_or_save_.entry_selected();
		editbox_.set_text(FileSystem::filename_without_ext(gamedata.filename.c_str()));
		ok_.set_enabled(true);
	}
}

/**
 * An Item has been doubleclicked
 */
void GameMainMenuSaveGame::double_clicked(uint32_t) {
	ok();
}

/*
 * fill the file list
 */
void GameMainMenuSaveGame::fill_table() {
	load_or_save_.fill_table();
}

/*
 * The editbox was changed. Enable ok button
 */
void GameMainMenuSaveGame::edit_box_changed() {
	// Prevent the user from creating nonsense directory names, like e.g. ".." or "...".
	ok_.set_enabled(LayeredFileSystem::is_legal_filename(editbox_.text()));
}

static void dosave(InteractiveGameBase& igbase, const std::string& complete_filename) {
	Widelands::Game& game = igbase.game();

	std::string error;
	if (!game.save_handler().save_game(game, complete_filename, &error)) {
		std::string s = _("Game Saving Error!\nSaved game file may be corrupt!\n\n"
		                  "Reason given:\n");
		s += error;
		UI::WLMessageBox mbox(&igbase, _("Save Game Error!"), s, UI::WLMessageBox::MBoxType::kOk);
		mbox.run<UI::Panel::Returncodes>();
	}
	game.save_handler().set_current_filename(complete_filename);
}

struct SaveWarnMessageBox : public UI::WLMessageBox {
	SaveWarnMessageBox(GameMainMenuSaveGame& parent, const std::string& filename)
	   : UI::WLMessageBox(&parent,
	                      _("Save Game Error!"),
	                      (boost::format(_("A file with the name ‘%s’ already exists. Overwrite?")) %
	                       FileSystem::fs_filename(filename.c_str()))
	                         .str(),
	                      MBoxType::kOkCancel),
	     filename_(filename) {
	}

	GameMainMenuSaveGame& menu_save_game() {
		return dynamic_cast<GameMainMenuSaveGame&>(*get_parent());
	}

	void clicked_ok() override {
		g_fs->fs_unlink(filename_);
		dosave(menu_save_game().igbase(), filename_);
		menu_save_game().die();
	}

	void clicked_back() override {
		die();
	}

private:
	std::string const filename_;
};

/**
 * Called when the Ok button is clicked or the Return key pressed in the edit box.
 */
void GameMainMenuSaveGame::ok() {
	if (editbox_.text().empty())
		return;

	std::string const complete_filename =
	   igbase().game().save_handler().create_file_name(curdir_, editbox_.text());

	//  Check if file exists. If it does, show a warning.
	if (g_fs->file_exists(complete_filename)) {
		new SaveWarnMessageBox(*this, complete_filename);
	} else {
		dosave(igbase(), complete_filename);
		die();
	}
}

void GameMainMenuSaveGame::die() {
	pause_game(false);
	UI::UniqueWindow::die();
}

struct DeletionMessageBox : public UI::WLMessageBox {
	DeletionMessageBox(GameMainMenuSaveGame& parent, const std::string& filename)
	   : UI::WLMessageBox(&parent,
	                      _("File deletion"),
	                      str(boost::format(_("Do you really want to delete the file %s?")) %
	                          FileSystem::fs_filename(filename.c_str())),
	                      MBoxType::kOkCancel),
	     filename_(filename) {
	}

	void clicked_ok() override {
		g_fs->fs_unlink(filename_);
		dynamic_cast<GameMainMenuSaveGame&>(*get_parent()).fill_table();
		die();
	}

	void clicked_back() override {
		die();
	}

private:
	std::string const filename_;
};

/**
 * Called when the delete button has been clicked
 */
void GameMainMenuSaveGame::delete_clicked() {
	std::string const complete_filename =
	   igbase().game().save_handler().create_file_name(curdir_, editbox_.text());

	//  Check if file exists. If it does, let the user confirm the deletion.
	if (g_fs->file_exists(complete_filename))
		new DeletionMessageBox(*this, complete_filename);
}

void GameMainMenuSaveGame::pause_game(bool paused) {
	if (igbase().is_multiplayer()) {
		return;
	}
	igbase().game().game_controller()->set_paused(paused);
}
