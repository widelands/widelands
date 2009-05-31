/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "map.h"

#include "checkstep.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "events/event.h"
#include "events/event_chain.h"
#include "findimmovable.h"
#include "findnode.h"
#include "io/filesystem/layered_filesystem.h"
#include "map_io/widelands_map_loader.h"
#include "mapfringeregion.h"
#include "graphic/overlay_manager.h"
#include "pathfield.h"
#include "logic/player.h"
#include "s2map.h"
#include "soldier.h"
#include "tribe.h"
#include "upcast.h"
#include "wexception.h"
#include "worlddata.h"

#include "log.h"

#include <algorithm>
#include <cstdio>


#define AVG_ELEVATION   (0x80000000)
#define MAX_ELEVATION   (0xffffffff)
#define MAP_ID_DIGITS   20
#define ISLAND_BORDER   10


namespace Widelands {

/**
 * Callback function for font renderer.
 */
std::string g_VariableCallback(std::string str, void * data) {
	if (const Map * const map = static_cast<const Map *>(data))
		if (Variable const * const var = map->mvm()[str])
			return var->get_string_representation();
	return (std::string("UNKNOWN:") + str).c_str();
}

/*
==============================================================================

Map IMPLEMENTATION

==============================================================================
*/


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
m_nrplayers      (0),
m_width          (0),
m_height         (0),
m_world          (0),
m_starting_pos   (0),
m_fields         (0),
m_overlay_manager(0),
m_pathfieldmgr(new PathfieldManager)
{
	m_worldname[0] = '\0';
}


Map::~Map()
{
	cleanup();
	delete m_overlay_manager;
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
			recalc_fieldcaps_pass1(f);
		}

