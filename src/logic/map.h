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

#ifndef MAP_H
#define MAP_H

#include <cstring>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "economy/itransport_cost_calculator.h"
#include "logic/field.h"
#include "interval.h"
#include "manager.h"
#include "logic/map_revision.h"
#include "logic/notification.h"
#include "logic/objective.h"
#include "random.h"
#include "logic/widelands_geometry.h"
#include "logic/world.h"

class FileSystem;
class Image;
struct Overlay_Manager;
struct S2_Map_Loader;

namespace Widelands {

struct MapGenerator;
struct BaseImmovable;
struct PathfieldManager;
class Player;
struct World;
class Map;
class Map_Loader;
#define WLMF_SUFFIX ".wmf"
#define S2MF_SUFFIX ".swd"
#define S2MF_SUFFIX2 ".wld"

#define S2MF_MAGIC  "WORLD_V1.0"


uint16_t const NUMBER_OF_MAP_DIMENSIONS = 29;
const uint16_t MAP_DIMENSIONS[] = {
	64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512
};


struct Path;
class Immovable;


struct ImmovableFound {
	BaseImmovable * object;
	Coords          coords;
};

/*
FindImmovable
FindBob
FindNode
FindResource
CheckStep

Predicates used in path finding and find functions.
*/
struct FindImmovable;
const FindImmovable & FindImmovableAlwaysTrue();

struct FindBob {
	//  Return true if this bob should be returned by find_bobs.
	virtual bool accept(Bob *) const = 0;
	virtual ~FindBob() {}  // make gcc shut up
};
struct FindNode;
struct CheckStep;

/*
Some very simple default predicates (more predicates below Map).
*/
struct FindBobAlwaysTrue : public FindBob {
	virtual bool accept(Bob *) const override {return true;}
	virtual ~FindBobAlwaysTrue() {}  // make gcc shut up
};

/** class Map
 *
 * This really identifies a map like it is in the game
 *
 * Odd rows are shifted FIELD_WIDTH/2 to the right. This means that moving
 * up and down depends on the row numbers:
 *               even   odd
 * top-left      -1/-1   0/-1
 * top-right      0/-1  +1/-1
 * bottom-left   -1/+1   0/+1
 * bottom-right   0/+1  +1/+1
 *
 * Warning: width and height must be even
 */
class Map :
	public ITransportCostCalculator,
	public NoteSender<NoteFieldTransformed>
{
public:
	friend class Editor_Game_Base;
	friend class Map_Loader;
	friend struct ::S2_Map_Loader;
	friend struct WL_Map_Loader;
	friend struct Map_Elemental_Data_Packet;
	friend struct Map_Extradata_Data_Packet;
	friend struct Map_Version_Data_Packet;
	friend class Editor;
	friend struct Main_Menu_New_Map;
	friend struct MapGenerator;
	friend struct MapAStarBase;

	typedef std::set<Coords, Coords::ordering_functor> PortSpacesSet;

	enum { // flags for findpath()

		//  use bidirection cost instead of normal cost calculations
		//  should be used for road building
		fpBidiCost = 1,

	};

	// ORed bits for scenario types
	typedef size_t ScenarioTypes;
	enum {
		NO_SCENARIO = 0,
		SP_SCENARIO = 1,
		MP_SCENARIO = 2 };

	Map ();
	virtual ~Map();

	Overlay_Manager * get_overlay_manager()       {return m_overlay_manager.get();}
	Overlay_Manager * get_overlay_manager() const {return m_overlay_manager.get();}
	const Overlay_Manager & overlay_manager() const {return *m_overlay_manager;}
	Overlay_Manager       & overlay_manager()       {return *m_overlay_manager;}

	/// Returns the correct initialized loader for the given mapfile
	std::unique_ptr<Map_Loader> get_correct_loader(const std::string& filename);

	void cleanup();

	void create_empty_map // for editor
		(uint32_t w = 64, uint32_t h = 64,
		 const std::string & worldname   =   "greenland",
		 char        const * name        = _("No Name"),
		 char        const * author      = _("Unknown"),
		 char        const * description = _("no description defined"));

	void load_graphics();
	void recalc_whole_map();
	virtual void recalc_for_field_area(Area<FCoords>);
	void recalc_default_resources();

	void set_nrplayers(Player_Number);

	void set_starting_pos(Player_Number, Coords);
	Coords get_starting_pos(Player_Number const p) const {
		assert(p);
		assert(p <= get_nrplayers());
		return m_starting_pos[p - 1];
	}

	void set_filename   (char const *);
	void set_author     (char const *);
	void set_world_name (char const *);
	void set_name       (char const *);
	void set_description(char const *);
	void set_hint       (std::string);
	void set_background (char const *);
	void add_tag        (std::string);
	void set_scenario_types(ScenarioTypes t) {m_scenario_types = t;}

	// Allows access to the filesystem of the map to access auxiliary files.
	// This can be nullptr if this file is new.
	FileSystem* filesystem() const;

	// informational functions
	const char * get_filename()    const {return m_filename;}
	const char * get_author()      const {return m_author;}
	const char * get_name()        const {return m_name;}
	const char * get_description() const {return m_description;}
	std::string  get_hint()        const {return m_hint;}
	const char * get_world_name()  const {return m_worldname;}
	const std::string & get_background() const {return m_background;}
	typedef std::set<std::string> Tags;
	const Tags & get_tags() const {return m_tags;}
	bool has_tag(std::string & s) const {return m_tags.count(s);}

	Player_Number get_nrplayers() const {return m_nrplayers;}
	ScenarioTypes scenario_types() const {return m_scenario_types;}
	Extent extent() const {return Extent(m_width, m_height);}
	X_Coordinate get_width   () const {return m_width;}
	Y_Coordinate get_height  () const {return m_height;}
	World & world() const {return *m_world;}
	World * get_world() const {return m_world;}

	//  The next few functions are only valid when the map is loaded as a
	//  scenario.
	const std::string & get_scenario_player_tribe    (Player_Number) const;
	const std::string & get_scenario_player_name     (Player_Number) const;
	const std::string & get_scenario_player_ai       (Player_Number) const;
	bool                get_scenario_player_closeable(Player_Number) const;
	void set_scenario_player_tribe    (Player_Number, const std::string &);
	void set_scenario_player_name     (Player_Number, const std::string &);
	void set_scenario_player_ai       (Player_Number, const std::string &);
	void set_scenario_player_closeable(Player_Number, bool);

	/// \returns the maximum theoretical possible nodecaps (no blocking bobs, etc.)
	NodeCaps get_max_nodecaps(FCoords &);

	BaseImmovable * get_immovable(Coords) const;
	uint32_t find_bobs
		(const Area<FCoords>,
		 std::vector<Bob *> * list,
		 const FindBob & functor = FindBobAlwaysTrue());
	uint32_t find_reachable_bobs
		(const Area<FCoords>,
		 std::vector<Bob *> * list,
		 const CheckStep &,
		 const FindBob & functor = FindBobAlwaysTrue());
	uint32_t find_immovables
		(const Area<FCoords>,
		 std::vector<ImmovableFound> * list,
		 const FindImmovable & = FindImmovableAlwaysTrue());
	uint32_t find_reachable_immovables
		(const Area<FCoords>,
		 std::vector<ImmovableFound> * list,
		 const CheckStep &,
		 const FindImmovable & = FindImmovableAlwaysTrue());
	uint32_t find_reachable_immovables_unique
		(const Area<FCoords>,
		 std::vector<BaseImmovable *> & list,
		 const CheckStep &,
		 const FindImmovable & = FindImmovableAlwaysTrue());
	uint32_t find_fields
		(const Area<FCoords>,
		 std::vector<Coords> * list,
		 const FindNode & functor);
	uint32_t find_reachable_fields
		(const Area<FCoords>,
		 std::vector<Coords>* list,
		 const CheckStep &,
		 const FindNode &);

	// Field logic
	static Map_Index get_index(const Coords &, X_Coordinate width);
	Map_Index max_index() const {return m_width * m_height;}
	Field & operator[](Map_Index) const;
	Field & operator[](const Coords &) const;
	FCoords get_fcoords(const Coords &) const;
	void normalize_coords(Coords &) const;
	FCoords get_fcoords(Field &) const;
	void get_coords(Field & f, Coords & c) const;

	uint32_t calc_distance(Coords, Coords) const;

	int32_t calc_cost_estimate(Coords, Coords) const override;
	int32_t calc_cost_lowerbound(Coords, Coords) const;
	int32_t calc_cost(int32_t slope) const;
	int32_t calc_cost(Coords, int32_t dir) const;
	int32_t calc_bidi_cost(Coords, int32_t dir) const;
	void calc_cost(const Path &, int32_t * forward, int32_t * backward) const;

	void get_ln  (const Coords &,  Coords *) const;
	void get_ln (const FCoords &, FCoords *) const;
	Coords  l_n  (const Coords &) const;
	FCoords l_n (const FCoords &) const;
	void get_rn  (const Coords &,  Coords *) const;
	void get_rn (const FCoords &, FCoords *) const;
	Coords  r_n  (const Coords &) const;
	FCoords r_n (const FCoords &) const;
	void get_tln (const Coords &,  Coords *) const;
	void get_tln(const FCoords &, FCoords *) const;
	Coords  tl_n (const Coords &) const;
	FCoords tl_n(const FCoords &) const;
	void get_trn (const Coords &,  Coords *) const;
	void get_trn(const FCoords &, FCoords *) const;
	Coords  tr_n (const Coords &) const;
	FCoords tr_n(const FCoords &) const;
	void get_bln (const Coords &,  Coords *) const;
	void get_bln(const FCoords &, FCoords *) const;
	Coords  bl_n (const Coords &) const;
	FCoords bl_n(const FCoords &) const;
	void get_brn (const Coords &,  Coords *) const;
	void get_brn(const FCoords &, FCoords *) const;
	Coords  br_n (const Coords &) const;
	FCoords br_n(const FCoords &) const;

	void get_neighbour (const Coords &, Direction dir,  Coords *) const;
	void get_neighbour(const FCoords &, Direction dir, FCoords *) const;
	FCoords get_neighbour(const FCoords &, Direction dir) const;

	// Pathfinding
	int32_t findpath
		(Coords instart,
		 Coords inend,
		 const int32_t persist,
		 Path &,
		 const CheckStep &,
		 const uint32_t flags = 0);

	/**
	 * We can reach a field by water either if it has MOVECAPS_SWIM or if it has
	 * MOVECAPS_WALK and at least one of the neighbours has MOVECAPS_SWIM
	 */
	bool can_reach_by_water(Coords) const;

	/// Sets the height to a value. Recalculates brightness. Changes the
	/// surrounding nodes if necessary. Returns the radius that covers all
	/// changes that were made.
	///
	/// Do not call this to set the height of each node in an area to the same
	/// value, because it adjusts the heights of surrounding nodes in each call,
	/// so it will be terribly slow. Use set_height for Area for that purpose
	/// instead.
	uint32_t set_height(FCoords, Field::Height);

	/// Changes the height of the nodes in an Area by a difference.
	uint32_t change_height(Area<FCoords>, int16_t difference);

	/**
	 * Ensures that the height of each node within radius from fc is in
	 * height_interval. If the height is < height_interval.min, it is changed to
	 * height_interval.min. If the height is > height_interval.max, it is changed
	 * to height_interval.max. Otherwise it is left unchanged.
	 *
	 * Recalculates brightness. Changes the surrounding nodes if necessary.
	 * Returns the radius of the area that covers all changes that were made.
	 *
	 * Calling this is much faster than calling change_height for each node in
	 * the area, because this adjusts the surrounding nodes only once, after all
	 * nodes in the area had their new height set.
	 */
	uint32_t set_height(Area<FCoords>, interval<Field::Height> height_interval);

	//  change terrain of a triangle, recalculate buildcaps
	int32_t change_terrain(TCoords<FCoords>, Terrain_Index);

	const Manager<Objective>  & mom() const {return m_mom;}
	Manager<Objective>        & mom()       {return m_mom;}

	/// Returns the military influence on a location from an area.
	Military_Influence calc_influence(Coords, Area<>) const;

	/// Translate the whole map so that the given point becomes the new origin.
	void set_origin(Coords);

	/// Port space specific functions
	bool is_port_space(const Coords& c);
	void set_port_space(Coords c, bool allowed);
	const PortSpacesSet& get_port_spaces() {return m_port_spaces;}
	std::vector<Coords> find_portdock(const Widelands::Coords& c) const;

protected: /// These functions are needed in Testclasses
	void set_size(uint32_t w, uint32_t h);

private:
	void load_world();
	void recalc_border(FCoords);

	/// # of players this map supports (!= Game's number of players!)
	Player_Number m_nrplayers;
	ScenarioTypes m_scenario_types; // whether the map is playable as scenario

	X_Coordinate m_width;
	Y_Coordinate m_height;
	char        m_filename    [256];
	char        m_author       [61];
	char        m_name         [61];
	char        m_description[1024];
	std::string m_hint;
	char        m_worldname  [1024];
	std::string m_background;
	Tags        m_tags;
	World     * m_world;           //  world type
	Coords    * m_starting_pos;    //  players' starting positions

	Field     * m_fields;

	std::unique_ptr<Overlay_Manager> m_overlay_manager;

	std::unique_ptr<PathfieldManager> m_pathfieldmgr;
	std::vector<std::string> m_scenario_tribes;
	std::vector<std::string> m_scenario_names;
	std::vector<std::string> m_scenario_ais;
	std::vector<bool>        m_scenario_closeables;

	// The map file as a filesystem.
	std::unique_ptr<FileSystem> filesystem_;

	PortSpacesSet m_port_spaces;

	Manager<Objective>  m_mom;

	void recalc_brightness(FCoords);
	void recalc_nodecaps_pass1(FCoords);
	void recalc_nodecaps_pass2(const FCoords & f);
	NodeCaps _calc_nodecaps_pass1(FCoords, bool consider_mobs = true);
	NodeCaps _calc_nodecaps_pass2(FCoords, bool consider_mobs = true, NodeCaps initcaps = CAPS_NONE);
	void check_neighbour_heights(FCoords, uint32_t & radius);
	int calc_buildsize
		(const Widelands::FCoords& f, bool avoidnature, bool * ismine = nullptr,
		 bool consider_mobs = true, NodeCaps initcaps = CAPS_NONE);
	bool is_cycle_connected
		(const FCoords & start, uint32_t length, const WalkingDir * dirs);

	template<typename functorT>
		void find_reachable(Area<FCoords>, const CheckStep &, functorT &);

	template<typename functorT> void find(const Area<FCoords>, functorT &) const;

	MapVersion m_map_version;
};


/*
==============================================================================

Field arithmetics

==============================================================================
*/

inline Map_Index Map::get_index(const Coords & c, X_Coordinate const width) {
	assert(0 < width);
	assert(0 <= c.x);
	assert     (c.x < width);
	assert(0 <= c.y);
	return c.y * width + c.x;
}

inline Field & Map::operator[](Map_Index const i) const {return m_fields[i];}
inline Field & Map::operator[](const Coords & c) const {
	return operator[](get_index(c, m_width));
}

inline FCoords Map::get_fcoords(const Coords & c) const
{
	return FCoords(c, &operator[](c));
}

inline void Map::normalize_coords(Coords & c) const
{
	while (c.x < 0)         c.x += m_width;
	while (c.x >= m_width)  c.x -= m_width;
	while (c.y < 0)         c.y += m_height;
	while (c.y >= m_height) c.y -= m_height;
}


/**
 * Calculate the field coordates from the pointer
 */
inline FCoords Map::get_fcoords(Field & f) const {
	const int32_t i = &f - m_fields;
	return FCoords(Coords(i % m_width, i / m_width), &f);
}
inline void Map::get_coords(Field & f, Coords & c) const {c = get_fcoords(f);}


/** get_ln, get_rn, get_tln, get_trn, get_bln, get_brn
 *
 * Calculate the coordinates and Field pointer of a neighboring field.
 * Assume input coordinates are valid.
 *
 * Note: Input coordinates are passed as value because we have to allow
 *       usage get_XXn(foo, &foo).
 */
inline void Map::get_ln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y;
	o->x = (f.x ? f.x : m_width) - 1;
	assert(0 <= o->x);
	assert(0 <= o->y);
	assert(o->x < m_width);
	assert(o->y < m_height);
}

