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

#ifndef __S__MAP_H
#define __S__MAP_H

#define WLMF_VERSION 	0x0001

#define WLMF_SUFFIX		".wmf"
#define S2MF_SUFFIX     ".swd"

#define WLMF_MAGIC      "WLmf"
#define S2MF_MAGIC		"WORLD_V1.0"

#define WLMF_VERSIONMAJOR(a)  (a >> 8)
#define WLMF_VERSIONMINOR(a)  (a & 0xFF)

#include "world.h"
#include "field.h"


/** struct Map_Header
 *
 * This is the header of a widelands map format
 * it's designed to be fastly read to give the user informations
 * about the map while browsing it.
 *   --> no pointers (oh wonder!)
 */
/*struct MapDescrHeader {
		  char   magic[6]; // "WLmf\0\0"
		  char	author[61];
		  char	name[61];
		  char   descr[1024];
		  char 	uses_world[32];
		  ulong	world_checksum;
		  ushort version;
		  ushort nplayers;
		  uint width;
		  uint height;
}*/


class Path;
class Immovable;


/** class Field
 *
 * a field like it is represented in the game
 */

struct ImmovableFound {
	BaseImmovable	*object;
	Coords			coords;
};
struct FindImmovable {
	virtual bool accept(BaseImmovable *imm) const = 0;
};
struct FindBob {
	virtual bool accept(Bob *imm) const = 0;
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
   friend class Editor;

public:
	struct Pathfield;
               
   Map(); 
   ~Map();

   // For loading
   Map_Loader* get_correct_loader(const char*);
	void load_graphics();
   void recalc_whole_map();

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

	bool find_bobs(Coords coord, uint radius, std::vector<Bob*> *list);
	bool find_bobs(Coords coord, uint radius, std::vector<Bob*> *list, const FindBob &functor);
	BaseImmovable *get_immovable(Coords coord);
	bool find_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list);
	bool find_immovables(Coords coord, uint radius, std::vector<ImmovableFound> *list, const FindImmovable &functor);

	// Field logic
	inline Field* get_field(const Coords c);
	inline Field* get_field(const int x, const int y);
	inline FCoords get_fcoords(const Coords c);
	inline void normalize_coords(int *x, int *y);
	inline void normalize_coords(Coords *c);
	inline Field* get_safe_field(int x, int y);
	inline void get_coords(Field * const f, Coords *c);
					
	int calc_distance(Coords a, Coords b);
	int is_neighbour(const Coords start, const Coords end);

	int calc_cost_estimate(Coords a, Coords b);
	int calc_cost(Coords coords, int dir);
	void calc_cost(const Path &path, int *forward, int *backward);

	inline void get_ln(const int fx, const int fy, int *ox, int *oy);
	inline void get_ln(const Coords f, Coords * const o);
	inline void get_ln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
	inline void get_ln(const FCoords f, FCoords * const o);
	inline void get_rn(const int fx, const int fy, int *ox, int *oy);
	inline void get_rn(const Coords f, Coords * const o);
	inline void get_rn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
	inline void get_rn(const FCoords f, FCoords * const o);
	inline void get_tln(const int fx, const int fy, int *ox, int *oy);
	inline void get_tln(const Coords f, Coords * const o);
	inline void get_tln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
	inline void get_tln(const FCoords f, FCoords * const o);
	inline void get_trn(const int fx, const int fy, int *ox, int *oy);
	inline void get_trn(const Coords f, Coords * const o);
	inline void get_trn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
	inline void get_trn(const FCoords f, FCoords * const o);
	inline void get_bln(const int fx, const int fy, int *ox, int *oy);
	inline void get_bln(const Coords f, Coords * const o);
	inline void get_bln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
	inline void get_bln(const FCoords f, FCoords * const o);
	inline void get_brn(const int fx, const int fy, int *ox, int *oy);
	inline void get_brn(const Coords f, Coords * const o);
	inline void get_brn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
	inline void get_brn(const FCoords f, FCoords * const o);

	void get_neighbour(const Coords f, int dir, Coords * const o);
	void get_neighbour(const FCoords f, int dir, FCoords * const o);

	// Field/screen coordinates
	inline void get_basepix(const Coords fc, int *px, int *py);
	inline void get_basepix(const int fx, const int fy, int *px, int *py);
	inline void get_pix(const Coords fc, Field * const f, int *px, int *py);
	inline void get_pix(const int fx, const int fy, Field * const f, int *px, int *py);
	inline void get_pix(const Coords c, int *px, int *py);
	inline void get_pix(const int fx, const int fy, int *px, int *py);

	// Pathfinding
	int findpath(Coords start, Coords end, uchar movecaps, int persist, Path *path,
	             Player *player = 0, bool roadfind = false, const std::vector<Coords> *forbidden = 0);

	bool can_reach_by_water(Coords field);
	
   // change field heights
   void change_field_height(const Coords&, int);
   void change_field_height(int, int, int);
   void set_field_height(const Coords&, int);
   void set_field_height(int, int, int);

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

	//int load_wlmf(const char*, Game*);
	void recalc_brightness(int fx, int fy, Field *f);
	void recalc_fieldcaps_pass1(int fx, int fy, Field *f);
	void recalc_fieldcaps_pass2(int fx, int fy, Field *f);
   void check_neighbour_heights(int fx, int fy, Field* f, int* area);
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
	CoordPath(Map *map, Coords c) : m_map(map) { m_coords.push_back(c); }
	CoordPath(const Path &path);

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

