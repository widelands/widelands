/*
 * Copyright (C) 2002 by the Widelands Development Team
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

class Profile;
class Section;

struct EncodeData {
	bool		hasclrkey;
	RGBColor	clrkey;
	bool		hasshadow;
	RGBColor	shadow;
	bool		hasplrclrs;
	RGBColor	plrclr[4];

	void clear();
	void parse(Section *s);
	void add(const EncodeData *o);
};

struct AnimationData {
	uint				frametime;
	Point				hotspot;
	EncodeData		encdata;
	std::string		picnametempl;
};

/*
class AnimationManager

The animation manager manages a list of all active animations.
They are flushed after a game is finished and loaded at gfxload time by the
Game code.
get() only works properly before gfxload. This means that all animations must
be loaded before the game starts.

Note that animation IDs are counted from 1, while the m_animations array is
counted from 0.
*/
class AnimationManager {
public:
	AnimationManager();
	~AnimationManager();

	void flush();
	uint get(const char *directory, Section *s, const char *picnametempl = 0,
	         const EncodeData *encdefaults = 0);

public: // for use by the graphics subsystem
	uint get_nranimations() const;
	const AnimationData* get_animation(uint id) const;
	
private:
	std::vector<AnimationData>	m_animations;
};


/*
class DirAnimations

Use this class to automatically manage a set of 6 animations, one for each
possible direction
*/
class DirAnimations {
public:
	DirAnimations();
	~DirAnimations();
	
	void parse(const char *directory, Profile *prof, const char *sectnametempl, Section *defaults = 0, 
	           const EncodeData *encdefaults = 0);
	
	inline uint get_animation(int dir) { return m_animations[dir-1]; }
	
private:
	uint	m_animations[6];
};

extern AnimationManager g_anim;


#endif // included_animation_h
