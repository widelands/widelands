/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include <algorithm>

#include "filesystem.h"
#include "map.h"
#include "player.h"
#include "world.h"
#include "worlddata.h"
#include "widelands_map_loader.h"
#include "error.h"

/*
==============================================================================

Map IMPLEMENTATION

==============================================================================
*/

/** Map::Pathfield
 *
 * Used in pathfinding. For better encapsulation, pathfinding structures
 * are seperate from normal fields
 *
 * Costs are in milliseconds to walk.
 *
 * Note: member sizes chosen so that we get a 16byte (=nicely aligned)
 * structure
 */
struct Map::Pathfield {
	int		heap_index;		// index of this field in heap, for backlinking
	int		real_cost;		// true cost up to this field
	int		estim_cost;		// estimated cost till goal
	ushort	cycle;
	uchar		backlink;		// how we got here (Map_Object::WALK_*)

	inline int cost() { return real_cost + estim_cost; }
};

/*
=============================

class S2_Map_Loader

implementation of the S2 Map Loader

=============================
*/

/*
===========
S2_Map_Loader::S2_Map_Loader()

inits the map loader
===========
*/
S2_Map_Loader::S2_Map_Loader(const char* filename, Map* map) :
   Map_Loader(filename, map) {
	snprintf(m_filename, sizeof(m_filename), "%s", filename);
   m_map=map;

}

/*
===========
S2_Map_Loader::~S2_Map_Loader()

cleanups
===========
*/
S2_Map_Loader::~S2_Map_Loader() {
}

/*
===========
S2_Map_Loader::preload_map()

preloads the map. The map will then return valid
infos when get_width() or get_nrplayers(),
get_author() and so on are called

load the header
===========
*/
int S2_Map_Loader::preload_map() {
   assert(get_state()!=STATE_LOADED);

   load_s2mf_header();

   if(!World::exists_world(m_map->get_world_name())) {
      throw wexception("%s: %s", m_map->get_world_name(), "World doesn't exist!");
   }

   set_state(STATE_PRELOADED);

   return 0;
}

/*
===========
S2_Map_Loader::load_map_complete()

Completly loads the map, loads the
corresponding world, loads the graphics
and places all the objects. From now on
the Map* can't be set to another one.
===========
*/
int S2_Map_Loader::load_map_complete(Editor_Game_Base* game) {

   // now, load the world, load the rest infos from the map
   m_map->load_world();
   // Postload the world which provides all the immovables found on a map
   m_map->m_world->postload(game);
   m_map->set_size(m_map->m_width, m_map->m_height);
   load_s2mf(game);


   m_map->recalc_whole_map();

   set_state(STATE_LOADED);

   return 0;
}

/** class Map
 *
 * This really identifies a map like it is in the game
 */

/*
===============
Map::Map

Inits a clean, empty map
===============
*/
Map::Map(void) {
 	m_nrplayers = 0;
	m_width = m_height = 0;
   m_world=0;
	m_pathcycle = 0;
	m_fields = 0;
	m_pathfields = 0;
	m_starting_pos = 0;
	m_world = 0;

   // Paranoia
   cleanup();
}

/*
===============
Map::~Map

cleanups
===============
*/
Map::~Map()
{
   cleanup();
}

/*
===========
Map::recalc_whole_map()

this recalculates all data that needs to be recalculated.
This is only needed when all fields have change, this means
a map has been loaded or newly created
===========
*/
void Map::recalc_whole_map(void)
{
   // Post process the map in the necessary two passes to calculate
   // brightness and building caps
   FCoords f;
   uint y;

   for(y=0; y<m_height; y++) {
      for(uint x=0; x<m_width; x++) {
         int area;

         f = get_fcoords(Coords(x, y));
         check_neighbour_heights(f,&area);
         recalc_brightness(f);
         recalc_fieldcaps_pass1(f);
      }
   }

   for(y=0; y<m_height; y++) {
      for(uint x=0; x<m_width; x++) {
			f = get_fcoords(Coords(x, y));
         recalc_fieldcaps_pass2(f);
      }
   }
}

/*
===============
Map::cleanup

remove your world, remove your data
go back to your initial state
===============
*/
void Map::cleanup(void) {
 	m_nrplayers = 0;
	m_width = m_height = 0;
   m_world=0;
	m_pathcycle = 0;

   if (m_fields)
      free(m_fields);
	m_fields = 0;
	if (m_pathfields)
		free(m_pathfields);
	m_pathfields = 0;
   if (m_starting_pos)
      free(m_starting_pos);
	m_starting_pos = 0;
   if (m_world)
		delete m_world;
	m_world = 0;

}

/*
===========
Map::create_emtpy_map

creates an empty map without name with
the given data
===========
*/
void Map::create_empty_map(int w, int h, std::string worldname) {
   set_world_name(worldname.c_str());
   load_world();
   set_size(w,h);
   set_name("No Name");
   set_author("Unknown");
   set_description("no description defined");

   for(int y=0; y<h; y++) {
      for(int x=0; x<w; x++) {
			FCoords coords = get_fcoords(Coords(x, y));

         coords.field->set_height(10);
         coords.field->set_terraind(get_world()->get_terrain(static_cast<uint>(0)));
         coords.field->set_terrainr(get_world()->get_terrain(static_cast<uint>(0)));
      }
   }
   recalc_whole_map();
}

/*
===============
Map::set_size [private]

Set the size of the map. This should only happen once during initial load.
===============
*/
void Map::set_size(uint w, uint h)
{
   assert(!m_fields);
	assert(!m_pathfields);

	m_width = w;
	m_height = h;

	m_fields = (Field*) malloc(sizeof(Field)*w*h);
	memset(m_fields, 0, sizeof(Field)*w*h);

	m_pathcycle = 0;
	m_pathfields = (Pathfield*)malloc(sizeof(Pathfield)*w*h);
	memset(m_pathfields, 0, sizeof(Pathfield)*w*h);
}

/*
===============
Map::set_nrplayers

Change the number of players the map supports.
Could happen multiple times in the map editor.
===============
*/
void Map::set_nrplayers(uint nrplayers)
{
	if (!nrplayers) {
		if (m_starting_pos)
			free(m_starting_pos);
		m_starting_pos = 0;
		m_nrplayers = 0;
		return;
	}

	m_starting_pos = (Coords*)realloc(m_starting_pos, sizeof(Coords)*nrplayers);
	while(m_nrplayers < nrplayers)
		m_starting_pos[m_nrplayers++] = Coords(0, 0);

	m_nrplayers = nrplayers; // in case the number players got less
}

/*
===============
Map::set_starting_pos

Set the starting coordinates of a player
===============
*/
void Map::set_starting_pos(uint plnum, Coords c)
{
	assert(plnum >= 1 && plnum <= m_nrplayers);

	m_starting_pos[plnum-1] = c;
}

/*
===============
Map::set_author
Map::set_name
Map::set_description
Map::set_world_name

Set informational strings
===============
*/
void Map::set_author(const char *string)
{
	snprintf(m_author, sizeof(m_author), "%s", string);
}

void Map::set_name(const char *string)
{
	snprintf(m_name, sizeof(m_name), "%s", string);
}

void Map::set_description(const char *string)
{
	snprintf(m_description, sizeof(m_description), "%s", string);
}

void Map::set_world_name(const char *string)
{
	snprintf(m_worldname, sizeof(m_worldname), "%s", string);
}

/*
===============
Map::load_world

load the corresponding world. This should only happen
once during initial load.
===============
*/
void Map::load_world(void)
{
   assert(!m_world);

	m_world = new World(m_worldname);
}

/*
===============
Map::load_graphics

Load data for the graphics subsystem.
===============
*/
void Map::load_graphics()
{
	m_world->load_graphics();
}


/*
===============
Map::get_immovable

Returns the immovable at the given coordinate
===============
*/
BaseImmovable *Map::get_immovable(Coords coord)
{
	Field *f = get_field(coord);
	return f->get_immovable();
}


