/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#include <SDL.h>
#include <boost/regex.hpp>

#include "base/log.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"

/** Create an FXset
 * \param directory The directory the sound files are in
 * \param base_filename The filename base
 */
FXset::FXset(const std::string& directory, const std::string& base_filename) : last_used_(0) {
	// Check directory
	if (!g_fs->is_directory(directory)) {
		throw Widelands::GameDataError("SoundHandler: Can't load files from %s, not a directory!", directory.c_str());
	}

	// Find files
	boost::regex re(base_filename + "_\\d+\\.ogg");
	paths_ = filter(g_fs->list_directory(directory), [&re](const std::string& fn) {
		return boost::regex_match(FileSystem::fs_filename(fn.c_str()), re);
	});

	// Ensure that we have at least 1 file
	if (paths_.empty()) {
		throw Widelands::GameDataError("FXset: No files matching the pattern '%s_<numbers>.ogg' found in directory %s\n",
							base_filename.c_str(), directory.c_str());
	}

	// Ensure that we haven't found any directories by mistake
	for (const std::string& path : paths_) {
		assert(!g_fs->is_directory(path));
	}
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

Mix_Chunk* FXset::get_fx(uint32_t random) {
	if (paths_.empty()) {
		return nullptr;
	}

	if (fxs_.empty()) {
		for (const std::string& path : paths_) {
			load_sound_file(path);
		}
	}
	assert(!fxs_.empty());
	assert(fxs_.size() == paths_.size());

	last_used_ = SDL_GetTicks();

	return fxs_.at(random % fxs_.size());
}


void FXset::load_sound_file(const std::string& path) {
	FileRead fr;
	if (!fr.try_open(*g_fs, path)) {
		log("WARNING: Could not open %s for reading!\n", path.c_str());
		return;
	}

	if (Mix_Chunk* const m =
	       Mix_LoadWAV_RW(SDL_RWFromMem(fr.data(fr.get_size(), 0), fr.get_size()), 1)) {
		// Append a sound effect to the end of the fxset
		assert(m);
		fxs_.push_back(m);
	} else {
		log("FXset: loading sound effect file \"%s\" failed: %s\n",
		    path.c_str(), Mix_GetError());
	}
}
