/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__MAP_H
#define __S__MAP_H

#include <string>
#include <vector>
#include "field.h"
#include "geometry.h"

class BaseImmovable;
class FileRead;
class Player;
class World;
class Overlay_Manager;
class MapVariableManager;
class MapObjectiveManager;
class MapEventManager;
class MapEventChainManager;
class MapTriggerManager;
class Map;
class Map_Loader;
#define WLMF_SUFFIX		".wmf"
#define S2MF_SUFFIX     ".swd"

#define S2MF_MAGIC		"WORLD_V1.0"


const ushort NUMBER_OF_MAP_DIMENSIONS=29;
const ushort MAP_DIMENSIONS[] = {
   64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256,
   272, 288, 304, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480,
   496, 512 };


class Path;
class Immovable;


struct ImmovableFound {
	BaseImmovable	*object;
	Coords			coords;
};

/*
FindImmovable
FindBob
FindField
FindResource
CheckStep

Predicates used in path finding and find functions.
*/
struct FindImmovable {
	// Return true if this immovable should be returned by find_immovables()
	virtual bool accept(BaseImmovable *imm) const = 0;
   virtual ~FindImmovable() {}  // make gcc shut up
};
struct FindBob {
	// Return true if this immovable should be returned by find_bobs()
	virtual bool accept(Bob *imm) const = 0;
   virtual ~FindBob() {}  // make gcc shut up
};
struct FindField {
	// Return true if this immovable should be returned by find_fields()
	virtual bool accept(FCoords coord) const = 0;
   virtual ~FindField() {}  // make gcc shut up
};
struct CheckStep {
	enum StepId {
		stepNormal,		// normal step treatment
		stepFirst,		// first step of a path
		stepLast,		// last step of a path
	};

	// Return true if moving from start to end (single step in the given
	// direction) is allowed.
	virtual bool allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const = 0;

	// Return true if the destination field can be reached at all
	// (e.g. return false for land-based bobs when dest is in water).
	virtual bool reachabledest(Map* map, FCoords dest) const = 0;
   virtual ~CheckStep() {}  // make gcc shut up
};


/*
Some very simple default predicates (more predicates below Map).
*/
struct FindImmovableAlwaysTrue : public FindImmovable {
	virtual bool accept(BaseImmovable *) const {return true;}
   virtual ~FindImmovableAlwaysTrue() {}  // make gcc shut up
};
struct FindBobAlwaysTrue : public FindBob {
	virtual bool accept(Bob *) const {return true;}
   virtual ~FindBobAlwaysTrue() {}  // make gcc shut up
};
struct FindBobAttribute : public FindBob {
	FindBobAttribute(uint attrib) : m_attrib(attrib) { }

	virtual bool accept(Bob *imm) const;

	int m_attrib;
   virtual ~FindBobAttribute() {}  // make gcc shut up
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
class Map {
	friend class Editor_Game_Base;
   friend class Map_Loader;
   friend class S2_Map_Loader;
   friend class Widelands_Map_Loader;
	friend class Widelands_Map_Elemental_Data_Packet;
   friend class Widelands_Map_Extradata_Data_Packet;
   friend class Editor;
   friend class Main_Menu_New_Map;

public:
	enum { // flags for findpath()
		fpBidiCost = 1,		// use bidirection cost instead of normal cost calculations
									// should be used for road building
	};

public:
	struct Pathfield;

   Map();
   ~Map();

   // For overlays
   inline Overlay_Manager* get_overlay_manager() { return m_overlay_manager; }
	Overlay_Manager & get_overlay_manager() const {return *m_overlay_manager;}

   // For loading
   Map_Loader* get_correct_loader(const char*);
   void cleanup(void);

   // for editor
	void create_empty_map
		(const uint w = 64,
		 const uint h = 64,
		 const std::string worldname = std::string("greenland"));

   void load_graphics();
   void recalc_whole_map();
   void recalc_for_field_area(Coords coords, int radius);
   void recalc_default_resources(void);

	void set_nrplayers(const Uint8 nrplayers);