inline void Map::get_ln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->y = f.y;
	o->x = f.x - 1;
	o->field = f.field - 1;
	if (o->x == -1) {
		o->x = m_width - 1;
		o->field += m_width;
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields <= o->field);
	assert            (o->field < m_fields + max_index());
}
inline Coords Map::l_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x - 1, f.y);
	if (result.x == -1)
		result.x = m_width - 1;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::l_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	FCoords result(Coords(f.x - 1, f.y), f.field - 1);
	if (result.x == -1) {
		result.x = m_width - 1;
		result.field += m_width;
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields <= result.field);
	assert            (result.field < m_fields + max_index());
	return result;
}

inline void Map::get_rn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y;
	o->x = f.x + 1;
	if (o->x == m_width)
		o->x = 0;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_rn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->y = f.y;
	o->x = f.x + 1;
	o->field = f.field + 1;
	if (o->x == m_width) {o->x = 0; o->field -= m_width;}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields <= o->field);
	assert            (o->field < m_fields + max_index());
}
inline Coords Map::r_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x + 1, f.y);
	if (result.x == m_width)
		result.x = 0;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::r_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	FCoords result(Coords(f.x + 1, f.y), f.field + 1);
	if (result.x == m_width) {result.x = 0; result.field -= m_width;}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields <= result.field);
	assert            (result.field < m_fields + max_index());
	return result;
}

