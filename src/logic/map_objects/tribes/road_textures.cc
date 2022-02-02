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

#include "logic/map_objects/tribes/road_textures.h"

const Image& RoadTextures::get_normal_texture(const Widelands::Coords& coords,
                                              int direction) const {
	return *normal_textures_.at((coords.x + coords.y + direction) % normal_textures_.size());
}

const Image& RoadTextures::get_busy_texture(const Widelands::Coords& coords, int direction) const {
	return *busy_textures_.at((coords.x + coords.y + direction) % busy_textures_.size());
}

const Image& RoadTextures::get_waterway_texture(const Widelands::Coords& coords,
                                                int direction) const {
	return *waterway_textures_.at((coords.x + coords.y + direction) % waterway_textures_.size());
}

void RoadTextures::add_normal_road_texture(const Image* image) {
	normal_textures_.emplace_back(image);
}

void RoadTextures::add_busy_road_texture(const Image* image) {
	busy_textures_.emplace_back(image);
}

void RoadTextures::add_waterway_texture(const Image* image) {
	waterway_textures_.emplace_back(image);
}
