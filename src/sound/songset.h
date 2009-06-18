/*
 * Copyright (C) 2006, 2008 by the Widelands Development Team
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

#ifndef SONGSET_H
#define SONGSET_H

#include "io/fileread.h"

#include <config.h> //  must be included before SDL_mixer.h!
#include <SDL_mixer.h>

#include <string>
#include <cstring>
#include <vector>

/** A collection of several pieces of music meant for the same situation.
 *
 * A Songset encapsulates a number of interchangeable pieces of (background)
 * music, e.g. all songs that might be played while the main menu is being
 * shown. It is possible to access those songs one after another or in
 * random order. The fact that a Songset really contains several different
 * songs is hidden from the outside.\n
 * A songset does not contain the audio data itself, to not use huge amounts of
 * memory. Instead, each song is loaded on request and the data is free()d
 * afterwards
 */
struct Songset {
	Songset();
	~Songset();

	void add_song(std::string const & filename);
	Mix_Music * get_song();
	bool empty() {return m_songs.empty();}

protected:
	/// The filenames of all configured songs
	std::vector < std::string > m_songs;

	/** Pointer to the song that is currently playing (actually the one that
	 * was last started); needed for linear playback
	 */
	std::vector < std::string >::iterator m_current_song;

	/// The current song
	Mix_Music * m_m;

	/** File reader object to fetch songs from disk when they start playing.
	 * Do not create this for each load, it's a major hassle to code.
	 * \sa m_rwops
	 * \sa get_song()
	 */
	FileRead m_fr;

	/** RWops object to fetch songs from disc when they start playing.
	 * Do not create this for each load, it's a major hassle to code.
	 * \sa m_fr
	 * \sa get_song()
	 */
	SDL_RWops * m_rwops;
};

#endif
