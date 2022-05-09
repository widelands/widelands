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

#include "logic/save_handler.h"

#include <SDL_timer.h>

#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/string.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "game_io/game_saver.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/generic_save_handler.h"
#include "wlapplication_options.h"
#include "wui/interactive_base.h"

SaveHandler::SaveHandler()
   : next_save_realtime_(0),
     last_save_realtime_(0),
     initialized_(false),
     allow_saving_(true),
     save_requested_(false),
     saving_next_tick_(false),
     autosave_filename_(kAutosavePrefix),
     fs_type_(FileSystem::ZIP),
     autosave_interval_in_ms_(kDefaultAutosaveInterval * 60 * 1000),
     number_of_rolls_(5) {
}

bool SaveHandler::roll_save_files(const std::string& filename, std::string* const error) const {
	int32_t rolls = 0;
	std::string filename_previous;

	// Only roll the smallest necessary number of files.
	while (rolls < number_of_rolls_) {
		filename_previous = create_file_name(kSaveDir, format("%s_%02d", filename, rolls));
		if (!g_fs->file_exists(filename_previous)) {
			break;
		}
		rolls++;
	}

	// If there is a file missing in the sequence; no need to delete any file.
	if (rolls < number_of_rolls_) {
		verb_log_info("Autosave: Rolling savefiles (count): %d of %d\n", rolls, number_of_rolls_);
	} else {
		verb_log_info("Autosave: Rolling savefiles (count): %d\n", rolls);
		rolls--;
		filename_previous = create_file_name(kSaveDir, format("%s_%02d", filename, rolls));
		if (rolls > 0) {
			try {
				g_fs->fs_unlink(filename_previous);  // Delete last of the rolling files
				verb_log_info("Autosave: Deleted %s\n", filename_previous.c_str());
			} catch (const FileError& e) {
				log_warn(
				   "Autosave: Unable to delete file %s: %s\n", filename_previous.c_str(), e.what());
				if (error != nullptr) {
					*error = format(
					   "Autosave: Unable to delete file %s: %s\n", filename_previous.c_str(), e.what());
				}
				return false;
			}
		}
	}

	rolls--;
	while (rolls >= 0) {
		const std::string filename_next =
		   create_file_name(kSaveDir, format("%s_%02d", filename, rolls));
		try {
			g_fs->fs_rename(
			   filename_next, filename_previous);  // e.g. wl_autosave_08 -> wl_autosave_09
			verb_log_info(
			   "Autosave: Rolled %s to %s\n", filename_next.c_str(), filename_previous.c_str());
		} catch (const FileError& e) {
			log_warn("Autosave: Unable to roll file %s to %s: %s\n", filename_previous.c_str(),
			         filename_next.c_str(), e.what());
			return false;
		}
		filename_previous = filename_next;
		rolls--;
	}
	return true;
}

/**
 * Check if game should be saved at next tick / think.
 *
 * @return true if game should be saved at next think().
 */
bool SaveHandler::check_next_tick(Widelands::Game& game, uint32_t realtime) const {
	// Perhaps save is due now?
	if (autosave_interval_in_ms_ <= 0 || next_save_realtime_ > realtime) {
		return false;  // no autosave or not due, yet
	}

	// check if game is paused (in any way)
	if (game.game_controller()->is_paused_or_zero_speed()) {
		return false;
	}

	verb_log_info_time(
	   game.get_gametime(), "Autosave: %d ms interval elapsed, current gametime: %s, saving...\n",
	   autosave_interval_in_ms_, gametimestring(game.get_gametime().get(), true).c_str());

	game.get_ibase()->log_message(_("Saving game…"));

	return true;
}

/**
 * Check if autosave is needed and allowed or save was requested by user.
 */
