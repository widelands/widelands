#include "savegameloader.h"

#include "base/i18n.h"
#include "base/time_string.h"
#include "game_io/game_loader.h"
#include "io/filesystem/layered_filesystem.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

SavegameLoader::SavegameLoader(Widelands::Game& game) : game_(game) {
}
std::vector<SavegameData> SavegameLoader::load_files(const std::string directory) {
	FilenameSet gamefiles = g_fs->list_directory(directory);

	for (const std::string& gamefilename : gamefiles) {
		load_gamefile(gamefilename);
	}
}

void SavegameLoader::load_gamefile(const std::string& gamefilename) {

	Widelands::GamePreloadPacket gpdp;
	std::string savename = gamefilename;

	//	if (filetype_ == FileType::kReplay) {
	//		savename += kSavegameExtension;
	//	}
	if (!g_fs->file_exists(savename.c_str())) {
		return;
	}

	SavegameData gamedata(gamefilename);
	try {
		Widelands::GameLoader gl(savename.c_str(), game_);
		gl.preload_game(gpdp);

		gamedata.gametype = gpdp.get_gametype();
		if (!is_valid_gametype(gamedata)) {
			return;
		}
		add_general_information(gamedata, gpdp);
		add_time_info(gamedata, gpdp);

	} catch (const std::exception& e) {

		if (g_fs->is_directory(gamefilename)) {
			add_sub_dir(gamefilename);
			return;
		} else {
			add_error_info(gamedata, e.what());
		}
	}

	// games_data_.push_back(gamedata);
}

void SavegameLoader::add_general_information(SavegameData& gamedata,
                                             const Widelands::GamePreloadPacket& gpdp) const {
	gamedata.set_mapname(gpdp.get_mapname());
	gamedata.set_gametime(gpdp.get_gametime());
	gamedata.set_nrplayers(gpdp.get_number_of_players());
	gamedata.version = gpdp.get_version();
	gamedata.wincondition = gpdp.get_localized_win_condition();
	gamedata.minimap_path = gpdp.get_minimap_path();
}

void SavegameLoader::add_error_info(SavegameData& gamedata, std::string errormessage) const {
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
}

bool SavegameLoader::is_valid_gametype(const SavegameData& gamedata) const {
	// Skip singleplayer games in multiplayer mode and vice versa
	//	if (filetype_ != FileType::kReplay && filetype_ != FileType::kShowAll) {
	//		if (filetype_ == FileType::kGameMultiPlayer) {
	//			if (gamedata.gametype == GameController::GameType::kSingleplayer) {
	//				return false;
	//			}
	//		} else if ((gamedata.gametype != GameController::GameType::kSingleplayer) &&
	//		           (gamedata.gametype != GameController::GameType::kReplay)) {
	//			return false;
	//		}
	//	}
	return true;
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
			const std::string minute = (boost::format("%02u") % savedate->tm_min).str();

			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute */
			   (boost::format(_("Today, %1%:%2%")) % savedate->tm_hour % minute).str();
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    hour:minute. This is part of a list.
			    */
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
			                                                    hour:minute. This is part of a list.
			    */
			   (boost::format(_("saved yesterday at %1%:%2%")) % savedate->tm_hour % minute).str();
		} else {  // Older
			gamedata.savedatestring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    month day, year */
			   (boost::format(_("%1% %2%, %3%")) % localize_month(savedate->tm_mon) %
			    savedate->tm_mday % (1900 + savedate->tm_year))
			      .str();
			gamedata.savedonstring =
			   /** TRANSLATORS: Display date for choosing a savegame/replay. Placeholders are:
			                                                    month (short name) day (number),
			      year (number). This is part of a list. */
			   (boost::format(_("saved on %1% %2%, %3%")) % savedate->tm_mday %
			    localize_month(savedate->tm_mon) % (1900 + savedate->tm_year))
			      .str();
		}
	}
}

void SavegameLoader::add_sub_dir(const std::string& gamefilename) {
	// Add subdirectory to the list
	const char* fs_filename = FileSystem::fs_filename(gamefilename.c_str());
	if (!strcmp(fs_filename, ".") || !strcmp(fs_filename, "..")) {
		return;
	}
	// games_data_.push_back(SavegameData::create_sub_dir(gamefilename));
}
