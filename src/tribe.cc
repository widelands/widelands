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
#include "graphic.h"
#include "descr_maintainer.h"
#include "pic.h"
#include "bob.h"
#include "md5.h"
#include "tribedata.h"
#include "ware.h"
#include "worker.h"
#include "tribe.h"

//
// Tribe_Descr class
// 
Tribe_Descr::Tribe_Descr(void) {
}
Tribe_Descr::~Tribe_Descr(void) {
}

void Tribe_Descr::load(const char* name)
{
	try
	{
		char filename[256];

		snprintf(filename, sizeof(filename), "tribes/%s.wtf", name);

		FileRead f;
		f.Open(g_fs, filename);

		ChkSum chksum;
		chksum.pass_data(f.Data(0, 0), f.GetSize()-16);
		chksum.finish_chksum();

		// parse the parts of the tribes file one by one
		parse_header(&f);
		parse_regent(&f);
		parse_bobs(&f);
		parse_wares(&f);
		parse_soldiers(&f);
		parse_workers(&f);
		parse_buildings(&f);

		// read science data!
		// not yet

		// checksum check 
		uchar *sum=(uchar*) chksum.get_chksum();
		uchar *sum_read = (uchar*)f.Data(16);
		
		if (memcmp(sum, sum_read, 16))
			throw wexception("Checksum failed");
	}
	catch(std::exception &e)
	{
		throw wexception("Error loading tribe %s: %s", name, e.what());
	}
}

//
// down here: private read functions for loading
// 
void Tribe_Descr::parse_buildings(FileRead* f)
{
   // read magic
   if(strcasecmp(f->CString(), "Buildings")) 
		throw wexception("Wrong buildings magic");

   ushort nbuilds;
	nbuilds = f->Unsigned16();

   uint i;
   uchar id;
   Building_Descr* b;
   for(i=0; i<nbuilds; i++) {
		id = f->Unsigned8();
      switch(id) {
         case SIT:       			
            b=new Sit_Building_Descr();
            break;
         case SIT_PRODU_WORKER:
            b=new Sit_Building_Produ_Worker_Descr();
            break;
         case DIG:	        
            b=new Dig_Building_Descr();
            break;
         case SEARCH:			     
            b=new Search_Building_Descr();
            break;
         case GROW:			    
            b=new Grow_Building_Descr();
            break;
         case PLANT:		   
            b=new Plant_Building_Descr();
            break;
         case SCIENCE:		
            b=new Science_Building_Descr();
            break;
         case MILITARY:	
            b=new Military_Building_Descr();
            break;
         case CANNON:		
            b=new Cannon_Descr();
            break;
         case SPEC_HQ:	
            b=new HQ_Descr();
            break;
         case SPEC_STORE:	
            b=new Store_Descr();
            break;
         case SPEC_PORT:
            b=new Port_Descr();
            break;
         case SPEC_DOCKYARD:
            b=new Dockyard_Descr();
            break;
         default:   
            b=0;
            assert(0); // never here!
            break;
      }
      buildings.add(b);
      b->read(f);
   }
}

void Tribe_Descr::parse_workers(FileRead* f)
{
   if(strcasecmp(f->CString(), "Workers"))
		throw wexception("Wrong workers magic");

   ushort nworkers;
   nworkers = f->Unsigned16();

   uint i;
   uchar id;
   Worker_Descr* w;
   for(i=0; i<nworkers; i++) {
		id = f->Unsigned8();
      switch(id) {
         case SITDIG:
            w=new SitDigger();
            break;
         case SEARCHER: 
            w=new Searcher();
            break;
         case GROWER: 
            w=new Grower();
            break;
         case PLANTER:
            w=new Planter();
            break;
         case SCIENTIST: 
            w=new Scientist();
            break;
         case SPEC_DEF_CARRIER:
            w=new Def_Carrier();
            break;
         case SPEC_ADD_CARRIER: 
            w=new Add_Carrier();
            break;
         case SPEC_BUILDER:
            w=new Builder();
            break; 
         case SPEC_PLANER: 
            w=new Planer();
            break;
         case SPEC_EXPLORER:
            w=new Explorer();
            break;
         case SPEC_GEOLOGIST:
            w=new Geologist();
            break;
         default:
            w=0;
            assert(0); // never here!
            break;
      }
      workers.add(w);
      w->read(f);
   }
}

