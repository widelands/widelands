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

#include <vector>
#include "widelands.h"
#include "instances.h"
#include "game.h"
#include "map.h"


/** Object_Manager::~Object_Manager()
 *
 * Remove all map objects
 */
Object_Manager::~Object_Manager(void)
{
	while(!m_objects.empty()) {
		objmap_t::iterator it = m_objects.begin();
		free_object(it->second);
	}
}

/** Object_Manager::create_object(Game *g, Map_Object_Descr *d, int owner)
 *
 * Create an object of type d
 */
Map_Object* Object_Manager::create_object(Game *g, Map_Object_Descr *d, int owner, int x, int y)
{
	Map_Object* obj = d->create_object();
	
	m_lastserial++;
	assert(m_lastserial);
	obj->m_serial = m_lastserial;
	
	m_objects[m_lastserial] = obj;

	obj->set_owned_by(owner);
	obj->set_position(g, x, y);
	obj->init(g);
	
	return obj;
}

/** Object_Manager::free_object(Game* g, Map_Object* obj)
 *
 * Free the given object.
 */
void Object_Manager::free_object(Map_Object* obj)
{
	m_objects.erase(obj->m_serial);
	delete obj;
}

Map_Object* Object_Ptr::get(Game* game)
{
	if (!m_serial) return 0;
	Map_Object* obj = game->get_objects()->get_object(m_serial);
	if (!obj)
		m_serial = 0;
	return obj;
}

/** Map_Object::Map_Object(Type t)
 *
 * Zero-initialize a map object
 */
Map_Object::Map_Object(Type t)
{
	type = t;
	m_serial = 0;

	m_owned_by = -1;
	m_field = 0; // not linked anywhere
	m_px = m_py = 0;
	m_linknext = 0;
	m_linkpprev = 0;

	m_anim = 0;
	m_animstart = 0;
	
	m_walking = IDLE;
	m_walkstart = m_walkend = 0;
}

/** Map_Object::~Map_Object()
 *
 * Cleanup an object. Removes map links
 */
Map_Object::~Map_Object()
{
	if (m_field) {
		m_field = 0;
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}
}

/** Map_Object::draw(Game *game, Bitmap* dst, int posx, int posy)
 *
 * Draw the map object. 
 * posx/posy is the on-bitmap position of the field we're currently on,
 * WITHOUT height taken into account.
 *
 * It LERPs between start and end position when we're walking.
 * Note that the current field is actually the field we're walking to, not
 * the one we start from.
 */
void Map_Object::draw(Game *game, Bitmap* dst, int posx, int posy)
{
	if (!m_anim)
		return;

	Map *map = game->get_map();
	Field *start, *end;
	int dummyx, dummyy;
	int sx, sy;
	int ex, ey;
	
	end = m_field;
	ex = posx;
	ey = posy;

	start = 0;
	sx = ex;
	sy = ey;
	
	switch(m_walking) {
	case WALK_NW: map->get_brn(m_px, m_py, end, &dummyx, &dummyy, &start); sx += FIELD_WIDTH/2; sy += FIELD_HEIGHT/2; break;
	case WALK_NE: map->get_bln(m_px, m_py, end, &dummyx, &dummyy, &start); sx -= FIELD_WIDTH/2; sy += FIELD_HEIGHT/2; break;
	case WALK_W: map->get_rn(m_px, m_py, end, &dummyx, &dummyy, &start); sx += FIELD_WIDTH; break;
	case WALK_E: map->get_ln(m_px, m_py, end, &dummyx, &dummyy, &start); sx -= FIELD_WIDTH; break;
	case WALK_SW: map->get_trn(m_px, m_py, end, &dummyx, &dummyy, &start); sx += FIELD_WIDTH/2; sy -= FIELD_HEIGHT/2; break;
	case WALK_SE: map->get_tln(m_px, m_py, end, &dummyx, &dummyy, &start); sx -= FIELD_WIDTH/2; sy -= FIELD_HEIGHT/2; break;
	
	case IDLE: break;
	}

	ey -= end->get_height()*HEIGHT_FACTOR;
	
	if (start) {
		sy -= start->get_height()*HEIGHT_FACTOR;

		float f = (float)(game->get_gametime() - m_walkstart) / (m_walkend - m_walkstart);
		if (f < 0) f = 0;
		else if (f > 1) f = 1;
		
		ex = (int)(f*ex + (1-f)*sx);
		ey = (int)(f*ey + (1-f)*sy);
	}

	copy_animation_pic(dst, m_anim, game->get_gametime() - m_animstart, ex, ey);
}


