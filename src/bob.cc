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
#include "map.h"

//
// this is a function which reads a animation from a file
//
int Animation::read(FileRead* f)
{
   ushort np;
   np = f->Unsigned16();

   uint i;
   
   ushort size;
   for(i=0; i<np; i++) {
      size = f->Unsigned16();;
      
      ushort *ptr = (ushort*)f->Data(size);
      add_pic(size, ptr);
   }
  
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
int Logic_Bob_Descr::read(FileRead* f)
{
   memcpy(name, f->Data(30), 30);

   ushort h, w, hsx, hsy;

   w = f->Unsigned16();
   h = f->Unsigned16();
   hsx = f->Unsigned16();
   hsy = f->Unsigned16();

   anim.set_dimensions(w,h);
   anim.set_hotspot(hsx, hsy);

   anim.read(f);

   return RET_OK;
}

//
// class Boring_Bob_Descr
//
int Boring_Bob_Descr::read(FileRead* f)
{
   Logic_Bob_Descr::read(f);

   ttl = f->Unsigned16();
      
   return RET_OK;
}

Map_Object *Boring_Bob_Descr::create_object()
{
	return new Boring_Bob(this);
}

//
// class Critter_Bob_Descr
//
int Critter_Bob_Descr::read(FileRead* f) {
   Logic_Bob_Descr::read(f);

   stock = f->Unsigned16();
   uchar temp;
   temp = f->Unsigned8();
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
int Diminishing_Bob_Descr::read(FileRead* f) {
   Logic_Bob_Descr::read(f);
   
   stock = f->Unsigned16();
   // TODO: ends in should be changed!
	f->Data(30);
   
   return RET_OK;
}

Map_Object *Diminishing_Bob_Descr::create_object()
{
	return new Diminishing_Bob(this);
}

//
// class Growing_Bob_Descr
//
int Growing_Bob_Descr::read(FileRead* f) {
   cerr << "Growing_Bob_Descr::read() TODO!" << endl;
   return RET_OK;
}

Map_Object *Growing_Bob_Descr::create_object()
{
   cerr << "Growing_Bob_Descr::create_object() TODO!" << endl;
	
	return 0; // uh oh
}

