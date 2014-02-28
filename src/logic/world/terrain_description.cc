/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include <sstream>

#include <boost/foreach.hpp>

#include "logic/world/terrain_description.h"

#include "constants.h"
#include "graphic/graphic.h"
#include "logic/game_data_error.h"
#include "logic/world/data.h"
#include "profile/profile.h"

namespace Widelands {

TerrainDescription::TerrainDescription(const std::string& directory,
		// NOCOM(#sirver): can this be const ref?
                             Section* const s,
									  // NOCOM(#sirver): this should be const ref too?
                             Descr_Maintainer<ResourceDescription>* const resources)
   : name_(s->get_name()),
     descname_(s->get_string("name", s->get_name())),
     frametime_(FRAME_LENGTH),
     dither_layer_(0),
     valid_resources_(nullptr),
     nr_valid_resources_(0),
     default_resources_(-1),
     default_amount_(0),
     texture_(0) {

	// Parse the default resource
	if (const char* str = s->get_string("def_resources", nullptr)) {
		std::istringstream str1(str);
		std::string resource;
		int32_t amount;
		str1 >> resource >> amount;
		int32_t const res = resources->get_index(resource.c_str());
		;
		if (res == -1)
			throw game_data_error("terrain type %s has valid resource type %s, which does not "
			                      "exist in world",
			                      s->get_name(),
			                      resource.c_str());
		default_resources_ = res;
		default_amount_ = amount;
	}

	//  parse valid resources
	std::string str1 = s->get_string("resources", "");
	if (str1 != "") {
		int32_t nres = 1;
		BOOST_FOREACH(const char chr, str1) {
			if (chr == ',') {
				++nres;
			}
		}

		nr_valid_resources_ = nres;
		valid_resources_ = new uint8_t[nres];
		std::string curres;
		int32_t cur_res = 0;
		for (uint32_t i = 0; i <= str1.size(); ++i) {
			if (i == str1.size() || str1[i] == ',') {
				const int32_t res = resources->get_index(curres.c_str());
				if (res == -1)
					throw game_data_error("terrain type %s has valid resource type %s which does not "
					                      "exist in world",
					                      s->get_name(),
					                      curres.c_str());
				valid_resources_[cur_res++] = res;
				curres = "";
			} else if (str1[i] != ' ' && str1[i] != '\t') {
				curres.append(1, str1[i]);
			}
		}
	}

	int32_t fps = s->get_int("fps");
	if (fps > 0)
		frametime_ = 1000 / fps;

	{
		const char* const is = s->get_safe_string("is");
		if (not strcmp(is, "dry"))
			is_ = TERRAIN_DRY;
		else if (not strcmp(is, "green"))
			is_ = 0;
		else if (not strcmp(is, "water"))
			is_ = TERRAIN_WATER | TERRAIN_DRY | TERRAIN_UNPASSABLE;
		else if (not strcmp(is, "acid"))
			is_ = TERRAIN_ACID | TERRAIN_DRY | TERRAIN_UNPASSABLE;
		else if (not strcmp(is, "mountain"))
			is_ = TERRAIN_DRY | TERRAIN_MOUNTAIN;
		else if (not strcmp(is, "dead"))
			is_ = TERRAIN_DRY | TERRAIN_UNPASSABLE | TERRAIN_ACID;
		else if (not strcmp(is, "unpassable"))
			is_ = TERRAIN_DRY | TERRAIN_UNPASSABLE;
		else
			throw game_data_error("%s: invalid type '%s'", name_.c_str(), is);
	}

	dither_layer_ = s->get_int("dither_layer", 0);

	// Determine template of the texture animation pictures
	char fnametmpl[256];

	if (const char* const texture = s->get_string("texture", nullptr))
		snprintf(fnametmpl, sizeof(fnametmpl), "%s/%s", directory.c_str(), texture);
	else
		snprintf(fnametmpl, sizeof(fnametmpl), "%s/pics/%s_??.png", directory.c_str(), name_.c_str());

	picnametempl_ = fnametmpl;
}

TerrainDescription::~TerrainDescription() {
	delete[] valid_resources_;
	nr_valid_resources_ = 0;
	valid_resources_ = nullptr;
}

void TerrainDescription::load_graphics() {
	if (!picnametempl_.empty())
		texture_ = g_gr->get_maptexture(picnametempl_, frametime_);
}

uint32_t TerrainDescription::get_texture() const {
	return texture_;
}

uint8_t TerrainDescription::get_is() const {
	return is_;
}

const std::string& TerrainDescription::name() const {
	return name_;
}

const std::string& TerrainDescription::descname() const {
	return descname_;
}

int32_t TerrainDescription::resource_value(const Resource_Index resource) const {
	return resource == get_default_resources() or is_resource_valid(resource) ?
	          (get_is() & TERRAIN_UNPASSABLE ? 8 : 1) :
	          -1;
}

uint8_t TerrainDescription::get_num_valid_resources() const {
	return nr_valid_resources_;
}

Resource_Index TerrainDescription::get_valid_resource(uint8_t index) const {
	return valid_resources_[index];
}

bool TerrainDescription::is_resource_valid(const int32_t res) const {
	for (int32_t i = 0; i < nr_valid_resources_; ++i)
		if (valid_resources_[i] == res)
			return true;
	return false;
}

int8_t TerrainDescription::get_default_resources() const {
	return default_resources_;
}

int32_t TerrainDescription::get_default_resources_amount() const {
	return default_amount_;
}

int32_t TerrainDescription::dither_layer() const {
	return dither_layer_;
}

}  // namespace Widelands
