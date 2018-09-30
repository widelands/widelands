/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "game_io/game_saver.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/playersmanager.h"
#include "ui_basic/messagebox.h"
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

     main_box_(this, 0, 0, UI::Box::Vertical),
     info_box_(&main_box_, 0, 0, UI::Box::Horizontal),

     load_or_save_(
        &info_box_, igbase().game(), LoadOrSaveGame::FileType::kGame, UI::PanelStyle::kWui, false),

     filename_box_(load_or_save_.table_box(), 0, 0, UI::Box::Horizontal),
     filename_label_(&filename_box_, 0, 0, 0, 0, _("Filename:"), UI::Align::kLeft),
     filename_editbox_(&filename_box_, 0, 0, 0, 0, 2, UI::PanelStyle::kWui),

     buttons_box_(load_or_save_.game_details()->button_box(), 0, 0, UI::Box::Horizontal),
     cancel_(&buttons_box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kWuiSecondary, _("Cancel")),
     ok_(&buttons_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kWuiPrimary, _("OK")),

     curdir_(kSaveDir),
     illegal_filename_tooltip_(FileSystem::illegal_filename_tooltip()) {

	layout();

	main_box_.add_space(padding_);
	main_box_.set_inner_spacing(padding_);
	main_box_.add(&info_box_, UI::Box::Resizing::kExpandBoth);

	info_box_.set_inner_spacing(padding_);
	info_box_.add_space(padding_);
	info_box_.add(load_or_save_.table_box(), UI::Box::Resizing::kFullSize);
	info_box_.add(load_or_save_.game_details(), UI::Box::Resizing::kExpandBoth);

	load_or_save_.table_box()->add_space(padding_);
	load_or_save_.table_box()->add(&filename_box_, UI::Box::Resizing::kFullSize);

	filename_box_.set_inner_spacing(padding_);
	filename_box_.add(&filename_label_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	filename_box_.add(&filename_editbox_, UI::Box::Resizing::kFillSpace);

	load_or_save_.game_details()->button_box()->add_space(padding_);
	load_or_save_.game_details()->button_box()->add(&buttons_box_, UI::Box::Resizing::kFullSize);
	buttons_box_.set_inner_spacing(padding_);
	buttons_box_.add(&cancel_, UI::Box::Resizing::kFillSpace);
	buttons_box_.add(&ok_, UI::Box::Resizing::kFillSpace);

	ok_.set_enabled(false);

	filename_editbox_.changed.connect(boost::bind(&GameMainMenuSaveGame::edit_box_changed, this));
	filename_editbox_.ok.connect(boost::bind(&GameMainMenuSaveGame::ok, this));
	filename_editbox_.cancel.connect(boost::bind(&GameMainMenuSaveGame::reset_editbox_or_die, this,
	                                             parent.game().save_handler().get_cur_filename()));

	ok_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::ok, this));
	cancel_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::die, this));

	load_or_save_.table().selected.connect(boost::bind(&GameMainMenuSaveGame::entry_selected, this));
	load_or_save_.table().double_clicked.connect(boost::bind(&GameMainMenuSaveGame::ok, this));
	load_or_save_.table().cancel.connect(boost::bind(&GameMainMenuSaveGame::die, this));

	load_or_save_.fill_table();
	load_or_save_.select_by_name(parent.game().save_handler().get_cur_filename());

	center_to_parent();
	move_to_top();

	pause_game(true);
	set_thinks(false);
	layout();
}

void GameMainMenuSaveGame::layout() {
	main_box_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 2 * padding_);
	load_or_save_.table().set_desired_size(get_inner_w() * 7 / 12, load_or_save_.table().get_h());
	load_or_save_.delete_button()->set_desired_size(ok_.get_w(), ok_.get_h());
}

void GameMainMenuSaveGame::entry_selected() {
	ok_.set_enabled(load_or_save_.table().selections().size() == 1);
	load_or_save_.delete_button()->set_enabled(load_or_save_.has_selection());
	if (load_or_save_.has_selection()) {
		std::unique_ptr<SavegameData> gamedata = load_or_save_.entry_selected();
		filename_editbox_.set_text(FileSystem::filename_without_ext(gamedata->filename.c_str()));
	}
}

void GameMainMenuSaveGame::edit_box_changed() {
	// Prevent the user from creating nonsense directory names, like e.g. ".." or "...".
	const bool is_legal_filename = FileSystem::is_legal_filename(filename_editbox_.text());
	ok_.set_enabled(is_legal_filename);
	filename_editbox_.set_tooltip(is_legal_filename ? "" : illegal_filename_tooltip_);
	load_or_save_.delete_button()->set_enabled(false);
	load_or_save_.clear_selections();
}

void GameMainMenuSaveGame::reset_editbox_or_die(const std::string& current_filename) {
	if (filename_editbox_.text() == current_filename) {
		die();
	} else {
		filename_editbox_.set_text(current_filename);
		load_or_save_.select_by_name(current_filename);
	}
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

void GameMainMenuSaveGame::ok() {
	if (filename_editbox_.text().empty()) {
		return;
	}

	std::string const complete_filename =
	   igbase().game().save_handler().create_file_name(curdir_, filename_editbox_.text());

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

bool GameMainMenuSaveGame::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			ok();
			return true;
		case SDLK_ESCAPE:
			die();
			return true;
		case SDLK_DELETE:
			load_or_save_.clicked_delete();
			return true;
		default:
			break;  // not handled
		}
	}
	return UI::Panel::handle_key(down, code);
}

void GameMainMenuSaveGame::pause_game(bool paused) {
	if (igbase().is_multiplayer()) {
		return;
	}
	igbase().game().game_controller()->set_paused(paused);
}
