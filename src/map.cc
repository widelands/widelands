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

/** class Map
 *
 * This really identifies a map like it is in the game
 *
 * DEPENDS: class File
 */

/** Map::Map(void) 
 *
 * inits
 */
Map::Map(void) {
		  name=0;
		  fields=0;
}

/** Map::~Map(void) 
 *
 * cleanups
 */
Map::~Map(void) {
		  if(name) 
					 delete[] name;

		  if(fields) {
					 for(uint y=0; y<height; y++) {
								for(uint x=0; x<width; x++) {
										  delete fields[y*width + x];
								}
					 }
					 free(fields);
		  }
}


/** int Map::load_map(const char* file)
 *
 * This loads a complete map from a file
 *
 * Args: file	filename to read
 * Returns: RET_OK or ERR_FAILED
 */
int Map::load_map(const char* file) {
		  Binary_file f;

		  f.open(file, File::READ);
		  if(f.get_state() != File::OPEN) {
					 return ERR_FAILED;
		  }

		  // read header:
		  MapDescrHeader hd;
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

		  // copy name
		  if(name) delete[] name;
		  name= new char[strlen(hd.name)+1]; 
		  strcpy(name, hd.name);

		  // ignore the player descriptions, probably the user has chnanged them. 
		  // as long as the game knows how many players are around, everything is ok
		  PlayerDescr pl;
		  for(uint i=0; i<hd.nplayers; i++) {
					 f.read(&pl, sizeof(pl));
		  }
		  
		  // set size
		  height=hd.height;
		  width=hd.width;
		  if(fields) {
					 for(uint y=0; y<height; y++) {
								for(uint x=0; x<width; x++) {
										  delete fields[y*width + x];
								}
					 }
					 free(fields);
		  }
		  fields = (Field**) malloc(sizeof(Field*)*height*width);

		  // now, read in the fields, one at a time and init the card
		  FieldDescr fd;
		  for(int y=0; y<height; y++) {
					 for(int x=0; x<width; x++) {
								f.read(&fd, sizeof(fd));
	
								fields[y*width + x ] = new Field(x, y, fd.height);
							 			// TODO care for the real fields;
					 }
		  }
		  
		  int l, r, t, b;
		  for(int y=0; y<height; y++) {
					 for(int x=0; x<width; x++) {

		 						l=x-1; 
								r=x+1;
								t=y-1; 
								b=y+1;
								
								if(!x) l=width-1;
								if(x==width-1) r=0;
								if(!y) t=height-1;
								if(y==height-1) b=0;

								if(y&1) { // %1
										  fields[y*width + x]->set_neighb(fields[y*width + l], fields[y*width + r], 
																fields[t*width + x],  fields[t*width + r],
																 fields[b*width + x],  fields[b*width + r]);
								} else {
										  fields[y*width + x]->set_neighb(fields[y*width + l], fields[y*width + r],
																 fields[t*width + l],  fields[t*width + x], 
																 fields[b*width + l],  fields[b*width + x]);
										  
								}
					 }
		  }
 
		  return RET_OK;
}
