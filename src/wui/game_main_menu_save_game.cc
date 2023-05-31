/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/game_main_menu_save_game.h"

#include <memory>

#include "base/i18n.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "game_io/game_saver.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/illegal_filename_check.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/generic_save_handler.h"
#include "logic/playersmanager.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"
#include "wui/interactive_gamebase.h"

struct TypeInfo {
	std::string window_name;
	std::string window_title;
	LoadOrSaveGame::FileType file_type;
};
static const std::map<GameMainMenuSaveGame::Type, TypeInfo> kTypes = {
   {GameMainMenuSaveGame::Type::kLoadReplay,
    {"load_replay", gettext_noop("Load Replay"), LoadOrSaveGame::FileType::kReplay}},
   {GameMainMenuSaveGame::Type::kLoadSavegame,
    {"load_game", gettext_noop("Load Game"), LoadOrSaveGame::FileType::kGameSinglePlayer}},
   {GameMainMenuSaveGame::Type::kSave,
    {"save_game", gettext_noop("Save Game"), LoadOrSaveGame::FileType::kShowAll}}};

InteractiveGameBase& GameMainMenuSaveGame::igbase() {
	return dynamic_cast<InteractiveGameBase&>(*get_parent());
}

GameMainMenuSaveGame::GameMainMenuSaveGame(InteractiveGameBase& parent,
                                           UI::UniqueWindow::Registry& registry,
                                           const Type type)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      kTypes.at(type).window_name,
                      &registry,
                      parent.get_inner_w() - 40,
                      parent.get_inner_h() - 40,
                      _(kTypes.at(type).window_title)),

     type_(type),

     main_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     info_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),

     load_or_save_(&info_box_,
                   igbase().game(),
                   kTypes.at(type).file_type,
                   UI::PanelStyle::kWui,
                   UI::WindowStyle::kWui,
                   false),

     filename_box_(load_or_save_.table_box(), UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
     filename_label_(&filename_box_,
                     UI::PanelStyle::kWui,
                     UI::FontStyle::kWuiLabel,
                     0,
                     0,
                     0,
                     0,
                     _("Filename:"),
                     UI::Align::kLeft),
     filename_editbox_(&filename_box_, 0, 0, 0, UI::PanelStyle::kWui),

     buttons_box_(load_or_save_.game_details()->button_box(),
                  UI::PanelStyle::kWui,
                  0,
                  0,
                  UI::Box::Horizontal),
     cancel_(&buttons_box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kWuiSecondary, _("Cancel")),
     ok_(&buttons_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kWuiPrimary, _("OK")),

     curdir_(kSaveDir),
     illegal_filename_tooltip_(FileSystemHelper::illegal_filename_tooltip()) {
	filename_box_.set_visible(type_ == Type::kSave);

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

	filename_editbox_.changed.connect([this]() { edit_box_changed(); });
	filename_editbox_.ok.connect([this]() { ok(); });
	filename_editbox_.cancel.connect(
	   [this, &parent]() { reset_editbox_or_die(parent.game().save_handler().get_cur_filename()); });

	ok_.sigclicked.connect([this]() { ok(); });
	cancel_.sigclicked.connect([this]() { die(); });

	load_or_save_.table().selected.connect([this](unsigned /* index */) { entry_selected(); });
	load_or_save_.table().double_clicked.connect([this](unsigned /* index */) { ok(); });
	load_or_save_.table().cancel.connect([this]() { die(); });

	load_or_save_.fill_table();
	load_or_save_.select_by_name(parent.game().save_handler().get_cur_filename());

	center_to_parent();
	set_z(UI::Panel::ZOrder::kFullscreenWindow);

	if (type_ == Type::kSave) {
		filename_editbox_.focus();
	} else {
		load_or_save_.table().focus();
	}
	pause_game(true);
	layout();

	initialization_complete();

	do_run();  // Modal main loop, blocks until the user closes the window.
	die();
}

void GameMainMenuSaveGame::layout() {
	main_box_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 2 * padding_);
	load_or_save_.table().set_desired_size(get_inner_w() * 7 / 12, 0);
	load_or_save_.delete_button()->set_desired_size(cancel_.get_w(), cancel_.get_h());

	UI::UniqueWindow::layout();
}

void GameMainMenuSaveGame::entry_selected() {
	ok_.set_enabled(load_or_save_.table().selections().size() == 1);
	load_or_save_.delete_button()->set_enabled(load_or_save_.has_selection());
	if (load_or_save_.has_selection()) {
		std::unique_ptr<SavegameData> gamedata = load_or_save_.entry_selected();
		if (!gamedata->is_directory()) {
			filename_editbox_.set_text(FileSystem::filename_without_ext(gamedata->filename.c_str()));
		}
	}
}

