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
#include "profile.h"
#include "tribedata.h"
#include "helper.h"
#include "tribe.h"
#include "game.h"
#include "player.h"
#include "transport.h"


/*
==============================================================================

Basic building

==============================================================================
*/

/*
===============
Building_Descr::Building_Descr

Initialize with sane defaults
===============
*/
Building_Descr::Building_Descr(Tribe_Descr *tribe, const char *name)
{
	m_tribe = tribe;
	snprintf(m_name, sizeof(m_name), "%s", name);
	strcpy(m_descname, m_name);
	m_buildable = true;
	m_size = BaseImmovable::SMALL;
	m_mine = false;
}


/*
===============
Building_Descr::~Building_Descr

Cleanup
===============
*/
Building_Descr::~Building_Descr(void)
{
}
		
/*
===============
Building_Descr::create

Create a building of this type. Does not perform any sanity checks.
===============
*/
Building *Building_Descr::create(Game *g, Player *owner, Coords pos)
{
	assert(owner);
	
	Building *b = create_object();
	b->m_owner = owner;
	b->m_position = pos;
	b->init(g);
	
	return b;
}

/*
===============
Building_Descr::parse

Parse the basic building settings from the given profile and directory
===============
*/
void Building_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Section *global = prof->get_safe_section("global");
	const char *string;
	
	snprintf(m_descname, sizeof(m_descname), "%s", global->get_safe_string("descname"));

	string = global->get_safe_string("size");
	if (!strcasecmp(string, "small")) {
		m_size = BaseImmovable::SMALL;
	} else if (!strcasecmp(string, "medium")) {
		m_size = BaseImmovable::MEDIUM;
	} else if (!strcasecmp(string, "big")) {
		m_size = BaseImmovable::BIG;
	} else if (!strcasecmp(string, "mine")) {
		m_size = BaseImmovable::SMALL;
		m_mine = true;
	} else
		throw wexception("Section [global], unknown size '%s'. Valid values are small, medium, big, mine",
		                 string);
	
	m_buildable = global->get_bool("buildable", true);
	
	Section *s = prof->get_section("idle");
	if (!s)
		throw wexception("Missing idle animation");
	m_idle.parse(directory, s, 0, encdata);
}


/*
==============================

Implementation

==============================
*/

Building::Building(Building_Descr *descr)
	: BaseImmovable(descr)
{
	m_owner = 0;
	m_flag = 0;
	m_optionswindow = 0;
}

Building::~Building()
{
	if (m_optionswindow)
		hide_options();
}

/*
===============
Building::get_type
Building::get_size
Building::get_passable
===============
*/
int Building::get_type()
{
	return BUILDING;
}

int Building::get_size()
{
	return get_descr()->get_size();
}

bool Building::get_passable()
{
	return false;
}

/*
===============
Building::add_to_economy [virtual]

Called when a building joins an economy.
Add all requests, provides etc.. to the economy.
===============
*/
void Building::add_to_economy(Economy *e)
{
}

/*
===============
Building::remove_from_economy [virtual]

Called when a building leaves an economy.
Remove all requests etc.. from the economy.
===============
*/
void Building::remove_from_economy(Economy *e)
{
}

/*
===============
Building::start_animation

Start the given animation
===============
*/
void Building::start_animation(Game *g, Animation *anim)
{
	m_anim = anim;
	m_animstart = g->get_gametime();
}

