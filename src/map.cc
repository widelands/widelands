/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "error.h"
#include "events/event_chain.h"
#include "layered_filesystem.h"
#include "mapfringeregion.h"
#include "map_event_manager.h"
#include "map_eventchain_manager.h"
#include "map_variable_manager.h"
#include "map_objective_manager.h"
#include "map_trigger_manager.h"
#include "overlay_manager.h"
#include "player.h"
#include "s2map.h"
#include "tribe.h"
#include "widelands_map_loader.h"
#include "worlddata.h"
#include "wexception.h"

#include <algorithm>

#include <stdio.h>


/**
 * Callback function for font renderer.
 */
std::string g_MapVariableCallback(std::string str, void* data) {
	if (const Map * const map = static_cast<const Map *>(data))
		if
			(const MapVariable * const var =
			 map->get_mvm().get_variable(str.c_str()))
			return var->get_string_representation();
	return (std::string("UNKNOWN:") + str).c_str();
}

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
	int    heap_index; //  index of this field in heap, for backlinking
	int    real_cost;  //  true cost up to this field
	int    estim_cost; //  estimated cost till goal
	ushort cycle;
	uchar  backlink;   //  how we got here (Map_Object::WALK_*)

	inline int cost() {return real_cost + estim_cost;}
};



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
Map::Map() :
m_pathcycle      (0),
m_nrplayers      (0),
m_width          (0),
m_height         (0),
m_world          (0),
m_starting_pos   (0),
m_fields         (0),
m_pathfields     (0),
m_overlay_manager(0),

m_mvm            (new MapVariableManager  ()),
m_mtm            (new MapTriggerManager   ()),
m_mem            (new MapEventManager     ()),
m_mecm           (new MapEventChainManager()),
m_mom            (new MapObjectiveManager ())
{}

/*
===============
Map::~Map

cleanups
===============
*/
Map::~Map()
{
   cleanup();
      delete m_overlay_manager;
      m_overlay_manager=0;
      delete m_mvm;
      m_mvm = 0;
      delete m_mom;
      m_mom = 0;
      delete m_mecm;
      m_mecm = 0;
      delete m_mtm;
      m_mtm = 0;
      delete m_mem;
      m_mem = 0;
}

void Map::recalc_border(const FCoords fc) {
	if (const Player_Number owner = fc.field->get_owned_by()) {
		//  A node that is owned by a player and has a neighbour that is not owned
		//  by that player is a border node.
		for (uchar i = 1; i <= 6; ++i) {
			FCoords neighbour;
			get_neighbour(fc, i, &neighbour);
			if (neighbour.field->get_owned_by() != owner) {
				fc.field->set_border(true);
				return; //  Do not calculate further if there is a border.
			}
		}
	}
	fc.field->set_border(false);
}

/*
===============
Map::recalc_for_field_area

Call this function whenever the field at fx/fy has changed in one of the ways:
 - height has changed
 - robust Map_Object has been added or removed

This performs the steps outlined in the comment above Map::recalc_brightness()
and recalcs the interactive player's overlay.
===============
*/
void Map::recalc_for_field_area(const Area<FCoords> area) {
	assert(0 <= area.x);
	assert(area.x < m_width);
	assert(0 <= area.y);
	assert(area.y < m_height);
	assert(m_fields <= area.field);
	assert            (area.field < m_fields + max_index());
   assert(m_overlay_manager);

	{ //  First pass.
		MapRegion<Area<FCoords> > mr(*this, area);
		do {
			recalc_brightness     (mr.location());
			recalc_border         (mr.location());
			recalc_fieldcaps_pass1(mr.location());
		} while (mr.advance(*this));
	}

	{ //  Second pass.
		MapRegion<Area<FCoords> > mr(*this, area);
		do recalc_fieldcaps_pass2(mr.location()); while (mr.advance(*this));
	}

	{ //  Now only recaluclate the overlays.
		Overlay_Manager & om = overlay_manager();
		MapRegion<Area<FCoords> > mr(*this, area);
		do om.recalc_field_overlays(mr.location()); while (mr.advance(*this));
	}
}


/*
===========
Map::recalc_whole_map()

this recalculates all data that needs to be recalculated.
This is only needed when all fields have change, this means
a map has been loaded or newly created or in the editor that
the overlays have completly changed
===========
*/
void Map::recalc_whole_map()
{
   assert(m_overlay_manager);

   // Post process the map in the necessary two passes to calculate
   // brightness and building caps
   FCoords f;

	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x) {
         f = get_fcoords(Coords(x, y));
			uint radius;
			check_neighbour_heights(f, radius);
         recalc_brightness(f);
         recalc_border(f);
         recalc_fieldcaps_pass1(f);
		}

	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x) {
			f = get_fcoords(Coords(x, y));
         recalc_fieldcaps_pass2(f);
		}

   // Now only recaluclate the overlays
	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x)
	      get_overlay_manager()->recalc_field_overlays(get_fcoords(Coords(x, y)));
}

/*
 * recalculates all default resources.
 *
 * This just needed for the game, not for
 * the editor. Since there, default resources
 * are not shown.
 */
