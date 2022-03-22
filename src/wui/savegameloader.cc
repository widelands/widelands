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

#include "wui/savegameloader.h"

#include "base/i18n.h"
#include "base/string.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"

SavegameLoader::SavegameLoader(Widelands::Game& game) : game_(game) {
}
std::vector<SavegameData> SavegameLoader::load_files(const std::string& directory) {
	std::vector<SavegameData> loaded_games;
	FilenameSet gamefiles = g_fs->list_directory(directory);

	for (const std::string& gamefilename : gamefiles) {
		load(gamefilename, loaded_games);
	}
	return loaded_games;
}

std::string SavegameLoader::get_savename(const std::string& gamefilename) const {
	return gamefilename;
}

void SavegameLoader::load(const std::string& to_be_loaded,
                          std::vector<SavegameData>& loaded_games) const {
	if (g_fs->is_directory(to_be_loaded)) {
		try {
			load_savegame_from_directory(to_be_loaded, loaded_games);
		} catch (const std::exception&) {
			// loading failed, so this is actually a normal directory
			add_sub_dir(to_be_loaded, loaded_games);
		}
	} else {
		load_savegame_from_file(to_be_loaded, loaded_games);
	}
}

void SavegameLoader::load_savegame_from_directory(const std::string& gamefilename,
                                                  std::vector<SavegameData>& loaded_games) const {

	Widelands::GamePreloadPacket gpdp;
	SavegameData gamedata(gamefilename);

	Widelands::GameLoader gl(gamefilename, game_);
	gl.preload_game(gpdp);
	gamedata.gametype = gpdp.get_gametype();
	if (!is_valid_gametype(gamedata)) {
		return;
	}

	add_general_information(gamedata, gpdp);
	add_time_info(gamedata, gpdp);
	loaded_games.push_back(gamedata);
}

void SavegameLoader::load_savegame_from_file(const std::string& gamefilename,
                                             std::vector<SavegameData>& loaded_games) const {
	std::string savename = get_savename(gamefilename);

	if (!g_fs->file_exists(savename) || !ends_with(savename, kSavegameExtension)) {
		return;
	}

	Widelands::GamePreloadPacket gpdp;
	SavegameData gamedata(gamefilename);
	try {
		Widelands::GameLoader gl(savename, game_);
		gl.preload_game(gpdp);
		gamedata.gametype = gpdp.get_gametype();
		if (!is_valid_gametype(gamedata)) {
			return;
		}

		add_general_information(gamedata, gpdp);
		add_time_info(gamedata, gpdp);

	} catch (const std::exception& e) {
		add_error_info(gamedata, e.what());
	}

	loaded_games.push_back(gamedata);
}

void SavegameLoader::add_general_information(SavegameData& gamedata,
                                             const Widelands::GamePreloadPacket& gpdp) const {
	gamedata.set_mapname(gpdp.get_mapname());
	gamedata.set_gametime(gpdp.get_gametime().get());
	gamedata.set_nrplayers(gpdp.get_number_of_players());
	gamedata.version = gpdp.get_version();
	gamedata.wincondition = gpdp.get_localized_win_condition();
	gamedata.minimap_path = gpdp.get_minimap_path();
	gamedata.required_addons = gpdp.required_addons();
}

void SavegameLoader::add_error_info(SavegameData& gamedata, std::string errormessage) const {
	replace_all(errormessage, "\n", "<br>");
	gamedata.errormessage =
	   format("<p>%s</p><p>%s</p><p>%s</p>",
	          /** TRANSLATORS: Error message introduction for when an old savegame can't be loaded */
	          _("This file has the wrong format and can’t be loaded."
	            " Maybe it was created with an older version of Widelands."),
	          /** TRANSLATORS: This text is on a separate line with an error message below */
	          _("Error message:"), errormessage);

	gamedata.mapname = FileSystem::filename_without_ext(gamedata.filename.c_str());
}

void SavegameLoader::add_time_info(SavegameData& gamedata,
                                   const Widelands::GamePreloadPacket& gpdp) const {
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
			const std::string minute = format("%02u", savedate->tm_min);

			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute */
			   format(_("Today, %1%:%2%"), savedate->tm_hour, minute);
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute. This is part of a list.
			    */
			   format(_("saved today at %1%:%2%"), savedate->tm_hour, minute);
		} else if ((savedate->tm_year == current_year && savedate->tm_mon == current_month &&
		            savedate->tm_mday == current_day - 1) ||
		           (savedate->tm_year == current_year - 1 && savedate->tm_mon == 11 &&
		            current_month == 0 && savedate->tm_mday == 31 &&
		            current_day == 1)) {  // Yesterday
			// Adding the 0 padding in a separate statement so translators won't have to deal
			// with it
			const std::string minute = format("%02u", savedate->tm_min);

			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute */
			   format(_("Yesterday, %1%:%2%"), savedate->tm_hour, minute);
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute. This is part of a list.
			    */
			   format(_("saved yesterday at %1%:%2%"), savedate->tm_hour, minute);
		} else {  // Older
			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    month day, year */
			   format(_("%1% %2%, %3%"), localize_month(savedate->tm_mon), savedate->tm_mday,
			          (1900 + savedate->tm_year));
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    month (short name) day (number),
			      year (number). This is part of a list. */
			   format(_("saved on %1$s %2$u, %3$u"), localize_month(savedate->tm_mon),
			          savedate->tm_mday, (1900 + savedate->tm_year));
		}
	}
}

void SavegameLoader::add_sub_dir(const std::string& gamefilename,
                                 std::vector<SavegameData>& loaded_games) const {
	// Add subdirectory to the list
	const char* fs_filename = FileSystem::fs_filename(gamefilename.c_str());
	if ((strcmp(fs_filename, ".") == 0) || (strcmp(fs_filename, "..") == 0)) {
		return;
	}
	loaded_games.push_back(SavegameData::create_sub_dir(gamefilename));
}

ReplayLoader::ReplayLoader(Widelands::Game& game) : SavegameLoader(game) {
}

bool ReplayLoader::is_valid_gametype(const SavegameData& /*gamedata*/) const {
	return true;  // TODO(jmoerschbach): why?? what is the purpose of
	              // GameController::GameType::kReplay? return gamedata.is_replay(); <-- should be
	              // this, right?!
}

std::string ReplayLoader::get_savename(const std::string& gamefilename) const {
	std::string savename = gamefilename;
	savename += kSavegameExtension;
	return savename;
}

MultiPlayerLoader::MultiPlayerLoader(Widelands::Game& game) : SavegameLoader(game) {
}

bool MultiPlayerLoader::is_valid_gametype(const SavegameData& gamedata) const {
	// TODO(jmoerschbach): workaround to be able to load replays in multiplayer loading screen
	return gamedata.is_multiplayer() || gamedata.is_replay();
}

SinglePlayerLoader::SinglePlayerLoader(Widelands::Game& game) : SavegameLoader(game) {
}

bool SinglePlayerLoader::is_valid_gametype(const SavegameData& gamedata) const {
	return gamedata.is_singleplayer();
}

EverythingLoader::EverythingLoader(Widelands::Game& game) : SavegameLoader(game) {
}

bool EverythingLoader::is_valid_gametype(const SavegameData& /*gamedata*/) const {
	return true;
}
