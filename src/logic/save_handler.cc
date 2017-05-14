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

#include "logic/save_handler.h"

#include <cstring>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/scoped_timer.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "game_io/game_saver.h"
#include "io/filesystem/filesystem.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "wui/interactive_base.h"

// The actual work of saving is done by the GameSaver
using Widelands::GameSaver;

SaveHandler::SaveHandler()
   : next_save_realtime_(0),
     initialized_(false),
     allow_saving_(true),
     save_requested_(false),
     saving_next_tick_(false),
     save_filename_(""),
     autosave_filename_("wl_autosave"),
     fs_type_(FileSystem::ZIP),
     autosave_interval_in_ms_(DEFAULT_AUTOSAVE_INTERVAL * 60 * 1000),
     number_of_rolls_(5) {
}

void SaveHandler::roll_save_files(const std::string& filename) {

	int32_t rolls = number_of_rolls_;
	log("Autosave: Rolling savefiles (count): %d\n", rolls);
	rolls--;
	std::string filename_previous =
	   create_file_name(get_base_dir(), (boost::format("%s_%02d") % filename % rolls).str());
	if (rolls > 0 && g_fs->file_exists(filename_previous)) {
		g_fs->fs_unlink(filename_previous);  // Delete last of the rolling files
		log("Autosave: Deleted %s\n", filename_previous.c_str());
	}
	rolls--;
	while (rolls >= 0) {
		const std::string filename_next =
		   create_file_name(get_base_dir(), (boost::format("%s_%02d") % filename % rolls).str());
		if (g_fs->file_exists(filename_next)) {
			g_fs->fs_rename(
			   filename_next, filename_previous);  // e.g. wl_autosave_08 -> wl_autosave_09
			log("Autosave: Rolled %s to %s\n", filename_next.c_str(), filename_previous.c_str());
		}
		filename_previous = filename_next;
		rolls--;
	}
}

/**
 * Check if game should be saved at next tick / think.
 *
 * @return true if game should be saved ad next think().
 */
bool SaveHandler::check_next_tick(Widelands::Game& game, uint32_t realtime) {

	// Perhaps save is due now?
	if (autosave_interval_in_ms_ <= 0 || next_save_realtime_ > realtime) {
		return false;  // no autosave or not due, yet
	}

	next_save_realtime_ = realtime + autosave_interval_in_ms_;

	// check if game is paused (in any way)
	if (game.game_controller()->is_paused_or_zero_speed()) {
		return false;
	}

	log("Autosave: %d ms interval elapsed, current gametime: %s, saving...\n",
	    autosave_interval_in_ms_, gametimestring(game.get_gametime(), true).c_str());

	game.get_ibase()->log_message(_("Saving game…"));
	return true;
}

/**
 * If saving fails restore the backup file.
 *
 * @return true when save was a success.
 */
bool SaveHandler::save_and_handle_error(Widelands::Game& game,
                                        const std::string& complete_filename,
                                        const std::string& backup_filename) {
	std::string error;
	bool result = save_game(game, complete_filename, &error);
	if (!result) {
		log("Autosave: ERROR! - %s\n", error.c_str());
		game.get_ibase()->log_message(_("Saving failed!"));

		// if backup file was created, move it back
		if (backup_filename.length() > 0) {
			if (g_fs->file_exists(complete_filename)) {
				g_fs->fs_unlink(complete_filename);
			}
			g_fs->fs_rename(backup_filename, complete_filename);
		}
		// Wait 30 seconds until next save try
		next_save_realtime_ += 30000;
	} else {
		// if backup file was created, time to remove it
		if (backup_filename.length() > 0 && g_fs->file_exists(backup_filename))
			g_fs->fs_unlink(backup_filename);
	}
	return result;
}

/**
 * Check if autosave is needed and allowed or save was requested by user.
 */
void SaveHandler::think(Widelands::Game& game) {

	if (!allow_saving_ || game.is_replay()) {
		return;
	}

	uint32_t realtime = SDL_GetTicks();
	initialize(realtime);

	// Are we saving now?
	if (saving_next_tick_ || save_requested_) {
		std::string filename = autosave_filename_;
		if (save_requested_) {
			// Requested by user
			if (!save_filename_.empty()) {
				filename = save_filename_;
			}
			log("Gamesave: save requested: %s\n", filename.c_str());
			save_requested_ = false;
			save_filename_ = "";
		} else {
			// Autosave ...
			roll_save_files(filename);
			filename = (boost::format("%s_00") % autosave_filename_).str();
			log("Autosave: saving as %s\n", filename.c_str());
		}

		// Saving now
		const std::string complete_filename = create_file_name(get_base_dir(), filename);
		std::string backup_filename;

		// always overwrite a file
		if (g_fs->file_exists(complete_filename)) {
			filename += "2";
			backup_filename = create_file_name(get_base_dir(), filename);
			if (g_fs->file_exists(backup_filename)) {
				g_fs->fs_unlink(backup_filename);
			}
			g_fs->fs_rename(complete_filename, backup_filename);
		}

		std::string error;
		if (!save_and_handle_error(game, complete_filename, backup_filename)) {
			return;
		}

		log("Autosave: save took %d ms\n", SDL_GetTicks() - realtime);
		game.get_ibase()->log_message(_("Game saved"));
		saving_next_tick_ = false;

	} else {
		saving_next_tick_ = check_next_tick(game, realtime);
	}
}

/**
 * Lazy intialisation on first call.
 */
void SaveHandler::initialize(uint32_t realtime) {
	if (initialized_)
		return;

	Section& global = g_options.pull_section("global");

	fs_type_ = global.get_bool("nozip", false) ? FileSystem::DIR : FileSystem::ZIP;

	autosave_interval_in_ms_ = global.get_int("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60) * 1000;

	next_save_realtime_ = realtime + autosave_interval_in_ms_;

	number_of_rolls_ = global.get_int("rolling_autosave", 5);

	initialized_ = true;
}

/*
 * Calculate the name of the save file
 */
std::string SaveHandler::create_file_name(const std::string& dir,
                                          const std::string& filename) const {
	// Append directory name.
	std::string complete_filename = dir + g_fs->file_separator() + filename;
	// Trim it for preceding/trailing whitespaces in user input
	boost::trim(complete_filename);

	// Now check if the extension matches (ignoring case)
	if (!boost::iends_with(filename, WLGF_SUFFIX)) {
		complete_filename += WLGF_SUFFIX;
	}

	return complete_filename;
}

/*
 * Save the game using the GameSaver.
 *
 * Will copy text of exceptions to error string.
 *
 * returns true if saved, false in case some error occured.
 */
bool SaveHandler::save_game(Widelands::Game& game,
                            const std::string& complete_filename,
                            std::string* const error) {
	ScopedTimer save_timer("SaveHandler::save_game() took %ums");

	// Make sure that the base directory exists
	g_fs->ensure_directory_exists(get_base_dir());

	// Make a filesystem out of this
	std::unique_ptr<FileSystem> fs;
	fs.reset(g_fs->create_sub_file_system(complete_filename, fs_type_));

	bool result = true;
	GameSaver gs(*fs, game);
	try {
		gs.save();
	} catch (const std::exception& e) {
		if (error)
			*error = e.what();
		result = false;
	}

	return result;
}
