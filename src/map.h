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

#define WLMF_VERSION 	0x0001

#define WLMF_SUFFIX		".wmf"
#define S2MF_SUFFIX     ".swd"

#define WLMF_MAGIC      "WLmf"
#define S2MF_MAGIC		"WORLD_V1.0"

#define MAP_NAME_LEN   30
#define MAP_AUTHOR_LEN 30
#define MAP_DESCR_LEN  1024


const ushort NUMBER_OF_MAP_DIMENSIONS=15;
const ushort MAP_DIMENSIONS[] = {
   64, 96, 128, 160, 192, 224, 256,
   288, 320, 352, 384, 416, 448, 480,
   512 };


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
CheckStep

Predicates used in path finding and find functions.
*/
struct FindImmovable {
	// Return true if this immovable should be returned by find_immovables()
	virtual bool accept(BaseImmovable *imm) const = 0;
};
struct FindBob {
	// Return true if this immovable should be returned by find_bobs()
	virtual bool accept(Bob *imm) const = 0;
};
struct FindField {
	// Return true if this immovable should be returned by find_fields()
	virtual bool accept(FCoords coord) const = 0;
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
};


/*
Some very simple default predicates (more predicates below Map).
*/
struct FindImmovableAlwaysTrue : public FindImmovable {
	virtual bool accept(BaseImmovable* imm) const { return true; }
};
struct FindBobAlwaysTrue : public FindBob {
	virtual bool accept(Bob *imm) const { return true; }
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
   
   // For loading
   Map_Loader* get_correct_loader(const char*);
   void cleanup(void);

   // for editor
   void create_empty_map(int w=64, int h=64, std::string worldname = std::string("greenland"));

   void load_graphics();
   void recalc_whole_map();
   void recalc_for_field_area(Coords coords, int radius);
   void recalc_default_resources(void);
   
	void set_nrplayers(uint nrplayers);

	void set_starting_pos(uint plnum, Coords c);
	inline const Coords &get_starting_pos(uint plnum) { return m_starting_pos[plnum-1]; }

	void set_author(const char *string);
	void set_world_name(const char *string);
	void set_name(const char *string);
	void set_description(const char *string);

	// informational functions
	inline const char* get_author(void) { return m_author; }
	inline const char* get_name(void) { return m_name; }
	inline const char* get_description(void) { return m_description; }
	inline const char* get_world_name(void) { return m_worldname; }
	inline ushort get_nrplayers(void) { return m_nrplayers; }
	inline uint get_width(void) { return m_width; }
	inline uint get_height(void) { return m_height; }
	inline World* get_world(void) { return m_world; }

	BaseImmovable *get_immovable(Coords coord);
	uint find_bobs(Coords coord, uint radius, std::vector<Bob*> *list,
								const FindBob &functor = FindBobAlwaysTrue());
	uint find_reachable_bobs(Coords coord, uint radius, std::vector<Bob*> *list,
								const CheckStep* checkstep, const FindBob &functor = FindBobAlwaysTrue());
	uint find_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list,
								const FindImmovable &functor = FindImmovableAlwaysTrue());
	uint find_reachable_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list,
								const CheckStep* checkstep, const FindImmovable &functor = FindImmovableAlwaysTrue());
	uint find_fields(Coords coord, uint radius, std::vector<Coords>* list,
								const FindField& functor);
	uint find_reachable_fields(Coords coord, uint radius, std::vector<Coords>* list,
								const CheckStep* checkstep, const FindField& functor);

	// Field logic
	inline Field* get_field(const Coords c);
	inline FCoords get_fcoords(const Coords c);
	inline void normalize_coords(Coords *c);
	inline void get_coords(Field * const f, Coords *c);

	int calc_distance(Coords a, Coords b);
	int is_neighbour(const Coords start, const Coords end);

	int calc_cost_estimate(Coords a, Coords b);
	int calc_cost(int slope);
	int calc_cost(Coords coords, int dir);
	int calc_bidi_cost(Coords coords, int dir);
	void calc_cost(const Path &path, int *forward, int *backward);

	inline void get_ln(const Coords f, Coords * const o);
	inline void get_ln(const FCoords f, FCoords * const o);
	inline void get_rn(const Coords f, Coords * const o);
	inline void get_rn(const FCoords f, FCoords * const o);
	inline void get_tln(const Coords f, Coords * const o);
	inline void get_tln(const FCoords f, FCoords * const o);
	inline void get_trn(const Coords f, Coords * const o);
	inline void get_trn(const FCoords f, FCoords * const o);
	inline void get_bln(const Coords f, Coords * const o);
	inline void get_bln(const FCoords f, FCoords * const o);
	inline void get_brn(const Coords f, Coords * const o);
	inline void get_brn(const FCoords f, FCoords * const o);

	void get_neighbour(const Coords f, int dir, Coords * const o);
	void get_neighbour(const FCoords f, int dir, FCoords * const o);

	// Field/screen coordinates
	FCoords calc_coords(Point pos);
	void normalize_pix(Point* p);
	int calc_pix_distance(Point a, Point b);
	inline void get_basepix(const Coords fc, int *px, int *py);
	inline void get_pix(const FCoords fc, int *px, int *py);
	inline void get_pix(const Coords c, int *px, int *py);
	inline void get_save_pix(const Coords c, int *px, int *py);

	// Pathfinding
	int findpath(Coords start, Coords end, int persist, Path *path, const CheckStep* checkstep,
						uint flags = 0);

	bool can_reach_by_water(Coords field);

   // change field heights
   int change_field_height(Coords coords, int delta);
   int set_field_height(Coords coords, int height);

	// change terrain of a field, recalculate buildcaps
   int change_field_terrain(Coords coords, int terrain, bool tdown, bool tright);

