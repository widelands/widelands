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
		parse_workers(directory);
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
		
		// Section [flag]
		s = prof.get_section("flag");
		if (!s)
			throw wexception("Missing section [flag]");
		
		m_anim_flag.parse(directory, s, 0, &m_default_encdata);
		
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
	char subdir[256];
	filenameset_t dirs;
	
	snprintf(subdir, sizeof(subdir), "%s/workers", directory);
	
	g_fs->FindFiles(subdir, "*", &dirs);
	
	for(filenameset_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
		Worker_Descr *descr = 0;

		try {
			descr = Worker_Descr::create_from_dir(this, it->c_str(), &m_default_encdata);
		} catch(std::exception &e) {
			log("Worker %s failed: %s (garbage directory?)\n", it->c_str(), e.what());
		} catch(...) {
			log("Worker %s failed: unknown exception (garbage directory?)\n", it->c_str());
		}

		if (descr)
			m_workers.add(descr);
	}
}