// top-left: even: -1/-1  odd: 0/-1
inline void Map::get_tln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y - 1;
	o->x = f.x;
	if (o->y & 1) {
		if (o->y == -1)
			o->y = m_height - 1;
		o->x = (o->x ? o->x : m_width) - 1;
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_tln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->y = f.y - 1;
	o->x = f.x;
	o->field = f.field - m_width;
	if (o->y & 1) {
		if (o->y == -1) {
			o->y = m_height - 1;
			o->field += max_index();
		}
		--o->x;
		--o->field;
		if (o->x == -1) {
			o->x = m_width - 1;
			o->field += m_width;
		}
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields <= o->field);
	assert            (o->field < m_fields + max_index());
}
inline Coords Map::tl_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y - 1);
	if (result.y & 1) {
		if (result.y == -1)
			result.y = m_height - 1;
		--result.x;
		if (result.x == -1)
			result.x = m_width  - 1;
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::tl_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	FCoords result(Coords(f.x, f.y - 1), f.field - m_width);
	if (result.y & 1) {
		if (result.y == -1) {
			result.y = m_height - 1;
			result.field += max_index();
		}
		--result.x;
		--result.field;
		if (result.x == -1) {
			result.x = m_width - 1;
			result.field += m_width;
		}
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields <= result.field);
	assert            (result.field < m_fields + max_index());
	return result;
}

