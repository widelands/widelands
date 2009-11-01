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
#include "logic/critter_bob.h"
#include "game_data_error.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "parse_map_object_types.h"
#include "profile/profile.h"
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
Parse a resource description section.
==============
*/
void Resource_Descr::parse(Section & s, std::string const & basedir)
{
	m_name = s.get_name();
	m_descname = s.get_string("name", s.get_name());
	m_is_detectable = s.get_bool("detectable", true);

	m_max_amount = s.get_safe_int("max_amount");
	while (Section::Value const * const v = s.get_next_val("editor_pic")) {
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
	if (m_editor_pics.empty())
		throw game_data_error("resource '%s' has no editor_pic", m_name.c_str());
}


/*
 * Get the correct editor pic for this amount of this resource
 */
const std::string & Resource_Descr::get_editor_pic
	(uint32_t const amount) const
{
	uint32_t bestmatch = 0;

	assert(m_editor_pics.size());

	for (uint32_t i = 1; i < m_editor_pics.size(); ++i) {
		const int32_t diff1 =
			m_editor_pics[bestmatch].upperlimit - static_cast<int32_t>(amount);
		const int32_t diff2 =
			m_editor_pics[i].upperlimit - static_cast<int32_t>(amount);

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
		if
			(m_editor_pics[bestmatch].upperlimit < 0     ||
			 diff2                               < diff1 ||
			 diff1                               < 0)
		{
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

Map Gen Info

=============================================================================
*/

int MapGenAreaInfo::split_string
	(std::vector<std::string> & strs, std::string & str)
{
	strs = ::split_string(str, ",");
	return strs.size();
}

void MapGenAreaInfo::readTerrains
	(std::vector<Terrain_Index> &       list,
	 Section                    &       s,
	 char const                 * const value_name)
{
	std::string str = s.get_string(value_name, "");
	if (str.empty())
		throw game_data_error
			("terrain info \"%s\" missing in section \"%s\" mapgenconf for world "
			 "\"%s\"",
			 value_name, s.get_name(), m_world->get_name());
	std::vector<std::string> strs;

	split_string(strs, str);

	for (uint32_t ix = 0; ix < strs.size(); ++ix) {
		Terrain_Index const tix = m_world->index_of_terrain(strs[ix].c_str());
		if (tix > 128)
			throw game_data_error
				("unknown terrain \"%s\" in section \"%s\" in mapgenconf for "
				 "world \"%s\"",
				 value_name, s.get_name(), m_world->get_name());
		list.push_back(tix);
	}
}


void MapGenAreaInfo::parseSection
	(World * const world, Section & s, MapGenAreaType const areaType)
{
	m_weight = s.get_positive("weight", 1);
	m_world = world;
	switch (areaType) {
		case atWater:
			readTerrains(m_Terrains1, s, "ocean_terrains");
			readTerrains(m_Terrains2, s, "shelf_terrains");
			readTerrains(m_Terrains3, s, "shallow_terrains");
			break;
		case atLand:
			readTerrains(m_Terrains1, s, "coast_terrains");
			readTerrains(m_Terrains2, s, "land_terrains");
			readTerrains(m_Terrains3, s, "upper_terrains");
			break;
		case atMountains:
			readTerrains(m_Terrains1, s, "mountainfoot_terrains");
			readTerrains(m_Terrains2, s, "mountain_terrains");
			readTerrains(m_Terrains3, s, "snow_terrains");
			break;
		case atWasteland:
			readTerrains(m_Terrains1, s, "inner_terrains");
			readTerrains(m_Terrains2, s, "outer_terrains");
			break;
	}
}

size_t MapGenAreaInfo::getNumTerrains(MapGenTerrainType const terrType)
{
	switch (terrType) {
		case ttWaterOcean:        return m_Terrains1.size();
		case ttWaterShelf:        return m_Terrains2.size();
		case ttWaterShallow:      return m_Terrains3.size();

		case ttLandCoast:         return m_Terrains1.size();
		case ttLandLand:          return m_Terrains2.size();
		case ttLandUpper:         return m_Terrains3.size();

		case ttWastelandInner:    return m_Terrains1.size();
		case ttWastelandOuter:    return m_Terrains2.size();

		case ttMountainsFoot:     return m_Terrains1.size();
		case ttMountainsMountain: return m_Terrains2.size();
		case ttMountainsSnow:     return m_Terrains3.size();

		default:                  return 0;
	}
}

Terrain_Index MapGenAreaInfo::getTerrain
	(MapGenTerrainType const terrType, uint32_t const index)
{
	switch (terrType) {
		case ttWaterOcean:        return m_Terrains1[index];
		case ttWaterShelf:        return m_Terrains2[index];
		case ttWaterShallow:      return m_Terrains3[index];

		case ttLandCoast:         return m_Terrains1[index];
		case ttLandLand:          return m_Terrains2[index];
		case ttLandUpper:         return m_Terrains3[index];

		case ttWastelandInner:    return m_Terrains1[index];
		case ttWastelandOuter:    return m_Terrains2[index];

		case ttMountainsFoot:     return m_Terrains1[index];
		case ttMountainsMountain: return m_Terrains2[index];
		case ttMountainsSnow:     return m_Terrains3[index];

		default:                  return 0;
	}
}

uint32_t MapGenInfo::getSumLandWeight()
{
	if (m_land_weight_valid)
		return m_land_weight;

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < getNumAreas(MapGenAreaInfo::atLand); ++ix)
		sum += getArea(MapGenAreaInfo::atLand, ix).getWeight();
	m_land_weight = sum;
	return m_land_weight;
}


size_t MapGenInfo::getNumAreas(MapGenAreaInfo::MapGenAreaType const areaType)
{
	switch (areaType) {
	case MapGenAreaInfo::atWater:     return m_WaterAreas    .size();
	case MapGenAreaInfo::atLand:      return m_LandAreas     .size();
	case MapGenAreaInfo::atMountains: return m_MountainAreas .size();
	case MapGenAreaInfo::atWasteland: return m_WasteLandAreas.size();
	}
	throw wexception("invalid MapGenAreaType %u", areaType);
}

MapGenAreaInfo & MapGenInfo::getArea
	(MapGenAreaInfo::MapGenAreaType const areaType, uint32_t const index)
{
	switch (areaType) {
	case MapGenAreaInfo::atWater:     return m_WaterAreas    .at(index);
	case MapGenAreaInfo::atLand:      return m_LandAreas     .at(index);
	case MapGenAreaInfo::atMountains: return m_MountainAreas .at(index);
	case MapGenAreaInfo::atWasteland: return m_WasteLandAreas.at(index);
	}
	throw wexception("invalid MapGenAreaType %u", areaType);
}

void MapGenInfo::parseProfile(World * const world, Profile & profile)
{
	m_world = world;
	m_land_weight_valid = false;

	{ //  find out about the general heights
		Section & s = profile.get_safe_section("heights");
		m_ocean_height        = s.get_positive("ocean",        10);
		m_shelf_height        = s.get_positive("shelf",        10);
		m_shallow_height      = s.get_positive("shallow",      10);
		m_coast_height        = s.get_positive("coast",        12);
		m_upperland_height    = s.get_positive("upperland",    16);
		m_mountainfoot_height = s.get_positive("mountainfoot", 18);
		m_mountain_height     = s.get_positive("mountain",     20);
		m_snow_height         = s.get_positive("snow",         33);
		m_summit_height       = s.get_positive("summit",       40);
	}


	//  read the area names
	Section & areas_s = profile.get_safe_section("areas");
	std::string              str;
	std::vector<std::string> water_strs;
	std::vector<std::string> land_strs;
	std::vector<std::string> wasteland_strs;
	std::vector<std::string> mountain_strs;

	str = areas_s.get_string("water", "water_area");
	MapGenAreaInfo::split_string(water_strs, str);

	str = areas_s.get_string("land", "land_area");
	MapGenAreaInfo::split_string(land_strs, str);

	str = areas_s.get_string("wasteland", "wasteland_area");
	MapGenAreaInfo::split_string(wasteland_strs, str);

	str = areas_s.get_string("mountains", "mountains_area");
	MapGenAreaInfo::split_string(mountain_strs, str);

	for (uint32_t ix = 0; ix < water_strs.size(); ++ix) {
		Section & s = profile.get_safe_section(water_strs[ix].c_str());
		MapGenAreaInfo info;
		info.parseSection(m_world, s, MapGenAreaInfo::atWater);
		m_WaterAreas.push_back(info);
	}

	for (uint32_t ix = 0; ix < land_strs.size(); ++ix) {
		Section & s = profile.get_safe_section(land_strs[ix].c_str());
		MapGenAreaInfo info;
		info.parseSection(m_world, s, MapGenAreaInfo::atLand);
		m_LandAreas.push_back(info);
	}

	for (uint32_t ix = 0; ix < wasteland_strs.size(); ++ix) {
		Section & s = profile.get_safe_section(wasteland_strs[ix].c_str());
		MapGenAreaInfo info;
		info.parseSection(m_world, s, MapGenAreaInfo::atWasteland);
		m_WasteLandAreas.push_back(info);
	}

	for (uint32_t ix = 0; ix < mountain_strs.size(); ++ix) {
		Section & s = profile.get_safe_section(mountain_strs[ix].c_str());
		MapGenAreaInfo info;
		info.parseSection(m_world, s, MapGenAreaInfo::atMountains);
		m_MountainAreas.push_back(info);
	}

}

/*
=============================================================================

World

=============================================================================
*/

World::World(std::string const & name) : m_basedir("worlds/" + name + '/') {
	try {
		i18n::Textdomain textdomain(m_basedir);

		FileSystem & fs = g_fs->MakeSubFileSystem(m_basedir);
		g_fs->AddFileSystem(fs);

		{
			Profile root_conf((m_basedir + "conf").c_str());
			parse_root_conf(name, root_conf);
			parse_resources();
			parse_terrains();
			parse_mapgen();
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

		g_fs->RemoveFileSystem(fs);
	} catch (std::exception const & e) {
		throw game_data_error("world %s: %s", name.c_str(), e.what());
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
			descr.parse(*section, m_basedir);
			m_resources.add(&descr);
		}
	}
	catch (std::exception const & e) {
		throw game_data_error("%s: %s", fname, e.what());
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
				throw game_data_error
					(_("%s: too many terrain types, can not be more than 16\n"),
					 fname);
			ters.add(new Terrain_Descr(m_basedir.c_str(), s, &m_resources));
		}

		prof.check_used();
	}
	catch (game_data_error const & e) {
		throw game_data_error("%s: %s", fname, e.what());
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

void World::parse_mapgen   ()
{
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/mapgenconf", m_basedir.c_str());

	try
	{
		Profile prof(fname);

		m_mapGenInfo.parseProfile(this, prof);

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atWater) < 1)
			throw game_data_error("missing a water area");

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atWater) < 1)
			throw game_data_error("too many water areas (>3)");

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atLand) < 1)
			throw game_data_error("missing a land area");

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atLand) > 3)
			throw game_data_error("too many land areas (>3)");

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atWasteland) < 1)
			throw game_data_error("missing a wasteland area");

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atWasteland) > 2)
			throw game_data_error("too many wasteland areas (>2)");

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atMountains) < 1)
			throw game_data_error("missing a mountain area");

		if (m_mapGenInfo.getNumAreas(MapGenAreaInfo::atMountains) < 1)
			throw game_data_error("too many mountain areas (>1)");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWaterOcean) < 1)
			throw game_data_error("missing a water/ocean terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWaterShelf)
			 <
			 1)
			throw game_data_error("missing a water/shelf terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atWater, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWaterShallow)
			 <
			 1)
			throw game_data_error("is missing a water/shallow terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atLand, 0).getNumTerrains
			 	(MapGenAreaInfo::ttLandCoast)
			 <
			 1)
			throw game_data_error("missing a land/coast terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atLand, 0).getNumTerrains
			 	(MapGenAreaInfo::ttLandLand)
			 <
			 1)
			throw game_data_error("missing a land/land terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
			 	(MapGenAreaInfo::ttMountainsFoot)
			 <
			 1)
			throw game_data_error("missing a mountain/foot terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
			 	(MapGenAreaInfo::ttMountainsMountain)
			 <
			 1)
			throw game_data_error("missing a monutain/mountain terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains
			 	(MapGenAreaInfo::ttMountainsSnow)
			 <
			 1)
			throw game_data_error("missing a mountain/snow terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWastelandInner)
			 <
			 1)
			throw game_data_error("missing a land/coast terrain type");

		if
			(m_mapGenInfo.getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains
			 	(MapGenAreaInfo::ttWastelandOuter)
			 <
			 1)
			throw game_data_error("missing a land/land terrain type");

		prof.check_used();
	} catch (_wexception const & e) {
		throw game_data_error("%s: %s", fname, e.what());
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
			 	(&g_fs->MakeSubFileSystem("worlds/" + worldname)),
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
		throw game_data_error
			("world %s does not define resource type \"%s\"",
			 get_name(), resourcename);
	return result;
}


MapGenInfo & World::getMapGenInfo()
{
	return m_mapGenInfo;
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
			throw game_data_error
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
					throw game_data_error
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
			throw game_data_error("%s: invalid type '%s'", m_name.c_str(), is);
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
	free(m_picnametempl);
	delete[] m_valid_resources;
	m_nr_valid_resources = 0;
	m_valid_resources    = 0;
}


/*
===============
Trigger load of the actual animation frames.
===============
*/
void Terrain_Descr::load_graphics()
{
	if (m_picnametempl)
		m_texture = g_gr->get_maptexture(*m_picnametempl, m_frametime);
}

}
