/*
 * Copyright (C) 2001 by Holger Rapp
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

#include "mytypes.h"


#define WLMF_VERSION 	0x0001

#define WLMF_SUFFIX		".wmf"
#define S2MF_SUFFIX     ".swd"

#define WLMF_MAGIC      "WLmf"
#define S2MF_MAGIC		"WORLD_V1.0"

#define WLMF_VERSIONMAJOR(a)  (a >> 8)
#define WLMF_VERSIONMINOR(a)  (a & 0xFF)

#include "world.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

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
		  ushort	width;
		  ushort	height;
		  ushort	nplayers;
} /* size: 1196 bytes */;


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

#ifdef _MSC_VER
#pragma pack(pop)
#else
#pragma pack()
#endif

class Building;
class Creature;
class Bob;

/** class Field
 *
 * a field like it is represented in the game
 */
#define FIELD_WIDTH   58
#define FIELD_HEIGHT  58
//#define FIELD_HEIGHT 48
//#define FIELD_WIDTH 48
#define HEIGHT_MAX    58
#define HEIGHT_FACTOR 6
class Field {
		  Field(const Field&);
		  Field& operator=(const Field&);

		  public:
					 Field();
					 ~Field(void);

					 // setting funcs
					 inline void set_pos(ushort x, ushort y, uchar h) {
								xpos=x;
								ypos=y;
								height=h;

								ypix=(((y+1)*FIELD_HEIGHT)>>1)-(h*HEIGHT_FACTOR);

								if((y&1)) { // %2
										  xpix=((((x<<1)+1)*FIELD_WIDTH)>>1);
								} else {
										  xpix=(x*FIELD_WIDTH);
								}
					 }
					 inline void set_td(Pic* p) {
								texd=p;
					 }
					 inline void set_tr(Pic* p) {
								texr=p;
					 }
					 void set_neighb(Field *, Field *, Field *, Field *, Field *, Field *);

					 // Getting funcs
					 inline uchar get_height(void) const { return height; }
					 inline int get_xpix(void) const { return xpix; }
					 inline int get_ypix(void) const { return ypix; }

					 inline int get_xpos() const { return xpos; }
					 inline int get_ypos() const { return ypos; }

					 // Get neigbor fields
					 inline Field* get_ln(void) const { return ln; }
					 inline Field* get_rn(void) const { return rn; }
					 inline Field* get_tln(void) const { return tln; }
					 inline Field* get_trn(void) const { return trn; }
					 inline Field* get_bln(void) const { return bln; }
					 inline Field* get_brn(void) const { return brn; }

					 // get textures
					 inline Pic* get_texr(void) { return texr; }
					 inline Pic* get_texd(void) { return texd; }

					 inline char get_brightness() const { return brightness; }
					 void set_brightness();

					 // ----- for class creature
					 void remove_creature(Creature*) {};
					 // -----

		  private:
					 Building* building;
					 Bob* bob;
					 Field *ln, *rn, *tln, *trn, *bln, *brn;

					 ushort xpos, ypos;
					 int	xpix, ypix;
					 uchar height;
					 char brightness;
					 Pic *texr, *texd;
};

/** class Map
 *
 * This really identifies a map like it is in the game
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

					 int load_map(const char*);

					 // informational functions
					 inline const char* get_author(void) { return hd.author; }
					 inline const char* get_name(void) { return hd.name; }
					 inline const char* get_descr(void) { return hd.descr; }
					 inline const char* get_world(void) { return hd.uses_world; }
					 inline const ushort get_version(void) { return hd.version; }
					 inline ushort get_nplayers(void) { return hd.nplayers; }
					 inline ushort get_w(void) { return hd.width; }
					 inline ushort get_h(void) { return hd.height; }

					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline Field* get_field(const uint x, const uint y) {
								lfield=(y)*(hd.width) + (x);
								return &fields[lfield];
					 }
					 inline Field* get_nfield(void) {
								return &fields[++lfield];
					 }
					 inline Field* get_pfield(void) {
								return &fields[--lfield];
					 }
					 inline Field* get_ffield(void) {
								lfield=0;
								return &fields[0];
					 }

					 // rewinding or forwarding widthithout change
					 inline void nfield(void) { ++lfield; }
					 inline void pfield(void) { --lfield; }
					 inline void ffield(void) { lfield=0; }
					 inline void set_cfield(const uint x, const uint y) { lfield=y*hd.width + x; }

		  private:
					 MapDescrHeader hd;
					 World* w;
					 Field* fields;
					 uint lfield;

					 // funcs
					 int load_s2mf(const char*);
					 int load_wlmf(const char*);
					 void set_size(uint, uint);
};

#endif // __S__MAP_H
