/*
 * Copyright (C) 2002 by Holger Rapp 
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

#include <stdlib.h>
#include "map.h"
#include "errors.h"
#include "myfile.h"
#include <string.h>

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
int Map::load_wlmf(const char* file) {
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

		  
		  // now, read in the fields, one at a time and init the card
		  FieldDescr fd;
		  int y;
		  Pic *td, *tr;
		  int l, r, t, b;
		  for(y=0; y<hd.height; y++) {
					 for(int x=0; x<hd.width; x++) {
								f.read(&fd, sizeof(fd));

								// TEMP
								tr=w->get_texture(fd.tex_r);
								if(!tr) {
										  //cerr << "Texture number " << fd.tex_r << " not found in file. Defaults to 0" << endl;
										  tr=w->get_texture(0);
								}
								td=w->get_texture(fd.tex_d);
								if(!td) {
										  // cerr << "Texture number " << fd.tex_d << " not found in file. Defaults to 0" << endl;
										  td=w->get_texture(0);
								}
								// TEMP end

								Field* f=get_field(x,y);
								f->set_pos(x,y, fd.height);
								f->set_tr(tr);
								f->set_td(td);

								l=x-1;
								r=x+1;
								t=y-1;
								b=y+1;

								if(!x) l=hd.width-1;
								if(x==hd.width-1) r=0;
								if(!y) t=hd.height-1;
								if(y==hd.height-1) b=0;

								if(y&1) { // %1
										  get_field(x, y)->set_neighb(get_field(l, y), get_field(r, y),
																get_field(x, t),  get_field(r, t),
																get_field(x, b),  get_field(r, b));
								} else {
										  get_field(x, y)->set_neighb(get_field(l, y), get_field(r, y),
																get_field(l, t),  get_field(x, t),
																get_field(l, b),  get_field(x, b));

								}
					 }
		  }


		  return RET_OK;
}

/** int Map::load_map(const char* file)
 *
 * This loads a complete map from a file
 *
 * Args: file	filename to read
 * Returns: RET_OK or ERR_FAILED
 */
int Map::load_map(const char* file) {
		  if(!strcmp(file+(strlen(file)-strlen(WLMF_SUFFIX)), WLMF_SUFFIX)) {
					 // It ends like a wide lands map file. try to load
					 // it as such 
					 return load_wlmf(file);
		  }

		  if(!strcmp(file+(strlen(file)-strlen(S2MF_SUFFIX)), S2MF_SUFFIX)) {
					 // it is a S2 Map file. load it as such
					 return load_s2mf(file);
		  }
		  
		  // Never here, or file is invalid (which can't happen)
		  assert(0);
		  

}
