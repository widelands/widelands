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

#ifndef included_transport_h
#define included_transport_h

#include "instances.h"
#include "map.h"

class Road;

/*
Flag represents a flag, obviously.
A flag itself doesn't do much. However, it can have up to 6 roads attached
to it. Instead of the WALK_NW road, it can also have a building attached to
it.
Flags also have a store of up to 8 wares.

Important: Do not access m_roads directly. get_road() and others use 
Map_Object::WALK_xx in all "direction" parameters.
*/
class Flag : public BaseImmovable {
public:
	Flag();
	virtual ~Flag();
	
	static Flag *create(Game *g, Player *owner, Coords coords);
	
	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();
	
	inline Player *get_owner() const { return m_owner; }
	
	inline Building *get_building() { return m_building; }
	void attach_building(Game *g, Building *building);
	void detach_building(Game *g);
	
	inline Road *get_road(int dir) { return m_roads[dir-1]; }
	void attach_road(int dir, Road *road);
	void detach_road(int dir);

	inline const Coords &get_position() const { return m_position; }
	
protected:
	virtual void init(Game*);
	virtual void cleanup(Game*);
	
	virtual void draw(Game* game, Bitmap* dst, FCoords coords, int posx, int posy);

private:
	Player		*m_owner;
	Coords		m_position;
	Animation	*m_anim;
	int			m_animstart;
	
	Building		*m_building;	// attached building (replaces road WALK_NW)
	Road			*m_roads[6];	// Map_Object::WALK_xx-1 as index
};

/*
Road is a special object which connects two flags.
The Road itself is never rendered; however, the appropriate Field::roads are
set to represent the road visually.
The actual steps involved in a road are stored as a Path from the staring flag
to the ending flag. Apart from that, however, the two flags are treated
exactly the same, as far as most transactions are concerned. There are minor
exceptions: placement of carriers if the path's length is odd, splitting
a road when a flag is inserted.

Every road has one or more Carriers attached to it.
*/
class Road : public BaseImmovable {
public:
	Road();
	virtual ~Road();

	static Road *create(Game *g, int type, Flag *start, Flag *end, const Path &path);

	inline Flag *get_flag_start() const { return m_start; }
	inline Flag *get_flag_end() const { return m_end; }
		
	virtual int get_type();
	virtual int get_size();
	virtual bool get_passable();

	void presplit(Game *g, Coords split);
	void postsplit(Game *g, Flag *flag);
	
protected:
	void mark_map(Game *g);
	void unmark_map(Game *g);

	virtual void init(Game *g);
	virtual void cleanup(Game *g);
	
	virtual void draw(Game* game, Bitmap* dst, FCoords coords, int posx, int posy);

private:
	int		m_type;		// use Field::Road_XXX
	Flag		*m_start;
	Flag		*m_end;
	Path		m_path;		// path goes from m_start to m_end
};

#endif // included_transport_h