	void set_starting_pos(const uint plnum, const Coords);
	Coords get_starting_pos(const uint plnum) const
	{return m_starting_pos[plnum - 1];}

	void set_filename(const char *string);
	void set_author(const char *string);
	void set_world_name(const char *string);
	void set_name(const char *string);
	void set_description(const char *string);

	// informational functions
	const char * get_filename() const {return m_filename;}
	const char * get_author() const {return m_author;}
	const char * get_name() const {return m_name;}
	const char * get_description() const {return m_description;}
	const char * get_world_name() const {return m_worldname;}
	Uint8 get_nrplayers() const throw () {return m_nrplayers;}
	X_Coordinate get_width   () const throw () {return m_width;}
	Y_Coordinate get_height  () const throw () {return m_height;}
	World & world() const throw () {return *m_world;}
	World * get_world() const {return m_world;}
   // The next few functions are only valid
   // when the map is loaded as an scenario.
   std::string get_scenario_player_tribe(uint i);
   std::string get_scenario_player_name(uint i);
   void set_scenario_player_tribe(uint i, std::string);
   void set_scenario_player_name(uint i, std::string);

	BaseImmovable * get_immovable(const Coords) const;
	uint find_bobs
		(const Coords,
		 const uint radius,
		 std::vector<Bob *> * list,
		 const FindBob & functor = FindBobAlwaysTrue());
	uint find_reachable_bobs
		(const Coords coord,
		 const uint radius,
		 std::vector<Bob *> * list,
		 const CheckStep &,
		 const FindBob & functor = FindBobAlwaysTrue());
	uint find_immovables
		(const Coords coord,
		 const uint radius,
		 std::vector<ImmovableFound> * list,
		 const FindImmovable & = FindImmovableAlwaysTrue());
	uint find_reachable_immovables
		(const Coords,
		 const uint radius,
		 std::vector<ImmovableFound> * list,
		 const CheckStep &,
		 const FindImmovable & = FindImmovableAlwaysTrue());
	uint find_fields
		(const Coords coord,
		 const uint radius,
		 std::vector<Coords> * list,
		 const FindField & functor);
	uint find_reachable_fields
		(const Coords coord,
		 const uint radius,
		 std::vector<Coords>* list,
		 const CheckStep &,
		 const FindField &);

	// Field logic
	typedef uint Index;
	static Index get_index(const Coords c, const X_Coordinate width);
	Index get_index(const FCoords c) {return c.field - m_fields;}
	Index max_index() const {return m_width * m_height;}
	Field & operator[](const Index)  const;
	Field & operator[](const Coords) const;
	Field * get_field(const Index) const;
	Field * get_field(const Coords) const;
	const Field & get_field(const uint x, const uint y) const;
	FCoords get_fcoords(const Coords) const;
	void normalize_coords(Coords *) const;
	FCoords get_fcoords(Field &) const;
	void get_coords(Field & f, Coords & c) const {c = get_fcoords(f);}

	int calc_distance(const Coords, const Coords) const;
	int is_neighbour(const Coords, const Coords) const;

	int calc_cost_estimate(const Coords, const Coords) const;
	int calc_cost(const int slope) const;
	int calc_cost(const Coords, const int dir) const;
	int calc_bidi_cost(const Coords, const int dir) const;
	void calc_cost(const Path &, int * forward, int * backward) const;

	void get_ln (const  Coords,  Coords * const) const;
	void get_ln (const FCoords, FCoords * const) const;
	void get_rn (const  Coords,  Coords * const) const;
	void get_rn (const FCoords, FCoords * const) const;
	void get_tln(const  Coords,  Coords * const) const;
	void get_tln(const FCoords, FCoords * const) const;
	void get_trn(const  Coords,  Coords * const) const;
	void get_trn(const FCoords, FCoords * const) const;
	void get_bln(const  Coords,  Coords * const) const;
	void get_bln(const FCoords, FCoords * const) const;
	void get_brn(const  Coords,  Coords * const) const;
	void get_brn(const FCoords, FCoords * const) const;

