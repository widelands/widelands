/*
 * Copyright (C) 2001 by the Widelands Development Team
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
struct MapDescrHeader {
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
} /* size: 1200 bytes */;


/** struct PlayerDescr
 *
 * This includes a player description for the different
 * players that are on a map
 */
struct PlayerDescr {
		  char	sug_name[22];
		  char	uses_tribe[32];
		  ushort	xpos;
		  ushort	ypos;
		  ushort force_tribe;
		  ulong	tribe_checksum;
} /* size: 64 bytes */;

/** struct FieldDescr
 *
 * This describes a field like it is in the
 * Map file! Please note, a real field in
 * the game will have a different class
 */
struct FieldDescr {
		  ushort height;
		  ushort on_island;
		  ushort res_nr;
		  ushort res_count;
		  ushort tex_r;
		  ushort tex_d;
		  ushort animal;
		  ushort bob_nr;
} /* size: 16 bytes */ ;

class Building;
class Creature;
class Bob;

/*
 * this is mainly the same as in graphic.h the 
 * class Point. but this represents coordinates
 * on the map and is a struct, has no overhead
 * for creation and deletion. this will be a
 * good thing, when this class is used in command
 * queues and so on
 */
struct Cords {
   uint x;
   uint y;
};
   
/** class Field
 *
 * a field like it is represented in the game
 */
#define FIELD_WIDTH 58
#define FIELD_HEIGHT 58
//#define FIELD_WIDTH   58
//#define FIELD_HEIGHT  58
#define HEIGHT_FACTOR 5

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
 *
 * Depends: class File
 * 			class g_fileloc
 */
class Map {
		  Map(const Map&);
		  Map& operator=(const Map&);

		  public:
               Map(void);
               ~Map(void);

               int load_map(const char*, Game*);
               int load_map_header(const char*); 

               // informational functions
               inline const char* get_author(void) { return hd.author; }
               inline const char* get_name(void) { return hd.name; }
               inline const char* get_descr(void) { return hd.descr; }
               inline const char* get_world_name(void) { return hd.uses_world; }
               inline const ushort get_version(void) { return hd.version; }
               inline ushort get_nplayers(void) { return hd.nplayers; }
               inline uint get_w(void) { return hd.width; }
               inline uint get_h(void) { return hd.height; }
               inline World* get_world(void) { return w; }
               Field::Build_Symbol get_build_symbol(const int x, const int y);

					inline const Cords* get_starting_pos(int plnum) { return &starting_pos[plnum]; }
					
               // Field logic
               inline Field *get_field(const int x, const int y);
               inline void normalize_coords(int *x, int *y);
               inline Field *get_safe_field(int x, int y);

               inline void get_ln(const int fx, const int fy, int *ox, int *oy);
					inline void get_ln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
					inline void get_rn(const int fx, const int fy, int *ox, int *oy);
					inline void get_rn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
					inline void get_tln(const int fx, const int fy, int *ox, int *oy);
					inline void get_tln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
					inline void get_trn(const int fx, const int fy, int *ox, int *oy);
					inline void get_trn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
					inline void get_bln(const int fx, const int fy, int *ox, int *oy);
					inline void get_bln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);
					inline void get_brn(const int fx, const int fy, int *ox, int *oy);
					inline void get_brn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o);

					// Field/screen coordinates
					inline void get_basepix(const int fx, const int fy, int *px, int *py);
					inline void get_pix(const int fx, const int fy, Field * const f, int *px, int *py);
					inline void get_pix(const int fx, const int fy, int *px, int *py);

        private:
               Cords* starting_pos;
               MapDescrHeader hd;
               World* w;
               Field* fields;

               // funcs
               int load_s2mf(const char*, Game*);
               int load_s2mf_header(const char*);
               uchar *load_s2mf_section(Binary_file *file, int width, int height);

               int load_wlmf(const char*, Game*);
               void set_size(uint, uint);

               void recalc_brightness(int fx, int fy);
};

/*
==============================================================================

Field arithmetics

==============================================================================
*/

inline Field *Map::get_field(const int x, const int y)
{
	return &fields[y*hd.width + x];
}

inline void Map::normalize_coords(int *x, int *y)
{
	if (*x < 0) {
		do { *x += hd.width; } while(*x < 0);
	} else {
		while(*x >= (int)hd.width) { *x -= hd.width; }
	}

	if (*y < 0) {
		do { *y += hd.height; } while(*y < 0);
	} else {
		while(*y >= (int)hd.height) { *y -= hd.height; }
	}
}

inline Field *Map::get_safe_field(int x, int y)
{
	normalize_coords(&x, &y);
	return &fields[y*hd.width + x];
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
	if (*ox < 0) *ox += hd.width;
}

inline void Map::get_ln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy;
	*ox = fx-1;
	*o = f-1;
	if (*ox < 0) { *ox += hd.width; *o += hd.width; }
}

inline void Map::get_rn(const int fx, const int fy, int *ox, int *oy)
{
	*oy = fy;
	*ox = fx+1;
	if (*ox >= (int)hd.width) *ox = 0;
}

