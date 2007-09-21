/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef included_animation_gfx_h
#define included_animation_gfx_h

#include "player.h"
#include <stdint.h>

struct Surface;

struct AnimationGfx { /// The graphics belonging to an animation.
	AnimationGfx(const AnimationData* data);
	~AnimationGfx();

	const Point get_hotspot() const throw () {return m_hotspot;}
	typedef std::vector<Surface *> Frames;
	typedef Frames::size_type Index;
	Index nr_frames() const {assert((*m_plrframes)[0]); return m_plrframes[0].size();}
	Surface * get_frame
		(const Index i, const Player_Number plyr, const Player * const player)
	{
		assert(i < nr_frames());
		assert(plyr <= MAX_PLAYERS);
		if (not m_encodedata.hasplrclrs) return m_plrframes[0][i];

		assert(player);

		// Encode for this player
		if (not m_plrframes[plyr].size()) encode(plyr, player->get_playercolor());
		return m_plrframes[plyr][i];
	}

private:
	void encode(uint8_t plyr, const RGBColor*);

	Frames   * m_plrframes;
	EncodeData m_encodedata;
	Point      m_hotspot;
};

#endif // included_animation_gfx_h