void SaveHandler::think(Widelands::Game& game) {
	if (!allow_saving_ || game.is_replay()) {
		return;
	}

	const uint32_t realtime = SDL_GetTicks();
	initialize(realtime);

	// Are we saving now?
	if (saving_next_tick_ || save_requested_) {
		saving_next_tick_ = false;
		bool save_success = true;
		std::string error;
		std::string filename = autosave_filename_;
		if (save_requested_) {
			// Requested by user
			if (!save_filename_.empty()) {
				filename = save_filename_;
			}
			verb_log_info_time(
			   game.get_gametime(), "Gamesave: save requested: %s\n", filename.c_str());
			save_requested_ = false;
			save_filename_ = "";
		} else {
			// Autosave ...
			save_success = roll_save_files(filename, &error);
			if (save_success) {
				filename = format("%s_00", autosave_filename_);
				verb_log_info_time(game.get_gametime(), "Autosave: saving as %s\n", filename.c_str());
			}
		}

		if (save_success) {
			// Saving now (always overwrite file)
			std::string complete_filename = create_file_name(kSaveDir, filename);
			save_success = save_game(game, complete_filename, &error);
		}
		if (!save_success) {
			log_err_time(game.get_gametime(), "Autosave: ERROR! - %s\n", error.c_str());
			game.get_ibase()->log_message(_("Saving failed!"));

			// Wait 30 seconds until next save try
			next_save_realtime_ = SDL_GetTicks() + 30000;
			return;
		}

		// Count save interval from end of save.
		// This prevents us from going into endless autosave cycles if the save
		// should take longer than the autosave interval.
		next_save_realtime_ = SDL_GetTicks() + autosave_interval_in_ms_;

		verb_log_info_time(
		   game.get_gametime(), "Autosave: save took %d ms\n", SDL_GetTicks() - realtime);
		game.get_ibase()->log_message(_("Game saved"));
	} else {
		saving_next_tick_ = check_next_tick(game, realtime);
	}
}

/**
 * Lazy intialisation on first call.
 */
void SaveHandler::initialize(uint32_t realtime) {
	if (initialized_) {
		return;
	}

	fs_type_ = get_config_bool("nozip", false) ? FileSystem::DIR : FileSystem::ZIP;

	autosave_interval_in_ms_ = get_config_int("autosave", kDefaultAutosaveInterval * 60) * 1000;

	next_save_realtime_ = realtime + autosave_interval_in_ms_;
	last_save_realtime_ = realtime;

	number_of_rolls_ = get_config_int("rolling_autosave", 5);

	initialized_ = true;
}

/*
 * Calculate the name of the save file
 */
std::string SaveHandler::create_file_name(const std::string& dir,
                                          const std::string& filename) const {
	// Append directory name.
	std::string complete_filename = dir + FileSystem::file_separator() + filename;
	// Trim it for preceding/trailing whitespaces in user input
	trim(complete_filename);

	// Now check if the extension matches (ignoring case)
	if (!ends_with(filename, kSavegameExtension, false)) {
		complete_filename += kSavegameExtension;
	}

	return complete_filename;
}

/*
 * Save the game using the GameSaver.
 *
 * Overwrites file if it exists.
 *
 * Will copy text of errors to error string.
 *
 * Returns true if saved, false in case some error occured.
 */
bool SaveHandler::save_game(Widelands::Game& game,
                            const std::string& complete_filename,
                            std::string* const error_str) {
	ScopedTimer save_timer("SaveHandler::save_game() took %ums", true);

	// save game via the GenericSaveHandler
	GenericSaveHandler gsh(
	   [&game](FileSystem& fs) {
		   Widelands::GameSaver gs(fs, game);
		   gs.save();
	   },
	   complete_filename, fs_type_);
	gsh.save();
	last_save_realtime_ = SDL_GetTicks();

	// Ignore it if only the temporary backup wasn't deleted
	// but save was successfull otherwise
	if (gsh.error() == GenericSaveHandler::Error::kSuccess ||
	    gsh.error() == GenericSaveHandler::Error::kDeletingBackupFailed) {
		return true;
	}

	if (error_str != nullptr) {
		*error_str = gsh.error_message();
	}
	return false;
}