inline void Map::get_rn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy;
	*ox = fx+1;
	*o = f+1;
	if (*ox >= (int)hd.width) { *ox = 0; *o -= hd.width; }
}

// top-left: even: -1/-1  odd: 0/-1
inline void Map::get_tln(const int fx, const int fy, int *ox, int *oy)
{
	*oy = fy-1;
	*ox = fx;
	if (*oy & 1) {
		if (*oy < 0) { *oy += hd.height; }
		(*ox)--;
		if (*ox < 0) { *ox += hd.width; }
	}
}

inline void Map::get_tln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy-1;
	*ox = fx;
	*o = f-hd.width;
	if (*oy & 1) {
		if (*oy < 0) { *oy += hd.height; *o += hd.width*hd.height; }
		(*ox)--;
		(*o)--;
		if (*ox < 0) { *ox += hd.width; *o += hd.width; }
	}
}

// top-right: even: 0/-1  odd: +1/-1
inline void Map::get_trn(const int fx, const int fy, int *ox, int *oy)
{
	*ox = fx;
	if (fy & 1) {
		(*ox)++;
		if (*ox >= (int)hd.width) { *ox = 0; }
	}
	*oy = fy-1;
	if (*oy < 0) { *oy += hd.height; }
}

inline void Map::get_trn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*ox = fx;
	*o = f-hd.width;
	if (fy & 1) {
		(*ox)++;
		(*o)++;
		if (*ox >= (int)hd.width) { *ox = 0; *o -= hd.width; }
	}
	*oy = fy-1;
	if (*oy < 0) { *oy += hd.height; *o += hd.width*hd.height; }
}

// bottom-left: even: -1/+1  odd: 0/+1
inline void Map::get_bln(const int fx, const int fy, int *ox, int *oy)
{
	*oy = fy+1;
	*ox = fx;
	if (*oy >= (int)hd.height) { *oy = 0; }
	if (*oy & 1) {
		(*ox)--;
		if (*ox < 0) { *ox += hd.width; }
	}
}

inline void Map::get_bln(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*oy = fy+1;
	*ox = fx;
	*o = f+hd.width;
	if (*oy >= (int)hd.height) { *oy = 0; *o -= hd.width*hd.height; }
	if (*oy & 1) {
		(*ox)--;
		(*o)--;
		if (*ox < 0) { *ox += hd.width; *o += hd.width; }
	}
}

// bottom-right: even: 0/+1  odd: +1/+1
inline void Map::get_brn(const int fx, const int fy, int *ox, int *oy)
{
	*ox = fx;
	if (fy & 1) {
		(*ox)++;
		if (*ox >= (int)hd.width) { *ox = 0; }
	}
	*oy = fy+1;
	if (*oy >= (int)hd.height) { *oy = 0; }
}

inline void Map::get_brn(const int fx, const int fy, Field * const f, int *ox, int *oy, Field **o)
{
	*ox = fx;
	*o = f+hd.width;
	if (fy & 1) {
		(*ox)++;
		(*o)++;
		if (*ox >= (int)hd.width) { *ox = 0; *o -= hd.width; }
	}
	*oy = fy+1;
	if (*oy >= (int)hd.height) { *oy = 0; *o -= hd.width*hd.height; }
}


/** Map::get_basepix(const int fx, const int fy, int *px, int *py)
 *
 * Calculate the on-screen position of the field without taking height
 * into account.
 */
inline void Map::get_basepix(const int fx, const int fy, int *px, int *py)
{
	*py = fy * (FIELD_HEIGHT>>1);
	*px = fx * FIELD_WIDTH;
	if (fy & 1)
		*px += FIELD_WIDTH>>1;
}

inline void Map::get_pix(const int fx, const int fy, Field * const f, int *px, int *py)
{
	get_basepix(fx, fy, px, py);
	*py -= f->get_height() * HEIGHT_FACTOR;
}

// assumes valid fx/fy!
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
      Map_Region(const int x, const int y, int area, Map* m) {
         backwards=0;
         _area=area;
         _map=m;
         _lf=m->get_safe_field(x, y);
         _tl=_tr=_bl=_br=_lf;
         tlx=trx=blx=brx=x;
         tly=tr_y=bly=bry=y;
         sx=x; sy=y;
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
      
      ~Map_Region() {
      }
      
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
// class Map_Region_Cords
//
// This class is init with a center field and
// a sourrounding. it then returns the next field
// in this region with each call to next() by map_cords
class Map_Region_Cords {
   public:
      Map_Region_Cords(const int x, const int y, int area, Map* m) {
         backwards=0;
         _area=area;
         _map=m;
         _lf=m->get_safe_field(x, y);
         _tl=_tr=_bl=_br=_lf;
         tlx=trx=blx=brx=x;
         tly=tr_y=bly=bry=y;
         sx=x; sy=y;
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
      
      ~Map_Region_Cords() {
      }
      
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


#endif // __S__MAP_H
