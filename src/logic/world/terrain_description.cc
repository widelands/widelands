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

#include <stdint.h>
#include <boost/foreach.hpp>

#include "logic/world/terrain_description.h"

#include "constants.h"
#include "graphic/graphic.h"
#include "logic/game_data_error.h"
#include "logic/world/data.h"
#include "profile/profile.h"

namespace Widelands {

TerrainDescription::TerrainDescription(const std::string& name,
                                       const std::string& descname,
                                       TerrainType type,
													const std::vector<std::string>& texture_files,
                                       int fps,
                                       int32_t dither_layer,
                                       std::vector<uint8_t> valid_resources,
                                       uint8_t default_resource,
                                       int32_t default_resource_amount)
   : name_(name),
     descname_(descname),
     is_(type),
     valid_resources_(valid_resources),
     default_resource_index_(default_resource),
     default_resource_amount_(default_resource_amount),
     dither_layer_(dither_layer),
     texture_(g_gr->new_maptexture(texture_files, fps > 0 ? 1000 / fps : FRAME_LENGTH)) {

	if (default_resource_amount > 0 && !is_resource_valid(default_resource)) {
		throw game_data_error("Default resource is not in valid resources.\n");
	}
}

TerrainDescription::~TerrainDescription() {
}

void TerrainDescription::load_graphics() {
	// NOCOM(#sirver): kill method
}

uint32_t TerrainDescription::get_texture() const {
	return texture_;
}

TerrainType TerrainDescription::get_is() const {
	return is_;
}

const std::string& TerrainDescription::name() const {
	return name_;
}

const std::string& TerrainDescription::descname() const {
	return descname_;
}

int32_t TerrainDescription::resource_value(const Resource_Index resource) const {
	return is_resource_valid(resource) ? (get_is() & TERRAIN_UNPASSABLE ? 8 : 1) : -1;
}

Resource_Index TerrainDescription::get_valid_resource(uint8_t index) const {
	return valid_resources_[index];
}

bool TerrainDescription::is_resource_valid(const int32_t res) const {
	for (const uint8_t resource_index : valid_resources_) {
		if (resource_index == res) {
			return true;
		}
	}
	return false;
}

int8_t TerrainDescription::get_default_resource() const {
	return default_resource_index_;
}

int32_t TerrainDescription::get_default_resource_amount() const {
	return default_resource_amount_;
}

int32_t TerrainDescription::dither_layer() const {
	return dither_layer_;
}

}  // namespace Widelands
