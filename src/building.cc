/*
 * Copyright (C) 2002 by Widelands Development Team
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
#include "tribedata.h"
#include "myfile.h"
#include "helper.h"
#include "myfile.h"
#include "tribe.h"
#include "game.h"

// 
// Need List
// 
int NeedWares_List::read(Binary_file* f) {
   f->read(&nneeds, sizeof(short));
   if(!nneeds) {
      // we're done, this guy is for free
      return RET_OK;
   }
   

   list=(List*) malloc(sizeof(List)*nneeds);
   
   int i;
   for(i=0; i< nneeds; i++) {
      f->read(&list[i].count, sizeof(ushort));
      f->read(&list[i].index, sizeof(ushort));
      f->read(&list[i].stock, sizeof(ushort));
   }

   return RET_OK;
}

// down here: Buildings as Map_Objects
int Building_HQ::act(Game* g) {

   // cerr << "Building HQ should act. Repeating in ten frames!" << endl;
   

   next_acting_frame=g->get_frame()+10;
   cur_pic=g->get_player_tribe(owned_by)->get_building_descr(0)->get_idle()->get_pic(0);

   return RET_OK;
}

// 
// Down here: Descriptions

int Building_Descr::read(Binary_file *f) {
   f->read(name, sizeof(name));

   uchar temp;
   f->read(&temp, sizeof(uchar));
   is_enabled=temp;
   f->read(&see_area, sizeof(ushort));
   f->read(&w, sizeof(ushort));
   f->read(&h, sizeof(ushort));
   f->read(&hsx, sizeof(ushort));
   f->read(&hsy, sizeof(ushort));

   idle.set_dimensions(w, h);
   idle.set_hotspot(hsx, hsy);

   read_anim(&idle, f);

   return RET_OK;
}

int Has_Needs_Building_Descr::read(Binary_file *f) {
   uchar temp;
   f->read(&temp, sizeof(uchar));
   needs_or=temp;
   
   // read needs
   needs.read(f);
   
   return RET_OK;
}

int Has_Products_Building_Descr::read(Binary_file *f) {
   uchar temp;
    f->read(&temp, sizeof(uchar));
   products_or=temp;

   // read products
   products.read(f);

   return RET_OK;
}

int Has_Is_A_Building_Descr::read(Binary_file *f) {
  f->read(&is_a, sizeof(ushort));

  return RET_OK;
}

int Buildable_Building_Descr::read(Binary_file *f) {
     f->read(category, sizeof(category));
     f->read(&build_time, sizeof(ushort));

     // read cost
     cost.read(f);

     build.set_dimensions(idle.get_w(), idle.get_h());
     build.set_hotspot(idle.get_hsx(), idle.get_hsy());
     
     read_anim(&build, f); 
     
     return RET_OK;
}

int Working_Building_Descr::read(Binary_file *f) {
   working.set_dimensions(idle.get_w(), idle.get_h());
   working.set_hotspot(idle.get_w(), idle.get_h());
   
   read_anim(&working, f);

   return RET_OK;
}

int Boring_Building_Descr::read(Binary_file *f) {
   // nothing to do
   return RET_OK;
}


// 
// DOWN HERE: The real buildings, no abstractions
// 

int Dig_Building_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // own 
   f->read(&working_time, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&worker, sizeof(ushort));
   f->read(resource, sizeof(resource));

   return RET_OK;
}
int Dig_Building_Descr::create_instance(Instance* inst) {
   cerr << "Dig_Building_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}

int Search_Building_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // read our own stuff
   f->read(&working_time, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&working_area, sizeof(ushort));
   f->read(&worker, sizeof(ushort));
   f->read(&nbobs, sizeof(nbobs));
   bobs=(char*) malloc(nbobs*30);
   f->read(bobs, nbobs*30);

   return RET_OK;
}
int Search_Building_Descr::create_instance(Instance* inst) {
   cerr << "Search_Building_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}

int Plant_Building_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // read our own stuff
   f->read(&working_time, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&working_area, sizeof(ushort));
   f->read(&worker, sizeof(ushort));
   f->read(&nbobs, sizeof(nbobs));
   bobs=(char*) malloc(nbobs*30);
   f->read(bobs, nbobs*30);

   return RET_OK;
}
int Plant_Building_Descr::create_instance(Instance* inst) {
   cerr << "Plant_Building_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Grow_Building_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // own stuff 
   f->read(&working_time, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&working_area, sizeof(ushort));
   f->read(&worker, sizeof(ushort));
   f->read(plant_bob, sizeof(plant_bob));
   f->read(search_bob, sizeof(search_bob));

   return RET_OK;
}
int Grow_Building_Descr::create_instance(Instance* inst) {
   cerr << "Grow_Building_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Sit_Building_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // our stuff 
   f->read(&working_time, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&worker, sizeof(ushort));
   uchar temp; 
   f->read(&temp, sizeof(uchar));
   order_worker=temp;

   return RET_OK;
}
int Sit_Building_Descr::create_instance(Instance* inst) {
   cerr << "Sit_Building_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Sit_Building_Produ_Worker_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own stuff 
   f->read(&working_time, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&worker, sizeof(ushort));
   f->read(&prod_worker, sizeof(ushort));

   return RET_OK;
}
int Sit_Building_Produ_Worker_Descr::create_instance(Instance* inst) {
   cerr << "Sit_Building_Produ_Worker_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Science_Building_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);

   cerr << "Science_Building_Descr::read() TODO!" << endl;

   return RET_OK;
}
int Science_Building_Descr::create_instance(Instance* inst) {
   cerr << "Science_Building_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Military_Building_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own stuff
   f->read(&beds, sizeof(ushort));
   f->read(&conquers, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&nupgr, sizeof(ushort));

   return RET_OK;
}
int Military_Building_Descr::create_instance(Instance* inst) {
   cerr << "Military_Building_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Cannon_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   uchar temp;
   // own stuff 
   f->read(&idle_time, sizeof(ushort));
   f->read(&projectile_speed, sizeof(ushort));
   f->read(&temp, sizeof(uchar));
   fires_balistic=temp;
   f->read(&worker, sizeof(ushort));
   //                         // width and height ob projectile bob
   ushort wproj, hproj;
   f->read(&wproj, sizeof(ushort));
   f->read(&hproj, sizeof(ushort));

   projectile.set_dimensions(wproj, hproj);
   projectile.set_hotspot(wproj/2, hproj/2);

   fire_ne.set_dimensions(idle.get_w(), idle.get_h());
   fire_ne.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_e.set_dimensions(idle.get_w(), idle.get_h());
   fire_e.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_se.set_dimensions(idle.get_w(), idle.get_h());
   fire_se.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_sw.set_dimensions(idle.get_w(), idle.get_h());
   fire_sw.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_w.set_dimensions(idle.get_w(), idle.get_h());
   fire_w.set_hotspot(idle.get_hsx(), idle.get_hsy());
   fire_nw.set_dimensions(idle.get_w(), idle.get_h());
   fire_nw.set_hotspot(idle.get_hsx(), idle.get_hsy());

   read_anim(&projectile, f);
   read_anim(&fire_ne, f);
   read_anim(&fire_e, f);
   read_anim(&fire_se, f);
   read_anim(&fire_sw, f);
   read_anim(&fire_w, f);
   read_anim(&fire_nw, f);

   return RET_OK;
}
int Cannon_Descr::create_instance(Instance* inst) {
   cerr << "Cannon_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int HQ_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);

   // own 
   f->read(&conquers, sizeof(ushort));

   return RET_OK;
}
int HQ_Descr::create_instance(Instance* inst) {
   cerr << "HQ_Descr::create_instance() not yet implemented: TODO!" << endl;

   inst->obj=new Building_HQ();

   return 10; // act in 10 frames again 
}


int Store_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);

   // nothing else

   return RET_OK;
}
int Store_Descr::create_instance(Instance* inst) {
   cerr << "Store_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Dockyard_Descr::read(Binary_file *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own
   f->read(&working_time, sizeof(ushort));
   f->read(&idle_time, sizeof(ushort));
   f->read(&worker, sizeof(ushort));

   return RET_OK;
}
int Dockyard_Descr::create_instance(Instance* inst) {
   cerr << "Dockyard_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}


int Port_Descr::read(Binary_file *f) {

   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);

   // nothing else

   return RET_OK;
}
int Port_Descr::create_instance(Instance* inst) {
   cerr << "Port_Descr::create_instance() not yet implemented: TODO!" << endl;

   return RET_OK;
}



