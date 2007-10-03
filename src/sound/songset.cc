/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#include "songset.h"

#include "fileread.h"
#include "layered_filesystem.h"
#include "sound_handler.h"

#include "log.h"

/// Prepare infrastructure for reading song files from disk
Songset::Songset() : m_m(0), m_rwops(0) {}

/// Close and delete all songs to avoid memory leaks.
Songset::~Songset()
{
	m_songs.clear();

	free(m_m);

	if (m_rwops) SDL_FreeRW(m_rwops);
}

/** Append a song to the end of the songset
 * \param filename  the song to append
 * \note The \ref current_song will unconditionally be set to the songset's
 * first song. If you do not want to disturb the (linear) playback order then
 * \ref register_song() all songs before you start playing
 */
void Songset::add_song(const std::string filename)
{
	m_songs.push_back(filename);
	m_current_song = m_songs.begin();
}

/** Get a song from the songset. Depending on
 * \ref Sound_Handler::sound_random_order, the selection will either be random
 * or linear (after last song, will start again with first).
 * \return  a pointer to the chosen song; 0 if none was found, music is disabled
 *          or an error occurred
 */
Mix_Music *Songset::get_song()
{
	int32_t songnumber;
	std::string filename;

	if (g_sound_handler.get_disable_music() || m_songs.empty())
		return 0;

	if (g_sound_handler.m_random_order) {
		songnumber = g_sound_handler.m_rng.rand() % m_songs.size();
		filename = m_songs.at(songnumber);
	} else {
		if (m_current_song == m_songs.end())
			m_current_song = m_songs.begin();

		filename = *(m_current_song++);
	}

	//first, close the previous song and remove it from memory
	free(m_m);

	if (m_rwops) {//  rwops might be 0
		SDL_FreeRW(m_rwops);
		m_fr.Close();
	}

	//then open the new song
	if (m_fr.TryOpen(*g_fs, filename.c_str()))
		m_rwops = SDL_RWFromMem(m_fr.Data(0), m_fr.GetSize());
	else return 0;

	//TODO: review the #ifdef'ed blocks

#if NEW_SDL_MIXER == 1
	m_m = Mix_LoadMUS_RW(m_rwops);

#else
#warning Please update your SDL_mixer library to at least version 1.2.6!!!

#ifdef __WIN32__
	// Hack for windows, works because cwd is directory where
	// executable is in
	m_m = Mix_LoadMUS(filename.c_str());

#else
	// We have to go the long way. We are pretty sure, we're not under windows
	// so we have a /tmp dir and mktemp (hopefully)
	// This solution is terribly slow, but we hope that there are only
	// a few sound musics around
	char tempfilebuf[256] = "/tmp/wl_tempmusic.XXXXXXXX";
	char* tempfile = tempfilebuf;
	tempfile = mktemp(tempfilebuf);

	FILE* f = fopen(tempfile, "w");
	fwrite(m_fr.Data(0), m_fr.GetSize(), 1, f);
	fclose(f);

	m_m = Mix_LoadMUS(tempfile);
	//TODO: this should use a RWopsified version!

#endif // __WIN32__
#endif // NEW_SDL_MIXER == 1

	if (m_m) {
		log(("Sound_Handler: loaded song \""+filename+"\"\n").c_str());
	} else {
		log(("Sound_Handler: loading song \"" + filename +
		     "\" failed!\n").c_str());
		log("Sound_Handler: %s\n", Mix_GetError());
	}

	return m_m;
}
