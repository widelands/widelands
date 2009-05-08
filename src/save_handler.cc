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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "save_handler.h"

#include "logic/game.h"
#include "wexception.h"
#include "wlapplication.h"
#include "io/filesystem/filesystem.h"
#include "game_io/game_saver.h"
#include "profile/profile.h"

#include "log.h"

using Widelands::Game_Saver;

/**
* Check if autosave is not needed.
 */
void SaveHandler::think(Widelands::Game & game, int32_t realtime) {
	initialize(realtime);

	int32_t const autosaveInterval =
		g_options.pull_section("global").get_int
			("autosave", DEFAULT_AUTOSAVE_INTERVAL * 60);
	if (autosaveInterval <= 0)
		return; // no autosave requested

	int32_t const elapsed = (realtime - m_lastSaveTime) / 1000;
	if (elapsed < autosaveInterval)
		return;

	log("Autosave: interval elapsed (%d s), saving\n", elapsed);

	// save the game
	std::string complete_filename =
		create_file_name (get_base_dir(), "wl_autosave");
	std::string backup_filename;

	// always overwrite a file
	if (g_fs->FileExists(complete_filename)) {
		backup_filename = create_file_name (get_base_dir(), "wl_autosave2");
		if (g_fs->FileExists(backup_filename)) {
			g_fs->Unlink(backup_filename);
		}
		g_fs->Rename(complete_filename, backup_filename);
	}

	if (!save_game(game, complete_filename)) {
		log("Autosave: ERROR\n");

		// if backup file was created, move it back
		if (backup_filename.length() > 0) {
			if (g_fs->FileExists(complete_filename)) {
				g_fs->Unlink(complete_filename);
			}
			g_fs->Rename(backup_filename, complete_filename);
		}
	} else {
		// if backup file was created, time to remove it
		if (backup_filename.length() > 0 && g_fs->FileExists(backup_filename))
			g_fs->Unlink(backup_filename);
	}

	log("Autosave: save took %d ms\n", m_lastSaveTime - realtime);
}

/**
* Initialize autosave timer
 */
void SaveHandler::initialize(int32_t currenttime) {
	if (m_initialized)
		return;

	m_lastSaveTime = currenttime;
	log("Autosave: initialized\n");
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
	 std::string const &       complete_filename,
	 std::string       * const error)
{
	bool const binary =
		!g_options.pull_section("global").get_bool("nozip", false);
	// Make sure that the base directory exists
	g_fs->EnsureDirectoryExists(get_base_dir());

	// Make a filesystem out of this
	FileSystem * fs = 0;
	if (!binary) {
		fs = &g_fs->CreateSubFileSystem(complete_filename, FileSystem::DIR);
	} else {
		fs = &g_fs->CreateSubFileSystem(complete_filename, FileSystem::ZIP);
	}

	bool result = true;
	Game_Saver gs(*fs, game);
	try {
		gs.save();
	} catch (std::exception const & e) {
		if (error)
			*error = e.what();
		result = false;
	}
	delete fs;

	if (result)
		m_lastSaveTime = WLApplication::get()->get_time();

	return result;
}
