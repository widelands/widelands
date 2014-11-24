/*
 * Copyright (C) 2002-2004, 2006, 2008-2010, 2012 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_GRAPHIC_TERRAIN_TEXTURE_H
#define WL_GRAPHIC_TERRAIN_TEXTURE_H

#include <memory>
#include <string>
#include <vector>

#include <stdint.h>

#include "graphic/colormap.h"

class Texture;

/// TerrainTextures have a fixed size and are squares.
constexpr int kTextureWidth = 64;
constexpr int kTextureHeight = kTextureWidth;

// TerrainTexture represents are terrain texture, which is strictly kTextureWidth by
// kTextureHeight pixels in size.
struct TerrainTexture {
	TerrainTexture(const std::vector<std::string>& texture_files, uint32_t frametime);

	// Returns the path to a representative image for this texture.
	const std::string& get_texture_image() const;

	// Returns the texture for the current animation phase.
	const Texture& texture() const;

	// Return the basic terrain colour to be used in the minimap.
	RGBColor get_minimap_color(int8_t shade);

	// Set the current frame according to the game time.
	void animate(uint32_t time);

private:
	RGBColor    m_minimap_colors[256];
	int32_t     m_frame_num;
	std::string m_texture_image;
	uint32_t    m_frametime;
	std::vector<std::unique_ptr<Texture>> m_textures;
};

#endif  // end of include guard: WL_GRAPHIC_TERRAIN_TEXTURE_H
