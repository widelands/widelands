/*
 * Copyright (C) 2006, 2008-2009 by the Widelands Development Team
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

#include "sound/songset.h"

#include <utility>

#include "base/log.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "sound/sound_handler.h"

/// Prepare infrastructure for reading song files from disk
Songset::Songset() : m_(nullptr), rwops_(nullptr) {}

/// Close and delete all songs to avoid memory leaks.
Songset::~Songset()
{
	songs_.clear();

	if (m_)
		Mix_FreeMusic(m_);

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
void Songset::add_song(const std::string & filename) {
	songs_.push_back(filename);
	current_song_ = songs_.begin();
}

/** Get a song from the songset. Depending on
 * \ref SoundHandler::sound_random_order, the selection will either be random
 * or linear (after last song, will start again with first).
 * \return  a pointer to the chosen song; 0 if none was found, music is disabled
 *          or an error occurred
 */
Mix_Music * Songset::get_song()
{
	std::string filename;

	if (g_sound_handler.get_disable_music() || songs_.empty())
		return nullptr;

	if (g_sound_handler.random_order_)
		filename = songs_.at(g_sound_handler.rng_.rand() % songs_.size());
	else {
		if (current_song_ == songs_.end())
			current_song_ = songs_.begin();

		filename = *(current_song_++);
	}

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
	}
	else
		return nullptr;

	if (rwops_)
		m_ = Mix_LoadMUS_RW(rwops_, 0);

	if (m_)
		log("SoundHandler: loaded song \"%s\"\n", filename.c_str());
	else {
		log("SoundHandler: loading song \"%s\" failed!\n", filename.c_str());
		log("SoundHandler: %s\n", Mix_GetError());
	}

	return m_;
}