/*
===============
Building::init

Common building initialization code. You must call this from derived class' init.
===============
*/
void Building::init(Game* g)
{
	BaseImmovable::init(g);

	// Set the building onto the map
	Map *map = g->get_map();
	Coords neighb;
	
	set_position(g, m_position);
	
	if (get_size() == BIG) {
		map->get_ln(m_position, &neighb);
		set_position(g, neighb);
		
		map->get_tln(m_position, &neighb);
		set_position(g, neighb);
		
		map->get_trn(m_position, &neighb);
		set_position(g, neighb);
	}
	
	// Make sure the flag is there
	BaseImmovable *imm;
	Flag *flag;
	
	map->get_brn(m_position, &neighb);
	imm = map->get_immovable(m_position);
	
	if (imm && imm->get_type() == FLAG)
		flag = (Flag *)imm;
	else
		flag = Flag::create(g, get_owner(), neighb);
	
	flag->attach_building(g, this);
	m_flag = flag;
	
	// Start the animation
	start_animation(g, get_descr()->get_idle_anim());
}

/*
===============
Building::cleanup

Cleanup the building
===============
*/
void Building::cleanup(Game *g)
{
	m_flag->detach_building(g);
	
	// Unset the building
	unset_position(g, m_position);
	
	if (get_size() == BIG) {
		Map *map = g->get_map();
		Coords neighb;
		
		map->get_ln(m_position, &neighb);
		unset_position(g, neighb);
		
		map->get_tln(m_position, &neighb);
		unset_position(g, neighb);
		
		map->get_trn(m_position, &neighb);
		unset_position(g, neighb);
	}
	
	BaseImmovable::cleanup(g);
}

/*
===============
Building::draw

Draw the building.
===============
*/
void Building::draw(Game* game, Bitmap* dst, FCoords coords, int posx, int posy)
{
	if (coords != m_position)
		return; // draw big buildings only once

	copy_animation_pic(dst, m_anim, game->get_gametime() - m_animstart, posx, posy, m_owner->get_playercolor_rgb());
	
	// door animation?
}

/*
==============================================================================

Warehouse building

==============================================================================
*/

/*
===============
Warehouse_Descr::Warehouse_Descr

Initialize with sane defaults
===============
*/
Warehouse_Descr::Warehouse_Descr(Tribe_Descr *tribe, const char *name)
	: Building_Descr(tribe, name)
{
	m_subtype = Subtype_Normal;
	m_conquers = 0;
}

/*
===============
Warehouse_Descr::parse

Parse the additional warehouse settings from the given profile and directory
===============
*/
void Warehouse_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Building_Descr::parse(directory, prof, encdata);

	Section *global = prof->get_safe_section("global");
	const char *string;
	
	string = global->get_safe_string("subtype");
	if (!strcasecmp(string, "HQ")) {
		m_subtype = Subtype_HQ;
	} else if (!strcasecmp(string, "port")) {
		m_subtype = Subtype_Port;
	} else if (!strcasecmp(string, "none")) {
		//
	} else
		throw wexception("Unsupported warehouse subtype '%s'. Possible values: none, HQ, port", string);
	
	if (m_subtype == Subtype_HQ)
		m_conquers = global->get_int("conquers");
}


/*
==============================

IMPLEMENTATION

==============================
*/

#define CARRIER_SPAWN_INTERVAL	2500

/*
===============
Warehouse::Warehouse

Initialize a warehouse (zero contents, etc...)
===============
*/
Warehouse::Warehouse(Warehouse_Descr *descr)
	: Building(descr)
{
}

	
/*
===============
Warehouse::Warehouse

Cleanup
===============
*/
Warehouse::~Warehouse()
{
	// During building cleanup, we're removed from the Economy.
	// Therefore, the wares can simply be cleared out. The global inventory 
	// will be okay.
	m_wares.clear();
}

/*
===============
Warehouse::init

Conquer the land around the HQ on init.
===============
*/	
void Warehouse::init(Game* g)
{
	Building::init(g);

	if (get_descr()->get_subtype() == Warehouse_Descr::Subtype_HQ)
		g->conquer_area(get_owner()->get_player_number(), m_position, get_descr()->get_conquers());
	
	g->get_cmdqueue()->queue(g->get_gametime()+CARRIER_SPAWN_INTERVAL,
			SENDER_MAPOBJECT, CMD_ACT, m_serial, 0, 0);
}

