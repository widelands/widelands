/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef included_animation_h
#define included_animation_h

#include "geometry.h"
#include "instances.h"
#include "rgbcolor.h"

#include <string>
#include <vector>
#include <map>

class Profile;
class Section;

struct EncodeData {
	bool     hasplrclrs;
	RGBColor plrclr[4];

	void clear();
	void parse(Section *s);
	void add(const EncodeData *o);
};

struct AnimationData {
	uint frametime;
	Point hotspot;
	EncodeData encdata;
	std::string picnametempl;

	/** mapping of soundeffect name to frame number, indexed by frame number
	 * \sa AnimationManager::trigger_sfx */
	std::map<uint, std::string> sfx_cues;
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
	AnimationManager();
	~AnimationManager();

	void flush();
	uint get(const char *directory, Section *s, const char *picnametempl = 0,
	         const EncodeData *encdefaults = 0);

	// for use by the graphics subsystem
	uint get_nranimations() const;
	const AnimationData* get_animation(uint id) const;
	void trigger_soundfx(uint animation, uint framenumber, uint stereo_position);

private:
	std::vector<AnimationData> m_animations;
};


/**
* Use this class to automatically manage a set of 6 animations, one for each
* possible direction
*/
struct DirAnimations {
	DirAnimations
		(const uint dir1 = 0,
		 const uint dir2 = 0,
		 const uint dir3 = 0,
		 const uint dir4 = 0,
		 const uint dir5 = 0,
		 const uint dir6 = 0);
	~DirAnimations();

	void parse(Map_Object_Descr* b, const char *directory, Profile *prof,
	           const char *sectnametempl, Section *defaults = 0,
	           const EncodeData *encdefaults = 0);

	uint get_animation(const int dir) const throw ()
			{return m_animations[dir - 1];}

private:
		uint m_animations[6];
};

extern AnimationManager g_anim;


#endif // included_animation_h
