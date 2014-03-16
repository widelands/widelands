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

#include "sound/fxset.h"

#include <cassert>

#include <SDL.h>

#include "sound/sound_handler.h"

/** Create an FXset and set it's \ref m_priority
 * \param[in] prio  The desired priority (optional)
 */
FXset::FXset(uint8_t const priority) : m_last_used(0), m_priority(priority) {}

/// Delete all fxs to avoid memory leaks. This also frees the audio data.
FXset::~FXset()
{
	std::vector < Mix_Chunk * >::iterator i = m_fxs.begin();

	while (i != m_fxs.end()) {
		Mix_FreeChunk(*i);
		++i;
	}

	m_fxs.clear();
}

/** Append a sound effect to the end of the fxset
 * \param[in] fx    The sound fx to append
 * \param[in] prio  Set previous \ref m_priority to new value (optional)
 */
void FXset::add_fx(Mix_Chunk * const fx, uint8_t const prio)
{
	assert(fx);

	m_priority = prio;
	m_fxs.push_back(fx);
}

/** Get a sound effect from the fxset. \e Which variant of the fx is actually
 * given out is determined at random
 * \return  a pointer to the chosen effect; 0 if sound effects are
 * disabled or no fx is registered
 */
Mix_Chunk * FXset::get_fx()
{
	if (g_sound_handler.get_disable_fx() || m_fxs.empty())
		return nullptr;

	m_last_used = SDL_GetTicks();

	return m_fxs.at(g_sound_handler.m_rng.rand() % m_fxs.size());
}