inline Field* Map::get_field(const int x, const int y)
{
	return &m_fields[y*m_width + x];
}

inline FCoords Map::get_fcoords(const Coords c)
{
	return FCoords(c, get_field(c));
}

inline void Map::normalize_coords(int *x, int *y)
{
	if (*x < 0) {
		do { *x += m_width; } while(*x < 0);
	} else {
		while(*x >= (int)m_width) { *x -= m_width; }
	}

	if (*y < 0) {
		do { *y += m_height; } while(*y < 0);
	} else {
		while(*y >= (int)m_height) { *y -= m_height; }
	}
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

inline Field *Map::get_safe_field(int x, int y)
{
	normalize_coords(&x, &y);
	return &m_fields[y*m_width + x];
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
 */
inline void Map::get_ln(const int fx, const int fy, int *ox, int *oy)
{
	*oy = fy;
	*ox = fx-1;
	if (*ox < 0) *ox += m_width;
}

inline void Map::get_ln(const Coords f, Coords * const o)
{
	o->y = f.y;
	o->x = f.x-1;
	if (o->x < 0) o->x += m_width;
}

inline void Map::get_ln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy;
	*ox = fx-1;
	*o = f-1;
	if (*ox < 0) { *ox += m_width; *o += m_width; }
}

inline void Map::get_ln(const FCoords f, FCoords * const o)
{
	o->y = f.y;
	o->x = f.x-1;
	o->field = f.field-1;
	if (o->x < 0) { o->x += m_width; o->field += m_width; }
}

inline void Map::get_rn(const int fx, const int fy, int *ox, int *oy)
{
	*oy = fy;
	*ox = fx+1;
	if (*ox >= (int)m_width) *ox = 0;
}

inline void Map::get_rn(const Coords f, Coords * const o)
{
	o->y = f.y;
	o->x = f.x+1;
	if (o->x >= (int)m_width) o->x = 0;
}

inline void Map::get_rn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy;
	*ox = fx+1;
	*o = f+1;
	if (*ox >= (int)m_width) { *ox = 0; *o -= m_width; }
}

inline void Map::get_rn(const FCoords f, FCoords * const o)
{
	o->y = f.y;
	o->x = f.x+1;
	o->field = f.field+1;
	if (o->x >= (int)m_width) { o->x = 0; o->field -= m_width; }
}

// top-left: even: -1/-1  odd: 0/-1
inline void Map::get_tln(const int fx, const int fy, int *ox, int *oy)
{
	*oy = fy-1;
	*ox = fx;
	if (*oy & 1) {
		if (*oy < 0) { *oy += m_height; }
		(*ox)--;
		if (*ox < 0) { *ox += m_width; }
	}
}

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

