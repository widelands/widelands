/*
 * Copyright (C) 2002 by Florian Bluemel
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
#include "md5file.h"
#include "tribe.h"
#include "tribedata.h"

//
// this is a function local to this file which reads a animation from a file
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
   for(i=0; i<nbuilds; i++) {

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

// 
// Need List
// 
int Need_List::read(Binary_file* f) {
   f->read(&nneeds, sizeof(short));
   if(!nneeds) {
      // we're done, this guy is for free
      return RET_OK;
   } else if(nneeds==-1) {
      // this guy can't be assembled in stores
      return RET_OK;
   }

   list=(List*) malloc(sizeof(List)*nneeds);
   
   int i;
   for(i=0; i< nneeds; i++) {
      f->read(&list[i].count, sizeof(ushort));
      f->read(&list[i].index, sizeof(ushort));
   }

   return RET_OK;
}


// 
// class Worker_Descr
//
int Worker_Descr::read(Binary_file* f) {
   uchar temp;
   
   f->read(name, sizeof(name));

   f->read(&temp, sizeof(uchar));
   is_enabled=temp;
   f->read(&walking_speed, sizeof(ushort));

   needs.read(f);

   ushort w, h, sx, sy;
   f->read(&w, sizeof(ushort));
   f->read(&h, sizeof(ushort));
   f->read(&sx, sizeof(ushort));
   f->read(&sy, sizeof(ushort));

   walk_ne.set_dimensions(w, h);
   walk_nw.set_dimensions(w, h);
   walk_w.set_dimensions(w, h);
   walk_sw.set_dimensions(w, h);
   walk_se.set_dimensions(w, h);
   walk_w.set_dimensions(w, h);
   walk_ne.set_hotspot(sx, sy);
   walk_nw.set_hotspot(sx, sy);
   walk_w.set_hotspot(sx, sy);
   walk_sw.set_hotspot(sx, sy);
   walk_se.set_hotspot(sx, sy);
   walk_w.set_hotspot(sx, sy);
   
   read_anim(&walk_ne, f);
   read_anim(&walk_e, f);
   read_anim(&walk_se, f);
   read_anim(&walk_sw, f);
   read_anim(&walk_w, f);
   read_anim(&walk_nw, f);

   return RET_OK;
}

// 
// class Soldier_Descr
// 
int Soldier_Descr::read(Binary_file* f) {
   
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   f->read(&energy, sizeof(ushort));

   attack_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&attack_l, f);
   attack1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&attack1_l, f);
   evade_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&evade_l, f);
   evade1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&evade1_l, f);

   attack_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&attack_r, f);
   attack1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&attack1_r, f);
   evade_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&evade_r, f);
   evade1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&evade1_r, f);
     
   return RET_OK;
}

// Worker class read functions
int Has_Working_Worker_Descr::read(Binary_file* f) {
   working.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&working, f);
   return RET_OK;
}

int Has_Working1_Worker_Descr::read(Binary_file* f) {
   working1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   read_anim(&working1, f);
   return RET_OK;
}
int Has_Walk1_Worker_Descr::read(Binary_file* f) {
   walk_ne1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_nw1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_w1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_sw1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_se1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_w1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   walk_ne1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_nw1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_w1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_sw1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_se1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   walk_w1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   
   read_anim(&walk_ne1, f);
   read_anim(&walk_e1, f);
   read_anim(&walk_se1, f);
   read_anim(&walk_sw1, f);
   read_anim(&walk_w1, f);
   read_anim(&walk_nw1, f);

   return RET_OK;
}
int Scientist::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   return RET_OK;
}

int Searcher::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);

   // nothing additional 
   return RET_OK;
}

int Grower::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Planter::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   
   return RET_OK;
}
int SitDigger_Base::read(Binary_file* f) {
   // Nothing to do
   return RET_OK;
}
int SitDigger::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   
   // Nothing of our own stuff to read
   return RET_OK;
}

int Carrier::read(Binary_file* f) {

   // nothing to do
   return RET_OK;
}

int Def_Carrier::read(Binary_file* f) {
   Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Add_Carrier::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}
int Builder::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);
   
   return RET_OK;
}

int Planer::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
      
   return RET_OK;
}

int Explorer::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);

   return RET_OK;
}

int Geologist::read(Binary_file* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);
   
   return RET_OK;
}
