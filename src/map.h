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

#define WLMF_MAGIC      "WLmf"
#define WLMF_VERSIONMAJOR(a)  (a >> 8)
#define WLMF_VERSIONMINOR(a)  (a & 0xFF)

#include "world.h"

/** struct Map_Header
 *
 * This is the header of a widelands card format
 * it's designed to be fastly read to give the user informations 
 * about the card while browsing it. 
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

/** class Field
 *
 * a field like it is represented in the game
 */
#define FIELD_WIDTH   64
#define FIELD_HEIGHT  64
#define FIELD_SW_W    6   // 1<<FIELD_SW_HEIGHT == FIELD_WIDTH
#define FIELD_SW_H    6
#define HEIGHT_MAX    54 
#define HEIGHT_FACTOR 3
class Field {
		  Field(const Field&);
		  Field& operator=(const Field&);

		  public:
					 Field(ushort, ushort, uchar, Pic*, Pic*);
					 ~Field(void);

					 inline uchar get_height(void) const { return height; }
					 inline int get_xpix(void) const { return xpix; }
					 inline int get_ypix(void) const { return ypix; }

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

					 void set_neighb(Field*, Field*, Field*, Field*, Field*, Field*);
					 
		  private: 
					 Field *ln, *rn, *tln, *trn, *bln, *brn;
					 
					 ushort xpos, ypos;
					 int	xpix, ypix;
					 uchar height;
					 Pic *texr, *texd;
};

/** class Map
 *
 * This really identifies a map like it is in the game
 *
 * Depends: class File
 */
class Map {
		  Map(const Map&);
		  Map& operator=(const Map&);

		  public:
					 Map(void);
					 ~Map(void);

					 int load_map(const char*);

					 // informational functions
					 inline ushort get_w(void) { return width; }
					 inline ushort get_h(void) { return height; }
					 
					 // THEY DON'T CHECK FOR OVERFLOWS!!
					 inline Field* get_field(const uint x, const uint y) {
								lfield=(y)*(width) + (x);
								return fields[lfield];
					 }
					 inline Field* get_nfield(void) {
								return fields[++lfield];
					 }
					 inline Field* get_pfield(void) {
								return fields[--lfield];
					 }
					 inline Field* get_ffield(void) {
								lfield=0;
								return fields[0];
					 }

					 // rewidthinding or forwidtharding widthithout change
					 inline void nfield(void) { ++lfield; }
					 inline void pfield(void) { --lfield; }
					 inline void ffield(void) { lfield=0; }
					 inline void set_cfield(const uint x, const uint y) { lfield=y*width + x; }

					
					 // temp functions
					 void set_world(World* mw) { w=mw; } 

		  
		  private:
					 ushort width, height;
					 World* w;
					 Field** fields;
					 uint lfield;
					 char *name;

};

#endif // __S__MAP_H 
