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

namespace {
// This function concatenates the filename and localized map name for a savegame/replay.
// If the filename starts with the map name, the map name is omitted.
// It also prefixes autosave files with a numbered and localized "Autosave" prefix.
std::string map_filename(const std::string& filename, const std::string& mapname) {
	std::string result = FileSystem::filename_without_ext(filename.c_str());
	std::string mapname_localized;
	{
		i18n::Textdomain td("maps");
		mapname_localized = _(mapname);
	}

	if (boost::starts_with(result, "wl_autosave")) {
		std::vector<std::string> autosave_name;
		boost::split(autosave_name, result, boost::is_any_of("_"));
		if (autosave_name.empty() || autosave_name.size() < 3) {
			/** TRANSLATORS: %1% is a map's name. */
			result = (boost::format(_("Autosave: %1%")) % mapname_localized).str();
		} else {
			/** TRANSLATORS: %1% is a number, %2% a map's name. */
			result = (boost::format(_("Autosave %1%: %2%")) % autosave_name.back() % mapname_localized)
			            .str();
		}
	} else if (!(boost::starts_with(result, mapname) ||
	             boost::starts_with(result, mapname_localized))) {
		/** TRANSLATORS: %1% is a filename, %2% a map's name. */
		result = (boost::format(_("%1% (%2%)")) % result % mapname_localized).str();
	}
	return result;
}

}  // namespace

FullscreenMenuLoadGame::FullscreenMenuLoadGame(Widelands::Game& g,
                                               GameSettingsProvider* gsp,
                                               GameController* gc,
                                               bool is_replay)
   : FullscreenMenuLoadMapOrGame(),
     table_(this, tablex_, tabley_, tablew_, tableh_, true),

     is_replay_(is_replay),
     // Main title
     title_(this,
            get_w() / 2,
            tabley_ / 3,
            is_replay_ ? _("Choose a replay") : _("Choose a saved game"),
            UI::Align::kHCenter),

     // Savegame description
     game_details_(this,
                   right_column_x_,
                   tabley_,
                   get_right_column_w(right_column_x_),
                   tableh_ - buth_ - 4 * padding_,
                   GameDetails::Style::kFsMenu),

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
	table_.add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align::kLeft);
	int used_width = 130;
	if (is_replay_ || settings_->settings().multiplayer) {
		std::vector<std::string> modes;
		if (is_replay_) {
			/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
			/** TRANSLATORS: Make sure that you keep consistency in your translation. */
			modes.push_back(_("SP = Single Player"));
		}
		/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
		/** TRANSLATORS: Make sure that you keep consistency in your translation. */
		modes.push_back(_("MP = Multiplayer"));
		/** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
		/** TRANSLATORS: Make sure that you keep consistency in your translation. */
		modes.push_back(_("H = Multiplayer (Host)"));
		const std::string mode_tooltip_1 =
		   /** TRANSLATORS: Tooltip for the "Mode" column when choosing a game/replay to load. */
		   /** TRANSLATORS: %s is a list of game modes. */
		   ((boost::format(_("Game Mode: %s.")) %
		     i18n::localize_list(modes, i18n::ConcatenateWith::COMMA)))
		      .str();
		const std::string mode_tooltip_2 = _("Numbers are the number of players.");

		table_.add_column(
		   65,
		   /** TRANSLATORS: Game Mode table column when choosing a game/replay to load. */
		   /** TRANSLATORS: Keep this to 5 letters maximum. */
		   /** TRANSLATORS: A tooltip will explain if you need to use an abbreviation. */
		   _("Mode"), (boost::format("%s %s") % mode_tooltip_1 % mode_tooltip_2).str(),
		   UI::Align::kLeft);
		used_width += 65;
	}
	table_.add_column(table_.get_w() - used_width, _("Description"),
	                  _("The filename that the game was saved under followed by the map’s name, "
	                    "or the map’s name followed by the last objective achieved."),
	                  UI::Align::kLeft);
	table_.set_column_compare(
	   0, boost::bind(&FullscreenMenuLoadGame::compare_date_descending, this, _1, _2));
	table_.selected.connect(boost::bind(&FullscreenMenuLoadGame::entry_selected, this));
	table_.double_clicked.connect(
	   boost::bind(&FullscreenMenuLoadGame::clicked_ok, boost::ref(*this)));
	table_.set_sort_column(0);
	table_.focus();
	fill_table();
}

void FullscreenMenuLoadGame::think() {
	if (ctrl_) {
		ctrl_->think();
	}
}

