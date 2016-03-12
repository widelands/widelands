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

#ifndef WL_LOGIC_MAP_H
#define WL_LOGIC_MAP_H

#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "economy/itransport_cost_calculator.h"
#include "logic/field.h"
#include "logic/description_maintainer.h"
#include "logic/map_revision.h"
#include "logic/objective.h"
#include "logic/map_objects/walkingdir.h"
#include "logic/widelands_geometry.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"
#include "random/random.h"

class FileSystem;
class Image;
struct S2MapLoader;

namespace Widelands {

class MapLoader;
class Objective;
class World;
struct BaseImmovable;
struct MapGenerator;
struct PathfieldManager;

#define WLMF_SUFFIX ".wmf"
#define S2MF_SUFFIX ".swd"
#define S2MF_SUFFIX2 ".wld"

#define S2MF_MAGIC  "WORLD_V1.0"

// Global list of available map dimensions.
const std::vector<int32_t> kMapDimensions = {
	64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256, 272, 288, 304,
	320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512
};


struct Path;
class Immovable;

struct NoteFieldTerrainChanged {
	CAN_BE_SENT_AS_NOTE(NoteId::FieldTerrainChanged)

	FCoords fc;
	MapIndex map_index;
};

/// Send when the resource of a field is changed.
struct NoteFieldResourceChanged {
	CAN_BE_SENT_AS_NOTE(NoteId::FieldResourceTypeChanged)

	FCoords fc;
	DescriptionIndex old_resource;
	uint8_t old_initial_amount;
	uint8_t old_amount;
};

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
const FindImmovable & find_immovable_always_true();

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
	bool accept(Bob *) const override {return true;}
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
class Map : public ITransportCostCalculator {
public:
	friend class Editor;
	friend class EditorGameBase;
	friend class MapLoader;
	friend class MapVersionPacket;
	friend struct ::S2MapLoader;
	friend struct MainMenuNewMap;
	friend struct MapAStarBase;
	friend struct MapGenerator;
	friend struct MapElementalPacket;
	friend struct WidelandsMapLoader;

	using PortSpacesSet = std::set<Coords, Coords::OrderingFunctor>;
	using Objectives = std::map<std::string, std::unique_ptr<Objective>>;
	using SuggestedTeam = std::vector<PlayerNumber>;             // Players in a team
	using SuggestedTeamLineup = std::vector<SuggestedTeam>; // Recommended teams to play against each other


	enum { // flags for findpath()

		//  use bidirection cost instead of normal cost calculations
		//  should be used for road building
		fpBidiCost = 1,
	};

	// ORed bits for scenario types
	using ScenarioTypes = size_t;
	enum {
		NO_SCENARIO = 0,
		SP_SCENARIO = 1,
		MP_SCENARIO = 2 };

	Map ();
	virtual ~Map();

	/// Returns the correct initialized loader for the given mapfile
	std::unique_ptr<MapLoader> get_correct_loader(const std::string& filename);

	void cleanup();

	void create_empty_map  // for editor
	   (const World& world,
	    uint32_t w = 64,
	    uint32_t h = 64,
		 const Widelands::DescriptionIndex default_terrain = 0,
		 const std::string& name = _("No Name"),
		 const std::string& author = pgettext("author_name", "Unknown"),
		 const std::string& description = _("No description defined"));

	void recalc_whole_map(const World& world);
	virtual void recalc_for_field_area(const World& world, Area<FCoords>);

	/***
	 * Ensures that resources match their adjacent terrains.
	 */
	void ensure_resource_consistency(const World& world);

	/***
	 * Recalculates all default resources.
	 *
	 * This is just needed for the game, not for
	 * the editor. Since there, default resources
	 * are not shown.
	 */
	void recalc_default_resources(const World& world);

	void set_nrplayers(PlayerNumber);

	void set_starting_pos(PlayerNumber, Coords);
	Coords get_starting_pos(PlayerNumber const p) const {
		assert(1 <= p && p <= get_nrplayers());
		return starting_pos_[p - 1];
	}

	void set_filename   (const std::string& filename);
	void set_author     (const std::string& author);
	void set_name       (const std::string& name);
	void set_description(const std::string& description);
	void set_hint       (const std::string& hint);
	void set_background (const std::string& image_path);
	void add_tag        (const std::string& tag);
	void delete_tag     (const std::string& tag);
	void set_scenario_types(ScenarioTypes t) {scenario_types_ = t;}

