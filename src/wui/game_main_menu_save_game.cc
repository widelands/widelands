/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
	ok_.set_enabled(false);
	delete_.set_enabled(false);

	editbox_.changed.connect(boost::bind(&GameMainMenuSaveGame::edit_box_changed, this));
	editbox_.ok.connect(boost::bind(&GameMainMenuSaveGame::ok, this));

	ok_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::ok, this));
	cancel_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::die, this));
	delete_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::delete_clicked, this));

	load_or_save_.table().selected.connect(
		boost::bind(&GameMainMenuSaveGame::entry_selected, this));
	load_or_save_.table().double_clicked.connect(
		boost::bind(&GameMainMenuSaveGame::double_clicked, this));

	load_or_save_.fill_table(parent.game().save_handler().get_cur_filename());
	center_to_parent();
	move_to_top();

	editbox_.focus();
	pause_game(true);
}

/**
 * called when a item is selected
 */
void GameMainMenuSaveGame::entry_selected() {
	// NOCOM multiselect only works after the user has clicked the table without holding down a mofifier key.
	ok_.set_enabled(load_or_save_.table().selections().size() == 1);
	delete_.set_enabled(load_or_save_.has_selection());
	if (load_or_save_.has_selection()) {
		const SavegameData& gamedata = *load_or_save_.entry_selected();
		editbox_.set_text(FileSystem::filename_without_ext(gamedata.filename.c_str()));
	}
}

/**
 * An Item has been doubleclicked
 */
void GameMainMenuSaveGame::double_clicked() {
	ok();
}

/*
 * The editbox was changed. Enable ok button
 */
void GameMainMenuSaveGame::edit_box_changed() {
	// Prevent the user from creating nonsense directory names, like e.g. ".." or "...".
	const bool is_legal_filename = LayeredFileSystem::is_legal_filename(editbox_.text());
	ok_.set_enabled(is_legal_filename);
	delete_.set_enabled(false);
	load_or_save_.clear_selections();
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

/**
 * Called when the delete button has been clicked
 */
void GameMainMenuSaveGame::delete_clicked() {
	if (!load_or_save_.has_selection()) {
		return;
	}
	std::set<uint32_t> selections = load_or_save_.table().selections();
	const SavegameData& gamedata = *load_or_save_.entry_selected();
	size_t no_selections = selections.size();
	const std::string header = no_selections == 1 ?
						_("Do you really want to delete this game?") :
						(boost::format(ngettext("Do you really want to delete this %d game?",
														"Do you really want to delete these %d games?",
														no_selections)) %
						 no_selections)
							.str();

	std::string message = no_selections > 1 ? gamedata.filename_list : gamedata.filename;
	message = (boost::format("%s\n%s") % header % message).str();

	UI::WLMessageBox confirmationBox(
		this, ngettext("Confirm deleting file", "Confirm deleting files", no_selections), message,
		UI::WLMessageBox::MBoxType::kOkCancel);

	if (confirmationBox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		for (const uint32_t index : selections) {
			const std::string& deleteme = load_or_save_.get_filename(index);
			g_fs->fs_unlink(deleteme);
		}
		load_or_save_.fill_table();
	}
}

void GameMainMenuSaveGame::pause_game(bool paused) {
	if (igbase().is_multiplayer()) {
		return;
	}
	igbase().game().game_controller()->set_paused(paused);
}