private:
	void set_size(uint w, uint h);
	void load_world();

	uint		m_nrplayers;		// # of players this map supports (!= Game's number of players)
	uint		m_width;
	uint		m_height;
	char		m_author[61];
	char		m_name[61];
	char		m_description[1024];
	char		m_worldname[1024];
	World*	m_world;				// world type
	Coords*	m_starting_pos;	// players' starting positions

	Field*	m_fields;

	ushort		m_pathcycle;
	Pathfield*	m_pathfields;
   Overlay_Manager* m_overlay_manager;

	void recalc_brightness(FCoords coords);
	void recalc_fieldcaps_pass1(FCoords coords);
	void recalc_fieldcaps_pass2(FCoords coords);
   void check_neighbour_heights(FCoords coords, int* area);
	void increase_pathcycle();

	template<typename functorT>
	void find_reachable(Coords coord, uint radius, const CheckStep* checkstep, functorT& functor);

	template<typename functorT>
	void find_radius(Coords coord, uint radius, functorT& functor);
};

// FindImmovable functor
struct FindImmovableSize : public FindImmovable {
	FindImmovableSize(int min, int max) : m_min(min), m_max(max) { }

	virtual bool accept(BaseImmovable *imm) const;

	int m_min, m_max;
};
struct FindImmovableType : public FindImmovable {
	FindImmovableType(int type) : m_type(type) { }

	virtual bool accept(BaseImmovable *imm) const;

	int m_type;
};
struct FindImmovableAttribute : public FindImmovable {
	FindImmovableAttribute(uint attrib) : m_attrib(attrib) { }

	virtual bool accept(BaseImmovable *imm) const;

	int m_attrib;
};
struct FindImmovablePlayerImmovable : public FindImmovable {
	FindImmovablePlayerImmovable() { }

	virtual bool accept(BaseImmovable* imm) const;
};

struct FindFieldCaps : public FindField {
	FindFieldCaps(uchar mincaps) : m_mincaps(mincaps) { }

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

	virtual bool accept(FCoords coord) const;

	Size m_size;
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

	virtual bool accept(FCoords coord) const;

	uint m_sizes;
};

// Accepts a field if it has an immovable with a given attribute
struct FindFieldImmovableAttribute : public FindField {
	FindFieldImmovableAttribute(uint attrib) : m_attribute(attrib) { }

	virtual bool accept(FCoords coord) const;

	uint m_attribute;
};