	// Allows access to the filesystem of the map to access auxiliary files.
	// This can be nullptr if this file is new.
	FileSystem* filesystem() const;
	// swap the filesystem after load / save
	void swap_filesystem(std::unique_ptr<FileSystem>& fs);

	// informational functions
	const std::string& get_filename()    const {return filename_;}
	const std::string& get_author()      const {return author_;}
	const std::string& get_name()        const {return name_;}
	const std::string& get_description() const {return description_;}
	const std::string& get_hint()        const {return hint_;}
	const std::string& get_background()  const {return background_;}

	using Tags = std::set<std::string>;
	const Tags & get_tags() const {return tags_;}
	void clear_tags() {tags_.clear();}
	bool has_tag(const std::string& s) const {return tags_.count(s);}

	const std::vector<SuggestedTeamLineup>& get_suggested_teams() const {return suggested_teams_;}

	PlayerNumber get_nrplayers() const {return nrplayers_;}
	ScenarioTypes scenario_types() const {return scenario_types_;}
	Extent extent() const {return Extent(width_, height_);}
	int16_t get_width   () const {return width_;}
	int16_t get_height  () const {return height_;}

	//  The next few functions are only valid when the map is loaded as a
	//  scenario.
	const std::string & get_scenario_player_tribe    (PlayerNumber) const;
	const std::string & get_scenario_player_name     (PlayerNumber) const;
	const std::string & get_scenario_player_ai       (PlayerNumber) const;
	bool                get_scenario_player_closeable(PlayerNumber) const;
	void set_scenario_player_tribe    (PlayerNumber, const std::string &);
	void set_scenario_player_name     (PlayerNumber, const std::string &);
	void set_scenario_player_ai       (PlayerNumber, const std::string &);
	void set_scenario_player_closeable(PlayerNumber, bool);

	/// \returns the maximum theoretical possible nodecaps (no blocking bobs, etc.)
	NodeCaps get_max_nodecaps(const World& world, const FCoords &);

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
		 const FindImmovable & = find_immovable_always_true());
	uint32_t find_reachable_immovables
		(const Area<FCoords>,
		 std::vector<ImmovableFound> * list,
		 const CheckStep &,
		 const FindImmovable & = find_immovable_always_true());
	uint32_t find_reachable_immovables_unique
		(const Area<FCoords>,
		 std::vector<BaseImmovable *> & list,
		 const CheckStep &,
		 const FindImmovable & = find_immovable_always_true());
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
	static MapIndex get_index(const Coords &, int16_t width);
	MapIndex max_index() const {return width_ * height_;}
	Field & operator[](MapIndex) const;
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
	uint32_t set_height(const World& world, FCoords, Field::Height);

	/// Changes the height of the nodes in an Area by a difference.
	uint32_t change_height(const World& world, Area<FCoords>, int16_t difference);

	/// Initializes the 'initial_resources' on 'coords' to the 'resource_type'
	/// with the given 'amount'.
	void initialize_resources(const FCoords& coords, DescriptionIndex resource_type, uint8_t amount);

	/// Sets the number of resources of the field to 'amount'. The type of the
	/// resource on this field is not changed.
	void set_resources(const FCoords& coords, uint8_t amount);

	/// Clears the resources, i.e. the amount will be set to 0 and the type of
	/// resources will be kNoResource.
	void clear_resources(const FCoords& coords);

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
	uint32_t set_height(const World& world, Area<FCoords>, HeightInterval height_interval);

	/***
	 * Changes the given triangle's terrain. This happens in the editor and might
	 * happen in the game too if some kind of land increasement is implemented (like
	 * drying swamps). The nodecaps need to be recalculated
	 *
	 * @return the radius of changes.
	 */
	int32_t change_terrain(const World& world, TCoords<FCoords>, DescriptionIndex);

	/***
	 * Verify if a resource attached to a vertex has enough adjacent matching terrains to be valid.
	 *
	 * To qualify as valid, resources need to be surrounded by at least two matching terrains.
	 */
	bool is_resource_valid
		(const Widelands::World& world, const Widelands::TCoords<Widelands::FCoords>& c,
		int32_t const curres);

	// The objectives that are defined in this map if it is a scenario.
	const Objectives& objectives() const {
		return objectives_;
	}
	Objectives* mutable_objectives() {
		return &objectives_;
	}