/*
===============
Warehouse::cleanup

Destroy the warehouse.
===============
*/
void Warehouse::cleanup(Game *g)
{
	// TODO: un-conquer the area?

	Building::cleanup(g);
}

/*
===============
Warehouse::act

Act regularly to create carriers. According to intelligence, this is some
highly advanced technology. Not only do the settlers have no problems with
birth control, they don't even need anybody to procreate. They must have
built-in DNA samples in those warehouses. And what the hell are they doing,
killing useless tribesmen! The Borg? Or just like Soilent Green?
Or maybe I should just stop writing comments that late at night ;-)
===============
*/
void Warehouse::act(Game *g)
{
	int id = g->get_safe_ware_id("carrier");
	int stock = m_wares.stock(id);
	int tdelta = CARRIER_SPAWN_INTERVAL;
	
	if (stock < 100) {
		tdelta -= 4*(100 - stock);
		create_wares(id, 1);
	} else if (stock > 100) {
		tdelta -= 4*(stock - 100);
		if (tdelta < 10)
			tdelta = 10;
		destroy_wares(id, 1);
	}
	
	g->get_cmdqueue()->queue(g->get_gametime() + tdelta, SENDER_MAPOBJECT, 
			CMD_ACT, m_serial, 0, 0);
}

/*
===============
Warehouse::add_to_economy

Register our storage with the economy.
===============
*/
void Warehouse::add_to_economy(Economy *e)
{
	e->add_warehouse(this);
}

/*
===============
Warehouse::remove_from_economy

Unregister our storage with the economy
===============
*/
void Warehouse::remove_from_economy(Economy *e)
{
	e->remove_warehouse(this);
}

/*
===============
Warehouse::create_wares

Magically create wares in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::create_wares(int id, int count)
{
	m_wares.add(id, count);
	
	assert(m_flag);
	Economy *e = m_flag->get_economy();
	
	e->add_wares(id, count);
}

/*
===============
Warehouse::destroy_wares

Magically destroy wares.
===============
*/
void Warehouse::destroy_wares(int id, int count)
{
	m_wares.remove(id, count);
	
	assert(m_flag);
	Economy *e = m_flag->get_economy();
	
	e->add_wares(id, count);
}

/*
===============
Warehouse_Descr::create_object
===============
*/
Building *Warehouse_Descr::create_object()
{
	return new Warehouse(this);
}


/*
==============================================================================

Building_Descr Factory

==============================================================================
*/

/*
===============
Building_Descr::create_from_dir

Open the appropriate configuration file and check if a building description
is there.

May return 0.
===============
*/
Building_Descr *Building_Descr::create_from_dir(Tribe_Descr *tribe, const char *directory,
		                                          const EncodeData *encdata)
{
	const char *name;
	
	// name = last element of path
	const char *slash = strrchr(directory, '/');
	const char *backslash = strrchr(directory, '\\');
	
	if (backslash && (!slash || backslash > slash))
		slash = backslash;
	
	if (slash)
		name = slash+1;
	else
		name = directory;

	// Open the config file
	Building_Descr *descr = 0;
	char fname[256];
	
	snprintf(fname, sizeof(fname), "%s/conf", directory);
	
	if (!g_fs->FileExists(fname))
		return 0;
		
	try
	{
		Profile prof(fname, "global"); // section-less file
		Section *s = prof.get_safe_section("global");
		const char *type = s->get_safe_string("type");
		
		if (!strcasecmp(type, "warehouse"))
			descr = new Warehouse_Descr(tribe, name);
		else
			throw wexception("Unknown building type '%s'", type);
		
		descr->parse(directory, &prof, encdata);
		prof.check_used();
	}
	catch(std::exception &e) {
		if (descr)
			delete descr;
		throw wexception("Error reading building %s: %s", name, e.what());
	}
	catch(...) {
		if (descr)
			delete descr;
		throw;
	}
	
	return descr;
}



#if 0
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


#endif
