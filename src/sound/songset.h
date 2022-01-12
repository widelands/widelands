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

#ifndef WL_SOUND_SONGSET_H
#define WL_SOUND_SONGSET_H

#include <SDL_mixer.h>

#include "io/fileread.h"

/** A collection of several pieces of music meant for the same situation.
 *
 * A Songset encapsulates a number of interchangeable pieces of (background)
 * music, e.g. all songs that might be played while the main menu is being
 * shown.
 * A songset does not contain the audio data itself, to not use huge amounts of
 * memory. Instead, each song is loaded on request and the data is free()d
 * afterwards
 */
struct Songset {

	static constexpr const char* const kIngame = "ingame";
	static constexpr const char* const kCustom = "custom";
	static constexpr const char* const kMenu = "menu";
	static constexpr const char* const kIntro = "intro";
	explicit Songset(const std::string& dir, const std::string& basename);
	~Songset();

	Mix_Music* get_song(uint32_t random);

private:
	void add_songs(const std::vector<std::string>& files);
	void add_song(const std::string& filename);

	/// The filenames of all configured songs
	std::vector<std::string> songs_;

	/** Index of the song that is currently playing
	 * (actually the one that was last started)
	 */
	uint32_t current_song_;

	/// The current song
	Mix_Music* m_;

	/** File reader object to fetch songs from disk when they start playing.
	 * Do not create this for each load, it's a major hassle to code.
	 * \sa rwops_
	 * \sa get_song()
	 */
	FileRead fr_;

	/** RWops object to fetch songs from disc when they start playing.
	 * Do not create this for each load, it's a major hassle to code.
	 * \sa fr_
	 * \sa get_song()
	 */
	SDL_RWops* rwops_;
};

#endif  // end of include guard: WL_SOUND_SONGSET_H