	void get_neighbour(const  Coords, const int dir,  Coords * const) const;
	void get_neighbour(const FCoords, const int dir, FCoords * const) const;

	// Field/screen coordinates
	FCoords calc_coords(Point pos);
	void normalize_pix(Point* p);
	int calc_pix_distance(Point a, Point b);
	inline void get_basepix(const Coords fc, int *px, int *py);
	inline void get_pix(const FCoords fc, int *px, int *py);
	inline void get_pix(const Coords c, int *px, int *py);
	inline void get_save_pix(const Coords c, int *px, int *py);

	// Pathfinding
	int findpath
		(Coords instart,
		 Coords inend,
		 const int persist,
		 Path &,
		 const CheckStep &,
		 const uint flags = 0);

	/**
	 * We can reach a field by water either if it has MOVECAPS_SWIM or if it has
	 * MOVECAPS_WALK and at least one of the neighbours has MOVECAPS_SWIM
	 */
	bool can_reach_by_water(const Coords) const;

   // change field heights
	int change_field_height(const Coords, const int delta);
	int set_field_height(const Coords, const int height);

	// change terrain of a field, recalculate buildcaps
   int change_field_terrain(Coords coords, int terrain, bool tdown, bool tright);

   /*
    * Get the a manager for registering or removing
    * something
    */
	const MapVariableManager   & get_mvm () const {return *m_mvm;}
	MapVariableManager         & get_mvm ()       {return *m_mvm;}
	const MapTriggerManager    & get_mtm () const {return *m_mtm;}
	MapTriggerManager          & get_mtm ()       {return *m_mtm;}
	const MapEventManager      & get_mem () const {return *m_mem;}
	MapEventManager            & get_mem ()       {return *m_mem;}
	const MapEventChainManager & get_mecm() const {return *m_mecm;}
	MapEventChainManager       & get_mecm()       {return *m_mecm;}
	const MapObjectiveManager  & get_mom () const {return *m_mom;}
	MapObjectiveManager        & get_mom ()       {return *m_mom;}


private:
	void set_size(const uint w, const uint h);
	void load_world();
	void recalc_border(const FCoords);

	Uint16 m_pathcycle;
	Uint8             m_nrplayers; // # of players this map supports (!= Game's number of players)
	X_Coordinate m_width;
	Y_Coordinate m_height;
	char		m_filename[256];
	char		m_author[61];
	char		m_name[61];
	char		m_description[1024];
	char		m_worldname[1024];
	World*	m_world;				// world type
	Coords*	m_starting_pos;	// players' starting positions

	Field*	m_fields;

	Pathfield*	m_pathfields;
   Overlay_Manager* m_overlay_manager;

   std::vector<std::string>  m_scenario_tribes; // only alloced when really needed
   std::vector<std::string>  m_scenario_names;

	MapVariableManager*   m_mvm;  // The mapvariable manager makes sure for handling all the variables
	MapTriggerManager*    m_mtm;  // The maptrigger manager
	MapEventManager*      m_mem;  // The mapevent manager
	MapEventChainManager* m_mecm; // The mapeventchain manager has a list of all event chains in this map
	MapObjectiveManager*  m_mom;  // The mapobjective manager lists all scenarios objectives.

   struct Extradata_Info {
      enum Type {
         PIC,
      };
      void*       data;
      std::string filename;
      Type        type;
   };
   std::vector<Extradata_Info> m_extradatainfos; // Only for caching of extradata for writing and reading

	void recalc_brightness(FCoords);
	void recalc_fieldcaps_pass1(FCoords);
	void recalc_fieldcaps_pass2(FCoords);
	void check_neighbour_heights(FCoords, int * area);
	void increase_pathcycle();

	template<typename functorT>
		void find_reachable
		(const Coords, const uint radius, const CheckStep &, functorT &);

	template<typename functorT>
		void find_radius(const Coords, const uint radius, functorT &) const;

	Map & operator=(const Map &);
	Map            (const Map &);
};

// FindImmovable functor
struct FindImmovableSize : public FindImmovable {
	FindImmovableSize(int min, int max) : m_min(min), m_max(max) { }
   virtual ~FindImmovableSize() {}  // make gcc shut up

