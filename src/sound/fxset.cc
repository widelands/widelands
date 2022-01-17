/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "sound/fxset.h"

#include <cassert>

#include <SDL_timer.h>

#include "base/log.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"

/**
 * Create an FXset
 * \param path The directory the sound files are in, followed by the filename base
 * \param random: Randomize the time last played a bit to prevent sound onslaught at game start
 */
FXset::FXset(const std::string& path, uint32_t random) : last_used_(random % 2000) {
	const std::string dirname = FileSystem::fs_dirname(path);
	const std::string basename = FileSystem::fs_filename(path.c_str());
	paths_ = g_fs->get_sequential_files(dirname, basename, "ogg");

	// Ensure that we have at least 1 file
	if (paths_.empty()) {
		throw Widelands::GameDataError(
		   "FXset: No files matching the pattern '%s_<numbers>.ogg' found in directory %s\n",
		   basename.c_str(), dirname.c_str());
	}

#ifndef NDEBUG
	// Ensure that we haven't found any directories by mistake
	for (const std::string& p : paths_) {
		assert(!g_fs->is_directory(p));
	}
#endif
}

/// Delete all fxs to avoid memory leaks. This also frees the audio data.
FXset::~FXset() {
	std::vector<Mix_Chunk*>::iterator i = fxs_.begin();

	while (i != fxs_.end()) {
		Mix_FreeChunk(*i);
		++i;
	}

	fxs_.clear();
}

uint32_t FXset::ticks_since_last_play() const {
	return SDL_GetTicks() - last_used_;
}

void FXset::load_sound_files() {
	if (!paths_.empty()) {
		// Load sounds from paths if this FX hasn't been played yet
		for (const std::string& path : paths_) {
			load_sound_file(path);
		}
		assert(fxs_.size() == paths_.size());
		// We don't need the paths any more
		paths_.clear();
	}
	assert(paths_.empty());
}

Mix_Chunk* FXset::get_fx(uint32_t random) {
	load_sound_files();

	if (fxs_.empty()) {
		return nullptr;
	}

	assert(!fxs_.empty());

	last_used_ = SDL_GetTicks();

	return fxs_.at(random % fxs_.size());
}

void FXset::load_sound_file(const std::string& path) {
	FileRead fr;
	if (!fr.try_open(*g_fs, path)) {
		log_err("WARNING: Could not open %s for reading!\n", path.c_str());
		return;
	}

	if (Mix_Chunk* const m =
	       Mix_LoadWAV_RW(SDL_RWFromMem(fr.data(fr.get_size(), 0), fr.get_size()), 1)) {
		// Append a sound effect to the end of the fxset
		assert(m);
		fxs_.push_back(m);
	} else {
		log_err("FXset: loading sound effect file \"%s\" failed: %s\n", path.c_str(), Mix_GetError());
	}
}
