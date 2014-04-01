/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/map.h"

#include <algorithm>
#include <cstdio>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "build_info.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "logic/checkstep.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/mapfringeregion.h"
#include "logic/pathfield.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/worlddata.h"
#include "map_generator.h"
#include "map_io/widelands_map_loader.h"
#include "s2map.h"
#include "upcast.h"
#include "wexception.h"
#include "wui/overlay_manager.h"


namespace Widelands {


/*
==============================================================================

Map IMPLEMENTATION

==============================================================================
*/


/** class Map
 *
 * This really identifies a map like it is in the game
 */

Map::Map() :
m_nrplayers      (0),
m_scenario_types (NO_SCENARIO),
m_width          (0),
m_height         (0),
m_world          (nullptr),
m_starting_pos   (nullptr),
m_fields         (nullptr),
m_pathfieldmgr   (new PathfieldManager)
{
	m_worldname[0] = '\0';
}


Map::~Map()
{
	cleanup();
}

void Map::recalc_border(const FCoords fc) {
	if (const Player_Number owner = fc.field->get_owned_by()) {
		//  A node that is owned by a player and has a neighbour that is not owned
		//  by that player is a border node.
		for (uint8_t i = 1; i <= 6; ++i) {
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
			recalc_nodecaps_pass1(mr.location());
		} while (mr.advance(*this));
	}

	{ //  Second pass.
		MapRegion<Area<FCoords> > mr(*this, area);
		do recalc_nodecaps_pass2(mr.location()); while (mr.advance(*this));
	}

	{ //  Now only recaluclate the overlays.
		Overlay_Manager & om = overlay_manager();
		MapRegion<Area<FCoords> > mr(*this, area);
		do om.recalc_field_overlays(mr.location()); while (mr.advance(*this));
	}
}


/*
===========

this recalculates all data that needs to be recalculated.
This is only needed when all fields have change, this means
a map has been loaded or newly created or in the editor that
the overlays have completely changed.
===========
*/
void Map::recalc_whole_map()
{
	assert(m_overlay_manager);

	//  Post process the map in the necessary two passes to calculate
	//  brightness and building caps
	FCoords f;

	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x) {
			f = get_fcoords(Coords(x, y));
			uint32_t radius;
			check_neighbour_heights(f, radius);
			recalc_brightness     (f);
			recalc_border         (f);
			recalc_nodecaps_pass1  (f);
		}

	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x) {
			f = get_fcoords(Coords(x, y));
			recalc_nodecaps_pass2(f);
		}

	//  Now only recaluclate the overlays.
	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x)
			overlay_manager().recalc_field_overlays(get_fcoords(Coords(x, y)));
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
			f = get_fcoords(Coords(x, y));
			//  only on unset nodes
			if (f.field->get_resources() != 0 || f.field->get_resources_amount())
				continue;
			std::map<int32_t, int32_t> m;
			int32_t amount = 0;

			//  this node
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

			//  If one of the neighbours is unpassable, count its resource
			//  stronger
			//  top left neigbour
			get_neighbour(f, WALK_NW, &f1);
			{
				const Terrain_Descr& terr = w.terrain_descr(f1.field->terrain_r());
				const int8_t resr = terr.get_default_resources();
				const int default_amount = terr.get_default_resources_amount();
				if ((terr.get_is() & TERRAIN_UNPASSABLE) && default_amount > 0)
					m[resr] += 3;
				else
					++m[resr];
				amount += default_amount;
			}
			{
				const Terrain_Descr & terd = w.terrain_descr(f1.field->terrain_d());
				const int8_t resd = terd.get_default_resources();
				const int default_amount = terd.get_default_resources_amount();
				if ((terd.get_is() & TERRAIN_UNPASSABLE) && default_amount > 0)
					m[resd] += 3;
				else
					++m[resd];
				amount += default_amount;
			}

			//  top right neigbour
			get_neighbour(f, WALK_NE, &f1);
			{
				const Terrain_Descr& terd = w.terrain_descr(f1.field->terrain_d());
				const int8_t resd = terd.get_default_resources();
				const int default_amount = terd.get_default_resources_amount();
				if ((terd.get_is() & TERRAIN_UNPASSABLE) && default_amount > 0)
					m[resd] += 3;
				else ++m[resd];
				amount += default_amount;
			}

			//  left neighbour
			get_neighbour(f, WALK_W, &f1);
			{
				const Terrain_Descr & terr = w.terrain_descr(f1.field->terrain_r());
				const int8_t resr = terr.get_default_resources();
				const int default_amount = terr.get_default_resources_amount();
				if ((terr.get_is() & TERRAIN_UNPASSABLE) && default_amount > 0)
					m[resr] += 3;
				else ++m[resr];
				amount += default_amount;
			}

			int32_t lv  = 0;
			int32_t res = 0;
			std::map<int32_t, int32_t>::iterator i = m.begin();
			while (i != m.end()) {
				if (i->second > lv) {
					lv  = i->second;
					res = i->first;
				}
				++i;
			}
			amount /= 6;

			if (res == -1 or not amount) {
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
remove your world, remove your data
go back to your initial state
===============
*/
void Map::cleanup() {
	m_nrplayers = 0;
	m_width = m_height = 0;

	free(m_fields);
	m_fields = nullptr;
	free(m_starting_pos);
	m_starting_pos = nullptr;
	delete m_world;
	m_world = nullptr;

	m_scenario_tribes.clear();
	m_scenario_names.clear();
	m_scenario_ais.clear();
	m_scenario_closeables.clear();

	m_tags.clear();
	m_hint = std::string();
	m_background = std::string();

	m_overlay_manager.reset();
	mom().remove_all();

	m_port_spaces.clear();
}

/*
===========
creates an empty map without name with
the given data
===========
*/
void Map::create_empty_map
	(uint32_t const w, uint32_t const h,
	 const std::string & worldname,
	 char const * const name,
	 char const * const author,
	 char const * const description)
{
	set_world_name(worldname.c_str());
	load_world();
	set_size(w, h);
	set_name       (name);
	set_author     (author);
	set_description(description);
	set_nrplayers(1);
	// Set first tribe found as the "basic" tribe
	// <undefined> (as set before) is useless and will lead to a
	// crash -> Widelands will search for tribe "<undefined>"
	set_scenario_player_tribe(1, Tribe_Descr::get_all_tribenames()[0]);
	set_scenario_player_name(1, _("Player 1"));
	set_scenario_player_ai(1, "");
	set_scenario_player_closeable(1, false);

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


void Map::set_origin(Coords const new_origin) {
	assert(0 <= new_origin.x);
	assert     (new_origin.x < m_width);
	assert(0 <= new_origin.y);
	assert     (new_origin.y < m_height);

	for (uint8_t i = get_nrplayers();              i;)
		m_starting_pos[--i].reorigin(new_origin, extent());

	Field * new_field_order = static_cast<Field *>(malloc(sizeof(Field)* m_width * m_height));
	memset(new_field_order, 0, sizeof(Field) * m_width * m_height);

	// Rearrange The fields
	// NOTE because of the triangle design, we have to take special care about cases
	// NOTE where y is changed by an odd number
	bool yisodd = (new_origin.y % 2) != 0;
	for (FCoords c(Coords(0, 0)); c.y < m_height; ++c.y) {
		bool cyisodd = (c.y % 2) != 0;
		for (c.x = 0; c.x < m_width; ++c.x) {
			Coords temp;
			if (yisodd && cyisodd)
				temp = Coords(c.x + new_origin.x + 1, c.y + new_origin.y);
			else
				temp = Coords(c.x + new_origin.x, c.y + new_origin.y);
			normalize_coords(temp);
			new_field_order[get_index(c, m_width)] = operator[](temp);
		}
	}
	// Now that we restructured the fields, we just overwrite the old order
	m_fields = new_field_order;

	//  Inform immovables and bobs about their new coordinates.
	for (FCoords c(Coords(0, 0), m_fields); c.y < m_height; ++c.y)
		for (c.x = 0; c.x < m_width; ++c.x, ++c.field) {
			assert(c.field == &operator[] (c));
			if (upcast(Immovable, immovable, c.field->get_immovable()))
				immovable->m_position = c;
			for
				(Bob * bob = c.field->get_first_bob();
				 bob;
				 bob = bob->get_next_bob())
			{
				bob->m_position.x     = c.x;
				bob->m_position.y     = c.y;
				bob->m_position.field = c.field;
			}
		}

	// Take care about port spaces
	PortSpacesSet new_port_spaces;
	for (PortSpacesSet::iterator it = m_port_spaces.begin(); it != m_port_spaces.end(); ++it) {
		Coords temp;
		if (yisodd && ((it->y % 2) == 0))
			temp = Coords(it->x - new_origin.x - 1, it->y - new_origin.y);
		else
			temp = Coords(it->x - new_origin.x, it->y - new_origin.y);
		normalize_coords(temp);
		log("(%i,%i) -> (%i,%i)\n", it->x, it->y, temp.x, temp.y);
		new_port_spaces.insert(temp);
	}
	m_port_spaces = new_port_spaces;

	m_overlay_manager.reset(new Overlay_Manager());
}




/*
===============
Set the size of the map. This should only happen once during initial load.
===============
*/
void Map::set_size(const uint32_t w, const uint32_t h)
{
	assert(!m_fields);

	m_width  = w;
	m_height = h;

	m_fields = static_cast<Field *>(malloc(sizeof(Field) * w * h));
	memset(m_fields, 0, sizeof(Field) * w * h);

	m_pathfieldmgr->setSize(w * h);

	m_overlay_manager.reset(new Overlay_Manager());
}

/*
 * The scenario get/set functions
 */
const std::string & Map::get_scenario_player_tribe(const Player_Number p) const
{
	assert(m_scenario_tribes.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return m_scenario_tribes[p - 1];
}

const std::string & Map::get_scenario_player_name(const Player_Number p) const
{
	assert(m_scenario_names.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return m_scenario_names[p - 1];
}

const std::string & Map::get_scenario_player_ai(const Player_Number p) const
{
	assert(m_scenario_ais.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return m_scenario_ais[p - 1];
}

bool Map::get_scenario_player_closeable(const Player_Number p) const
{
	assert(m_scenario_closeables.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return m_scenario_closeables[p - 1];
}

FileSystem* Map::filesystem() const {
	return filesystem_.get();
}

void Map::set_scenario_player_tribe(Player_Number const p, const std::string & tribename)
{
	assert(p);
	assert(p <= get_nrplayers());
	m_scenario_tribes.resize(get_nrplayers());
	m_scenario_tribes[p - 1] = tribename;
}

void Map::set_scenario_player_name(Player_Number const p, const std::string & playername)
{
	assert(p);
	assert(p <= get_nrplayers());
	m_scenario_names.resize(get_nrplayers());
	m_scenario_names[p - 1] = playername;
}

void Map::set_scenario_player_ai(Player_Number const p, const std::string & ainame)
{
	assert(p);
	assert(p <= get_nrplayers());
	m_scenario_ais.resize(get_nrplayers());
	m_scenario_ais[p - 1] = ainame;
}

void Map::set_scenario_player_closeable(Player_Number const p, bool closeable)
{
	assert(p);
	assert(p <= get_nrplayers());
	m_scenario_closeables.resize(get_nrplayers());
	m_scenario_closeables[p - 1] = closeable;
}

/*
===============
Change the number of players the map supports.
Could happen multiple times in the map editor.
===============
*/
void Map::set_nrplayers(Player_Number const nrplayers) {
	if (!nrplayers) {
		free(m_starting_pos);
		m_starting_pos = nullptr;
		m_nrplayers = 0;
		return;
	}

	Coords* new_starting_pos = static_cast<Coords *>
		(realloc(m_starting_pos, sizeof(Coords) * nrplayers));
	if (!new_starting_pos)
		throw wexception("Out of memory.");
	m_starting_pos = new_starting_pos;

	while (m_nrplayers < nrplayers)
		m_starting_pos[m_nrplayers++] = Coords(-1, -1);

	m_scenario_tribes.resize(nrplayers);
	m_scenario_ais.resize(nrplayers);
	m_scenario_closeables.resize(nrplayers);
	m_scenario_names.resize(nrplayers);
	m_scenario_tribes.resize(nrplayers);

	m_nrplayers = nrplayers; // in case the number players got less
}

/*
===============
Set the starting coordinates of a player
===============
*/
void Map::set_starting_pos(Player_Number const plnum, Coords const c)
{
	assert(1 <= plnum);
	assert     (plnum <= get_nrplayers());

	m_starting_pos[plnum - 1] = c;
}


void Map::set_filename(char const * const string)
{
	snprintf(m_filename, sizeof(m_filename), "%s", string);
}

void Map::set_author(char const * const string)
{
	snprintf(m_author, sizeof(m_author), "%s", string);
}

void Map::set_name(char const * const string)
{
	snprintf(m_name, sizeof(m_name), "%s", string);
}

void Map::set_description(char const * const string)
{
	snprintf(m_description, sizeof(m_description), "%s", string);
}

void Map::set_hint(std::string string)
{
	m_hint = string;
}

void Map::set_world_name(char const * const string)
{
	snprintf(m_worldname, sizeof(m_worldname), "%s", string);
}

void Map::set_background(char const * const string)
{
	if (string)
		m_background = string;
	else
		m_background.clear();
}

void Map::add_tag(std::string tag) {
	m_tags.insert(tag);
}

/*
===============
load the corresponding world. This should only happen
once during initial load.
===============
*/
void Map::load_world()
{
	if (!m_world)
		m_world = new World(m_worldname);
}

/// Load data for the graphics subsystem.
void Map::load_graphics()
{
	m_world->load_graphics();
}


NodeCaps Map::get_max_nodecaps(FCoords & fc) {
	NodeCaps caps = _calc_nodecaps_pass1(fc, false);
	caps = _calc_nodecaps_pass2(fc, false, caps);
	return caps;
}


/// \returns the immovable at the given coordinate
BaseImmovable * Map::get_immovable(const Coords coord) const
{
	return operator[](coord).get_immovable();
}


/*
===============
Call the functor for every field that can be reached from coord without moving
outside the given radius.

Functor is of the form: functor(Map*, FCoords)
===============
*/
template<typename functorT>
void Map::find_reachable
	(Area<FCoords> const area, const CheckStep & checkstep, functorT & functor)
{
	std::vector<Coords> queue;
	boost::shared_ptr<Pathfields> pathfields = m_pathfieldmgr->allocate();

	queue.push_back(area);

	while (queue.size()) {
		// Pop the last ware from the queue
		FCoords const cur = get_fcoords(*queue.rbegin());
		queue.pop_back();
		Pathfield & curpf = pathfields->fields[cur.field - m_fields];

		//  handle this node
		functor(*this, cur);
		curpf.cycle = pathfields->cycle;

		// Get neighbours
		for (Direction dir = 1; dir <= 6; ++dir) {
			FCoords neighb;

			get_neighbour(cur, dir, &neighb);

			if  //  node not already handled?
				(pathfields->fields[neighb.field - m_fields].cycle
				 !=
				 pathfields->cycle
				 and
				 //  node within the radius?
				 calc_distance(area, neighb) <= area.radius
				 and
				 //  allowed to move onto this node?
				 checkstep.allowed
				 	(*this,
				 	 cur,
				 	 neighb,
				 	 dir,
				 	 cur == area ? CheckStep::stepFirst : CheckStep::stepNormal))
				queue.push_back(neighb);
		}
	}
}


/*
===============
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
	FindBobsCallback(std::vector<Bob *> * const list, const FindBob & functor)
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map &, const FCoords cur) {
		for
			(Bob * bob = cur.field->get_first_bob();
			 bob;
			 bob = bob->get_next_bob())
		{
			if
				(m_list &&
				 std::find(m_list->begin(), m_list->end(), bob) != m_list->end())
				continue;

			if (m_functor.accept(bob)) {
				if (m_list)
					m_list->push_back(bob);

				++m_found;
			}
		}
	}

	std::vector<Bob *> * m_list;
	const FindBob      & m_functor;
	uint32_t                 m_found;
};


/*
===============
Find Bobs in the given area. Only finds objects for which
functor.accept() returns true (the default functor always returns true)
If list is non-zero, pointers to the relevant objects will be stored in
the list.

Returns the number of objects found.
===============
*/
uint32_t Map::find_bobs
	(Area<FCoords>        const area,
	 std::vector<Bob *> * const list,
	 const FindBob      &       functor)
{
	FindBobsCallback cb(list, functor);

	find(area, cb);

	return cb.m_found;
}


/*
===============
Find Bobs that are reachable by moving within the given radius (also see
find_reachable()).
Only finds objects for which functor.accept() returns true (the default functor
always returns true).
If list is non-zero, pointers to the relevant objects will be stored in
the list.

Returns the number of objects found.
===============
*/
uint32_t Map::find_reachable_bobs
	(Area<FCoords>        const area,
	 std::vector<Bob *> * const list,
	 const CheckStep    &       checkstep,
	 const FindBob      &       functor)
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
	FindImmovablesCallback
		(std::vector<ImmovableFound> * const list, const FindImmovable & functor)
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map &, const FCoords cur) {
		BaseImmovable * const imm = cur.field->get_immovable();

		if (!imm)
			return;

		if (m_functor.accept(*imm)) {
			if (m_list) {
				ImmovableFound imf;
				imf.object = imm;
				imf.coords = cur;
				m_list->push_back(imf);
			}

			++m_found;
		}
	}

	std::vector<ImmovableFound> * m_list;
	const FindImmovable         & m_functor;
	uint32_t                          m_found;
};


/*
===============
Find all immovables in the given area for which functor returns true
(the default functor always returns true).
Returns true if an immovable has been found.
If list is not 0, found immovables are stored in list.
===============
*/
uint32_t Map::find_immovables
	(Area<FCoords>                 const area,
	 std::vector<ImmovableFound> * const list,
	 const FindImmovable         &       functor)
{
	FindImmovablesCallback cb(list, functor);

	find(area, cb);

	return cb.m_found;
}


/*
===============
Find all immovables reachable by moving in the given radius (see
find_reachable()).
Return immovables for which functor returns true (the default functor
always returns true).
If list is not 0, found immovables are stored in list.
Returns the number of immovables we found.
===============
*/
uint32_t Map::find_reachable_immovables
	(Area<FCoords>                 const area,
	 std::vector<ImmovableFound> * const list,
	 const CheckStep             &       checkstep,
	 const FindImmovable         &       functor)
{
	FindImmovablesCallback cb(list, functor);

	find_reachable(area, checkstep, cb);

	return cb.m_found;
}


/**
 * Find all immovables that are reachable without moving out of the
 * given area with the additional constraints given by checkstep,
 * and store them uniquely in a list.
 *
 * \return the number of immovables found.
 */
uint32_t Map::find_reachable_immovables_unique
	(const Area<FCoords> area,
	 std::vector<BaseImmovable *> & list,
	 const CheckStep & checkstep,
	 const FindImmovable & functor)
{
	std::vector<ImmovableFound> duplist;
	FindImmovablesCallback cb(&duplist, FindImmovableAlwaysTrue());

	find_reachable(area, checkstep, cb);

	container_iterate_const(std::vector<ImmovableFound>, duplist, i) {
		BaseImmovable & obj = *i.current->object;
		if (std::find(list.begin(), list.end(), &obj) == list.end())
			if (functor.accept(obj))
				list.push_back(&obj);
	}

	return list.size();
}

/*
===============
FindNodesCallback

The actual logic behind find_fields and find_reachable_fields.
===============
*/
struct FindNodesCallback {
	FindNodesCallback
		(std::vector<Coords> * const list, const FindNode & functor)
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map & map, const FCoords cur) {
		if (m_functor.accept(map, cur)) {
			if (m_list)
				m_list->push_back(cur);

			++m_found;
		}
	}

	std::vector<Coords> * m_list;
	const FindNode     & m_functor;
	uint32_t                  m_found;
};


/*
===============
Fills in a list of coordinates of fields within the given area that functor
accepts.
Returns the number of matching fields.

Note that list can be 0.
===============
*/
uint32_t Map::find_fields
	(Area<FCoords>         const area,
	 std::vector<Coords> *       list,
	 const FindNode      &       functor)
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
uint32_t Map::find_reachable_fields
	(Area<FCoords>         const area,
	 std::vector<Coords> *       list,
	 const CheckStep     &       checkstep,
	 const FindNode      &       functor)
{
	FindNodesCallback cb(list, functor);

	find_reachable(area, checkstep, cb);

	return cb.m_found;
}


/*
Node attribute recalculation passes
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
1. Call recalc_brightness() and recalc_nodecaps_pass1() on all nodes
2. Call recalc_nodecaps_pass2() on all fields

Note: it is possible to leave out recalc_brightness() unless the height has
been changed.

The Field::caps calculation is split into more passes because of inter-field
dependencies.
*/

/*
===============
Fetch the slopes to neighbours and call the actual logic in Field
===============
*/
void Map::recalc_brightness(FCoords const f) {
	int32_t left, right, top_left, top_right, bottom_left, bottom_right;
	Field::Height const height = f.field->get_height();

	{
		FCoords neighbour;
		get_ln(f, &neighbour);
		left         = height - neighbour.field->get_height();
	}
	{
		FCoords neighbour;
		get_rn(f, &neighbour);
		right        = height - neighbour.field->get_height();
	}
	{
		FCoords neighbour;
		get_tln(f, &neighbour);
		top_left     = height - neighbour.field->get_height();
		get_rn(neighbour, &neighbour);
		top_right    = height - neighbour.field->get_height();
	}
	{
		FCoords neighbour;
		get_bln(f, &neighbour);
		bottom_left  = height - neighbour.field->get_height();
		get_rn(neighbour, &neighbour);
		bottom_right = height - neighbour.field->get_height();
	}
	f.field->set_brightness
		(left, right, top_left, top_right, bottom_left, bottom_right);
}


/*
===============
Recalculate the caps for the given node.
 - Check terrain types for passability and flag buildability

I hope this is understandable and maintainable.

Note: due to inter-field dependencies, nodecaps calculations are split up
into two passes. You should always perform both passes. See the comment
above recalc_brightness.
===============
*/
void Map::recalc_nodecaps_pass1(FCoords const f) {
	f.field->caps = _calc_nodecaps_pass1(f, true);
}

NodeCaps Map::_calc_nodecaps_pass1(FCoords const f, bool consider_mobs) {
	uint8_t caps = CAPS_NONE;

	// 1a) Get all the neighbours to make life easier
	const FCoords tr = tr_n(f);
	const FCoords tl = tl_n(f);
	const FCoords  l =  l_n(f);

	const World & w = world();

	uint8_t const tr_d_terrain_is =
		w.terrain_descr(tr.field->terrain_d()).get_is();
	uint8_t const tl_r_terrain_is =
		w.terrain_descr(tl.field->terrain_r()).get_is();
	uint8_t const tl_d_terrain_is =
		w.terrain_descr(tl.field->terrain_d()).get_is();
	uint8_t const  l_r_terrain_is =
		w.terrain_descr (l.field->terrain_r()).get_is();
	uint8_t const  f_d_terrain_is =
		w.terrain_descr (f.field->terrain_d()).get_is();
	uint8_t const  f_r_terrain_is =
		w.terrain_descr (f.field->terrain_r()).get_is();

	//  1b) Collect some information about the neighbours
	uint8_t cnt_unpassable = 0;
	uint8_t cnt_water = 0;
	uint8_t cnt_acid = 0;

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


	//  2) Passability

	//  2a) If any of the neigbouring triangles is walkable this node is
	//  walkable.
	if (cnt_unpassable < 6)
		caps |= MOVECAPS_WALK;

	//  2b) If all neighbouring triangles are water, the node is swimable.
	if (cnt_water == 6)
		caps |= MOVECAPS_SWIM;


	// 2c) [OVERRIDE] If any of the neighbouring triangles is really "bad" (such
	// as lava), we can neither walk nor swim to this node.
	if (cnt_acid)
		caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);

	//  === everything below is used to check buildability ===

	// if we are interested in the maximum theoretically available NodeCaps, this is not run
	if (consider_mobs) {
		//  3) General buildability check: if a "robust" Map_Object is on this node
		//  we cannot build anything on it. Exception: we can build flags on roads.
		if (BaseImmovable * const imm = get_immovable(f))
			if
				(not dynamic_cast<Road const *>(imm)
				&&
				imm->get_size() >= BaseImmovable::SMALL)
			{
				// 3b) [OVERRIDE] check for "unpassable" Map_Objects
				if (!imm->get_passable())
					caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);
				return static_cast<NodeCaps>(caps);
			}
	}

	//  4) Flags
	//  We can build flags on anything that's walkable and buildable, with some
	//  restrictions
	if (caps & MOVECAPS_WALK) {
		//  4b) Flags must be at least 2 edges apart
		if (consider_mobs && find_immovables(Area<FCoords>(f, 1), nullptr, FindImmovableType(Map_Object::FLAG)))
			return static_cast<NodeCaps>(caps);
		caps |= BUILDCAPS_FLAG;
	}
	return static_cast<NodeCaps>(caps);
}


/*
===============
Second pass of nodecaps. Determine which kind of building (if any) can be built
on this Field.

Important: flag buildability has already been checked in the first pass.
===============
*/
void Map::recalc_nodecaps_pass2(const FCoords & f) {
	f.field->caps = _calc_nodecaps_pass2(f, true);
}

NodeCaps Map::_calc_nodecaps_pass2(FCoords const f, bool consider_mobs, NodeCaps initcaps) {
	uint8_t caps = consider_mobs ? f.field->caps : static_cast<uint8_t>(initcaps);

	// NOTE  This dependency on the bottom-right neighbour is the reason
	// NOTE  why the caps calculation is split into two passes
	// NOTE  However this dependency has to be recalculated in case we are interested in the
	// NOTE  maximum possible NodeCaps for this FCoord
	const FCoords br = br_n(f);
	if (consider_mobs) {
		if
			(!(br.field->caps & BUILDCAPS_FLAG)
			&&
			(!br.field->get_immovable() || br.field->get_immovable()->get_type() != Map_Object::FLAG))
			return static_cast<NodeCaps>(caps);
	} else {
		if (!(_calc_nodecaps_pass1(br, false) & BUILDCAPS_FLAG))
			return static_cast<NodeCaps>(caps);
	}

	bool mine;
	uint8_t buildsize = calc_buildsize(f, true, &mine, consider_mobs, initcaps);
	if (buildsize < BaseImmovable::SMALL)
		return static_cast<NodeCaps>(caps);
	assert(buildsize >= BaseImmovable::SMALL && buildsize <= BaseImmovable::BIG);

	if (buildsize == BaseImmovable::BIG) {
		if
			(calc_buildsize(l_n(f),  false, nullptr, consider_mobs, initcaps) < BaseImmovable::BIG ||
			 calc_buildsize(tl_n(f), false, nullptr, consider_mobs, initcaps) < BaseImmovable::BIG ||
			 calc_buildsize(tr_n(f), false, nullptr, consider_mobs, initcaps) < BaseImmovable::BIG)
			buildsize = BaseImmovable::MEDIUM;
	}

	// Reduce building size if it would block connectivity
	if (buildsize == BaseImmovable::BIG) {
		static const WalkingDir cycledirs[10] = {
			WALK_NE, WALK_NE, WALK_NW,
			WALK_W, WALK_W, WALK_SW, WALK_SW,
			WALK_SE, WALK_E, WALK_E
		};
		if (!is_cycle_connected(br, 10, cycledirs))
			buildsize = BUILDCAPS_MEDIUM;
	}
	if (buildsize < BaseImmovable::BIG) {
		static const WalkingDir cycledirs[6] = {
			WALK_NE, WALK_NW, WALK_W, WALK_SW, WALK_SE, WALK_E
		};
		if (!is_cycle_connected(br, 6, cycledirs))
			return static_cast<NodeCaps>(caps);
	}

	if (mine) {
		if (static_cast<int32_t>(br.field->get_height()) - f.field->get_height() < 4)
			caps |= BUILDCAPS_MINE;
	} else {
		Field::Height const f_height = f.field->get_height();

		// Reduce building size based on slope of direct neighbours:
		//  - slope >= 4: can't build anything here -> return
		//  - slope >= 3: maximum size is small
		{
			MapFringeRegion<Area<FCoords> > mr(*this, Area<FCoords>(f, 1));
			do {
				uint16_t const slope =
					abs(mr.location().field->get_height() - f_height);
				if (slope >= 4)
					return static_cast<NodeCaps>(caps);
				if (slope >= 3)
					buildsize = BaseImmovable::SMALL;
			} while (mr.advance(*this));
		}
		if (abs(br.field->get_height() - f_height) >= 2)
			return static_cast<NodeCaps>(caps);

		// Reduce building size based on height diff. of second order
		// neighbours  If height difference between this field and second
		// order neighbour is >= 3, we can only build a small house here.
		// Additionally, we can potentially build a port on this field
		// if one of the second order neighbours is swimmable.
		if (buildsize >= BaseImmovable::MEDIUM) {
			MapFringeRegion<Area<FCoords> > mr(*this, Area<FCoords>(f, 2));

			do {
				if (abs(mr.location().field->get_height() - f_height) >= 3) {
					buildsize = BaseImmovable::SMALL;
					break;
				}
			} while (mr.advance(*this));
		}

		if ((buildsize == BaseImmovable::BIG) && is_port_space(f) && !find_portdock(f).empty())
			caps |= BUILDCAPS_PORT;

		caps |= buildsize;
	}
	return static_cast<NodeCaps>(caps);
}

/**
 * Return the size of immovable that is supposed to be buildable on \p f,
 * based on immovables on \p f and its neighbours.
 * Sets \p ismine depending on whether the field is on mountaineous terrain
 * or not.
 * \p consider_mobs defines, whether mapobjects currently on \p f or neighbour fields should be considered
 * for the calculation. If not (calculation of maximum theoretical possible buildsize) initcaps must be set.
 */
int Map::calc_buildsize
	(const FCoords & f, bool avoidnature, bool * ismine, bool consider_mobs, NodeCaps initcaps)
{
	if (consider_mobs) {
		if (!(f.field->get_caps() & MOVECAPS_WALK))
			return BaseImmovable::NONE;
		if (BaseImmovable const * const immovable = get_immovable(f))
			if (immovable->get_size() >= BaseImmovable::SMALL)
				return BaseImmovable::NONE;
	} else
		if (!(initcaps & MOVECAPS_WALK))
			return BaseImmovable::NONE;

	// Get all relevant neighbours and count terrain triangle types.
	const FCoords tr = tr_n(f);
	const FCoords tl = tl_n(f);
	const FCoords  l =  l_n(f);

	const World & w = world();

	uint8_t terrains[6] = {
		w.terrain_descr(tr.field->terrain_d()).get_is(),
		w.terrain_descr(tl.field->terrain_r()).get_is(),
		w.terrain_descr(tl.field->terrain_d()).get_is(),
		w.terrain_descr (l.field->terrain_r()).get_is(),
		w.terrain_descr (f.field->terrain_d()).get_is(),
		w.terrain_descr (f.field->terrain_r()).get_is()
	};

	uint32_t cnt_mountain = 0;
	uint32_t cnt_dry = 0;
	for (uint32_t i = 0; i < 6; ++i) {
		if (terrains[i] & TERRAIN_WATER)
			return BaseImmovable::NONE;
		if (terrains[i] & TERRAIN_MOUNTAIN) ++cnt_mountain;
		if (terrains[i] & TERRAIN_DRY) ++cnt_dry;
	}

	if (cnt_mountain == 6) {
		if (ismine)
			*ismine = true;
		return BaseImmovable::SMALL;
	}
	if (cnt_mountain || cnt_dry)
		return BaseImmovable::NONE;

	// Adjust size based on neighbouring immovables
	int buildsize = BaseImmovable::BIG;
	if (consider_mobs) {
		std::vector<ImmovableFound> objectlist;
		find_immovables
			(Area<FCoords>(f, 1),
			&objectlist,
			FindImmovableSize(BaseImmovable::SMALL, BaseImmovable::BIG));
		for (uint32_t i = 0; i < objectlist.size(); ++i) {
			const BaseImmovable * obj = objectlist[i].object;
			int objsize = obj->get_size();
			if (objsize == BaseImmovable::NONE)
				continue;
			if (avoidnature && obj->get_type() == Map_Object::IMMOVABLE)
				objsize += 1;
			if (objsize + buildsize > BaseImmovable::BIG)
				buildsize = BaseImmovable::BIG - objsize + 1;
		}
	}

	if (ismine)
		*ismine = false;
	return buildsize;
}

/**
 * We call a cycle on the map simply connected
 * if the subgraph of the cycle which can be walked on is connected.
 *
 * The cycle is described as a \p start point plus
 * a description of the directions in which to walk from the starting point.
 * The array \p dirs must have length \p length, where \p length is
 * the length of the cycle.
 */
bool Map::is_cycle_connected
	(const FCoords & start, uint32_t length, const WalkingDir * dirs)
{
	FCoords f = start;
	bool prev_walkable = start.field->get_caps() & MOVECAPS_WALK;
	uint32_t alternations = 0;

	for (uint32_t i = 0; i < length; ++i) {
		f = get_neighbour(f, dirs[i]);
		const bool walkable = f.field->get_caps() & MOVECAPS_WALK;
		alternations += walkable != prev_walkable;
		if (alternations > 2)
			return false;
		prev_walkable = walkable;
	}

	assert(start == f);

	return true;
}

/**
 * Returns a list of portdock fields (if any) that a port built at \p c should have.
 */
std::vector<Coords> Map::find_portdock(const Coords & c) const
{
	static const WalkingDir cycledirs[16] = {
		WALK_NE, WALK_NE, WALK_NE, WALK_NW, WALK_NW,
		WALK_W, WALK_W, WALK_W,
		WALK_SW, WALK_SW, WALK_SW, WALK_SE, WALK_SE,
		WALK_E, WALK_E, WALK_E
	};
	const FCoords start = br_n(br_n(get_fcoords(c)));
	FCoords f[16];
	bool iswater[16];
	int firstwater = -1;
	int lastnonwater = -1;
	f[0] = start;
	for (uint32_t i = 0; i < 16; ++i) {
		iswater[i] = (f[i].field->get_caps() & (MOVECAPS_SWIM|MOVECAPS_WALK)) == MOVECAPS_SWIM;
		if (iswater[i]) {
			if (firstwater < 0)
				firstwater = i;
		} else {
			lastnonwater = i;
		}
		if (i < 15)
			f[i + 1] = get_neighbour(f[i], cycledirs[i]);
	}

	std::vector<Coords> portdock;
	if (firstwater >= 0) {
		for (uint32_t i = firstwater; i < 16 && iswater[i]; ++i)
			portdock.push_back(f[i]);
		if (firstwater == 0 && lastnonwater >= 0) {
			for (uint32_t i = lastnonwater + 1; i < 16; ++i)
				portdock.push_back(f[i]);
		}
	}

	return portdock;
}

/// \returns true, if Coordinates are in port space list
bool Map::is_port_space(const Coords& c) {
	return m_port_spaces.count(c);
}

/// Set or unset a space as port space
void Map::set_port_space(Coords c, bool allowed) {
	if (allowed) {
		m_port_spaces.insert(c);
	} else {
		m_port_spaces.erase(c);
	}
}


/**
 * Calculate the (Manhattan) distance from a to b
 * a and b are expected to be normalized!
 */
uint32_t Map::calc_distance(const Coords a, const Coords b) const
{
	uint32_t dist;
	int32_t dy;

	// do we fly up or down?
	dy = b.y - a.y;
	if (dy > static_cast<int32_t>(m_height >> 1)) //  wrap-around!
		dy -= m_height;
	else if (dy < -static_cast<int32_t>(m_height >> 1))
		dy += m_height;

	dist = abs(dy);

	if (static_cast<X_Coordinate>(dist) >= m_width)
		// no need to worry about x movement at all
		return dist;

	// [lx..rx] is the x-range we can cover simply by walking vertically
	// towards b
	// Hint: (~a.y & 1) is 1 for even rows, 0 for odd rows.
	// This means we round UP for even rows, and we round DOWN for odd rows.
	int32_t lx, rx;

	lx = a.x - ((dist + (~a.y & 1)) >> 1); // div 2
	rx = lx + dist;

	// Allow for wrap-around
	// Yes, the second is an else if; see the above if (dist >= m_width)
	if (lx < 0)
		lx += m_width;
	else if (rx >= static_cast<int32_t>(m_width))
		rx -= m_width;

	// Normal, non-wrapping case
	if (lx <= rx)
	{
		if (b.x < lx) {
			int32_t dx1 = lx - b.x;
			int32_t dx2 = b.x - (rx - m_width);
			dist += std::min(dx1, dx2);
		}
		else if (b.x > rx)
		{
			int32_t dx1 = b.x - rx;
			int32_t dx2 = (lx + m_width) - b.x;
			dist += std::min(dx1, dx2);
		}
	}
	else
	{
		// Reverse case
		if (b.x > rx && b.x < lx)
		{
			int32_t dx1 = b.x - rx;
			int32_t dx2 = lx - b.x;
			dist += std::min(dx1, dx2);
		}
	}

	return dist;
}


#define BASE_COST_PER_FIELD 1800
#define SLOPE_COST_DIVISOR  50
#define SLOPE_COST_STEPS    8

/*
===============
Calculates the cost estimate between the two points.
This function is used mainly for the path-finding estimate.
===============
*/
int32_t Map::calc_cost_estimate(const Coords a, const Coords b) const
{
	return calc_distance(a, b) * BASE_COST_PER_FIELD;
}


/**
 * \return a lower bound on the time required to walk from \p a to \p b
 */
int32_t Map::calc_cost_lowerbound(const Coords a, const Coords b) const
{
	return calc_distance(a, b) * calc_cost(-SLOPE_COST_STEPS);
}


/*
===============
Calculate the hard cost of walking the given slope (positive means up,
negative means down).
The cost is in milliseconds it takes to walk.

The time is calculated as BASE_COST_PER_FIELD * f, where

f = 1.0 + d(Slope) - d(0)
d = (Slope + SLOPE_COST_STEPS) * (Slope + SLOPE_COST_STEPS - 1)
       / (2 * SLOPE_COST_DIVISOR)

Note that the actual calculations multiply through by (2 * SLOPE_COST_DIVISOR)
to avoid using floating point numbers in game logic code.

Slope is limited to the range [ -SLOPE_COST_STEPS; +oo [
===============
*/
#define CALC_COST_D(slope)                                             \
   (((slope) + SLOPE_COST_STEPS) * ((slope) + SLOPE_COST_STEPS - 1))

static int32_t calc_cost_d(int32_t slope)
{
	if (slope < -SLOPE_COST_STEPS)
		slope = -SLOPE_COST_STEPS;

	return CALC_COST_D(slope);
}

int32_t Map::calc_cost(int32_t const slope) const
{
	return
		BASE_COST_PER_FIELD
		*
		(2 * SLOPE_COST_DIVISOR + calc_cost_d(slope) - CALC_COST_D(0))
		/
		(2 * SLOPE_COST_DIVISOR);
}


/*
===============
Return the time it takes to walk the given step from coords in the given
direction, in milliseconds.
===============
*/
int32_t Map::calc_cost(const Coords coords, const int32_t dir) const
{
	FCoords f;
	int32_t startheight;
	int32_t delta;

	// Calculate the height delta
	f = get_fcoords(coords);
	startheight = f.field->get_height();

	get_neighbour(f, dir, &f);
	delta = f.field->get_height() - startheight;

	return calc_cost(delta);
}


/*
===============
Calculate the average cost of walking the given step in both directions.
===============
*/
int32_t Map::calc_bidi_cost(const Coords coords, const int32_t dir) const
{
	FCoords f;
	int32_t startheight;
	int32_t delta;

	// Calculate the height delta
	f = get_fcoords(coords);
	startheight = f.field->get_height();

	get_neighbour(f, dir, &f);
	delta = f.field->get_height() - startheight;

	return (calc_cost(delta) + calc_cost(-delta)) / 2;
}


/*
===============
Calculate the cost of walking the given path.
If either of the forward or backward pointers is set, it will be filled in
with the cost of walking in said direction.
===============
*/
void Map::calc_cost
	(const Path & path, int32_t * const forward, int32_t * const backward) const
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

/// Get a node's neighbour by direction.
void Map::get_neighbour
	(const Coords & f, Direction const dir, Coords * const o) const
{
	switch (dir) {
	case WALK_NW: get_tln(f, o); break;
	case WALK_NE: get_trn(f, o); break;
	case WALK_E:  get_rn (f, o); break;
	case WALK_SE: get_brn(f, o); break;
	case WALK_SW: get_bln(f, o); break;
	case WALK_W:  get_ln (f, o); break;
	default:
		assert(false);
		break;
	}
}

void Map::get_neighbour
	(const FCoords & f, Direction const dir, FCoords * const o) const
{
	switch (dir) {
	case WALK_NW: get_tln(f, o); break;
	case WALK_NE: get_trn(f, o); break;
	case WALK_E:  get_rn (f, o); break;
	case WALK_SE: get_brn(f, o); break;
	case WALK_SW: get_bln(f, o); break;
	case WALK_W:  get_ln (f, o); break;
	default:
		assert(false);
		break;
	}
}

std::unique_ptr<Map_Loader> Map::get_correct_loader(const std::string& filename) {
	std::unique_ptr<Map_Loader> result;

	std::string lower_filename = filename;
	boost::algorithm::to_lower(lower_filename);

	if (boost::algorithm::ends_with(lower_filename, WLMF_SUFFIX)) {
		try {
			result.reset(new WL_Map_Loader(g_fs->MakeSubFileSystem(filename), this));
		} catch (...) {
			//  If this fails, it is an illegal file (maybe old plain binary map
			//  format)
			//  TODO: catchall hides real errors! Replace with more specific code
		}
	} else if (boost::algorithm::ends_with(lower_filename, S2MF_SUFFIX) ||
	           boost::algorithm::ends_with(lower_filename, S2MF_SUFFIX2)) {
		result.reset(new S2_Map_Loader(filename.c_str(), *this));
	}
	return result;
}

/**
 * Finds a path from start to end for a Map_Object with the given movecaps.
 *
 * The path is stored in \p path, as a series of Map_Object::WalkingDir entries.
 *
 * \param persist tells the function how hard it should try to find a path:
 * If \p persist is \c 0, the function will never give up early. Otherwise, the
 * function gives up when it becomes clear that the path takes longer than
 * persist * bird's distance of flat terrain.
 * Note that if the terrain contains steep hills, the cost calculation will
 * cause the search to terminate earlier than you may think. If persist==1,
 * findpath() can only find a path if the terrain is completely flat.
 *
 * \param checkstep findpath() calls this checkstep functor-like to determine
 * whether moving from one field to another is legal.
 *
 * \param instart starting point of the search
 * \param inend end point of the search
 * \param path will receive the found path if successful
 * \param flags UNDOCUMENTED
 *
 * \return the cost of the path (in milliseconds of normal walking
 * speed) or -1 if no path has been found.
 *
 * \todo Document parameters instart, inend, path, flags
 */
int32_t Map::findpath
	(Coords                  instart,
	 Coords                  inend,
	 int32_t           const persist,
	 Path            &       path,
	 const CheckStep &       checkstep,
	 uint32_t          const flags)
{
	FCoords start;
	FCoords end;
	int32_t upper_cost_limit;
	FCoords cur;

	normalize_coords(instart);
	normalize_coords(inend);

	start = FCoords(instart, &operator[](instart));
	end   = FCoords(inend,   &operator[](inend));

	path.m_path.clear();

	// Some stupid cases...
	if (start == end) {
		path.m_start = start;
		path.m_end = end;
		return 0; // duh...
	}

	if (not checkstep.reachabledest(*this, end))
		return -1;

	if (!persist)
		upper_cost_limit = 0;
	else
		// assume flat terrain
		upper_cost_limit = persist * calc_cost_estimate(start, end);

	// Actual pathfinding
	boost::shared_ptr<Pathfields> pathfields = m_pathfieldmgr->allocate();
	Pathfield::Queue Open;
	Pathfield * curpf = &pathfields->fields[start.field - m_fields];
	curpf->cycle      = pathfields->cycle;
	curpf->real_cost  = 0;
	curpf->estim_cost = calc_cost_lowerbound(start, end);
	curpf->backlink   = IDLE;

	Open.push(curpf);

	for (;;) {
		if (Open.empty()) // there simply is no path
			return -1;
		curpf = Open.top();
		Open.pop(curpf);

		cur.field = m_fields + (curpf - pathfields->fields.get());
		get_coords(*cur.field, cur);

		if (upper_cost_limit && curpf->real_cost > upper_cost_limit)
			break; // upper cost limit reached, give up
		if (cur == end)
			break; // found our target

		// avoid bias by using different orders when pathfinding
		static const int8_t order1[] =
			{WALK_NW, WALK_NE, WALK_E, WALK_SE, WALK_SW, WALK_W};
		static const int8_t order2[] =
			{WALK_NW, WALK_W, WALK_SW, WALK_SE, WALK_E, WALK_NE};
		int8_t const * direction = (cur.x + cur.y) & 1 ? order1 : order2;

		// Check all the 6 neighbours
		for (uint32_t i = 6; i; i--, direction++) {
			FCoords neighb;
			int32_t cost;

			get_neighbour(cur, *direction, &neighb);
			Pathfield & neighbpf = pathfields->fields[neighb.field - m_fields];

			// Is the field Closed already?
			if
				(neighbpf.cycle == pathfields->cycle &&
				 !neighbpf.heap_cookie.is_active())
				continue;

			// Check passability
			if
				(not
				 checkstep.allowed
				 	(*this,
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
				curpf->real_cost +
				(flags & fpBidiCost ?
				 calc_bidi_cost(cur, *direction) : calc_cost(cur, *direction));

			if (neighbpf.cycle != pathfields->cycle) {
				// add to open list
				neighbpf.cycle      = pathfields->cycle;
				neighbpf.real_cost  = cost;
				neighbpf.estim_cost = calc_cost_lowerbound(neighb, end);
				neighbpf.backlink   = *direction;
				Open.push(&neighbpf);
			} else if (neighbpf.cost() > cost + neighbpf.estim_cost) {
				// found a better path to a field that's already Open
				neighbpf.real_cost = cost;
				neighbpf.backlink = *direction;
				Open.decrease_key(&neighbpf);
			}
		}
	}

	// Now unwind the taken route (even if we couldn't find a complete one!)
	int32_t const result = cur == end ? curpf->real_cost : -1;

	path.m_start = start;
	path.m_end = cur;

	path.m_path.clear();

	while (curpf->backlink != IDLE) {
		path.m_path.push_back(curpf->backlink);

		// Reverse logic! (WALK_NW needs to find the SE neighbour)
		get_neighbour(cur, get_reverse_dir(curpf->backlink), &cur);
		curpf = &pathfields->fields[cur.field - m_fields];
	}

	return result;
}


bool Map::can_reach_by_water(const Coords field) const
{
	FCoords fc = get_fcoords(field);

	if (fc.field->nodecaps() & MOVECAPS_SWIM)
		return true;
	if (!(fc.field->nodecaps() & MOVECAPS_WALK))
		return false;

	FCoords neighb;

	for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
		if (get_neighbour(fc, dir).field->nodecaps() & MOVECAPS_SWIM)
			return true;
	}

	return false;
}

/*
===========
changes the given triangle's terrain.
this happens in the editor and might happen in the game
too if some kind of land increasement is implemented (like
drying swamps).
The nodecaps need to be recalculated

returns the radius of changes (which are always 2)
===========
*/
int32_t Map::change_terrain
	(TCoords<FCoords> const c, Terrain_Index const terrain)
{
	c.field->set_terrain(c.t, terrain);

	NoteSender<NoteFieldTransformed>::send(NoteFieldTransformed(c));

	recalc_for_field_area(Area<FCoords>(c, 2));

	return 2;
}


uint32_t Map::set_height(const FCoords fc, uint8_t const new_value) {
	assert(new_value <= MAX_FIELD_HEIGHT);
	assert(m_fields <= fc.field);
	assert            (fc.field < m_fields + max_index());
	fc.field->height = new_value;
	uint32_t radius = 2;
	check_neighbour_heights(fc, radius);
	recalc_for_field_area(Area<FCoords>(fc, radius));
	return radius;
}

uint32_t Map::change_height(Area<FCoords> area, int16_t const difference) {
	{
		MapRegion<Area<FCoords> > mr(*this, area);
		do {
			if
				(difference < 0
				 and
				 mr.location().field->height
				 <
				 static_cast<uint8_t>(-difference))
				mr.location().field->height = 0;
			else if
				(static_cast<int16_t>(MAX_FIELD_HEIGHT) - difference
				 <
				 static_cast<int16_t>(mr.location().field->height))
				mr.location().field->height = MAX_FIELD_HEIGHT;
			else  mr.location().field->height += difference;
		} while (mr.advance(*this));
	}
	uint32_t regional_radius = 0;
	MapFringeRegion<Area<FCoords> > mr(*this, area);
	do {
		uint32_t local_radius = 0;
		check_neighbour_heights(mr.location(), local_radius);
		regional_radius = std::max(regional_radius, local_radius);
	} while (mr.advance(*this));
	area.radius += regional_radius + 2;
	recalc_for_field_area(area);
	return area.radius;
}

uint32_t Map::set_height
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
void Map::check_neighbour_heights(FCoords coords, uint32_t & area)
{
	assert(m_fields <= coords.field);
	assert            (coords.field < m_fields + max_index());

	int32_t height = coords.field->get_height();
	bool check[] = {false, false, false, false, false, false};

	const FCoords n[] = {
		tl_n(coords),
		tr_n(coords),
		l_n (coords),
		r_n (coords),
		bl_n(coords),
		br_n(coords)
	};

	for (uint8_t i = 0; i < 6; ++i) {
		Field & f = *n[i].field;
		const int32_t diff = height - f.get_height();
		if (diff > MAX_FIELD_HEIGHT_DIFF) {
			++area;
			f.set_height(height - MAX_FIELD_HEIGHT_DIFF);
			check[i] = true;
		}
		if (diff < -MAX_FIELD_HEIGHT_DIFF) {
			++area;
			f.set_height(height + MAX_FIELD_HEIGHT_DIFF);
			check[i] = true;
		}
	}

	for (uint8_t i = 0; i < 6; ++i)
		if (check[i])
			check_neighbour_heights(n[i], area);
}


#define MAX_RADIUS 32
Military_Influence Map::calc_influence
	(Coords const a, Area<> const area) const
{
	const X_Coordinate w = get_width();
	const Y_Coordinate h = get_height();
	Military_Influence influence =
		std::max
			(std::min
			 	(std::min(abs(a.x - area.x), abs(a.x - area.x + w)),
			 	 abs(a.x - area.x - w)),
			 std::min
			 	(std::min(abs(a.y - area.y), abs(a.y - area.y + h)),
			 	 abs(a.y - area.y - h)));

	influence =
		influence > area.radius ? 0 :
		influence == 0          ? MAX_RADIUS :  MAX_RADIUS - influence;
	influence *= influence;

	return influence;
}

} // namespace Widelands
