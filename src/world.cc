/*
 * Copyright (C) 2002, 2004, 2006-2009 by the Widelands Development Team
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
#include "critter_bob.h"
#include "graphic.h"
#include "helper.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "parse_map_object_types.h"
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
	m_name = s->get_name();
	m_descname = s->get_string("name", s->get_name());
	m_is_detectable = s->get_bool("detectable", true);

	m_max_amount = s->get_safe_int("max_amount");
	while (Section::Value const * const v = s->get_next_val("editor_pic")) {
		Editor_Pic i;

		std::vector<std::string> const args(split_string(v->get_string(), " \t"));
		if (args.size() != 1 and args.size() != 2) {
			log
				("Resource '%s' has bad editor_pic=%s\n",
				 m_name.c_str(), v->get_string());
			continue;
		}

		i.picname = basedir + "/pics/";
		i.picname += args[0];
		i.upperlimit = -1;

		if (args.size() >= 2) {
			char * endp;
			i.upperlimit = strtol(args[1].c_str(), &endp, 0);

			if (*endp) {
				log
					("Resource '%s' has bad editor_pic=%s\n",
					 m_name.c_str(), v->get_string());
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

World::World(std::string const & name) : m_basedir("worlds/" + name + '/') {
	try {
		i18n::Textdomain textdomain(m_basedir);

		FileSystem & fs = *g_fs->MakeSubFileSystem(m_basedir);
		g_fs->AddFileSystem(&fs);

		{
			Profile root_conf((m_basedir + "conf").c_str());
			parse_root_conf(name, root_conf);
			parse_resources();
			parse_terrains();
			log("Parsing world bobs...\n");
			parse_bobs(m_basedir, root_conf);
			root_conf.check_used();
		}

		{ //  General bobs mainly for scenarios
			Profile global_root_conf("global/conf");
			std::string global_dir = "global/bobs/";
			log("Parsing global bobs in world...\n");
			parse_bobs(global_dir, global_root_conf);
			global_root_conf.check_used();
		}

		g_fs->RemoveFileSystem(&fs);
	} catch (std::exception const & e) {
		// tag with world name
		throw wexception("Error loading world %s: %s", name.c_str(), e.what());
	}
}


/*
===============
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
void World::parse_root_conf(std::string const & name, Profile & root_conf)
{
	Section & s = root_conf.get_safe_section("world");
	snprintf
		(hd.name,   sizeof(hd.name),   "%s", s.get_string("name", name.c_str()));
	snprintf
		(hd.author, sizeof(hd.author), "%s", s.get_safe_string("author"));
	snprintf
		(hd.descr,  sizeof(hd.descr),  "%s", s.get_safe_string("descr"));
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

		for (Terrain_Index i = 0;; ++i) {
			Section * const s = prof.get_next_section(0);
			if (not s)
				break;
			if (i == 0x10)
				throw wexception
					("%s: too many terrain types, can not be more than 16\n",
					 fname);
			ters.add(new Terrain_Descr(m_basedir.c_str(), s, &m_resources));
		}

		prof.check_used();
	}
	catch (std::exception &e) {
		throw wexception("%s: %s", fname, e.what());
	}
}

void World::parse_bobs(std::string & path, Profile & root_conf) {
	std::string::size_type const base_path_size = path.size();
	std::set<std::string> names; //  To enforce name uniqueness.

	PARSE_MAP_OBJECT_TYPES_BEGIN("immovable")
		immovables.add
			(new Immovable_Descr
			 	(_name, _descname, path, prof, global_s, *this, 0));
	PARSE_MAP_OBJECT_TYPES_END;

	PARSE_MAP_OBJECT_TYPES_BEGIN("critter bob")
		bobs      .add
			(new Critter_Bob_Descr
			 	(_name, _descname, path, prof, global_s, 0));
	PARSE_MAP_OBJECT_TYPES_END;
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

void World::get_all_worlds(std::vector<std::string> & result) {
	result.clear();

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
			result.push_back(world);
	}
}

int32_t World::safe_resource_index(const char * const resourcename) const {
	int32_t const result = get_resource(resourcename);

	if (result == -1)
		throw wexception
			("world %s does not define resource type \"%s\"",
			 get_name(), resourcename);
	return result;
}


/*
==============================================================================

Terrain_Descr

==============================================================================
*/

Terrain_Descr::Terrain_Descr
	(char                       const * const directory,
	 Section                          * const s,
	 Descr_Maintainer<Resource_Descr> * const resources)
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
				("terrain type %s has valid resource type %s, which does not "
				 "exist in world",
				 s->get_name(), resource.c_str());
		m_default_resources = res;
		m_default_amount    = amount;
	}

	//  parse valid resources
	std::string str1 = s->get_string("resources", "");
	if (str1 != "") {
		int32_t nres = 1;
		container_iterate_const(std::string, str1, i)
			if (*i.current == ',')
				++nres;

		m_nr_valid_resources = nres;
		m_valid_resources    = new uint8_t[nres];
		std::string curres;
		uint32_t i = 0;
		int32_t cur_res = 0;
		while (i <= str1.size()) {
			if (str1[i] == ' ' || str1[i] == ' ' || str1[i] == '\t') {
				++i;
				continue;
			}
			if (str1[i] == ',' || i == str1.size()) {
				const int32_t res = resources->get_index(curres.c_str());;
				if (res == -1)
					throw wexception
						("terrain type %s has valid resource type %s which does not "
						 "exist in world",
						 s->get_name(), curres.c_str());
				m_valid_resources[cur_res++] = res;
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
