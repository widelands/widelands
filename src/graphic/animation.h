/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "logic/instances.h"
#include "rgbcolor.h"

#include "point.h"

#include <cstring>
#include <map>
#include <string>
#include <vector>

class Profile;
class Section;

struct EncodeData {
	enum {No, Mask, Old} hasplrclrs;
	RGBColor plrclr[4];

	void clear();
	void parse(Section &);
	void add(const EncodeData *o);
};

struct AnimationData {
	uint32_t frametime;
	Point hotspot;
	EncodeData encdata;
	std::string picnametempl;

	/** mapping of soundeffect name to frame number, indexed by frame number
	 * \sa AnimationManager::trigger_sfx */
	std::map<uint32_t, std::string> sfx_cues;
};

/**
* The animation manager manages a list of all active animations.
* They are flushed after a game is finished and loaded at gfxload time by the
* Game code.
* get() only works properly before gfxload. This means that all animations must
* be loaded before the game starts.
*
* Note that animation IDs are counted from 1, while the m_animations array is
* counted from 0.
*/
struct AnimationManager {
	void flush();
	uint32_t get
		(std::string const & directory,
		 Section           & s,
		 char       const * picnametempl = 0,
		 EncodeData  const * encdata = 0)
	{
		return get(directory.c_str(), s, picnametempl, encdata);
	}
	uint32_t get
		(char       const * directory,
		 Section          &,
		 char       const * picnametempl = 0,
		 EncodeData const * = 0);

	// for use by the graphics subsystem
	uint32_t get_nranimations() const;
	AnimationData const * get_animation(uint32_t id) const;
	void trigger_soundfx
		(uint32_t animation, uint32_t framenumber, uint32_t stereo_position);

private:
	std::vector<AnimationData> m_animations;
};


/**
* Use this class to automatically manage a set of 6 animations, one for each
* possible direction
*/
struct DirAnimations {
	explicit DirAnimations
		(const uint32_t dir1 = 0,
		 const uint32_t dir2 = 0,
		 const uint32_t dir3 = 0,
		 const uint32_t dir4 = 0,
		 const uint32_t dir5 = 0,
		 const uint32_t dir6 = 0);

	void parse
		(Widelands::Map_Object_Descr &,
		 std::string           const & directory,
		 Profile                     &,
		 char                  const * sectnametempl,
		 Section                     * defaults    = 0,
		 EncodeData            const * encdefaults = 0);

	uint32_t get_animation(const int32_t dir) const throw ()
	{return m_animations[dir - 1];}

private:
	uint32_t m_animations[6];
};

extern AnimationManager g_anim;


#endif
