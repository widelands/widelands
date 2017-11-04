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

namespace {

#define WINDOW_WIDTH 440
#define WINDOW_HEIGHT 440
#define VMARGIN 5
#define VSPACING 5
#define HSPACING 5
#define BUTTON_HEIGHT 20
#define LIST_WIDTH 280
#define LIST_HEIGHT (WINDOW_HEIGHT - 2 * VMARGIN - VSPACING)
#define EDITBOX_Y (WINDOW_HEIGHT - 24 - VMARGIN)
#define DESCRIPTION_X (VMARGIN + LIST_WIDTH + VSPACING)
#define DESCRIPTION_WIDTH (WINDOW_WIDTH - DESCRIPTION_X - VMARGIN)
#define CANCEL_Y (WINDOW_HEIGHT - BUTTON_HEIGHT - VMARGIN)
#define DELETE_Y (CANCEL_Y - BUTTON_HEIGHT - VSPACING)
#define OK_Y (DELETE_Y - BUTTON_HEIGHT - VSPACING)

}  // namespace

InteractiveGameBase& GameMainMenuSaveGame::igbase() {
	return dynamic_cast<InteractiveGameBase&>(*get_parent());
}

GameMainMenuSaveGame::GameMainMenuSaveGame(InteractiveGameBase& parent,
                                           UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "save_game", &registry, WINDOW_WIDTH, WINDOW_HEIGHT, _("Save Game")),
     editbox_(this,
              HSPACING,
              EDITBOX_Y,
              LIST_WIDTH,
              0,
              2,
              g_gr->images().get("images/ui_basic/but1.png")),
     ls_(this,
         HSPACING,
         VSPACING,
         LIST_WIDTH,
         LIST_HEIGHT - editbox_.get_h(),
         g_gr->images().get("images/ui_basic/but1.png")),
     name_label_(this, DESCRIPTION_X, 5, 0, 20, _("Map Name:")),
     mapname_(this, DESCRIPTION_X, 20, 0, 20),
     gametime_label_(this, DESCRIPTION_X, 45, 0, 20, _("Game Time:")),
     gametime_(this, DESCRIPTION_X, 60, 0, 20),
     players_label_(this, DESCRIPTION_X, 85, 0, 20),
     win_condition_label_(this, DESCRIPTION_X, 110, 0, 20, _("Win condition:")),
     win_condition_(this, DESCRIPTION_X, 125, 0, 20),
     curdir_(SaveHandler::get_base_dir()) {
	editbox_.changed.connect(boost::bind(&GameMainMenuSaveGame::edit_box_changed, this));
	editbox_.ok.connect(boost::bind(&GameMainMenuSaveGame::ok, this));

	button_ok_ = new UI::Button(this, "ok", DESCRIPTION_X, OK_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
	                            g_gr->images().get("images/ui_basic/but4.png"), _("OK"));
	button_ok_->sigclicked.connect(boost::bind(&GameMainMenuSaveGame::ok, this));

	UI::Button* cancelbtn =
	   new UI::Button(this, "cancel", DESCRIPTION_X, CANCEL_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
	                  g_gr->images().get("images/ui_basic/but4.png"), _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&GameMainMenuSaveGame::die, this));

	UI::Button* deletebtn =
	   new UI::Button(this, "delete", DESCRIPTION_X, DELETE_Y, DESCRIPTION_WIDTH, BUTTON_HEIGHT,
	                  g_gr->images().get("images/ui_basic/but4.png"), _("Delete"));
	deletebtn->sigclicked.connect(boost::bind(&GameMainMenuSaveGame::delete_clicked, this));

	ls_.selected.connect(boost::bind(&GameMainMenuSaveGame::selected, this, _1));
	ls_.double_clicked.connect(boost::bind(&GameMainMenuSaveGame::double_clicked, this, _1));

	fill_list();

	center_to_parent();
	move_to_top();

	std::string cur_filename = parent.game().save_handler().get_cur_filename();
	if (!cur_filename.empty()) {
		select_by_name(cur_filename);
	} else {
		// Display current game infos
		{
			// Try to translate the map name.
			i18n::Textdomain td("maps");
			mapname_.set_text(_(parent.game().map().get_name()));
		}
		uint32_t gametime = parent.game().get_gametime();
		gametime_.set_text(gametimestring(gametime));

		int player_nr = parent.game().player_manager()->get_number_of_players();
		players_label_.set_text(
		   (boost::format(ngettext("%i player", "%i players", player_nr)) % player_nr).str());
		{
			i18n::Textdomain td("win_conditions");
			win_condition_.set_text(_(parent.game().get_win_condition_displayname()));
		}
	}

	editbox_.focus();
	pause_game(true);
}

/**
 * called when a item is selected
 */
void GameMainMenuSaveGame::selected(uint32_t) {
	const std::string& name = ls_.get_selected();

	Widelands::GameLoader gl(name, igbase().game());
	Widelands::GamePreloadPacket gpdp;
	gl.preload_game(gpdp);  //  This has worked before, no problem
	{ editbox_.set_text(FileSystem::filename_without_ext(name.c_str())); }
	edit_box_changed();

	// Try to translate the map name.
	{
		i18n::Textdomain td("maps");
		mapname_.set_text(_(gpdp.get_mapname()));
	}

	uint32_t gametime = gpdp.get_gametime();
	gametime_.set_text(gametimestring(gametime));

	if (gpdp.get_number_of_players() > 0) {
		const std::string text =
		   (boost::format(ngettext("%u Player", "%u Players", gpdp.get_number_of_players())) %
		    static_cast<unsigned int>(gpdp.get_number_of_players()))
		      .str();
		players_label_.set_text(text);
	} else {
		// Keep label empty
		players_label_.set_text("");
	}
	win_condition_.set_text(_(gpdp.get_localized_win_condition()));
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
void GameMainMenuSaveGame::fill_list() {
	ls_.clear();
	FilenameSet gamefiles;

	//  Fill it with all files we find.
	gamefiles = g_fs->list_directory(curdir_);

	Widelands::GamePreloadPacket gpdp;

	for (FilenameSet::iterator pname = gamefiles.begin(); pname != gamefiles.end(); ++pname) {
		char const* const name = pname->c_str();

		try {
			Widelands::GameLoader gl(name, igbase().game());
			gl.preload_game(gpdp);
			ls_.add(FileSystem::filename_without_ext(name), name);
		} catch (const WException&) {
		}  //  we simply skip illegal entries
	}
	edit_box_changed();
}

void GameMainMenuSaveGame::select_by_name(const std::string& name) {
	for (uint32_t idx = 0; idx < ls_.size(); idx++) {
		const std::string val = ls_[idx];
		if (name == val) {
			ls_.select(idx);
			return;
		}
	}
}

/*
 * The editbox was changed. Enable ok button
 */
void GameMainMenuSaveGame::edit_box_changed() {
	// Prevent the user from creating nonsense directory names, like e.g. ".." or "...".
	button_ok_->set_enabled(LayeredFileSystem::is_legal_filename(editbox_.text()));
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
		dynamic_cast<GameMainMenuSaveGame&>(*get_parent()).fill_list();
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
