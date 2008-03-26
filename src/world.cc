/*
 * Copyright (C) 2002, 2004, 2006-2008 by the Widelands Development Team
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

#include "world.h"

#include "constants.h"
#include "graphic.h"
#include "helper.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "profile.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "worlddata.h"

#include <iostream>
#include <sstream>

using std::cerr;
using std::endl;


namespace Widelands {

/*
==============
Resource_Descr::parse

Parse a resource description section.
==============
*/
void Resource_Descr::parse(Section *s, std::string basedir)
{
	const char * string;

	m_name = s->get_name();
	m_descrname = s->get_string("name", s->get_name());
	m_is_detectable = s->get_bool("detectable", true);

	m_max_amount = s->get_safe_int("max_amount");
	while (s->get_next_string("editor_pic", &string)) {
		Editor_Pic i;

		const std::vector<std::string> args(split_string(string, " \t"));
		if (args.size() != 1 and args.size() != 2) {
			log("Resource '%s' has bad editor_pic=%s\n", m_name.c_str(), string);
			continue;
		}

		i.picname = basedir + "/pics/";
		i.picname += args[0];
		i.upperlimit = -1;

		if (args.size() >= 2) {
			char* endp;

			i.upperlimit = strtol(args[1].c_str(), &endp, 0);

			if (endp && *endp) {
				log
					("Resource '%s' has bad editor_pic=%s\n",
					 m_name.c_str(), string);
				continue;
			}
		}

		m_editor_pics.push_back(i);
	}
	if (!m_editor_pics.size())
		throw wexception("Resource '%s' has no editor_pic", m_name.c_str());
}


/*
 * Get the correct editor pic for this amount of this resource
 */
const std::string & Resource_Descr::get_editor_pic(const uint32_t amount) const {
	uint32_t bestmatch = 0;

	assert(m_editor_pics.size());

	for (uint32_t i = 1; i < m_editor_pics.size(); ++i)
	{
		const int32_t diff1 =
			m_editor_pics[bestmatch].upperlimit - static_cast<int32_t>(amount);
		const int32_t diff2 = m_editor_pics[i].upperlimit - static_cast<int32_t>(amount);

		// This is a catch-all for high amounts
		if (m_editor_pics[i].upperlimit < 0)
		{
			if (diff1 < 0) {
				bestmatch = i;
				continue;
			}

			continue;
		}

		// This is lower than the actual amount
		if (diff2 < 0)
		{
			if (m_editor_pics[bestmatch].upperlimit < 0)
				continue;

			if (diff1 < diff2) {
				bestmatch = i; // still better than previous best match
				continue;
			}

			continue;
		}

		// This is higher than the actual amount
		if (m_editor_pics[bestmatch].upperlimit < 0 || diff1 > diff2 || diff1 < 0) {
			bestmatch = i;
			continue;
		}
	}

	//noLog("Resource(%s): Editor_Pic '%s' for amount = %u\n",
	//m_name.c_str(), m_editor_pics[bestmatch].picname.c_str(), amount);

	return m_editor_pics[bestmatch].picname;
}


/*
=============================================================================

World

=============================================================================
*/

World::World(std::string const & name) : m_basedir("worlds/" + name) {
	try {
		i18n::Textdomain textdomain(m_basedir);

		FileSystem & fs = *g_fs->MakeSubFileSystem(m_basedir);
		g_fs->AddFileSystem(&fs);

		parse_root_conf(name.c_str());
		parse_resources();
		parse_terrains();
		parse_bobs();

		g_fs->RemoveFileSystem(&fs);
	} catch (std::exception const & e) {
		// tag with world name
		throw wexception("Error loading world %s: %s", name.c_str(), e.what());
	}
}


/*
===============
World::postload

Load all logic game data now
===============
*/
void World::postload(Editor_Game_Base *) {
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
	int32_t i;

	// Load terrain graphics
	for (i = 0; i < ters.get_nitems(); ++i)
		ters.get(i)->load_graphics();

	// TODO: load more graphics
}


//
// down here: Private functions for loading
//

/**
 * Read the <world-directory>/conf
 */
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
	catch (std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
}

void World::parse_resources()
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/resconf", m_basedir.c_str());

	try {
		Profile prof(fname);
		while (Section * const section = prof.get_next_section(0)) {
			Resource_Descr & descr = *new Resource_Descr();
			descr.parse(section, m_basedir);
			m_resources.add(&descr);
		}
	}
	catch (std::exception &e) {
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

		for (Terrain_Descr::Index i = 0; (s = prof.get_next_section(0)); ++i) {
			if (i == 0x10)
				throw wexception
					("%s: too many terrain types, can not be more than 16\n", fname);
			ters.add(new Terrain_Descr(m_basedir.c_str(), s, &m_resources));
		}

		prof.check_used();
	}
	catch (std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
}

