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
#include "profile.h"
#include "pic.h"
#include "bob.h"
#include "worker.h"
#include "tribe.h"


/*
==============================================================================

Worker IMPLEMENTATION

==============================================================================
*/

/*
===============
Worker_Descr::Worker_Descr
Worker_Descr::~Worker_Descr
===============
*/
Worker_Descr::Worker_Descr(Tribe_Descr *tribe, const char *name)
	: Bob_Descr(name)
{
	m_tribe = tribe;
	m_menu_pic = 0;
}

Worker_Descr::~Worker_Descr(void)
{
	if (m_menu_pic)
		delete m_menu_pic;
}

/*
===============
Worker_Descr::parse

Parse the worker data from configuration
===============
*/
void Worker_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	const char *string;
	char buf[256];
	char fname[256];
	Section *s;

	Bob_Descr::parse(directory, prof, encdata);
	
	s = prof->get_safe_section("global");

	snprintf(buf, sizeof(buf),	"%s_menu.bmp", get_name());
	string = s->get_string("menu_pic", buf);
	
	snprintf(fname, sizeof(fname), "%s/%s", directory, string);
	m_menu_pic = new Pic;
	m_menu_pic->load(fname);
	
	if (m_menu_pic->get_w() != WARE_MENU_PIC_W || m_menu_pic->get_h() != WARE_MENU_PIC_H)
		throw wexception("%s: menu pic must be %ix%i pixels.", fname, WARE_MENU_PIC_W, WARE_MENU_PIC_H);
	
	// Read the walking animations
	s = prof->get_section("walk");
	m_walk_anims.parse(directory, prof, "walk_??", s, encdata);

	s = prof->get_section("walkload");
	m_walkload_anims.parse(directory, prof, "walkload_??", s, encdata);
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Worker::Worker
Worker::~Worker
===============
*/
Worker::Worker(Worker_Descr *descr)
	: Bob(descr)
{
	m_economy = 0;
	m_carried_ware = -1;
}

Worker::~Worker()
{
}


/*
===============
Worker::task_start_best

Give the worker something to do after the last task has finished.
===============
*/
void Worker::task_start_best(Game* g, uint prev, bool success, uint nexthint)
{
	start_task_idle(g, get_descr()->get_idle_anim(), -1);
}


/*
==============================================================================

Carrier IMPLEMENTATION
	
==============================================================================
*/

/*
===============
Carrier_Descr::Carrier_Descr
Carrier_Descr::~Carrier_Descr
===============
*/
Carrier_Descr::Carrier_Descr(Tribe_Descr *tribe, const char *name)
	: Worker_Descr(tribe, name)
{
}

Carrier_Descr::~Carrier_Descr(void)
{
}
	
/*
===============
Carrier_Descr::parse

Parse carrier-specific configuration data
===============
*/
void Carrier_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Worker_Descr::parse(directory, prof, encdata);
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Carrier::Carrier
Carrier::~Carrier
===============
*/
Carrier::Carrier(Carrier_Descr *descr)
	: Worker(descr)
{
}

Carrier::~Carrier()
{
}


/*
===============
Carrier_Descr::create_object

Create a carrier of this type.
===============
*/
Bob *Carrier_Descr::create_object()
{
	return new Carrier(this);
}

/*
==============================================================================

Worker factory
	
==============================================================================
*/

/*
===============
Worker_Descr::create_from_dir [static]

Automatically create the appropriate Worker_Descr type from the given
config data.
May return 0.
===============
*/
Worker_Descr *Worker_Descr::create_from_dir(Tribe_Descr *tribe, const char *directory, const EncodeData *encdata)
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
	Worker_Descr *descr = 0;
	char fname[256];
	
	snprintf(fname, sizeof(fname), "%s/conf", directory);
	
	if (!g_fs->FileExists(fname))
		return 0;
		
	try
	{
		Profile prof(fname);
		Section *s = prof.get_safe_section("global");
		const char *type = s->get_safe_string("type");
		
		if (!strcasecmp(type, "carrier"))
			descr = new Carrier_Descr(tribe, name);
		else
			throw wexception("Unknown carrier type '%s' [supported: carrier]", type);
		
		descr->parse(directory, &prof, encdata);
		prof.check_used();
	}
	catch(std::exception &e) {
		if (descr)
			delete descr;
		throw wexception("Error reading worker %s: %s", name, e.what());
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
// class Soldier_Descr
// 
int Soldier_Descr::read(FileRead* f)
{
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   energy = f->Unsigned16();
/*
   attack_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack_l.read(f);
   attack1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack1_l.read(f);
   evade_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade_l.read(f);
   evade1_l.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_l.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade1_l.read(f);

   attack_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack_r.read(f);
   attack1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   attack1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   attack1_r.read(f);
   evade_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade_r.read(f);
   evade1_r.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   evade1_r.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   evade1_r.read(f);
*/     
   return RET_OK;
}

// Worker class read functions
int Has_Working_Worker_Descr::read(FileRead* f) {
/*
   working.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   working.read(f);
*/
   return RET_OK;
}

int Has_Working1_Worker_Descr::read(FileRead* f) {
/*
   working1.set_dimensions(walk_ne.get_w(), walk_ne.get_h());
   working1.set_hotspot(walk_ne.get_hsx(), walk_ne.get_hsy());
   working1.read(f);
*/
   return RET_OK;
}
int Has_Walk1_Worker_Descr::read(FileRead* f) {
/*
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
   
   walk_ne1.read(f);
   walk_e1.read(f);
   walk_se1.read(f);
   walk_sw1.read(f);
   walk_w1.read(f);
   walk_nw1.read(f);
*/
   return RET_OK;
}
int Scientist::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   
   return RET_OK;
}

int Searcher::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);

   // nothing additional 
   return RET_OK;
}

int Grower::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Planter::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   
   return RET_OK;
}
int SitDigger_Base::read(FileRead* f) {
   // Nothing to do
   return RET_OK;
}
int SitDigger::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   
   // Nothing of our own stuff to read
   return RET_OK;
}

int Carrier::read(FileRead* f) {

   // nothing to do
   return RET_OK;
}

int Def_Carrier::read(FileRead* f) {
   Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}

int Add_Carrier::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Carrier::read(f);
   Has_Walk1_Worker_Descr::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);

   return RET_OK;
}
int Builder::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);
   
   return RET_OK;
}

int Planer::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
      
   return RET_OK;
}

int Explorer::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);

   return RET_OK;
}

int Geologist::read(FileRead* f) {
   Worker_Descr::read(f);
   Menu_Worker_Descr::read(f);
   SitDigger_Base::read(f);
   Has_Working_Worker_Descr::read(f);
   Has_Working1_Worker_Descr::read(f);
   
   return RET_OK;
}
#endif
