/*
 * Copyright (C) 2002, 2004 by the Widelands Development Team
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

#include <iostream>
#include "constants.h"
#include "filesystem.h"
#include "graphic.h"
#include "profile.h"
#include "wexception.h"
#include "world.h"
#include "worlddata.h"
#include "util.h"
#include "error.h"

using std::cerr;
using std::endl;

/*
=============================================================================

Resource_Descr

=============================================================================
*/

/*
==============
Resource_Descr::parse

Parse a resource description section.
==============
*/
void Resource_Descr::parse(Section *s)
{
	const char* string;

	m_name = s->get_string("name", s->get_name());

	while(s->get_next_string("indicator", &string))
	{
		std::vector<std::string> args;
		Indicator i;

		split_string(string, &args, " \t");

		if (args.size() != 1 && args.size() != 2)
		{
			log("Resource '%s' has bad indicator=%s\n", m_name.c_str(), string);
			continue;
		}

		i.bobname = args[0];
		i.upperlimit = -1;

		if (args.size() >= 2)
		{
			char* endp;

			i.upperlimit = strtol(args[1].c_str(), &endp, 0);

			if (endp && *endp)
			{
				log("Resource '%s' has bad indicator=%s\n", m_name.c_str(), string);
				continue;
			}
		}

		m_indicators.push_back(i);
	}

	if (!m_indicators.size())
		throw wexception("Resource '%s' has no indicators", m_name.c_str());
}


/*
==============
Resource_Descr::get_indicator

Find the best matching indicator for the given amount.
==============
*/
std::string Resource_Descr::get_indicator(uint amount) const
{
	uint bestmatch = 0;

	assert(m_indicators.size());

	for(uint i = 1; i < m_indicators.size(); ++i)
	{
		int diff1 = m_indicators[bestmatch].upperlimit - (int)amount;
		int diff2 = m_indicators[i].upperlimit - (int)amount;

		// This indicator is a catch-all for high amounts
		if (m_indicators[i].upperlimit < 0)
		{
			if (diff1 < 0) {
				bestmatch = i;
				continue;
			}

			continue;
		}

		// This indicator is lower than the actual amount
		if (diff2 < 0)
		{
			if (m_indicators[bestmatch].upperlimit < 0)
				continue;

			if (diff1 < diff2) {
				bestmatch = i; // still better than previous best match
				continue;
			}

			continue;
		}

		// This indicator is higher than the actual amount
		if (m_indicators[bestmatch].upperlimit < 0 || diff1 > diff2) {
			bestmatch = i;
			continue;
		}
	}

	log("Resource(%s): Indicator '%s' for amount = %u\n",
		m_name.c_str(), m_indicators[bestmatch].bobname.c_str(), amount);

	return m_indicators[bestmatch].bobname;
}


/*
=============================================================================

World

=============================================================================
*/

World::World(const char* name)
{
	char directory[256];

	try
	{
		snprintf(directory, sizeof(directory), "worlds/%s", name);
		m_basedir = directory;

		parse_root_conf(name);
		parse_resources();
		parse_terrains();
		parse_bobs();
	}
	catch(std::exception &e)
	{
		// tag with world name
		throw wexception("Error loading world %s: %s", name, e.what());
	}
}

World::~World(void)
{

}


/*
===============
World::postload

Load all logic game data now
===============
*/
void World::postload(Editor_Game_Base* g)
{
	// TODO: move more loads to postload
}


/*
===============
World::load_graphics

Load graphics data here
===============
*/
void World::load_graphics()
{
	int i;

	// Load terrain graphics
	for(i = 0; i < ters.get_nitems(); i++)
		ters.get(i)->load_graphics();

	// TODO: load more graphics
}


//
// down here: Private functions for loading
//

