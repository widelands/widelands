/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_ROAD_TEXTURES_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_ROAD_TEXTURES_H

#include <vector>

#include "graphic/image.h"
#include "logic/widelands_geometry.h"

// Simple container to give access of the road textures of a tribe.
class RoadTextures {
public:
	// Returns the road texture that should be used for 'coords' and the road
	// going into direction 'direction' (which can be any number).
	const Image& get_normal_texture(const Widelands::Coords& coords, int direction) const;
	const Image& get_busy_texture(const Widelands::Coords& coords, int direction) const;
	const Image& get_waterway_texture(const Widelands::Coords& coords, int direction) const;

	// Adds a new road texture.
	void add_normal_road_texture(const Image* image);
	void add_busy_road_texture(const Image* image);
	void add_waterway_texture(const Image* image);

private:
	std::vector<const Image*> normal_textures_;
	std::vector<const Image*> busy_textures_;
	std::vector<const Image*> waterway_textures_;
};

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_ROAD_TEXTURES_H