// Accepts a field if it has the given resource
struct FindFieldResource : public FindField {
	FindFieldResource(uchar res) : m_resource(res) { }

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
	void truncate(int after);
	void starttrim(int before);
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

inline Field* Map::get_field(const Coords c)
{
	return &m_fields[c.y*m_width + c.x];
}

inline FCoords Map::get_fcoords(const Coords c)
{
	return FCoords(c, get_field(c));
}

inline void Map::normalize_coords(Coords* c)
{
	if (c->x < 0) {
		do { c->x += m_width; } while(c->x < 0);
	} else {
		while(c->x >= (int)m_width) { c->x -= m_width; }
	}

	if (c->y < 0) {
		do { c->y += m_height; } while(c->y < 0);
	} else {
		while(c->y >= (int)m_height) { c->y -= m_height; }
	}
}


/** get_coords
 *
 * Calculate the field coordates from the pointer
 */
inline void Map::get_coords(Field * const f, Coords *c)
{
	int i = f - m_fields;
	c->x = i % m_width;
	c->y = i / m_width;
}


/** get_ln, get_rn, get_tln, get_trn, get_bln, get_brn
 *
 * Calculate the coordinates and Field pointer of a neighboring field.
 * Assume input coordinates are valid.
 *
 * Note: Input coordinates are passed as value because we have to allow
 *       usage get_XXn(foo, &foo).
 */
inline void Map::get_ln(const Coords f, Coords * const o)
{
	o->y = f.y;
	o->x = f.x-1;
	if (o->x < 0) o->x += m_width;
}

inline void Map::get_ln(const FCoords f, FCoords * const o)
{
	o->y = f.y;
	o->x = f.x-1;
	o->field = f.field-1;
	if (o->x < 0) { o->x += m_width; o->field += m_width; }
}

inline void Map::get_rn(const Coords f, Coords * const o)
{
	o->y = f.y;
	o->x = f.x+1;
	if (o->x >= (int)m_width) o->x = 0;
}

inline void Map::get_rn(const FCoords f, FCoords * const o)
{
	o->y = f.y;
	o->x = f.x+1;
	o->field = f.field+1;
	if (o->x >= (int)m_width) { o->x = 0; o->field -= m_width; }
}

// top-left: even: -1/-1  odd: 0/-1
inline void Map::get_tln(const Coords f, Coords * const o)
{
	o->y = f.y-1;
	o->x = f.x;
	if (o->y & 1) {
		if (o->y < 0) { o->y += m_height; }
		(o->x)--;
		if (o->x < 0) { o->x += m_width; }
	}
}

inline void Map::get_tln(const FCoords f, FCoords * const o)
{
	o->y = f.y-1;
	o->x = f.x;
	o->field = f.field - m_width;
	if (o->y & 1) {
		if (o->y < 0) { o->y += m_height; o->field += m_width*m_height; }
		o->x--;
		o->field--;
		if (o->x < 0) { o->x += m_width; o->field += m_width; }
	}
}

// top-right: even: 0/-1  odd: +1/-1
inline void Map::get_trn(const Coords f, Coords * const o)
{
	o->x = f.x;
	if (f.y & 1) {
		(o->x)++;
		if (o->x >= (int)m_width) { o->x = 0; }
	}
	o->y = f.y-1;
	if (o->y < 0) { o->y += m_height; }
}

inline void Map::get_trn(const FCoords f, FCoords * const o)
{
	o->x = f.x;
	o->field = f.field - m_width;
	if (f.y & 1) {
		o->x++;
		o->field++;
		if (o->x >= (int)m_width) { o->x = 0; o->field -= m_width; }
	}
	o->y = f.y - 1;
	if (o->y < 0) { o->y += m_height; o->field += m_width*m_height; }
}

// bottom-left: even: -1/+1  odd: 0/+1
inline void Map::get_bln(const Coords f, Coords * const o)
{
	o->y = f.y+1;
	o->x = f.x;
	if (o->y >= (int)m_height) { o->y = 0; }
	if (o->y & 1) {
		(o->x)--;
		if (o->x < 0) { o->x += m_width; }
	}
}

inline void Map::get_bln(const FCoords f, FCoords * const o)
{
	o->y = f.y + 1;
	o->x = f.x;
	o->field = f.field + m_width;
	if (o->y >= (int)m_height) { o->y = 0; o->field -= m_width*m_height; }
	if (o->y & 1) {
		o->x--;
		o->field--;
		if (o->x < 0) { o->x += m_width; o->field += m_width; }
	}
}

// bottom-right: even: 0/+1  odd: +1/+1
inline void Map::get_brn(const Coords f, Coords * const o)
{
	o->x = f.x;
	if (f.y & 1) {
		(o->x)++;
		if (o->x >= (int)m_width) { o->x = 0; }
	}
	o->y = f.y+1;
	if (o->y >= (int)m_height) { o->y = 0; }
}

inline void Map::get_brn(const FCoords f, FCoords * const o)
{
	o->x = f.x;
	o->field = f.field + m_width;
	if (f.y & 1) {
		o->x++;
		o->field++;
		if (o->x >= (int)m_width) { o->x = 0; o->field -= m_width; }
	}
	o->y = f.y+1;
	if (o->y >= (int)m_height) { o->y = 0; o->field -= m_width*m_height; }
}


/*
===============
Map::get_basepix

Calculate the on-screen position of the field without taking height
into account.
===============
*/
inline void Map::get_basepix(const Coords fc, int *px, int *py)
{
	*py = MULTIPLY_WITH_HALF_FIELD_HEIGHT(fc.y);
	*px = MULTIPLY_WITH_FIELD_WIDTH(fc.x);
	if (fc.y & 1)
		*px += FIELD_WIDTH>>1;
}
// assumes valid fx/fy!
inline void Map::get_pix(const FCoords fc, int *px, int *py)
{
	get_basepix(fc, px, py);
	*py -= MULTIPLY_WITH_HEIGHT_FACTOR(fc.field->get_height());
}
inline void Map::get_pix(const Coords fc, int *px, int *py)
{
	get_pix(get_fcoords(fc), px, py);
}
// fx and fy might be out of range, must be normalized for the field
// theres no need for such a function for FCoords, since x,y out of range
// but field valid doesn't make sense
inline void Map::get_save_pix(const Coords c, int* px, int *py) {
   Coords c1=c;
   normalize_coords(&c1);
   FCoords fc= get_fcoords(c1);
   fc.x=c.x; fc.y=c.y;
   get_pix(fc, px, py);
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
	MapRegion(Map* map, Coords coords, uint radius) { init(map, coords, radius); }
	~MapRegion() { }

	void init(Map* map, Coords coords, uint radius);
	bool next(FCoords* fc);
	bool next(Coords* c);
	Field* next();

private:
	enum Phase {
		phaseNone,		// not initialized or completed
		phaseUpper,		// upper half
		phaseLower,		// lower half
	};

	Map*		m_map;
	Phase		m_phase;
	uint		m_radius;		// radius of area
	uint		m_row;			// # of rows completed in this phase
	uint		m_rowwidth;		// # of fields to return per row
	uint		m_rowpos;		// # of fields we have returned in this row

	FCoords	m_left;			// left-most field of current row
	FCoords	m_next;			// next field to return
};


/*
=============================

class Map_Loader

This class loads a map from a file. It firsts only loads
small junks of informations like size, nr of players for the
map select dialog. For this loading function the same class Map* can be reused.
Then, when the player has a map selected, the Map is completly filled with
objects and information. When now the player selects another map, this class Map*
must be deleted, a new one must be selected

=============================
*/
class Map_Loader {
   public:
      Map_Loader(const char*, Map*) { m_s=STATE_INIT; m_map=0; }
      virtual ~Map_Loader() { };

      virtual int preload_map()=0;
      virtual int load_map_complete(Editor_Game_Base*)=0;

      inline Map* get_map() { return m_map; }

   protected:
      enum State {
         STATE_INIT,
         STATE_PRELOADED,
         STATE_LOADED
      };
      void set_state(State s) { m_s=s; }
      State get_state(void) { return m_s; }
      Map* m_map;

   private:
      State m_s;
};

class S2_Map_Loader : public Map_Loader {
   public:
      S2_Map_Loader(const char*, Map*);
      virtual ~S2_Map_Loader();

      virtual int preload_map();
      virtual int load_map_complete(Editor_Game_Base*);

   private:
      char  m_filename[256];

      uchar *load_s2mf_section(FileRead *, int width, int height);
      void  load_s2mf_header();
      void  load_s2mf(Editor_Game_Base*);
};

#endif // __S__MAP_H