	for (Y_Coordinate y = 0; y < m_height; ++y)
		for (X_Coordinate x = 0; x < m_width; ++x) {
			f = get_fcoords(Coords(x, y));
			recalc_fieldcaps_pass2(f);
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
			get_neighbour(f, Map_Object::WALK_NW, &f1);
			{
				const Terrain_Descr & terr = w.terrain_descr(f1.field->terrain_r());
				const int8_t resr =
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
				const int8_t resd =
					terd.get_default_resources();
				if
					(terd.get_is() & TERRAIN_UNPASSABLE
					 and
					 resd != Descr_Maintainer<Resource_Descr>::invalid_index())
					m[resd] += 3;
				else ++m[resd];
				amount += terd.get_default_resources_amount();
			}

			//  top right neigbour
			get_neighbour(f, Map_Object::WALK_NE, &f1);
			{
				const Terrain_Descr & terd = w.terrain_descr(f1.field->terrain_d());
				const int8_t resd =
					terd.get_default_resources();
				if
					(terd.get_is() & TERRAIN_UNPASSABLE
					 and
					 resd != Descr_Maintainer<Resource_Descr>::invalid_index())
					m[resd] += 3;
				else ++m[resd];
				amount += terd.get_default_resources_amount();
			}

			//  left neighbour
			get_neighbour(f, Map_Object::WALK_W, &f1);
			{
				const Terrain_Descr & terr = w.terrain_descr(f1.field->terrain_r());
				const int8_t resr =
					terr.get_default_resources();
				if
					(terr.get_is() & TERRAIN_UNPASSABLE
					 and
					 resr != Descr_Maintainer<Resource_Descr>::invalid_index())
					m[resr] += 3;
				else ++m[resr];
				amount += terr.get_default_resources_amount();
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
Map::cleanup

remove your world, remove your data
go back to your initial state
===============
*/
void Map::cleanup() {
	m_nrplayers = 0;
	m_width = m_height = 0;

	free(m_fields);
	m_fields = 0;
	free(m_starting_pos);
	m_starting_pos = 0;
	delete m_world;
	m_world = 0;

	m_scenario_tribes.clear();
	m_scenario_names.clear();
	m_scenario_ais.clear();

	if (m_overlay_manager)
		m_overlay_manager->reset();

	mom().remove_all();
	mcm().remove_all();
	mem().remove_unreferenced();
	{
		Manager<Event>::Index nr_events = mem().size();
		for (Manager<Trigger>::Index e_idx = 0; e_idx < nr_events; ++e_idx) {
			Trigger const & event = mtm()[e_idx];
			Trigger::Referencers const referencers = event.referencers();
			container_iterate_const(Trigger::Referencers, referencers, i)
				log
					("ERROR: event %s is still referenced by %s\n",
					 event.name().c_str(), i.current->first->identifier().c_str());
		}
	}
	mtm().remove_unreferenced();
	{
		Manager<Trigger>::Index nr_triggers = mtm().size();
		for (Manager<Trigger>::Index t_idx = 0; t_idx < nr_triggers; ++t_idx) {
			Trigger const & trigger = mtm()[t_idx];
			Trigger::Referencers const referencers = trigger.referencers();
			container_iterate_const(Trigger::Referencers, referencers, i)
				log
					("ERROR: trigger %s is still referenced by %s\n",
					 trigger.name().c_str(),
					 i.current->first->identifier().c_str());
		}
	}

	mvm().remove_all();

	//  Remove all extra data. Pay attention here, maybe some freeing would be
	//  needed.
#ifdef DEBUG
	for (uint32_t i = 0; i < m_extradatainfos.size(); ++i) {
		assert(m_extradatainfos[i].type == Extradata_Info::PIC);
	}
#endif
	m_extradatainfos.clear();
}

/*
===========
creates an empty map without name with
the given data
===========
*/
void Map::create_empty_map
	(uint32_t const w, uint32_t const h,
	 std::string const & worldname,
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
	std::vector<std::string> tribes;
	Tribe_Descr::get_all_tribenames(tribes);
	set_scenario_player_tribe(1, tribes[0]);
	set_scenario_player_name(1, _("Player 1"));
	set_scenario_player_ai(1, "");

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
	for (Manager<Event>  ::Index i = mtm().size(); i;)
		mtm()         [--i].reorigin(new_origin, extent());
	for (Manager<Trigger>::Index i = mem().size(); i;)
		mem()         [--i].reorigin(new_origin, extent());


	//  Rearrange the fields. This is done in 2 steps. First each column is
	//  rotated. Then each row is rotated. How to rotate an array is described
	//  at [http://home.tiac.net/~cri/2004/frotate.html].
	uint16_t const w = m_width, h = m_height;


	{ //  Rotate each column. This code is divided into 4 cases:
		uint16_t const k = new_origin.y, nk = h - k;
		//    * k = 0:
		//        Do nothing.
		//    * k = nk:
		//        The new origin divides the column in 2 halves, which should
		//        just be swapped.
		//    * k < nk:
		//        The new origin is in the first half. Increment = k.
		//        Ascending run.
		//    * k > nk:
		//        The new origin is in the second half. Decrement = nk.
		//        Descending run.
		if        (k == nk) { //  swap first and last halves
			for (X_Coordinate x = 0; x < w; ++x)
				for (Y_Coordinate y1 = 0, y2 = k; y2 < h; ++y1, ++y2)
					std::swap
						(operator[] (Coords(x, y1)), operator[] (Coords(x, y2)));
		} else if (k <  nk) { //  ascending run
			for (X_Coordinate x = 0; x < w; ++x)
				for (uint16_t first = 0, count = 0; count < k; ++first) {
					Field const t = operator[](Coords(x, first));
					uint16_t const last = first + nk;
					for (uint16_t index = first;;) {
						++count;
						while (index < nk) {
							Field & dst = operator[](Coords(x, index));
							dst = operator[](Coords(x, index += k));
						}
						if (index == last)
							break;
						Field & dst = operator[](Coords(x, index));
						dst = operator[] (Coords(x, index -= nk));
					}
					operator[] (Coords(x, last)) = t;
				}
		} else if (k >  nk) { //  descending run
			for (X_Coordinate x = 0; x < w; ++x)
				for (uint16_t first = h - 1, count = 0; count < nk; --first) {
					Field const t = operator[](Coords(x, first));
					uint16_t const last = first - k;
					for (uint16_t index = first;;) {
						++count;
						while (index >= nk) {
							Field & dst = operator[] (Coords(x, index));
							dst = operator[] (Coords(x, index -= nk));
						}
						if (index == last)
							break;
						Field & dst = operator[] (Coords(x, index));
						dst = operator[] (Coords(x, index += k));
					}
					operator[] (Coords(x, last)) = t;
				}
		}
	}

	for (uint8_t i = 2; i;) { //  Rotate each odd row, then each even.
		--i;
		uint16_t k = new_origin.x;
		if (i and new_origin.y & 1 and ++k == w)
			k = 0;
		uint16_t const nk = w - k;
		if      (k == nk)
			for (Y_Coordinate y = i; y < h; y += 2)
				for (X_Coordinate x1 = 0, x2 = k; x2 < w; ++x1, ++x2)
					std::swap
						(operator[] (Coords(x1, y)), operator[] (Coords(x2, y)));
		else if (k <  nk)
			for (Y_Coordinate y = i; y < h; y += 2)
				for (uint16_t first = 0, count = 0; count < k; ++first) {
					Field const t = operator[] (Coords(first, y));
					uint16_t const last = first + nk;
					for (uint16_t index = first;;) {
						++count;
						while (index < nk) {
							Field & dst = operator[] (Coords(index, y));
							dst = operator[] (Coords(index += k, y));
						}
						if (index == last)
							break;
						Field & dst = operator[] (Coords(index, y));
						dst = operator[] (Coords(index -= nk, y));
					}
					operator[] (Coords(last, y)) = t;
				}
		else if (k >  nk)
			for (Y_Coordinate y = i; y < h; y += 2)
				for (uint16_t first = w - 1, count = 0; count < nk; --first) {
					Field const t = operator[] (Coords(first, y));
					uint16_t const last = first - k;
					for (uint16_t index = first;;) {
						++count;
						while (index >= nk) {
							Field & dst = operator[] (Coords(index, y));
							dst = operator[] (Coords(index -= nk, y));
						}
						if (index == last)
							break;
						Field & dst = operator[] (Coords(index, y));
						dst = operator[] (Coords(index += k, y));
					}
					operator[] (Coords(last, y)) = t;
				}
	}

	//  Inform immovables and bobs about their new coordinates.
	for (FCoords c(Coords(0, 0), m_fields); c.y < h; ++c.y)
		for (c.x = 0; c.x < w; ++c.x, ++c.field) {
			assert(c.field == &operator[] (c));
			if (upcast(Immovable, immovable, c.field->get_immovable()))
				immovable->m_position = c;
			for
				(Bob * bob = c.field->get_first_bob();
				 bob;
				 bob = bob->get_next_bob())
				bob->m_position = c;
		}
}


/*
===============
Translates a random value into a map field height. This
method is used within the random map generation methods.

ele:         Random Value ("elevation")
mapGenInfo:  Map generator information used to translate
             Random values to height information (world-
             specific info)
x:           x position within map
y:           y position within map
mapInfo:     Information about the random map currently
             begin created (map specific info)

Return value: A map height value.corresponding to ele
===============
*/
uint8_t Map::make_field_elevation
	(double ele,
	 MapGenInfo & mapGenInfo,
	 uint16_t const x, uint16_t const y,
	 UniqueRandomMapInfo const & mapInfo)
{
	int32_t const water_h  = mapGenInfo.getWaterShallowHeight();
	int32_t const mount_h  = mapGenInfo.getMountainFootHeight();
	int32_t const summit_h = mapGenInfo.getSummitHeight      ();

	double water_fac = mapInfo.waterRatio;
	double land_fac  = mapInfo.landRatio;

	uint8_t res_h;

	if (ele < water_fac)
		res_h = water_h;
	else if (ele < water_fac + land_fac)
		res_h =
			water_h + 1 + ((ele - water_fac) / land_fac) * (mount_h - water_h);
	else
		res_h =
			mount_h +
			((ele - water_fac - land_fac) / (1 - water_fac - land_fac))
			*
			(summit_h - mount_h);

	//  Handle Map Border in island mode

	if (mapInfo.islandMode) {
		int32_t const border_dist_x = x < mapInfo.w - x ? x : mapInfo.w - x;
		int32_t const border_dist_y = y < mapInfo.h - y ? y : mapInfo.h - y;
		int32_t const border_dist   =
			border_dist_x < border_dist_y ? border_dist_x : border_dist_y;

		if (border_dist <= ISLAND_BORDER) {
			res_h =
				static_cast<uint8_t>
					(static_cast<double>(res_h) * border_dist /
					 static_cast<double>(ISLAND_BORDER));
			if (res_h < water_h)
				res_h = water_h;
		}
	}

	return res_h;
}

/*
===============
Map::generate_random_value_map

Generate a "continuous" array of "reasonable" random values.
The array generated is in fact organized in a two-dimensional
way. "Reasonable" means that the values are not purely random.
Neighboring values (in a two-dimensional way) are fitting
together so that such an array can be used to directly generate
height information for mountains, wasteland, resources etc.
"Continuous" means that also value of the left border fit to
the right border values and values of the top border fit to the
bottom border values. This means we have some kind of "endlessly"
repeating set of random values.
What is more, the different heights are weighed so that the
random distribution of all random values in the array is linear.
The minimum valu will be 0, the maximum value will be MAX_ELEVATION,
the average will be AVG_ELEVATION.

w, h: are width and height of the two-dimensional array
      produced. Thus, the array has w*h entries. To access a certain
      "coordinate" in the array, use array[x+w*y] to retrieve the entry.

rng:  is the random number generator to be used.
      This will mostly be the current rng of the random map currently being
      created.
===============
*/
uint32_t * Map::generate_random_value_map
	(uint32_t const w, uint32_t const h, RNG & rng)
{
	uint32_t const numFields = h * w; //  Size of the resulting array

	uint32_t * const values = new uint32_t[numFields]; //  Array to be filled

	try {
		//  We will do some initing here...

		for (uint32_t ix = 0; ix < numFields; ++ix)
			values[ix] = AVG_ELEVATION;

		//  This will be the first starting random values...

		for (uint32_t x = 0; x < w; x += 16)
			for (uint32_t y = 0; y < h; y += 16) {
				values[x + y * w] = rng.rand();
				if (x % 32 or y % 32) {
					values[x + y * w] += AVG_ELEVATION;
					values[x + y * w] /= 2;
				}
			}

		//  randomize the values

		uint32_t step_x  = std::min(16U, w), step_y  = std::min(16U, h);
		uint32_t max = AVG_ELEVATION, min = AVG_ELEVATION;
		double    ele_fac  = 0.5;

		bool end = false;

		while (!end) {
			for (uint32_t x = 0; x < w; x += step_x) {
				for (uint32_t y = 0; y < h; y += step_y) {
					//  Calculate coordinates of left and bottom left neighbours of
					//  the current node.

					uint32_t right_x = x + step_x;
					uint32_t lower_y = y + step_y;
					if (right_x >= w)
						right_x -= w;
					if (lower_y >= h)
						lower_y -= h;

					//  Get the current values of my neighbor nodes and of my node.

					uint32_t const x_0_y_0 = values[x       + w *       y];
					uint32_t const x_1_y_0 = values[right_x + w *       y];
					uint32_t const x_0_y_1 = values[x       + w * lower_y];
					uint32_t const x_1_y_1 = values[right_x + w * lower_y];

					//  calculate the in-between values

					uint32_t x_new       =
						x_0_y_0 / 2 + x_1_y_0 / 2 +
						static_cast<uint32_t>
							(ele_fac * rng.rand() - ele_fac * AVG_ELEVATION);

					uint32_t y_new       =
						x_0_y_0 / 2 + x_0_y_1 / 2 +
						static_cast<uint32_t>
							(ele_fac * rng.rand() - ele_fac * AVG_ELEVATION);

					uint32_t xy_new      =
						x_0_y_0 / 4 + x_1_y_1 / 4 + x_1_y_0 / 4 + x_0_y_1 / 4 +
						static_cast<uint32_t>
							(ele_fac * rng.rand() - ele_fac * AVG_ELEVATION);

					values[x + step_x / 2 + w * (y)]              =  x_new;
					values[x + step_x / 2 + w * (y + step_y / 2)] = xy_new;
					values[x              + w * (y + step_y / 2)] =  y_new;

					//  see if we have got a new min or max value

					if  (x_new > max)
						max =  x_new;
					if  (y_new > max)
						max =  y_new;
					if (xy_new > max)
						max = xy_new;

					if  (x_new < min)
						min =  x_new;
					if  (y_new < min)
						min =  y_new;
					if (xy_new < min)
						min = xy_new;
				}
			}

			//  preparations for the next iteration
			if (step_y == 2 && step_x == 2)
				end = true;
			step_x /= 2;
			step_y /= 2;
			if (step_x <= 1)
				step_x = 2;
			if (step_y <= 1)
				step_y = 2;
			ele_fac *= 0.5;
		}

		//  make a histogram of the heights

		uint32_t histo[1024];

		for (uint32_t x = 0; x < 1024; ++x)
			histo[x] = 0;

		for (uint32_t x = 0; x < w; ++x)
			for (uint32_t y = 0; y < h; ++y) {
				values[x + y * w] =
					((static_cast<double>(values[x + y * w] - min))
					 /
					 static_cast<double>(max - min))
					*
					MAX_ELEVATION;
				++histo[values[x + y * w] >> 22];
			}

		//  sort the histo out

		double minVals[1024];

		double currVal = 0.0;

		for (uint32_t x = 0; x < 1024; ++x) {
			minVals[x] = currVal;
			currVal +=
				static_cast<double>(histo[x]) / static_cast<double>(numFields);
		}

		//  Adjust the heights so that all height values are equal of density.
		//  This is done to have reliable water/land ratio later on.
		for (uint32_t x = 0; x < w; ++x)
			for (uint32_t y = 0; y < h; ++y)
				values[x + y * w] =
					minVals[values[x + y * w] >> 22]
					*
					static_cast<double>(MAX_ELEVATION);
		return values;
	} catch (...) {
		delete[] values;
		throw;
	}
}


/*
===============
Map::figure_out_terrain

Figures out terrain info for a field in a random map.

mapGenInfo:  Map generator information used to translate
             Random values to height information (world-
             specific info)
x, y:        first coordinate of the current triangle
x1, y1:      second coordinate of the current triangle
x2, y2:      third coordinate of the current triangle
random2:     Random array for generating different
             terrain types on land
random3:     Random array for generating different
             terrain types on land
random4:     Random array for wasteland generation
h1, h2, h3:  Map height information for the three triangle coords
mapInfo:     Information about the random map currently
             begin created (map specific info)
rng:         is the random number generator to be used.
             This will mostly be the current rng of the random map
             currently being created.
===============
*/
static Terrain_Index figure_out_terrain
	(MapGenInfo                &       mapGenInfo,
	 uint32_t                  * const random2,
	 uint32_t                  * const random3,
	 uint32_t                  * const random4,
	 uint32_t const x,  uint32_t const y,
	 uint32_t const x1, uint32_t const y1,
	 uint32_t const x2, uint32_t const y2,
	 uint32_t const h1, uint32_t const h2, uint32_t const h3,
	 UniqueRandomMapInfo const &       mapInfo,
	 RNG                       &       rng)
{
	uint32_t       numLandAreas      =
		mapGenInfo.getNumAreas(MapGenAreaInfo::atLand);
	uint32_t const numWasteLandAreas =
		mapGenInfo.getNumAreas(MapGenAreaInfo::atWasteland);

	bool isDesert  = false;
	bool isDesertOuter = false;
	uint32_t landAreaIndex = 0;
	MapGenAreaInfo::MapGenAreaType landType = MapGenAreaInfo::atLand;

	uint32_t rand2 = random2[x + mapInfo.w * y] / 3 +
		random2[x1 + mapInfo.w * y1] / 3 +
		random2[x2 + mapInfo.w * y2] / 3;
	uint32_t rand3 = random3[x + mapInfo.w * y] / 3 +
		random3[x1 + mapInfo.w * y1] / 3 +
		random3[x2 + mapInfo.w * y2] / 3;
	uint32_t rand4 = random4[x + mapInfo.w * y] / 3 +
		random4[x1 + mapInfo.w * y1] / 3 +
		random4[x2 + mapInfo.w * y2] / 3;

	//  At first we figure out if its wasteland or not.

	if        (numWasteLandAreas == 0) {
	} else if (numWasteLandAreas == 1) {
		if (rand4 < (AVG_ELEVATION * mapInfo.wastelandRatio)) {
			numLandAreas = numWasteLandAreas;
			isDesert = true;
			isDesertOuter =
				rand4 > (AVG_ELEVATION * mapInfo.wastelandRatio / 4) * 3;
			landAreaIndex = 0;
		}
	} else {
		if (rand4<(AVG_ELEVATION * mapInfo.wastelandRatio * 0.5)) {
			numLandAreas = numWasteLandAreas;
			isDesert = true;
			isDesertOuter =
				rand4 > (AVG_ELEVATION * mapInfo.wastelandRatio * 0.5 / 4) * 3;
			landAreaIndex = 0;
		}
		else if
			(rand4
			 >
			 (MAX_ELEVATION - AVG_ELEVATION * mapInfo.wastelandRatio * 0.5))
		{
			numLandAreas = numWasteLandAreas;
			isDesert = true;
			isDesertOuter =
				rand4
				<
				1 - AVG_ELEVATION * mapInfo.wastelandRatio * 0.5 / 4 * 3;
			landAreaIndex = 1;
		}
	}

	MapGenAreaInfo::MapGenAreaType    atp = MapGenAreaInfo::atLand;
	MapGenAreaInfo::MapGenTerrainType ttp = MapGenAreaInfo::ttLandLand;

	if (!isDesert) { //  see what kind of land it is

		if      (numLandAreas == 1)
			landAreaIndex = 0;
		else if (numLandAreas == 2) {
			uint32_t const weight1 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 0).getWeight();
			uint32_t const weight2 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 1).getWeight();
			uint32_t const sum     = mapGenInfo.getSumLandWeight();
			if
				(weight1 * (random2[x + mapInfo.w * y] / sum)
				 >=
				 weight2 * (AVG_ELEVATION / sum))
				landAreaIndex = 0;
			else
				landAreaIndex = 1;
		} else {
			uint32_t const weight1 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 0).getWeight();
			uint32_t const weight2 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 1).getWeight();
			uint32_t const weight3 =
				mapGenInfo.getArea(MapGenAreaInfo::atLand, 2).getWeight();
			uint32_t const sum     = mapGenInfo.getSumLandWeight();
			uint32_t const randomX = (rand2 + rand3) / 2;
			if
				(weight1 * (rand2 / sum) > weight2 * (rand3   / sum) &&
				 weight1 * (rand2 / sum) > weight3 * (randomX / sum))
				landAreaIndex = 0;
			else if
				(weight2 * (rand3 / sum) > weight1 * (rand2   / sum) &&
				 weight2 * (rand3 / sum) > weight3 * (randomX / sum))
				landAreaIndex = 1;
			else
				landAreaIndex = 2;
		}

