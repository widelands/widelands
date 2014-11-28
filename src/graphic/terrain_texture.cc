/*
 * Copyright (C) 2002-2004, 2006, 2010, 2012 by the Widelands Development Team
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

#include "graphic/terrain_texture.h"

#include <SDL_image.h>

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"

using namespace std;

/**
 * Create a texture, taking the pixel data from an Image.
 * Currently it converts a 16 bit image to a 8 bit texture. This should
 * be changed to load a 8 bit file directly, however.
 */
TerrainTexture::TerrainTexture(const std::vector<std::string>& texture_files, const uint32_t frametime)
   : m_frame_num(0), m_frametime(frametime) {
	if (texture_files.empty()) {
		throw wexception("No images for texture.");
	}

	for (const std::string& fname : texture_files) {
		if (!g_fs->file_exists(fname)) {
			throw wexception("Could not find %s.", fname.c_str());
		}

		m_texture_image = fname;
		SDL_Surface* sdl_surface = load_image_as_sdl_surface(fname, g_fs);
		if (!sdl_surface) {
			throw wexception(
			   "WARNING: Failed to load texture frame %s: %s\n", fname.c_str(), IMG_GetError());
		}
		if (sdl_surface->w != kTextureWidth || sdl_surface->h != kTextureHeight) {
			SDL_FreeSurface(sdl_surface);
			throw wexception("WARNING: %s: texture must be %ix%i pixels big\n",
			                 fname.c_str(),
			                 kTextureWidth,
			                 kTextureHeight);
		}

		// calculate shades on the first frame
		if (m_textures.empty()) {
			uint8_t top_left_pixel = static_cast<uint8_t*>(sdl_surface->pixels)[0];
			SDL_Color top_left_pixel_color = sdl_surface->format->palette->colors[top_left_pixel];
			for (int i = -128; i < 128; i++) {
				const int shade = 128 + i;
				int32_t r = std::min<int32_t>((top_left_pixel_color.r * shade) >> 7, 255);
				int32_t g = std::min<int32_t>((top_left_pixel_color.g * shade) >> 7, 255);
				int32_t b = std::min<int32_t>((top_left_pixel_color.b * shade) >> 7, 255);
				m_minimap_colors[shade] = RGBColor(r, g, b);
			}
		}
		m_textures.emplace_back(new Texture(sdl_surface));
	}

	if (m_textures.empty())
		throw wexception("TerrainTexture has no frames");
}

RGBColor TerrainTexture::get_minimap_color(int8_t shade) {
	return m_minimap_colors[128 + shade];
}

void TerrainTexture::animate(uint32_t time)
{
	m_frame_num = (time / m_frametime) % m_textures.size();
}

const std::string& TerrainTexture::get_texture_image() const {
	return m_texture_image;
}

const Texture& TerrainTexture::texture() const {
	return *m_textures.at(m_frame_num);
}
