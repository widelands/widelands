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

#include <vector>
#include "widelands.h"
#include "world.h"
#include "bob.h"
#include "game.h"
#include "cmd_queue.h"
#include "map.h"

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


/*
==============================================================================   

class Diminishing_Bob

==============================================================================   
*/

class Diminishing_Bob : public Map_Object {
		MO_DESCR(Diminishing_Bob_Descr);

   public:
		Diminishing_Bob(Diminishing_Bob_Descr* d);
      virtual ~Diminishing_Bob(void);

		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Diminishing_Bob::Diminishing_Bob(Diminishing_Bob_Descr* d)
	: Map_Object(d)
{
} 

Diminishing_Bob::~Diminishing_Bob()
{
}

void Diminishing_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	// infinitely idle
	start_task_idle(g, get_descr()->get_anim(), -1);
}


/*
==============================================================================   

class Boring_Bob

==============================================================================   
*/

class Boring_Bob : public Map_Object {
		MO_DESCR(Boring_Bob_Descr);

   public:
      Boring_Bob(Boring_Bob_Descr *d);
      virtual ~Boring_Bob(void);

		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Boring_Bob::Boring_Bob(Boring_Bob_Descr *d)
	: Map_Object(d)
{
}

Boring_Bob::~Boring_Bob()
{
}

void Boring_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	// infinitely idle
	start_task_idle(g, get_descr()->get_anim(), -1);
}


/*
==============================================================================   

class Critter_Bob

==============================================================================   
*/

#define CRITTER_MAX_WAIT_TIME_BETWEEN_WALK 2000 // wait up to 12 seconds between moves

class Critter_Bob : public Map_Object {
		MO_DESCR(Critter_Bob_Descr);

   public:
      Critter_Bob(Critter_Bob_Descr *d);
      virtual ~Critter_Bob(void);

		uint get_movecaps();

		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

Critter_Bob::Critter_Bob(Critter_Bob_Descr *d)
	: Map_Object(d)
{
}

Critter_Bob::~Critter_Bob()
{
}

uint Critter_Bob::get_movecaps() { return get_descr()->is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK; }

void Critter_Bob::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	if (prev == TASK_IDLE)
	{
		// Build the animation list - this needs a better solution (AnimationManager, anyone?)
		Animation *anims[6];
		
		anims[0] = get_descr()->get_walk_ne_anim();
		anims[1] = get_descr()->get_walk_e_anim();
		anims[2] = get_descr()->get_walk_se_anim();
		anims[3] = get_descr()->get_walk_sw_anim();
		anims[4] = get_descr()->get_walk_w_anim();
		anims[5] = get_descr()->get_walk_nw_anim();
		
		// Pick a target at random
		Coords dst;
		
		dst.x = m_pos.x + (rand()%5) - 2;
		dst.y = m_pos.y + (rand()%5) - 2;
		
		if (start_task_movepath(g, dst, 3, anims))
			return;
	
		start_task_idle(g, get_descr()->get_anim(), 1 + g->logic_rand()%1000);
		return;
	}
	
	// idle for a longer period
	start_task_idle(g, get_descr()->get_anim(), 1000 + g->logic_rand() % CRITTER_MAX_WAIT_TIME_BETWEEN_WALK);
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
   cerr << "Growing_Bob_Descr::create_object() TODO!" << endl;
	
	return 0; // uh oh
}