//
// read the <world-directory>/conf
//
void World::parse_root_conf(const char *name)
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/conf", m_basedir.c_str());

	try
	{
		Profile prof(fname);
		Section* s;

		s = prof.get_safe_section("world");

		const char* str;

		str = s->get_string("name", name);
		snprintf(hd.name, sizeof(hd.name), "%s", str);

		str = s->get_safe_string("author");
		snprintf(hd.author, sizeof(hd.author), "%s", str);

		str = s->get_safe_string("descr");
		snprintf(hd.descr, sizeof(hd.descr), "%s", str);

		prof.check_used();
	}
	catch(std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
}

void World::parse_resources()
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/resconf", m_basedir.c_str());

	try
	{
		Profile prof(fname);

		m_resources[Resource_None].parse(prof.pull_section("none"));
		m_resources[Resource_Coal].parse(prof.pull_section("coal"));
		m_resources[Resource_Iron].parse(prof.pull_section("iron"));
		m_resources[Resource_Gold].parse(prof.pull_section("gold"));
	}
	catch(std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
}

void World::parse_terrains()
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/terrainconf", m_basedir.c_str());

	try
	{
		Profile prof(fname);
		Section* s;

		while((s = prof.get_next_section(0)))
		{
			Terrain_Descr *ter = new Terrain_Descr(m_basedir.c_str(), s);
			ters.add(ter);
		}

		prof.check_used();
	}
	catch(std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
}

void World::parse_bobs()
{
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/bobs", m_basedir.c_str());

	g_fs->FindFiles(subdir, "*", &dirs);

	for(filenameset_t::iterator it = dirs.begin(); it != dirs.end(); it++) {
		char fname[256];

		snprintf(fname, sizeof(fname), "%s/conf", it->c_str());

		if (!g_fs->FileExists(fname))
			continue;

		const char *name;
		const char *slash = strrchr(it->c_str(), '/');
		const char *backslash = strrchr(it->c_str(), '\\');

		if (backslash && (!slash || backslash > slash))
			slash = backslash;

		if (slash)
			name = slash+1;
		else
			name = it->c_str();

		try
		{
			Profile prof(fname, "global"); // section-less file
			Section *s = prof.get_safe_section("global");
			const char *type = s->get_safe_string("type");

			if (!strcasecmp(type, "critter")) {
				Bob_Descr *descr;
				descr = Bob_Descr::create_from_dir(name, it->c_str(), &prof);
				bobs.add(descr);
			} else {
				Immovable_Descr *descr = new Immovable_Descr(name);
				descr->parse(it->c_str(), &prof);
				immovables.add(descr);
			}
		} catch(std::exception &e) {
			cerr << it->c_str() << ": " << e.what() << " (garbage directory?)" << endl;
		} catch(...) {
			cerr << it->c_str() << ": unknown exception (garbage directory?)" << endl;
		}
	}
}

/*
 * World::exists_world()
 */
bool World::exists_world(std::string worldname) {
   std::string buf;
   buf="worlds/" + worldname + "/conf";;

   FileRead f;
   return f.TryOpen(g_fs, buf.c_str());
}

/*
 * World::get_all_worlds()
 */
void World::get_all_worlds(std::vector<std::string>* retval) {
   retval->resize(0);

   // get all worlds
   filenameset_t m_worlds;
   g_fs->FindFiles("worlds", "*", &m_worlds);
   for(filenameset_t::iterator pname = m_worlds.begin(); pname != m_worlds.end(); pname++) {
      std::string world=*pname;
      world.erase(0,7); // remove worlds/
      if(World::exists_world(world.c_str()))
         retval->push_back(world);
   }
}


/*
==============================================================================

Terrain_Descr

==============================================================================
*/

