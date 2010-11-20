/*
 * Copyright (C) 2002-2004, 2006, 2010 by the Widelands Development Team
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

#include "texture.h"

#include "io/filesystem/layered_filesystem.h"
#include "io/fileread.h"
#include "graphic.h"

#include "log.h"
#include "constants.h"
#include "wexception.h"
#include "container_iterate.h"

#include <SDL_image.h>

/**
 * Create a texture, taking the pixel data from a Pic.
 * Currently it converts a 16 bit pic to a 8 bit texture. This should
 * be changed to load a 8 bit file directly, however.
 */
Texture::Texture
	(char            const &       fnametmpl,
	 uint32_t                const frametime,
	 SDL_PixelFormat const &       format)
	:
	m_colormap (0),
	m_pixels   (0),
	m_curframe (0),
	m_frame_num(0),
	m_nrframes (0),
	m_frametime(frametime),
	is_32bit   (format.BytesPerPixel == 4)
{

	// Load the pictures one by one
	char fname[256];

	for (;;) {
		int32_t nr = m_nrframes;

		// create the file name by reverse-scanning for '?' and replacing
		snprintf(fname, sizeof(fname), "%s", &fnametmpl);
		char * p = fname + strlen(fname);
		while (p > fname) {
			if (*--p != '?')
				continue;

			*p = '0' + (nr % 10);
			nr = nr / 10;
		}

		if (nr) // cycled up to maximum possible frame number
			break;

		if (!g_fs->FileExists(fname))
			break;

		SDL_Surface * surf;

		m_texture_picture = strdup(fname);

		FileRead fr;

		//fastOpen tries to use mmap
		fr.fastOpen(*g_fs, fname);

		surf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);

		if (!surf) {
			log
				("WARNING: Failed to load texture frame %s: %s\n",
				 fname, IMG_GetError());
			break;
		}

		if (surf->w != TEXTURE_WIDTH || surf->h != TEXTURE_HEIGHT) {
			SDL_FreeSurface(surf);
			log
				("WARNING: %s: texture must be %ix%i pixels big\n",
				 fname,
				 TEXTURE_WIDTH,
				 TEXTURE_HEIGHT);
			break;
		}

#ifdef USE_OPENGL
		if (g_opengl) {
			SurfaceOpenGL * tsurface =
				&dynamic_cast<SurfaceOpenGL &>
				(g_gr->load_image(fname));
			// SDL_ConvertSurface(surf, &fmt, 0);
			m_glFrames.push_back(tsurface);
			tsurface->lock();
			m_mmap_color = tsurface->get_pixel(0, 0);
			tsurface->unlock();
			++m_nrframes;
			continue;
		}
#endif


		// Determine color map if it's the first frame
		if (!m_nrframes) {
			if (surf->format->BitsPerPixel == 8)
				m_colormap = new Colormap(*surf->format->palette->colors, format);
			else {
				SDL_Color pal[256];

				log("WARNING: %s: using 332 default palette\n", fname);

				for (int32_t r = 0; r < 8; ++r)
					for (int32_t g = 0; g < 8; ++g)
						for (int32_t b = 0; b < 4; ++b) {
							pal[(r << 5) | (g << 2) | b].r = r << 5;
							pal[(r << 5) | (g << 2) | b].g = g << 5;
							pal[(r << 5) | (g << 2) | b].b = b << 6;
						}

				m_colormap = new Colormap(*pal, format);
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
		//  FIXME cppcheck: (error) Common realloc mistake: "m_pixels" nulled but
		//  FIXME cppcheck: (error) not freed upon failure
		m_pixels =
			static_cast<uint8_t *>
				(realloc
				 	(m_pixels, TEXTURE_WIDTH * TEXTURE_HEIGHT * (m_nrframes + 1)));
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
		throw wexception("%s: texture has no frames", &fnametmpl);
}


Texture::~Texture ()
{
	delete m_colormap;
	free(m_pixels);
	free(m_texture_picture);

#ifdef USE_OPENGL
	container_iterate(std::vector<SurfaceOpenGL *>, m_glFrames, it)
		delete *it.current;
#endif
}

/**
 * Return the basic terrain colour to be used in the minimap.
*/
Uint32 Texture::get_minimap_color(const char shade) {
	if (not m_pixels)
		return m_mmap_color;

	uint8_t clr = m_pixels[0]; // just use the top-left pixel
	uint32_t table = static_cast<uint8_t>(shade);
	return
		is_32bit ?
		static_cast<const Uint32 *>(m_colormap->get_colormap())
		[clr | (table << 8)]
		:
		static_cast<const Uint16 *>(m_colormap->get_colormap())
		[clr | (table << 8)];
}

/**
 * Set the current frame according to the game time.
 */
void Texture::animate(uint32_t time)
{
	m_frame_num = (time / m_frametime) % m_nrframes;

#ifdef USE_OPENGL
	if (g_opengl)
		return;
#endif

	uint8_t * const lastframe = m_curframe;

	m_curframe = &m_pixels[TEXTURE_WIDTH * TEXTURE_HEIGHT * m_frame_num];
	if (lastframe != m_curframe)
		m_was_animated = true;
}
