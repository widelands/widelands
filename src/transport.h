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
Flag_Descr is pretty much a dummy class. It is necessary because of special
flag attributes
*/
class Flag_Descr : public Map_Object_Descr {
public:
	Flag_Descr();
	
	virtual Map_Object *create_object();
};

/*
Flag represents a flag, obviously.
A flag itself doesn't do much. However, it can have up to 6 roads attached
to it. Instead of the WALK_NW road, it can also have a building attached to
it.
Flags also have a store of up to 8 wares.

Important: Do not access m_roads directly. get_road() and others use 
Map_Object::WALK_xx in all "direction" parameters.
*/
class Flag : public Map_Object {
	MO_DESCR(Flag_Descr);

public:
	Flag(Flag_Descr *descr);
	virtual ~Flag();
	
	static Flag *create(Game *g, int owner, Coords &coords);
	
	inline Building *get_building() { return m_building; }
	void attach_building(Game *g, Building *building);
	void detach_building(Game *g);
	
	inline Road *get_road(int dir) { return m_roads[dir-1]; }
	void attach_road(int dir, Road *road);
	void detach_road(int dir);
	
protected:
	virtual void init(Game*);
	virtual void cleanup(Game*);
	
	virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
	
private:
	Building	*m_building;	// attached building (replaces road WALK_NW)
	Road		*m_roads[6];	// Map_Object::WALK_xx-1 as index
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
class Road {
public:
	Road(int type, Flag *start, Flag *end, const Path &path);
	~Road();

	void init(Game *g);
	void cleanup(Game *g);
	
private:
	int		m_type;		// use Field::Road_XXX
	Flag		*m_start;
	Flag		*m_end;
	Path		m_path;		// path goes from m_start to m_end
};

#endif // included_transport_h