// top-right: even: 0/-1  odd: +1/-1
inline void Map::get_trn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->x = f.x;
	if (f.y & 1) {
		++o->x;
		if (o->x == m_width)
			o->x = 0;
	}
	o->y = (f.y ? f.y : m_height) - 1;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_trn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->x = f.x;
	o->field = f.field - m_width;
	if (f.y & 1) {
		++o->x;
		++o->field;
		if (o->x == m_width) {
			o->x = 0;
			o->field -= m_width;
		}
	}
	o->y = f.y - 1;
	if (o->y == -1) {
		o->y = m_height - 1;
		o->field += max_index();
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields <= o->field);
	assert            (o->field < m_fields + max_index());
}
inline Coords Map::tr_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y - 1);
	if (f.y & 1) {
		++result.x;
		if (result.x == m_width)
			result.x = 0;
	}
	if (result.y == -1)
		result.y = m_height - 1;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::tr_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	FCoords result(Coords(f.x, f.y - 1), f.field - m_width);
	if (f.y & 1) {
		++result.x;
		++result.field;
		if (result.x == m_width) {
			result.x = 0;
			result.field -= m_width;
		}
	}
	if (result.y == -1) {
		result.y = m_height - 1;
		result.field += max_index();
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields <= result.field);
	assert            (result.field < m_fields + max_index());
	return result;
}

