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
#include "world.h"
//#include "worldfiletypes.h"
#include "bob.h"
#include "myfile.h"
#include "md5file.h"
#include "worlddata.h"

// 
// class World
// 
World::World(void)
{
}

World::~World(void) {

}

// 
// This loads a sane world file
//
int World::load_world(const char* name) {
   MD5_Binary_file f;
   f.open(name, File::READ);
   if(f.get_state() != File::OPEN) return ERR_FAILED;

   int retval;
   
   // read header, skip need list (this is already done)
   if((retval=parse_header(&f))) return retval;

   if((retval=parse_resources(&f))) return retval;

   if((retval=parse_terrains(&f))) return retval;
  
   if((retval=parse_bobs(&f))) return retval;
  
   // checksum check 
   uchar* sum=(uchar*) f.get_chksum();
   uchar  sum_read[16];
   f.read(sum_read, 16);
   uint i;
   for(i=0; i<16; i++) {
      // cerr << hex << (int) sum[i] << ":" << (int) sum_read[i] << endl;
      if(sum[i] != sum_read[i]) return ERR_FAILED; // chksum inval
   }
   
   return RET_OK;
} 

// 
// down here: Private functions for loading
// 

//
//function for loading the header of a worlds file
//skips the provides list also
int World::parse_header(Binary_file* f) {
   char buf[1024];
   
   // read magic
   f->read(buf, 5);
   if(strcasecmp(buf, "WLwf\0")) return ERR_FAILED;

   // read version
   ushort given_vers;
   f->read(&given_vers, sizeof(ushort));
   if(VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLWF_VERSION)) return ERR_WRONGVERSION;
   if(VERSION_MAJOR(given_vers)==VERSION_MAJOR(WLWF_VERSION)) 
      if(VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLWF_VERSION)) return ERR_WRONGVERSION;
   
   // read name, skip author and description
   f->read(hd.name, sizeof(hd.name));
   f->read(hd.author, sizeof(hd.author)); // author
   f->read(hd.descr, 1024); // description

   // skip provides list
   // read magic
   f->read(buf, 13);
   if(strcasecmp(buf, "ProvidesList\0")) return ERR_FAILED;
   ushort nprovides=0;
   f->read(&nprovides, sizeof(ushort));

   uint i;
   ushort is_a;
   for(i=0; i<nprovides; i++) {
      f->read(&is_a, sizeof(ushort));
      f->read(buf, 30);
   }

   return OK;
}

int World::parse_resources(Binary_file* f) {
   char buf[1024];

   f->read(buf, 10);
   if(strcasecmp(buf, "Resources\0")) return ERR_FAILED;

   ushort nres;
   f->read(&nres, sizeof(ushort));

   uint i;
   Resource_Descr* r;
   for(i=0; i<nres; i++) {
      r=new Resource_Descr();
      res.add(r);
      r->read(f);
   }

   return RET_OK;
}

int World::parse_terrains(Binary_file* f) {
   char buf[1024];

   f->read(buf, 9);
   if(strcasecmp(buf, "Terrains\0")) return ERR_FAILED;

   ushort nters;
   f->read(&nters, sizeof(ushort));

   uint i;
   Terrain_Descr* t;
   for(i=0; i<nters; i++) {
      t=new Terrain_Descr();
      ters.add(t);
      t->read(f);
   }

   return RET_OK;
}

int World::parse_bobs(Binary_file* f) {
   char buf[1024];

   f->read(buf, 5);
   if(strcasecmp(buf, "Bobs\0")) return ERR_FAILED;

   ushort nbobs;
   f->read(&nbobs, sizeof(ushort));
   Logic_Bob_Descr* b;
   uchar id;
   uint i;

   bobs.reserve(nbobs);

   for(i=0; i<nbobs; i++) {
      f->read(&id, sizeof(uchar));

      switch(id) {
         case Logic_Bob_Descr::BOB_DIMINISHING:
            b=new Diminishing_Bob_Descr();
            break;
               
         case Logic_Bob_Descr::BOB_BORING:
            b=new Boring_Bob_Descr();
            break;

         case Logic_Bob_Descr::BOB_GROWING:
         case Logic_Bob_Descr::BOB_CRITTER:
         default:
            // illegal bob for world
            assert(0);
            return ERR_FAILED;
            break;
      }
      assert(b);
      bobs.add(b);
      b->read(f);
   }

   return RET_OK;
}

// 
// Down here: subclasses of world
// 

// 
// read a terrain description in
// 
int Terrain_Descr::read(Binary_file* f) {
   // for the moment, we skip a lot of stuff, since it is not yet needed
   f->read(name, 30);
   f->read(&is, sizeof(uchar));
   
   char buf[100];
  
   // skip def resource
//   f->read(&def_res, sizeof(uchar));
//   f->read(&def_stock, sizeof(ushort));
   // skip maxh, minh
//   f->read(&minh, sizeof(uchar));
//   f->read(&maxh, sizeof(uchar));
   f->read(buf, 5);
   
   // skip resources
   uchar nres;
   f->read(&nres, sizeof(uchar));
   f->read(buf, nres*sizeof(uchar));
   
   f->read(&ntex, sizeof(ushort));
   
   char *buf1=(char*) malloc(sizeof(ushort)*TEXTURE_W*TEXTURE_H);
   uint i;
   tex=new Pic*[ntex];
   for(i=0; i<ntex; i++) {
      tex[i]=new Pic();
      f->read(buf1, sizeof(ushort)*TEXTURE_W*TEXTURE_H);
      tex[i]->create(TEXTURE_W, TEXTURE_H, (ushort*)buf1);
   }
   free(buf1);

   return RET_OK;
}