/** Map_Object::set_animation(Game* g, Animation *anim)
 *
 * Set a looping animation, starting now.
 */
void Map_Object::set_animation(Game* g, Animation* anim)
{
	m_anim = anim;
	m_animstart = g->get_gametime();
}


/** Map_Object::end_walk()
 *
 * You must call this function when handling act() after a period of walking
 */
void Map_Object::end_walk()
{
	m_walking = IDLE;
}


/** Map_Object::start_walk()
 *
 * Cause the object to walk, honoring passable/impassable parts of the map (checks can_*()).
 *
 * Important: this function schedules a CMD_ACT event which you must deal with correctly,
 *   which is a bit nasty. Ideally, all this acting business should be handled using an
 *   extensible state machine or even better, member function pointers.
 */
bool Map_Object::start_walk(Game *g, WalkingDir dir, Animation *a)
{
	int newx, newy;
	Field *newf;
	
	switch(dir) {
	case IDLE: assert(0); break;
	case WALK_NW: g->get_map()->get_tln(m_px, m_py, m_field, &newx, &newy, &newf); break;
	case WALK_NE: g->get_map()->get_trn(m_px, m_py, m_field, &newx, &newy, &newf); break;
	case WALK_W: g->get_map()->get_ln(m_px, m_py, m_field, &newx, &newy, &newf); break;
	case WALK_E: g->get_map()->get_rn(m_px, m_py, m_field, &newx, &newy, &newf); break;
	case WALK_SW: g->get_map()->get_bln(m_px, m_py, m_field, &newx, &newy, &newf); break;
	case WALK_SE: g->get_map()->get_brn(m_px, m_py, m_field, &newx, &newy, &newf); break;
	}

	// Move capability check
	// :TODO: shouldn't passability be on a per-field basis? this check only honours a
	// single triangle attached to the field.
	// For example, every field could have a bitmap that reflects that movecaps bit;
	// we'd only need a single bitmask for the check
	uchar is = newf->get_terr()->get_is();
	uint movecaps = get_movecaps();
	
	if (is & TERRAIN_WATER && !(movecaps & MOVECAPS_SWIM))
		return false;
	if (is & TERRAIN_UNPASSABLE)
		return false;

	// Move is go
	m_walking = dir;
	m_walkstart = g->get_gametime();
	m_walkend = m_walkstart + 2000; // :TODO: height-based speed changes
	
	set_position(g, newx, newy, newf);
	
	g->get_cmdqueue()->queue(m_walkend, SENDER_MAPOBJECT, CMD_ACT, m_serial, 0, 0);
	
	set_animation(g, a);
	
	return true; // yep, we were successful
}

/** Map_Object::set_position(Game* g, uint x, uint y, Field* f=0)
 *
 * Moves the Map_Object to the given position.
 */
void Map_Object::set_position(Game* g, uint x, uint y, Field* f)
{
	if (m_field) {
		*m_linkpprev = m_linknext;
		if (m_linknext)
			m_linknext->m_linkpprev = m_linkpprev;
	}

	if (!f)
		f = g->get_map()->get_field(x, y);

	m_field = f;		
	m_px = x;
	m_py = y;
	
	m_linknext = f->objects;
	m_linkpprev = &f->objects;
	if (m_linknext)
		m_linknext->m_linkpprev = &m_linknext;
	f->objects = this;
}