// bottom-left: even: -1/+1  odd: 0/+1
inline void Map::get_bln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y + 1;
	o->x = f.x;
	if (o->y == m_height)
		o->y = 0;
	if (o->y & 1)
		o->x = (o->x ? o->x : m_width) - 1;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_bln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->y = f.y + 1;
	o->x = f.x;
	o->field = f.field + m_width;
	if (o->y == m_height) {
		o->y = 0;
		o->field -= max_index();
	}
	if (o->y & 1) {
		--o->x;
		--o->field;
		if (o->x == -1) {
			o->x = m_width - 1;
			o->field += m_width;
		}
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields <= o->field);
	assert            (o->field < m_fields + max_index());
}
inline Coords Map::bl_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y + 1);
	if (result.y == m_height)
		result.y = 0;
	if (result.y & 1) {
		--result.x;
		if (result.x == -1)
			result.x = m_width - 1;
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::bl_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	FCoords result(Coords(f.x, f.y + 1), f.field + m_width);
	if (result.y == m_height) {
		result.y = 0;
		result.field -= max_index();
	}
	if (result.y & 1) {
		--result.x;
		--result.field;
		if (result.x == -1) {
			result.x = m_width - 1;
			result.field += m_width;
		}
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields <= result.field);
	assert            (result.field < m_fields + max_index());
	return result;
}

