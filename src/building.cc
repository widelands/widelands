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
#include "helper.h"
#include "tribe.h"
#include "game.h"
#include "player.h"

// 
// Need List
// 
int NeedWares_List::read(FileRead* f)
{
   nneeds = f->Signed16();
   if(!nneeds) {
      // we're done, this guy is for free
      return RET_OK;
   }
   

   list=(List*) malloc(sizeof(List)*nneeds);
   
   int i;
   for(i=0; i< nneeds; i++) {
      list[i].count = f->Unsigned16();
      list[i].index = f->Unsigned16();
      list[i].stock = f->Unsigned16();
   }

   return RET_OK;
}

// 
// Down here: Descriptions

bool Building_Descr::has_attribute(uint attrib)
{
	switch(attrib) {
	case Map_Object::ROBUST:
	case Map_Object::UNPASSABLE:
		return true;
	}
	
	return false; // no need to call parent has_attribute()
}

int Building_Descr::read(FileRead *f)
{
   memcpy(name, f->Data(sizeof(name)), sizeof(name));

   uchar temp;
   temp = f->Unsigned8();
   is_enabled = temp ? true : false;
   see_area = f->Unsigned16();
   uint w, h, hsx, hsy;
	w = f->Unsigned16();
	h = f->Unsigned16();
	hsx = f->Unsigned16();
	hsy = f->Unsigned16();

   idle.set_dimensions(w, h);
   idle.set_hotspot(hsx, hsy);

   idle.read(f);

   return RET_OK;
}

int Has_Needs_Building_Descr::read(FileRead *f)
{
   uchar temp;
   temp = f->Unsigned8();
   needs_or=temp ? true : false;
   
   // read needs
   needs.read(f);
   
   return RET_OK;
}

int Has_Products_Building_Descr::read(FileRead *f)
{
   uchar temp;
	temp = f->Unsigned8();
	products_or = temp ? true : false;

   // read products
   products.read(f);

   return RET_OK;
}

int Has_Is_A_Building_Descr::read(FileRead *f)
{
	is_a = f->Unsigned16();

	return RET_OK;
}

int Buildable_Building_Descr::read(FileRead *f)
{
	memcpy(category, f->Data(sizeof(category)), sizeof(category));
	build_time = f->Unsigned16();;

	// read cost
	cost.read(f);

	build.set_dimensions(idle.get_w(), idle.get_h());
	build.set_hotspot(idle.get_hsx(), idle.get_hsy());
	
	build.read(f);
	
	return RET_OK;
}

int Working_Building_Descr::read(FileRead *f)
{
   working.set_dimensions(idle.get_w(), idle.get_h());
   working.set_hotspot(idle.get_w(), idle.get_h());
   
   working.read(f);

   return RET_OK;
}

int Boring_Building_Descr::read(FileRead *f)
{
   // nothing to do
   return RET_OK;
}

/*
==============================================================================

Building helper functions

==============================================================================
*/
void conquer_area(uchar player, Map* map, int x, int y, ushort area) {
   Map_Region m(x, y, area, map);
   Field* f;
   while((f=m.next())) {
      if(f->get_owned_by() == player) continue;
      if(f->get_owned_by() == FIELD_OWNED_BY_NOONE) {
         f->set_owned_by(player);
         continue;
      }
      // TODO: add support here what to do if some fields are already
      // occupied by another player
      cerr << "warning: already occupied field is claimed by another user!" << endl;
   }
}

/*
==============================================================================

Base Building

==============================================================================
*/

class Building : public Map_Object {
	MO_DESCR(Building_Descr)

public:
	Building(Building_Descr *descr);
	
	virtual void init(Game* g);
};

Building::Building(Building_Descr *descr)
	: Map_Object(descr)
{
}

/** Building::init(Game *g)
 *
 * Common building initialization code. You must call this from derived class' init.
 */
void Building::init(Game* g)
{
   Player* player = g->get_player(get_owned_by());

	assert(player);
   
	player->set_area_seen(m_pos.x, m_pos.y, get_descr()->get_see_area(), true);

	Map_Object::init(g);
}

// 
// DOWN HERE: The real buildings, no abstractions
// 
/*
==============================================================================

Dig Building

==============================================================================
*/

int Dig_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // own 
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();
   memcpy(resource, f->Data(sizeof(resource)), sizeof(resource));

   return RET_OK;
}
Map_Object *Dig_Building_Descr::create_object()
{
   cerr << "Dig_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
   return 0;
}

/*
==============================================================================

Search Building

==============================================================================
*/

int Search_Building_Descr::read(FileRead *f)
{
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // read our own stuff
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   working_area = f->Unsigned16();
   worker = f->Unsigned16();
   nbobs = f->Unsigned16();
   bobs = (char*)malloc(nbobs*30);
	memcpy(bobs, f->Data(nbobs*30), nbobs*30);

   return RET_OK;
}
Map_Object *Search_Building_Descr::create_object()
{
   cerr << "Search_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
   return 0;
}

