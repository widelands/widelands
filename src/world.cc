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
#include "bob.h"
#include "worlddata.h"
#include "md5.h"

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
void World::load_world(const char* name)
{
	char filename[256];

	try
	{	
		snprintf(filename, sizeof(filename), "worlds/%s.wwf", name);

		FileRead f;
		f.Open(g_fs, filename);

		ChkSum chksum;
		chksum.pass_data(f.Data(0,0), f.GetSize()-16); // don't chksum the chksum ;)
		chksum.finish_chksum();

		// read header, skip need list (this is already done)
		parse_header(&f);
		parse_resources(&f);
		parse_terrains(&f);
		parse_bobs(&f);

		// checksum check 
		uchar *sum=(uchar*)chksum.get_chksum();
		uchar *sum_read = (uchar*)f.Data(16);

		if (memcmp(sum, sum_read, 16))
			throw wexception("Checksum failed");
	}
	catch(std::exception &e)
	{
		// tag with world name
		throw wexception("Error loading world %s: %s", name, e.what());
	}
} 

// 
// down here: Private functions for loading
// 

//
//function for loading the header of a worlds file
//skips the provides list also
void World::parse_header(FileRead* f)
{
   // read magic
   if(strcasecmp(f->CString(), "WLwf"))
		throw wexception("Wrong header magic");

   // read version
   ushort given_vers;
   given_vers = f->Unsigned16();
   if(VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLWF_VERSION) ||
      (VERSION_MAJOR(given_vers)==VERSION_MAJOR(WLWF_VERSION) &&
       VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLWF_VERSION)))
		throw wexception("Unsupported version");
   
   // read name, skip author and description
   memcpy(hd.name, f->Data(sizeof(hd.name)), sizeof(hd.name));
   memcpy(hd.author, f->Data(sizeof(hd.author)), sizeof(hd.author)); // author
   memcpy(hd.descr, f->Data(1024), 1024); // description

   // skip provides list
   // read magic
   if(strcasecmp(f->CString(), "ProvidesList"))
		throw wexception("Wrong ProvidesList magic");
	
   ushort nprovides;
   nprovides = f->Unsigned16();

   uint i;
   ushort is_a;
   for(i=0; i<nprovides; i++) {
      is_a = f->Unsigned16();
      f->Data(30);
   }
}

void World::parse_resources(FileRead* f)
{
   if(strcasecmp(f->CString(), "Resources"))
		throw wexception("Wrong resources magic");

   ushort nres;
   nres = f->Unsigned16();

   uint i;
   Resource_Descr* r;
   for(i=0; i<nres; i++) {
      r=new Resource_Descr();
      res.add(r);
      r->read(f);
   }
}

void World::parse_terrains(FileRead* f)
{
   if(strcasecmp(f->CString(), "Terrains"))
		throw wexception("Wrong terrains magic");

   ushort nters;
   nters = f->Unsigned16();

   uint i;
   Terrain_Descr* t;
   for(i=0; i<nters; i++) {
      t=new Terrain_Descr();
      ters.add(t);
      t->read(f);
   }
}

void World::parse_bobs(FileRead* f)
{
   if(strcasecmp(f->CString(), "Bobs"))
		throw wexception("Wrong bobs magic");

   ushort nbobs;
   nbobs = f->Unsigned16();
	
   Logic_Bob_Descr* b;
   uchar id;
   uint i;

   bobs.reserve(nbobs);

   for(i=0; i<nbobs; i++) {
      id = f->Unsigned8();

      switch(id) {
         case Logic_Bob_Descr::BOB_DIMINISHING:
            b=new Diminishing_Bob_Descr();
            break;
               
         case Logic_Bob_Descr::BOB_BORING:
            b=new Boring_Bob_Descr();
            break;
         
         case Logic_Bob_Descr::BOB_CRITTER:
            b=new Critter_Bob_Descr();
            break;

         case Logic_Bob_Descr::BOB_GROWING:
         default:
				throw wexception("Unsupported BOB_GROWING");
      }
      assert(b);
      bobs.add(b);
      b->read(f);
   }
}

// 
// Down here: subclasses of world
// 

// 
// read a terrain description in
// 
void Terrain_Descr::read(FileRead* f)
{
   // for the moment, we skip a lot of stuff, since it is not yet needed
   memcpy(name, f->Data(30), 30);
   is = f->Unsigned8();
   
   // skip def resource
//   f->read(&def_res, sizeof(uchar));
//   f->read(&def_stock, sizeof(ushort));
   // skip maxh, minh
//   f->read(&minh, sizeof(uchar));
//   f->read(&maxh, sizeof(uchar));
   f->Data(5);
   
   // skip resources
   uchar nres;
   nres = f->Unsigned8();
   f->Data(nres);
   
   ntex = f->Unsigned16();
   
   uint i;
   tex=new Pic*[ntex];
   for(i=0; i<ntex; i++) {
      tex[i]=new Pic();
      
		ushort *ptr = (ushort*)f->Data(sizeof(ushort)*TEXTURE_W*TEXTURE_H);
      tex[i]->create(TEXTURE_W, TEXTURE_H, ptr);
   }
}