// bottom-right: even: 0/+1  odd: +1/+1
inline void Map::get_brn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->x = f.x;
	if (f.y & 1) {
		++o->x;
		if (o->x == m_width)
			o->x = 0;
	}
	o->y = f.y + 1;
	if (o->y == m_height)
		o->y = 0;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_brn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->x = f.x;
	o->field = f.field + m_width;
	if (f.y & 1) {
		++o->x;
		++o->field;
		if (o->x == m_width) {
			o->x = 0;
			o->field -= m_width;
		}
	}
	o->y = f.y + 1;
	if (o->y == m_height) {
		o->y = 0;
		o->field -= max_index();
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields <= o->field);
	assert            (o->field < m_fields + max_index());
}
inline Coords Map::br_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	Coords result(f.x, f.y + 1);
	if (f.y & 1) {
		++result.x;
		if (result.x == m_width)
			result.x = 0;
	}
	if (result.y == m_height)
		result.y = 0;
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	return result;
}
inline FCoords Map::br_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	FCoords result(Coords(f.x, f.y + 1), f.field + m_width);
	if (f.y & 1) {
		++result.x;
		++result.field;
		if (result.x == m_width) {
			result.x = 0;
			result.field -= m_width;
		}
	}
	if (result.y == m_height) {
		result.y = 0;
		result.field -= max_index();
	}
	assert(0 <= result.x);
	assert(result.x < m_width);
	assert(0 <= result.y);
	assert(result.y < m_height);
	assert(m_fields <= result.field);
	assert            (result.field < m_fields + max_index());
	return result;
}

inline FCoords Map::get_neighbour(const FCoords & f, const Direction dir) const
{
	switch (dir) {
	case WALK_NW: return tl_n(f);
	case WALK_NE: return tr_n(f);
	case WALK_E:  return  r_n(f);
	case WALK_SE: return br_n(f);
	case WALK_SW: return bl_n(f);
	//case WALK_W:  return  l_n(f);
	default:
	assert(WALK_W == dir);
	return l_n(f);
	}
}

inline void move_r(const X_Coordinate mapwidth, FCoords & f) {
	assert(f.x < mapwidth);
	++f.x;
	++f.field;
	if (f.x == mapwidth) {f.x = 0; f.field -= mapwidth;}
	assert(f.x < mapwidth);
}

inline void move_r(X_Coordinate const mapwidth, FCoords & f, Map_Index & i) {
	assert(f.x < mapwidth);
	++f.x;
	++f.field;
	++i;
	if (f.x == mapwidth) {f.x = 0; f.field -= mapwidth; i -= mapwidth;}
	assert(f.x < mapwidth);
}


#define iterate_Map_FCoords(map, extent, fc)                                  \
   for                                                                        \
      (Widelands::FCoords fc = (map).get_fcoords(Widelands::Coords(0, 0));    \
       fc.y < static_cast<Widelands::Y_Coordinate>(extent.h);                 \
       ++fc.y)                                                                \
      for                                                                     \
         (fc.x = 0;                                                           \
          fc.x < static_cast<Widelands::X_Coordinate>(extent.w);              \
          ++fc.x, ++fc.field)                                                 \

}


#endif
