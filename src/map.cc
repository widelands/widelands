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

#include "widelands.h"
#include "map.h"
#include "worlddata.h"
#include "game.h"
#include "player.h"

using std::cerr;
using std::endl;
using std::min;

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
 * Costs are in milliseconds to walk.the route.
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

class Map_Loader

=============================
*/
bool Map_Loader::exists_world(const char* worldname) {
   char buf[256];

   snprintf(buf, sizeof(buf), "worlds/%s/conf", worldname);
   FileRead f;
   return f.TryOpen(g_fs, buf);
}

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
	
   if(!exists_world(m_map->get_world_name())) {
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
Preload the map. This only loads the map header.
===============
*/
Map::Map(void) {
	m_nrplayers = 0;
	m_width = m_height = 0;
	m_world = 0;
	m_fields = 0;
	m_starting_pos = 0;

	m_pathcycle = 0;
	m_pathfields = 0;
}

/*
===============
Map::~Map

cleanups
===============
*/
Map::~Map()
{
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
	
	m_nrplayers = 0;
}

/*
===========
Map::recalc_whole_map()

this recalculates all data that needs to be recalculated. 
This is only needed when all fields have change, this means
a map has been loaded or newly created
===========
*/
void Map::recalc_whole_map(void) {
   // Post process the map in the necessary two passes to calculate 
   // brightness and building caps
   Field *f;
   uint y;
   for(y=0; y<m_height; y++) {
      for(uint x=0; x<m_width; x++) {
         f = get_field(x, y);
         int area;
         check_neighbour_heights(x,y,f,&area);
         recalc_brightness(x, y, f);
         recalc_fieldcaps_pass1(x, y, f);
      }
   }

   for(y=0; y<m_height; y++) {
      for(uint x=0; x<m_width; x++) {
         f = get_field(x, y);
         recalc_fieldcaps_pass2(x, y, f);
      }
   }
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
Map::find_bobs

Find Map_Objects in the given area. The functor-version only finds objects for
which functor.accept() returns true.
If list is non-zero, pointers to the relevant objects will be stored in the list.

Returns true if objects could be found
===============
*/
bool Map::find_bobs(Coords coords, uint radius, std::vector<Bob*> *list)
{
	Map_Region mr(coords, radius, this);
	Field *f;
	bool found = false;
	
	while((f = mr.next())) {
		Bob *bob;
		
		for(bob = f->get_first_bob(); bob; bob = bob->get_next_bob()) {
			if (find(list->begin(), list->end(), bob) != list->end())
				continue;
			
			if (!list)
				return true;
			
			list->push_back(bob);
			found = true;
		}
	}
	
	return found;
}

bool Map::find_bobs(Coords coord, uint radius, std::vector<Bob*> *list, const FindBob &functor)
{
	Map_Region mr(coord, radius, this);
	Field *f;
	bool found = false;
	
	while((f = mr.next())) {
		Bob *bob;
		
		for(bob = f->get_first_bob(); bob; bob = bob->get_next_bob()) {
			if (find(list->begin(), list->end(), bob) != list->end())
				continue;
			
			if (functor.accept(bob)) {
				if (!list)
					return true;
			
				list->push_back(bob);
				found = true;
			}
		}
	}
	
	return found;
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
Map::find_immovables

Find all immovables in the given area. Returns true if an immovable has been found.
If list is not 0, found immovables are stored in list.
===============
*/
bool Map::find_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list)
{
	Map_Region mr(coord, radius, this);
	Field *f;
	bool found = false;

	while((f = mr.next())) {
		BaseImmovable *imm = f->get_immovable();

		if (!imm)
			continue;

		if (!list)
			return true; // no need to look any further

		ImmovableFound imf;
		imf.object = imm;
		get_coords(f, &imf.coords);
		list->push_back(imf);
		found = true;
	}
	
	return found;
}

/*
===============
Map::find_immovables

Find all immovables in the given area for which functor returns true.
Returns true if an immovable has been found.
If list is not 0, found immovables are stored in list.
===============
*/
bool Map::find_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list, const FindImmovable &functor)
{
   Map_Region mr(coord, radius, this);
	Field *f;
	bool found = false;

	while((f = mr.next())) {
		BaseImmovable *imm = f->get_immovable();

		if (!imm)
			continue;

		if (functor.accept(imm)) {
			if (!list)
				return true; // no need to look any further

			ImmovableFound imf;
			imf.object = imm;
			get_coords(f, &imf.coords);
			list->push_back(imf);
			found = true;
		}
	}

	return found;
}


/*
===============
Map::find_fields

Fills in a list of coordinates of fields within the given area that functor
accepts.
Returns true if a matching field was found.

Note that list can be 0. In that case, the function returns true immediately as
soon as the first matching field is found.
===============
*/
bool Map::find_fields(Coords coord, uint radius, std::vector<Coords>* list, const FindField& functor)
{
   Map_Region_Coords mrc(coord, radius, this);
	Coords c;
	bool found = false;

	while(mrc.next(&c)) {
		Field* f = get_field(c);

		if (functor.accept(c, f))
		{
			if (!list)
				return true; // no need to look any further

			list->push_back(c);
			found = true;
		}
	}

	return found;
}


/*
Field attribute recalculation passes
------------------------------------

Some events can change the map in a way that run-time calculated attributes
(Field::brightness and Field::caps) need to be recalculated.

These events include:
- change of height (e.g. by planing)
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

/** Map::recalc_brightness(int fx, int fy, Field *f)
 *
 * Fetch the slopes to neighbours and call the actual logic in Field
 */
void Map::recalc_brightness(int fx, int fy, Field *f)
{
   Field *n;
   int dx, dy;
   int l, r, tl, tr, bl, br;

   f = get_field(fx, fy);

   // left
   get_ln(fx, fy, f, &dx, &dy, &n);
   l = f->get_height() - n->get_height();

   // right
   get_rn(fx, fy, f, &dx, &dy, &n);
   r = f->get_height() - n->get_height();

   // top-left
   get_tln(fx, fy, f, &dx, &dy, &n);
   tl = f->get_height() - n->get_height();

   // top-right
   get_rn(dx, dy, n, &dx, &dy, &n);
   tr = f->get_height() - n->get_height();

   // bottom-left
   get_bln(fx, fy, f, &dx, &dy, &n);
   bl = f->get_height() - n->get_height();

   // bottom-right
   get_rn(dx, dy, n, &dx, &dy, &n);
   br = f->get_height() - n->get_height();

   f->set_brightness(l, r, tl, tr, bl, br);
}

/** Map::recalc_fieldcaps_pass1(int fx, int fy, Field *f)
 *
 * Recalculate the fieldcaps for the given field.
 *  - Check terrain types for passability and flag buildability
 *
 * I hope this is understandable and maintainable. 
 *
 * Note: due to inter-field dependencies, fieldcaps calculations are split up
 * into two passes. You should always perform both passes. See the comment
 * above recalc_brightness.
 */
void Map::recalc_fieldcaps_pass1(int fx, int fy, Field *f)
{
   f->caps = 0;

   // 1a) Get all the neighbours to make life easier
   int tlnx, tlny, trnx, trny, lnx, lny, rnx, rny, blnx, blny, brnx, brny;
   Field *tln, *trn, *ln, *rn, *bln, *brn;

   get_tln(fx, fy, f, &tlnx, &tlny, &tln);
   get_trn(fx, fy, f, &trnx, &trny, &trn);
   get_ln(fx, fy, f, &lnx, &lny, &ln);
   get_rn(fx, fy, f, &rnx, &rny, &rn);
   get_bln(fx, fy, f, &blnx, &blny, &bln);
   get_brn(fx, fy, f, &brnx, &brny, &brn);

   // 1b) Collect some information about the neighbours
   int cnt_unpassable = 0;
   int cnt_water = 0;
   int cnt_acid = 0;

   if (f->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (f->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (ln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (tln->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (tln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
   if (trn->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;

   if (f->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
   if (f->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
   if (ln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
   if (tln->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
   if (tln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
   if (trn->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;

   if (f->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (f->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (ln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (tln->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (tln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
   if (trn->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;


   // 2) Passability

   // 2a) If any of the neigbouring triangles is walkable this field is
   //     walkable.
   if (cnt_unpassable < 6)
      f->caps |= MOVECAPS_WALK;

   // 2b) If all neighbouring triangles are water, the field is swimable
   if (cnt_water == 6)
      f->caps |= MOVECAPS_SWIM;
   

   // 2c) [OVERRIDE] If any of the neighbouring triangles is really
   //     "bad" (such as lava), we can neither walk nor swim to this field.
   if (cnt_acid)
      f->caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);

   // === everything below is used to check buildability ===

   // 3) General buildability check: if a "robust" Map_Object is on this field
   //    we cannot build anything on it.
   //    Exception: we can build flags on roads
   BaseImmovable *imm = get_immovable(Coords(fx, fy));

   if (imm && imm->get_type() != Map_Object::ROAD && imm->get_size() >= BaseImmovable::SMALL)
   {
      // 3b) [OVERRIDE] check for "unpassable" Map_Objects
      if (!imm->get_passable())
         f->caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
      return;
   }

   // 4) Flags
   //    We can build flags on anything that's walkable and buildable, with some 
   //    restrictions
   if (f->caps & MOVECAPS_WALK)
   {
      // 4b) Flags must be at least 1 field apart
      if (find_immovables(Coords(fx, fy), 1, 0, FindImmovableType(Map_Object::FLAG))) {
         return;
      }
      f->caps |= BUILDCAPS_FLAG;
   }
}

/** Map::recalc_fieldcaps_pass2(int fx, int fy, Field *f)
 *
 * Second pass of fieldcaps. Determine which kind of building (if any) can be built
 * on this Field.
 *
 * Important: flag buildability has already been checked in the first pass.
 */
void Map::recalc_fieldcaps_pass2(int fx, int fy, Field *f)
{
	// 1) Collect neighbour information
	//
	// NOTE: Yes, this reproduces some of the things done in pass1.
	// This is unavoidable and shouldn't hurt too much anyway.

	// 1a) Get all the neighbours to make life easier
	int tlnx, tlny, trnx, trny, lnx, lny, rnx, rny, blnx, blny, brnx, brny;
	Field *tln, *trn, *ln, *rn, *bln, *brn;
   
	
	get_tln(fx, fy, f, &tlnx, &tlny, &tln);
	get_trn(fx, fy, f, &trnx, &trny, &trn);
	get_ln(fx, fy, f, &lnx, &lny, &ln);
	get_rn(fx, fy, f, &rnx, &rny, &rn);
	get_bln(fx, fy, f, &blnx, &blny, &bln);
	get_brn(fx, fy, f, &brnx, &brny, &brn);

	// 1b) Collect some information about the neighbours
	int cnt_unpassable = 0;
	int cnt_water = 0;
	int cnt_acid = 0;
	int cnt_mountain = 0;
	int cnt_dry = 0;
	
	if (f->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (f->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (ln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (tln->get_terr()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	if (trn->get_terd()->get_is() & TERRAIN_UNPASSABLE) cnt_unpassable++;
	
	if (f->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (f->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (ln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;
	if (tln->get_terr()->get_is() & TERRAIN_WATER) cnt_water++;
	if (trn->get_terd()->get_is() & TERRAIN_WATER) cnt_water++;

	if (f->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (f->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (ln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (tln->get_terr()->get_is() & TERRAIN_ACID) cnt_acid++;
	if (trn->get_terd()->get_is() & TERRAIN_ACID) cnt_acid++;
	
	if (f->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (f->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (ln->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (tln->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (tln->get_terr()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	if (trn->get_terd()->get_is() & TERRAIN_MOUNTAIN) cnt_mountain++;
	
	if (f->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (f->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (ln->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (tln->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (tln->get_terr()->get_is() & TERRAIN_DRY) cnt_dry++;
	if (trn->get_terd()->get_is() & TERRAIN_DRY) cnt_dry++;
	
	// 2) We can only build something on fields that are
	//     - walkable
	//     - have no water triangles next to them
	//     - are not blocked by "robust" Map_Objects
	BaseImmovable *immovable = get_immovable(Coords(fx, fy));

	if (!(f->caps & MOVECAPS_WALK) ||
	    cnt_water ||
	    (immovable && immovable->get_size() >= BaseImmovable::SMALL))
		return;
		
	// 3) We can only build something if there is a flag on the bottom-right neighbour
	//    (or if we could build a flag on the bottom-right neighbour)
	//
	// NOTE: This dependency on the bottom-right neighbour is the reason why the caps
	// calculation is split into two passes
	if (!(brn->caps & BUILDCAPS_FLAG) &&
	    !find_immovables(Coords(brnx, brny), 0, 0, FindImmovableType(Map_Object::FLAG)))
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
	
	find_immovables(Coords(fx, fy), 2, &objectlist, FindImmovableSize(BaseImmovable::SMALL, BaseImmovable::BIG));
	for(uint i = 0; i < objectlist.size(); i++) {
		BaseImmovable *obj = objectlist[i].object;
		Coords objpos = objectlist[i].coords;
		int dist = calc_distance(Coords(fx, fy), objpos);
		
		switch(obj->get_size()) {
		case BaseImmovable::SMALL:
			if (dist == 1) {
		 		if (building > BUILDCAPS_MEDIUM) {
					// a flag to the bottom-right does not reduce building size (obvious)
					// additionally, roads going top-right and left from a big building's
					// flag should be allowed
					if ((objpos != Coords(brnx, brny) && objpos != Coords(rnx, rny) &&
					     objpos != Coords(blnx, blny)) ||
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
		if ((int)brn->get_height() - f->get_height() < 4)
			f->caps |= BUILDCAPS_MINE;
		return;
	}

	// 6) Can't build anything if there are mountain or desert triangles next to the field
	if (cnt_mountain || cnt_dry)
		return;
	
	// 7) Reduce building size based on slope of direct neighbours:
	//    - slope >= 4: can't build anything here -> return
	//    - slope >= 3: maximum size is small
	int slope;

	slope = abs((int)tln->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;
	
	slope = abs((int)trn->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;
	
	slope = abs((int)rn->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;

	// Special case for bottom-right neighbour (where our flag is)
	// Is this correct?	
   // Yep, it is - Holger
	slope = abs((int)brn->get_height() - f->get_height());
	if (slope >= 2) return;
	
	slope = abs((int)bln->get_height() - f->get_height());
	if (slope >= 4) return;
	if (slope >= 3) building = BUILDCAPS_SMALL;
	
	slope = abs((int)ln->get_height() - f->get_height());
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
	int secx, secy;
	Field *sec;

	get_rn(rnx, rny, rn, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;
	
	get_brn(rnx, rny, rn, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_brn(brnx, brny, brn, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_bln(brnx, brny, brn, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_bln(blnx, blny, bln, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_ln(blnx, blny, bln, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_ln(lnx, lny, ln, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_tln(lnx, lny, ln, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_tln(tlnx, tlny, tln, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_trn(tlnx, tlny, tln, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_trn(trnx, trny, trn, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	get_rn(trnx, trny, trn, &secx, &secy, &sec);
	if (abs((int)sec->get_height() - f->get_height()) >= 3) building = BUILDCAPS_SMALL;

	// 9) That's it, store the collected information
	f->caps |= building;
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
			dist += min(dx1, dx2);
		}
		else if (b.x > rx)
		{
			int dx1 = b.x - rx;
			int dx2 = (lx + m_width) - b.x;
			dist += min(dx1, dx2);
		}
	}
	else
	{
		// Reverse case
		if (b.x > rx && b.x < lx)
		{
			int dx1 = b.x - rx;
			int dx2 = lx - b.x;
			dist += min(dx1, dx2);
		}
	}
	
//	cerr << a.x << "," << a.y << " -> " << b.x << "," << b.y << " : " << dist << endl;
	
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

/*
===============
Map::calc_cost_estimate

Calculates the cost estimate between the two points.
This function is used mainly for the path-finding estimate.
===============
*/
#define COST_PER_FIELD		2000 // TODO
 
int Map::calc_cost_estimate(Coords a, Coords b)
{
	return calc_distance(a, b) * COST_PER_FIELD;
}

/*
===============
Map::calc_cost

Calculate the hard cost of walking from coords in the given direction.
The cost is in milliseconds it takes to walk.

TODO: slope must impact cost
===============
*/
int Map::calc_cost(Coords coords, int dir)
{
	return COST_PER_FIELD;
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
      throw wexception("TODO: WLMF not implemented");
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

If player is not 0, the player's get_buildcaps() function will be used.
If roadfind is true, special road-finding mode is activated:
	do not visit fields with robust bobs, however
	do accept flags on the final field, and
	do accept roads where flags can be built as final field
If forbidden is not 0, the fields in this array will not be considered unpassable.

The function returns the cost of the path (in milliseconds of normal walking
speed) or -1 if no path has been found.
===============
*/
int Map::findpath(Coords start, Coords end, uchar movecaps, int persist, Path *path,
                  Player *player, bool roadfind, const std::vector<Coords> *forbidden)
{
	Field *startf, *endf;
	int upper_cost_limit;
	Field *curf;
	Coords cur;
	int caps;

	path->m_path.clear();
	
	normalize_coords(&start.x, &start.y);
	normalize_coords(&end.x, &end.y);
	
	// Some stupid cases...
	if (start == end) {
		path->m_map = this;
		path->m_start = start;
		path->m_end = end;
		return 0; // duh...
	}
		
	startf = get_field(start);
/* // Animals shouldn't be trapped by players building buildings...
	if (!(startf->get_caps() & movecaps)) {
		if (!(movecaps & MOVECAPS_SWIM))
			return -1;
		
		if (!can_reach_by_water(start))
			return -1;
	}
*/

	endf = get_field(end);
	caps = player ? player->get_buildcaps(end) : endf->get_caps();
	if (!(caps & movecaps)) {
		if (!(movecaps & MOVECAPS_SWIM))
			return -1;
		
		if (!can_reach_by_water(end))
			return -1;
	}
	
	// Increase the counter
	// If the counter wrapped, clear the entire pathfinding array
	// This means we clear the array only every 65536 runs
	m_pathcycle++;
	if (!m_pathcycle) {
		memset(m_pathfields, 0, sizeof(Pathfield)*m_height*m_width);
		m_pathcycle++;
	}
	
	if (!persist)
		upper_cost_limit = 0;
	else
		upper_cost_limit = persist * calc_cost_estimate(start, end); // assume 2 secs per field
	
	// Actual pathfinding
	StarQueue Open;
	Pathfield *curpf;
	
	curpf = m_pathfields + (startf-m_fields);
	curpf->cycle = m_pathcycle;
	curpf->real_cost = 0;
	curpf->estim_cost = calc_cost_estimate(start, end);
	curpf->backlink = Map_Object::IDLE;

	Open.push(curpf);

	while((curpf = Open.pop()))
	{
		curf = m_fields + (curpf-m_pathfields);
		
		get_coords(curf, &cur);

		if (upper_cost_limit && curpf->real_cost > upper_cost_limit)
			break; // upper cost limit reached, give up
		if (curf == endf)
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

		int curcaps = player ? player->get_buildcaps(cur) : curf->get_caps();
		
		// Check all the 6 neighbours		
		for(uint i = 6; i; i--, direction++) {
			Field *neighbf;
			Pathfield *neighbpf;
			Coords neighb;
			int cost;

			switch(*direction) {
			case Map_Object::WALK_NW: get_tln(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_NE: get_trn(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_E: get_rn(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_SE: get_brn(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_SW: get_bln(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			case Map_Object::WALK_W: get_ln(cur.x, cur.y, curf, &neighb.x, &neighb.y, &neighbf); break;
			}
			
			neighbpf = m_pathfields + (neighbf-m_fields);
			
			// Is the field Closed already?
			if (neighbpf->cycle == m_pathcycle && neighbpf->heap_index < 0)
				continue;
			
			// Calculate cost and passability
			caps = player ? player->get_buildcaps(neighb) : neighbf->get_caps();
			if (!(caps & movecaps)) {
				if (!(curcaps & movecaps & MOVECAPS_SWIM))
					continue;
			}
			
			// Special road finding
			if (roadfind) {
				BaseImmovable *imm = get_immovable(neighb);
				if (imm && imm->get_size() >= BaseImmovable::SMALL) {
					if (neighb != end)
						continue;
					if (!(imm->get_type() == Map_Object::FLAG ||
					      (imm->get_type() == Map_Object::ROAD && caps & BUILDCAPS_FLAG)))
						continue;
				}
			}
			
			// Is the field forbidden?
			if (forbidden) {
				int c = forbidden->size()-1;
				while(c >= 0 && neighb != (*forbidden)[c])
					c--;
				if (c >= 0)
					continue;
			}
			
			cost = curpf->real_cost + calc_cost(Coords(cur.x, cur.y), *direction);

			if (neighbpf->cycle != m_pathcycle) {
				// add to open list
				neighbpf->cycle = m_pathcycle;
				neighbpf->real_cost = cost;
				neighbpf->estim_cost = calc_cost_estimate(neighb, end);
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
	
	if (curf == endf)
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
		switch(curpf->backlink) {
		case Map_Object::WALK_NW: get_brn(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_NE: get_bln(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_E: get_ln(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_SE: get_tln(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_SW: get_trn(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		case Map_Object::WALK_W: get_rn(cur.x, cur.y, curf, &cur.x, &cur.y, &curf); break;
		}

		curpf = m_pathfields + (curf-m_fields);
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
	Field *f = get_field(field);
	
	if (f->get_caps() & MOVECAPS_SWIM)
		return true;
	if (!(f->get_caps() & MOVECAPS_WALK))
		return false;
	
	Coords n;
	Field *nf;

	get_tln(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_trn(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_rn(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_brn(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_bln(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;

	get_ln(field.x, field.y, f, &n.x, &n.y, &nf);
	if (nf->get_caps() & MOVECAPS_SWIM) return true;
	
	return false;
}

/*
===========
Map::change_field_terrain()

changes the given field terrain to another one.
this happens in the editor and might happen in the game
too if some kind of land increasement is implemented (like
drying swamps).
The fieldcaps need to be recalculated

returns the radius of changes (which are always 2)
===========
*/
int Map::change_field_terrain(Coords c, int terrain, bool tdown, bool tright) {
   assert(tdown || tright);

   Field* f=get_field(c);

   Terrain_Descr* ter=get_world()->get_terrain(terrain);

   if(tdown)
      f->set_terraind(ter);
   if(tright)
      f->set_terrainr(ter);

   Map_Region_Coords mrc(c,1,this);
   Coords coords;
   while(mrc.next(&coords)) {
      recalc_fieldcaps_pass1(coords.x, coords.y, get_field(coords));
   }
   mrc.init(c,1,this);
   while(mrc.next(&coords)) {
      recalc_fieldcaps_pass2(coords.x, coords.y, get_field(coords));
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
int Map::set_field_height(int x, int y, int to) {
   return set_field_height(Coords(x,y),to);
}

int Map::set_field_height(const Coords& coordinates, int to) {
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
int Map::change_field_height(const Coords& coordinates, int by) {
   Field* m_field=get_field(coordinates);
   int height=m_field->get_height();


   height+=by;
   m_field->set_height(height);
   int radius=0;

   check_neighbour_heights(coordinates.x, coordinates.y, m_field, &radius);
   recalc_brightness(coordinates.x,coordinates.y,m_field);
   recalc_fieldcaps_pass1(coordinates.x,coordinates.y,m_field);

   // First recalculation step
   radius+=1;
   Map_Region_Coords mrc(coordinates, radius, this);
	Coords c;

	while(mrc.next(&c)) {
		Field* f = get_field(c);

      recalc_brightness(c.x,c.y,f);
      recalc_fieldcaps_pass1(c.x,c.y,f);
   }

   // seconc recalculation step
   mrc.init(coordinates, radius, this);
   while(mrc.next(&c)) {
      recalc_fieldcaps_pass2(c.x,c.y,get_field(c));
   }

   recalc_fieldcaps_pass2(coordinates.x,coordinates.y,m_field);

   return radius;
}


int Map::change_field_height(int x, int y, int by) {
   return change_field_height(Coords(x,y), by);
}
  
/*
===========
Map::check_neighbour_heights()

This private functions checks all neighbours of a field 
if they are in valid boundaries, if not, they are reheighted
accordingly. returns 0 if no changes where needed, the radius
for the recalculation otherwise
=============
*/
void Map::check_neighbour_heights(int fx, int fy, Field* f, int* area) {
   int m_height=f->get_height();
   int diff;
   bool check[6] = { false, false, false, false, false, false };
   
   
   FCoords m(fx,fy,f);
   FCoords tln, trn, ln, rn, bln, brn;
 
   get_tln(m, &tln); 
   get_trn(m, &trn); 
   get_ln(m, &ln);
   get_rn(m, &rn); 
   get_bln(m, &bln);
   get_brn(m, &brn);
   
   diff=m_height-tln.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; tln.field->set_height(m_height-MAX_FIELD_HEIGHT_DIFF); check[0]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; tln.field->set_height(m_height+MAX_FIELD_HEIGHT_DIFF); check[0]=true; }

   diff=m_height-trn.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; trn.field->set_height(m_height-MAX_FIELD_HEIGHT_DIFF); check[1]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; trn.field->set_height(m_height+MAX_FIELD_HEIGHT_DIFF); check[1]=true; }

   diff=m_height-ln.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; ln.field->set_height(m_height-MAX_FIELD_HEIGHT_DIFF); check[2]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; ln.field->set_height(m_height+MAX_FIELD_HEIGHT_DIFF); check[2]=true; }

   diff=m_height-rn.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; rn.field->set_height(m_height-MAX_FIELD_HEIGHT_DIFF); check[3]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; rn.field->set_height(m_height+MAX_FIELD_HEIGHT_DIFF); check[3]=true; }

   diff=m_height-bln.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; bln.field->set_height(m_height-MAX_FIELD_HEIGHT_DIFF); check[4]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; bln.field->set_height(m_height+MAX_FIELD_HEIGHT_DIFF); check[4]=true; }

   diff=m_height-brn.field->get_height();
   if(diff > MAX_FIELD_HEIGHT_DIFF) { *area++; brn.field->set_height(m_height-MAX_FIELD_HEIGHT_DIFF); check[5]=true; }
   if(diff < -MAX_FIELD_HEIGHT_DIFF) { *area++; brn.field->set_height(m_height+MAX_FIELD_HEIGHT_DIFF); check[5]=true; }

   if(check[0]) check_neighbour_heights(tln.x, tln.y, tln.field, area);
   if(check[1]) check_neighbour_heights(trn.x, trn.y, trn.field, area);
   if(check[2]) check_neighbour_heights(ln.x, ln.y, ln.field, area);
   if(check[3]) check_neighbour_heights(rn.x, rn.y, rn.field, area);
   if(check[4]) check_neighbour_heights(bln.x, bln.y, bln.field, area);
   if(check[5]) check_neighbour_heights(brn.x, brn.y, brn.field, area);
   
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


/*
==============================================================================

Field search functors

==============================================================================
*/

bool FindFieldCaps::accept(Coords c, Field* f) const
{
	uchar fieldcaps = f->get_caps();

	if ((fieldcaps & BUILDCAPS_SIZEMASK) < (m_mincaps & BUILDCAPS_SIZEMASK))
		return false;

	if ((m_mincaps & ~BUILDCAPS_SIZEMASK) & ~(fieldcaps & ~BUILDCAPS_SIZEMASK))
		return false;

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


// 
// class Map_Region
// 
void Map_Region::init(Coords coords, int area, Map *m)
{
	backwards=0;
	_area=area;
	_map=m;
	_lf=m->get_field(coords);
	_tl=_tr=_bl=_br=_lf;
	tlx=trx=blx=brx=coords.x;
	tly=tr_y=bly=bry=coords.y;
	sx=coords.x; sy=coords.y;
	int i;
	for(i=0; i<area; i++) {
		m->get_tln(tlx, tly, _tl, &tlx, &tly, &_tl);
		m->get_trn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
		m->get_bln(blx, bly, _bl, &blx, &bly, &_bl);
		m->get_brn(brx, bry, _br, &brx, &bry, &_br);
	}
	_lf=_tl;
	cx=tlx; cy=tly;
}
     

Field* Map_Region::next(void) {
   Field* retval=_lf;

   if(_lf==_tr) {
      if(tly==sy) backwards=true;
      if(backwards) {
          if(_lf==_br) {
             _lf=0;
             return retval;
          }
         _map->get_brn(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_bln(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
      } else {
         _map->get_bln(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_brn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
         return retval;
      }
   } else {
      _map->get_rn(cx, cy, _lf, &cx, &cy, &_lf);
   }

   
   return retval;
}

// 
// class Map_Region_Coords
// 
void Map_Region_Coords::init(Coords coords, int area, Map *m)
{
	backwards=0;
	_area=area;
	_map=m;
	_lf=m->get_field(coords);
	_tl=_tr=_bl=_br=_lf;
	tlx=trx=blx=brx=coords.x;
	tly=tr_y=bly=bry=coords.y;
	sx=coords.x; sy=coords.y;
	int i;
	for(i=0; i<area; i++) {
		m->get_tln(tlx, tly, _tl, &tlx, &tly, &_tl);
		m->get_trn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
		m->get_bln(blx, bly, _bl, &blx, &bly, &_bl);
		m->get_brn(brx, bry, _br, &brx, &bry, &_br);
	}
	_lf=_tl;
	cx=tlx; cy=tly;
}

bool Map_Region_Coords::next(Coords* pc) {
   bool retval = false;

   if(_lf) retval = true;
   pc->x=cx;
   pc->y=cy;


   if(_lf==_tr) {
      if(tly==sy) backwards=true;
      if(backwards) {
          if(_lf==_br) {
             _lf=0;
             return retval;
          }
         _map->get_brn(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_bln(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
      } else {
         _map->get_bln(tlx, tly, _tl, &tlx, &tly, &_tl);
         _map->get_brn(trx, tr_y, _tr, &trx, &tr_y, &_tr);
         _lf=_tl;
         cx=tlx;
         cy=tly;
         return retval;
      }
   } else {
      _map->get_rn(cx, cy, _lf, &cx, &cy, &_lf);
   }


   return retval;
}