		atp = MapGenAreaInfo::atLand;
		ttp = MapGenAreaInfo::ttLandLand;
	} else {
		atp = MapGenAreaInfo::atWasteland;
		ttp = MapGenAreaInfo::ttWastelandInner;
	}

	//  see whether it is water

	uint32_t const coast_h   = mapGenInfo.getLandCoastHeight();
	if (h1 <= coast_h && h2 <= coast_h && h3 <= coast_h) { //  water or coast...
		atp = landType;
		ttp = MapGenAreaInfo::ttLandCoast;

		uint32_t const ocean_h   = mapGenInfo.getWaterOceanHeight();
		uint32_t const shelf_h   = mapGenInfo.getWaterShelfHeight();
		uint32_t const shallow_h = mapGenInfo.getWaterShallowHeight();

		//  TODO: The heights can not be lower than water-Shallow --
		//  TODO: there will never be an ocean yet

		if        (h1 <= ocean_h   && h2 <= ocean_h   && h3 <= ocean_h)   {
			atp = MapGenAreaInfo::atWater;
			ttp = MapGenAreaInfo::ttWaterOcean;
		} else if (h1 <= shelf_h   && h2 <= shelf_h   && h3 <= shelf_h)   {
			atp = MapGenAreaInfo::atWater;
			ttp = MapGenAreaInfo::ttWaterShelf;
		} else if (h1 <= shallow_h && h2 <= shallow_h && h3 <= shallow_h) {
			atp = MapGenAreaInfo::atWater;
			ttp = MapGenAreaInfo::ttWaterShallow;
		}
	} else { //  it is not water
		uint32_t const upper_h = mapGenInfo.getLandUpperHeight   ();
		uint32_t const foot_h  = mapGenInfo.getMountainFootHeight();
		uint32_t const mount_h = mapGenInfo.getMountainHeight    ();
		uint32_t const snow_h  = mapGenInfo.getSnowHeight        ();
		if        (h1 >= snow_h  && h2 >= snow_h  && h3 >= snow_h)  {
			atp = MapGenAreaInfo::atMountains;
			ttp = MapGenAreaInfo::ttMountainsSnow;
		} else if (h1 >= mount_h && h2 >= mount_h && h3 >= mount_h) {
			atp = MapGenAreaInfo::atMountains;
			ttp = MapGenAreaInfo::ttMountainsMountain;
		} else if (h1 >= foot_h  && h2 >= foot_h  && h3 >= foot_h)  {
			atp = MapGenAreaInfo::atMountains;
			ttp = MapGenAreaInfo::ttMountainsFoot;
		} else if (h1 >= upper_h && h2 >= upper_h && h3 >= upper_h) {
			atp = MapGenAreaInfo::atLand;
			ttp = MapGenAreaInfo::ttLandUpper;
		}
	}

	//  Aftermath for land/Wasteland.

	uint32_t usedLandIndex = landAreaIndex;
	if (atp != MapGenAreaInfo::atLand && atp != MapGenAreaInfo::atWasteland)
		usedLandIndex = 0;
	else if (isDesert) {
		atp = MapGenAreaInfo::atWasteland;
		ttp =
			ttp == MapGenAreaInfo::ttLandCoast || isDesertOuter ?
			MapGenAreaInfo::ttWastelandOuter : MapGenAreaInfo::ttWastelandInner;
	}

	//  Figure out which terrain to use at this point in the map...
	return
		mapGenInfo.getArea(atp, usedLandIndex).getTerrain
			(ttp,
			 rng.rand()
			 %
			 mapGenInfo.getArea(atp, usedLandIndex).getNumTerrains(ttp));
}


