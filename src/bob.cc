/*
 * Copyright (C) 2002 by The Widelands Development Team
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
#include "game.h"

//
// this is a function which reads a animation from a file
//
int Animation::read(Binary_file* f) {
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
      add_pic(size, (ushort*) buf);
   }
  
   free(buf);
   
   return RET_OK;
}
   
// 
// class Diminishing_Bob
//
int Diminishing_Bob::act(Game* g) {
   cur_pic=g->get_map()->get_world()->get_bob_descr(idx)->get_anim()->get_pic(0);

   return RET_OK;
}

//
// class Boring_Bob
// 
int Boring_Bob::act(Game* g) {
   cur_pic=g->get_map()->get_world()->get_bob_descr(idx)->get_anim()->get_pic(0);

   return RET_OK;
}

// DOWN HERE: DECRIPTION CLASSES
// 
// class Logic_Bob_Descr
//
int Logic_Bob_Descr::read(Binary_file* f) {
   f->read(name, 30);

   ushort h, w, hsx, hsy;

   f->read(&w, sizeof(ushort));
   f->read(&h, sizeof(ushort));
   f->read(&hsx, sizeof(ushort));
   f->read(&hsy, sizeof(ushort));

   anim.set_dimensions(w,h);
   anim.set_hotspot(hsx, hsy);

   anim.read(f);


   return RET_OK;
}

//
// class Boring_Bob_Descr
//
int Boring_Bob_Descr::read(Binary_file* f) {
   Logic_Bob_Descr::read(f);

   f->read(&ttl, sizeof(ushort));
      
   return RET_OK;
}
int Boring_Bob_Descr::create_instance(Instance* inst) {
   
   inst->obj=new Boring_Bob(g_game->get_map()->get_world()->get_bob(this->get_name()));

   cerr << "Boring_Bob_Descr::create_instance() TODO!" << endl;

   return RET_OK;
}

//
// class Diminishing_Bob_Descr
//
int Diminishing_Bob_Descr::read(Binary_file* f) {
   Logic_Bob_Descr::read(f);
   
   f->read(&stock, sizeof(ushort));
   // TODO: ends in should be changed!
   char buf[30];
   f->read(buf, 30);
   
   return RET_OK;
}
int Diminishing_Bob_Descr::create_instance(Instance* inst) {
   inst->obj=new Diminishing_Bob(g_game->get_map()->get_world()->get_bob(this->get_name()));
   
   cerr << "Diminishing_Bob_Descr::create_instance() TODO!" << endl;

   return RET_OK;
}

//
// class Growing_Bob_Descr
//
int Growing_Bob_Descr::read(Binary_file* f) {
   cerr << "Growing_Bob_Descr::read() TODO!" << endl;
   return RET_OK;
}
int Growing_Bob_Descr::create_instance(Instance* inst) {
   cerr << "Growing_Bob_Descr::create_instance() TODO!" << endl;

   return RET_OK;
}
//
// class Boring_Bob_Descr
//
/*int Critter_Bob_Descr::read(Binary_file* f) {
   cerr << "Critter_Bob_Descr::read() TODO!" << endl;
   return RET_OK;
}*/