void World::parse_bobs()
{
	char subdir[256];
	filenameset_t dirs;

	snprintf(subdir, sizeof(subdir), "%s/bobs", m_basedir.c_str());

	g_fs->FindFiles(subdir, "*", &dirs);

	for (filenameset_t::iterator it = dirs.begin(); it != dirs.end(); ++it) {
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
				Bob::Descr *descr;
				descr = Bob::Descr::create_from_dir(name, it->c_str(), &prof, 0);
				bobs.add(descr);
			} else {
				Immovable_Descr * const descr = new Immovable_Descr(0, name);
				descr->parse(it->c_str(), &prof);
				immovables.add(descr);
			}
		} catch (std::exception &e) {
			cerr << it->c_str() << ": " << e.what() << " (garbage directory?)" << endl;
		} catch (...) {
			cerr << it->c_str() << ": unknown exception (garbage directory?)" << endl;
		}
	}
}

/**
 * Check if the world data can actually be read
 */
bool World::exists_world(std::string worldname)
{
	FileRead f;
	return
		f.TryOpen
		(*
		 std::auto_ptr<FileSystem>
		 	(g_fs->MakeSubFileSystem("worlds/" + worldname)),
		 "conf");
}

/*
 * World::get_all_worlds()
 */
void World::get_all_worlds(std::vector<std::string>* retval) {
	retval->clear();

	//  get all worlds
	filenameset_t m_worlds;
	g_fs->FindFiles("worlds", "*", &m_worlds);
	for
		(filenameset_t::iterator pname = m_worlds.begin();
		 pname != m_worlds.end();
		 ++pname)
	{
		std::string world = *pname;
		world.erase(0, 7); //  remove worlds/
		if (World::exists_world(world.c_str()))
			retval->push_back(world);
	}
}


/*
==============================================================================

Terrain_Descr

==============================================================================
*/

Terrain_Descr::Terrain_Descr(const char* directory, Section* s, Descr_Maintainer<Resource_Descr>* resources)
:
m_name              (s->get_name()),
m_picnametempl      (0),
m_frametime         (FRAME_LENGTH),
m_valid_resources   (0),
m_nr_valid_resources(0),
m_default_resources (-1),
m_default_amount    (0),
m_texture           (0)
{

	// Parse the default resource
	if (const char * str = s->get_string("def_resources", 0)) {
		std::istringstream str1(str);
		std::string resource;
		int32_t amount;
		str1 >> resource >> amount;
		int32_t const res = resources->get_index(resource.c_str());;
		if (res == -1)
			throw wexception
				("Terrain %s has valid resource %s which doesn't exist in world!",
				 s->get_name(), resource.c_str());
		m_default_resources = res;
		m_default_amount    = amount;
	}

	//  parse valid resources
	std::string str1 = s->get_string("resources", "");
	if (str1 != "") {
		int32_t nres = 1;
		const std::string::const_iterator str1_end = str1.end();
		for (std::string::const_iterator it = str1.begin(); it != str1_end; ++it)
			if (*it == ',') ++nres;

		m_nr_valid_resources =nres;
		m_valid_resources    = new uint8_t[nres];
		std::string curres;
		uint32_t i = 0;
		int32_t cur_res = 0;
		while (i <= str1.size()) {
			if (str1[i] == ' ' || str1[i] == ' ' || str1[i]=='\t') {
				++i;
				continue;
			}
			if (str1[i] == ',' || i == str1.size()) {
				const int32_t res = resources->get_index(curres.c_str());;
				if (res == -1)
					throw wexception
						("Terrain %s has valid resource %s which doesn't exist in "
						 "world!",
						 s->get_name(), curres.c_str());
				m_valid_resources[cur_res++]=res;
				curres = "";
			} else
				curres.append(1, str1[i]);
			++i;
		}
	}

	int32_t fps = s->get_int("fps");
	if (fps > 0)
		m_frametime = 1000 / fps;

	{
		const char * const is = s->get_safe_string("is");
		if      (not strcmp(is, "dry"))
			m_is = TERRAIN_DRY;
		else if (not strcmp(is, "green"))
			m_is = 0;
		else if (not strcmp(is, "water"))
			m_is = TERRAIN_WATER|TERRAIN_DRY|TERRAIN_UNPASSABLE;
		else if (not strcmp(is, "acid"))
			m_is = TERRAIN_ACID|TERRAIN_DRY|TERRAIN_UNPASSABLE;
		else if (not strcmp(is, "mountain"))
			m_is = TERRAIN_DRY|TERRAIN_MOUNTAIN;
		else if (not strcmp(is, "dead"))
			m_is = TERRAIN_DRY|TERRAIN_UNPASSABLE|TERRAIN_ACID;
		else if (not strcmp(is, "unpassable"))
			m_is = TERRAIN_DRY|TERRAIN_UNPASSABLE;
		else
			throw wexception("%s: invalid type '%s'", m_name.c_str(), is);
	}

	// Determine template of the texture animation pictures
	char fnametmpl[256];

	if (const char * const texture = s->get_string("texture", 0))
		snprintf(fnametmpl, sizeof(fnametmpl), "%s/%s", directory, texture);
	else
		snprintf
			(fnametmpl, sizeof(fnametmpl),
			 "%s/pics/%s_??.png", directory, m_name.c_str());

	m_picnametempl = strdup(fnametmpl);
}

Terrain_Descr::~Terrain_Descr()
{
	if (m_picnametempl)
		free(m_picnametempl);
	delete[] m_valid_resources;
	m_nr_valid_resources = 0;
	m_valid_resources    = 0;
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
		m_texture = g_gr->get_maptexture(*m_picnametempl, m_frametime);
}

};
