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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "sound/songset.h"

#include <cassert>

#include "base/log.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"

/// Prepare infrastructure for reading song files from disk and register the matching files
Songset::Songset(const std::string& dir, const std::string& basename)
   : m_(nullptr), rwops_(nullptr) {
	assert(g_fs);
	std::vector<std::string> mp3_files = g_fs->get_sequential_files(dir, basename, "mp3");
	std::vector<std::string> ogg_files = g_fs->get_sequential_files(dir, basename, "ogg");
	add_songs(mp3_files);
	add_songs(ogg_files);
}
void Songset::add_songs(const std::vector<std::string>& files) {
	for (const std::string& filename : files) {
		assert(!g_fs->is_directory(filename));
		add_song(filename);
	}
}

/// Close and delete all songs to avoid memory leaks.
Songset::~Songset() {
	songs_.clear();

	if (m_) {
		Mix_FreeMusic(m_);
	}

	if (rwops_) {
		SDL_FreeRW(rwops_);
		fr_.close();
	}
}

/** Append a song to the end of the songset
 * \param filename  the song to append
 * \note The \ref current_song will unconditionally be set to the songset's
 * first song. If you do not want to disturb the (linear) playback order then
 * \ref register_song() all songs before you start playing
 */
void Songset::add_song(const std::string& filename) {
	songs_.push_back(filename);
	current_song_ = 0;
}

/**
 * Uses a 'random' number to select a song and return its audio data.
 * \param random A random number for picking the song
 * \return  a pointer to the chosen song; nullptr if none was found
 *          or an error occurred
 */
Mix_Music* Songset::get_song(uint32_t random) {
	std::string filename;

	if (songs_.empty()) {
		return nullptr;
	}

	if (songs_.size() > 1) {
		// Exclude current_song from playing two times in a row
		current_song_ += 1 + random % (songs_.size() - 1);
		current_song_ = current_song_ % songs_.size();
	}
	filename = songs_.at(current_song_);

	// First, close the previous song and remove it from memory
	if (m_) {
		Mix_FreeMusic(m_);
		m_ = nullptr;
	}

	if (rwops_) {
		SDL_FreeRW(rwops_);
		rwops_ = nullptr;
		fr_.close();
	}

	// Then open the new song
	if (fr_.try_open(*g_fs, filename)) {
		if (!(rwops_ = SDL_RWFromMem(fr_.data(0), fr_.get_size()))) {
			fr_.close();  // fr_ should be Open iff rwops_ != 0
			return nullptr;
		}
	} else {
		return nullptr;
	}

	if (rwops_) {
		m_ = Mix_LoadMUS_RW(rwops_, 0);
	}

	if (m_) {
		log_info("Songset: Loaded song \"%s\"\n", filename.c_str());
	} else {
		log_err("Songset: Loading song \"%s\" failed!\n", filename.c_str());
		log_err("Songset: %s\n", Mix_GetError());
	}

	return m_;
}