/*
===============
Map::find_reachable

Call the functor for every field that can be reached from coord without moving
outside the given radius.

Functor is of the form: functor(Map*, FCoords)
===============
*/
template<typename functorT>
void Map::find_reachable(Coords coord, uint radius, const CheckStep* checkstep, functorT& functor)
{
	std::vector<Field*> queue;

	increase_pathcycle();

	queue.push_back(get_field(coord));

	while(queue.size()) {
		Pathfield* curpf;
		FCoords cur;

		// Pop the last item from the queue
		cur.field = queue[queue.size() - 1];
		curpf = m_pathfields + (cur.field-m_fields);
		get_coords(cur.field, &cur);
		queue.pop_back();

		// Handle this field
		functor(this, cur);
		curpf->cycle = m_pathcycle;

		// Get neighbours
		for(uint dir = 1; dir <= 6; ++dir) {
			Pathfield *neighbpf;
			FCoords neighb;

			get_neighbour(cur, dir, &neighb);
			neighbpf = m_pathfields + (neighb.field-m_fields);

			// Have we already visited this field?
			if (neighbpf->cycle == m_pathcycle)
				continue;

			// Is the field still within the radius?
			if ((uint)calc_distance(coord, neighb) > radius)
				continue;

			// Are we allowed to move onto this field?
			CheckStep::StepId id;

			if (cur == coord)
				id = CheckStep::stepFirst;
			else
				id = CheckStep::stepNormal;

			if (!checkstep->allowed(this, cur, neighb, dir, id))
				continue;

			// Queue this field
			queue.push_back(neighb.field);
		}
	}
}


/*
===============
Map::find_radius

Call the functor for every field within the given radius.

Functor is of the form: functor(Map*, FCoords)
===============
*/
template<typename functorT>
void Map::find_radius(Coords coord, uint radius, functorT& functor)
{
	MapRegion mr(this, coord, radius);
	FCoords c;

	while(mr.next(&c))
		functor(this, c);
}


/*
===============
FindBobsCallback

The actual logic behind find_bobs and find_reachable_bobs.
===============
*/
struct FindBobsCallback {
	FindBobsCallback(std::vector<Bob*>* list, const FindBob& functor)
		: m_list(list), m_functor(functor), m_found(0) { }

	void operator()(Map* map, FCoords cur)
	{
		Bob *bob;

		for(bob = cur.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
			if (m_list && std::find(m_list->begin(), m_list->end(), bob) != m_list->end())
				continue;

			if (m_functor.accept(bob)) {
				if (m_list)
					m_list->push_back(bob);

				m_found++;
			}
		}
	}

	std::vector<Bob*>*	m_list;
	const FindBob&			m_functor;
	uint						m_found;
};


/*
===============
Map::find_bobs

Find Bobs in the given area. Only finds objects for which
functor.accept() returns true (the default functor always returns true)
If list is non-zero, pointers to the relevant objects will be stored in the list.

Returns the number of objects found.
===============
*/
uint Map::find_bobs(Coords coord, uint radius, std::vector<Bob*>* list, const FindBob &functor)
{
	FindBobsCallback cb(list, functor);

	find_radius(coord, radius, cb);

	return cb.m_found;
}


/*
===============
Map::find_reachable_bobs

Find Bobs that are reachable by moving within the given radius (also see
find_reachable()).
Only finds objects for which functor.accept() returns true (the default functor
always returns true).
If list is non-zero, pointers to the relevant objects will be stored in the list.

Returns the number of objects found.
===============
*/
uint Map::find_reachable_bobs(Coords coord, uint radius, std::vector<Bob*>* list,
										const CheckStep* checkstep, const FindBob &functor)
{
	FindBobsCallback cb(list, functor);

	find_reachable(coord, radius, checkstep, cb);

	return cb.m_found;
}


/*
===============
FindImmovablesCallback

The actual logic behind find_immovables and find_reachable_immovables.
===============
*/
struct FindImmovablesCallback {
	FindImmovablesCallback(std::vector<ImmovableFound>* list, const FindImmovable& functor)
		: m_list(list), m_functor(functor), m_found(0) { }

	void operator()(Map* map, FCoords cur)
	{
		BaseImmovable *imm = cur.field->get_immovable();

		if (!imm)
			return;

		if (m_functor.accept(imm)) {
			if (m_list) {
				ImmovableFound imf;
				imf.object = imm;
				imf.coords = cur;
				m_list->push_back(imf);
			}

			m_found++;
		}
	}

	std::vector<ImmovableFound>*	m_list;
	const FindImmovable&				m_functor;
	uint									m_found;
};


/*
===============
Map::find_immovables

Find all immovables in the given area for which functor returns true
(the default functor always returns true).
Returns true if an immovable has been found.
If list is not 0, found immovables are stored in list.
===============
*/
uint Map::find_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list,
								  const FindImmovable &functor)
{
	FindImmovablesCallback cb(list, functor);

	find_radius(coord, radius, cb);

	return cb.m_found;
}


/*
===============
Map::find_reachable_immovables

Find all immovables reachable by moving in the given radius (see
find_reachable()).
Return immovables for which functor returns true (the default functor
always returns true).
If list is not 0, found immovables are stored in list.
Returns the number of immovables we found.
===============
*/
uint Map::find_reachable_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list,
												const CheckStep* checkstep, const FindImmovable &functor)
{
	FindImmovablesCallback cb(list, functor);

	find_reachable(coord, radius, checkstep, cb);

	return cb.m_found;
}


/*
===============
FindFieldsCallback

The actual logic behind find_fields and find_reachable_fields.
===============
*/
struct FindFieldsCallback {
	FindFieldsCallback(std::vector<Coords>* list, const FindField& functor)
		: m_list(list), m_functor(functor), m_found(0) { }

	void operator()(Map* map, FCoords cur)
	{
		if (m_functor.accept(cur))
		{
			if (m_list)
				m_list->push_back(cur);

			m_found++;
		}
	}

	std::vector<Coords>*	m_list;
	const FindField&		m_functor;
	uint						m_found;
};


/*
===============
Map::find_fields

Fills in a list of coordinates of fields within the given area that functor
accepts.
Returns the number of matching fields.

Note that list can be 0.
===============
*/
uint Map::find_fields(Coords coord, uint radius, std::vector<Coords>* list, const FindField& functor)
{
	FindFieldsCallback cb(list, functor);

	find_radius(coord, radius, cb);

	return cb.m_found;
}


/*
===============
Map::find_reachable_fields

Fills in a list of coordinates of fields reachable by walking within the given
radius that functor accepts.
Returns the number of matching fields.

Note that list can be 0.
===============
*/
uint Map::find_reachable_fields(Coords coord, uint radius, std::vector<Coords>* list,
										  const CheckStep* checkstep, const FindField& functor)
{
	FindFieldsCallback cb(list, functor);

	find_reachable(coord, radius, checkstep, cb);

	return cb.m_found;
}


/*
===============
Map::calc_coords

Calculate the coordinate of the field that is closed to the given point in
pixels. This is the inverse of get_pix().
===============
*/
FCoords Map::calc_coords(Point pos)
{
	FCoords coords;

	// First of all, get a preliminary field coordinate based on the basic
	// grid (not taking heights into account)
	coords.y = (pos.y + (FIELD_HEIGHT>>2)) / (FIELD_HEIGHT>>1) - 1;
	coords.x = pos.x;
	if (coords.y & 1)
		coords.x -= FIELD_WIDTH>>1;
	coords.x = (coords.x + (FIELD_WIDTH>>1)) / FIELD_WIDTH;

	normalize_coords(&coords);

	// Now, coords point to where we'd be if the field's height was 0.
	// We now recursively move towards the correct field. Because height cannot
	// be negative, we only need to consider the bottom-left or bottom-right neighbour
	int mapheight = MULTIPLY_WITH_HALF_FIELD_HEIGHT(get_height());
	int mapwidth = MULTIPLY_WITH_FIELD_WIDTH(get_width());
	int fscrx, fscry;

	coords.field = get_field(coords);
	get_pix(coords, &fscrx, &fscry);

	for(;;) {
		FCoords bln, brn;
		int blscrx, blscry, brscrx, brscry;
		bool movebln, movebrn;
		int fd, blnd, brnd;
		int d2;

		get_bln(coords, &bln);
		get_brn(coords, &brn);

		get_pix(bln, &blscrx, &blscry);
		get_pix(brn, &brscrx, &brscry);

		// determine which field the mouse is closer to on the y-axis
		// bit messy because it has to be aware of map wrap-arounds
		fd = pos.y - fscry;
		d2 = pos.y - mapheight - fscry;
		if (abs(d2) < abs(fd))
			fd = d2;

		blnd = blscry - pos.y;
		d2 = blscry - (pos.y - mapheight);
		if (abs(d2) < abs(blnd))
			blnd = d2;
		movebln = blnd < fd;

		brnd = brscry - pos.y;
		d2 = brscry - (pos.y - mapheight);
		if (abs(d2) < abs(brnd))
			brnd = d2;
		movebrn = brnd < fd;

		if (!movebln && !movebrn)
			break;

		// determine which field is closer on the x-axis
		blnd = pos.x - blscrx;
		d2 = pos.x - mapwidth - blscrx;
		if (abs(d2) < abs(blnd))
			blnd = d2;

		brnd = brscrx - pos.x;
		d2 = brscrx - (pos.x - mapwidth);
		if (abs(d2) < abs(brnd))
			brnd = d2;

		if (brnd < blnd) {
			if (movebrn) {
				coords = brn;
				fscrx = brscrx;
				fscry = brscry;
			} else if (movebln) {
				coords = bln;
				fscrx = blscrx;
				fscry = blscry;
			} else
				break;
		} else {
			if (movebln)  {
				coords = bln;
				fscrx = blscrx;
				fscry = blscry;
			} else if (movebrn) {
				coords = brn;
				fscrx = brscrx;
				fscry = brscry;
			} else
				break;
		}
	}

	return coords;
}


