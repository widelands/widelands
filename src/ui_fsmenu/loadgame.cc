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
#include "ui_basic/messagebox.h"

FullscreenMenuLoadGame::FullscreenMenuLoadGame(Widelands::Game& g,
                                               GameSettingsProvider* gsp,
                                               GameController* gc,
                                               bool is_replay)
   : FullscreenMenuLoadMapOrGame(),
     load_or_save_(this,
                   g,
                   tablex_,
                   tabley_,
                   tablew_,
                   tableh_,
                   padding_,
                   is_replay ? LoadOrSaveGame::FileType::kReplay : gsp->settings().multiplayer ?
                               LoadOrSaveGame::FileType::kGameMultiPlayer :
                               LoadOrSaveGame::FileType::kGameSinglePlayer,
                   GameDetails::Style::kFsMenu,
                   true),

     is_replay_(is_replay),
     // Main title
     title_(this,
            get_w() / 2,
            tabley_ / 3,
            is_replay_ ? _("Choose a replay") : _("Choose a saved game"),
            UI::Align::kCenter),

     delete_(this,
             "delete",
             right_column_x_,
             buty_ - buth_ - 2 * padding_,
             butw_,
             buth_,
             g_gr->images().get("images/ui_basic/but0.png"),
				 _("Delete")),
     game_(g),
     settings_(gsp),
     ctrl_(gc) {
	title_.set_fontsize(UI_FONT_SIZE_BIG);
	delete_.set_enabled(false);

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

void FullscreenMenuLoadGame::layout() {
	// TODO(GunChleoc): Implement when we have box layout for the details.
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
// NOCOM
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
	/* NOCOM
	std::set<uint32_t> selections = table_.selections();
	size_t no_selections = selections.size();
	std::string message;
	if (no_selections > 1) {
		if (is_replay_) {
			message = (boost::format(ngettext("Do you really want to delete this %d replay?",
														 "Do you really want to delete these %d replays?",
														 no_selections)) %
						  no_selections)
							 .str();
		} else {
			message = (boost::format(ngettext("Do you really want to delete this %d game?",
														 "Do you really want to delete these %d games?",
														 no_selections)) %
						  no_selections)
							 .str();
		}
		message = (boost::format("%s\n%s") % message % filename_list_string()).str();

	} else {
		const SavegameData& gamedata = *load_or_save_.entry_selected();

		message = (boost::format("%s %s\n") % _("Map Name:") % gamedata.mapname).str();

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
	}

	UI::WLMessageBox confirmationBox(
		this, ngettext("Confirm deleting file", "Confirm deleting files", no_selections), message,
		UI::WLMessageBox::MBoxType::kOkCancel);

	if (confirmationBox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		for (const uint32_t index : selections) {
			const std::string& deleteme = games_data_[table_.get(table_.get_record(index))].filename;
			g_fs->fs_unlink(deleteme);
			if (is_replay_) {
				g_fs->fs_unlink(deleteme + WLGF_SUFFIX);
			}
		}
		fill_table();
	}
	*/
}

bool LoadOrSaveGame::set_has_selection() {
	bool has_selection = table_.selections().size() < 2;
	// NOCOM ok_.set_enabled(has_selection);
	// NOCOM delete_.set_enabled(table_.has_selection());

	if (!has_selection) {
		/* NOCOM
		label_mapname_.set_text(std::string());
		label_gametime_.set_text(std::string());
		label_players_.set_text(std::string());
		label_version_.set_text(std::string());
		label_win_condition_.set_text(std::string());

		ta_mapname_.set_text(std::string());
		ta_gametime_.set_text(std::string());
		ta_players_.set_text(std::string());
		ta_version_.set_text(std::string());
		ta_win_condition_.set_text(std::string());
		minimap_icon_.set_icon(nullptr);
		minimap_icon_.set_visible(false);
		minimap_icon_.set_no_frame();
		minimap_image_.reset();
		*/
	} else {
		/* NOCOM
		label_mapname_.set_text(_("Map Name:"));

		label_gametime_.set_text(_("Gametime:"));
		label_players_.set_text(_("Players:"));
		label_win_condition_.set_text(_("Win Condition:"));
		*/
	}
	return has_selection;
}

void FullscreenMenuLoadGame::entry_selected() {
/* NOCOM
	bool has_selection = load_or_save_.has_selection();
	ok_.set_enabled(has_selection);
	delete_.set_enabled(has_selection);

	if (has_selection) {
		const SavegameData& gamedata = *load_or_save_.entry_selected();
		ok_.set_enabled(gamedata.errormessage.empty());
	}
*/
	/* NOCOM
	size_t selections = table_.selections().size();
	if (set_has_selection()) {

		const SavegameData& gamedata = games_data_[table_.get_selected()];
		descr_.set_text(gamedata.errormessage);

		if (gamedata.errormessage.empty()) {
			descr_.set_visible(false);
			ta_mapname_.set_text(gamedata.mapname);
			ta_gametime_.set_text(gametimestring(gamedata.gametime));

			uint8_t number_of_players = gamedata.nrplayers;
			if (number_of_players > 0) {
				ta_players_.set_text(
					(boost::format("%u") % static_cast<unsigned int>(number_of_players)).str());
			} else {
				label_players_.set_text("");
				ta_players_.set_text("");
			}

			if (gamedata.version.empty()) {
				label_version_.set_text("");
				ta_version_.set_text("");
			} else {
				label_version_.set_text(_("Widelands Version:"));
				ta_version_.set_text(gamedata.version);
			}

			{
				i18n::Textdomain td("win_conditions");
				ta_win_condition_.set_text(_(gamedata.wincondition));
			}

			std::string minimap_path = gamedata.minimap_path;
			// Delete former image
			minimap_icon_.set_icon(nullptr);
			minimap_icon_.set_visible(false);
			minimap_icon_.set_no_frame();
			minimap_image_.reset();
			// Load the new one
			if (!minimap_path.empty()) {
				try {
					// Load the image
					minimap_image_ = load_image(
						minimap_path,
						std::unique_ptr<FileSystem>(g_fs->make_sub_file_system(gamedata.filename)).get());

					// Scale it
					double scale = double(minimap_w_) / minimap_image_->width();
					double scaleY = double(minimap_h_) / minimap_image_->height();
					if (scaleY < scale) {
						scale = scaleY;
					}
					if (scale > 1.0)
						scale = 1.0;  // Don't make the image too big; fuzziness will result
					uint16_t w = scale * minimap_image_->width();
					uint16_t h = scale * minimap_image_->height();

					// Center the minimap in the available space
					int32_t xpos =
						right_column_x_ + (get_w() - right_column_margin_ - w - right_column_x_) / 2;
					int32_t ypos = minimap_y_;

					// Set small minimaps higher up for a more harmonious look
					if (h < minimap_h_ * 2 / 3) {
						ypos += (minimap_h_ - h) / 3;
					} else {
						ypos += (minimap_h_ - h) / 2;
					}

					minimap_icon_.set_size(w, h);
					minimap_icon_.set_pos(Vector2i(xpos, ypos));
					minimap_icon_.set_frame(UI_FONT_CLR_FG);
					minimap_icon_.set_visible(true);
					minimap_icon_.set_icon(minimap_image_.get());
				} catch (const std::exception& e) {
					log("Failed to load the minimap image : %s\n", e.what());
				}
			}
		} else {
			label_mapname_.set_text(_("Filename:"));
			ta_mapname_.set_text(gamedata.mapname);
			label_gametime_.set_text("");
			ta_gametime_.set_text("");
			label_players_.set_text("");
			ta_players_.set_text("");
			label_version_.set_text("");
			ta_version_.set_text("");
			label_win_condition_.set_text("");
			ta_win_condition_.set_text("");

			minimap_icon_.set_icon(nullptr);
			minimap_icon_.set_visible(false);
			minimap_icon_.set_no_frame();
			minimap_image_.reset();

			descr_.set_visible(true);
			ok_.set_enabled(false);
		}
	} else if (selections > 1) {
		label_mapname_.set_text(
			(boost::format(ngettext("Selected %d file:", "Selected %d files:", selections)) %
			 selections)
				.str());
		descr_.set_visible(true);
		descr_.set_text(filename_list_string());
	}
	*/
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