	virtual bool accept(BaseImmovable *imm) const;

	int m_min, m_max;
};
struct FindImmovableType : public FindImmovable {
	FindImmovableType(int type) : m_type(type) { }
   virtual ~FindImmovableType() {}  // make gcc shut up

	virtual bool accept(BaseImmovable *imm) const;

	int m_type;
};
struct FindImmovableAttribute : public FindImmovable {
	FindImmovableAttribute(uint attrib) : m_attrib(attrib) { }
   virtual ~FindImmovableAttribute() {}  // make gcc shut up

	virtual bool accept(BaseImmovable *imm) const;

	int m_attrib;
};
struct FindImmovablePlayerImmovable : public FindImmovable {
	FindImmovablePlayerImmovable() { }
   virtual ~FindImmovablePlayerImmovable() {}  // make gcc shut up

	virtual bool accept(BaseImmovable* imm) const;
};

struct FindFieldCaps : public FindField {
	FindFieldCaps(uchar mincaps) : m_mincaps(mincaps) { }
   virtual ~FindFieldCaps() {}  // make gcc shut up

	virtual bool accept(FCoords coord) const;

	uchar m_mincaps;
};

// Accepts fields if they are accepted by all subfunctors.
struct FindFieldAnd : public FindField {
	FindFieldAnd() { }
	virtual ~FindFieldAnd() { }

	void add(const FindField* findfield, bool negate = false);

	virtual bool accept(FCoords coord) const;

	struct Subfunctor {
		bool					negate;
		const FindField*	findfield;
	};

	std::vector<Subfunctor> m_subfunctors;
};

// Accepts fields based on what can be built there
struct FindFieldSize : public FindField {
	enum Size {
		sizeAny = 0,	// any field not occupied by a robust immovable
		sizeBuild,		// any field we can build on (flag or building)
		sizeSmall,		// at least small size
		sizeMedium,
		sizeBig,
		sizeMine,		// can build a mine on this field
		sizePort,		// can build a port on this field
	};

	FindFieldSize(Size size) : m_size(size) { }
   virtual ~FindFieldSize() {}  // make gcc shut up

	virtual bool accept(FCoords coord) const;

	Size m_size;
};

// Accepts a field for a certain size if it has
// a valid resource and amount on it
struct FindFieldSizeResource : public FindFieldSize {
   FindFieldSizeResource(Size size, int res) : FindFieldSize(size) { m_res=res; }
   virtual ~FindFieldSizeResource() {}  // make gcc shut up

   virtual bool accept(FCoords coord) const;

   int m_res;
};

// Accepts fields based on the size of immovables on the field
struct FindFieldImmovableSize : public FindField {
	enum {
		sizeNone		= (1 << 0),
		sizeSmall	= (1 << 1),
		sizeMedium	= (1 << 2),
		sizeBig		= (1 << 3)
	};

	FindFieldImmovableSize(uint sizes) : m_sizes(sizes) { }
   virtual ~FindFieldImmovableSize() {}  // make gcc shut up

	virtual bool accept(FCoords coord) const;

	uint m_sizes;
};

// Accepts a field if it has an immovable with a given attribute
struct FindFieldImmovableAttribute : public FindField {
	FindFieldImmovableAttribute(uint attrib) : m_attribute(attrib) { }
   virtual ~FindFieldImmovableAttribute() {}  // make gcc shut up

	virtual bool accept(FCoords coord) const;

	uint m_attribute;
};


// Accepts a field if it has the given resource
struct FindFieldResource : public FindField {
	FindFieldResource(uchar res) : m_resource(res) { }
   virtual ~FindFieldResource() {}  // make gcc shut up

	virtual bool accept(FCoords coord) const;

	uchar m_resource;
};


/*
CheckStepDefault
----------------
Implements the default step checking behaviours that should be used for all
normal bobs.

Simply check whether the movecaps are matching (basic exceptions for water bobs
moving onto the shore).
*/
class CheckStepDefault : public CheckStep {
public:
	CheckStepDefault(uchar movecaps) : m_movecaps(movecaps) { }
   virtual ~CheckStepDefault() {}  // make gcc shut up