/*
===============
Map::normalize_pix

Normalize pixel points of the map.
===============
*/
void Map::normalize_pix(Point* p)
{
	int mapwidth = MULTIPLY_WITH_FIELD_WIDTH(get_width());
	int mapheight = MULTIPLY_WITH_HALF_FIELD_HEIGHT(get_height());

	while(p->x >= mapwidth)
		p->x -= mapwidth;
	while(p->x < 0)
		p->x += mapwidth;

	while(p->y >= mapheight)
		p->y -= mapheight;
	while(p->y < 0)
		p->y += mapheight;
}


/*
===============
Map::calc_pix_distance

Calculate the pixel (currently Manhattan) distance between the two points,
taking wrap-arounds into account.
===============
*/
int Map::calc_pix_distance(Point a, Point b)
{
	int mapheight = MULTIPLY_WITH_HALF_FIELD_HEIGHT(get_height());
	int mapwidth = MULTIPLY_WITH_FIELD_WIDTH(get_width());
	int dx, dy;

	normalize_pix(&a);
	normalize_pix(&b);

	dx = abs(a.x - b.x);
	if (dx > mapwidth/2)
		dx = -(dx - mapwidth);

	dy = abs(a.y - b.y);
	if (dy > mapheight/2)
		dy = -(dy - mapheight);

	return dx + dy;
}


/*
Field attribute recalculation passes
------------------------------------

Some events can change the map in a way that run-time calculated attributes
(Field::brightness and Field::caps) need to be recalculated.

These events include:
- change of height (e.g. by planing)
- change of terrain (in the editor)
- insertion of a "robust" Map_Object
- removal of a "robust" Map_Object

All these events can change the passability, buildability, etc. of fields
with a radius of two fields. This means that you must build a list of the
directly affected field and all fields that can be reached in two steps.

You must then perform the following operations:
1. Call recalc_brightness() and recalc_fieldcaps_pass1() on all fields
2. Call recalc_fieldcaps_pass2() on all fields

Note: it is possible to leave out recalc_brightness() unless the height has
been changed.

The Field::caps calculation is split into more passes because of inter-field
dependencies.
*/

/*
===============
Map::recalc_brightness

Fetch the slopes to neighbours and call the actual logic in Field
===============
*/
void Map::recalc_brightness(FCoords f)
{
	FCoords d;
   int l, r, tl, tr, bl, br;

   // left
   get_ln(f, &d);
   l = f.field->get_height() - d.field->get_height();

   // right
   get_rn(f, &d);
   r = f.field->get_height() - d.field->get_height();

   // top-left
   get_tln(f, &d);
   tl = f.field->get_height() - d.field->get_height();

   // top-right
   get_rn(d, &d);
   tr = f.field->get_height() - d.field->get_height();

   // bottom-left
   get_bln(f, &d);
   bl = f.field->get_height() - d.field->get_height();

   // bottom-right
   get_rn(d, &d);
   br = f.field->get_height() - d.field->get_height();

   f.field->set_brightness(l, r, tl, tr, bl, br);
}


