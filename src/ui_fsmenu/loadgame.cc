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

#include "ui_fsmenu/loadgame.h"

#include <memory>

#include "base/i18n.h"
#include "wlapplication_options.h"
#include "wui/gamedetails.h"
namespace FsMenu {
FullscreenMenuLoadGame::FullscreenMenuLoadGame(FullscreenMenuMain& fsmm,
                                               Widelands::Game& g,
                                               GameSettingsProvider* gsp,
                                               bool is_replay)
   : TwoColumnsNavigationMenu(
        fsmm, "choose_game", is_replay ? _("Choose Replay") : _("Choose Game")),
     load_or_save_(&right_column_content_box_,
                   g,
                   is_replay ?
                      LoadOrSaveGame::FileType::kReplay :
                      (gsp->settings().multiplayer ? LoadOrSaveGame::FileType::kGameMultiPlayer :
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
		back_.set_tooltip(gsp->settings().multiplayer ? _("Return to the multiplayer game setup") :
		                                                _("Return to the single player menu"));
		ok_.set_tooltip(_("Load this game"));
	}

	back_.sigclicked.connect([this]() { clicked_back(); });
	ok_.sigclicked.connect([this]() { clicked_ok(); });
	load_or_save_.table().selected.connect([this](unsigned) { entry_selected(); });
	load_or_save_.table().double_clicked.connect([this](unsigned) { clicked_ok(); });

	if (is_replay_) {
		show_filenames_->changed.connect([this]() { toggle_filenames(); });
		show_filenames_->set_state(get_config_bool("display_replay_filenames", true));
	}

	fill_table();
	if (!load_or_save_.table().empty()) {
		load_or_save_.table().select(0);
	}

	load_or_save_.table().cancel.connect([this]() { clicked_back(); });
}
void FullscreenMenuLoadGame::layout() {
	TwoColumnsNavigationMenu::layout();
	load_or_save_.delete_button()->set_desired_size(0, standard_height_);
}
void FullscreenMenuLoadGame::think() {
	TwoColumnsNavigationMenu::think();

	if (update_game_details_) {
		// Call performance heavy draw_minimap function only during think
		update_game_details_ = false;
		load_or_save_.entry_selected();
	}
}

void FullscreenMenuLoadGame::toggle_filenames() {
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

void FullscreenMenuLoadGame::clicked_ok() {
	if (load_or_save_.table().selections().size() != 1) {
		return;
	}

	std::unique_ptr<SavegameData> gamedata = load_or_save_.entry_selected();
	if (gamedata->is_directory()) {
		load_or_save_.change_directory_to(gamedata->filename);
	} else {
		if (gamedata && gamedata->errormessage.empty()) {
			filename_ = gamedata->filename;
			end_modal<MenuTarget>(MenuTarget::kOk);
		}
	}
}

void FullscreenMenuLoadGame::entry_selected() {
	ok_.set_enabled(load_or_save_.table().selections().size() == 1);
	if (load_or_save_.has_selection()) {
		// Update during think() instead of every keypress
		update_game_details_ = true;
	} else {
		load_or_save_.delete_button()->set_enabled(false);
	}
}

void FullscreenMenuLoadGame::fill_table() {
	load_or_save_.set_show_filenames(showing_filenames_);
	load_or_save_.fill_table();
}

const std::string& FullscreenMenuLoadGame::filename() const {
	return filename_;
}

bool FullscreenMenuLoadGame::handle_key(bool down, SDL_Keysym code) {
	if (!down) {
		return false;
	}

	switch (code.sym) {
	case SDLK_KP_PERIOD:
		if (code.mod & KMOD_NUM) {
			break;
		}
		FALLS_THROUGH;
	case SDLK_DELETE:
		load_or_save_.clicked_delete();
		return true;
	default:
		break;
	}

	return TwoColumnsNavigationMenu::handle_key(down, code);
}
}  // namespace FsMenu
