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
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "build_info.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "editor/tools/editor_increase_resources_tool.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapfringeregion.h"
#include "logic/objective.h"
#include "logic/pathfield.h"
#include "logic/player.h"
#include "map_io/s2map.h"
#include "map_io/widelands_map_loader.h"
#include "notifications/notifications.h"

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
nrplayers_      (0),
scenario_types_ (NO_SCENARIO),
width_          (0),
height_         (0),
pathfieldmgr_   (new PathfieldManager)
{
}


Map::~Map()
{
	cleanup();
}

void Map::recalc_border(const FCoords fc) {
	if (const PlayerNumber owner = fc.field->get_owned_by()) {
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
 - robust MapObject has been added or removed

This performs the steps outlined in the comment above Map::recalc_brightness()
and recalcs the interactive player's overlay.
===============
*/
void Map::recalc_for_field_area(const World& world, const Area<FCoords> area) {
	assert(0 <= area.x);
	assert(area.x < width_);
	assert(0 <= area.y);
	assert(area.y < height_);
	assert(fields_.get() <= area.field);
	assert            (area.field < fields_.get() + max_index());

	{ //  First pass.
		MapRegion<Area<FCoords> > mr(*this, area);
		do {
			recalc_brightness(mr.location());
			recalc_border(mr.location());
			recalc_nodecaps_pass1(world, mr.location());
		} while (mr.advance(*this));
	}

	{ //  Second pass.
		MapRegion<Area<FCoords> > mr(*this, area);
		do recalc_nodecaps_pass2(world, mr.location()); while (mr.advance(*this));
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
void Map::recalc_whole_map(const World& world)
{
	//  Post process the map in the necessary two passes to calculate
	//  brightness and building caps
	FCoords f;

	for (int16_t y = 0; y < height_; ++y)
		for (int16_t x = 0; x < width_; ++x) {
			f = get_fcoords(Coords(x, y));
			uint32_t radius = 0;
			check_neighbour_heights(f, radius);
			recalc_brightness     (f);
			recalc_border         (f);
			recalc_nodecaps_pass1  (world, f);
		}

	for (int16_t y = 0; y < height_; ++y)
		for (int16_t x = 0; x < width_; ++x) {
			f = get_fcoords(Coords(x, y));
			recalc_nodecaps_pass2(world, f);
		}
}

void Map::recalc_default_resources(const World& world) {
	for (int16_t y = 0; y < height_; ++y)
		for (int16_t x = 0; x < width_; ++x) {
			FCoords f, f1;
			f = get_fcoords(Coords(x, y));
			//  only on unset nodes
			if (f.field->get_resources() != Widelands::kNoResource || f.field->get_resources_amount())
				continue;
			std::map<int32_t, int32_t> m;
			int32_t amount = 0;

			//  this node
			{
				const TerrainDescription & terr = world.terrain_descr(f.field->terrain_r());
				++m[terr.get_default_resource()];
				amount += terr.get_default_resource_amount();
			}
			{
				const TerrainDescription & terd = world.terrain_descr(f.field->terrain_d());
				++m[terd.get_default_resource()];
				amount += terd.get_default_resource_amount();
			}

			//  If one of the neighbours is unwalkable, count its resource
			//  stronger
			//  top left neigbour
			get_neighbour(f, WALK_NW, &f1);
			{
				const TerrainDescription& terr = world.terrain_descr(f1.field->terrain_r());
				const int8_t resr = terr.get_default_resource();
				const int default_amount = terr.get_default_resource_amount();
				if ((terr.get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0)
					m[resr] += 3;
				else
					++m[resr];
				amount += default_amount;
			}
			{
				const TerrainDescription& terd = world.terrain_descr(f1.field->terrain_d());
				const int8_t resd = terd.get_default_resource();
				const int default_amount = terd.get_default_resource_amount();
				if ((terd.get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0)
					m[resd] += 3;
				else
					++m[resd];
				amount += default_amount;
			}

			//  top right neigbour
			get_neighbour(f, WALK_NE, &f1);
			{
				const TerrainDescription& terd = world.terrain_descr(f1.field->terrain_d());
				const int8_t resd = terd.get_default_resource();
				const int default_amount = terd.get_default_resource_amount();
				if ((terd.get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0)
					m[resd] += 3;
				else
					++m[resd];
				amount += default_amount;
			}

			//  left neighbour
			get_neighbour(f, WALK_W, &f1);
			{
				const TerrainDescription& terr = world.terrain_descr(f1.field->terrain_r());
				const int8_t resr = terr.get_default_resource();
				const int default_amount = terr.get_default_resource_amount();
				if ((terr.get_is() & TerrainDescription::Is::kUnwalkable) && default_amount > 0)
					m[resr] += 3;
				else
					++m[resr];
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

			if (res == -1 || !amount) {
				clear_resources(f);
			} else {
				initialize_resources(f, res, amount);
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
	nrplayers_ = 0;
	width_ = height_ = 0;

	fields_.reset();

	starting_pos_.clear();
	scenario_tribes_.clear();
	scenario_names_.clear();
	scenario_ais_.clear();
	scenario_closeables_.clear();

	tags_.clear();
	hint_ = std::string();
	background_ = std::string();

	objectives_.clear();

	port_spaces_.clear();

	// TODO(meitis): should be done here ... but WidelandsMapLoader::preload_map calls
	// this cleanup AFTER assigning filesystem_ in WidelandsMapLoader::WidelandsMapLoader
	// ... so we can't do it here :/
	// filesystem_.reset(nullptr);
}

/*
===========
creates an empty map without name with
the given data
===========
*/
void Map::create_empty_map
	(const World& world, uint32_t const w, uint32_t const h,
	 const Widelands::DescriptionIndex default_terrain,
	 const std::string& name,
	 const std::string& author,
	 const std::string& description)
{
	set_size(w, h);
	set_name       (name);
	set_author     (author);
	set_description(description);
	set_nrplayers(1);
	// Set first tribe found as the "basic" tribe
	// <undefined> (as set before) is useless and will lead to a
	// crash -> Widelands will search for tribe "<undefined>"
	set_scenario_player_tribe(1, Tribes::get_all_tribenames()[0]);
	set_scenario_player_name(1, (boost::format(_("Player %u")) % 1).str());
	set_scenario_player_ai(1, "");
	set_scenario_player_closeable(1, false);

	{
		Field::Terrains default_terrains;
		default_terrains.d = default_terrain;
		default_terrains.r = default_terrain;
		for (int16_t y = 0; y < height_; ++y) {
			for (int16_t x = 0; x < width_; ++x) {
				auto f = get_fcoords(Coords(x, y));
				f.field->set_height(10);
				f.field->set_terrains(default_terrains);
				clear_resources(f);
			}
		}
	}
	recalc_whole_map(world);

	filesystem_.reset(nullptr);
}


void Map::set_origin(Coords const new_origin) {
	assert(0 <= new_origin.x);
	assert     (new_origin.x < width_);
	assert(0 <= new_origin.y);
	assert     (new_origin.y < height_);

	for (uint8_t i = get_nrplayers(); i;) {
		starting_pos_[--i].reorigin(new_origin, extent());
	}

	std::unique_ptr<Field[]> new_field_order(new Field[width_ * height_]);
	memset(new_field_order.get(), 0, sizeof(Field) * width_ * height_);

	// Rearrange The fields
	// NOTE because of the triangle design, we have to take special care about cases
	// NOTE where y is changed by an odd number
	bool yisodd = (new_origin.y % 2) != 0;
	for (FCoords c(Coords(0, 0)); c.y < height_; ++c.y) {
		bool cyisodd = (c.y % 2) != 0;
		for (c.x = 0; c.x < width_; ++c.x) {
			Coords temp;
			if (yisodd && cyisodd)
				temp = Coords(c.x + new_origin.x + 1, c.y + new_origin.y);
			else
				temp = Coords(c.x + new_origin.x, c.y + new_origin.y);
			normalize_coords(temp);
			new_field_order[get_index(c, width_)] = operator[](temp);
		}
	}
	// Now that we restructured the fields, we just overwrite the old order
	fields_.reset(new_field_order.release());

	//  Inform immovables and bobs about their new coordinates.
	for (FCoords c(Coords(0, 0), fields_.get()); c.y < height_; ++c.y)
		for (c.x = 0; c.x < width_; ++c.x, ++c.field) {
			assert(c.field == &operator[] (c));
			if (upcast(Immovable, immovable, c.field->get_immovable()))
				immovable->position_ = c;
			for
				(Bob * bob = c.field->get_first_bob();
				 bob;
				 bob = bob->get_next_bob())
			{
				bob->position_.x     = c.x;
				bob->position_.y     = c.y;
				bob->position_.field = c.field;
			}
		}

	// Take care about port spaces
	PortSpacesSet new_port_spaces;
	for (PortSpacesSet::iterator it = port_spaces_.begin(); it != port_spaces_.end(); ++it) {
		Coords temp;
		if (yisodd && ((it->y % 2) == 0))
			temp = Coords(it->x - new_origin.x - 1, it->y - new_origin.y);
		else
			temp = Coords(it->x - new_origin.x, it->y - new_origin.y);
		normalize_coords(temp);
		log("(%i,%i) -> (%i,%i)\n", it->x, it->y, temp.x, temp.y);
		new_port_spaces.insert(temp);
	}
	port_spaces_ = new_port_spaces;
}




/*
===============
Set the size of the map. This should only happen once during initial load.
===============
*/
void Map::set_size(const uint32_t w, const uint32_t h)
{
	assert(!fields_);

	width_  = w;
	height_ = h;

	fields_.reset(new Field[w * h]);
	memset(fields_.get(), 0, sizeof(Field) * w * h);

	pathfieldmgr_->set_size(w * h);
}

/*
 * The scenario get/set functions
 */
const std::string & Map::get_scenario_player_tribe(const PlayerNumber p) const
{
	assert(scenario_tribes_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_tribes_[p - 1];
}

const std::string & Map::get_scenario_player_name(const PlayerNumber p) const
{
	assert(scenario_names_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_names_[p - 1];
}

const std::string & Map::get_scenario_player_ai(const PlayerNumber p) const
{
	assert(scenario_ais_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_ais_[p - 1];
}

bool Map::get_scenario_player_closeable(const PlayerNumber p) const
{
	assert(scenario_closeables_.size() == get_nrplayers());
	assert(p);
	assert(p <= get_nrplayers());
	return scenario_closeables_[p - 1];
}

void Map::swap_filesystem(std::unique_ptr<FileSystem>& fs)
{
	filesystem_.swap(fs);
}

FileSystem* Map::filesystem() const {
	return filesystem_.get();
}

void Map::set_scenario_player_tribe(PlayerNumber const p, const std::string & tribename)
{
	assert(p);
	assert(p <= get_nrplayers());
	scenario_tribes_.resize(get_nrplayers());
	scenario_tribes_[p - 1] = tribename;
}

void Map::set_scenario_player_name(PlayerNumber const p, const std::string & playername)
{
	assert(p);
	assert(p <= get_nrplayers());
	scenario_names_.resize(get_nrplayers());
	scenario_names_[p - 1] = playername;
}

void Map::set_scenario_player_ai(PlayerNumber const p, const std::string & ainame)
{
	assert(p);
	assert(p <= get_nrplayers());
	scenario_ais_.resize(get_nrplayers());
	scenario_ais_[p - 1] = ainame;
}

void Map::set_scenario_player_closeable(PlayerNumber const p, bool closeable)
{
	assert(p);
	assert(p <= get_nrplayers());
	scenario_closeables_.resize(get_nrplayers());
	scenario_closeables_[p - 1] = closeable;
}

/*
===============
Change the number of players the map supports.
Could happen multiple times in the map editor.
===============
*/
void Map::set_nrplayers(PlayerNumber const nrplayers) {
	if (!nrplayers) {
		nrplayers_ = 0;
		return;
	}

	starting_pos_.resize(nrplayers, Coords(-1, -1));
	scenario_tribes_.resize(nrplayers);
	scenario_ais_.resize(nrplayers);
	scenario_closeables_.resize(nrplayers);
	scenario_names_.resize(nrplayers);
	scenario_tribes_.resize(nrplayers);

	nrplayers_ = nrplayers; // in case the number players got less
}

/*
===============
Set the starting coordinates of a player
===============
*/
void Map::set_starting_pos(PlayerNumber const plnum, Coords const c)
{
	assert(1 <= plnum && plnum <= get_nrplayers());
	starting_pos_[plnum - 1] = c;
}


void Map::set_filename(const std::string& filename)
{
	filename_ = filename;
}

void Map::set_author(const std::string& author)
{
	author_ = author;
}

void Map::set_name(const std::string& name)
{
	name_ = name;
}

void Map::set_description(const std::string& description)
{
	description_ = description;
}

void Map::set_hint(const std::string& hint)
{
	hint_ = hint;
}

void Map::set_background(const std::string& image_path)
{
	if (image_path.empty())
		background_.clear();
	else
		background_ = image_path;
}

void Map::add_tag(const std::string& tag) {
	tags_.insert(tag);
}

void Map::delete_tag(const std::string& tag) {
	if (has_tag(tag)) {
		tags_.erase(tags_.find(tag));
	}
}

NodeCaps Map::get_max_nodecaps(const World& world, const FCoords& fc) {
	NodeCaps caps = calc_nodecaps_pass1(world, fc, false);
	caps = calc_nodecaps_pass2(world, fc, false, caps);
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
	boost::shared_ptr<Pathfields> pathfields = pathfieldmgr_->allocate();

	queue.push_back(area);

	while (queue.size()) {
		// Pop the last ware from the queue
		FCoords const cur = get_fcoords(*queue.rbegin());
		queue.pop_back();
		Pathfield & curpf = pathfields->fields[cur.field - fields_.get()];

		//  handle this node
		functor(*this, cur);
		curpf.cycle = pathfields->cycle;

		// Get neighbours
		for (Direction dir = 1; dir <= 6; ++dir) {
			FCoords neighb;

			get_neighbour(cur, dir, &neighb);

			if  //  node not already handled?
				(pathfields->fields[neighb.field - fields_.get()].cycle
				 !=
				 pathfields->cycle
				 &&
				 //  node within the radius?
				 calc_distance(area, neighb) <= area.radius
				 &&
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
	FindImmovablesCallback cb(&duplist, find_immovable_always_true());

	find_reachable(area, checkstep, cb);

	for (ImmovableFound& imm_found : duplist) {
		BaseImmovable & obj = *imm_found.object;
		if (std::find(list.begin(), list.end(), &obj) == list.end()) {
			if (functor.accept(obj)) {
				list.push_back(&obj);
			}
		}
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
- insertion of a "robust" MapObject
- removal of a "robust" MapObject

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
void Map::recalc_nodecaps_pass1(const World& world, FCoords const f) {
	f.field->caps = calc_nodecaps_pass1(world, f, true);
}

NodeCaps Map::calc_nodecaps_pass1(const World& world, FCoords const f, bool consider_mobs) {
	uint8_t caps = CAPS_NONE;

	// 1a) Get all the neighbours to make life easier
	const FCoords tr = tr_n(f);
	const FCoords tl = tl_n(f);
	const FCoords  l =  l_n(f);

	const TerrainDescription::Is tr_d_terrain_is =
		world.terrain_descr(tr.field->terrain_d()).get_is();
	const TerrainDescription::Is tl_r_terrain_is =
		world.terrain_descr(tl.field->terrain_r()).get_is();
	const TerrainDescription::Is tl_d_terrain_is =
		world.terrain_descr(tl.field->terrain_d()).get_is();
	const TerrainDescription::Is  l_r_terrain_is =
		world.terrain_descr (l.field->terrain_r()).get_is();
	const TerrainDescription::Is  f_d_terrain_is =
		world.terrain_descr (f.field->terrain_d()).get_is();
	const TerrainDescription::Is  f_r_terrain_is =
		world.terrain_descr (f.field->terrain_r()).get_is();

	//  1b) Collect some information about the neighbours
	uint8_t cnt_unwalkable = 0;
	uint8_t cnt_water = 0;
	uint8_t cnt_unreachable = 0;

	if  (tr_d_terrain_is & TerrainDescription::Is::kUnwalkable) ++cnt_unwalkable;
	if  (tl_r_terrain_is & TerrainDescription::Is::kUnwalkable) ++cnt_unwalkable;
	if  (tl_d_terrain_is & TerrainDescription::Is::kUnwalkable) ++cnt_unwalkable;
	if   (l_r_terrain_is & TerrainDescription::Is::kUnwalkable) ++cnt_unwalkable;
	if   (f_d_terrain_is & TerrainDescription::Is::kUnwalkable) ++cnt_unwalkable;
	if   (f_r_terrain_is & TerrainDescription::Is::kUnwalkable) ++cnt_unwalkable;

	if  (tr_d_terrain_is & TerrainDescription::Is::kWater)      ++cnt_water;
	if  (tl_r_terrain_is & TerrainDescription::Is::kWater)      ++cnt_water;
	if  (tl_d_terrain_is & TerrainDescription::Is::kWater)      ++cnt_water;
	if   (l_r_terrain_is & TerrainDescription::Is::kWater)      ++cnt_water;
	if   (f_d_terrain_is & TerrainDescription::Is::kWater)      ++cnt_water;
	if   (f_r_terrain_is & TerrainDescription::Is::kWater)      ++cnt_water;

	if  (tr_d_terrain_is & TerrainDescription::Is::kUnreachable)       ++cnt_unreachable;
	if  (tl_r_terrain_is & TerrainDescription::Is::kUnreachable)       ++cnt_unreachable;
	if  (tl_d_terrain_is & TerrainDescription::Is::kUnreachable)       ++cnt_unreachable;
	if   (l_r_terrain_is & TerrainDescription::Is::kUnreachable)       ++cnt_unreachable;
	if   (f_d_terrain_is & TerrainDescription::Is::kUnreachable)       ++cnt_unreachable;
	if   (f_r_terrain_is & TerrainDescription::Is::kUnreachable)       ++cnt_unreachable;


	//  2) Passability

	//  2a) If any of the neigbouring triangles is walkable this node is
	//  walkable.
	if (cnt_unwalkable < 6)
		caps |= MOVECAPS_WALK;

	//  2b) If all neighbouring triangles are water, the node is swimable.
	if (cnt_water == 6)
		caps |= MOVECAPS_SWIM;


	// 2c) [OVERRIDE] If any of the neighbouring triangles is really "bad" (such
	// as lava), we can neither walk nor swim to this node.
	if (cnt_unreachable)
		caps &= ~(MOVECAPS_WALK | MOVECAPS_SWIM);

	//  === everything below is used to check buildability ===

	// if we are interested in the maximum theoretically available NodeCaps, this is not run
	if (consider_mobs) {
		//  3) General buildability check: if a "robust" MapObject is on this node
		//  we cannot build anything on it. Exception: we can build flags on roads.
		if (BaseImmovable * const imm = get_immovable(f))
			if
				(!dynamic_cast<Road const *>(imm)
				&&
				imm->get_size() >= BaseImmovable::SMALL)
			{
				// 3b) [OVERRIDE] check for "unwalkable" MapObjects
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
		if (consider_mobs &&
			 find_immovables(
				Area<FCoords>(f, 1),
				nullptr,
				FindImmovableType(MapObjectType::FLAG)))
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
void Map::recalc_nodecaps_pass2(const World& world, const FCoords & f) {
	f.field->caps = calc_nodecaps_pass2(world, f, true);
}

NodeCaps Map::calc_nodecaps_pass2
	(const World& world, FCoords const f, bool consider_mobs, NodeCaps initcaps)
{
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
			(!br.field->get_immovable() || br.field->get_immovable()->descr().type() != MapObjectType::FLAG))
			return static_cast<NodeCaps>(caps);
	} else {
		if (!(calc_nodecaps_pass1(world, br, false) & BUILDCAPS_FLAG))
			return static_cast<NodeCaps>(caps);
	}

	bool mine;
	uint8_t buildsize = calc_buildsize(world, f, true, &mine, consider_mobs, initcaps);
	if (buildsize < BaseImmovable::SMALL)
		return static_cast<NodeCaps>(caps);
	assert(buildsize >= BaseImmovable::SMALL && buildsize <= BaseImmovable::BIG);

	if (buildsize == BaseImmovable::BIG) {
		if
			(calc_buildsize(world, l_n(f),  false, nullptr, consider_mobs, initcaps) < BaseImmovable::BIG ||
			 calc_buildsize(world, tl_n(f), false, nullptr, consider_mobs, initcaps) < BaseImmovable::BIG ||
			 calc_buildsize(world, tr_n(f), false, nullptr, consider_mobs, initcaps) < BaseImmovable::BIG)
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
	(const World& world, const FCoords & f, bool avoidnature, bool * ismine,
	 bool consider_mobs, NodeCaps initcaps)
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

	const TerrainDescription::Is terrains[6] = {
		world.terrain_descr(tr.field->terrain_d()).get_is(),
		world.terrain_descr(tl.field->terrain_r()).get_is(),
		world.terrain_descr(tl.field->terrain_d()).get_is(),
		world.terrain_descr (l.field->terrain_r()).get_is(),
		world.terrain_descr (f.field->terrain_d()).get_is(),
		world.terrain_descr (f.field->terrain_r()).get_is()
	};

	uint32_t cnt_mineable = 0;
	uint32_t cnt_walkable = 0;
	for (uint32_t i = 0; i < 6; ++i) {
		if (terrains[i] & TerrainDescription::Is::kWater ||
			 terrains[i] & TerrainDescription::Is::kUnwalkable)
			return BaseImmovable::NONE;
		if (terrains[i] & TerrainDescription::Is::kMineable) ++cnt_mineable;
		if (terrains[i] & TerrainDescription::Is::kWalkable) ++cnt_walkable;
	}

	if (cnt_mineable == 6) {
		if (ismine)
			*ismine = true;
		return BaseImmovable::SMALL;
	}
	if (cnt_mineable || cnt_walkable)
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
			if (avoidnature && obj->descr().type() == MapObjectType::IMMOVABLE)
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
	const Widelands::PlayerNumber owner = start.field->get_owned_by();
	bool is_good_water;
	FCoords f = start;
	std::vector<Coords> portdock;
	for (uint32_t i = 0; i < 16; ++i) {
		is_good_water = (f.field->get_caps() & (MOVECAPS_SWIM|MOVECAPS_WALK)) == MOVECAPS_SWIM;

		// Any immovable here? (especially another portdock)
		if (is_good_water && f.field->get_immovable()) {
			is_good_water = false;
		}

		// If starting point is owned we make sure this field has the same owner
		if (is_good_water && owner > 0 && f.field->get_owned_by() != owner) {
			is_good_water = false;
		}

		// ... and is not on a border
		if (is_good_water && owner > 0 && f.field->is_border()) {
			is_good_water = false;
		}

		if (is_good_water) {
			portdock.push_back(f);
			// Occupy 2 fields maximum in order not to block space for other ports that
			// might be built in the vicinity.
			if (portdock.size() == 2) {
				return portdock;
			}
		}

		if (i < 15)
			f = get_neighbour(f, cycledirs[i]);
	}

	return portdock;
}

/// \returns true, if Coordinates are in port space list
bool Map::is_port_space(const Coords& c) const {
	return port_spaces_.count(c);
}

/// Set or unset a space as port space
void Map::set_port_space(Coords c, bool allowed) {
	if (allowed) {
		port_spaces_.insert(c);
	} else {
		port_spaces_.erase(c);
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
	if (dy > static_cast<int32_t>(height_ >> 1)) //  wrap-around!
		dy -= height_;
	else if (dy < -static_cast<int32_t>(height_ >> 1))
		dy += height_;

	dist = abs(dy);

	if (static_cast<int16_t>(dist) >= width_)
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
		lx += width_;
	else if (rx >= static_cast<int32_t>(width_))
		rx -= width_;

	// Normal, non-wrapping case
	if (lx <= rx)
	{
		if (b.x < lx) {
			int32_t dx1 = lx - b.x;
			int32_t dx2 = b.x - (rx - width_);
			dist += std::min(dx1, dx2);
		}
		else if (b.x > rx)
		{
			int32_t dx1 = b.x - rx;
			int32_t dx2 = (lx + width_) - b.x;
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

	const Path::StepVector::size_type nr_steps = path.get_nsteps();
	for (Path::StepVector::size_type i = 0; i < nr_steps; ++i) {
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
		NEVER_HERE();
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
		NEVER_HERE();
	}
}

std::unique_ptr<MapLoader> Map::get_correct_loader(const std::string& filename) {
	std::unique_ptr<MapLoader> result;

	std::string lower_filename = filename;
	boost::algorithm::to_lower(lower_filename);

	if (boost::algorithm::ends_with(lower_filename, WLMF_SUFFIX)) {
		try {
			result.reset(new WidelandsMapLoader(g_fs->make_sub_file_system(filename), this));
		} catch (...) {
			//  If this fails, it is an illegal file.
			//  TODO(unknown): catchall hides real errors! Replace with more specific code
		}
	} else if (boost::algorithm::ends_with(lower_filename, S2MF_SUFFIX) ||
	           boost::algorithm::ends_with(lower_filename, S2MF_SUFFIX2)) {
		result.reset(new S2MapLoader(filename, *this));
	}
	return result;
}

/**
 * Finds a path from start to end for a MapObject with the given movecaps.
 *
 * The path is stored in \p path, as a series of MapObject::WalkingDir entries.
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
 */
// TODO(unknown): Document parameters instart, inend, path, flags
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

	if (!checkstep.reachable_dest(*this, end))
		return -1;

	if (!persist)
		upper_cost_limit = 0;
	else
		// assume flat terrain
		upper_cost_limit = persist * calc_cost_estimate(start, end);

	// Actual pathfinding
	boost::shared_ptr<Pathfields> pathfields = pathfieldmgr_->allocate();
	Pathfield::Queue Open;
	Pathfield * curpf = &pathfields->fields[start.field - fields_.get()];
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

		cur.field = fields_.get() + (curpf - pathfields->fields.get());
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
			Pathfield & neighbpf = pathfields->fields[neighb.field - fields_.get()];

			// Is the field Closed already?
			if
				(neighbpf.cycle == pathfields->cycle &&
				 !neighbpf.heap_cookie.is_active())
				continue;

			// Check passability
			if
				(!
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
		curpf = &pathfields->fields[cur.field - fields_.get()];
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

int32_t Map::change_terrain
	(const World& world, TCoords<FCoords> const c, DescriptionIndex const terrain)
{
	c.field->set_terrain(c.t, terrain);

	// remove invalid resources if necessary
	// check vertex to which the triangle belongs
	if (!is_resource_valid(world, c, c.field->get_resources())) {
		clear_resources(c);
	}

	// always check south-east vertex
	Widelands::FCoords f_se(c, c.field);
	get_neighbour(f_se, Widelands::WALK_SE, &f_se);
	if (!is_resource_valid(world, f_se, f_se.field->get_resources())) {
		clear_resources(f_se);
	}

	// check south-west vertex if d-Triangle is changed, check east vertex if r-Triangle is changed
	Widelands::FCoords f_sw_e(c, c.field);
	get_neighbour(f_sw_e, c.t == TCoords<FCoords>::D ? Widelands::WALK_SW : Widelands::WALK_E, &f_sw_e);
	if (!is_resource_valid(world, f_sw_e, f_sw_e.field->get_resources())) {
		clear_resources(f_sw_e);
	}

	Notifications::publish(
	   NoteFieldTerrainChanged{c, static_cast<MapIndex>(c.field - &fields_[0])});

	// Changing the terrain can affect ports, which can be up to 3 fields away.
	constexpr int kPotentiallyAffectedNeighbors = 3;
	recalc_for_field_area(world, Area<FCoords>(c, kPotentiallyAffectedNeighbors));
	return kPotentiallyAffectedNeighbors;
}

bool Map::is_resource_valid
	(const Widelands::World& world, const TCoords<Widelands::FCoords>& c, int32_t const curres)
{
	if (curres == Widelands::kNoResource)
		return true;

	Widelands::FCoords f(c, c.field);
	Widelands::FCoords f1;

	int32_t count = 0;

	//  this field
	count += world.terrain_descr(f.field->terrain_r()).is_resource_valid(curres);
	count += world.terrain_descr(f.field->terrain_d()).is_resource_valid(curres);

	//  If one of the neighbours is impassable, count its resource stronger.
	//  top left neigbour
	get_neighbour(f, Widelands::WALK_NW, &f1);
	count += world.terrain_descr(f1.field->terrain_r()).is_resource_valid(curres);
	count += world.terrain_descr(f1.field->terrain_d()).is_resource_valid(curres);

	//  top right neigbour
	get_neighbour(f, Widelands::WALK_NE, &f1);
	count += world.terrain_descr(f1.field->terrain_d()).is_resource_valid(curres);

	//  left neighbour
	get_neighbour(f, Widelands::WALK_W, &f1);
	count += world.terrain_descr(f1.field->terrain_r()).is_resource_valid(curres);

	return count > 1;
}

void Map::ensure_resource_consistency(const World& world)
{
	for (MapIndex i = 0; i < max_index(); ++i) {
		auto fcords = get_fcoords(fields_[i]);
		if (!is_resource_valid(world, fcords, fcords.field->get_resources())) {
			clear_resources(fcords);
		}
	}
}

void Map::initialize_resources(const FCoords& c,
                               const DescriptionIndex resource_type,
                               uint8_t amount) {
	// You cannot have an amount of nothing.
	if (resource_type == Widelands::kNoResource) {
		amount = 0;
	}
	const auto note = NoteFieldResourceChanged{
	   c, c.field->resources, c.field->initial_res_amount, c.field->res_amount,
	};

	c.field->resources = resource_type;
	c.field->initial_res_amount = amount;
	c.field->res_amount = amount;
	Notifications::publish(note);
}

void Map::set_resources(const FCoords& c, uint8_t amount) {
	// You cannot change the amount of resources on a field without resources.
	if (c.field->resources == Widelands::kNoResource) {
		return;
	}
	const auto note = NoteFieldResourceChanged{
	   c, c.field->resources, c.field->initial_res_amount, c.field->res_amount,
	};
	c.field->res_amount = amount;
	Notifications::publish(note);
}

void Map::clear_resources(const FCoords& c) {
	initialize_resources(c, Widelands::kNoResource, 0);
}

uint32_t Map::set_height(const World& world, const FCoords fc, uint8_t const new_value) {
	assert(new_value <= MAX_FIELD_HEIGHT);
	assert(fields_.get() <= fc.field);
	assert            (fc.field < fields_.get() + max_index());
	fc.field->height = new_value;
	uint32_t radius = 2;
	check_neighbour_heights(fc, radius);
	recalc_for_field_area(world, Area<FCoords>(fc, radius));
	return radius;
}

uint32_t Map::change_height(const World& world, Area<FCoords> area, int16_t const difference) {
	{
		MapRegion<Area<FCoords> > mr(*this, area);
		do {
			if
				(difference < 0
				 &&
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
	recalc_for_field_area(world, area);
	return area.radius;
}

uint32_t Map::set_height
	(const World& world, Area<FCoords> area, HeightInterval height_interval)
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
	recalc_for_field_area(world, area);
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
	assert(fields_.get() <= coords.field);
	assert            (coords.field < fields_.get() + max_index());

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

/*
===========
Map::allows_seafaring()

This function checks if there are two ports that are reachable
for each other - then the map is seafaring.
=============
*/
bool Map::allows_seafaring() {
	Map::PortSpacesSet port_spaces = get_port_spaces();
	std::vector<Coords> portdocks;
	std::set<Coords, Coords::OrderingFunctor> swim_coords;

	for (const Coords& c : port_spaces) {
		std::queue<Coords> q_positions;
		std::set<Coords, Coords::OrderingFunctor> visited_positions;
		FCoords fc = get_fcoords(c);
		portdocks = find_portdock(fc);

		/* remove the port space if it is not longer valid port space */
		if ((fc.field->get_caps() & BUILDCAPS_SIZEMASK) != BUILDCAPS_BIG || portdocks.empty()) {
			set_port_space(c, false);
			continue;
		}

		for (const Coords& portdock: portdocks) {
			visited_positions.insert(portdock);
			q_positions.push(portdock);
		}

		while (!q_positions.empty()) {
			const Coords& swim_coord = q_positions.front();
			q_positions.pop();
			for (uint8_t i = 1; i <= 6; ++i) {
				FCoords neighbour;
				get_neighbour(get_fcoords(swim_coord), i, &neighbour);
				if ((neighbour.field->get_caps() & (MOVECAPS_SWIM | MOVECAPS_WALK)) == MOVECAPS_SWIM) {
					if (visited_positions.count(neighbour) == 0) {
						visited_positions.insert(neighbour);
						q_positions.push(neighbour);
					}
				}
			}
		}

		for (const Coords& swim_coord: visited_positions)
			if (swim_coords.count(swim_coord) == 0)
				swim_coords.insert(swim_coord);
			else
				return true;
	}
	return false;
}

bool Map::has_artifacts() {
	for (MapIndex i = 0; i < max_index(); ++i) {
		if (upcast(Immovable, immovable, fields_[i].get_immovable())) {
			if (immovable->descr().has_attribute(immovable->descr().get_attribute_id("artifact"))) {
				return true;
			}
		}
	}
	return false;
}


#define MAX_RADIUS 32
MilitaryInfluence Map::calc_influence
	(Coords const a, Area<> const area) const
{
	const int16_t w = get_width();
	const int16_t h = get_height();
	MilitaryInfluence influence =
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
