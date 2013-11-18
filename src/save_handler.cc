/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "save_handler.h"

#include "chat.h"
#include "game_io/game_saver.h"
#include "io/filesystem/filesystem.h"
#include "log.h"
#include "logic/game.h"
#include "profile/profile.h"
#include "scoped_timer.h"
#include "upcast.h"
#include "wexception.h"
#include "wlapplication.h"
#include "wui/interactive_base.h"

using Widelands::Game_Saver;

/**
* Check if autosave is not needed.
 */
void SaveHandler::think(Widelands::Game & game, int32_t realtime) {
	initialize(realtime);
	std::string filename = "wl_autosave";

	if (!m_allow_saving) {
		return;
	}
	if (game.is_replay()) {
		return;
	}

	if (m_save_requested) {
		if (!m_save_filename.empty()) {
			filename = m_save_filename;
		}

		log("Autosave: save requested : %s\n", filename.c_str());
		m_save_requested = false;
		m_save_filename = "";
	} else {
		const int32_t autosave_interval_in_seconds =
			g_options.pull_section("global").get_int
				("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60);
		if (autosave_interval_in_seconds <= 0) {
			return; // no autosave requested
		}

		const int32_t elapsed = (realtime - m_last_saved_time) / 1000;
		if (elapsed < autosave_interval_in_seconds) {
			return;
		}

		log("Autosave: interval elapsed (%d s), saving\n", elapsed);
	}

	// TODO: defer saving to next tick so that this message is shown
	// before the actual save, or put the saving logic in another thread
	game.get_ibase()->log_message(_("Saving game..."));

	// save the game
	const std::string complete_filename = create_file_name(get_base_dir(), filename);
	std::string backup_filename;

	// always overwrite a file
	if (g_fs->FileExists(complete_filename)) {
		filename += "2";
		backup_filename = create_file_name (get_base_dir(), filename);
		if (g_fs->FileExists(backup_filename)) {
			g_fs->Unlink(backup_filename);
		}
		g_fs->Rename(complete_filename, backup_filename);
	}

	std::string error;
	if (!save_game(game, complete_filename, &error)) {
		log("Autosave: ERROR! - %s\n", error.c_str());
		game.get_ibase()->log_message(_("Saving failed!"));

		// if backup file was created, move it back
		if (backup_filename.length() > 0) {
			if (g_fs->FileExists(complete_filename)) {
				g_fs->Unlink(complete_filename);
			}
			g_fs->Rename(backup_filename, complete_filename);
		}
		// Wait 30 seconds until next save try
		m_last_saved_time = m_last_saved_time + 30000;
		return;
	} else {
		// if backup file was created, time to remove it
		if (backup_filename.length() > 0 && g_fs->FileExists(backup_filename))
			g_fs->Unlink(backup_filename);
	}

	log("Autosave: save took %d ms\n", m_last_saved_time - realtime);
	game.get_ibase()->log_message(_("Game saved"));
}

/**
* Initialize autosave timer
 */
void SaveHandler::initialize(int32_t currenttime) {
	if (m_initialized)
		return;

	m_last_saved_time = currenttime;
	m_initialized = true;
}

/*
 * Calculate the name of the save file
 */
std::string SaveHandler::create_file_name
	(std::string dir, std::string filename)
{
	// ok, first check if the extension matches (ignoring case)
	bool assign_extension = true;
	if (filename.size() >= strlen(WLGF_SUFFIX)) {
		char buffer[10]; // enough for the extension
		filename.copy
			(buffer, sizeof(WLGF_SUFFIX), filename.size() - strlen(WLGF_SUFFIX));
		if (!strncasecmp(buffer, WLGF_SUFFIX, strlen(WLGF_SUFFIX)))
			assign_extension = false;
	}
	if (assign_extension)
		filename += WLGF_SUFFIX;

	// Now append directory name
	std::string complete_filename = dir;
	complete_filename += "/";
	complete_filename += filename;

	return complete_filename;
}

/*
 * Save the game
 *
 * returns true if saved
 */
bool SaveHandler::save_game
	(Widelands::Game   &       game,
	 const std::string &       complete_filename,
	 std::string       * const error)
{
	ScopedTimer save_timer("SaveHandler::save_game() took %ums");

	bool const binary =
		!g_options.pull_section("global").get_bool("nozip", false);
	// Make sure that the base directory exists
	g_fs->EnsureDirectoryExists(get_base_dir());

	// Make a filesystem out of this
	std::unique_ptr<FileSystem> fs;
	if (!binary) {
		fs.reset(g_fs->CreateSubFileSystem(complete_filename, FileSystem::DIR));
	} else {
		fs.reset(g_fs->CreateSubFileSystem(complete_filename, FileSystem::ZIP));
	}

	bool result = true;
	Game_Saver gs(*fs, game);
	try {
		gs.save();
	} catch (const std::exception & e) {
		if (error)
			*error = e.what();
		result = false;
	}

	if (result)
		m_last_saved_time = WLApplication::get()->get_time();

	return result;
}