Terrain_Descr::Terrain_Descr(const char* directory, Section* s)
{
	const char *str;

	// Set some defaults
	m_texture = 0;
 	m_frametime = FRAME_LENGTH;
	m_picnametempl = 0;

	// Read configuration
	snprintf(m_name, sizeof(m_name), "%s", s->get_name());

	// TODO: Implement the following fields
	// def_res = water|fish   (example)
	str = s->get_string("def_res", 0);
	if (str) {
		// def_stock = 10  (count)
		s->get_int("def_stock", 0);
	}
	s->get_string("resources");

	int fps = s->get_int("fps");
	if (fps > 0)
		m_frametime = 1000 / fps;

	// switch is
	str = s->get_safe_string("is");

	if(!strcasecmp(str, "dry")) {
		m_is = TERRAIN_DRY;
	} else if(!strcasecmp(str, "green")) {
		m_is = 0;
	} else if(!strcasecmp(str, "water")) {
		m_is = TERRAIN_WATER|TERRAIN_DRY|TERRAIN_UNPASSABLE;
	} else if(!strcasecmp(str, "acid")) {
		m_is = TERRAIN_ACID|TERRAIN_DRY|TERRAIN_UNPASSABLE;
	} else if(!strcasecmp(str, "mountain")) {
		m_is = TERRAIN_DRY|TERRAIN_MOUNTAIN;
	} else if(!strcasecmp(str, "dead")) {
		m_is = TERRAIN_DRY|TERRAIN_UNPASSABLE|TERRAIN_ACID;
	} else if(!strcasecmp(str, "unpassable")) {
		m_is = TERRAIN_DRY|TERRAIN_UNPASSABLE;
	} else
		throw wexception("%s: invalid type '%s'", m_name, str);

	// Determine template of the texture animation pictures
	char fnametmpl[256];

	str = s->get_string("texture", 0);
	if (str)
		snprintf(fnametmpl, sizeof(fnametmpl), "%s/%s", directory, str);
	else
		snprintf(fnametmpl, sizeof(fnametmpl), "%s/pics/%s_??.bmp", directory, m_name);

	m_picnametempl = strdup(fnametmpl);
}

Terrain_Descr::~Terrain_Descr()
{
	if (m_picnametempl)
		free(m_picnametempl);
}


/*
===============
Terrain_Descr::load_graphics

Trigger load of the actual animation frames.
===============
*/
void Terrain_Descr::load_graphics()
{
	if (m_picnametempl)
		m_texture = g_gr->get_maptexture(m_picnametempl, m_frametime);
}

/* "attic" -- re-add this in when it's needed

	// parse resources
   str=s->get_string("resources", 0);
   if(str && strcasecmp("", str)) {
      nres=1;
      uint i=0;
      while(i < strlen(str)) { if(str[i]==',') { nres++; }  i++; }

      res=new uchar[nres];
      char temp[200];
      uint n=0;
      uint cur_res=0;
      i=0;
      Resource_Descr* rtemp;
      while(i<strlen(str)) {
         temp[n]=str[i];
         i++;
         n++;
         if(str[i]==',') {
            temp[n]='\0';
            n--;
            while(temp[n] == ',' || temp[n]==' ' || temp[n]=='\t') temp[n--]='\0';
            uint z=0;
            while(temp[z] == ' ' || temp[z] == '\t') z++;
            n=0;
            i++;
            rtemp=resf.exists(temp+z);
            if(!rtemp) {
               strcpy(err_sec,s->get_name());
               strcpy(err_key,"resource");
               strcpy(err_msg, temp+z);
               strcat(err_msg,": Resource does not exist!");
               return ERROR;
            }
            res[cur_res++]=resf.get_index(rtemp->get_name());
         }
      }
      temp[n]='\0';
      n--;
      while(temp[n] == ',' || temp[n]==' ' || temp[n]=='\t') temp[n--]='\0';
      uint z=0;
      while(temp[z] == ' ' || temp[z] == '\t') z++;
      rtemp=resf.exists(temp+z);
      if(!rtemp) {
         strcpy(err_sec,s->get_name());
         strcpy(err_key,"resource");
         strcpy(err_msg, temp+z);
         strcat(err_msg,": Resource does not exist!");
         return ERROR;
      }
      res[cur_res++]=resf.get_index(rtemp->get_name());
   }
*/
