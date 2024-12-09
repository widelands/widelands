/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#ifndef WL_SOUND_SONGSET_H
#define WL_SOUND_SONGSET_H

#include <cassert>
#include <map>
#include <stdexcept>
#include <vector>

#include <SDL_mixer.h>

#include "io/fileread.h"
#include "sound/song.h"

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

	Mix_Music* get_song(uint32_t random = 0);
	bool is_song_enabled(std::string& filename);
	void set_song_enabled(std::string& filename, bool on);
	std::vector<Song> get_song_data();

private:
	Mix_Music* load_file(const std::string& filename);
	void load_songs(const std::string& basename);
	void init_songs(std::vector<std::string> files);
	std::map<std::string, Song> create_playlist();

	/// List of song data
	std::map<std::string, Song> songs_;

	/** Index of the song that is currently playing
	 *  relative to the playlist of user selected songs.
	 */
	uint32_t current_song_;

	/// The current song
	Mix_Music* m_{nullptr};

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
	SDL_RWops* rwops_{nullptr};
};

#endif  // end of include guard: WL_SOUND_SONGSET_H