void GameMainMenuSaveGame::edit_box_changed() {
	// Prevent the user from creating nonsense directory names, like e.g. ".." or "...".
	const bool is_legal_filename = FileSystemHelper::is_legal_filename(filename_editbox_.get_text());
	ok_.set_enabled(is_legal_filename);
	filename_editbox_.set_tooltip(is_legal_filename ? "" : illegal_filename_tooltip_);
	load_or_save_.delete_button()->set_enabled(false);
	load_or_save_.clear_selections();
}

void GameMainMenuSaveGame::reset_editbox_or_die(const std::string& current_filename) {
	if (filename_editbox_.get_text() == current_filename) {
		die();
	} else {
		filename_editbox_.set_text(current_filename);
		load_or_save_.select_by_name(current_filename);
	}
}

void GameMainMenuSaveGame::ok() {
	if (!ok_.enabled()) {
		return;
	}

	if (load_or_save_.has_selection()) {
		std::unique_ptr<SavegameData> gamedata = load_or_save_.entry_selected();
		if (gamedata->is_directory()) {
			load_or_save_.change_directory_to(gamedata->filename);
			curdir_ = gamedata->filename;
			filename_editbox_.focus();
			return;
		}
		if (type_ == Type::kLoadSavegame || type_ == Type::kLoadReplay) {
			if (load_or_save_.check_replay_compatibility(*gamedata)) {
				igbase().game().set_next_game_to_load(gamedata->filename);
				end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
				igbase().end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
			}
			return;
		}
	}
	if (type_ == Type::kSave) {
		std::string filename = filename_editbox_.get_text();
		if (save_game(filename, !get_config_bool("nozip", false))) {
			die();
		} else {
			load_or_save_.table().focus();
		}
	}
}

void GameMainMenuSaveGame::die() {
	pause_game(false);
	UI::UniqueWindow::die();
}

bool GameMainMenuSaveGame::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		if (matches_shortcut(KeyboardShortcut::kCommonDeleteItem, code)) {
			load_or_save_.clicked_delete();
			return true;
		}
		switch (code.sym) {
		case SDLK_RETURN:
			ok();
			return true;
		case SDLK_ESCAPE:
			die();
			return true;
		default:
			break;  // not handled
		}
	}
	return UI::UniqueWindow::handle_key(down, code);
}

void GameMainMenuSaveGame::pause_game(bool paused) {
	if (igbase().is_multiplayer()) {
		return;
	}
	igbase().game().game_controller()->set_paused(paused);
}

/**
 * Save the game in the Savegame directory with
 * the given filename
 *
 * returns true if dialog should close, false if it
 * should stay open
 */
bool GameMainMenuSaveGame::save_game(std::string filename, bool binary) {
	// Trim it for preceding/trailing whitespaces in user input
	trim(filename);

	//  OK, first check if the extension matches (ignoring case).
	if (!ends_with(filename, kSavegameExtension, false)) {
		filename += kSavegameExtension;
	}

	//  Append directory name.
	const std::string complete_filename = curdir_ + FileSystem::file_separator() + filename;

	//  Check if file exists. If so, show a warning.
	if (g_fs->file_exists(complete_filename)) {
		const std::string s = format(_("A file with the name ‘%s’ already exists. Overwrite?"),
		                             FileSystem::fs_filename(filename.c_str()));
		UI::WLMessageBox mbox(this, UI::WindowStyle::kWui, _("Error Saving Game!"), s,
		                      UI::WLMessageBox::MBoxType::kOkCancel);
		if (mbox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack) {
			return false;
		}
	}

	// Try saving the game.
	Widelands::Game& game = igbase().game();

	game.create_loader_ui(
	   {"general_game"}, true, game.map().get_background_theme(), game.map().get_background(), true);

	GenericSaveHandler gsh(
	   [&game](FileSystem& fs) {
		   Widelands::GameSaver gs(fs, game);
		   gs.save();
	   },
	   complete_filename, binary ? FileSystem::ZIP : FileSystem::DIR);
	GenericSaveHandler::Error error;
	{
		MutexLock m(MutexLock::ID::kLogicFrame, [this]() { stay_responsive(); });
		error = gsh.save();
	}

	game.remove_loader_ui();

	// If only the temporary backup couldn't be deleted, we still treat it as
	// success. Automatic cleanup will deal with later. No need to bother the
	// player with it.
	if (error == GenericSaveHandler::Error::kSuccess ||
	    error == GenericSaveHandler::Error::kDeletingBackupFailed) {
		game.save_handler().set_current_filename(complete_filename);
		igbase().log_message(_("Game saved"));
		return true;
	}

	// Show player an error message.
	std::string msg = gsh.localized_formatted_result_message();
	UI::WLMessageBox mbox(
	   this, UI::WindowStyle::kWui, _("Error Saving Game!"), msg, UI::WLMessageBox::MBoxType::kOk);
	mbox.run<UI::Panel::Returncodes>();

	// If only the backup failed (likely just because of a file lock),
	// then leave the dialog open for the player to try with a new filename.
	if (error == GenericSaveHandler::Error::kBackupFailed) {
		return false;
	}

	// In the other error cases close the dialog.
	igbase().log_message(_("Saving failed!"));
	return true;
}
