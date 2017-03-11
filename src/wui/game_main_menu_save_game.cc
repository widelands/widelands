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
     butw_(150),

     main_box_(this, 0, 0, UI::Box::Vertical),
     info_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     filename_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     buttons_box_(&main_box_, 0, 0, UI::Box::Horizontal),

     load_or_save_(&info_box_,
                   igbase().game(),
                   LoadOrSaveGame::FileType::kGame,
                   GameDetails::Style::kWui,
                   false),
     delete_(load_or_save_.game_details(),
             "delete",
             0,
             0,
             0,
             0,
             g_gr->images().get("images/ui_basic/but1.png"),
             _("Delete")),

     editbox_label_(&filename_box_, 0, 0, 0, 0, _("Filename:"), UI::Align::kLeft),
     editbox_(&filename_box_, 0, 0, 0, 0, 2, g_gr->images().get("images/ui_basic/but1.png")),

     cancel_(&buttons_box_,
             "cancel",
             0,
             0,
             butw_,
             0,
             g_gr->images().get("images/ui_basic/but1.png"),
             _("Cancel")),
     ok_(&buttons_box_,
         "ok",
         0,
         0,
         butw_,
         0,
         g_gr->images().get("images/ui_basic/but5.png"),
         _("OK")),

     curdir_(SaveHandler::get_base_dir()) {

	layout();

	main_box_.add_space(padding_);
	main_box_.set_inner_spacing(padding_);
	main_box_.add(&info_box_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(padding_);
	main_box_.add(&filename_box_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(0);
	main_box_.add(&buttons_box_, UI::Box::Resizing::kFullSize);

	info_box_.set_inner_spacing(padding_);
	info_box_.add_space(padding_);
	info_box_.add(&load_or_save_.table(), UI::Box::Resizing::kFullSize);
	info_box_.add(load_or_save_.game_details(), UI::Box::Resizing::kExpandBoth);
	load_or_save_.game_details()->add(&delete_, UI::Box::Resizing::kFullSize);

	filename_box_.set_inner_spacing(padding_);
	filename_box_.add_space(padding_);
	filename_box_.add(&editbox_label_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	filename_box_.add(&editbox_, UI::Box::Resizing::kFillSpace);

	buttons_box_.set_inner_spacing(padding_);
	buttons_box_.add_space(padding_);
	buttons_box_.add_inf_space();
	buttons_box_.add_inf_space();
	buttons_box_.add(&cancel_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	buttons_box_.add_inf_space();
	buttons_box_.add(&ok_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	buttons_box_.add_inf_space();
	buttons_box_.add_inf_space();

	ok_.set_enabled(false);
	delete_.set_enabled(false);

	editbox_.changed.connect(boost::bind(&GameMainMenuSaveGame::edit_box_changed, this));
	editbox_.ok.connect(boost::bind(&GameMainMenuSaveGame::ok, this));

	ok_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::ok, this));
	cancel_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::die, this));
	delete_.sigclicked.connect(boost::bind(&GameMainMenuSaveGame::delete_clicked, this));

	load_or_save_.table().selected.connect(boost::bind(&GameMainMenuSaveGame::entry_selected, this));
	load_or_save_.table().double_clicked.connect(
	   boost::bind(&GameMainMenuSaveGame::double_clicked, this));

	load_or_save_.fill_table(parent.game().save_handler().get_cur_filename());
	center_to_parent();
	move_to_top();

	pause_game(true);
	set_thinks(false);
}

void GameMainMenuSaveGame::layout() {
	main_box_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 2 * padding_);
	load_or_save_.table().set_desired_size(get_inner_w() * 7 / 12, load_or_save_.table().get_h());
}

/**
 * called when a item is selected
 */
void GameMainMenuSaveGame::entry_selected() {
	// TODO(GunChleoc): When editbox is focused, multiselect is not possible, because it steals the
	// key presses.
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
	const std::string header =
	   no_selections == 1 ?
	      _("Do you really want to delete this game?") :
	      (boost::format(ngettext("Do you really want to delete this %d game?",
	                              "Do you really want to delete these %d games?", no_selections)) %
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
