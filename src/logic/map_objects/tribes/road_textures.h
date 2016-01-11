/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_ROAD_TEXTURES_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_ROAD_TEXTURES_H

#include <memory>
#include <vector>

#include "graphic/texture.h"

// Simple container to give access of the road textures of a tribe.
class RoadTextures {
public:
	// Returns the road texture that should be used for the Cooordinate x, y and
	// the road going into direction 'direction' (which can be any number).
	const Texture& get_normal_texture(int x, int y, int direction) const;
	const Texture& get_busy_texture(int x, int y, int direction) const;

	// Adds a new road texture.
	void add_normal_road_texture(std::unique_ptr<Texture> texture);
	void add_busy_road_texture(std::unique_ptr<Texture> texture);

private:
	std::vector<std::unique_ptr<Texture>> normal_textures_;
	std::vector<std::unique_ptr<Texture>> busy_textures_;
};

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_ROAD_TEXTURES_H