// Reverse default sort order for save date column
bool FullscreenMenuLoadGame::compare_date_descending(uint32_t rowa, uint32_t rowb) {
	const SavegameData& r1 = games_data_[table_[rowa]];
	const SavegameData& r2 = games_data_[table_[rowb]];

	return r1.savetimestamp < r2.savetimestamp;
}

void FullscreenMenuLoadGame::clicked_ok() {
	const SavegameData& gamedata = games_data_[table_.get_selected()];
	if (gamedata.errormessage.empty()) {
		filename_ = gamedata.filename;
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}
}

void FullscreenMenuLoadGame::clicked_delete() {
	if (!table_.has_selection()) {
		return;
	}
	const SavegameData& gamedata = games_data_[table_.get_selected()];

	std::string message = (boost::format("%s %s\n") % _("Map:") % gamedata.mapname).str();

	message =
	   (boost::format("%s %s %s\n") % message % _("Win Condition:") % gamedata.wincondition).str();

	message =
	   (boost::format("%s %s %s\n") % message % _("Save Date:") % gamedata.savedatestring).str();

	message =
	   (boost::format("%s %s %s\n") % message % _("Gametime:") % gametimestring(gamedata.gametime))
	      .str();

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
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);
	delete_.set_enabled(has_selection);

	if (has_selection) {
		const SavegameData& gamedata = games_data_[table_.get_selected()];
		game_details_.update(gamedata);
		ok_.set_enabled(gamedata.errormessage.empty());
	} else {
		game_details_.clear();
	}
}

/**
 * Fill the file list
 */