void Map::create_random_map
	(UniqueRandomMapInfo const & mapInfo,
	 std::string const & worldname,
	 char        const * name,
	 char        const * author,
	 char        const * description)
{
	//  Init random number generator with map number

	//  We will use our own random number generator here so we do not influence
	//  someone else...
	RNG rng;

	rng.seed(mapInfo.mapNumber);

	//  basic initialization of map
	create_empty_map
		(mapInfo.w, mapInfo.h, worldname, name, author, description);

	//  get world generator info
	MapGenInfo & mapGenInfo = m_world->getMapGenInfo();

	//  Create a "raw" random elevation matrix.
	//  We will transform this into reasonable elevations and terrains later on.

	uint32_t * const elevations =
		generate_random_value_map(mapInfo.w, mapInfo.h, rng);

	//  for land stuff
	uint32_t * const random2    =
		generate_random_value_map(mapInfo.w, mapInfo.h, rng);
	uint32_t * const random3    =
		generate_random_value_map(mapInfo.w, mapInfo.h, rng);

	//  for desert/land
	uint32_t * const random4    =
		generate_random_value_map(mapInfo.w, mapInfo.h, rng);

	try {
		//  Now we have generated a lot of elevation!!
		//  Lets use them !!!

		for (uint32_t x = 0; x < mapInfo.w; ++x)
			for (uint32_t y = 0; y < mapInfo.h; ++y) {
				double adj_ele =
					static_cast<double>(elevations[x + mapInfo.w * y])
					/
					static_cast<double>(MAX_ELEVATION);
				m_fields[x + mapInfo.w * y].set_height
					(make_field_elevation(adj_ele, mapGenInfo, x, y, mapInfo));
			}

		//  Now lets set the terrain right according to the heights.

		for (uint32_t x = 0; x < mapInfo.w; ++x)
			for (uint32_t y = 0; y < mapInfo.h; ++y) {
				//  Calculate coordinates of left and bottom left neighbours of the
				//  current node.

				//  ... Treat "even" and "uneven" row numbers differently
				uint32_t x_dec = y % 2 == 0;

				uint32_t right_x       = x + 1;
				uint32_t lower_y       = y + 1;
				uint32_t lower_x       = x - x_dec;
				uint32_t lower_right_x = x - x_dec + 1;

				if (lower_x > mapInfo.w) lower_x += mapInfo.w;

				if (right_x >= mapInfo.w)             right_x -= mapInfo.w;
				if (lower_x >= mapInfo.w)             lower_x -= mapInfo.w;
				if (lower_right_x >= mapInfo.w) lower_right_x -= mapInfo.w;
				if (lower_y >= mapInfo.h)             lower_y -= mapInfo.h;

				//  get the heights of my neighbour nodes and of my current node

				uint8_t height_x0_y0 =
					m_fields[x + mapInfo.w * y].get_height();
				uint8_t height_x1_y0 =
					m_fields[right_x + mapInfo.w * y].get_height();
				uint8_t height_x0_y1 =
					m_fields[lower_x + mapInfo.w * lower_y].get_height();
				uint8_t height_x1_y1 =
					m_fields[lower_right_x + mapInfo.w * lower_y].get_height();

				//  "D" triangle

				Terrain_Index tix;
				tix = figure_out_terrain
					(mapGenInfo, random2, random3, random4,
					 x, y, lower_x, lower_y, lower_right_x, lower_y,
					 height_x0_y0, height_x0_y1, height_x1_y1, mapInfo, rng);

				m_fields[x  + mapInfo.w * y].set_terrain_d(tix);


				//  "R" triangle

				tix = figure_out_terrain
					(mapGenInfo, random2, random3, random4,
					 x, y, right_x, y, lower_right_x, lower_y,
					 height_x0_y0, height_x1_y0, height_x1_y1, mapInfo, rng);

				m_fields[x  + mapInfo.w * y].set_terrain_r(tix);

			}

	} catch (...) {
		delete[] elevations;
		delete[] random2;
		delete[] random3;
		delete[] random4;
		throw;
	}
	delete[] elevations;
	delete[] random2;
	delete[] random3;
	delete[] random4;
	//  Aftermaths...

	recalc_whole_map();
}

