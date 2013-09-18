/*
 * Copyright (C) 2006, 2009 by the Widelands Development Team
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

#ifndef FXSET_H
#define FXSET_H

#include <vector>

#include <SDL_mixer.h>


class Sound_Handler;

/// Predefined priorities for easy reading
/// \warning DO NOT CHANGE !! The values have meaning beyond just being numbers
/// \todo These values should not have any meaning beyond just being numbers.
#define PRIO_ALWAYS_PLAY 255
#define PRIO_ALLOW_MULTIPLE 128
#define PRIO_MEDIUM 63

/** A collection of several sound effects meant for the same event.
 *
 * An FXset encapsulates a number of interchangeable sound effects, e.g.
 * all effects that might be played when a blacksmith is happily hammering away.
 * It is possible to select the effects one after another or in random order.
 * The fact that an FXset really contains several different effects is hidden
 * from the outside
 */
struct FXset {
	friend class Sound_Handler;
	FXset(uint8_t priority = PRIO_MEDIUM);
	~FXset();

	void add_fx(Mix_Chunk * fx, Uint8 prio = PRIO_MEDIUM);
	Mix_Chunk * get_fx();
	bool empty() {return m_fxs.empty();}

protected:
	/// The collection of sound effects
	std::vector<Mix_Chunk *> m_fxs;

	/** When the effect was played the last time (milliseconds since SDL
	 * initialization). Set via SDL_GetTicks()
	 */
	Uint32 m_last_used;

	/** How important is it to play the effect even when others are running
	 * already?
	 *
	 * Value 0-127: probability between 0.0 and 1.0, only one instance can
	 * be playing at any time
	 *
	 * Value 128-254: probability between 0.0 and 1.0, many instances can
	 * be playing at any time
	 *
	 * Value 255: always play; unconditional
	 */
	uint8_t m_priority;
};

#endif
