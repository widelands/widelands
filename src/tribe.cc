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

#include <string.h>
#include "widelands.h"
#include "myfile.h"
#include "graphic.h"
#include "descr_maintainer.h"
#include "pic.h"
#include "bob.h"
#include "md5file.h"
#include "tribedata.h"
#include "ware.h"
#include "worker.h"
#include "building.h"
#include "tribe.h"

//
// this is a function which reads a animation from a file
//
int read_anim(Animation* a, Binary_file* f) {
   ushort npics;
   f->read(&npics, sizeof(ushort));

   uint i;
   char* buf;
   ushort buf_size=15000;
   buf=(char*) malloc(buf_size);
   
   ushort size;
   for(i=0; i<npics; i++) {
      f->read(&size, sizeof(ushort));
      if(size > buf_size) {
         buf_size=size;
         buf=(char*) realloc(buf, buf_size);
      }
      f->read(buf, size);
      a->add_pic(size, (ushort*) buf);
   }
  
   free(buf);
   
   return RET_OK;
}

//
// Tribe_Descr class
// 
Tribe_Descr::Tribe_Descr(void) {
}
Tribe_Descr::~Tribe_Descr(void) {
}

int Tribe_Descr::load(const char* name) {
   MD5_Binary_file f;
   f.open(name, File::READ);
   if(f.get_state() != File::OPEN) return ERR_FAILED;

   int retval;
   
   // read header, skip need list (this is already done)
   if((retval=parse_header(&f))) return retval;

   // read regent data
   if((retval=parse_regent(&f))) return retval;

   // read bob data
   if((retval=parse_bobs(&f))) return retval;
   
   // read wares data
   if((retval=parse_wares(&f))) return retval;
   
   // read soldiers data
   if((retval=parse_soldiers(&f))) return retval;

   // read workers data
   if((retval=parse_workers(&f))) return retval;

   // read buildings data
   if((retval=parse_buildings(&f))) return retval;
  
   // read science data!
   // not yet

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
// down here: private read functions for loading
// 
int Tribe_Descr::parse_buildings(Binary_file* f) {
   char buf[1024];

   // read magic
   f->read(buf, 10);
   if(strcasecmp(buf, "Buildings\0")) return ERR_FAILED;

   ushort nbuilds;
   f->read(&nbuilds, sizeof(ushort));

   uint i;
   uchar id;
   Building_Descr* b;
   for(i=0; i<nbuilds; i++) {
      f->read(&id, sizeof(uchar));
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

   return RET_OK;
}

int Tribe_Descr::parse_workers(Binary_file* f) {
   char buf[1024];

   f->read(buf, 8);
   if(strcasecmp(buf, "Workers\0")) return ERR_FAILED;

   ushort nworkers;
   f->read(&nworkers, sizeof(ushort));

   uint i;
   uchar id;
   Worker_Descr* w;
   for(i=0; i<nworkers; i++) {
      f->read(&id, sizeof(uchar));
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

   return RET_OK;
}

int Tribe_Descr::parse_soldiers(Binary_file* f) {
   char buf[1024];

   // read magic
   f->read(buf, 9);
   if(strcasecmp("Soldiers\0", buf)) return ERR_FAILED;

   ushort nitems;
   f->read(&nitems, sizeof(ushort));
   uint i;
   uchar id;
   Soldier_Descr* sol;
   int retval;
   for(i=0; i<nitems; i++) {
      f->read(&id, sizeof(uchar));
      assert(id==SOLDIER);

      sol=new Soldier_Descr;
      if((retval=sol->read(f))) return retval;

      soldiers.add(sol);
   }

   return RET_OK;
}

int Tribe_Descr::parse_wares(Binary_file* f) {
   char *buf= (char*) malloc(1152);
   ushort buf_size=1152;
   
   // read magic
   f->read(buf, 6);
   if(strcasecmp(buf, "Wares\0")) {
      free(buf); 
      return ERR_FAILED;
   }

   ushort nware;
   f->read(&nware, sizeof(ushort));
   
   Ware_Descr* ware;
   ushort w, h, clrkey;
   uint i;
   for(i=0; i<nware; i++) {
      ware=new Ware_Descr;
      f->read(ware->name, sizeof(ware->name));
      f->read(&w, sizeof(ushort));
      f->read(&h, sizeof(ushort));
      f->read(&clrkey, sizeof(ushort));
      if(w*h*sizeof(ushort) > buf_size) {
         buf_size=w*h*sizeof(ushort);
         buf= (char*) realloc(buf, buf_size);
      }
      f->read(buf, 1152); // menu_pic
      ware->menu_pic.create(24, 24, (ushort*) buf);
      f->read(buf, w*h*sizeof(ushort));
      ware->pic.create(w, h, (ushort*) buf);
      ware->menu_pic.set_clrkey(clrkey);
      ware->pic.set_clrkey(clrkey);
      wares.add(ware);
   }
   
   free(buf);
   return OK;
}

int Tribe_Descr::parse_bobs(Binary_file* f) {
   // TODO: for the moment, this data (since this really belongs to the game or the map) is completly ignored
   char buf[1024];
   
   // read magic
   f->read(buf, 5);
   if(strcasecmp("Bobs\0", buf)) return ERR_FAILED;
   ushort nitems;
   f->read(&nitems, sizeof(ushort));

   uint i;
   ushort npics;
   ushort size;
   for(i=0; i< nitems; i++) {
      // 1 byte type
      f->read(buf, 1);
      // 30 name
      f->read(buf, 30);
      
      // 4 dimensions
      // 4 hot_spot
      f->read(buf, 8);
      f->read(&npics, sizeof(ushort));
      uint j;
      for(j=0; j<npics; j++) {
         f->read(&size, sizeof(ushort));
         while(size>1024) {
            f->read(buf, 1024);
            size-=1024;
         }
         f->read(buf, size);
      }
      // 
      // 32 bytes garbage (ONLY AT THE MOMENT!)
      f->read(buf, 32);
   }

   return RET_OK;
}

//
// this function reads out the header, skips some fields
// (like descr, since they just burn memory in game) 
// and loads others
int Tribe_Descr::parse_header(Binary_file* f) {
   char buf[1024];
   
   // read magic
   f->read(buf, 5);
   if(strcasecmp(buf, "WLtf\0")) return ERR_FAILED;

   // read version
   ushort given_vers;
   f->read(&given_vers, sizeof(ushort));
   if(VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLTF_VERSION)) return ERR_WRONGVERSION;
   if(VERSION_MAJOR(given_vers)==VERSION_MAJOR(WLTF_VERSION)) 
      if(VERSION_MAJOR(given_vers) > VERSION_MAJOR(WLTF_VERSION)) return ERR_WRONGVERSION;
   
   // read name, skip author and description
   f->read(name, sizeof(name));
   f->read(buf, sizeof(name)); // author
   f->read(buf, 1024); // description

   // skip need list
   // read magic
   f->read(buf, 9);
   if(strcasecmp(buf, "NeedList\0")) return ERR_FAILED;
   ushort nneeds=0;
   f->read(&nneeds, sizeof(ushort));

   uint i;
   ushort is_a;
   for(i=0; i<nneeds; i++) {
      f->read(&is_a, sizeof(ushort));
      f->read(buf, 30);
   }

   return OK;
}

//
// this reads out the regent data
//
int Tribe_Descr::parse_regent(Binary_file* f) {
   char buf[14000];

   // read magic
   f->read(buf, 7);
   if(strcasecmp(buf, "Regent\0")) return ERR_FAILED;
   
   // we ignore all of the stuff. this is already in the player description
   f->read(buf, 30); // name
   f->read(buf, 6300);// small pic
   f->read(buf, 14000); // big pic
   
   return OK;
}