	/// Returns the military influence on a location from an area.
	MilitaryInfluence calc_influence(Coords, Area<>) const;

	/// Translate the whole map so that the given point becomes the new origin.
	void set_origin(Coords);

	/// Port space specific functions
	bool is_port_space(const Coords& c) const;
	void set_port_space(Coords c, bool allowed);
	const PortSpacesSet& get_port_spaces() const {return port_spaces_;}
	std::vector<Coords> find_portdock(const Widelands::Coords& c) const;
	bool allows_seafaring();

	/// Checks whether there are any artifacts on the map
	bool has_artifacts();

protected: /// These functions are needed in Testclasses
	void set_size(uint32_t w, uint32_t h);

private:
	void recalc_border(FCoords);

	/// # of players this map supports (!= Game's number of players!)
	PlayerNumber nrplayers_;
	ScenarioTypes scenario_types_; // whether the map is playable as scenario

	int16_t width_;
	int16_t height_;
	std::string filename_;
	std::string author_;
	std::string name_;
	std::string description_;
	std::string hint_;
	std::string background_;
	Tags        tags_;
	std::vector<SuggestedTeamLineup> suggested_teams_;

	std::vector<Coords> starting_pos_;    //  players' starting positions

	std::unique_ptr<Field[]> fields_;

	std::unique_ptr<PathfieldManager> pathfieldmgr_;
	std::vector<std::string> scenario_tribes_;
	std::vector<std::string> scenario_names_;
	std::vector<std::string> scenario_ais_;
	std::vector<bool>        scenario_closeables_;

	// The map file as a filesystem.
	std::unique_ptr<FileSystem> filesystem_;

	PortSpacesSet port_spaces_;
	Objectives objectives_;

	void recalc_brightness(FCoords);
	void recalc_nodecaps_pass1(const World& world, FCoords);
	void recalc_nodecaps_pass2(const World& world, const FCoords & f);
	NodeCaps calc_nodecaps_pass1(const World& world, FCoords, bool consider_mobs = true);
	NodeCaps calc_nodecaps_pass2(const World& world,
	                              FCoords,
	                              bool consider_mobs = true,
	                              NodeCaps initcaps = CAPS_NONE);
	void check_neighbour_heights(FCoords, uint32_t & radius);
	int calc_buildsize
		(const World& world, const FCoords& f, bool avoidnature, bool * ismine = nullptr,
		 bool consider_mobs = true, NodeCaps initcaps = CAPS_NONE);
	bool is_cycle_connected
		(const FCoords & start, uint32_t length, const WalkingDir * dirs);
	template<typename functorT>
		void find_reachable(Area<FCoords>, const CheckStep &, functorT &);
	template<typename functorT> void find(const Area<FCoords>, functorT &) const;

	MapVersion map_version_;
};


/*
==============================================================================

Field arithmetics

==============================================================================
*/

inline MapIndex Map::get_index(const Coords & c, int16_t const width) {
	assert(0 < width);
	assert(0 <= c.x);
	assert     (c.x < width);
	assert(0 <= c.y);
	return c.y * width + c.x;
}

inline Field & Map::operator[](MapIndex const i) const {return fields_[i];}
inline Field & Map::operator[](const Coords & c) const {
	return operator[](get_index(c, width_));
}

inline FCoords Map::get_fcoords(const Coords & c) const
{
	return FCoords(c, &operator[](c));
}

inline void Map::normalize_coords(Coords & c) const
{
	while (c.x < 0)         c.x += width_;
	while (c.x >= width_)  c.x -= width_;
	while (c.y < 0)         c.y += height_;
	while (c.y >= height_) c.y -= height_;
}


/**
 * Calculate the field coordates from the pointer
 */
inline FCoords Map::get_fcoords(Field & f) const {
	const int32_t i = &f - fields_.get();
	return FCoords(Coords(i % width_, i / width_), &f);
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
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	o->y = f.y;
	o->x = (f.x ? f.x : width_) - 1;
	assert(0 <= o->x);
	assert(0 <= o->y);
	assert(o->x < width_);
	assert(o->y < height_);
}