	virtual bool allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const;
	virtual bool reachabledest(Map* map, FCoords dest) const;

private:
	uchar	m_movecaps;
};


/*
CheckStepWalkOn
---------------
Implements the default step checking behaviours with one exception: we can move
from a walkable field onto an unwalkable one.
If onlyend is true, we can only do this on the final step.
*/
class CheckStepWalkOn : public CheckStep {
public:
	CheckStepWalkOn(uchar movecaps, bool onlyend) : m_movecaps(movecaps), m_onlyend(onlyend) { }
   virtual ~CheckStepWalkOn() {}  // make gcc shut up

	virtual bool allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const;
	virtual bool reachabledest(Map* map, FCoords dest) const;

private:
	uchar	m_movecaps;
	bool	m_onlyend;
};


/*
CheckStepRoad
-------------
Implements the step checking behaviour for road building.

player is the player who is building the road.
movecaps are the capabilities with which the road is to be built (swimming
for boats, walking for normal roads).
forbidden is an array of coordinates that must not be crossed by the road.
*/
class CheckStepRoad : public CheckStep {
public:
	CheckStepRoad(Player* player, uchar movecaps, const std::vector<Coords>* forbidden)
		: m_player(player), m_movecaps(movecaps), m_forbidden(forbidden) { }
   virtual ~CheckStepRoad() {}  // make gcc shut up

	virtual bool allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const;
	virtual bool reachabledest(Map* map, FCoords dest) const;

private:
	Player*							m_player;
	uchar								m_movecaps;
	const std::vector<Coords>*	m_forbidden;
};



/** class Path
 *
 * Represents a cross-country path found by Path::findpath, for example
 */
class CoordPath;

class Path {
	friend class Map;

public:
	Path() { m_map = 0; }
	Path(Map *map, Coords c) : m_map(map), m_start(c), m_end(c) { }
	Path(CoordPath &o);

	void reverse();

	inline Map *get_map() const { return m_map; }

	inline bool is_valid() const { return m_map; }
	inline const Coords &get_start() const { return m_start; }
	inline const Coords &get_end() const { return m_end; }

	inline int get_nsteps() const { return m_path.size(); }
	inline char get_step(int idx) const { return m_path[m_path.size()-idx-1]; }

	void append(int dir);

private:
	Map *m_map;
	Coords m_start;
	Coords m_end;
	std::vector<char> m_path;
};

// CoordPath is an extended path that also caches related Coords
class CoordPath {
public:
	CoordPath() { m_map = 0; }
	CoordPath(Map *map, Coords c) : m_map(map) { m_coords.push_back(c); }
	CoordPath(const Path &path);

	CoordPath& operator=(const Path& path);

	inline Map *get_map() const { return m_map; }
	inline bool is_valid() const { return m_map; }

	inline const Coords &get_start() const { return m_coords.front(); }
	inline const Coords &get_end() const { return m_coords.back(); }
	inline const std::vector<Coords> &get_coords() const { return m_coords; }

	inline int get_nsteps() const { return m_path.size(); }
	inline char get_step(int idx) const { return m_path[idx]; }
	inline const std::vector<char> &get_steps() const { return m_path; }

	int get_index(Coords field) const;

