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

#include "wui/load_or_save_game.h"

#include <ctime>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "game_io/game_preload_packet.h"
#include "graphic/font_handler1.h"
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
std::string
map_filename(const std::string& filename, const std::string& mapname, bool localize_autosave) {
	std::string result = FileSystem::filename_without_ext(filename.c_str());

	if (localize_autosave && boost::starts_with(result, "wl_autosave")) {
		std::vector<std::string> autosave_name;
		boost::split(autosave_name, result, boost::is_any_of("_"));
		if (autosave_name.empty() || autosave_name.size() < 3) {
			/** TRANSLATORS: %1% is a map's name. */
			result = (boost::format(_("Autosave: %1%")) % mapname).str();
		} else {
			/** TRANSLATORS: %1% is a number, %2% a map's name. */
			result = (boost::format(_("Autosave %1%: %2%")) % autosave_name.back() % mapname).str();
		}
	} else if (!(boost::starts_with(result, mapname))) {
		/** TRANSLATORS: %1% is a filename, %2% a map's name. */
		result = (boost::format(_("%1% (%2%)")) % result % mapname).str();
	}
	return result;
}
}  // namespace

LoadOrSaveGame::LoadOrSaveGame(UI::Panel* parent,
                               Widelands::Game& g,
                               FileType filetype,
                               GameDetails::Style style,
                               bool localize_autosave)
   : parent_(parent),
     table_(parent,
            0,
            0,
            0,
            0,
            g_gr->images().get(style == GameDetails::Style::kFsMenu ? "images/ui_basic/but3.png" :
                                                                      "images/ui_basic/but1.png"),
            UI::TableRows::kMultiDescending),
     filetype_(filetype),
     localize_autosave_(localize_autosave),
     // Savegame description
     game_details_(parent, style),
     delete_(new UI::Button(game_details()->button_box(),
                            "delete",
                            0,
                            0,
                            0,
                            0,
                            g_gr->images().get("images/ui_basic/but0.png"),
                            _("Delete"))),
     game_(g) {
	table_.add_column(130, _("Save Date"), _("The date this game was saved"), UI::Align::kLeft);
	if (filetype_ != FileType::kGameSinglePlayer) {
		std::vector<std::string> modes;
		if (filetype_ == FileType::kReplay) {
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
		   _("Mode"), (boost::format("%s %s") % mode_tooltip_1 % mode_tooltip_2).str());
	}
	table_.add_column(0, _("Description"),
	                  _("The filename that the game was saved under followed by the map’s name, "
	                    "or the map’s name followed by the last objective achieved."),
	                  UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.set_column_compare(
	   0, boost::bind(&LoadOrSaveGame::compare_date_descending, this, _1, _2));
	table_.set_sort_column(0);
	table_.focus();
	fill_table();

	game_details_.button_box()->add(delete_, style == GameDetails::Style::kFsMenu ?
	                                            UI::Box::Resizing::kAlign :
	                                            UI::Box::Resizing::kFullSize,
	                                UI::Align::kLeft);
	delete_->set_enabled(false);
	delete_->sigclicked.connect(boost::bind(&LoadOrSaveGame::clicked_delete, boost::ref(*this)));
}

const std::string LoadOrSaveGame::filename_list_string() const {
	std::set<uint32_t> selections = table_.selections();
	boost::format message;
	int counter = 0;
	for (const uint32_t index : selections) {
		++counter;
		// TODO(GunChleoc): We can exceed the texture size for the font renderer,
		// so we have to restrict this for now.
		if (counter > 50) {
			message = boost::format("%s\n%s") % message % "...";
			break;
		}
		const SavegameData& gamedata = games_data_[table_.get(table_.get_record(index))];

		if (gamedata.errormessage.empty()) {
			std::vector<std::string> listme;
			listme.push_back(richtext_escape(gamedata.mapname));
			listme.push_back(gamedata.savedonstring);
			message = (boost::format("%s\n%s") % message %
			           i18n::localize_list(listme, i18n::ConcatenateWith::COMMA));
		} else {
			message = boost::format("%s\n%s") % message % richtext_escape(gamedata.filename);
		}
	}
	return message.str();
}

bool LoadOrSaveGame::compare_date_descending(uint32_t rowa, uint32_t rowb) {
	const SavegameData& r1 = games_data_[table_[rowa]];
	const SavegameData& r2 = games_data_[table_[rowb]];

	return r1.savetimestamp < r2.savetimestamp;
}

const SavegameData* LoadOrSaveGame::entry_selected() {
	SavegameData* result = new SavegameData();
	size_t selections = table_.selections().size();
	if (selections == 1) {
		delete_->set_tooltip(
		   filetype_ == FileType::kReplay ?
		      /** TRANSLATORS: Tooltip for the delete button. The user has selected 1 file */
		      _("Delete this replay") :
		      /** TRANSLATORS: Tooltip for the delete button. The user has selected 1 file */
		      _("Delete this game"));
		result = &games_data_[table_.get_selected()];
	} else if (selections > 1) {
		delete_->set_tooltip(
		   filetype_ == FileType::kReplay ?
		      /** TRANSLATORS: Tooltip for the delete button. The user has selected multiple files */
		      _("Delete these replays") :
		      /** TRANSLATORS: Tooltip for the delete button. The user has selected multiple files */
		      _("Delete these games"));
		result->mapname =
		   (boost::format(ngettext("Selected %d file:", "Selected %d files:", selections)) %
		    selections)
		      .str();
		result->filename_list = filename_list_string();
	} else {
		delete_->set_tooltip("");
	}
	game_details_.update(*result);
	return result;
}

bool LoadOrSaveGame::has_selection() {
	return table_.has_selection();
}

void LoadOrSaveGame::clear_selections() {
	table_.clear_selections();
	game_details_.clear();
}

void LoadOrSaveGame::select_by_name(const std::string& name) {
	table_.clear_selections();
	for (size_t idx = 0; idx < table_.size(); ++idx) {
		const SavegameData& gamedata = games_data_[table_[idx]];
		if (name == gamedata.filename) {
			table_.select(idx);
			return;
		}
	}
}

const std::string LoadOrSaveGame::get_filename(int index) const {
	return games_data_[table_.get(table_.get_record(index))].filename;
}

void LoadOrSaveGame::clicked_delete() {
	if (!has_selection()) {
		return;
	}
	std::set<uint32_t> selections = table().selections();
	const SavegameData& gamedata = *entry_selected();
	size_t no_selections = selections.size();
	std::string header = "";
	if (filetype_ == FileType::kReplay) {
		header = no_selections == 1 ?
		            _("Do you really want to delete this replay?") :
		            /** TRANSLATORS: Used with multiple replays, 1 replay has a separate string. */
		            (boost::format(ngettext("Do you really want to delete this %d replay?",
		                                    "Do you really want to delete these %d replays?",
		                                    no_selections)) %
		             no_selections)
		               .str();
	} else {
		header = no_selections == 1 ?
		            _("Do you really want to delete this game?") :
		            /** TRANSLATORS: Used with multiple games, 1 game has a separate string. */
		            (boost::format(ngettext("Do you really want to delete this %d game?",
		                                    "Do you really want to delete these %d games?",
		                                    no_selections)) %
		             no_selections)
		               .str();
	}
	std::string message = no_selections > 1 ? gamedata.filename_list : gamedata.filename;
	message = (boost::format("%s\n%s") % header % message).str();

	bool do_delete = SDL_GetModState() & KMOD_CTRL;
	if (!do_delete) {
		UI::WLMessageBox confirmationBox(
			parent_, ngettext("Confirm deleting file", "Confirm deleting files", no_selections), message,
			UI::WLMessageBox::MBoxType::kOkCancel);
		do_delete = confirmationBox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk;
	}
	if (do_delete) {
		for (const uint32_t index : selections) {
			const std::string& deleteme = get_filename(index);
			g_fs->fs_unlink(deleteme);
			if (filetype_ == FileType::kReplay) {
				g_fs->fs_unlink(deleteme + WLGF_SUFFIX);
			}
		}
		fill_table();
	}
}

void LoadOrSaveGame::fill_table() {

	games_data_.clear();
	table_.clear();

	FilenameSet gamefiles;

	if (filetype_ == FileType::kReplay) {
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
		if (filetype_ == FileType::kReplay)
			savename += WLGF_SUFFIX;

		if (!g_fs->file_exists(savename.c_str())) {
			continue;
		}

		gamedata.filename = gamefilename;

		try {
			Widelands::GameLoader gl(savename.c_str(), game_);
			gl.preload_game(gpdp);

			gamedata.gametype = gpdp.get_gametype();

			if (filetype_ != FileType::kReplay) {
				if (filetype_ == FileType::kGame) {
					if (gamedata.gametype == GameController::GameType::REPLAY) {
						continue;
					}
				} else if (filetype_ == FileType::kGameMultiPlayer) {
					if (gamedata.gametype == GameController::GameType::SINGLEPLAYER) {
						continue;
					}
				} else if (gamedata.gametype > GameController::GameType::SINGLEPLAYER) {
					continue;
				}
			}

			gamedata.set_mapname(gpdp.get_mapname());
			gamedata.set_gametime(gpdp.get_gametime());
			gamedata.set_nrplayers(gpdp.get_number_of_players());
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

					gamedata.savedatestring =
					   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
					      hour:minute */
					   (boost::format(_("Today, %1%:%2%")) % savedate->tm_hour % minute).str();
					gamedata.savedonstring =
					   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
					      hour:minute. This is part of a list. */
					   (boost::format(_("saved today at %1%:%2%")) % savedate->tm_hour % minute).str();
				} else if ((savedate->tm_year == current_year && savedate->tm_mon == current_month &&
				            savedate->tm_mday == current_day - 1) ||
				           (savedate->tm_year == current_year - 1 && savedate->tm_mon == 11 &&
				            current_month == 0 && savedate->tm_mday == 31 &&
				            current_day == 1)) {  // Yesterday
					// Adding the 0 padding in a separate statement so translators won't have to deal
					// with it
					const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

					gamedata.savedatestring =
					   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
					      hour:minute */
					   (boost::format(_("Yesterday, %1%:%2%")) % savedate->tm_hour % minute).str();
					gamedata.savedonstring =
					   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
					      hour:minute. This is part of a list. */
					   (boost::format(_("saved yesterday at %1%:%2%")) % savedate->tm_hour % minute)
					      .str();
				} else {  // Older
					gamedata.savedatestring =
					   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
					      month day, year */
					   (boost::format(_("%2% %1%, %3%")) % savedate->tm_mday %
					    localize_month(savedate->tm_mon) % (1900 + savedate->tm_year))
					      .str();
					gamedata.savedonstring =
					   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
					      month day, year. This is part of a list. */
					   (boost::format(_("saved on %2% %1%, %3%")) % savedate->tm_mday %
					    localize_month(savedate->tm_mon) % (1900 + savedate->tm_year))
					      .str();
				}
			}

			gamedata.wincondition = gpdp.get_localized_win_condition();
			gamedata.minimap_path = gpdp.get_minimap_path();
			games_data_.push_back(gamedata);

			UI::Table<uintptr_t const>::EntryRecord& te = table_.add(games_data_.size() - 1);
			te.set_string(0, gamedata.savedatestring);

			if (filetype_ != FileType::kGameSinglePlayer) {
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
					gametypestring = (boost::format(_("H (%1%)")) % gamedata.nrplayers).str();
					break;
				case GameController::GameType::NETCLIENT:
					/** TRANSLATORS: "Multiplayer" entry in the Game Mode table column. */
					/** TRANSLATORS: "Keep this to 2 letters maximum. */
					/** TRANSLATORS: A tooltip will explain the abbreviation. */
					/** TRANSLATORS: Make sure that this translation is consistent with the tooltip. */
					/** TRANSLATORS: %1% is the number of players */
					gametypestring = (boost::format(_("MP (%1%)")) % gamedata.nrplayers).str();
					break;
				case GameController::GameType::REPLAY:
					gametypestring = "";
					break;
				}
				te.set_string(1, gametypestring);
				if (filetype_ == FileType::kReplay) {
					if (UI::g_fh1->fontset()->is_rtl()) {
						te.set_string(
						   2, (boost::format("%1% ← %2%") % gamedata.gametime % gamedata.mapname).str());
					} else {
						te.set_string(
						   2, (boost::format("%1% → %2%") % gamedata.gametime % gamedata.mapname).str());
					}
				} else {
					te.set_string(
					   2, map_filename(gamedata.filename, gamedata.mapname, localize_autosave_));
				}
			} else {
				te.set_string(1, map_filename(gamedata.filename, gamedata.mapname, localize_autosave_));
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

			gamedata.mapname = FileSystem::filename_without_ext(gamedata.filename.c_str());
			games_data_.push_back(gamedata);

			UI::Table<uintptr_t const>::EntryRecord& te = table_.add(games_data_.size() - 1);
			te.set_string(0, "");
			if (filetype_ != FileType::kGameSinglePlayer) {
				te.set_string(1, "");
				/** TRANSLATORS: Prefix for incompatible files in load game screens */
				te.set_string(2, (boost::format(_("Incompatible: %s")) % gamedata.mapname).str());
			} else {
				te.set_string(1, (boost::format(_("Incompatible: %s")) % gamedata.mapname).str());
			}
		}
	}
	table_.sort();
}
