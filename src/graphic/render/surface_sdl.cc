/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "surface_sdl.h"
#include "logic/editor_game_base.h"
#include "log.h"

#include <cassert>

SurfaceSDL::~SurfaceSDL() {
	//log("SurfaceSDL::~SurfaceSDL()\n");
	if (m_surface and m_surf_type != SURFACE_SCREEN)
		SDL_FreeSurface(m_surface);
}

void SurfaceSDL::set_sdl_surface(SDL_Surface & surface)
{
	//log("SurfaceSDL::set_sdl_surface(SDL_Surface&)\n");
	if (m_surface)
		SDL_FreeSurface(m_surface);

	m_surface = &surface;
	m_w = m_surface->w;
	m_h = m_surface->h;
}

const SDL_PixelFormat & SurfaceSDL::format() const {
	//log("SurfaceSDL::format()\n");
	assert(m_surface);
	return *m_surface->format;
}

uint8_t * SurfaceSDL::get_pixels() const {
	//log("SurfaceSDL::get_pixels()\n");
	assert(m_surface);
	
	return
		static_cast<uint8_t *>(m_surface->pixels)
		+
		m_offsy * m_surface->pitch
		+
		m_offsx * m_surface->format->BytesPerPixel;
}

void SurfaceSDL::lock() {
	if (SDL_MUSTLOCK(m_surface))
		SDL_LockSurface(m_surface);
}

void SurfaceSDL::unlock() {
	if (SDL_MUSTLOCK(m_surface))
		SDL_UnlockSurface(m_surface);
}

uint32_t SurfaceSDL::get_pixel(uint32_t x, uint32_t y) {
	x += m_offsx;
	y += m_offsy;

	assert(x < get_w());
	assert(y < get_h());
	assert(m_surface);

	// Locking not needed: reading only
	const Uint8 bytes_per_pixel = m_surface->format->BytesPerPixel;
	Uint8 * const pix =
		static_cast<Uint8 *>(m_surface->pixels) +
		y * m_surface->pitch + x * bytes_per_pixel;

	switch (bytes_per_pixel) {
	case 1:
		return *pix; //  Maybe needed for save_png.
	case 2:
		return *reinterpret_cast<const Uint16 *>(pix);
	case 3: //Needed for save_png.
		//  We can not dereference a pointer to a size 4 object in this case
		//  since that would casue a read beyond the end of the block pointed to
		//  by m_surface. Furthermore it would not be properly aligned to a 4
		//  byte boundary.
		//
		//  Suppose that the image is 2 * 2 pixels. Then m_surface points to a
		//  block of size 2 * 2 * 3 = 12. The values for the last pixel are at
		//  m_surface[9], m_surface[10] and m_surface[11]. But m_surface[12] is
		//  beyond the end of the block, so we can not read 4 bytes starting at
		//  m_surface[9] (even if unaligned access is allowed).
		//
		//  Therefore we read the 3 bytes separately and get the result by
		//  shifting the values. It is alignment safe.
		return pix[0] << 0x00 | pix[1] << 0x08 | pix[2] << 0x10;
	case 4:
		return *reinterpret_cast<const Uint32 *>(pix);
	}
	assert(false);

	return 0; // Should never be here
}

void SurfaceSDL::set_pixel(uint32_t x, uint32_t y, const Uint32 clr) {
	x += m_offsx;
	y += m_offsy;

	assert(x < get_w());
	assert(y < get_h());
	assert(m_surface);

	if (SDL_MUSTLOCK(m_surface))
		SDL_LockSurface(m_surface);

	const Uint8 bytes_per_pixel = m_surface->format->BytesPerPixel;
	Uint8 * const pix =
		static_cast<Uint8 *>(m_surface->pixels) +
		y * m_surface->pitch + x * bytes_per_pixel;
	switch (bytes_per_pixel) {
	case 2: *reinterpret_cast<Uint16 *>(pix) = static_cast<Uint16>(clr); break;
	case 4: *reinterpret_cast<Uint32 *>(pix) = clr;                      break;
	};

	if (SDL_MUSTLOCK(m_surface))
		SDL_UnlockSurface(m_surface);
}

void SurfaceSDL::set_subwin(Rect r) {
	m_offsx = r.x;
	m_offsy = r.y;
	m_w = r.w;
	m_h = r.h;
}

void SurfaceSDL::unset_subwin() {
	m_offsx = 0;
	m_offsy = 0;
	m_w = m_surface->w;
	m_h = m_surface->h;
}
