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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_GRAPHIC_BUILD_TEXTURE_ATLAS_H
#define WL_GRAPHIC_BUILD_TEXTURE_ATLAS_H

#include <map>
#include <memory>

#include "graphic/texture.h"

// Builds a texture atlas where no texture is bigger than 'max_size' x
// 'max_size' using the most commonly used images like UI elements, roads and
// textures. Returns the texture_atlases which must be kept around in memory
// and fills in 'textures_in_atlas' which is a map from filename to Texture in
// the atlas.
std::vector<std::unique_ptr<Texture>>
build_texture_atlas(const int max_size,
                    std::map<std::string, std::unique_ptr<Texture>>* textures_in_atlas);

#endif  // end of include guard: WL_GRAPHIC_BUILD_TEXTURE_ATLAS_H
