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

#ifndef __S__FIELD_H
#define __S__FIELD_H

// TODO; Think, if we shouldn't call for each field a new() in map::set_size
// and a delete - Holger
// Okay, as it stands now, Field can be safely memset()ed to 0.
//
// No. In fact, you should view Field more as a plain old structure rather than
// a class. If you think of Fields as a class you get into a whole lot of 
// problems (like the 6 neighbour field pointers we used to have). - Nicolai
// Probably it would be better to make Field a struct, rather than a class (things wouldn't
// change much, it's just a question of style and code understanding)

enum FieldCaps {
	// can we build normal buildings? (use BUILDCAPS_SIZEMASK for binary masking)
	BUILDCAPS_SMALL = 1,
	BUILDCAPS_MEDIUM = 2,
	BUILDCAPS_BIG = 3,
	BUILDCAPS_SIZEMASK = 3,

	// can we build a flag on this field?
	BUILDCAPS_FLAG = 4,

	// can we build a mine on this field (completely independent from build size!)
	BUILDCAPS_MINE = 8,
	
	// (only if BUILDCAPS_BIG): can we build a harbour on this field?
	// this should be automatically set for BUILDCAPS_BIG fields that have a swimmable second-order neighbour
	BUILDCAPS_PORT = 16,

	// Can Map_Objects walk or swim here? Also used for Map_Object::get_movecaps()
	// If MOVECAPS_WALK, any walking being can walk to this field
	MOVECAPS_WALK = 64,
	
	// If MOVECAPS_SWIM, any swimming being (including ships) can go there
	// Additionally, swimming beings can temporarily visit fields that are walkable
	// but not swimmable if those fields are at the start or end of their path.
	// Without this clause, harbours would be kind of impossible ;)
	// This clause stops ducks from "swimwalking" along the coast
	MOVECAPS_SWIM = 128,	
};

enum Roads {
	Road_None = 0,
	Road_Normal = 1,
	Road_Busy = 2,
	Road_Water = 3,
	Road_Mask = 3,
	
	Road_East = 0,	// shift values
	Road_SouthEast = 2,
	Road_SouthWest = 4,
};

class Terrain_Descr;
class Bob;
class BaseImmovable;

class Field {
   friend class Map;
	friend class Bob;
	friend class BaseImmovable;
	
private:
   uchar height;
	char brightness;
	uchar caps; // what can we build here, who can walk here [8 bits used]
	uchar owned_by; // 0 = neutral; otherwise: player number
	uchar roads; // are any roads on this field? [6 bits used]
	Terrain_Descr *terr, *terd;
	Bob* bobs; // linked list, see Bob::m_linknext
	BaseImmovable* immovable;

public:
   inline uchar get_height() const { return height; }
	inline uchar get_caps() const { return caps; }
	
   inline Terrain_Descr *get_terr() const { return terr; }
   inline Terrain_Descr *get_terd() const { return terd; }
   inline void set_terrainr(Terrain_Descr *p) { assert(p); terr = p; }
   inline void set_terraind(Terrain_Descr *p) { assert(p); terd = p; }

	inline Bob* get_first_bob(void) { return bobs; }
	inline BaseImmovable* get_immovable() { return immovable; }

	void set_brightness(int l, int r, int tl, int tr, int bl, int br);
   inline char get_brightness() const { return brightness; }

   inline void set_owned_by(uint pln) { owned_by = pln; }
   inline uchar get_owned_by(void) { return owned_by; }

	inline int get_roads() const { return roads; }
	inline int get_road(int dir) const { return (roads >> dir) & Road_Mask; }
	inline void set_road(int dir, int type) {
		roads &= ~(Road_Mask << dir); 
		roads |= type << dir;
	}
	
private:
   // note: you must reset this field's + neighbor's brightness when you change the height
   // Map's set_height does this
   inline void set_height(uchar h) { height = h; }
};


#endif
