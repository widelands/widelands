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
#include "graphic.h"
#include "descr_maintainer.h"
#include "pic.h"
#include "bob.h"
#include "tribedata.h"
#include "ware.h"
#include "worker.h"
#include "tribe.h"

//
// Tribe_Descr class
// 
Tribe_Descr::Tribe_Descr(void) {
}
Tribe_Descr::~Tribe_Descr(void) {
}

void Tribe_Descr::load(const char* name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);
	
	try
	{
		char directory[256];

		snprintf(directory, sizeof(directory), "tribes/%s", name);
		
		m_default_encdata.clear();
		parse_root_conf(directory);
		parse_buildings(directory);
		//parse_workers(directory);
		//parse_soldiers(directory);
		//parse_wares(directory); //???
	}
	catch(std::exception &e)
	{
		throw wexception("Error loading tribe %s: %s", name, e.what());
	}
}

//
// down here: private read functions for loading
//

/*
===============
Tribe_Descr::parse_root_conf

Read and process the main conf file
===============
*/
void Tribe_Descr::parse_root_conf(const char *directory)
{
	char fname[256];
	
	snprintf(fname, sizeof(fname), "%s/conf", directory);
	
	try
	{
		Profile prof(fname);
		Section *s;

		// Section [tribe]		
		s = prof.get_safe_section("tribe");
		
		s->get_string("author");
		s->get_string("name"); // descriptive name
		s->get_string("descr"); // long description
		
		// Section [defaults]
		s = prof.get_section("defaults");
		
		if (s)
			m_default_encdata.parse(s);
		
		// Section [regent]
		s = prof.get_safe_section("regent");
		
		s->get_string("name");
		s->get_string("pic_small");
		s->get_string("pic_big");
		
		// Section [frontier]
		s = prof.get_section("frontier");
		if (!s)
			throw wexception("Missing section [frontier]");
		
		m_anim_frontier.parse(directory, s, 0, &m_default_encdata);
		
		prof.check_used();
	}
	catch(std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
}
	

/*
===============
Tribe_Descr::parse_buildings

Read all the building descriptions
===============
*/
void Tribe_Descr::parse_buildings(const char *rootdir)
{
	char subdir[256];
	filenameset_t dirs;
	
	snprintf(subdir, sizeof(subdir), "%s/buildings", rootdir);
	
	g_fs->FindFiles(subdir, "*", &dirs);
	
	for(filenameset_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
		Building_Descr *descr = 0;

		try {
			descr = Building_Descr::create_from_dir(this, it->c_str(), &m_default_encdata);
		} catch(std::exception &e) {
			log("Building %s failed: %s (garbage directory?)\n", it->c_str(), e.what());
		} catch(...) {
			log("Building %s failed: unknown exception (garbage directory?)\n", it->c_str());
		}

		if (descr)
			buildings.add(descr);
	}
}


/*
===============
Tribe_Descr::parse_workers

Read all worker descriptions
===============
*/
void Tribe_Descr::parse_workers(const char *directory)
{
/*
	char directory[256];
	char fname[256];
	
	snprintf(directory, sizeof(directory), "%s/workers", rootdir);
	snprintf(fname, sizeof(fname), "%s/conf", directory);
	
	try
	{
		Profile prof(fname);
		Section *s, *defaults;
		EncodeData encdata;

		// Section [defaults]
		defaults = prof.get_safe_section("defaults");
		
		encdata = m_default_encdata;
		encdata.parse(defaults);
		
		// Parse all buildings
		while((s = prof.get_next_section(0))) {
			Worker_Descr *descr = 0;
			
			try {
				descr = Worker_Descr::create_from_section(this, directory, s, defaults, &encdata);
			} catch(std::exception &e) {
				log("Worker %s failed: %s\n", s->get_name(), e.what());
			} catch(...) {
				log("Worker %s failed: unknown exception\n", s->get_name());
			}
			
			if (descr)
				workers.add(descr);
		}
	}
	catch(std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
*/
}


/*
===============
Tribe_Descr::parse_soldiers

Parse soldier descriptions
===============
*/
void Tribe_Descr::parse_soldiers(const char *directory)
{
/*
	char directory[256];
	char fname[256];
	
	snprintf(directory, sizeof(directory), "%s/soldiers", rootdir);
	snprintf(fname, sizeof(fname), "%s/conf", directory);
	
	try
	{
		Profile prof(fname);
		Section *s, *defaults;
		EncodeData encdata;

		// Section [defaults]
		defaults = prof.get_safe_section("defaults");
		
		encdata = m_default_encdata;
		encdata.parse(defaults);
		
		// Parse all buildings
		while((s = prof.get_next_section(0))) {
			Soldier_Descr *descr = 0;
			
			try {
				descr = Soldier_Descr::create_from_section(this, directory, s, defaults, &encdata);
			} catch(std::exception &e) {
				log("Soldier %s failed: %s\n", s->get_name(), e.what());
			} catch(...) {
				log("Soldier %s failed: unknown exception\n", s->get_name());
			}
			
			if (descr)
				soldiers.add(descr);
		}
	}
	catch(std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
*/
}


/*
===============
Tribe_Descr::parse_wares

===============
*/
void Tribe_Descr::parse_wares(const char *directory)
{
/*
   // read magic
   if(strcasecmp(f->CString(), "Wares"))
		throw wexception("Wrong wares magic");

   ushort nware;
   nware = f->Unsigned16();
   
   Ware_Descr* ware;
	ushort *ptr;
   ushort w, h, clrkey;
   uint i;
   for(i=0; i<nware; i++) {
      ware=new Ware_Descr;
      memcpy(ware->name, f->Data(sizeof(ware->name)), sizeof(ware->name));
      w = f->Unsigned16();
      h = f->Unsigned16();
      clrkey = f->Unsigned16();

		ptr = (ushort*)f->Data(24*24*2);
      ware->menu_pic.create(24, 24, ptr);
		
      ptr = (ushort*)f->Data(w*h*sizeof(ushort));
      ware->pic.create(w, h, ptr);
      ware->menu_pic.set_clrkey(clrkey);
      ware->pic.set_clrkey(clrkey);
      wares.add(ware);
   }
*/
}


