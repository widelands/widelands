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

#ifndef WL_SOUND_FXSET_H
#define WL_SOUND_FXSET_H

#include <string>
#include <vector>

#include <SDL_mixer.h>

/** A collection of several sound effects meant for the same event.
 *
 * An FXset encapsulates a number of interchangeable sound effects, e.g.
 * all effects that might be played when a blacksmith is happily hammering away.
 * The fact that an FXset really contains several different effects is hidden
 * from the outside
 */
struct FXset {
	explicit FXset(const std::string& path, uint32_t random);
	~FXset();

	/**
	 * Number of ticks since this FXSet was last played
	 */
	uint32_t ticks_since_last_play() const;

	/** Get a sound effect from the fxset. Load the audio on demand.
	 * \param random A random number for picking a variant
	 * \return  a pointer to the chosen effect; 0 if sound effects are
	 * disabled or no fx is registered
	 */
	Mix_Chunk* get_fx(uint32_t random);

	/// Load the sound files into memory
	void load_sound_files();

private:
	/** Load an audio file into memory.
	 * \param path      the effect to be loaded
	 * The file format must be ogg. Otherwise this call will complain and
	 * not load the file.
	 * \note The complete audio file will be loaded into memory and stays there
	 * until the game is finished.
	 */
	void load_sound_file(const std::string& path);

	/** When the effect was played the last time (milliseconds since SDL
	 * initialization). Set via SDL_GetTicks()
	 */
	uint32_t last_used_;

	/**
	 * Filename paths for the physical sound files
	 * This will be cleared when the effects have been loaded into memory by \ref get_fx on first
	 * play.
	 */
	std::vector<std::string> paths_;

	/// The collection of sound effects, to be loaded on demand
	std::vector<Mix_Chunk*> fxs_;
};

#endif  // end of include guard: WL_SOUND_FXSET_H
