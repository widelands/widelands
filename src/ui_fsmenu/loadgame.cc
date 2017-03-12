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

#include "ui_fsmenu/loadgame.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <memory>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "graphic/text_constants.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/replay.h"

FullscreenMenuLoadGame::FullscreenMenuLoadGame(Widelands::Game& g,
                                               GameSettingsProvider* gsp,
                                               GameController* gc,
                                               bool is_replay)
   : FullscreenMenuLoadMapOrGame(),

     main_box_(this, 0, 0, UI::Box::Vertical),
     info_box_(&main_box_, 0, 0, UI::Box::Horizontal),

     // Main title
     title_(&main_box_,
            0,
            0,
            is_replay_ ? _("Choose a replay") : _("Choose a saved game"),
            UI::Align::kCenter),

     load_or_save_(&info_box_,
                   g,
                   is_replay ? LoadOrSaveGame::FileType::kReplay : gsp->settings().multiplayer ?
                               LoadOrSaveGame::FileType::kGameMultiPlayer :
                               LoadOrSaveGame::FileType::kGameSinglePlayer,
                   GameDetails::Style::kFsMenu,
                   true),

     is_replay_(is_replay),
     game_(g),
     settings_(gsp),
     ctrl_(gc) {

	// Make sure that we have some space to work with.
	main_box_.set_size(get_w(), get_w());

	main_box_.add_space(padding_);
	main_box_.add_inf_space();
	main_box_.add(&title_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add_inf_space();
	main_box_.add_inf_space();
	main_box_.add(&info_box_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(padding_);

	info_box_.add(&load_or_save_.table(), UI::Box::Resizing::kFullSize);
	info_box_.add_space(right_column_margin_);
	info_box_.add(load_or_save_.game_details(), UI::Box::Resizing::kFullSize);

	button_spacer_ = new UI::Panel(load_or_save_.game_details()->button_box(), 0, 0, 0, 0);
	load_or_save_.game_details()->button_box()->add(button_spacer_);

	layout();

	ok_.set_enabled(false);
	set_thinks(false);

	if (is_replay_) {
		back_.set_tooltip(_("Return to the main menu"));
		ok_.set_tooltip(_("Load this replay"));
	} else {
		back_.set_tooltip(gsp->settings().multiplayer ? _("Return to the multiplayer game setup") :
		                                                _("Return to the single player menu"));
		ok_.set_tooltip(_("Load this game"));
	}

	back_.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_back, boost::ref(*this)));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	load_or_save_.table().selected.connect(
	   boost::bind(&FullscreenMenuLoadGame::entry_selected, this));
	load_or_save_.table().double_clicked.connect(
	   boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));

	fill_table();
}

void FullscreenMenuLoadGame::layout() {
	FullscreenMenuLoadMapOrGame::layout();
	main_box_.set_size(get_w() - 2 * tablex_, tabley_ + tableh_ + padding_);
	main_box_.set_pos(Vector2i(tablex_, 0));
	title_.set_fontsize(fs_big());
	load_or_save_.delete_button()->set_desired_size(butw_, buth_);
	button_spacer_->set_desired_size(butw_, buth_ + 2 * padding_);
	load_or_save_.table().set_desired_size(tablew_, tableh_);
	load_or_save_.game_details()->set_max_size(
	   main_box_.get_w() - tablew_ - right_column_margin_, tableh_);
}

void FullscreenMenuLoadGame::think() {
	if (ctrl_) {
		ctrl_->think();
	}
}

void FullscreenMenuLoadGame::clicked_ok() {
	if (!load_or_save_.has_selection()) {
		return;
	}

	const SavegameData* gamedata = load_or_save_.entry_selected();
	if (gamedata && gamedata->errormessage.empty()) {
		filename_ = gamedata->filename;
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}
}

void FullscreenMenuLoadGame::entry_selected() {
	ok_.set_enabled(load_or_save_.table().selections().size() == 1);
	load_or_save_.delete_button()->set_enabled(load_or_save_.has_selection());
	if (load_or_save_.has_selection()) {
		load_or_save_.entry_selected();
	}
}

/**
 * Fill the file list
 */
void FullscreenMenuLoadGame::fill_table() {
	load_or_save_.fill_table();
}

bool FullscreenMenuLoadGame::handle_key(bool down, SDL_Keysym code) {
	if (!down)
		return false;

	switch (code.sym) {
	case SDLK_KP_PERIOD:
		if (code.mod & KMOD_NUM)
			break;
	/* no break */
	case SDLK_DELETE:
		load_or_save_.clicked_delete();
		return true;
	default:
		break;
	}

	return FullscreenMenuLoadMapOrGame::handle_key(down, code);
}
