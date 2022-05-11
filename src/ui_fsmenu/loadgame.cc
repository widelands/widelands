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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_fsmenu/loadgame.h"

#include <memory>

#include "base/i18n.h"
#include "build_info.h"
#include "logic/replay.h"
#include "logic/replay_game_controller.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/main.h"
#include "wlapplication.h"
#include "wlapplication_options.h"
#include "wui/gamedetails.h"
#include "wui/interactive_spectator.h"

namespace FsMenu {

LoadGame::LoadGame(MenuCapsule& fsmm,
                   Widelands::Game& g,
                   GameSettingsProvider& gsp,
                   bool take_ownership_of_game_and_settings,
                   bool is_replay,
                   const std::function<void(const std::string&)>& callback)
   : TwoColumnsFullNavigationMenu(fsmm, is_replay ? _("Choose Replay") : _("Choose Game")),
     game_(g),
     settings_(gsp),
     take_ownership_of_game_and_settings_(take_ownership_of_game_and_settings),
     callback_on_ok_(callback),
     load_or_save_(&right_column_content_box_,
                   g,
                   is_replay ?
                      LoadOrSaveGame::FileType::kReplay :
                      (gsp.settings().multiplayer ? LoadOrSaveGame::FileType::kGameMultiPlayer :
                                                    LoadOrSaveGame::FileType::kGameSinglePlayer),
                   UI::PanelStyle::kFsMenu,
                   UI::WindowStyle::kFsMenu,
                   true,
                   &left_column_box_,
                   &right_column_content_box_),

     is_replay_(is_replay),
     update_game_details_(false),
     showing_filenames_(false) {

	if (is_replay_) {
		show_filenames_ = new UI::Checkbox(
		   &header_box_, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show Filenames"));
		header_box_.add(show_filenames_, UI::Box::Resizing::kFullSize);
		header_box_.add_space(5 * kPadding);
	}

	left_column_box_.add(load_or_save_.table_box(), UI::Box::Resizing::kExpandBoth);
	right_column_content_box_.add(load_or_save_.game_details(), UI::Box::Resizing::kExpandBoth);
	right_column_content_box_.add(load_or_save_.delete_button(), UI::Box::Resizing::kFullSize);

	layout();

	ok_.set_enabled(false);

	if (is_replay_) {
		back_.set_tooltip(_("Return to the main menu"));
		ok_.set_tooltip(_("Load this replay"));
	} else {
		back_.set_tooltip(gsp.settings().multiplayer ? _("Return to the multiplayer game setup") :
                                                     _("Return to the single player menu"));
		ok_.set_tooltip(_("Load this game"));
	}

	load_or_save_.table().selected.connect([this](unsigned /* value */) { entry_selected(); });
	load_or_save_.table().double_clicked.connect([this](unsigned /* value */) { clicked_ok(); });

	if (is_replay_) {
		show_filenames_->changed.connect([this]() { toggle_filenames(); });
		show_filenames_->set_state(get_config_bool("display_replay_filenames", true));
	}

	fill_table();
	if (!load_or_save_.table().empty()) {
		load_or_save_.table().select(0);
	}

	load_or_save_.table().cancel.connect([this]() { clicked_back(); });

	initialization_complete();
}

LoadGame::~LoadGame() {
	if (take_ownership_of_game_and_settings_) {
		delete &game_;
		delete &settings_;
	}
}

void LoadGame::layout() {
	TwoColumnsFullNavigationMenu::layout();
	load_or_save_.delete_button()->set_desired_size(0, standard_height_);
}
void LoadGame::think() {
	TwoColumnsFullNavigationMenu::think();

	if (update_game_details_) {
		// Call performance heavy draw_minimap function only during think
		update_game_details_ = false;
		load_or_save_.entry_selected();
		ok_.set_enabled(!load_or_save_.game_details()->has_conflicts());
	}
}

void LoadGame::toggle_filenames() {
	showing_filenames_ = show_filenames_->get_state();
	set_config_bool("display_replay_filenames", showing_filenames_);

	// Remember selection
	const std::set<uint32_t> selected = load_or_save_.table().selections();
	// Fill table again
	fill_table();

	// Restore selection items
	// TODO(GunChleoc): It would be nice to have a function to just change the entry texts
	for (const uint32_t selectme : selected) {
		load_or_save_.table().multiselect(selectme, true);
	}
	entry_selected();
}

void LoadGame::clicked_ok() {
	if (load_or_save_.game_details()->has_conflicts() ||
	    load_or_save_.table().selections().size() != 1) {
		return;
	}

	std::unique_ptr<SavegameData> gamedata = load_or_save_.entry_selected();
	if (gamedata->is_directory()) {
		load_or_save_.change_directory_to(gamedata->filename);
	} else {
		if (gamedata && gamedata->errormessage.empty()) {
			if (!take_ownership_of_game_and_settings_) {
				callback_on_ok_(gamedata->filename);
				die();
				return;
			}

			if (is_replay_ && gamedata->version != build_id()) {
				UI::WLMessageBox w(&capsule_.menu(), UI::WindowStyle::kFsMenu, _("Version Mismatch"),
				                   _("This replay was created with a different Widelands version. It "
				                     "might be compatible, but will more likely desync or even fail to "
				                     "load.\n\nPlease do not report any bugs that occur while watching "
				                     "this replay.\n\nDo you want to load the replay anyway?"),
				                   UI::WLMessageBox::MBoxType::kOkCancel);
				if (w.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
					return;
				}
			}

			capsule_.set_visible(false);

			try {
				if (is_replay_) {
					game_.create_loader_ui({"general_game"}, true, settings_.settings().map_theme,
					                       settings_.settings().map_background);

					game_.set_ibase(new InteractiveSpectator(game_, get_config_section()));
					game_.set_write_replay(false);

					new ReplayGameController(game_, gamedata->filename);
					game_.save_handler().set_allow_saving(false);

					game_.run(Widelands::Game::StartGameType::kSaveGame, "", true, "replay");

				} else {
					game_.run_load_game(gamedata->filename, "");
				}
			} catch (const std::exception& e) {
				WLApplication::emergency_save(&capsule_.menu(), game_, e.what());
			}
			return_to_main_menu();
		}
	}
}

void LoadGame::entry_selected() {
	ok_.set_enabled(!load_or_save_.game_details()->has_conflicts() &&
	                load_or_save_.table().selections().size() == 1);
	if (load_or_save_.has_selection()) {
		// Update during think() instead of every keypress
		update_game_details_ = true;
	} else {
		load_or_save_.delete_button()->set_enabled(false);
	}
}

void LoadGame::fill_table() {
	load_or_save_.set_show_filenames(showing_filenames_);
	load_or_save_.fill_table();
}

bool LoadGame::handle_key(bool down, SDL_Keysym code) {
	if (!down) {
		return false;
	}
	if (matches_shortcut(KeyboardShortcut::kCommonDeleteItem, code)) {
		load_or_save_.clicked_delete();
		return true;
	}
	return TwoColumnsFullNavigationMenu::handle_key(down, code);
}
}  // namespace FsMenu