/*
===============
Map::recalc_fieldcaps_pass1

Recalculate the fieldcaps for the given field.
 - Check terrain types for passability and flag buildability

I hope this is understandable and maintainable.

Note: due to inter-field dependencies, fieldcaps calculations are split up
into two passes. You should always perform both passes. See the comment
above recalc_brightness.
===============
*/
void Map::recalc_fieldcaps_pass1(FCoords f)
{
   f.field->caps = 0;


   // 1a) Get all the neighbours to make life easier
	FCoords tln, trn, ln, rn, bln, brn;

   get_tln(f, &tln);
   get_trn(f, &trn);
   get_ln(f, &ln);
   get_rn(f, &rn);
   get_bln(f, &bln);
   get_brn(f, &brn);

   // 1b) Collect some information about the neighbours
   int cnt_unpassable = 0;
   int cnt_water = 0;
   int cnt_acid = 0;

   if (f.field->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (f.field->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (ln.field->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (tln.field->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (tln.field->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (trn.field->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;

   if (f.field->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
   if (f.field->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
   if (ln.field->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
   if (tln.field->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
   if (tln.field->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
   if (trn.field->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;

   if (f.field->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (f.field->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (ln.field->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (tln.field->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (tln.field->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (trn.field->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;


   // 2) Passability

   // 2a) If any of the neigbouring triangles is walkable this field is
   //     walkable.
   if (cnt_unpassable < 6)
      f.field->caps |= MOVECAPS_WALK;

   // 2b) If all neighbouring triangles are water, the field is swimable
   if (cnt_water == 6)
      f.field->caps |= MOVECAPS_SWIM;


   // 2c) [OVERRIDE] If any of the neighbouring triangles is really
   //     "bad" (such as lava), we can neither walk nor swim to this field.
   if (cnt_acid)
      f.field->caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);

   // === everything below is used to check buildability ===

   // 3) General buildability check: if a "robust" Map_Object is on this field
   //    we cannot build anything on it.
   //    Exception: we can build flags on roads
   BaseImmovable *imm = get_immovable(f);

   if (imm && imm->get_type() != Map_Object::ROAD && imm->get_size() >= BaseImmovable::SMALL)
   {
      // 3b) [OVERRIDE] check for "unpassable" Map_Objects
      if (!imm->get_passable())
         f.field->caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
      return;
   }

   // 4) Flags
   //    We can build flags on anything that's walkable and buildable, with some
   //    restrictions
   if (f.field->caps & MOVECAPS_WALK)
   {
      // 4b) Flags must be at least 1 field apart
      if (find_immovables(f, 1, 0, FindImmovableType(Map_Object::FLAG))) {
         return;
      }
      f.field->caps |= BUILDCAPS_FLAG;
   }
}


/*
===============
Map::recalc_fieldcaps_pass2

Second pass of fieldcaps. Determine which kind of building (if any) can be built
on this Field.

Important: flag buildability has already been checked in the first pass.
===============
*/
void Map::recalc_fieldcaps_pass2(FCoords f)
{
	// 1) Collect neighbour information
	//
	// NOTE: Yes, this reproduces some of the things done in pass1.
	// This is unavoidable and shouldn't hurt too much anyway.

	// 1a) Get all the neighbours to make life easier
	FCoords tln, trn, ln, rn, bln, brn;

   get_tln(f, &tln);
   get_trn(f, &trn);
   get_ln(f, &ln);
   get_rn(f, &rn);
   get_bln(f, &bln);
   get_brn(f, &brn);

	// 1b) Collect some information about the neighbours
	int cnt_unpassable = 0;
	int cnt_water = 0;
	int cnt_acid = 0;
	int cnt_mountain = 0;
	int cnt_dry = 0;

	if (f.field->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (f.field->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (ln.field->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln.field->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln.field->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (trn.field->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;

	if (f.field->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (f.field->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (ln.field->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln.field->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln.field->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (trn.field->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;

	if (f.field->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (f.field->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (ln.field->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln.field->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln.field->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (trn.field->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;

	if (f.field->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (f.field->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (ln.field->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (tln.field->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (tln.field->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (trn.field->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;

	if (f.field->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (f.field->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (ln.field->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (tln.field->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (tln.field->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (trn.field->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;

	// 2) We can only build something on fields that are
	//     - walkable
	//     - have no water triangles next to them
	//     - are not blocked by "robust" Map_Objects
	BaseImmovable *immovable = get_immovable(f);

	if (!(f.field->caps & MOVECAPS_WALK) ||
	    cnt_water ||
	    (immovable && immovable->get_size() >= BaseImmovable::SMALL))
		return;

	// 3) We can only build something if there is a flag on the bottom-right neighbour
	//    (or if we could build a flag on the bottom-right neighbour)
	//
	// NOTE: This dependency on the bottom-right neighbour is the reason why the caps
	// calculation is split into two passes
	if (!(brn.field->caps & BUILDCAPS_FLAG) &&
	    !find_immovables(brn, 0, 0, FindImmovableType(Map_Object::FLAG)))
		return;

	// === passability and flags allow us to build something beyond this point ===

	// 4) Reduce building size based on nearby objects (incl. buildings) and roads.
	//
	// Small object: allow medium-sized first-order neighbour, big second-order neighbour
	// Medium object: allow small-sized first-order neighbour, big second-order neighbour
	// Big object: allow no first-order neighbours, small second-order neighbours
	//
	// Small buildings: same as small objects
	// Medium buildings: allow only medium second-order neighbours
	// Big buildings:  same as big objects
	uchar building = BUILDCAPS_BIG;
	std::vector<ImmovableFound> objectlist;

	find_immovables(f, 2, &objectlist, FindImmovableSize(BaseImmovable::SMALL, BaseImmovable::BIG));
	for(uint i = 0; i < objectlist.size(); i++) {
		BaseImmovable *obj = objectlist[i].object;
		Coords objpos = objectlist[i].coords;
		int dist = calc_distance(f, objpos);

		switch(obj->get_size()) {
		case BaseImmovable::SMALL:
			if (dist == 1) {
		 		if (building > BUILDCAPS_MEDIUM) {
					// a flag to the bottom-right does not reduce building size (obvious)
					// additionally, roads going top-right and left from a big building's
					// flag should be allowed
					if ((objpos != brn && objpos != rn && objpos != bln) ||
					    (obj->get_type() != Map_Object::FLAG && obj->get_type() != Map_Object::ROAD))
						building = BUILDCAPS_MEDIUM;
				}
			}
			break;

		case BaseImmovable::MEDIUM:
			if (obj->get_type() == Map_Object::BUILDING) {
				if (building > BUILDCAPS_MEDIUM)
					building = BUILDCAPS_MEDIUM;
			} else {
				if (dist == 1) {
					if (building > BUILDCAPS_SMALL)
						building = BUILDCAPS_SMALL;
				}
			}
			break;

		case BaseImmovable::BIG:
			if (dist == 2)
				building = BUILDCAPS_SMALL;
			else
				return; // can't build buildings next to big objects
		}
	}

	// 5) Build mines on mountains
	if (cnt_mountain == 6)
	{
		// 4b) Check the mountain slope
		if ((int)brn.field->get_height() - f.field->get_height() < 4)
			f.field->caps |= BUILDCAPS_MINE;
		return;
	}

	// 6) Can't build anything if there are mountain or desert triangles next to the field
	if (cnt_mountain || cnt_dry)
		return;

	// 7) Reduce building size based on slope of direct neighbours:
	//    - slope >= 4: can't build anything here -> return
	//    - slope >= 3: maximum size is small
	int slope;

	slope = abs((int)tln.field->get_height() - f.field->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	slope = abs((int)trn.field->get_height() - f.field->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	slope = abs((int)rn.field->get_height() - f.field->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	// Special case for bottom-right neighbour (where our flag is)
	// Is this correct?
   // Yep, it is - Holger
	slope = abs((int)brn.field->get_height() - f.field->get_height());
	if (slope >= 2) return;

	slope = abs((int)bln.field->get_height() - f.field->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	slope = abs((int)ln.field->get_height() - f.field->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	// 8) Reduce building size based on height diff. of second order neighbours
	//     If height difference between this field and second order neighbour
	//     is >= 3, we can only build a small house here.
	//    Additionally, we can potentially build a harbour on this field if one
	//    of the second order neighbours is swimmable.
   //    TODO: didn't we agree that it would be easier to make harbours only on specially set
   //    fields? - Holger
	//    Ah right. Well, for the final decision, we'd probably have to see how
	//    ships work out. If they become an important part in day-to-day gameplay,
	//    an automatic method will probably work better.
	//    However, it should probably be more clever than this, to avoid harbours at tiny
	//    lakes... - Nicolai
	//
	// Processes the neighbours in clockwise order, starting from the right-most
	FCoords sec;

	get_rn(rn, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_brn(rn, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_brn(brn, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_bln(brn, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_bln(bln, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_ln(bln, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_ln(ln, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_tln(ln, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_tln(tln, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_trn(tln, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_trn(trn, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_rn(trn, &sec);
	if (abs((int)sec.field->get_height() - f.field->get_height()) >= 3) building = BUILDCAPS_SMALL;

	// 9) That's it, store the collected information
	f.field->caps |= building;
}


/** Map::calc_distance(Coords a, Coords b)
 *
 * Calculate the (Manhattan) distance from a to b
 * a and b are expected to be normalized!
 */
int Map::calc_distance(Coords a, Coords b)
{
	uint dist;
	int dy;

	// do we fly up or down?
	dy = b.y - a.y;
	if (dy > (int)(m_height>>1)) // wrap-around!
		dy -= m_height;
	else if (dy < -(int)(m_height>>1))
		dy += m_height;

	dist = abs(dy);

	if (dist >= m_width) // no need to worry about x movement at all
		return dist;

	// [lx..rx] is the x-range we can cover simply by walking vertically
	// towards b
	// Hint: (~a.y & 1) is 1 for even rows, 0 for odd rows.
	// This means we round UP for even rows, and we round DOWN for odd rows.
	int lx, rx;

	lx = a.x - ((dist + (~a.y & 1))>>1); // div 2
	rx = lx + dist;

	// Allow for wrap-around
	// Yes, the second is an else if; see the above if (dist >= m_width)
	if (lx < 0)
		lx += m_width;
	else if (rx >= (int)m_width)
		rx -= m_width;

	// Normal, non-wrapping case
	if (lx <= rx)
	{
		if (b.x < lx)
		{
			int dx1 = lx - b.x;
			int dx2 = b.x - (rx - m_width);
			dist += std::min(dx1, dx2);
		}
		else if (b.x > rx)
		{
			int dx1 = b.x - rx;
			int dx2 = (lx + m_width) - b.x;
			dist += std::min(dx1, dx2);
		}
	}
	else
	{
		// Reverse case
		if (b.x > rx && b.x < lx)
		{
			int dx1 = b.x - rx;
			int dx2 = lx - b.x;
			dist += std::min(dx1, dx2);
		}
	}

	return dist;
}

/*
===============
Map::is_neighbour

Find out if the two given fields are neighbours.
If they aren't, the function returns 0. If they are the same, return -1.
Otherwise, it returns the direction from start to end
===============
*/
int Map::is_neighbour(const Coords start, const Coords end)
{
	int dx, dy;

	dy = end.y - start.y;
	dx = end.x - start.x;
	if (dx > (int)(m_width>>1))
		dx -= m_width;
	else if (dx < -(int)(m_width>>1))
		dx += m_width;

	// end and start are on the same row
	if (!dy) {
		switch(dx) {
		case -1: return Map_Object::WALK_W;
		case 0: return -1;
		case 1: return Map_Object::WALK_E;
		default: return 0;
		}
	}

	if (dy > (int)(m_height>>1))
		dy -= m_height;
	else if (dy < -(int)(m_height>>1))
		dy += m_height;

	// end is one row below start
	if (dy == 1) {
		if (start.y & 1)
			dx--;

		switch(dx) {
		case -1: return Map_Object::WALK_SW;
		case 0: return Map_Object::WALK_SE;
		default: return 0;
		}
	}

	// end is one row above start
	if (dy == -1) {
		if (start.y & 1)
			dx--;

		switch(dx) {
		case -1: return Map_Object::WALK_NW;
		case 0: return Map_Object::WALK_NE;
		default: return 0;
		}
	}

	return 0;
}


#define BASE_COST_PER_FIELD		1800
#define SLOPE_COST_FACTOR			0.02
#define SLOPE_COST_STEPS			8

/*
===============
Map::calc_cost_estimate

Calculates the cost estimate between the two points.
This function is used mainly for the path-finding estimate.
===============
*/
int Map::calc_cost_estimate(Coords a, Coords b)
{
	return calc_distance(a, b) * BASE_COST_PER_FIELD;
}


/*
===============
Map::calc_cost

Calculate the hard cost of walking the given slope (positive means up,
negative means down).
The cost is in milliseconds it takes to walk.

The time is calculated as BASE_COST_PER_FIELD * f, where

f = base + d(Slope)
d = SLOPE_COST_FACTOR * (Slope + SLOPE_COST_STEPS)*(Slope + SLOPE_COST_STEPS - 1) / 2
base = 1.0 - d(0)

Slope is limited to the range [ -SLOPE_COST_STEPS; +oo [
===============
*/
#define CALC_COST_D(slope)		(SLOPE_COST_FACTOR * ((slope) + SLOPE_COST_STEPS) * ((slope) + SLOPE_COST_STEPS - 1) * 0.5)
#define CALC_COST_BASE			(1.0 - CALC_COST_D(0))

static inline float calc_cost_d(int slope)
{
	if (slope < -SLOPE_COST_STEPS)
		slope = -SLOPE_COST_STEPS;

	return CALC_COST_D(slope);
}

int Map::calc_cost(int slope)
{
	return (int)(BASE_COST_PER_FIELD * (CALC_COST_BASE + calc_cost_d(slope)));
}


/*
===============
Map::calc_cost

Return the time it takes to walk the given step from coords in the given
direction, in milliseconds.
===============
*/
int Map::calc_cost(Coords coords, int dir)
{
	FCoords f;
	int startheight;
	int delta;

	// Calculate the height delta
	f = get_fcoords(coords);
	startheight = f.field->get_height();

	get_neighbour(f, dir, &f);
	delta = f.field->get_height() - startheight;

	return calc_cost(delta);
}


/*
===============
Map::calc_bidi_cost

Calculate the average cost of walking the given step in both directions.
===============
*/
int Map::calc_bidi_cost(Coords coords, int dir)
{
	FCoords f;
	int startheight;
	int delta;

	// Calculate the height delta
	f = get_fcoords(coords);
	startheight = f.field->get_height();

	get_neighbour(f, dir, &f);
	delta = f.field->get_height() - startheight;

	return (calc_cost(delta) + calc_cost(-delta)) / 2;
}


/*
===============
Map::calc_cost

Calculate the cost of walking the given path.
If either of the forward or backward pointers is set, it will be filled in
with the cost of walking in said direction.
===============
*/
void Map::calc_cost(const Path &path, int *forward, int *backward)
{
	Coords coords = path.get_start();

	if (forward)
		*forward = 0;
	if (backward)
		*backward = 0;

	for(int i = 0; i < path.get_nsteps(); i++) {
		int dir = path.get_step(i);

		if (forward)
			*forward += calc_cost(coords, dir);
		get_neighbour(coords, dir, &coords);
		if (backward)
			*backward += calc_cost(coords, get_reverse_dir(dir));
	}
}

/*
===============
Map::get_neighbour

Get a field's neighbour by direction
===============
*/
void Map::get_neighbour(const Coords f, int dir, Coords * const o)
{
	switch(dir) {
	case Map_Object::WALK_NW: get_tln(f, o); break;
	case Map_Object::WALK_NE: get_trn(f, o); break;
	case Map_Object::WALK_E: get_rn(f, o); break;
	case Map_Object::WALK_SE: get_brn(f, o); break;
	case Map_Object::WALK_SW: get_bln(f, o); break;
	case Map_Object::WALK_W: get_ln(f, o); break;
	}
}

void Map::get_neighbour(const FCoords f, int dir, FCoords * const o)
{
	switch(dir) {
	case Map_Object::WALK_NW: get_tln(f, o); break;
	case Map_Object::WALK_NE: get_trn(f, o); break;
	case Map_Object::WALK_E: get_rn(f, o); break;
	case Map_Object::WALK_SE: get_brn(f, o); break;
	case Map_Object::WALK_SW: get_bln(f, o); break;
	case Map_Object::WALK_W: get_ln(f, o); break;
	}
}

/*
===========
Map::get_correct_loader()

returns the correct initialized loader for
the given mapfile
===========
*/
Map_Loader* Map::get_correct_loader(const char* filename) {
   Map_Loader* retval=0;

   if(!strcasecmp(filename+(strlen(filename)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
   {
      retval=new Widelands_Map_Loader(filename, this);
   }
   else if (!strcasecmp(filename+(strlen(filename)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
   {
      // it is a S2 Map file. load it as such
      retval=new S2_Map_Loader(filename, this);
   }
   else
      throw wexception("Map filename %s has unknown extension", filename);

   return retval;
}

/** class StarQueue
 *
 * Provides the flexible priority queue to maintain the open list.
 */
class StarQueue {
	std::vector<Map::Pathfield*> m_data;

public:
	StarQueue() { }
	~StarQueue() { }

	void flush() { m_data.clear(); }

	// Return the best node and readjust the tree
	// Basic idea behind the algorithm:
	//  1. the top slot of the tree is empty
	//  2. if this slot has both children:
	//       fill this slot with one of its children or with slot[_size], whichever
	//       is best;
	//       if we filled with slot[_size], stop
	//       otherwise, repeat the algorithm with the child slot
	//     if it doesn't have any children (l >= _size)
	//       put slot[_size] in its place and stop
	//     if only the left child is there
	//       arrange left child and slot[_size] correctly and stop
	Map::Pathfield* pop()
	{
		if (m_data.empty())
			return 0;

		Map::Pathfield* head = m_data[0];

		unsigned nsize = m_data.size()-1;
		unsigned fix = 0;
		while(fix < nsize) {
			unsigned l = fix*2 + 1;
			unsigned r = fix*2 + 2;
			if (l >= nsize) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->heap_index = fix;
				break;
			}
			if (r >= nsize) {
				if (m_data[nsize]->cost() <= m_data[l]->cost()) {
					m_data[fix] = m_data[nsize];
					m_data[fix]->heap_index = fix;
				} else {
					m_data[fix] = m_data[l];
					m_data[fix]->heap_index = fix;
					m_data[l] = m_data[nsize];
					m_data[l]->heap_index = l;
				}
				break;
			}

			if (m_data[nsize]->cost() <= m_data[l]->cost() && m_data[nsize]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->heap_index = fix;
				break;
			}
			if (m_data[l]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[l];
				m_data[fix]->heap_index = fix;
				fix = l;
			} else {
				m_data[fix] = m_data[r];
				m_data[fix]->heap_index = fix;
				fix = r;
			}
		}

		m_data.pop_back();

		debug(0, "pop");

		head->heap_index = -1;
		return head;
	}

	// Add a new node and readjust the tree
	// Basic idea:
	//  1. Put the new node in the last slot
	//  2. If parent slot is worse than self, exchange places and recurse
	// Note that I rearranged this a bit so swap isn't necessary
	void push(Map::Pathfield *t)
	{
		unsigned slot = m_data.size();
		m_data.push_back(0);

		while(slot > 0) {
			unsigned parent = (slot - 1) / 2;

			if (m_data[parent]->cost() < t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->heap_index = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->heap_index = slot;

		debug(0, "push");
	}

	// Rearrange the tree after a node has become better, i.e. move the
	// node up
	// Pushing algorithm is basically the same as in push()
	void boost(Map::Pathfield *t)
	{
		unsigned slot = t->heap_index;

		assert(m_data[slot] == t);

		while(slot > 0) {
			unsigned parent = (slot - 1) / 2;

			if (m_data[parent]->cost() <= t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->heap_index = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->heap_index = slot;

		debug(0, "boost");
	}

	// Recursively check integrity
	void debug(unsigned node, const char *str)
	{
		unsigned l = node*2 + 1;
		unsigned r = node*2 + 2;
		if (m_data[node]->heap_index != (int)node) {
			fprintf(stderr, "%s: heap_index integrity!\n", str);
			exit(-1);
		}
		if (l < m_data.size()) {
			if (m_data[node]->cost() > m_data[l]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				exit(-1);
			}
			debug(l, str);
		}
		if (r < m_data.size()) {
			if (m_data[node]->cost() > m_data[r]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				exit(-1);
			}
			debug(r, str);
		}
	}

};


/*
===============
Map::increase_pathcycle

Increment pathcycle and clear the pathfields array if necessary.
===============
*/
void Map::increase_pathcycle()
{
	m_pathcycle++;
	if (!m_pathcycle) {
		memset(m_pathfields, 0, sizeof(Pathfield)*m_height*m_width);
		m_pathcycle++;
	}
}


/*
===============
Map::findpath

Pathfinding entry-point.
Finds a path from start to end for a Map_Object with the given movecaps.

The path is stored in path, in terms of Map_Object::MOVE_* constants.
persist tells the function how hard it should try to find a path:
If persist is 0, the function will never give up early. Otherwise, the
function gives up when it becomes clear that the path must be longer than
persist*bird's distance fields long.
[not implement right now]: If the terrain contains lots of hills, the cost
calculation will cause the search to terminate earlier than this. If persist==1,
findpath() can only find a path if the terrain is completely flat.

findpath() calls the checkstep functor-like to determine whether moving from
one field to another is legal.

The function returns the cost of the path (in milliseconds of normal walking
speed) or -1 if no path has been found.
===============
*/
int Map::findpath(Coords instart, Coords inend, int persist, Path *path,
                  const CheckStep* checkstep, uint flags)
{
	FCoords start;
	FCoords end;
	int upper_cost_limit;
	FCoords cur;

	normalize_coords(&instart);
	normalize_coords(&inend);

	start = FCoords(instart, get_field(instart));
	end =	FCoords(inend, get_field(inend));

	path->m_path.clear();

	// Some stupid cases...
	if (start == end) {
		path->m_map = this;
		path->m_start = start;
		path->m_end = end;
		return 0; // duh...
	}

	if (!checkstep->reachabledest(this, end))
		return -1;

	// Increase the counter
	// If the counter wrapped, clear the entire pathfinding array
	// This means we clear the array only every 65536 runs
	increase_pathcycle();

	if (!persist)
		upper_cost_limit = 0;
	else
		upper_cost_limit = persist * calc_cost_estimate(start, end); // assume flat terrain

	// Actual pathfinding
	StarQueue Open;
	Pathfield *curpf;

	curpf = m_pathfields + (start.field-m_fields);
	curpf->cycle = m_pathcycle;
	curpf->real_cost = 0;
	curpf->estim_cost = calc_cost_estimate(start, end);
	curpf->backlink = Map_Object::IDLE;

	Open.push(curpf);

	while((curpf = Open.pop()))
	{
		cur.field = m_fields + (curpf-m_pathfields);
		get_coords(cur.field, &cur);

		if (upper_cost_limit && curpf->real_cost > upper_cost_limit)
			break; // upper cost limit reached, give up
		if (cur == end)
			break; // found our target

		// avoid bias by using different orders when pathfinding
		static const char order1[6] = { Map_Object::WALK_NW, Map_Object::WALK_NE,
			Map_Object::WALK_E, Map_Object::WALK_SE, Map_Object::WALK_SW, Map_Object::WALK_W };
		static const char order2[6] = { Map_Object::WALK_NW, Map_Object::WALK_W,
			Map_Object::WALK_SW, Map_Object::WALK_SE, Map_Object::WALK_E, Map_Object::WALK_NE };
		const char *direction;

		if ((cur.x+cur.y) & 1)
			direction = order1;
		else
			direction = order2;

		// Check all the 6 neighbours
		for(uint i = 6; i; i--, direction++) {
			Pathfield *neighbpf;
			FCoords neighb;
			int cost;

			get_neighbour(cur, *direction, &neighb);
			neighbpf = m_pathfields + (neighb.field-m_fields);

			// Is the field Closed already?
			if (neighbpf->cycle == m_pathcycle && neighbpf->heap_index < 0)
				continue;

			// Check passability
			CheckStep::StepId id;

			if (neighb == end)
				id = CheckStep::stepLast;
			else if (cur == start)
				id = CheckStep::stepFirst;
			else
				id = CheckStep::stepNormal;

			if (!checkstep->allowed(this, cur, neighb, *direction, id))
				continue;

			// Calculate cost
			if (flags & fpBidiCost)
				cost = curpf->real_cost + calc_bidi_cost(Coords(cur.x, cur.y), *direction);
			else
				cost = curpf->real_cost + calc_cost(Coords(cur.x, cur.y), *direction);

			if (neighbpf->cycle != m_pathcycle) {
				// add to open list
				neighbpf->cycle = m_pathcycle;
				neighbpf->real_cost = cost;
				// use a slightly lower estimate for better accuracy
				neighbpf->estim_cost = calc_cost_estimate(neighb, end) >> 1;
				neighbpf->backlink = *direction;
				Open.push(neighbpf);
			} else if (neighbpf->cost() > cost+neighbpf->estim_cost) {
				// found a better path to a field that's already Open
				neighbpf->real_cost = cost;
				neighbpf->backlink = *direction;
				Open.boost(neighbpf);
			}
		}
	}
	if (!curpf) // there simply is no path
		return -1;

	// Now unwind the taken route (even if we couldn't find a complete one!)
	int retval;

	if (cur == end)
		retval = curpf->real_cost;
	else
		retval = -1;

	path->m_map = this;
	path->m_start = start;
	path->m_end = cur;

	path->m_path.clear();

	while(curpf->backlink != Map_Object::IDLE) {
		path->m_path.push_back(curpf->backlink);

		// Reverse logic! (WALK_NW needs to find the SE neighbour)
		get_neighbour(cur, get_reverse_dir(curpf->backlink), &cur);
		curpf = m_pathfields + (cur.field-m_fields);
	}

	return retval;
}


/** Map::can_reach_by_water(Coords field)
 *
 * We can reach a field by water either if it has MOVECAPS_SWIM or if it has
 * MOVECAPS_WALK and at least one of the neighbours has MOVECAPS_SWIM
 */
bool Map::can_reach_by_water(Coords field)
{
	FCoords fc = get_fcoords(field);

	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;
	if (!(fc.field->get_caps() & MOVECAPS_WALK))
		return false;

	FCoords neighb;

	get_tln(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM) return true;

	get_trn(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM) return true;

	get_rn(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM) return true;

	get_brn(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM) return true;

	get_bln(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM) return true;

	get_ln(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM) return true;

	return false;
}

/*
===========
Map::change_field_terrain

changes the given field terrain to another one.
this happens in the editor and might happen in the game
too if some kind of land increasement is implemented (like
drying swamps).
The fieldcaps need to be recalculated

returns the radius of changes (which are always 2)
===========
*/
int Map::change_field_terrain(Coords c, int terrain, bool tdown, bool tright)
{
   assert(tdown || tright);

   Field* f=get_field(c);

   Terrain_Descr* ter=get_world()->get_terrain(terrain);

   if(tdown)
      f->set_terraind(ter);
   if(tright)
      f->set_terrainr(ter);

   MapRegion mr;
   FCoords coords;

	mr.init(this,c,2);
   while(mr.next(&coords)) {
      recalc_fieldcaps_pass1(coords);
   }

   mr.init(this,c,2);
   while(mr.next(&coords)) {
      recalc_fieldcaps_pass2(coords);
   }

   return 2;
}


/*
===========
Map::set_field_height()

sets the field height to an absolut value. This changes the surrounding
fields are changed as well
===========
*/
int Map::set_field_height(Coords coordinates, int to)
{
   Field* m_field=get_field(coordinates);
   int val=m_field->get_height();
   int diff=to-val;
   return change_field_height(coordinates, diff);
}

/*
===========
Map::change_field_height()

relativly change field height, recalculate brightness and
if needed change surrounding fields so that walking is still
possible

returns the area of fields, that have been changed
===========
*/
int Map::change_field_height(Coords coords, int by)
{
	FCoords coordinates = get_fcoords(coords);
   int height = coordinates.field->get_height();

   height+=by;
   coordinates.field->set_height(height);
   int radius=0;

   check_neighbour_heights(coordinates, &radius);
   radius += 2;

   // First recalculation step
   MapRegion mr;
	FCoords c;

	mr.init(this, coordinates, radius);
	while(mr.next(&c)) {
      recalc_brightness(c);
      recalc_fieldcaps_pass1(c);
   }

   // Second recalculation step
   mr.init(this, coordinates, radius);
   while(mr.next(&c)) {
      recalc_fieldcaps_pass2(c);
   }

   return radius;
}


/*
===========
Map::check_neighbour_heights()

This private functions checks all neighbours of a field
if they are in valid boundaries, if not, they are reheighted
accordingly.
The radius of modified fields is stored in *area.
=============
*/
void Map::check_neighbour_heights(FCoords coords, int* area)
{
   int height = coords.field->get_height();
   int diff;
   bool check[6] = { false, false, false, false, false, false };


   FCoords tln, trn, ln, rn, bln, brn;

   get_tln(coords, &tln);
   get_trn(coords, &trn);
   get_ln(coords, &ln);
   get_rn(coords, &rn);
   get_bln(coords, &bln);
   get_brn(coords, &brn);

   diff=height-tln.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; tln.field->set_height(height-MAX_FIELD_HEIGHT_DIFF); check[0]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; tln.field->set_height(height+MAX_FIELD_HEIGHT_DIFF); check[0]=true; }

   diff=height-trn.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; trn.field->set_height(height-MAX_FIELD_HEIGHT_DIFF); check[1]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; trn.field->set_height(height+MAX_FIELD_HEIGHT_DIFF); check[1]=true; }

   diff=height-ln.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; ln.field->set_height(height-MAX_FIELD_HEIGHT_DIFF); check[2]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; ln.field->set_height(height+MAX_FIELD_HEIGHT_DIFF); check[2]=true; }

   diff=height-rn.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; rn.field->set_height(height-MAX_FIELD_HEIGHT_DIFF); check[3]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; rn.field->set_height(height+MAX_FIELD_HEIGHT_DIFF); check[3]=true; }

   diff=height-bln.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; bln.field->set_height(height-MAX_FIELD_HEIGHT_DIFF); check[4]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; bln.field->set_height(height+MAX_FIELD_HEIGHT_DIFF); check[4]=true; }

   diff=height-brn.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; brn.field->set_height(height-MAX_FIELD_HEIGHT_DIFF); check[5]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; brn.field->set_height(height+MAX_FIELD_HEIGHT_DIFF); check[5]=true; }

   if(check[0]) check_neighbour_heights(tln, area);
   if(check[1]) check_neighbour_heights(trn, area);
   if(check[2]) check_neighbour_heights(ln, area);
   if(check[3]) check_neighbour_heights(rn, area);
   if(check[4]) check_neighbour_heights(bln, area);
   if(check[5]) check_neighbour_heights(brn, area);
}


/*
==============================================================================

BaseImmovable search functors

==============================================================================
*/

bool FindImmovableSize::accept(BaseImmovable *imm) const
{
	int size = imm->get_size();
	return (m_min <= size && size <= m_max);
}

bool FindImmovableType::accept(BaseImmovable *imm) const
{
	return (m_type == imm->get_type());
}

bool FindImmovableAttribute::accept(BaseImmovable *imm) const
{
	return imm->has_attribute(m_attrib);
}

bool FindImmovablePlayerImmovable::accept(BaseImmovable* imm) const
{
	return imm->get_type() >= Map_Object::BUILDING;
}


/*
==============================================================================

Field search functors

==============================================================================
*/


void FindFieldAnd::add(const FindField* findfield, bool negate)
{
	Subfunctor sf;

	sf.negate = negate;
	sf.findfield = findfield;

	m_subfunctors.push_back(sf);
}

bool FindFieldAnd::accept(FCoords coord) const
{
	for(std::vector<Subfunctor>::const_iterator it = m_subfunctors.begin();
	    it != m_subfunctors.end();
		 ++it)
	{
		if (it->findfield->accept(coord) == it->negate)
			return false;
	}

	return true;
}


bool FindFieldCaps::accept(FCoords coord) const
{
	uchar fieldcaps = coord.field->get_caps();

	if ((fieldcaps & BUILDCAPS_SIZEMASK) < (m_mincaps & BUILDCAPS_SIZEMASK))
		return false;

	if ((m_mincaps & ~BUILDCAPS_SIZEMASK) & ~(fieldcaps & ~BUILDCAPS_SIZEMASK))
		return false;

	return true;
}

bool FindFieldSize::accept(FCoords coord) const
{
	BaseImmovable* imm = coord.field->get_immovable();
	bool hasrobust = (imm && imm->get_size() > BaseImmovable::NONE);
	uchar fieldcaps = coord.field->get_caps();

	if (hasrobust)
		return false;

	switch(m_size) {
	default:
	case sizeAny:		return true;
	case sizeBuild:	return (fieldcaps & (BUILDCAPS_SIZEMASK | BUILDCAPS_FLAG | BUILDCAPS_MINE));
	case sizeMine:		return (fieldcaps & BUILDCAPS_MINE);
	case sizePort:		return (fieldcaps & BUILDCAPS_PORT);
	case sizeSmall:	return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_SMALL;
	case sizeMedium:	return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_MEDIUM;
	case sizeBig:		return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_BIG;
	}
}

bool FindFieldImmovableSize::accept(FCoords coord) const
{
	BaseImmovable* imm = coord.field->get_immovable();
	int size = BaseImmovable::NONE;

	if (imm)
		size = imm->get_size();

	switch(size) {
	case BaseImmovable::NONE: return m_sizes & sizeNone;
	case BaseImmovable::SMALL: return m_sizes & sizeSmall;
	case BaseImmovable::MEDIUM: return m_sizes & sizeMedium;
	case BaseImmovable::BIG: return m_sizes & sizeBig;
	default: throw wexception("FindFieldImmovableSize: bad size = %i", size);
	}
}


bool FindFieldImmovableAttribute::accept(FCoords coord) const
{
	BaseImmovable* imm = coord.field->get_immovable();

	if (!imm)
		return false;

	return imm->has_attribute(m_attribute);
}


bool FindFieldResource::accept(FCoords coord) const
{
	uchar res = coord.field->get_resources();

	if ((res & Resource_TypeMask) != (m_resource & Resource_TypeMask))
		return false;

	return true;
}


/*
==============================================================================

CheckStep implementations

==============================================================================
*/


/*
===============
CheckStepDefault
===============
*/
bool CheckStepDefault::allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const
{
	uchar endcaps = end.field->get_caps();

	if (endcaps & m_movecaps)
		return true;

	// Swimming bobs are allowed to move from a water field to a shore field
	uchar startcaps = start.field->get_caps();

	if ((endcaps & MOVECAPS_WALK) && (startcaps & m_movecaps & MOVECAPS_SWIM))
		return true;

	return false;
}

bool CheckStepDefault::reachabledest(Map* map, FCoords dest) const
{
	uchar caps = dest.field->get_caps();

	if (!(caps & m_movecaps)) {
		if (!((m_movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map->can_reach_by_water(dest))
			return false;
	}

	return true;
}


/*
===============
CheckStepWalkOn
===============
*/
bool CheckStepWalkOn::allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const
{
	uchar startcaps = start.field->get_caps();
	uchar endcaps = end.field->get_caps();

	// Make sure that we don't find paths where we walk onto an unwalkable field,
	// then move back onto a walkable field.
	if (!m_onlyend && id != stepFirst && !(startcaps & m_movecaps))
		return false;

	if (endcaps & m_movecaps)
		return true;

	// We can't move onto the field using normal rules.
	// If onlyend is true, exception rules only apply for the last step.
	if (m_onlyend && id != stepLast)
		return false;

	// If the previous field was walkable, we can move onto this one
	if (startcaps & m_movecaps)
		return true;

	return false;
}

bool CheckStepWalkOn::reachabledest(Map* map, FCoords dest) const
{
	// Don't bother solving this.
	return true;
}


/*
===============
CheckStepRoad
===============
*/
bool CheckStepRoad::allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const
{
	uchar endcaps = m_player->get_buildcaps(end);

	// Calculate cost and passability
	if (!(endcaps & m_movecaps)) {
		uchar startcaps = m_player->get_buildcaps(start);

		if (!((endcaps & MOVECAPS_WALK) && (startcaps & m_movecaps & MOVECAPS_SWIM)))
			return false;
	}

	// Check for blocking immovables
	BaseImmovable *imm = map->get_immovable(end);
	if (imm && imm->get_size() >= BaseImmovable::SMALL) {
		if (id != stepLast)
			return false;

		if (!((imm->get_type() == Map_Object::FLAG) ||
			   (imm->get_type() == Map_Object::ROAD && endcaps & BUILDCAPS_FLAG)))
			return false;
	}

	// Is the field forbidden?
	if (m_forbidden) {
		int c = m_forbidden->size()-1;
		while(c >= 0 && end != (*m_forbidden)[c])
			c--;
		if (c >= 0)
			return false;
	}

	return true;
}

bool CheckStepRoad::reachabledest(Map* map, FCoords dest) const
{
	uchar caps = dest.field->get_caps();

	if (!(caps & m_movecaps)) {
		if (!((m_movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map->can_reach_by_water(dest))
			return false;
	}

	return true;
}


/*
==============================================================================

Path/CoordPath IMPLEMENTATION

==============================================================================
*/

/*
===============
Path::Path

Initialize a Path from a CoordPath
===============
*/
Path::Path(CoordPath &o)
{
	m_map = o.get_map();
	m_start = o.get_start();
	m_end = o.get_end();
	m_path = o.get_steps();
	std::reverse(m_path.begin(), m_path.end()); // Path stores in reversed order!
}

/*
===============
Path::reverse

Change the path so that it goes in the opposite direction
===============
*/
void Path::reverse()
{
	std::swap(m_start, m_end);
	std::reverse(m_path.begin(), m_path.end());

	for(uint i = 0; i < m_path.size(); i++)
		m_path[i] = get_reverse_dir(m_path[i]);
}

/*
===============
Path::append

Add the given step at the end of the path.
===============
*/
void Path::append(int dir)
{
	m_path.insert(m_path.begin(), dir); // stores in reversed order!
	m_map->get_neighbour(m_end, dir, &m_end);
}


/*
===============
CoordPath::CoordPath

Initialize from a path, calculating the coordinates as needed
===============
*/
CoordPath::CoordPath(const Path &path)
{
	m_map = 0;
	*this = path;
}


/*
===============
CoordPath::operator=

Assign from a path, calculating coordinates as needed.
===============
*/
CoordPath& CoordPath::operator=(const Path& path)
{
	m_coords.clear();
	m_path.clear();

	m_map = path.get_map();
	m_coords.push_back(path.get_start());

	Coords c = path.get_start();

	for(int i = 0; i < path.get_nsteps(); i++) {
		int dir = path.get_step(i);

		m_path.push_back(dir);
		m_map->get_neighbour(c, dir, &c);
		m_coords.push_back(c);
	}

	return *this;
}


/*
===============
CoordPath::get_index

After which step does the field appear in this path?
Return -1 if field is not part of this path.
===============
*/
int CoordPath::get_index(Coords field) const
{
	for(uint i = 0; i < m_coords.size(); i++)
		if (m_coords[i] == field)
			return i;

	return -1;
}


/*
===============
CoordPath::reverse

Reverse the direction of the path.
===============
*/
void CoordPath::reverse()
{
	std::reverse(m_path.begin(), m_path.end());
	std::reverse(m_coords.begin(), m_coords.end());

	for(uint i = 0; i < m_path.size(); i++)
		m_path[i] = get_reverse_dir(m_path[i]);
}


/*
===============
CoordPath::truncate

Truncate the path after the given number of steps
===============
*/
void CoordPath::truncate(int after)
{
	assert(after >= 0);
	assert((uint)after <= m_path.size());

	m_path.erase(m_path.begin()+after, m_path.end());
	m_coords.erase(m_coords.begin()+after+1, m_coords.end());
}

/*
===============
CoordPath::starttrim

Opposite of truncate: remove the first n steps of the path.
===============
*/
void CoordPath::starttrim(int before)
{
	assert(before >= 0);
	assert((uint)before <= m_path.size());

	m_path.erase(m_path.begin(), m_path.begin()+before);
	m_coords.erase(m_coords.begin(), m_coords.begin()+before);
}

/*
===============
CoordPath::append

Append the given path. Automatically created the necessary coordinates.
===============
*/
void CoordPath::append(const Path &tail)
{
	assert(m_map);
	assert(tail.get_start() == get_end());

	Coords c = get_end();

	for(int i = 0; i < tail.get_nsteps(); i++) {
		char dir = tail.get_step(i);

		m_map->get_neighbour(c, dir, &c);
		m_path.push_back(dir);
		m_coords.push_back(c);
	}

	// debug
	//log("CoordPath; start %i %i\n", m_coords[0].x, m_coords[0].y);
	//for(uint i = 0; i < m_path.size(); i++)
	//	log("  %i -> %i %i\n", m_path[i], m_coords[i+1].x, m_coords[i+1].y);
}

/*
===============
CoordPath::append

Append the given path.
===============
*/
void CoordPath::append(const CoordPath &tail)
{
	assert(m_map);
	assert(tail.get_start() == get_end());

	m_path.insert(m_path.end(), tail.m_path.begin(), tail.m_path.end());
	m_coords.insert(m_coords.end(), tail.m_coords.begin()+1, tail.m_coords.end());
}


/*
==============================================================================

MapRegion IMPLEMENTATION

==============================================================================
*/

/*
===============
MapRegion::init

Initialize the region.
===============
*/
void MapRegion::init(Map* map, Coords coords, uint radius)
{
	m_map = map;
	m_phase = phaseUpper;
	m_radius = radius;

	m_row = 0;
	m_rowwidth = radius+1;
	m_rowpos = 0;

	m_left = map->get_fcoords(coords);
	for(uint i = 0; i < radius; ++i)
		map->get_tln(m_left, &m_left);

	m_next = m_left;
}


/*
===============
MapRegion::next

Traverse the region by row.
I hope this results in slightly better cache behaviour than other algorithms
(e.g. one could also walk concentric "circles"/hexagons).
===============
*/
bool MapRegion::next(FCoords* fc)
{
	if (m_phase == phaseNone)
		return false;

	*fc = m_next;

	m_rowpos++;
	if (m_rowpos < m_rowwidth)
	{
		m_map->get_rn(m_next, &m_next);
	}
	else
	{
		m_row++;

		// If we completed the widest, center line, switch into lower mode
		// There are m_radius+1 lines in the upper "half", because the upper
		// half includes the center line.
		if (m_phase == phaseUpper && m_row > m_radius) {
			m_row = 0;
			m_phase = phaseLower;
		}

		if (m_phase == phaseUpper)
		{
			m_map->get_bln(m_left, &m_left);
			m_rowwidth++;
		}
		else
		{
			// There are m_radius lines in the lower "half"
			if (m_row >= m_radius) {
				m_phase = phaseNone;
				return true; // early out
			}

			m_map->get_brn(m_left, &m_left);
			m_rowwidth--;
		}

		m_next = m_left;
		m_rowpos = 0;
	}

	return true;
}


/*
===============
MapRegion::next

Call the main algorithm and filter out the data we're interested in.
===============
*/
bool MapRegion::next(Coords* c)
{
	FCoords fc;

	if (!next(&fc))
		return false;

	*c = fc;
	return true;
}

Field* MapRegion::next()
{
	FCoords fc;

	if (!next(&fc))
		return 0;

	return fc.field;
}