void Tribe_Descr::parse_soldiers(FileRead* f)
{
   // read magic
   if(strcasecmp(f->CString(), "Soldiers"))
		throw wexception("Wrong soldiers magic");

   ushort nitems;
   nitems = f->Unsigned16();
   uint i;
   uchar id;
   Soldier_Descr* sol;
   int retval;
   for(i=0; i<nitems; i++) {
      id = f->Unsigned8();
      assert(id==SOLDIER);

      sol=new Soldier_Descr;
      if((retval=sol->read(f)))
			throw wexception("Error reading soldier description");

      soldiers.add(sol);
   }
}

void Tribe_Descr::parse_wares(FileRead* f)
{
   // read magic
   if(strcasecmp(f->CString(), "Wares"))
		throw wexception("Wrong wares magic");

   ushort nware;
   nware = f->Unsigned16();;
   
   Ware_Descr* ware;
	ushort *ptr;
   ushort w, h, clrkey;
   uint i;
   for(i=0; i<nware; i++) {
      ware=new Ware_Descr;
      memcpy(ware->name, f->Data(sizeof(ware->name)), sizeof(ware->name));
      w = f->Unsigned16();
      h = f->Unsigned16();
      clrkey = f->Unsigned16();

		ptr = (ushort*)f->Data(24*24*2);
      ware->menu_pic.create(24, 24, ptr);
		
      ptr = (ushort*)f->Data(w*h*sizeof(ushort));
      ware->pic.create(w, h, ptr);
      ware->menu_pic.set_clrkey(clrkey);
      ware->pic.set_clrkey(clrkey);
      wares.add(ware);
   }
}

void Tribe_Descr::parse_bobs(FileRead* f)
{
   // TODO: for the moment, this data (since this really belongs to the game or the map) is completly ignored
   
   // read magic
   if(strcasecmp(f->CString(), "Bobs"))
		throw wexception("Wrong bobs magic");
	
   ushort nitems;
   nitems = f->Unsigned16();

   uint i;
   ushort npics;
   ushort size;
   for(i=0; i< nitems; i++) {
      // 1 byte type
      f->Unsigned8();
      // 30 name
      f->Data(30);
      
      // 4 dimensions
      // 4 hot_spot
      f->Data(8);
      npics = f->Unsigned16();
      uint j;
      for(j=0; j<npics; j++) {
			size = f->Unsigned16();
			f->Data(size);
      }
      // 
      // 32 bytes garbage (ONLY AT THE MOMENT!)
      f->Data(32);
   }
}

//
// this function reads out the header, skips some fields
// (like descr, since they just burn memory in game) 
// and loads others
void Tribe_Descr::parse_header(FileRead* f)
{
   // read magic
   if(strcasecmp(f->CString(), "WLtf"))
		throw wexception("Bad WLtf magic");

   // read version
   ushort given_vers;
   given_vers = f->Unsigned16();
   if(VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLTF_VERSION) ||
      (VERSION_MAJOR(given_vers)==VERSION_MAJOR(WLTF_VERSION) &&
       VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLTF_VERSION)))
		throw wexception("Unsupported version");
   
   // read name, skip author and description
   memcpy(name, f->Data(sizeof(name)), sizeof(name));
   f->Data(sizeof(name)); // author
   f->Data(1024); // description

   // read frontier bob
   // TODO: does this belong into the header (before the need list??!!)
   // will be fixed, when ships and other mandatory tribe bobs are introduced
   frontier.read(f);
   frontier.set_dimensions(FRONTIER_W, FRONTIER_H);
   frontier.set_hotspot(FRONTIER_HSX, FRONTIER_HSY);

   // skip need list
   // read magic
   if(strcasecmp(f->CString(), "NeedList"))
		throw wexception("Wrong NeedList magic");
	
   ushort nneeds=0;
   nneeds = f->Unsigned16();

   uint i;
   ushort is_a;
   for(i=0; i<nneeds; i++) {
      is_a = f->Unsigned16();
      f->Data(30);
   }
}

//
// this reads out the regent data
//
void Tribe_Descr::parse_regent(FileRead* f)
{
   // read magic
   if(strcasecmp(f->CString(), "Regent"))
		throw wexception("Wrong regent magic");
   
   // we ignore all of the stuff. this is already in the player description
   f->Data(30); // name
   f->Data(6300);// small pic
   f->Data(14000); // big pic
}