inline void Map::get_ln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert            (f.field < fields_.get() + max_index());
	o->y = f.y;
	o->x = f.x - 1;
	o->field = f.field - 1;
	if (o->x == -1) {
		o->x = width_ - 1;
		o->field += width_;
	}
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
	assert(fields_.get() <= o->field);
	assert(o->field < fields_.get() + max_index());
}
inline Coords Map::l_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	Coords result(f.x - 1, f.y);
	if (result.x == -1)
		result.x = width_ - 1;
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	return result;
}
inline FCoords Map::l_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	FCoords result(Coords(f.x - 1, f.y), f.field - 1);
	if (result.x == -1) {
		result.x = width_ - 1;
		result.field += width_;
	}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	assert(fields_.get() <= result.field);
	assert(result.field < fields_.get() + max_index());
	return result;
}

inline void Map::get_rn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	o->y = f.y;
	o->x = f.x + 1;
	if (o->x == width_)
		o->x = 0;
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
}

inline void Map::get_rn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert            (f.field < fields_.get() + max_index());
	o->y = f.y;
	o->x = f.x + 1;
	o->field = f.field + 1;
	if (o->x == width_) {o->x = 0; o->field -= width_;}
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
	assert(fields_.get() <= o->field);
	assert(o->field < fields_.get() + max_index());
}
inline Coords Map::r_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	Coords result(f.x + 1, f.y);
	if (result.x == width_)
		result.x = 0;
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	return result;
}
inline FCoords Map::r_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	FCoords result(Coords(f.x + 1, f.y), f.field + 1);
	if (result.x == width_) {result.x = 0; result.field -= width_;}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	assert(fields_.get() <= result.field);
	assert(result.field < fields_.get() + max_index());
	return result;
}

// top-left: even: -1/-1  odd: 0/-1
inline void Map::get_tln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	o->y = f.y - 1;
	o->x = f.x;
	if (o->y & 1) {
		if (o->y == -1)
			o->y = height_ - 1;
		o->x = (o->x ? o->x : width_) - 1;
	}
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
}

inline void Map::get_tln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	o->y = f.y - 1;
	o->x = f.x;
	o->field = f.field - width_;
	if (o->y & 1) {
		if (o->y == -1) {
			o->y = height_ - 1;
			o->field += max_index();
		}
		--o->x;
		--o->field;
		if (o->x == -1) {
			o->x = width_ - 1;
			o->field += width_;
		}
	}
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
	assert(fields_.get() <= o->field);
	assert(o->field < fields_.get() + max_index());
}
inline Coords Map::tl_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	Coords result(f.x, f.y - 1);
	if (result.y & 1) {
		if (result.y == -1)
			result.y = height_ - 1;
		--result.x;
		if (result.x == -1)
			result.x = width_  - 1;
	}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	return result;
}
inline FCoords Map::tl_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	FCoords result(Coords(f.x, f.y - 1), f.field - width_);
	if (result.y & 1) {
		if (result.y == -1) {
			result.y = height_ - 1;
			result.field += max_index();
		}
		--result.x;
		--result.field;
		if (result.x == -1) {
			result.x = width_ - 1;
			result.field += width_;
		}
	}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	assert(fields_.get() <= result.field);
	assert(result.field < fields_.get() + max_index());
	return result;
}

// top-right: even: 0/-1  odd: +1/-1
inline void Map::get_trn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	o->x = f.x;
	if (f.y & 1) {
		++o->x;
		if (o->x == width_)
			o->x = 0;
	}
	o->y = (f.y ? f.y : height_) - 1;
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
}

inline void Map::get_trn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	o->x = f.x;
	o->field = f.field - width_;
	if (f.y & 1) {
		++o->x;
		++o->field;
		if (o->x == width_) {
			o->x = 0;
			o->field -= width_;
		}
	}
	o->y = f.y - 1;
	if (o->y == -1) {
		o->y = height_ - 1;
		o->field += max_index();
	}
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
	assert(fields_.get() <= o->field);
	assert(o->field < fields_.get() + max_index());
}
inline Coords Map::tr_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	Coords result(f.x, f.y - 1);
	if (f.y & 1) {
		++result.x;
		if (result.x == width_)
			result.x = 0;
	}
	if (result.y == -1)
		result.y = height_ - 1;
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	return result;
}
inline FCoords Map::tr_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	FCoords result(Coords(f.x, f.y - 1), f.field - width_);
	if (f.y & 1) {
		++result.x;
		++result.field;
		if (result.x == width_) {
			result.x = 0;
			result.field -= width_;
		}
	}
	if (result.y == -1) {
		result.y = height_ - 1;
		result.field += max_index();
	}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	assert(fields_.get() <= result.field);
	assert(result.field < fields_.get() + max_index());
	return result;
}

