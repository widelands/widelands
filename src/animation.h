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
class Animation;

struct Animation_Pic {
   ushort *data;
   Animation* parent;
};

struct EncodeData {
	bool hasclrkey;
	uchar clrkey_r, clrkey_g, clrkey_b;
	bool hasshadow;
	uchar shadow_r, shadow_g, shadow_b;
	bool hasplrclrs;
	uchar plrclr_r[4];
	uchar plrclr_g[4];
	uchar plrclr_b[4];

	void clear();
	void parse(Section *s);
	void add(const EncodeData *o);
};

class Animation {
public:
	Animation(void);
	~Animation(void);
   
	inline ushort get_w(void) { return w; }
	inline ushort get_h(void) { return h; }
	inline ushort get_hsx(void) { return hsx; }
	inline ushort get_hsy(void) { return hsy; }

	void add_pic(ushort size, ushort* data);
	void add_pic(Pic* pic, const EncodeData *enc);

	void set_flags(uint mflags) { flags=mflags; }
	void set_dimensions(ushort mw, ushort mh) { w=mw; h=mh; }
	void set_hotspot(ushort x, ushort y) { hsx=x; hsy=y; }

	void parse(const char *directory, Section *s, const char *picnametempl = 0, const EncodeData *encdefaults = 0);
		
	inline Animation_Pic* get_pic(ushort n) { assert(n<npics); return &pics[n]; }
	inline ushort get_npics(void) { return npics; }

	inline uint get_duration() { return m_frametime * npics; }
	inline Animation_Pic* get_time_pic(uint time) { return &pics[(time / m_frametime) % npics]; }
		
private:
	uint flags;
	uint m_frametime;
	ushort w, h;
	ushort hsx, hsy;
	ushort npics;
	Animation_Pic *pics;
};

class DirAnimations {
public:
	DirAnimations();
	~DirAnimations();
	
	inline Animation *get_animation(int dir) { return &m_animations[dir-1]; }
	
	void parse(const char *directory, Profile *prof, const char *sectnametempl, Section *defaults = 0, 
	           const EncodeData *encdefaults = 0);
	
private:
	Animation	m_animations[6];
};

#endif // included_animation_h
