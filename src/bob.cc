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
#include "cmd_queue.h"

//
// this is a function which reads a animation from a file
//
int Animation::read(Binary_file* f) {
   ushort np;
   f->read(&np, sizeof(ushort));

   uint i;
   char* buf;
   ushort buf_size=15000;
   buf=(char*) malloc(buf_size);
   
   ushort size;
   for(i=0; i<np; i++) {
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
Diminishing_Bob::Diminishing_Bob(Diminishing_Bob_Descr* d)
	: Map_Object(DIMINISHING_BOB)
{
	descr=d;
} 

void Diminishing_Bob::init(Game *g, Instance *i)
{
	set_animation(g, descr->get_anim());
}

//
// class Boring_Bob
// 
Boring_Bob::Boring_Bob(Boring_Bob_Descr *d)
	: Map_Object(BORING_BOB)
{
	descr = d;
}

void Boring_Bob::init(Game *g, Instance *i)
{
	set_animation(g, descr->get_anim());
}

//
// class Critter_Bob
// 
#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000 // wait up to 12 seconds between moves

Critter_Bob::Critter_Bob(Critter_Bob_Descr *d)
	: Map_Object(CRITTER_BOB)
{
	descr = d;
}

uint Critter_Bob::get_movecaps() { return descr->is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK; }

void Critter_Bob::init(Game *g, Instance *i)
{
	set_animation(g, descr->get_anim());
	
	// gotcha... need to schedule an initial act() ;)
	g->get_cmdqueue()->queue(g->get_gametime(), SENDER_MAPOBJECT, CMD_ACT, (int)i, 0, 0);
}

void Critter_Bob::act(Game* g, Instance *i)
{
	if (m_walking != IDLE) {
		end_walk();

//		cerr << "Critter stops walking" << endl;
				
		// 30% chance of stopping
		if (g->logic_rand() % 100 < 30) {
			int t = g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK;

			set_animation(g, descr->get_anim());			
			g->get_cmdqueue()->queue(g->get_gametime()+t, SENDER_MAPOBJECT, CMD_ACT, (int)i, 0, 0);
			return;
		}
	}
	
	Animation *a = 0;
	WalkingDir dir = IDLE;
	
	switch(g->logic_rand() % 6) {
	case 0: dir = WALK_NW; a = descr->get_walk_nw_anim(); break;
	case 1: dir = WALK_NE; a = descr->get_walk_ne_anim(); break;
	case 2: dir = WALK_W; a = descr->get_walk_w_anim(); break;
	case 3: dir = WALK_E; a = descr->get_walk_e_anim(); break;
	case 4: dir = WALK_SW; a = descr->get_walk_sw_anim(); break;
	case 5: dir = WALK_SE; a = descr->get_walk_se_anim(); break;
	}
	
//	cerr << "Critter attempts to walk" << endl;
	
	if (!start_walk(g, i, dir, a))
		g->get_cmdqueue()->queue(g->get_gametime()+g->logic_rand()%1000, SENDER_MAPOBJECT, CMD_ACT, (int)i, 0, 0);
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

Map_Object *Boring_Bob_Descr::create_object()
{
	return new Boring_Bob(this);
}

//
// class Critter_Bob_Descr
//
int Critter_Bob_Descr::read(Binary_file* f) {
   Logic_Bob_Descr::read(f);

   f->read(&stock, sizeof(ushort));
   uchar temp;
   f->read(&temp, sizeof(uchar));
   swimming=temp;

   // read all the other bobs
   walk_ne.set_dimensions(anim.get_w(), anim.get_h());
   walk_nw.set_dimensions(anim.get_w(), anim.get_h());
   walk_w.set_dimensions(anim.get_w(), anim.get_h());
   walk_sw.set_dimensions(anim.get_w(), anim.get_h());
   walk_se.set_dimensions(anim.get_w(), anim.get_h());
   walk_e.set_dimensions(anim.get_w(), anim.get_h());
   walk_ne.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_nw.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_w.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_sw.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_se.set_hotspot(anim.get_hsx(), anim.get_hsy());
   walk_e.set_hotspot(anim.get_hsx(), anim.get_hsy());
   
   walk_ne.read(f);
   walk_e.read(f);
   walk_se.read(f);
   walk_sw.read(f);
   walk_w.read(f);
   walk_nw.read(f);

   return RET_OK;
}

Map_Object *Critter_Bob_Descr::create_object()
{
	return new Critter_Bob(this);
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

Map_Object *Diminishing_Bob_Descr::create_object()
{
	return new Diminishing_Bob(this);
}

//
// class Growing_Bob_Descr
//
int Growing_Bob_Descr::read(Binary_file* f) {
   cerr << "Growing_Bob_Descr::read() TODO!" << endl;
   return RET_OK;
}

Map_Object *Growing_Bob_Descr::create_object()
{
   cerr << "Growing_Bob_Descr::create_instance() TODO!" << endl;
	
	return 0; // uh oh
}


