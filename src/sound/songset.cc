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

#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "sound/sound_handler.h"

namespace {
	// The behaviour of whether SDL_Mixer frees the RW it uses was changed with SDL_Mixer version 1.2.12, this
	// check is so that we don't have a memory leak in the new version.
	// TODO: Once we can demand that everyone use SDL_Mixer version >= 1.2.12, this function should be removed,
	// and all usages replaced supposing it's true.
	bool have_to_free_rw() {
		return
			SDL_VERSIONNUM(SDL_MIXER_MAJOR_VERSION, SDL_MIXER_MINOR_VERSION, SDL_MIXER_PATCHLEVEL) >=
			SDL_VERSIONNUM(1, 2, 12);
	}
}

/// Prepare infrastructure for reading song files from disk
Songset::Songset() : m_m(nullptr), m_rwops(nullptr) {}

/// Close and delete all songs to avoid memory leaks.
Songset::~Songset()
{
	m_songs.clear();

	if (m_m)
		Mix_FreeMusic(m_m);

	if (m_rwops) {
		if (have_to_free_rw())
			SDL_FreeRW(m_rwops);
		m_fr.Close();
	}
}

/** Append a song to the end of the songset
 * \param filename  the song to append
 * \note The \ref current_song will unconditionally be set to the songset's
 * first song. If you do not want to disturb the (linear) playback order then
 * \ref register_song() all songs before you start playing
 */
void Songset::add_song(const std::string & filename) {
	m_songs.push_back(filename);
	m_current_song = m_songs.begin();
}

/** Get a song from the songset. Depending on
 * \ref Sound_Handler::sound_random_order, the selection will either be random
 * or linear (after last song, will start again with first).
 * \return  a pointer to the chosen song; 0 if none was found, music is disabled
 *          or an error occurred
 */
Mix_Music * Songset::get_song()
{
	std::string filename;

	if (g_sound_handler.get_disable_music() || m_songs.empty())
		return nullptr;

	if (g_sound_handler.m_random_order)
		filename = m_songs.at(g_sound_handler.m_rng.rand() % m_songs.size());
	else {
		if (m_current_song == m_songs.end())
			m_current_song = m_songs.begin();

		filename = *(m_current_song++);
	}

	//first, close the previous song and remove it from memory
	if (m_m) {
		Mix_FreeMusic(m_m);
		m_m = nullptr;
	}

	if (m_rwops) {
		if (have_to_free_rw())
			SDL_FreeRW(m_rwops);
		m_rwops = nullptr;
		m_fr.Close();
	}

	//then open the new song
	if (m_fr.TryOpen(*g_fs, filename.c_str())) {
		if (!(m_rwops = SDL_RWFromMem(m_fr.Data(0), m_fr.GetSize()))) {
			m_fr.Close();  // m_fr should be Open iff m_rwops != 0
			return nullptr;
		}
	}
	else
		return nullptr;

	if (m_rwops)
		m_m = Mix_LoadMUS_RW(m_rwops);

	if (m_m)
		log("Sound_Handler: loaded song \"%s\"\n", filename.c_str());
	else {
		log("Sound_Handler: loading song \"%s\" failed!\n", filename.c_str());
		log("Sound_Handler: %s\n", Mix_GetError());
	}

	return m_m;
}