// bottom-left: even: -1/+1  odd: 0/+1
inline void Map::get_bln(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	o->y = f.y + 1;
	o->x = f.x;
	if (o->y == height_)
		o->y = 0;
	if (o->y & 1)
		o->x = (o->x ? o->x : width_) - 1;
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
}

inline void Map::get_bln(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	o->y = f.y + 1;
	o->x = f.x;
	o->field = f.field + width_;
	if (o->y == height_) {
		o->y = 0;
		o->field -= max_index();
	}
	if (o->y & 1) {
		--o->x;
		--o->field;
		if (o->x == -1) {
			o->x = width_ - 1;
			o->field += width_;
		}
	}
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
	assert(fields_.get() <= o->field);
	assert(o->field < fields_.get() + max_index());
}
inline Coords Map::bl_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	Coords result(f.x, f.y + 1);
	if (result.y == height_)
		result.y = 0;
	if (result.y & 1) {
		--result.x;
		if (result.x == -1)
			result.x = width_ - 1;
	}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	return result;
}
inline FCoords Map::bl_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	FCoords result(Coords(f.x, f.y + 1), f.field + width_);
	if (result.y == height_) {
		result.y = 0;
		result.field -= max_index();
	}
	if (result.y & 1) {
		--result.x;
		--result.field;
		if (result.x == -1) {
			result.x = width_ - 1;
			result.field += width_;
		}
	}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	assert(fields_.get() <= result.field);
	assert(result.field < fields_.get() + max_index());
	return result;
}

// bottom-right: even: 0/+1  odd: +1/+1
inline void Map::get_brn(const Coords & f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	o->x = f.x;
	if (f.y & 1) {
		++o->x;
		if (o->x == width_)
			o->x = 0;
	}
	o->y = f.y + 1;
	if (o->y == height_)
		o->y = 0;
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
}

inline void Map::get_brn(const FCoords & f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	o->x = f.x;
	o->field = f.field + width_;
	if (f.y & 1) {
		++o->x;
		++o->field;
		if (o->x == width_) {
			o->x = 0;
			o->field -= width_;
		}
	}
	o->y = f.y + 1;
	if (o->y == height_) {
		o->y = 0;
		o->field -= max_index();
	}
	assert(0 <= o->x);
	assert(o->x < width_);
	assert(0 <= o->y);
	assert(o->y < height_);
	assert(fields_.get() <= o->field);
	assert(o->field < fields_.get() + max_index());
}
inline Coords Map::br_n(const Coords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	Coords result(f.x, f.y + 1);
	if (f.y & 1) {
		++result.x;
		if (result.x == width_)
			result.x = 0;
	}
	if (result.y == height_)
		result.y = 0;
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	return result;
}
inline FCoords Map::br_n(const FCoords & f) const {
	assert(0 <= f.x);
	assert(f.x < width_);
	assert(0 <= f.y);
	assert(f.y < height_);
	assert(fields_.get() <= f.field);
	assert(f.field < fields_.get() + max_index());
	FCoords result(Coords(f.x, f.y + 1), f.field + width_);
	if (f.y & 1) {
		++result.x;
		++result.field;
		if (result.x == width_) {
			result.x = 0;
			result.field -= width_;
		}
	}
	if (result.y == height_) {
		result.y = 0;
		result.field -= max_index();
	}
	assert(0 <= result.x);
	assert(result.x < width_);
	assert(0 <= result.y);
	assert(result.y < height_);
	assert(fields_.get() <= result.field);
	assert(result.field < fields_.get() + max_index());
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
	case WALK_W:  return  l_n(f);
	default: NEVER_HERE();
	}
}

inline void move_r(const int16_t mapwidth, FCoords & f) {
	assert(f.x < mapwidth);
	++f.x;
	++f.field;
	if (f.x == mapwidth) {f.x = 0; f.field -= mapwidth;}
	assert(f.x < mapwidth);
}

inline void move_r(int16_t const mapwidth, FCoords & f, MapIndex & i) {
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
		 fc.y < static_cast<int16_t>(extent.h);                 \
       ++fc.y)                                                                \
      for                                                                     \
         (fc.x = 0;                                                           \
			 fc.x < static_cast<int16_t>(extent.w);              \
          ++fc.x, ++fc.field)                                                 \

}


#endif  // end of include guard: WL_LOGIC_MAP_H
