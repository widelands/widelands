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

#include "widelands.h"
#include "map.h"
#include "myfile.h"

/** class Map
 *
 * This really identifies a map like it is in the game
 *
 * DEPENDS: class File
 * 			class g_fileloc
 */

/** Map::Map(void)
 *
 * inits
 */
Map::Map(void) {
		  w=0;
		  fields=0;
        starting_pos=0;
}

/** Map::~Map(void)
 *
 * cleanups
 */
Map::~Map(void) {
   if(fields) {
      // WARNING: if Field has to free something, we have to do
      // it here manually!!!
      free(fields);
   }

   if(starting_pos) {
      free(starting_pos);
   }

   if(w) delete w;
}

/** void Map::set_size(uint w, uint h)
 *
 * This sets the size of the current map
 *
 * *** PRIAVTE FUNC ****
 *
 * Args:	w, h	size of map
 * Returns: Nothing
 */
void Map::set_size(uint w, uint h) {
   hd.width=w;
   hd.height=h;

   if(fields) {
      fields = (Field*) realloc(fields, sizeof(Field)*hd.height*hd.width);
   }
   else fields = (Field*) malloc(sizeof(Field)*hd.height*hd.width);

}



/** int Map::load_wlmf(const char* file)
 *
 * this loads a given file as a widelands map file
 *
 * ***** PRIVATE FUNC ******
 *
 * Args: 	file		filename to read
 * Returns: RET_OK or RET_FAILED
 */

// TODO: This function is not working!!
int Map::load_wlmf(const char* file, Cmd_Queue* q) {
   Binary_file f;

   f.open(file, File::READ);
   if(f.get_state() != File::OPEN) {
      return ERR_FAILED;
   }

   // read header:
   f.read(&hd, sizeof(hd));

   // check version
   if(WLMF_VERSIONMAJOR(hd.version) > WLMF_VERSIONMAJOR(WLMF_VERSION)) {
      return ERR_FAILED;
   }
   if(WLMF_VERSIONMAJOR(hd.version) == WLMF_VERSIONMAJOR(WLMF_VERSION)) {
      if(WLMF_VERSIONMINOR(hd.version) > WLMF_VERSIONMINOR(WLMF_VERSION)) {
         return ERR_FAILED;
      }
   }

   // ignore the player descriptions, probably the user has chnanged them.
   // as long as the game knows how many players are around, everything is ok
   PlayerDescr pl;
   for(uint i=0; i<hd.nplayers; i++) {
      f.read(&pl, sizeof(pl));
   }

   set_size(hd.width, hd.height);


   // now, read in the fields, one at a time and init the map
   FieldDescr fd;
   uint y;
   Terrain_Descr *td, *tr;
   for(y=0; y<hd.height; y++) {
      for(uint x=0; x<hd.width; x++) {
         f.read(&fd, sizeof(fd));

         // TEMP
         tr=w->get_terrain(fd.tex_r);
         if(!tr) {
            //cerr << "Texture number " << fd.tex_r << " not found in file. Defaults to 0" << endl;
            tr=w->get_terrain(0);
            assert(0); // we should never be here!  
         }
         td=w->get_terrain(fd.tex_d);
         if(!td) {
            // cerr << "Texture number " << fd.tex_d << " not found in file. Defaults to 0" << endl;
            td=w->get_terrain(0);
            assert(0); // we should never be here!  
         }
         // TEMP end

         Field* f=get_field(x,y);
         f->set_height((uchar)fd.height);
         f->set_terrainr(tr);
         f->set_terraind(td);
         f->hook_instance(0); 
      }
   }

   return RET_OK;
}

/** 
 * This functions load a map header, for previewing maps.
 */
int Map::load_map_header(const char* file) {
   int ret=RET_OK;

   if(!strcmp(file+(strlen(file)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
   {
      // It ends like a wide lands map file. try to load
      // it as such
      // TODO: do this
      // ret = load_wlmf(file, q);
   }
   else if(!strcmp(file+(strlen(file)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
   {
      // it is a S2 Map file. load it as such
      ret = load_s2mf_header(file);
   }
   else
   {
      assert(0);
      return ERR_FAILED;
   }

   return ret;
}

/** int Map::load_map(const char* file)
 *
 * This loads a complete map from a file
 *
 * Args: file	filename to read
 * Returns: RET_OK or ERR_FAILED
 */
int Map::load_map(const char* file, Cmd_Queue* q)
{
   int ret;

   if(!strcmp(file+(strlen(file)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX))
   {
      // It ends like a wide lands map file. try to load
      // it as such
      ret = load_wlmf(file, q);
   }
   else if(!strcmp(file+(strlen(file)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX))
   {
      // it is a S2 Map file. load it as such
      ret = load_s2mf(file, q);
   }
   else
   {
      assert(0);
      return ERR_FAILED;
   }

   if (ret != RET_OK)
      return ret;

   // post process the map
   for(uint y=0; y<hd.height; y++)
      for(uint x=0; x<hd.width; x++)
         recalc_brightness(x, y);

   return RET_OK;
}

/** Map::recalc_brightness(int fx, int fy)
 *
 * Fetch the slopes to neighbours and call the actual logic in Field
 */
void Map::recalc_brightness(int fx, int fy)
{
   Field *f, *n;
   int dx, dy;
   int l, r, tl, tr, bl, br;

   f = get_field(fx, fy);

   // left
   get_ln(fx, fy, f, &dx, &dy, &n);
   l = f->get_height() - n->get_height();

   // right
   get_rn(fx, fy, f, &dx, &dy, &n);
   r = f->get_height() - n->get_height();

   // top-left
   get_tln(fx, fy, f, &dx, &dy, &n);
   tl = f->get_height() - n->get_height();

   // top-right
   get_rn(dx, dy, n, &dx, &dy, &n);
   tr = f->get_height() - n->get_height();

   // bottom-left
   get_bln(fx, fy, f, &dx, &dy, &n);
   bl = f->get_height() - n->get_height();

   // bottom-right
   get_rn(dx, dy, n, &dx, &dy, &n);
   br = f->get_height() - n->get_height();

   f->set_brightness(l, r, tl, tr, bl, br);
}
