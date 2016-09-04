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
#include "ui_basic/messagebox.h"

FullscreenMenuLoadGame::FullscreenMenuLoadGame(Widelands::Game& g,
                                               GameSettingsProvider* gsp,
                                               GameController* gc,
                                               bool is_replay)
   : FullscreenMenuLoadMapOrGame(),
	  load_or_save_(this, g, tablex_, tabley_, tablew_, tableh_, padding_,
						 is_replay? LoadOrSaveGame::FileType::kReplay :
										gsp->settings().multiplayer ? LoadOrSaveGame::FileType::kGameMultiPlayer : LoadOrSaveGame::FileType::kGameSinglePlayer,
						 true),

     is_replay_(is_replay),
     // Main title
     title_(this,
            get_w() / 2,
            tabley_ / 3,
            is_replay_ ? _("Choose a replay") : _("Choose a saved game"),
            UI::Align::kHCenter),

     delete_(this,
             "delete",
             right_column_x_,
             buty_ - buth_ - 2 * padding_,
             butw_,
             buth_,
             g_gr->images().get("images/ui_basic/but0.png"),
             _("Delete"),
             std::string(),
             false,
             false),
     game_(g),
     settings_(gsp),
     ctrl_(gc) {
	title_.set_fontsize(UI_FONT_SIZE_BIG);

	if (is_replay_) {
		back_.set_tooltip(_("Return to the main menu"));
		ok_.set_tooltip(_("Load this replay"));
		delete_.set_tooltip(_("Delete this replay"));
	} else {
		back_.set_tooltip(_("Return to the single player menu"));
		ok_.set_tooltip(_("Load this game"));
		delete_.set_tooltip(_("Delete this game"));
	}

	back_.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_back, boost::ref(*this)));
	ok_.sigclicked.connect(boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	delete_.sigclicked.connect(
	   boost::bind(&FullscreenMenuLoadGame::clicked_delete, boost::ref(*this)));
	load_or_save_.table().selected.connect(
	   boost::bind(&FullscreenMenuLoadGame::entry_selected, this));
	load_or_save_.table().double_clicked.connect(
	   boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));

	fill_table();
}

void FullscreenMenuLoadGame::think() {
	if (ctrl_) {
		ctrl_->think();
	}
}

void FullscreenMenuLoadGame::clicked_ok() {
	const SavegameData* gamedata = load_or_save_.entry_selected();
	if (gamedata && gamedata->errormessage.empty()) {
		filename_ = gamedata->filename;
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}
}

void FullscreenMenuLoadGame::clicked_delete() {
	if (!load_or_save_.has_selection()) {
		return;
	}
	const SavegameData& gamedata = *load_or_save_.entry_selected();

	std::string message = (boost::format("%s %s\n") % _("Map Name:") % gamedata.mapname).str();

	message =
	   (boost::format("%s %s %s\n") % message % _("Win Condition:") % gamedata.wincondition).str();

	message =
	   (boost::format("%s %s %s\n") % message % _("Save Date:") % gamedata.savedatestring).str();

	message = (boost::format("%s %s %s\n") % message % _("Game Time:") % gamedata.gametime).str();

	message = (boost::format("%s %s %s\n\n") % message % _("Players:") % gamedata.nrplayers).str();

	message = (boost::format("%s %s %s\n") % message % _("Filename:") % gamedata.filename).str();

	if (is_replay_) {
		message =
		   (boost::format("%s\n\n%s") % _("Do you really want to delete this replay?") % message)
		      .str();
	} else {
		message =
		   (boost::format("%s\n\n%s") % _("Do you really want to delete this game?") % message).str();
	}

	UI::WLMessageBox confirmationBox(
	   this, _("Confirm deleting file"), message, UI::WLMessageBox::MBoxType::kOkCancel);
	if (confirmationBox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		g_fs->fs_unlink(gamedata.filename);
		if (is_replay_) {
			g_fs->fs_unlink(gamedata.filename + WLGF_SUFFIX);
		}
		fill_table();
	}
}

void FullscreenMenuLoadGame::entry_selected() {
	bool has_selection = load_or_save_.has_selection();
	ok_.set_enabled(has_selection);
	delete_.set_enabled(has_selection);

	if (has_selection) {
		const SavegameData& gamedata = *load_or_save_.entry_selected();
		ok_.set_enabled(gamedata.errormessage.empty());
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
		clicked_delete();
		return true;
	default:
		break;
	}

	return FullscreenMenuLoadMapOrGame::handle_key(down, code);
}