// TODO: MapGen: Bob generation, configurable in mapgenconf
// TODO: MapGen: How to handle "Bob layers" ???
// TODO: MapGen: Resource generation, configurable in mapgenconf
// TODO: MapGen: Check out sample map
// TODO: MapGen: Generate Start positions
// TODO: MapGen: How to handle height profile in make_blah...
// TODO: MapGen: Display something else than
// TODO:         "Preparing..." when generating map...
// TODO: MapGen: Allow up to 3 different water areas


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

	if (not m_overlay_manager)
		m_overlay_manager = new Overlay_Manager();
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

void Map::set_scenario_player_tribe
	(Player_Number const p, std::string const & tribename)
{
	assert(p);
	assert(p <= get_nrplayers());
	m_scenario_tribes.resize(get_nrplayers());
	m_scenario_tribes[p - 1] = tribename;
}

void Map::set_scenario_player_name
	(Player_Number const p, std::string const & playername)
{
	assert(p);
	assert(p <= get_nrplayers());
	m_scenario_names.resize(get_nrplayers());
	m_scenario_names[p - 1] = playername;
}

void Map::set_scenario_player_ai
	(Player_Number const p, std::string const & ainame)
{
	assert(p);
	assert(p <= get_nrplayers());
	m_scenario_ais.resize(get_nrplayers());
	m_scenario_ais[p - 1] = ainame;
}

