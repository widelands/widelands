/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef ANIMATION_GFX_H
#define ANIMATION_GFX_H

#include "player.h"

struct Surface;

struct AnimationGfx { /// The graphics belonging to an animation.
	AnimationGfx(const AnimationData* data);
	~AnimationGfx();

	const Point get_hotspot() const throw () {return m_hotspot;}
	typedef std::vector<Surface *> Frames;
	typedef Frames::size_type Index;
	Index nr_frames() const {assert((*m_plrframes)[0]); return m_plrframes[0].size();}
	Surface * get_frame
		(Index                     const i,
		 Widelands::Player_Number  const plyr,
		 Widelands::Player const * const player)
	{
		assert(i < nr_frames());
		assert(plyr <= MAX_PLAYERS);
		if (!m_encodedata.hasplrclrs || !plyr)
			return m_plrframes[0][i];

		assert(player);

		// Encode for this player
		if (not m_plrframes[plyr].size()) encode(plyr, player->get_playercolor());
		return m_plrframes[plyr][i];
	}

private:
	void encode(uint8_t plyr, const RGBColor*);

	Frames * m_plrframes;
	Frames m_pcmasks;
	EncodeData m_encodedata;
	Point m_hotspot;
};

#endif