void Map::recalc_default_resources() {
	const World & w = world();
	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x) {
         FCoords f, f1;
         f=get_fcoords(Coords(x, y));
         // only on unset fields
         if (f.field->get_resources()!=0 || f.field->get_resources_amount()) continue;
         std::map<int, int> m;
         int amount=0;

         // This field
			{
				const Terrain_Descr & terr = w.terrain_descr(f.field->terrain_r());
				++m[terr.get_default_resources()];
				amount += terr.get_default_resources_amount();
			}
			{
				const Terrain_Descr & terd = w.terrain_descr(f.field->terrain_d());
				++m[terd.get_default_resources()];
				amount += terd.get_default_resources_amount();
			}

         // If one of the neighbours is unpassable, count its resource stronger
         // top left neigbour
         get_neighbour(f, Map_Object::WALK_NW, &f1);
			{
				const Terrain_Descr & terr = w.terrain_descr(f1.field->terrain_r());
				const char resr =
					terr.get_default_resources();
				if
					(terr.get_is() & TERRAIN_UNPASSABLE
					 and
					 resr != Descr_Maintainer<Resource_Descr>::invalid_index())
					m[resr] += 3;
				else ++m[resr];
				amount += terr.get_default_resources_amount();
			}
			{
				const Terrain_Descr & terd = w.terrain_descr(f1.field->terrain_d());
				const char resd =
					terd.get_default_resources();
				if
					(terd.get_is() & TERRAIN_UNPASSABLE
					 and
					 resd != Descr_Maintainer<Resource_Descr>::invalid_index())
					m[resd] += 3;
				else ++m[resd];
				amount += terd.get_default_resources_amount();
			}

         // top right neigbour
         get_neighbour(f, Map_Object::WALK_NE, &f1);
			{
				const Terrain_Descr & terd = w.terrain_descr(f1.field->terrain_d());
				const char resd =
					terd.get_default_resources();
				if
					(terd.get_is() & TERRAIN_UNPASSABLE
					 and
					 resd != Descr_Maintainer<Resource_Descr>::invalid_index())
					m[resd] += 3;
				else ++m[resd];
				amount += terd.get_default_resources_amount();
			}

         // left neighbour
         get_neighbour(f, Map_Object::WALK_W, &f1);
			{
				const Terrain_Descr & terr = w.terrain_descr(f1.field->terrain_r());
				const char resr =
					terr.get_default_resources();
				if
					(terr.get_is() & TERRAIN_UNPASSABLE
					 and
					 resr != Descr_Maintainer<Resource_Descr>::invalid_index())
					m[resr] += 3;
				else ++m[resr];
				amount += terr.get_default_resources_amount();
			}

         int lv=0;
         int res=0;
         std::map<int, int>::iterator i=m.begin();
         while (i!=m.end()) {
            if (i->second>lv) {
               lv=i->second;
               res=i->first;
				}
            i++;
			}
         amount/=6;

         if (res==-1 || !amount) {
            f.field->set_resources(0, 0);
            f.field->set_starting_res_amount(0);
			} else {
            f.field->set_resources(res, amount);
            f.field->set_starting_res_amount(amount);
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
void Map::cleanup() {
	m_nrplayers = 0;
	m_width = m_height = 0;
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
		delete m_world;
	m_world = 0;

   m_scenario_tribes.clear();
   m_scenario_names.clear();

   if (m_overlay_manager)
      m_overlay_manager->reset();

   delete m_mom;
   m_mom = new MapObjectiveManager();

	{
		MapEventChainManager & mecm = *m_mecm;
		while (mecm.get_nr_eventchains())
			mecm.delete_eventchain(mecm.get_eventchain_by_nr(0).get_name());
	}
	m_mem->delete_unreferenced_events();
	m_mtm->delete_unreferenced_triggers();

	assert(m_mtm->get_nr_triggers() == 0);
	assert(m_mem->get_nr_events  () == 0);

   delete m_mvm;
   m_mvm = new MapVariableManager();

   delete m_mecm;
   m_mecm = new MapEventChainManager();

   // Remove all extra datas. Pay attention here, maybe some freeing would be needed
#ifdef DEBUG
   for (uint i = 0; i < m_extradatainfos.size(); i++) {
      assert(m_extradatainfos[i].type == Extradata_Info::PIC) ;
	}
#endif
   m_extradatainfos.resize(0);
}

/*
===========
Map::create_emtpy_map

creates an empty map without name with
the given data
===========
*/
void Map::create_empty_map
(const uint w, const uint h, const std::string worldname)
{
	set_world_name(worldname.c_str());
	load_world();
	set_size(w, h);
	set_name(_("No Name").c_str());
	set_author(_("Unknown").c_str());
	set_description(_("no description defined").c_str());
	set_nrplayers(1);
	// Set first tribe found as the "basic" tribe
	// <undefined> (as set before) is useless and will lead to a crash -> Widelands will search for tribe "<undefined>"
	std::vector<std::string> tribes;
	Tribe_Descr::get_all_tribenames(tribes);
	set_scenario_player_tribe(1, tribes[0]);
	set_scenario_player_name(1, _("Player 1"));

	{
		Field::Terrains default_terrains;
		default_terrains.d = 0;
		default_terrains.r = 0;
		Field * field = m_fields;
		const Field * const fields_end = field + max_index();
		for (; field < fields_end; ++field) {
			field->set_height(10);
			field->set_terrains(default_terrains);
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
void Map::set_size(const uint w, const uint h)
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

	if (not m_overlay_manager) m_overlay_manager = new Overlay_Manager();
}

/*
 * The scenario get/set functions
 */
std::string Map::get_scenario_player_tribe(uint i) {
   assert(m_scenario_tribes.size()==m_nrplayers);

   return m_scenario_tribes[i-1];
}

std::string Map::get_scenario_player_name(uint i) {
   assert(m_scenario_names.size()==m_nrplayers);

   return m_scenario_names[i-1];
}

void Map::set_scenario_player_tribe(uint i, std::string str) {
   assert(i<=m_nrplayers);
   m_scenario_tribes.resize(m_nrplayers);
   m_scenario_tribes[i-1]=str;
}
void Map::set_scenario_player_name(uint i, std::string str) {
   assert(i<=m_nrplayers);
   m_scenario_names.resize(m_nrplayers);
   m_scenario_names[i-1]=str;
}

/*
===============
Map::set_nrplayers

Change the number of players the map supports.
Could happen multiple times in the map editor.
===============
*/
void Map::set_nrplayers(const Uint8 nrplayers) {
	if (!nrplayers) {
		if (m_starting_pos)
			free(m_starting_pos);
		m_starting_pos = 0;
		m_nrplayers = 0;
		return;
	}

	m_starting_pos = (Coords*)realloc(m_starting_pos, sizeof(Coords)*nrplayers);
	while (m_nrplayers < nrplayers)
		m_starting_pos[m_nrplayers++] = Coords(-1, -1);

	m_nrplayers = nrplayers; // in case the number players got less
}

/*
===============
Map::set_starting_pos

Set the starting coordinates of a player
===============
*/
void Map::set_starting_pos(const uint plnum, const Coords c)
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
void Map::set_filename(const char *string)
{
	snprintf(m_filename, sizeof(m_filename), "%s", string);
}

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

void Map::set_background(const char * string)
{
	if (NULL == string)
		m_background.clear();
	else
		m_background = string;
}

/*
===============
Map::load_world

load the corresponding world. This should only happen
once during initial load.
===============
*/
void Map::load_world()
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
BaseImmovable * Map::get_immovable(const Coords coord) const
{return get_field(coord)->get_immovable();}


/*
===============
Call the functor for every field that can be reached from coord without moving
outside the given radius.

Functor is of the form: functor(Map*, FCoords)
===============
*/
template<typename functorT>
void Map::find_reachable
(const Area<FCoords> area,
 const CheckStep & checkstep,
 functorT & functor)
{
	std::vector<Field*> queue;

	increase_pathcycle();

	queue.push_back(area.field);

	while (queue.size()) {
		Pathfield* curpf;
		FCoords cur;

		// Pop the last item from the queue
		cur.field = queue[queue.size() - 1];
		curpf = m_pathfields + (cur.field-m_fields);
		get_coords(*cur.field, cur);
		queue.pop_back();

		// Handle this field
		functor(*this, cur);
		curpf->cycle = m_pathcycle;

		// Get neighbours
		for (Direction dir = 1; dir <= 6; ++dir) {
			Pathfield *neighbpf;
			FCoords neighb;

			get_neighbour(cur, dir, &neighb);
			neighbpf = m_pathfields + (neighb.field-m_fields);

			// Have we already visited this field?
			if (neighbpf->cycle == m_pathcycle)
				continue;

			// Is the field still within the radius?
			if (calc_distance(area, neighb) > area.radius)
				continue;

			// Are we allowed to move onto this field?
			if
				(not
				 checkstep.allowed
				 (this,
				  cur,
				  neighb,
				  dir,
				  cur == area ? CheckStep::stepFirst : CheckStep::stepNormal))
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

Functor is of the form: functor(Map &, FCoords)
===============
*/
template<typename functorT>
void Map::find(const Area<FCoords> area, functorT & functor) const {
	MapRegion<Area<FCoords> > mr(*this, area);
	do functor(*this, mr.location()); while (mr.advance(*this));
}


/*
===============
FindBobsCallback

The actual logic behind find_bobs and find_reachable_bobs.
===============
*/
struct FindBobsCallback {
	FindBobsCallback(std::vector<Bob*>* list, const FindBob& functor)
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map &, const FCoords cur) {
		Bob *bob;

		for (bob = cur.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
			if (m_list && std::find(m_list->begin(), m_list->end(), bob) != m_list->end())
				continue;

			if (m_functor.accept(bob)) {
				if (m_list)
					m_list->push_back(bob);

				m_found++;
			}
		}
	}

	std::vector<Bob *> * m_list;
	const FindBob      & m_functor;
	uint                 m_found;
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
unsigned int Map::find_bobs
(const Area<FCoords> area, std::vector<Bob*> * list, const FindBob & functor)
{
	FindBobsCallback cb(list, functor);

	find(area, cb);

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
unsigned int Map::find_reachable_bobs
(const Area<FCoords> area,
 std::vector<Bob*> * list,
 const CheckStep & checkstep,
 const FindBob & functor)
{
	FindBobsCallback cb(list, functor);

	find_reachable(area, checkstep, cb);

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
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map &, const FCoords cur) {
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

	std::vector<ImmovableFound> * m_list;
	const FindImmovable         & m_functor;
	uint                          m_found;
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
unsigned int Map::find_immovables
(const Area<FCoords>           area,
 std::vector<ImmovableFound> * list,
 const FindImmovable & functor)
{
	FindImmovablesCallback cb(list, functor);

	find(area, cb);

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
uint Map::find_reachable_immovables
(const Area<FCoords>           area,
 std::vector<ImmovableFound> * list,
 const CheckStep & checkstep,
 const FindImmovable & functor)
{
	FindImmovablesCallback cb(list, functor);

	find_reachable(area, checkstep, cb);

	return cb.m_found;
}

/*
===============
FindNodesCallback

The actual logic behind find_fields and find_reachable_fields.
===============
*/
struct FindNodesCallback {
	FindNodesCallback(std::vector<Coords>* list, const FindNode& functor)
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map & map, const FCoords cur) {
		if (m_functor.accept(map, cur)) {
			if (m_list)
				m_list->push_back(cur);

			m_found++;
		}
	}

	std::vector<Coords> * m_list;
	const FindNode     & m_functor;
	uint                  m_found;
};


/*
===============
Fills in a list of coordinates of fields within the given area that functor
accepts.
Returns the number of matching fields.

Note that list can be 0.
===============
*/
unsigned int Map::find_fields
(const Area<FCoords>   area,
 std::vector<Coords> * list,
 const FindNode & functor)
{
	FindNodesCallback cb(list, functor);

	find(area, cb);

	return cb.m_found;
}


/*
===============
Fills in a list of coordinates of fields reachable by walking within the given
radius that functor accepts.
Returns the number of matching fields.

Note that list can be 0.
===============
*/
unsigned int Map::find_reachable_fields
(const Area<FCoords>   area,
 std::vector<Coords> * list,
 const CheckStep & checkstep,
 const FindNode & functor)
{
	FindNodesCallback cb(list, functor);

	find_reachable(area, checkstep, cb);

	return cb.m_found;
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
   uchar caps = CAPS_NONE;


   // 1a) Get all the neighbours to make life easier
	const FCoords  r =  r_n(f);
	const FCoords tr = tr_n(f);
	const FCoords tl = tl_n(f);
	const FCoords  l =  l_n(f);

	const World & w = world();

	const Uint8 tr_d_terrain_is = w.terrain_descr(tr.field->terrain_d()).get_is();
	const Uint8 tl_r_terrain_is = w.terrain_descr(tl.field->terrain_r()).get_is();
	const Uint8 tl_d_terrain_is = w.terrain_descr(tl.field->terrain_d()).get_is();
	const Uint8  l_r_terrain_is = w.terrain_descr (l.field->terrain_r()).get_is();
	const Uint8  f_d_terrain_is = w.terrain_descr (f.field->terrain_d()).get_is();
	const Uint8  f_r_terrain_is = w.terrain_descr (f.field->terrain_r()).get_is();

   // 1b) Collect some information about the neighbours
	uchar cnt_unpassable = 0;
	uchar cnt_water = 0;
	uchar cnt_acid = 0;

	if  (tr_d_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if  (tl_r_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if  (tl_d_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if   (l_r_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if   (f_d_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if   (f_r_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;

	if  (tr_d_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if  (tl_r_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if  (tl_d_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if   (l_r_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if   (f_d_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if   (f_r_terrain_is & TERRAIN_WATER)      ++cnt_water;

	if  (tr_d_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if  (tl_r_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if  (tl_d_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if   (l_r_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if   (f_d_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if   (f_r_terrain_is & TERRAIN_ACID)       ++cnt_acid;


   // 2) Passability

   // 2a) If any of the neigbouring triangles is walkable this field is
   //     walkable.
   if (cnt_unpassable < 6)
      caps |= MOVECAPS_WALK;

   // 2b) If all neighbouring triangles are water, the field is swimable
   if (cnt_water == 6)
      caps |= MOVECAPS_SWIM;


   // 2c) [OVERRIDE] If any of the neighbouring triangles is really
   //     "bad" (such as lava), we can neither walk nor swim to this field.
   if (cnt_acid)
      caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);

   // === everything below is used to check buildability ===

   // 3) General buildability check: if a "robust" Map_Object is on this field
   //    we cannot build anything on it.
   //    Exception: we can build flags on roads
   BaseImmovable *imm = get_immovable(f);

   if (imm && imm->get_type() != Map_Object::ROAD && imm->get_size() >= BaseImmovable::SMALL)
   {
      // 3b) [OVERRIDE] check for "unpassable" Map_Objects
      if (!imm->get_passable())
         caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
      goto end;
	}

   // 4) Flags
   //    We can build flags on anything that's walkable and buildable, with some
   //    restrictions
	if (caps & MOVECAPS_WALK) {
      // 4b) Flags must be at least 1 field apart
		if
			(find_immovables
			 (Area<FCoords>(f, 1), 0, FindImmovableType(Map_Object::FLAG)))
         goto end;
      caps |= BUILDCAPS_FLAG;
	}
end:
	f.field->caps = static_cast<FieldCaps>(caps);
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
	const FCoords  r =  r_n(f);
	const FCoords tr = tr_n(f);
	const FCoords tl = tl_n(f);
	const FCoords  l =  l_n(f);

	const World & w = world();

	const Uint8 tr_d_terrain_is = w.terrain_descr(tr.field->terrain_d()).get_is();
	const Uint8 tl_r_terrain_is = w.terrain_descr(tl.field->terrain_r()).get_is();
	const Uint8 tl_d_terrain_is = w.terrain_descr(tl.field->terrain_d()).get_is();
	const Uint8  l_r_terrain_is = w.terrain_descr (l.field->terrain_r()).get_is();
	const Uint8  f_d_terrain_is = w.terrain_descr (f.field->terrain_d()).get_is();
	const Uint8  f_r_terrain_is = w.terrain_descr (f.field->terrain_r()).get_is();

	// 1b) Collect some information about the neighbours
	int cnt_unpassable = 0;
	int cnt_water = 0;
	int cnt_acid = 0;
	int cnt_mountain = 0;
	int cnt_dry = 0;

	if  (tr_d_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if  (tl_r_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if  (tl_d_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if   (l_r_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if   (f_d_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;
	if   (f_r_terrain_is & TERRAIN_UNPASSABLE) ++cnt_unpassable;

	if  (tr_d_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if  (tl_r_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if  (tl_d_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if   (l_r_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if   (f_d_terrain_is & TERRAIN_WATER)      ++cnt_water;
	if   (f_r_terrain_is & TERRAIN_WATER)      ++cnt_water;

	if  (tr_d_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if  (tl_r_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if  (tl_d_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if   (l_r_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if   (f_d_terrain_is & TERRAIN_ACID)       ++cnt_acid;
	if   (f_r_terrain_is & TERRAIN_ACID)       ++cnt_acid;

	if  (tr_d_terrain_is & TERRAIN_MOUNTAIN)   ++cnt_mountain;
	if  (tl_r_terrain_is & TERRAIN_MOUNTAIN)   ++cnt_mountain;
	if  (tl_d_terrain_is & TERRAIN_MOUNTAIN)   ++cnt_mountain;
	if   (l_r_terrain_is & TERRAIN_MOUNTAIN)   ++cnt_mountain;
	if   (f_d_terrain_is & TERRAIN_MOUNTAIN)   ++cnt_mountain;
	if   (f_r_terrain_is & TERRAIN_MOUNTAIN)   ++cnt_mountain;

	if  (tr_d_terrain_is & TERRAIN_DRY)        ++cnt_dry;
	if  (tl_r_terrain_is & TERRAIN_DRY)        ++cnt_dry;
	if  (tl_d_terrain_is & TERRAIN_DRY)        ++cnt_dry;
	if   (l_r_terrain_is & TERRAIN_DRY)        ++cnt_dry;
	if   (f_d_terrain_is & TERRAIN_DRY)        ++cnt_dry;
	if   (f_r_terrain_is & TERRAIN_DRY)        ++cnt_dry;

	uchar caps = f.field->caps;

	// 2) We can only build something on fields that are
	//     - walkable
	//     - have no water triangles next to them
	//     - are not blocked by "robust" Map_Objects
	BaseImmovable *immovable = get_immovable(f);

	if
		(not (caps & MOVECAPS_WALK)
		 or
	    cnt_water
		 or
	    (immovable and immovable->get_size() >= BaseImmovable::SMALL))
		goto end;

	// 3) We can only build something if there is a flag on the bottom-right neighbour
	//    (or if we could build a flag on the bottom-right neighbour)
	//
	// NOTE: This dependency on the bottom-right neighbour is the reason why the caps
	// calculation is split into two passes
	{
		const FCoords br = br_n(f);
		if
			(not (br.field->caps & BUILDCAPS_FLAG)
			 and
			 not find_immovables
			 (Area<FCoords>(br, 0), 0, FindImmovableType(Map_Object::FLAG)))
			goto end;

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
		{
			const FCoords bl = bl_n(f);
	uchar building = BUILDCAPS_BIG;
	std::vector<ImmovableFound> objectlist;

			find_immovables
				(Area<FCoords>(f, 2),
				 &objectlist,
				 FindImmovableSize(BaseImmovable::SMALL, BaseImmovable::BIG));
	for (uint i = 0; i < objectlist.size(); i++) {
		BaseImmovable *obj = objectlist[i].object;
		Coords objpos = objectlist[i].coords;
		int dist = calc_distance(f, objpos);

		switch (obj->get_size()) {
		case BaseImmovable::SMALL:
			if (dist == 1) {
				if (building > BUILDCAPS_MEDIUM) {
					// a flag to the bottom-right does not reduce building size (obvious)
					// additionally, roads going top-right and left from a big building's
					// flag should be allowed
							if
								((objpos != br and objpos != r and objpos != bl)
								 or
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
				goto end; // can't build buildings next to big objects
		}
	}

	// 5) Build mines on mountains
	if (cnt_mountain == 6)
	{
		// 4b) Check the mountain slope
		if
			(static_cast<int>(br.field->get_height()) - f.field->get_height()
			 <
			 4)
			caps |= BUILDCAPS_MINE;
		goto end;
	}

	// 6) Can't build anything if there are mountain or desert triangles next to the field
	if (cnt_mountain || cnt_dry)
		goto end;

			const Sint16 f_height = f.field->get_height();

	// 7) Reduce building size based on slope of direct neighbours:
	//    - slope >= 4: can't build anything here -> return
	//    - slope >= 3: maximum size is small
			{
				MapFringeRegion<Area<FCoords> > mr(*this, Area<FCoords>(f, 1));
				do {
					const Uint16 slope =
						abs(mr.location().field->get_height() - f_height);
					if (slope >= 4) goto end;
					if (slope >= 3) building = BUILDCAPS_SMALL;
				} while (mr.advance(*this));
			}
	// Special case for bottom-right neighbour (where our flag is)
	// Is this correct?
   // Yep, it is - Holger
			if (abs(br.field->get_height() - f_height) >= 2) goto end;

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
			{
				MapFringeRegion<Area<FCoords> > mr(*this, Area<FCoords>(f, 2));
				do if (abs(mr.location().field->get_height() - f_height) >= 3)
				{building = BUILDCAPS_SMALL; break;} while (mr.advance(*this));
			}

	caps |= building;
		}
	}
end: //  9) That's it, store the collected information.
	f.field->caps = static_cast<FieldCaps>(caps);
}


/**
 * Calculate the (Manhattan) distance from a to b
 * a and b are expected to be normalized!
 */
uint Map::calc_distance(const Coords a, const Coords b) const
{
	uint dist;
	int dy;

	// do we fly up or down?
	dy = b.y - a.y;
	if (dy > static_cast<int>(m_height >> 1)) //  wrap-around!
		dy -= m_height;
	else if (dy < -static_cast<int>(m_height >> 1))
		dy += m_height;

	dist = abs(dy);

	if (static_cast<X_Coordinate>(dist) >= m_width) // no need to worry about x movement at all
		return dist;

	// [lx..rx] is the x-range we can cover simply by walking vertically
	// towards b
	// Hint: (~a.y & 1) is 1 for even rows, 0 for odd rows.
	// This means we round UP for even rows, and we round DOWN for odd rows.
	int lx, rx;

	lx = a.x - ((dist + (~a.y & 1)) >> 1); // div 2
	rx = lx + dist;

	// Allow for wrap-around
	// Yes, the second is an else if; see the above if (dist >= m_width)
	if (lx < 0)
		lx += m_width;
	else if (rx >= static_cast<int>(m_width))
		rx -= m_width;

	// Normal, non-wrapping case
	if (lx <= rx)
	{
		if (b.x < lx) {
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
int Map::is_neighbour(const Coords start, const Coords end) const
{
	int dx, dy;

	dy = end.y - start.y;
	dx = end.x - start.x;
	if (dx > static_cast<int>(m_width >> 1))
		dx -= m_width;
	else if (dx < -static_cast<int>(m_width >> 1))
		dx += m_width;

	// end and start are on the same row
	if (!dy) {
		switch (dx) {
		case -1: return Map_Object::WALK_W;
		case 0: return -1;
		case 1: return Map_Object::WALK_E;
		default: return 0;
		}
	}

	if (dy > static_cast<int>(m_height >> 1))
		dy -= m_height;
	else if (dy < -static_cast<int>(m_height >> 1))
		dy += m_height;

	// end is one row below start
	if (dy == 1) {
		if (start.y & 1) --dx;
		switch (dx) {
		case -1: return Map_Object::WALK_SW;
		case 0: return Map_Object::WALK_SE;
		default: return 0;
		}
	}

	// end is one row above start
	if (dy == -1) {
		if (start.y & 1) --dx;
		switch (dx) {
		case -1: return Map_Object::WALK_NW;
		case 0: return Map_Object::WALK_NE;
		default: return 0;
		}
	}

	return 0;
}


#define BASE_COST_PER_FIELD 1800
#define SLOPE_COST_FACTOR      0.02
#define SLOPE_COST_STEPS       8

/*
===============
Map::calc_cost_estimate

Calculates the cost estimate between the two points.
This function is used mainly for the path-finding estimate.
===============
*/
int Map::calc_cost_estimate(const Coords a, const Coords b) const
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
#define CALC_COST_D(slope) (SLOPE_COST_FACTOR * ((slope) + SLOPE_COST_STEPS) * ((slope) + SLOPE_COST_STEPS - 1) * 0.5)
#define CALC_COST_BASE     (1.0 - CALC_COST_D(0))

static inline float calc_cost_d(int slope)
{
	if (slope < -SLOPE_COST_STEPS)
		slope = -SLOPE_COST_STEPS;

	return CALC_COST_D(slope);
}

int Map::calc_cost(int slope) const
{
	return static_cast<int>
		(BASE_COST_PER_FIELD * (CALC_COST_BASE + calc_cost_d(slope)));
}


/*
===============
Map::calc_cost

Return the time it takes to walk the given step from coords in the given
direction, in milliseconds.
===============
*/
int Map::calc_cost(const Coords coords, const int dir) const
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
int Map::calc_bidi_cost(const Coords coords, const int dir) const
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
void Map::calc_cost(const Path & path, int *forward, int *backward) const
{
	Coords coords = path.get_start();

	if (forward)
		*forward = 0;
	if (backward)
		*backward = 0;

	const Path::Step_Vector::size_type nr_steps = path.get_nsteps();
	for (Path::Step_Vector::size_type i = 0; i < nr_steps; ++i) {
		const Direction dir = path[i];

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
void Map::get_neighbour
(const Coords f, const Direction dir, Coords * const o) const
{
	switch (dir) {
	case Map_Object::WALK_NW: get_tln(f, o); break;
	case Map_Object::WALK_NE: get_trn(f, o); break;
	case Map_Object::WALK_E: get_rn(f, o); break;
	case Map_Object::WALK_SE: get_brn(f, o); break;
	case Map_Object::WALK_SW: get_bln(f, o); break;
	case Map_Object::WALK_W: get_ln(f, o); break;
	}
}

void Map::get_neighbour
(const FCoords f, const Direction dir, FCoords * const o) const
{
	switch (dir) {
	case Map_Object::WALK_NW: get_tln(f, o); break;
	case Map_Object::WALK_NE: get_trn(f, o); break;
	case Map_Object::WALK_E: get_rn(f, o); break;
	case Map_Object::WALK_SE: get_brn(f, o); break;
	case Map_Object::WALK_SW: get_bln(f, o); break;
	case Map_Object::WALK_W: get_ln(f, o); break;
	}
}

/**
 * Returns the correct initialized loader for the given mapfile
*/
Map_Loader* Map::get_correct_loader(const char* filename) {
   Map_Loader* retval=0;
   if (strlen(filename)<strlen(WLMF_SUFFIX) || strlen(filename)<strlen(S2MF_SUFFIX)) return 0;

   if (!strcasecmp(filename+(strlen(filename)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
   {
      try {
         FileSystem* fs = g_fs->MakeSubFileSystem(filename);
			retval = new Widelands_Map_Loader(*fs, this);
		} catch (...) {
         // If this fails, it is an illegal file (maybe old plain binary map format)
         //TODO: catchall hides real errors! Replace with more specific code
		}
	}
   else if (!strcasecmp(filename+(strlen(filename)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
   {
      // it is a S2 Map file. load it as such
      retval=new S2_Map_Loader(filename, this);
	}

   return retval;
}

/**
 * Provides the flexible priority queue to maintain the open list.
 */
class StarQueue {
	std::vector<Map::Pathfield*> m_data;

public:
	void flush() {m_data.clear();}

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
		while (fix < nsize) {
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

		while (slot > 0) {
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

		while (slot > 0) {
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
		if (m_data[node]->heap_index != static_cast<int>(node)) {
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
int Map::findpath
(Coords instart,
 Coords inend,
 const int persist,
 Path & path,
 const CheckStep & checkstep,
 const uint flags)
{
	FCoords start;
	FCoords end;
	int upper_cost_limit;
	FCoords cur;

	normalize_coords(&instart);
	normalize_coords(&inend);

	start = FCoords(instart, get_field(instart));
	end   = FCoords(inend,   get_field(inend));

	path.m_path.clear();

	// Some stupid cases...
	if (start == end) {
		path.m_start = start;
		path.m_end = end;
		return 0; // duh...
	}

	if (not checkstep.reachabledest(this, end))
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

	while ((curpf = Open.pop()))
	{
		cur.field = m_fields + (curpf-m_pathfields);
		get_coords(*cur.field, cur);

		if (upper_cost_limit && curpf->real_cost > upper_cost_limit)
			break; // upper cost limit reached, give up
		if (cur == end)
			break; // found our target

		// avoid bias by using different orders when pathfinding
		static const char order1[6] = {Map_Object::WALK_NW, Map_Object::WALK_NE,
			Map_Object::WALK_E, Map_Object::WALK_SE, Map_Object::WALK_SW, Map_Object::WALK_W};
		static const char order2[6] = {Map_Object::WALK_NW, Map_Object::WALK_W,
			Map_Object::WALK_SW, Map_Object::WALK_SE, Map_Object::WALK_E, Map_Object::WALK_NE};
		const char *direction;

		direction = (cur.x + cur.y) & 1 ? order1 : order2;

		// Check all the 6 neighbours
		for (uint i = 6; i; i--, direction++) {
			Pathfield *neighbpf;
			FCoords neighb;
			int cost;

			get_neighbour(cur, *direction, &neighb);
			neighbpf = m_pathfields + (neighb.field-m_fields);

			// Is the field Closed already?
			if (neighbpf->cycle == m_pathcycle && neighbpf->heap_index < 0)
				continue;

			// Check passability
			if
				(not
				 checkstep.allowed
				 (this,
				  cur,
				  neighb,
				  *direction,
				  neighb == end
				  ?
				  CheckStep::stepLast
				  :
				  cur == start ? CheckStep::stepFirst : CheckStep::stepNormal))
				continue;

			// Calculate cost
			cost =
				curpf->real_cost
				+
				(flags & fpBidiCost) ?
				calc_bidi_cost(cur, *direction) : calc_cost(cur, *direction);

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

	path.m_start = start;
	path.m_end = cur;

	path.m_path.clear();

	while (curpf->backlink != Map_Object::IDLE) {
		path.m_path.push_back(curpf->backlink);

		// Reverse logic! (WALK_NW needs to find the SE neighbour)
		get_neighbour(cur, get_reverse_dir(curpf->backlink), &cur);
		curpf = m_pathfields + (cur.field-m_fields);
	}

	return retval;
}


bool Map::can_reach_by_water(const Coords field) const
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
changes the given triangle's terrain.
this happens in the editor and might happen in the game
too if some kind of land increasement is implemented (like
drying swamps).
The fieldcaps need to be recalculated

returns the radius of changes (which are always 2)
===========
*/
int Map::change_terrain
(const TCoords<FCoords> c, const Terrain_Descr::Index terrain)
{
	c.field->set_terrain(c.t, terrain);
	recalc_for_field_area(Area<FCoords>(c, 2));

   return 2;
}


uint Map::set_height(const FCoords fc, const Uint8 new_value) {
	assert(new_value <= MAX_FIELD_HEIGHT);
	assert(m_fields <= fc.field);
	assert            (fc.field < m_fields + max_index());
	fc.field->height = new_value;
	uint radius = 2;
	check_neighbour_heights(fc, radius);
	recalc_for_field_area(Area<FCoords>(fc, radius));
	return radius;
}

uint Map::change_height(Area<FCoords> area, const Sint16 difference) {
	{
		MapRegion<Area<FCoords> > mr(*this, area);
		do {
			if
				(difference < 0
			    and
				 mr.location().field->height
				 <
				 static_cast<Uint8>(-difference))
				mr.location().field->height = 0;
			else if
				(static_cast<Sint16>(MAX_FIELD_HEIGHT) - difference
				 <
				 static_cast<Sint16>(mr.location().field->height))
				mr.location().field->height = MAX_FIELD_HEIGHT;
			else  mr.location().field->height += difference;
		} while (mr.advance(*this));
	}
	uint regional_radius = 0;
	MapFringeRegion<Area<FCoords> > mr(*this, area);
	do {
		uint local_radius = 0;
		check_neighbour_heights(mr.location(), local_radius);
		regional_radius = std::max(regional_radius, local_radius);
	} while (mr.advance(*this));
	area.radius += regional_radius + 2;
	recalc_for_field_area(area);
	return area.radius;
}

uint Map::set_height
(Area<FCoords> area, interval<Field::Height> height_interval)
{
	assert(height_interval.valid());
	assert(height_interval.max <= MAX_FIELD_HEIGHT);
	{
		MapRegion<Area<FCoords> > mr(*this, area);
		do {
			if      (mr.location().field->height < height_interval.min)
				mr.location().field->height = height_interval.min;
			else if (height_interval.max < mr.location().field->height)
				mr.location().field->height = height_interval.max;
		} while (mr.advance(*this));
	}
	++area.radius;
	{
		MapFringeRegion<Area<FCoords> > mr(*this, area);
		bool changed;
		do {
			changed = false;
			height_interval.min = height_interval.min < MAX_FIELD_HEIGHT_DIFF ?
				0 : height_interval.min - MAX_FIELD_HEIGHT_DIFF;
			height_interval.max =
				height_interval.max < MAX_FIELD_HEIGHT - MAX_FIELD_HEIGHT_DIFF ?
				height_interval.max + MAX_FIELD_HEIGHT_DIFF : MAX_FIELD_HEIGHT;
			do {
				if (mr.location().field->height < height_interval.min) {
					mr.location().field->height = height_interval.min;
					changed = true;
				} else if (height_interval.max < mr.location().field->height) {
					mr.location().field->height = height_interval.max;
					changed = true;
				}
			} while (mr.advance(*this));
			mr.extend(*this);
		} while (changed);
		area.radius = mr.radius();
	}
	recalc_for_field_area(area);
	return area.radius;
}


/*
===========
Map::check_neighbour_heights()

This private functions checks all neighbours of a field
if they are in valid boundaries, if not, they are reheighted
accordingly.
The radius of modified fields is stored in area.
=============
*/
void Map::check_neighbour_heights(FCoords coords, uint & area)
{
	assert(m_fields <= coords.field);
	assert            (coords.field < m_fields + max_index());

   int height = coords.field->get_height();
   bool check[6] = {false, false, false, false, false, false};

	const FCoords n[6] = {
		tl_n(coords),
		tr_n(coords),
		l_n (coords),
		r_n (coords),
		bl_n(coords),
		br_n(coords)
	};

	for (Uint8 i = 0; i < 6; ++i) {
		Field & f = *n[i].field;
		const int diff = height - f.get_height();
		if (diff > MAX_FIELD_HEIGHT_DIFF) {
			++area;
			f.set_height(height-MAX_FIELD_HEIGHT_DIFF);
			check[i] = true;
		}
		if (diff < -MAX_FIELD_HEIGHT_DIFF) {
			++area;
			f.set_height(height+MAX_FIELD_HEIGHT_DIFF);
			check[i] = true;
		}
	}

	for (Uint8 i = 0; i < 6; ++i)
		if (check[i]) check_neighbour_heights(n[i], area);
}


#define MAX_RADIUS 32
Military_Influence Map::calc_influence
(const Coords a, const Area<> area) const
{
	const X_Coordinate w = get_width();
	const Y_Coordinate h = get_height();
	Military_Influence influence = std::max
		(std::min
		 (std::min(abs(a.x - area.x), abs(a.x - area.x + w)),
		  abs(a.x - area.x - w)),
		 std::min
		 (std::min(abs(a.y - area.y), abs(a.y - area.y + h)),
		  abs(a.y - area.y - h)));

	//  This method makes a "parabola" like x^4, but the maxium radius is
	//  MAX_RADIUS,
	if (influence > area.radius) influence = 0;
	else if (influence == 0) influence = MAX_RADIUS;
	else influence = MAX_RADIUS - influence;
	influence *= influence;
	influence *= influence;

   return influence;
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

Bob search functors

==============================================================================
*/
bool FindBobAttribute::accept(Bob *bob) const
{
	return bob->has_attribute(m_attrib);
}



/*
==============================================================================

Field search functors

==============================================================================
*/


void FindNodeAnd::add(const FindNode* findfield, bool negate)
{
	Subfunctor sf;

	sf.negate = negate;
	sf.findfield = findfield;

	m_subfunctors.push_back(sf);
}

bool FindNodeAnd::accept(const Map & map, const FCoords coord) const {
	for (std::vector<Subfunctor>::const_iterator it = m_subfunctors.begin();
	    it != m_subfunctors.end();
		 ++it)
	{
		if (it->findfield->accept(map, coord) == it->negate)
			return false;
	}

	return true;
}


bool FindNodeCaps::accept(const Map &, const FCoords coord) const {
	uchar fieldcaps = coord.field->get_caps();

	if ((fieldcaps & BUILDCAPS_SIZEMASK) < (m_mincaps & BUILDCAPS_SIZEMASK))
		return false;

	if ((m_mincaps & ~BUILDCAPS_SIZEMASK) & ~(fieldcaps & ~BUILDCAPS_SIZEMASK))
		return false;

	return true;
}

bool FindNodeSize::accept(const Map &, const FCoords coord) const {
	BaseImmovable* imm = coord.field->get_immovable();
	bool hasrobust = (imm && imm->get_size() > BaseImmovable::NONE);
	uchar fieldcaps = coord.field->get_caps();

	if (hasrobust)
		return false;

	switch (m_size) {
	default:
	case sizeAny:    return true;
	case sizeBuild:  return (fieldcaps & (BUILDCAPS_SIZEMASK | BUILDCAPS_FLAG | BUILDCAPS_MINE));
	case sizeMine:   return (fieldcaps & BUILDCAPS_MINE);
	case sizePort:   return (fieldcaps & BUILDCAPS_PORT);
	case sizeSmall:  return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_SMALL;
	case sizeMedium: return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_MEDIUM;
	case sizeBig:    return (fieldcaps & BUILDCAPS_SIZEMASK) >= BUILDCAPS_BIG;
	}
}

bool FindNodeSizeResource::accept(const Map & map, const FCoords coord) const {
	if (FindNodeSize::accept(map, coord)) {
   if (coord.field->get_resources()==m_res &&
         coord.field->get_resources_amount())
      return true;
	}
   return false;
}

bool FindNodeImmovableSize::accept(const Map &, const FCoords coord) const {
	BaseImmovable* imm = coord.field->get_immovable();
	int size = BaseImmovable::NONE;

	if (imm)
		size = imm->get_size();

	switch (size) {
	case BaseImmovable::NONE: return m_sizes & sizeNone;
	case BaseImmovable::SMALL: return m_sizes & sizeSmall;
	case BaseImmovable::MEDIUM: return m_sizes & sizeMedium;
	case BaseImmovable::BIG: return m_sizes & sizeBig;
	default: throw wexception("FindNodeImmovableSize: bad size = %i", size);
	}
}


bool FindNodeImmovableAttribute::accept(const Map &, const FCoords coord) const
{
	BaseImmovable* imm = coord.field->get_immovable();

	if (!imm)
		return false;

	return imm->has_attribute(m_attribute);
}


bool FindNodeResource::accept(const Map &, const FCoords coord) const {
	uchar res = coord.field->get_resources();
   uchar amount = coord.field->get_resources_amount();

	if ((res==m_resource) && amount)
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
bool CheckStepDefault::allowed
(Map *, FCoords start, FCoords end, int, StepId) const
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
bool CheckStepWalkOn::allowed
(Map *, FCoords start, FCoords end, int, StepId id) const
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

bool CheckStepWalkOn::reachabledest(Map *, FCoords) const {
	// Don't bother solving this.
	return true;
}


/*
===============
CheckStepRoad
===============
*/
bool CheckStepRoad::allowed
(Map* map, FCoords start, FCoords end, int, StepId id) const
{
	const Uint8 endcaps = m_player.get_buildcaps(end);

	// Calculate cost and passability
	if
		(not (endcaps & m_movecaps)
		 and
		 not
		 ((endcaps & MOVECAPS_WALK)
		  and
		  (m_player.get_buildcaps(start) & m_movecaps & MOVECAPS_SWIM)))
		return false;

	// Check for blocking immovables
	BaseImmovable *imm = map->get_immovable(end);
	if (imm && imm->get_size() >= BaseImmovable::SMALL) {
		if (id != stepLast)
			return false;

		if
			(not
			 (dynamic_cast<const Flag *>(imm)
			  or
			  (dynamic_cast<const Road *>(imm) and endcaps & BUILDCAPS_FLAG)))
			return false;
	}

	return //  Is the field forbidden?
		not m_forbidden_locations
		or
		m_forbidden_locations->find(end) == m_forbidden_locations->end();
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
Path::Path(CoordPath & o) :
m_start(o.get_start()),
m_end  (o.get_end()),
m_path (o.steps())
{std::reverse(m_path.begin(), m_path.end());} //  Path stores in reversed order!

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

	for (uint i = 0; i < m_path.size(); i++)
		m_path[i] = get_reverse_dir(m_path[i]);
}

/*
===============
Path::append

Add the given step at the end of the path.
===============
*/
void Path::append(const Map & map, const Direction dir) {
	m_path.insert(m_path.begin(), dir); // stores in reversed order!
	map.get_neighbour(m_end, dir, &m_end);
}


/*
===============
CoordPath::CoordPath

Initialize from a path, calculating the coordinates as needed
===============
*/
CoordPath::CoordPath(const Map & map, const Path & path) {
	m_coords.clear();
	m_path.clear();

	m_coords.push_back(path.get_start());

	Coords c = path.get_start();

	Path::Step_Vector::size_type nr_steps = path.get_nsteps();
	for (Path::Step_Vector::size_type i = 0; i < nr_steps; ++i) {
		const char dir = path[i];

		m_path.push_back(dir);
		map.get_neighbour(c, dir, &c);
		m_coords.push_back(c);
	}
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
	for (uint i = 0; i < m_coords.size(); i++)
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

	for (uint i = 0; i < m_path.size(); i++)
		m_path[i] = get_reverse_dir(m_path[i]);
}


/*
===============
CoordPath::truncate

Truncate the path after the given number of steps
===============
*/
void CoordPath::truncate(const std::vector<char>::size_type after) {
	assert(after <= m_path.size());

	m_path.erase(m_path.begin()+after, m_path.end());
	m_coords.erase(m_coords.begin()+after+1, m_coords.end());
}

/*
===============
CoordPath::starttrim

Opposite of truncate: remove the first n steps of the path.
===============
*/
void CoordPath::starttrim(const std::vector<char>::size_type before) {
	assert(before <= m_path.size());

	m_path.erase(m_path.begin(), m_path.begin()+before);
	m_coords.erase(m_coords.begin(), m_coords.begin()+before);
}

/*
===============
CoordPath::append

Append the given path. Automatically created the necessary coordinates.
===============
*/
void CoordPath::append(const Map & map, const Path & tail) {
	assert(tail.get_start() == get_end());

	Coords c = get_end();

	const Path::Step_Vector::size_type nr_steps = tail.get_nsteps();
	for (CoordPath::Step_Vector::size_type i = 0; i < nr_steps; ++i) {
		const char dir = tail[i];

		map.get_neighbour(c, dir, &c);
		m_path.push_back(dir);
		m_coords.push_back(c);
	}

	// debug
	//log("CoordPath; start %i %i\n", m_coords[0].x, m_coords[0].y);
	//for (uint i = 0; i < m_path.size(); i++)
	//log("  %i -> %i %i\n", m_path[i], m_coords[i+1].x, m_coords[i+1].y);
}

/*
===============
CoordPath::append

Append the given path.
===============
*/
void CoordPath::append(const CoordPath &tail)
{
	assert(tail.get_start() == get_end());

	m_path.insert(m_path.end(), tail.m_path.begin(), tail.m_path.end());
	m_coords.insert(m_coords.end(), tail.m_coords.begin()+1, tail.m_coords.end());
}
