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
#include <boost/foreach.hpp>

#include "constants.h"
#include "container_iterate.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "wexception.h"

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
     m_frametime(frametime),
     m_was_animated(false) {
	if (texture_files.empty()) {
		throw wexception("No images for texture.");
	}

	for (const std::string& fname : texture_files) {
		if (!g_fs->FileExists(fname)) {
			throw wexception("Could not find %s.", fname.c_str());
		}

		SDL_Surface * surf;
		m_texture_image = fname;
		FileRead fr;
		fr.fastOpen(*g_fs, fname.c_str());

		surf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);
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

		if (g_opengl) {
			// Note: we except the constructor to free the SDL surface
			GLSurfaceTexture* surface = new GLSurfaceTexture(surf);
			m_glFrames.emplace_back(surface);

			// calculate shades on the first frame
			if (!m_nrframes) {
				surface->lock(Surface::Lock_Normal);
				uint32_t mmap_color_base = surface->get_pixel(0, 0);
				surface->unlock(Surface::Unlock_NoChange);

				int32_t i, shade, r, g, b, a;
				for (i = -128; i < 128; i++) {
					shade = 128 + i;

					a = (mmap_color_base & 0xff000000) >> 24;
					b = (mmap_color_base & 0x00ff0000) >> 16;
					g = (mmap_color_base & 0x0000ff00) >> 8;
					r = (mmap_color_base & 0x000000ff);

					b = (b * shade) >> 7;
					g = (g * shade) >> 7;
					r = (r * shade) >> 7;

					if (b > 255) b = 255;
					if (g > 255) g = 255;
					if (r > 255) r = 255;

					m_mmap_color[shade] = (a << 24) | (b << 16) | (g << 8) | r;
				}
			}

			++m_nrframes;
			continue;
		}

		// Determine color map if it's the first frame
		if (!m_nrframes) {
			if (surf->format->BitsPerPixel == 8) {
				m_colormap.reset(new Colormap(*surf->format->palette->colors, format));
			} else {
				SDL_Color pal[256];

				log("WARNING: %s: using 332 default palette\n", fname.c_str());

				for (int32_t r = 0; r < 8; ++r)
					for (int32_t g = 0; g < 8; ++g)
						for (int32_t b = 0; b < 4; ++b) {
							pal[(r << 5) | (g << 2) | b].r = r << 5;
							pal[(r << 5) | (g << 2) | b].g = g << 5;
							pal[(r << 5) | (g << 2) | b].b = b << 6;
						}

				m_colormap.reset(new Colormap(*pal, format));
			}
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
Uint32 Texture::get_minimap_color(char shade) {
	if (not m_pixels)
		return m_mmap_color[128 + shade];

	uint8_t clr = m_pixels[0]; // just use the top-left pixel

	uint32_t table = static_cast<uint8_t>(shade);
	return static_cast<const Uint32*>(m_colormap->get_colormap())[clr | (table << 8)];
}

/**
 * Set the current frame according to the game time.
 */
void Texture::animate(uint32_t time)
{
	m_frame_num = (time / m_frametime) % m_nrframes;

	if (g_opengl)
		return;

	uint8_t * const lastframe = m_curframe;

	m_curframe = &m_pixels[TEXTURE_WIDTH * TEXTURE_HEIGHT * m_frame_num];
	if (lastframe != m_curframe)
		m_was_animated = true;
}