void FullscreenMenuLoadGame::fill_table() {

	games_data_.clear();
	table_.clear();

	FilenameSet gamefiles;

	if (is_replay_) {
		gamefiles = filter(g_fs->list_directory(REPLAY_DIR),
		                   [](const std::string& fn) { return boost::ends_with(fn, REPLAY_SUFFIX); });
	} else {
		gamefiles = g_fs->list_directory("save");
	}

	Widelands::GamePreloadPacket gpdp;

	for (const std::string& gamefilename : gamefiles) {
		if (gamefilename == "save/campvis" || gamefilename == "save\\campvis") {
			continue;
		}

		SavegameData gamedata;

		std::string savename = gamefilename;
		if (is_replay_)
			savename += WLGF_SUFFIX;

		if (!g_fs->file_exists(savename.c_str())) {
			continue;
		}

		gamedata.filename = gamefilename;

		try {
			Widelands::GameLoader gl(savename.c_str(), game_);
			gl.preload_game(gpdp);

			gamedata.gametype = gpdp.get_gametype();

			if (!is_replay_) {
				if (settings_->settings().multiplayer) {
					if (gamedata.gametype == GameController::GameType::SINGLEPLAYER) {
						continue;
					}
				} else if (gamedata.gametype > GameController::GameType::SINGLEPLAYER) {
					continue;
				}
			}

			{
				i18n::Textdomain td("maps");
				gamedata.mapname = _(gpdp.get_mapname());
			}
			gamedata.gametime = gpdp.get_gametime();
			gamedata.nrplayers = gpdp.get_number_of_players();
			gamedata.version = gpdp.get_version();

			gamedata.savetimestamp = gpdp.get_savetimestamp();
			time_t t;
			time(&t);
			struct tm* currenttime = localtime(&t);
			// We need to put these into variables because of a sideeffect of the localtime function.
			int8_t current_year = currenttime->tm_year;
			int8_t current_month = currenttime->tm_mon;
			int8_t current_day = currenttime->tm_mday;

			struct tm* savedate = localtime(&gamedata.savetimestamp);

			if (gamedata.savetimestamp > 0) {
				if (savedate->tm_year == current_year && savedate->tm_mon == current_month &&
				    savedate->tm_mday == current_day) {  // Today

					// Adding the 0 padding in a separate statement so translators won't have to deal
					// with it
					const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

					/** TRANSLATORS: Display date for choosing a savegame/replay */
					/** TRANSLATORS: hour:minute */
					gamedata.savedatestring =
					   (boost::format(_("Today, %1%:%2%")) % savedate->tm_hour % minute).str();
				} else if ((savedate->tm_year == current_year && savedate->tm_mon == current_month &&
				            savedate->tm_mday == current_day - 1) ||
				           (savedate->tm_year == current_year - 1 && savedate->tm_mon == 11 &&
				            current_month == 0 && savedate->tm_mday == 31 &&
				            current_day == 1)) {  // Yesterday
					// Adding the 0 padding in a separate statement so translators won't have to deal
					// with it
					const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

					/** TRANSLATORS: Display date for choosing a savegame/replay */
					/** TRANSLATORS: hour:minute */
					gamedata.savedatestring =
					   (boost::format(_("Yesterday, %1%:%2%")) % savedate->tm_hour % minute).str();
				} else {  // Older

					/** TRANSLATORS: Display date for choosing a savegame/replay */
					/** TRANSLATORS: month day, year */
					gamedata.savedatestring =
					   (boost::format(_("%2% %1%, %3%")) % savedate->tm_mday %
					    localize_month(savedate->tm_mon) % (1900 + savedate->tm_year))
					      .str();
				}
			}

			// Win condition localization can come from the 'widelands' or 'win_conditions' textdomain.
			gamedata.wincondition = _(gpdp.get_localized_win_condition());
			{
				i18n::Textdomain td("win_conditions");
				gamedata.wincondition = _(gamedata.wincondition);
			}
			gamedata.minimap_path = gpdp.get_minimap_path();
			games_data_.push_back(gamedata);

			UI::Table<uintptr_t const>::EntryRecord& te = table_.add(games_data_.size() - 1);
			te.set_string(0, gamedata.savedatestring);

			if (is_replay_ || settings_->settings().multiplayer) {
				std::string gametypestring;
				switch (gamedata.gametype) {
				case GameController::GameType::SINGLEPLAYER:
					/** TRANSLATORS: "Single Player" entry in the Game Mode table column. */
					/** TRANSLATORS: "Keep this to 6 letters maximum. */
					/** TRANSLATORS: A tooltip will explain the abbreviation. */
					/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
					gametypestring = _("SP");
					break;
				case GameController::GameType::NETHOST:
					/** TRANSLATORS: "Multiplayer Host" entry in the Game Mode table column. */
					/** TRANSLATORS: "Keep this to 2 letters maximum. */
					/** TRANSLATORS: A tooltip will explain the abbreviation. */
					/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
					/** TRANSLATORS: %1% is the number of players */
					gametypestring =
					   (boost::format(_("H (%1%)")) % static_cast<unsigned int>(gamedata.nrplayers))
					      .str();
					break;
				case GameController::GameType::NETCLIENT:
					/** TRANSLATORS: "Multiplayer" entry in the Game Mode table column. */
					/** TRANSLATORS: "Keep this to 2 letters maximum. */
					/** TRANSLATORS: A tooltip will explain the abbreviation. */
					/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
					/** TRANSLATORS: %1% is the number of players */
					gametypestring =
					   (boost::format(_("MP (%1%)")) % static_cast<unsigned int>(gamedata.nrplayers))
					      .str();
					break;
				case GameController::GameType::REPLAY:
					gametypestring = "";
					break;
				}
				te.set_string(1, gametypestring);
				te.set_string(2, map_filename(gamedata.filename, gamedata.mapname));
			} else {
				te.set_string(1, map_filename(gamedata.filename, gamedata.mapname));
			}
		} catch (const WException& e) {
			std::string errormessage = e.what();
			boost::replace_all(errormessage, "\n", "<br>");
			gamedata.errormessage =
			   ((boost::format("<p>%s</p><p>%s</p><p>%s</p>"))
			    /** TRANSLATORS: Error message introduction for when an old savegame can't be loaded */
			    % _("This file has the wrong format and can’t be loaded."
			        " Maybe it was created with an older version of Widelands.")
			    /** TRANSLATORS: This text is on a separate line with an error message below */
			    % _("Error message:") % errormessage)
			      .str();

			const std::string fs_filename =
			   FileSystem::filename_without_ext(gamedata.filename.c_str());
			gamedata.mapname = fs_filename;
			games_data_.push_back(gamedata);

			UI::Table<uintptr_t const>::EntryRecord& te = table_.add(games_data_.size() - 1);
			te.set_string(0, "");
			if (is_replay_ || settings_->settings().multiplayer) {
				te.set_string(1, "");
				/** TRANSLATORS: Prefix for incompatible files in load game screens */
				te.set_string(2, (boost::format(_("Incompatible: %s")) % fs_filename).str());
			} else {
				te.set_string(1, (boost::format(_("Incompatible: %s")) % fs_filename).str());
			}
		}
	}
	table_.sort();

	if (table_.size()) {
		table_.select(0);
	}
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