/*
===============
Map::set_nrplayers

Change the number of players the map supports.
Could happen multiple times in the map editor.
===============
*/
void Map::set_nrplayers(Player_Number const nrplayers) {
	if (!nrplayers) {
		free(m_starting_pos);
		m_starting_pos = 0;
		m_nrplayers = 0;
		return;
	}

	m_starting_pos = static_cast<Coords *>
		(realloc(m_starting_pos, sizeof(Coords) * nrplayers));
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
void Map::set_starting_pos(Player_Number const plnum, Coords const c)
{
	assert(1 <= plnum);
	assert     (plnum <= get_nrplayers());

	m_starting_pos[plnum - 1] = c;
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

void Map::set_background(char const * const string)
{
	if (string)
		m_background = string;
	else
		m_background.clear();
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
	if (!m_world)
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
{return operator[](coord).get_immovable();}


/*
===============
Call the functor for every field that can be reached from coord without moving
outside the given radius.

Functor is of the form: functor(Map*, FCoords)
===============
*/
template<typename functorT>
void Map::find_reachable
	(Area<FCoords> const area, CheckStep const & checkstep, functorT & functor)
{
	std::vector<Field *> queue;
	boost::shared_ptr<Pathfields> pathfields = m_pathfieldmgr->allocate();

	queue.push_back(area.field);

	while (queue.size()) {
		Pathfield * curpf;
		FCoords cur;

		// Pop the last item from the queue
		cur.field = queue[queue.size() - 1];
		curpf = &pathfields->fields[cur.field - m_fields];
		get_coords(*cur.field, cur);
		queue.pop_back();

		// Handle this field
		functor(*this, cur);
		curpf->cycle = pathfields->cycle;

		// Get neighbours
		for (Direction dir = 1; dir <= 6; ++dir) {
			Pathfield *neighbpf;
			FCoords neighb;

			get_neighbour(cur, dir, &neighb);
			neighbpf = &pathfields->fields[neighb.field - m_fields];

			// Have we already visited this field?
			if (neighbpf->cycle == pathfields->cycle)
				continue;

			// Is the field still within the radius?
			if (calc_distance(area, neighb) > area.radius)
				continue;

			// Are we allowed to move onto this field?
			if
				(not
				 checkstep.allowed
				 	(*this,
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
	FindBobsCallback(std::vector<Bob *> * const list, FindBob const & functor)
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map &, const FCoords cur) {
		Bob *bob;

		for (bob = cur.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
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
Map::find_bobs

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
	 FindBob      const &       functor)
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
If list is non-zero, pointers to the relevant objects will be stored in
the list.

Returns the number of objects found.
===============
*/
uint32_t Map::find_reachable_bobs
	(Area<FCoords>        const area,
	 std::vector<Bob *> * const list,
	 CheckStep    const &       checkstep,
	 FindBob      const &       functor)
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
		(std::vector<ImmovableFound> * const list, FindImmovable const & functor)
		: m_list(list), m_functor(functor), m_found(0) {}

	void operator()(const Map &, const FCoords cur) {
		BaseImmovable * const imm = cur.field->get_immovable();

		if (!imm)
			return;

		if (m_functor.accept(imm)) {
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
Map::find_immovables

Find all immovables in the given area for which functor returns true
(the default functor always returns true).
Returns true if an immovable has been found.
If list is not 0, found immovables are stored in list.
===============
*/
uint32_t Map::find_immovables
	(Area<FCoords>                 const area,
	 std::vector<ImmovableFound> * const list,
	 FindImmovable const         &       functor)
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
uint32_t Map::find_reachable_immovables
	(Area<FCoords>                 const area,
	 std::vector<ImmovableFound> * const list,
	 CheckStep             const &       checkstep,
	 FindImmovable         const &       functor)
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
	 std::vector<BaseImmovable *> * list,
	 const CheckStep & checkstep,
	 const FindImmovable & functor)
{
	std::vector<ImmovableFound> duplist;
	FindImmovablesCallback cb(&duplist, FindImmovableAlwaysTrue());

	find_reachable(area, checkstep, cb);

	container_iterate_const(std::vector<ImmovableFound>, duplist, i) {
		BaseImmovable * const obj = i.current->object;
		if (std::find(list->begin(), list->end(), obj) == list->end())
			if (functor.accept(obj))
				list->push_back(obj);
	}

	return list->size();
}

/*
===============
FindNodesCallback

The actual logic behind find_fields and find_reachable_fields.
===============
*/
struct FindNodesCallback {
	FindNodesCallback
		(std::vector<Coords> * const list, FindNode const & functor)
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
	 FindNode const      &       functor)
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
	 CheckStep     const &       checkstep,
	 FindNode      const &       functor)
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
	uint8_t caps = CAPS_NONE;


	// 1a) Get all the neighbours to make life easier
	const FCoords  r =  r_n(f);
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
			goto end;
		}

	//  4) Flags
	//  We can build flags on anything that's walkable and buildable, with some
	//  restrictions
	if (caps & MOVECAPS_WALK) {
		//  4b) Flags must be at least 2 edges apart
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

	// 1b) Collect some information about the neighbours
	int32_t cnt_unpassable = 0;
	int32_t cnt_water = 0;
	int32_t cnt_acid = 0;
	int32_t cnt_mountain = 0;
	int32_t cnt_dry = 0;

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

	uint8_t caps = f.field->caps;

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

	// 3) We can only build something if there is a flag on the
	// bottom-right neighbour (or if we could build a flag on the
	// bottom-right neighbour)
	//
	// NOTE: This dependency on the bottom-right neighbour is the reason
	// why the caps calculation is split into two passes
	{
		const FCoords br = br_n(f);
		if
			(not (br.field->caps & BUILDCAPS_FLAG)
			 and
			 not find_immovables
			 	(Area<FCoords>(br, 0), 0, FindImmovableType(Map_Object::FLAG)))
			goto end;

	// === passability and flags allow us to build something beyond this
	// point ===

	// 4) Reduce building size based on nearby objects (incl. buildings)
	// and roads.
	//
	// Small object: allow medium-sized first-order neighbour, big
	// second-order neighbour
	//
	// Medium object: allow small-sized first-order neighbour, big
	// second-order neighbour
	//
	// Big object: allow no first-order neighbours, small second-order
	// neighbours
	//
	// Small buildings: same as small objects
	// Medium buildings: allow only medium second-order neighbours
	// Big buildings:  same as big objects
		{
			const FCoords bl = bl_n(f);
			uint8_t building = BUILDCAPS_BIG;
			std::vector<ImmovableFound> objectlist;

			find_immovables
				(Area<FCoords>(f, 2),
				 &objectlist,
				 FindImmovableSize(BaseImmovable::SMALL, BaseImmovable::BIG));
			for (uint32_t i = 0; i < objectlist.size(); ++i) {
				BaseImmovable * obj = objectlist[i].object;
				Coords objpos = objectlist[i].coords;
				int32_t dist = calc_distance(f, objpos);

				switch (obj->get_size()) {
				case BaseImmovable::SMALL:
					if (dist == 1) {
						if (building > BUILDCAPS_MEDIUM) {
							//  A flag to the bottom-right does not reduce building
							//  size (obvious) additionally, roads going top-right and
							//  left from a big building's flag should be allowed
							if
								((objpos != br and objpos != r and objpos != bl)
								 or
								 (not dynamic_cast<Flag const *>(obj)
								  &&
								  not dynamic_cast<Road const *>(obj)))
								building = BUILDCAPS_MEDIUM;
						}
					}
					break;

				case BaseImmovable::MEDIUM:
					if (dynamic_cast<Building const *>(obj)) {
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

			//  5) Build mines on mountains.
			if (cnt_mountain == 6) {
				//  4b) Check the mountain slope
				if
					(static_cast<int32_t>(br.field->get_height())
					 -
					 f.field->get_height()
					 <
					 4)
					caps |= BUILDCAPS_MINE;
				goto end;
			}

			//  6) Can not build anything if there are mountain or desert
			//  triangles next to the node
			if (cnt_mountain || cnt_dry)
				goto end;

			Field::Height const f_height = f.field->get_height();

			//  7) Reduce building size based on slope of direct neighbours:
			//    - slope >= 4: can't build anything here -> return
			//    - slope >= 3: maximum size is small
			{
				MapFringeRegion<Area<FCoords> > mr(*this, Area<FCoords>(f, 1));
				do {
					uint16_t const slope =
						abs(mr.location().field->get_height() - f_height);
					if (slope >= 4) goto end;
					if (slope >= 3) building = BUILDCAPS_SMALL;
				} while (mr.advance(*this));
			}
			if (abs(br.field->get_height() - f_height) >= 2) goto end;

			//  8) Reduce building size based on height diff. of second order
			//    neighbours  If height difference between this field and second
			//    order neighbour is >= 3, we can only build a small house here.
			//    Additionally, we can potentially build a harbour on this field
			//    if one of the second order neighbours is swimmable.
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

/*
===============
Map::is_neighbour

Find out if the two given fields are neighbours.
If they aren't, the function returns 0. If they are the same, return -1.
Otherwise, it returns the direction from start to end
===============
*/
int32_t Map::is_neighbour(const Coords start, const Coords end) const
{
	int32_t dx, dy;

	dy = end.y - start.y;
	dx = end.x - start.x;
	if (dx > static_cast<int32_t>(m_width >> 1))
		dx -= m_width;
	else if (dx < -static_cast<int32_t>(m_width >> 1))
		dx += m_width;

	// end and start are on the same row
	if (!dy) {
		switch (dx) {
		case -1: return Map_Object::WALK_W;
		case  0: return -1;
		case  1: return Map_Object::WALK_E;
		default:
			return 0;
		}
	}

	if (dy > static_cast<int32_t>(m_height >> 1))
		dy -= m_height;
	else if (dy < -static_cast<int32_t>(m_height >> 1))
		dy += m_height;

	// end is one row below start
	if (dy == 1) {
		if (start.y & 1) --dx;
		switch (dx) {
		case -1: return Map_Object::WALK_SW;
		case  0: return Map_Object::WALK_SE;
		default:
			return 0;
		}
	}

	// end is one row above start
	if (dy == -1) {
		if (start.y & 1) --dx;
		switch (dx) {
		case -1: return Map_Object::WALK_NW;
		case  0: return Map_Object::WALK_NE;
		default:
			return 0;
		}
	}

	return 0;
}


#define BASE_COST_PER_FIELD 1800
#define SLOPE_COST_DIVISOR  50
#define SLOPE_COST_STEPS    8

/*
===============
Map::calc_cost_estimate

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
Map::calc_cost

Calculate the hard cost of walking the given slope (positive means up,
negative means down).
The cost is in milliseconds it takes to walk.

The time is calculated as BASE_COST_PER_FIELD * f, where

f = 1.0 + d(Slope) - d(0)
d = (Slope + SLOPE_COST_STEPS)*(Slope + SLOPE_COST_STEPS - 1)
       / (2*SLOPE_COST_DIVISOR)

Note that the actual calculations multiply through by (2*SLOPE_COST_DIVISOR)
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
Map::calc_cost

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
Map::calc_bidi_cost

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
Map::calc_cost

Calculate the cost of walking the given path.
If either of the forward or backward pointers is set, it will be filled in
with the cost of walking in said direction.
===============
*/
void Map::calc_cost
	(Path const & path, int32_t * const forward, int32_t * const backward) const
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
	(Coords const f, Direction const dir, Coords * const o) const
{
	switch (dir) {
	case Map_Object::WALK_NW: get_tln(f, o); break;
	case Map_Object::WALK_NE: get_trn(f, o); break;
	case Map_Object::WALK_E:  get_rn (f, o); break;
	case Map_Object::WALK_SE: get_brn(f, o); break;
	case Map_Object::WALK_SW: get_bln(f, o); break;
	case Map_Object::WALK_W:  get_ln (f, o); break;
	default:
		assert(false);
	}
}

void Map::get_neighbour
	(FCoords const f, Direction const dir, FCoords * const o) const
{
	switch (dir) {
	case Map_Object::WALK_NW: get_tln(f, o); break;
	case Map_Object::WALK_NE: get_trn(f, o); break;
	case Map_Object::WALK_E:  get_rn (f, o); break;
	case Map_Object::WALK_SE: get_brn(f, o); break;
	case Map_Object::WALK_SW: get_bln(f, o); break;
	case Map_Object::WALK_W:  get_ln (f, o); break;
	default:
		assert(false);
	}
}

/**
 * Returns the correct initialized loader for the given mapfile
*/
Map_Loader * Map::get_correct_loader(char const * const filename) {
	Map_Loader * result = 0;

	if
		(!
		 strcasecmp
		 	(filename + (strlen(filename) - strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
		try {
			result = new WL_Map_Loader(g_fs->MakeSubFileSystem(filename), this);
		} catch (...) {
			//  If this fails, it is an illegal file (maybe old plain binary map
			//  format)
			//  TODO: catchall hides real errors! Replace with more specific code
		}
	else if
		(!
		 strcasecmp
		 	(filename + (strlen(filename) - strlen(S2MF_SUFFIX)), S2MF_SUFFIX)
		 |
		 !
		 strcasecmp
		 	(filename + (strlen(filename) - strlen(S2MF_SUFFIX2)), S2MF_SUFFIX2))
		//  It is a S2 Map file. Load it as such.
		result = new S2_Map_Loader(filename, *this);

	return result;
}

/**
 * Provides the flexible priority queue to maintain the open list.
 */
class StarQueue {
	std::vector<Pathfield *> m_data;

public:
	void flush() {m_data.clear();}

	// Return the best node and readjust the tree
	// Basic idea behind the algorithm:
	//  1. the top slot of the tree is empty
	//  2. if this slot has both children:
	//       fill this slot with one of its children or with slot[_size],
	//       whichever is best;
	//       if we filled with slot[_size], stop
	//       otherwise, repeat the algorithm with the child slot
	//     if it doesn't have any children (l >= _size)
	//       put slot[_size] in its place and stop
	//     if only the left child is there
	//       arrange left child and slot[_size] correctly and stop
	Pathfield * pop()
	{
		if (m_data.empty())
			return 0;

		Pathfield * const head = m_data[0];

		uint32_t nsize = m_data.size() - 1;
		uint32_t fix = 0;
		while (fix < nsize) {
			uint32_t l = fix * 2 + 1;
			uint32_t r = fix * 2 + 2;
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

			if
				(m_data[nsize]->cost() <= m_data[l]->cost() &&
				 m_data[nsize]->cost() <= m_data[r]->cost())
			{
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
	void push(Pathfield *t)
	{
		uint32_t slot = m_data.size();
		m_data.push_back(static_cast<Pathfield *>(0));

		while (slot > 0) {
			uint32_t parent = (slot - 1) / 2;

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
	void boost(Pathfield *t)
	{
		uint32_t slot = t->heap_index;

		assert(m_data[slot] == t);

		while (slot > 0) {
			uint32_t parent = (slot - 1) / 2;

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
	void debug(uint32_t node, const char *str)
	{
		uint32_t l = node * 2 + 1;
		uint32_t r = node * 2 + 2;
		if (m_data[node]->heap_index != static_cast<int32_t>(node)) {
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


/**
 * Finds a path from start to end for a Map_Object with the given movecaps.
 *
 * The path is stored in \p path, as a series of Map_Object::WalkingDir entries.
 *
 * \param persist tells the function how hard it should try to find a path:
 * If \p persist is \c 0, the function will never give up early. Otherwise, the
 * function gives up when it becomes clear that the path takes longer than
 * persist*bird's distance of flat terrain.
 * Note that if the terrain contains steep hills, the cost calculation will
 * cause the search to terminate earlier than you may think. If persist==1,
 * findpath() can only find a path if the terrain is completely flat.
 *
 * \param checkstep findpath() calls this checkstep functor-like to determine
 * whether moving from one field to another is legal.
 *
 * \param instart UNDOCUMENTED
 * \param inend UNDOCUMENTED
 * \param path UNDOCUMENTED
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
	 CheckStep const &       checkstep,
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
	StarQueue Open;
	Pathfield *curpf;

	curpf = &pathfields->fields[start.field - m_fields];
	curpf->cycle = pathfields->cycle;
	curpf->real_cost = 0;
	curpf->estim_cost = calc_cost_lowerbound(start, end);
	curpf->backlink = Map_Object::IDLE;

	Open.push(curpf);

	while ((curpf = Open.pop()))
	{
		cur.field = m_fields + (curpf - pathfields->fields.get());
		get_coords(*cur.field, cur);

		if (upper_cost_limit && curpf->real_cost > upper_cost_limit)
			break; // upper cost limit reached, give up
		if (cur == end)
			break; // found our target

		// avoid bias by using different orders when pathfinding
		static const int8_t order1[] = {
			Map_Object::WALK_NW, Map_Object::WALK_NE, Map_Object::WALK_E,
			Map_Object::WALK_SE, Map_Object::WALK_SW, Map_Object::WALK_W
		};
		static const int8_t order2[] = {
			Map_Object::WALK_NW, Map_Object::WALK_W, Map_Object::WALK_SW,
			Map_Object::WALK_SE, Map_Object::WALK_E, Map_Object::WALK_NE
		};
		const int8_t *direction;

		direction = (cur.x + cur.y) & 1 ? order1 : order2;

		// Check all the 6 neighbours
		for (uint32_t i = 6; i; i--, direction++) {
			Pathfield *neighbpf;
			FCoords neighb;
			int32_t cost;

			get_neighbour(cur, *direction, &neighb);
			neighbpf = &pathfields->fields[neighb.field - m_fields];

			// Is the field Closed already?
			if (neighbpf->cycle == pathfields->cycle && neighbpf->heap_index < 0)
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
			const int32_t stepcost = (flags & fpBidiCost) ?
				calc_bidi_cost(cur, *direction) : calc_cost(cur, *direction);

			cost = curpf->real_cost + stepcost;

			if (neighbpf->cycle != pathfields->cycle) {
				// add to open list
				neighbpf->cycle = pathfields->cycle;
				neighbpf->real_cost = cost;
				neighbpf->estim_cost = calc_cost_lowerbound(neighb, end);
				neighbpf->backlink = *direction;
				Open.push(neighbpf);
			} else if (neighbpf->cost() > cost + neighbpf->estim_cost) {
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
	int32_t const result = cur == end ? curpf->real_cost : -1;

	path.m_start = start;
	path.m_end = cur;

	path.m_path.clear();

	while (curpf->backlink != Map_Object::IDLE) {
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

	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;
	if (!(fc.field->get_caps() & MOVECAPS_WALK))
		return false;

	FCoords neighb;

	get_tln(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;

	get_trn(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;

	get_rn(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;

	get_brn(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;

	get_bln(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;

	get_ln(fc, &neighb);
	if (fc.field->get_caps() & MOVECAPS_SWIM)
		return true;

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
int32_t Map::change_terrain
	(TCoords<FCoords> const c, Terrain_Index const terrain)
{
	c.field->set_terrain(c.t, terrain);
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
		if (check[i]) check_neighbour_heights(n[i], area);
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

	if (influence > area.radius) influence = 0;
	else if (influence == 0) influence = MAX_RADIUS;
	else influence = MAX_RADIUS - influence;
	influence *= influence;

	return influence;
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

bool FindBobEnemySoldier::accept(Bob *imm) const
{
	if (upcast(Soldier, soldier, imm))
		if (soldier->isOnBattlefield() && &soldier->owner() != &player)
			return true;

	return false;
}



};

/*
===============
UniqueRandomMapInfo::mapIdNumberToChar

Converts a character out of a mapId-String into an integer value.
Valid characters are 'a'-'z' (or 'A'-'Z') and '2'-'9'. 'i' and 'o'
(or 'I' and 'O') are not valid.
The character is treated case-insensitive.

num:          Number to convert
Return value: The resulting number (0-31) or -1 if the character
              was no legal character.
===============
*/

int  Widelands::UniqueRandomMapInfo::mapIdCharToNumber(char ch)
{
	if ((ch >= 'A') && (ch < 'O'))
	{
		char res = ch - 'A';
		if (ch > 'I') res --;
		if (ch > 'O') res --;
		return res;
	}
	else if ((ch >= 'a') && (ch <= 'z'))
	{
		char res = ch - 'a';
		if (ch > 'i') res --;
		if (ch > 'o') res --;
		return res;
	}
	else if ((ch >= '2') && (ch <= '9'))
	{
		return 24 + ch - '2';
	}
	return -1;
}

/*
===============
UniqueRandomMapInfo::mapIdNumberToChar

Converts an integer number (0-31) to a characted usable in
a map id string.

num:          Number to convert
Return value: The converted value as a character
===============
*/
char Widelands::UniqueRandomMapInfo::mapIdNumberToChar(int32_t const num)
{
	if         (0 <= num && num < 24) {
		char result =  num + 'a';
		if (result >= 'i')
			++result;
		if (result >= 'o')
			++result;
		return result;
	} else if (24 <= num && num < 32)
		return (num - 24) + '2';
	else
		return '?';
}

/*
===============
UniqueRandomMapInfo::setFromIdString

Fills a UniqueRandomMapInfo structure from a given Map-id-string.

mapIdString:  Map-Id-String
mapInfo_out:  UniqueRandomMapInfo-Structure to be filled
Return value: true if the map-id-string was valid, false otherwise
===============
*/

bool Widelands::UniqueRandomMapInfo::setFromIdString
	(UniqueRandomMapInfo & mapInfo_out, std::string const & mapIdString)
{
	//  check string

	if (mapIdString.length() != MAP_ID_DIGITS + MAP_ID_DIGITS / 4 - 1)
		return false;

	for (uint32_t ix = 4; ix < MAP_ID_DIGITS; ix += 5)
		if (mapIdString[ix] != '-')
			return false;

	//  convert digits to values

	char nums[MAP_ID_DIGITS];

	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix) {
		int const num = mapIdCharToNumber(mapIdString[ix + (ix / 4)]);
		if (num < 0)
			return false;
		nums[ix] = num;
	}

	//  get xxor start value

	char xorr = nums[MAP_ID_DIGITS - 1];

	for (int32_t ix = MAP_ID_DIGITS - 1; ix >= 0; --ix) {
		nums[ix] = nums[ix] ^ xorr;
		xorr -= 7;
		xorr -= ix;
		if (xorr < 0)
			xorr &= 0x0000003f;
	}

	//  check if xxor was right
	if (nums[MAP_ID_DIGITS - 1])
		return false;

	//  check if version number is 1
	if (nums[MAP_ID_DIGITS - 2] != 1)
		return false;

	//  check if csm is right
	if (nums[MAP_ID_DIGITS - 3] != 0x15)
		return false;


	//  convert map number
	mapInfo_out.mapNumber =
		(nums[0])       |
		(nums[1] <<  5) |
		(nums[2] << 10) |
		(nums[3] << 15) |
		(nums[4] << 20) |
		(nums[5] << 25) |
		(nums[6] << 30);

	//  Convert map size
	mapInfo_out.w = nums[8] * 16 + 64;
	mapInfo_out.h = nums[9] * 16 + 64;

	//  Convert water percent
	mapInfo_out.waterRatio = static_cast<double>(nums[10]) / 20.0;
	//  Convert land percent
	mapInfo_out.landRatio  = static_cast<double>(nums[11]) / 20.0;
	//  Convert wasteland percent
	mapInfo_out.wastelandRatio  = static_cast<double>(nums[12]) / 20.0;
	//  Number of players
	mapInfo_out.numPlayers      = nums[13];
	//  Island mode
	mapInfo_out.islandMode      = (nums[14] == 1) ? true : false;

	return true;
}

/*
===============
UniqueRandomMapInfo::generateIdString

Generates an ID-String for map generation.
The ID-String is an encoded version of the
information in a UniqueMapInfo structure.
Thus, the ID_String will contain all info
necessary to re-create a given random map.

mapIdsString_out: Output buffer for the resulting
                  Map-ID-String
mapInfo:     Information about the random map currently
             begin created (map specific info)
===============
*/
void Widelands::UniqueRandomMapInfo::generateIdString
	(std::string & mapIdsString_out, UniqueRandomMapInfo const & mapInfo)
{
	//  Init
	assert(mapInfo.w <= 560);
	assert(mapInfo.h <= 560);
	assert(mapInfo.waterRatio >= 0.0);
	assert(mapInfo.waterRatio <= 1.0);
	assert(mapInfo.landRatio >= 0.0);
	assert(mapInfo.landRatio <= 1.0);
	assert(mapInfo.wastelandRatio >= 0.0);
	assert(mapInfo.wastelandRatio <= 1.0);

	mapIdsString_out = "";
	char nums[MAP_ID_DIGITS];
	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix)
		nums[ix] = 0;

	//  Convert map random number
	nums [0] =  mapInfo.mapNumber        & 31;
	nums [1] = (mapInfo.mapNumber >>  5) & 31;
	nums [2] = (mapInfo.mapNumber >> 10) & 31;
	nums [3] = (mapInfo.mapNumber >> 15) & 31;

	nums [4] = (mapInfo.mapNumber >> 20) & 31;
	nums [5] = (mapInfo.mapNumber >> 25) & 31;
	nums [6] = (mapInfo.mapNumber >> 30) &  3;

	//  Convert width
	nums [8] = (mapInfo.w - 64) / 16;
	//  Convert height
	nums [9] = (mapInfo.h - 64) / 16;
	//  Convert water percent
	nums[10] = (mapInfo.waterRatio + 0.025) * 20.0;
	//  Convert land  percent
	nums[11] = (mapInfo.landRatio + 0.025)  * 20.0;

	//  Convert wasteland percent
	nums[12] = (mapInfo.wastelandRatio + 0.025)  * 20.0;
	//  Set number of islands
	nums[13] = mapInfo.numPlayers;
	//  Island mode
	nums[14] = mapInfo.islandMode ? 1 : 0;

	//  Set id csm
	nums[MAP_ID_DIGITS - 3] = 0x15;
	//  Set id version number
	nums[MAP_ID_DIGITS - 2] = 0x01;
	//  Last number intentionally left blank
	nums[MAP_ID_DIGITS - 1] = 0x00;


	//  Nox xor everything
	//  This lets it look better
	//  Every change in a digit will result in a complete id change

	char xorr = 0x0a;
	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix)
		xorr = xorr ^ nums[ix];

	for (int32_t ix = MAP_ID_DIGITS - 1; ix >= 0; --ix) {
		nums[ix] = nums[ix] ^ xorr;
		xorr -= 7;
		xorr -= ix;
		if (xorr < 0)
			xorr &= 0x0000003f;
	}

	//  translate it to ASCII
	for (uint32_t ix = 0; ix < MAP_ID_DIGITS; ++ix) {
		mapIdsString_out += mapIdNumberToChar(nums[ix]);
		if (ix % 4 == 3 && ix != MAP_ID_DIGITS - 1)
			mapIdsString_out += "-";
	}
}