	void reverse();
	void truncate (const std::vector<char>::size_type after);
	void starttrim(const std::vector<char>::size_type before);
	void append(const Path &tail);
	void append(const CoordPath &tail);

private:
	Map *m_map;
	std::vector<char> m_path;			// directions
	std::vector<Coords> m_coords;		// m_coords.size() == m_path.size()+1
};

/*
==============================================================================

Field arithmetics

==============================================================================
*/

inline Map::Index Map::get_index(const Coords c, const X_Coordinate width)
{return c.y * width + c.x;}

inline Field & Map::operator[](const Index i) const {return m_fields[i];}
inline Field & Map::operator[](const Coords c) const
{return operator[](get_index(c, m_width));}

inline Field * Map::get_field(const Index i) const {return &m_fields[i];}

inline Field * Map::get_field(const Coords c) const
{return get_field(get_index(c, m_width));}

inline const Field & Map::get_field(const uint x, const uint y) const
{return m_fields[y * m_width + x];}

inline FCoords Map::get_fcoords(const Coords c) const
{
	return FCoords(c, get_field(c));
}

inline void Map::normalize_coords(Coords * c) const
{
	while (c->x < 0)         c->x += m_width;
	while (c->x >= m_width)  c->x -= m_width;
	while (c->y < 0)         c->y += m_height;
	while (c->y >= m_height) c->y -= m_height;
}


/**
 * Calculate the field coordates from the pointer
 */
inline FCoords Map::get_fcoords(Field & f) const {
	const int i = &f - m_fields;
	return FCoords(i % m_width, i / m_width, &f);
}


/** get_ln, get_rn, get_tln, get_trn, get_bln, get_brn
 *
 * Calculate the coordinates and Field pointer of a neighboring field.
 * Assume input coordinates are valid.
 *
 * Note: Input coordinates are passed as value because we have to allow
 *       usage get_XXn(foo, &foo).
 */
inline void Map::get_ln(const Coords f, Coords * const o) const
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

inline void Map::get_ln(const FCoords f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->y = f.y;
	o->x = f.x-1;
	o->field = f.field-1;
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

inline void Map::get_rn(const Coords f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y;
	o->x = f.x+1;
	if (o->x == m_width) o->x = 0;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_rn(const FCoords f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->y = f.y;
	o->x = f.x+1;
	o->field = f.field+1;
	if (o->x == m_width) {o->x = 0; o->field -= m_width;}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
	assert(m_fields <= o->field);
	assert            (o->field < m_fields + max_index());
}

// top-left: even: -1/-1  odd: 0/-1
inline void Map::get_tln(const Coords f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y-1;
	o->x = f.x;
	if (o->y & 1) {
		if (o->y == -1) o->y = m_height - 1;
		o->x = (o->x ? o->x : m_width) - 1;
	}
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_tln(const FCoords f, FCoords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	assert(m_fields <= f.field);
	assert            (f.field < m_fields + max_index());
	o->y = f.y-1;
	o->x = f.x;
	o->field = f.field - m_width;
	if (o->y & 1) {
		if (o->y == -1) {
			o->y = m_height - 1;
			o->field += max_index();
		}
		o->x--;
		o->field--;
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

// top-right: even: 0/-1  odd: +1/-1
inline void Map::get_trn(const Coords f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->x = f.x;
	if (f.y & 1) {
		(o->x)++;
		if (o->x == m_width) o->x = 0;
	}
	o->y = (f.y ? f.y : m_height) - 1;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_trn(const FCoords f, FCoords * const o) const
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
		o->x++;
		o->field++;
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

// bottom-left: even: -1/+1  odd: 0/+1
inline void Map::get_bln(const Coords f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->y = f.y+1;
	o->x = f.x;
	if (o->y == m_height) o->y = 0;
	if (o->y & 1) o->x = (o->x ? o->x : m_width) - 1;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_bln(const FCoords f, FCoords * const o) const
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
		o->x--;
		o->field--;
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

// bottom-right: even: 0/+1  odd: +1/+1
inline void Map::get_brn(const Coords f, Coords * const o) const
{
	assert(0 <= f.x);
	assert(f.x < m_width);
	assert(0 <= f.y);
	assert(f.y < m_height);
	o->x = f.x;
	if (f.y & 1) {
		(o->x)++;
		if (o->x == m_width) o->x = 0;
	}
	o->y = f.y+1;
	if (o->y == m_height) o->y = 0;
	assert(0 <= o->x);
	assert(o->x < m_width);
	assert(0 <= o->y);
	assert(o->y < m_height);
}

inline void Map::get_brn(const FCoords f, FCoords * const o) const
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
		o->x++;
		o->field++;
		if (o->x == m_width) {
			o->x = 0;
			o->field -= m_width;
		}
	}
	o->y = f.y+1;
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


inline void move_r(const X_Coordinate mapwidth, FCoords & f) {
	assert(f.x < mapwidth);
	++f.x;
	++f.field;
	if (f.x == mapwidth) {f.x = 0; f.field -= mapwidth;}
	assert(f.x < mapwidth);
}

inline void move_r(const X_Coordinate mapwidth, FCoords & f, Map::Index & i) {
	assert(f.x < mapwidth);
	++f.x;
	++f.field;
	++i;
	if (f.x == mapwidth) {f.x = 0; f.field -= mapwidth; i -= mapwidth;}
	assert(f.x < mapwidth);
}


/*
class MapRegion
---------------
Producer/Coroutine class that returns every field within a given radius
around the center point exactly once via next().
Note that the order in which fields are returned is not guarantueed.

next() returns false when no more fields are to be traversed.
*/
class MapRegion {
public:
	MapRegion() { }
	MapRegion(const Map &, const Coords, const Uint16 radius);
	MapRegion(const Map* map, Coords coords, uint radius) { init(map, coords, radius); }
	~MapRegion() { }

	void init(const Map* map, Coords coords, uint radius);
	bool next(FCoords* fc);
	bool next(Coords* c);
	Field* next();

private:
	enum Phase {
		phaseNone,		// not initialized or completed
		phaseUpper,		// upper half
		phaseLower,		// lower half
	};

	const Map*		m_map;
	Phase		m_phase;
	uint		m_radius;		// radius of area
	uint		m_row;			// # of rows completed in this phase
	uint		m_rowwidth;		// # of fields to return per row
	uint		m_rowpos;		// # of fields we have returned in this row

	FCoords	m_left;			// left-most field of current row
	FCoords	m_next;			// next field to return
};

/*
struct MapHollowRegion
---------------
Producer/Coroutine struct that returns every field for which the distance to
the center point is greater than hole_radius and at most radius exactly once
via next(). Note that the order in which fields are returned is not
guarantueed.
*/
struct MapHollowRegion {
	MapHollowRegion
	(Map & map, const Coords center,
	 const unsigned int radius, const unsigned int hole_radius);

	bool next(Coords & c);

private:
	enum Phase {
		None   = 0, // not initialized or completed
		Top    = 1, // above the hole
		Upper  = 2, // upper half
		Lower  = 4, // lower half
		Bottom = 8, // below the hole
	};

	Map & m_map;
	Phase m_phase;
	const unsigned int m_radius;      // radius of the area
	const unsigned int m_hole_radius; // radius of the hole
	const unsigned int m_delta_radius;
	unsigned int m_row; // # of rows completed in this phase
	unsigned int m_rowwidth; // # of fields to return per row
	unsigned int m_rowpos; // # of fields we have returned in this row
	Coords m_left; // left-most field of current row
	Coords m_next; // next field to return
};

/**
 * Producer/Coroutine struct that returns every triangle which can be reached by
 * crossing at most <radius> edges.
 *
 * Each such location is returned exactly once via next(). But this does not
 * guarantee that a location is returned at most once when the radius is so
 * large that the area overlaps itself because of wrapping.
 *
 * Note that the order in which locations are returned is not guarantueed. (But
 * in fact the triangles are returned row by row from top to bottom and from
 * left to right in each row and I see no reason why that would ever change.)
 *
 * The initial coordinates must refer to a triangle (TCoords::D or TCoords::R).
 * Use MapRegion instead for nodes (TCoords::None).
 */
struct MapTriangleRegion {
	MapTriangleRegion(const Map &, TCoords, const unsigned short radius);
	bool next(TCoords & c);
private:
	const Map & m_map;
	const bool m_radius_is_odd;
	enum {Top, Upper, Lower, Bottom} m_phase;
	unsigned short m_remaining_rows_in_upper_phase;
	unsigned short m_remaining_rows_in_lower_phase;
	unsigned short m_row_length, m_remaining_in_row;
	Coords m_left, m_next;
	TCoords::TriangleIndex m_next_triangle;
};

std::string g_MapVariableCallback( std::string str, void* data );
#endif // __S__MAP_H
