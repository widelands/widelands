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

#include "graphic/texture.h"

#include <SDL_image.h>

#include "base/deprecated.h"
#include "base/log.h"
#include "base/wexception.h"
#include "graphic/image_io.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"

extern bool g_opengl;

using namespace std;

/**
 * Create a texture, taking the pixel data from an Image.
 * Currently it converts a 16 bit image to a 8 bit texture. This should
 * be changed to load a 8 bit file directly, however.
 */
Texture::Texture(const std::vector<std::string>& texture_files,
                 const uint32_t frametime,
                 const SDL_PixelFormat& format)
   : m_colormap(nullptr),
     m_pixels(nullptr),
     m_curframe(nullptr),
     m_frame_num(0),
     m_nrframes(0),
     m_frametime(frametime) {
	if (texture_files.empty()) {
		throw wexception("No images for texture.");
	}

	for (const std::string& fname : texture_files) {
		if (!g_fs->file_exists(fname)) {
			throw wexception("Could not find %s.", fname.c_str());
		}

		m_texture_image = fname;
		SDL_Surface* surf = load_image_as_sdl_surface(fname, g_fs);
		if (!surf) {
			throw wexception("WARNING: Failed to load texture frame %s: %s\n", fname.c_str(), IMG_GetError());
		}
		if (surf->w != TEXTURE_WIDTH || surf->h != TEXTURE_HEIGHT) {
			SDL_FreeSurface(surf);
			throw wexception("WARNING: %s: texture must be %ix%i pixels big\n",
			                 fname.c_str(),
			                 TEXTURE_WIDTH,
			                 TEXTURE_HEIGHT);
		}

		// calculate shades on the first frame
		if (!m_nrframes) {
			uint8_t top_left_pixel = static_cast<uint8_t*>(surf->pixels)[0];
			SDL_Color top_left_pixel_color = surf->format->palette->colors[top_left_pixel];
			for (int i = -128; i < 128; i++) {
				const int shade = 128 + i;
				int32_t r = std::min<int32_t>((top_left_pixel_color.r * shade) >> 7, 255);
				int32_t g = std::min<int32_t>((top_left_pixel_color.g * shade) >> 7, 255);
				int32_t b = std::min<int32_t>((top_left_pixel_color.b * shade) >> 7, 255);
				m_minimap_colors[shade] = RGBColor(r, g, b);
			}
		}

		if (g_opengl) {
			// Note: we except the constructor to free the SDL surface
			GLSurfaceTexture* surface = new GLSurfaceTexture(surf);
			m_glFrames.emplace_back(surface);

			++m_nrframes;
			continue;
		}

		// Determine color map if it's the first frame
		if (!m_nrframes) {
			if (surf->format->BitsPerPixel != 8) {
				throw wexception("Terrain %s is not 8 bits per pixel.", fname.c_str());
			}
			m_colormap.reset(new Colormap(*surf->format->palette->colors, format));
		}

		// Convert to our palette
		SDL_Palette palette;
		SDL_PixelFormat fmt;

		palette.ncolors = 256;
		palette.colors = m_colormap->get_palette();

		memset(&fmt, 0, sizeof(fmt));
		fmt.BitsPerPixel = 8;
		fmt.BytesPerPixel = 1;
		fmt.palette = &palette;

		SDL_Surface * const cv = SDL_ConvertSurface(surf, &fmt, 0);

		// Add the frame
		uint8_t* new_ptr =
			static_cast<uint8_t *>
				(realloc
				 	(m_pixels, TEXTURE_WIDTH * TEXTURE_HEIGHT * (m_nrframes + 1)));
		if (!new_ptr)
			throw wexception("Out of memory.");
		m_pixels = new_ptr;


		m_curframe = &m_pixels[TEXTURE_WIDTH * TEXTURE_HEIGHT * m_nrframes];
		++m_nrframes;

		SDL_LockSurface(cv);

		for (int32_t y = 0; y < TEXTURE_HEIGHT; ++y)
			memcpy
				(m_curframe + y * TEXTURE_WIDTH,
				 static_cast<uint8_t *>(cv->pixels) + y * cv->pitch,
				 TEXTURE_WIDTH);
		SDL_UnlockSurface(cv);
		SDL_FreeSurface(cv);
		SDL_FreeSurface(surf);
	}

	if (!m_nrframes)
		throw wexception("Texture has no frames");
}


Texture::~Texture ()
{
	free(m_pixels);
}

/**
 * Return the basic terrain colour to be used in the minimap.
*/
RGBColor Texture::get_minimap_color(int8_t shade) {
	return m_minimap_colors[128 + shade];
}

/**
 * Set the current frame according to the game time.
 */
void Texture::animate(uint32_t time)
{
	m_frame_num = (time / m_frametime) % m_nrframes;
	if (g_opengl)
		return;
	m_curframe = &m_pixels[TEXTURE_WIDTH * TEXTURE_HEIGHT * m_frame_num];
}
