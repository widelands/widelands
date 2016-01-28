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

#include "logic/map_objects/tribes/road_textures.h"

#include <memory>

const Image& RoadTextures::get_normal_texture(int x, int y, int direction) const {
	return *normal_textures_.at((x + y + direction) % normal_textures_.size());
}

const Image& RoadTextures::get_busy_texture(int x, int y, int direction) const {
	return *busy_textures_.at((x + y + direction) % busy_textures_.size());
}

void RoadTextures::add_normal_road_texture(const Image* image) {
	normal_textures_.emplace_back(image);
}

void RoadTextures::add_busy_road_texture(const Image* image) {
	busy_textures_.emplace_back(image);
}