inline void Map::get_tln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy-1;
	*ox = fx;
	*o = f-m_width;
	if (*oy & 1) {
		if (*oy < 0) { *oy += m_height; *o += m_width*m_height; }
		(*ox)--;
		(*o)--;
		if (*ox < 0) { *ox += m_width; *o += m_width; }
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
inline void Map::get_trn(const int fx, const int fy, int *ox, int *oy)
{
	*ox = fx;
	if (fy & 1) {
		(*ox)++;
		if (*ox >= (int)m_width) { *ox = 0; }
	}
	*oy = fy-1;
	if (*oy < 0) { *oy += m_height; }
}

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

inline void Map::get_trn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*ox = fx;
	*o = f-m_width;
	if (fy & 1) {
		(*ox)++;
		(*o)++;
		if (*ox >= (int)m_width) { *ox = 0; *o -= m_width; }
	}
	*oy = fy-1;
	if (*oy < 0) { *oy += m_height; *o += m_width*m_height; }
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
inline void Map::get_bln(const int fx, const int fy, int *ox, int *oy)
{
	*oy = fy+1;
	*ox = fx;
	if (*oy >= (int)m_height) { *oy = 0; }
	if (*oy & 1) {
		(*ox)--;
		if (*ox < 0) { *ox += m_width; }
	}
}

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

inline void Map::get_bln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy+1;
	*ox = fx;
	*o = f+m_width;
	if (*oy >= (int)m_height) { *oy = 0; *o -= m_width*m_height; }
	if (*oy & 1) {
		(*ox)--;
		(*o)--;
		if (*ox < 0) { *ox += m_width; *o += m_width; }
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
inline void Map::get_brn(const int fx, const int fy, int *ox, int *oy)
{
	*ox = fx;
	if (fy & 1) {
		(*ox)++;
		if (*ox >= (int)m_width) { *ox = 0; }
	}
	*oy = fy+1;
	if (*oy >= (int)m_height) { *oy = 0; }
}

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

inline void Map::get_brn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*ox = fx;
	*o = f+m_width;
	if (fy & 1) {
		(*ox)++;
		(*o)++;
		if (*ox >= (int)m_width) { *ox = 0; *o -= m_width; }
	}
	*oy = fy+1;
	if (*oy >= (int)m_height) { *oy = 0; *o -= m_width*m_height; }
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


/** Map::get_basepix(const int fx, const int fy, int *px, int *py)
 *
 * Calculate the on-screen position of the field without taking height
 * into account.
 */
inline void Map::get_basepix(const Coords fc, int *px, int *py)
{
	*py = MULTIPLY_WITH_HALF_FIELD_HEIGHT(fc.y);
	*px = MULTIPLY_WITH_FIELD_WIDTH(fc.x);
	if (fc.y & 1)
		*px += FIELD_WIDTH>>1;
}

inline void Map::get_basepix(const int fx, const int fy, int *px, int *py)
{
	get_basepix(Coords(fx, fy), px, py);
}

inline void Map::get_pix(const Coords fc, Field * const f, int *px, int *py)
{
	get_basepix(fc, px, py);
	*py -= MULTIPLY_WITH_HEIGHT_FACTOR(f->get_height());
}

inline void Map::get_pix(const int fx, const int fy, Field * const f, int *px, int *py)
{
	get_pix(Coords(fx, fy), f, px, py);
}

// assumes valid fx/fy!
inline void Map::get_pix(const Coords fc, int *px, int *py)
{
	get_pix(fc, get_field(fc), px, py);
}

inline void Map::get_pix(const int fx, const int fy, int *px, int *py)
{
	get_pix(fx, fy, get_field(fx, fy), px, py);
}

// 
// class Map_Region 
//
// This class is init with a center field and
// a sourrounding. it then returns the next field
// in this region with each call to next()
class Map_Region {
   public:
		Map_Region() { }
      Map_Region(Coords coords, int area, Map* m) { init(coords, area, m); }
      ~Map_Region() { }
		 
		void init(Coords coords, int area, Map *m);      
      Field* next(void);

   private:
      int _area;
      int backwards;
      int sx, sy;
      int cx, cy;
      int tlx, tly, trx, tr_y, blx, bly, brx, bry;
      Field* _lf, *_tl, *_tr, *_bl, *_br;
      Map* _map;
};

// 
// class Map_Region_Coords
//
// This class is init with a center field and
// a sourrounding. it then returns the next field
// in this region with each call to next() by map_Coords
class Map_Region_Coords {
   public:
		Map_Region_Coords() { }
      Map_Region_Coords(Coords coords, int area, Map* m) { init(coords, area, m); }
      ~Map_Region_Coords() { }
		
		void init(Coords coords, int area, Map *m);
		int next(int*, int*);

   private:
      int backwards;
      int _area;
      int sx, sy;
      int cx, cy;
      int tlx, tly, trx, tr_y, blx, bly, brx, bry;
      Field *_tl, *_tr, *_bl, *_br, *_lf;
      Map* _map;
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

      inline Map* get_map() { assert(m_map); return m_map; }
      
   protected:
      bool exists_world(const char*);

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