/*
==============================================================================

Plant Building

==============================================================================
*/

int Plant_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // read our own stuff
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   working_area = f->Unsigned16();
   worker = f->Unsigned16();
   nbobs = f->Unsigned16();
   bobs = (char*)malloc(nbobs*30);
   memcpy(bobs, f->Data(nbobs*30), nbobs*30);

   return RET_OK;
}
Map_Object *Plant_Building_Descr::create_object()
{
   cerr << "Plant_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Grow Building

==============================================================================
*/

int Grow_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // own stuff 
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   working_area = f->Unsigned16();
   worker = f->Unsigned16();
   memcpy(plant_bob, f->Data(sizeof(plant_bob)), sizeof(plant_bob));
	memcpy(search_bob, f->Data(sizeof(search_bob)), sizeof(search_bob));

   return RET_OK;
}
Map_Object *Grow_Building_Descr::create_object()
{
   cerr << "Grow_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Sitter Building

==============================================================================
*/

int Sit_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);
   Has_Products_Building_Descr::read(f);

   // our stuff 
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();
   uchar temp; 
   temp = f->Unsigned8();
   order_worker=temp ? true : false;

   return RET_OK;
}
Map_Object *Sit_Building_Descr::create_object()
{
   cerr << "Sit_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
   return 0;
}


int Sit_Building_Produ_Worker_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own stuff 
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();
   prod_worker = f->Unsigned16();

   return RET_OK;
}
Map_Object *Sit_Building_Produ_Worker_Descr::create_object()
{
   cerr << "Sit_Building_Produ_Worker_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Science Building

==============================================================================
*/

int Science_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Working_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);

   cerr << "Science_Building_Descr::read() TODO!" << endl;

   return RET_OK;
}
Map_Object *Science_Building_Descr::create_object()
{
   cerr << "Science_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Military Building

==============================================================================
*/

int Military_Building_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own stuff
   beds = f->Unsigned16();
   conquers = f->Unsigned16();
   idle_time = f->Unsigned16();
   nupgr = f->Unsigned16();

   return RET_OK;
}
Map_Object *Military_Building_Descr::create_object()
{
   cerr << "Military_Building_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Cannon

==============================================================================
*/

int Cannon_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   uchar temp;
   // own stuff 
   idle_time = f->Unsigned16();
   projectile_speed = f->Unsigned16();
   temp = f->Unsigned8();
   fires_balistic=temp ? true : false;
   worker = f->Unsigned16();
   //                         // width and height ob projectile bob
   ushort wproj, hproj;
   wproj = f->Unsigned16();
   hproj = f->Unsigned16();

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

   projectile.read(f);
   fire_ne.read(f);
   fire_e.read(f);
   fire_se.read(f);
   fire_sw.read(f);
   fire_w.read(f);
   fire_nw.read(f);

   return RET_OK;
}
Map_Object *Cannon_Descr::create_object()
{
   cerr << "Cannon_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Headquarters

==============================================================================
*/

class Building_HQ : public Building {
		MO_VIRTUAL_DESCR(HQ_Descr)

   public:
      Building_HQ(HQ_Descr* d);
		
		virtual void init(Game* g);
		virtual void task_start_best(Game*, uint prev, bool success, uint nexthint);
};

// HQ code
Building_HQ::Building_HQ(HQ_Descr *d)
	: Building(d)
{
	m_descr = d;
}

void Building_HQ::init(Game* g)
{
   // conquer area
	conquer_area(get_owned_by(), g->get_map(), m_pos.x, m_pos.y, get_descr()->get_conquers());
	
	Building::init(g);
}

void Building_HQ::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	start_task_idle(g, get_descr()->get_idle_anim(), -1);
}

// HQ description
int HQ_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);

   // own 
   conquers = f->Unsigned16();

   return 0;
}
Map_Object *HQ_Descr::create_object()
{
	return new Building_HQ(this);
}

/*
==============================================================================

Store

==============================================================================
*/

int Store_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);

   // nothing else

   return RET_OK;
}
Map_Object *Store_Descr::create_object()
{
   cerr << "Store_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Dockyard

==============================================================================
*/

int Dockyard_Descr::read(FileRead *f) {
   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);
   Has_Is_A_Building_Descr::read(f);
   Has_Needs_Building_Descr::read(f);

   // own
   working_time = f->Unsigned16();
   idle_time = f->Unsigned16();
   worker = f->Unsigned16();

   return RET_OK;
}
Map_Object *Dockyard_Descr::create_object()
{
   cerr << "Dockyard_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}


/*
==============================================================================

Port

==============================================================================
*/

int Port_Descr::read(FileRead *f) {

   Building_Descr::read(f);
   Boring_Building_Descr::read(f);
   Buildable_Building_Descr::read(f);

   // nothing else

   return RET_OK;
}
Map_Object *Port_Descr::create_object()
{
   cerr << "Port_Descr::create_instance() not yet implemented: TODO!" << endl;
	return 0;
}

