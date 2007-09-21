/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "surface.h"

#include "editor_game_base.h"
#include "terrain.h"

#include <assert.h>


Surface::~Surface() {
	if (m_surface)
		SDL_FreeSurface(m_surface);
	m_surface = 0;
}

void Surface::set_sdl_surface(SDL_Surface & surface)
{
	m_surface = &surface;
	m_w = m_surface->w;
	m_h = m_surface->h;
}

const SDL_PixelFormat * Surface::get_format() const {
	assert(m_surface);
	return m_surface->format;
}

const SDL_PixelFormat & Surface::format() const {
	assert(m_surface);
	return *m_surface->format;
}

void * Surface::get_pixels() const throw () {
	assert(m_surface);
	return
		static_cast<uint8_t *>(m_surface->pixels)
		+
		m_offsy * m_surface->pitch
		+
		m_offsx * m_surface->format->BytesPerPixel;
}

void Surface::lock() {
	if (SDL_MUSTLOCK(m_surface))
		SDL_LockSurface(m_surface);
}

void Surface::unlock() {
	if (SDL_MUSTLOCK(m_surface))
		SDL_UnlockSurface(m_surface);
}

uint32_t Surface::get_pixel(uint32_t x, uint32_t y) {
	x+= m_offsx;
	y+= m_offsy;

	assert(x < get_w() && y < get_h());
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
			//fallthrough
	case 4:
			return *reinterpret_cast<const Uint32 *>(pix);
	}
	assert(false);

	return 0; // Should never be here
}

void Surface::set_pixel(uint32_t x, uint32_t y, const Uint32 clr) {
	x+= m_offsx;
	y+= m_offsy;

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
	}

	if (SDL_MUSTLOCK(m_surface))
		SDL_UnlockSurface(m_surface);
}

void Surface::set_subwin(Rect r) {
	m_offsx = r.x;
	m_offsy = r.y;
	m_w =r.w;
	m_h = r.h;
}

void Surface::unset_subwin() {
	m_offsx = 0;
	m_offsy = 0;
	m_w = m_surface->w;
	m_h = m_surface->h;
}

/**
 * Draw ground textures and roads for the given parallelogram (two triangles)
 * into the bitmap.
*/
void Surface::draw_field
(Rect & subwin,
 Field * const f, Field * const rf, Field * const fl, Field * const rfl,
	                const int posx, const int rposx, const int posy,
	                const int blposx, const int rblposx, const int blposy,
 uint8_t roads,
 Sint8            f_brightness,
 Sint8            r_brightness,
 Sint8           bl_brightness,
 Sint8           br_brightness,
 const Texture & tr_d_texture,
 const Texture &  l_r_texture,
 const Texture &  f_d_texture,
 const Texture &  f_r_texture,
bool draw_all)
{
	set_subwin(subwin);

	switch (get_format()->BytesPerPixel) {
	case 2:
		draw_field_int<Uint16>
		    (*this,
		     f, rf, fl, rfl,
		     posx, rposx, posy, blposx, rblposx, blposy,
		     roads,
		     f_brightness, r_brightness, bl_brightness, br_brightness,
		     tr_d_texture, l_r_texture, f_d_texture, f_r_texture,
		     draw_all);
		break;
	case 4:
		draw_field_int<Uint32>
		    (*this,
		     f, rf, fl, rfl,
		     posx, rposx, posy, blposx, rblposx, blposy,
		     roads,
		     f_brightness, r_brightness, bl_brightness, br_brightness,
		     tr_d_texture, l_r_texture, f_d_texture, f_r_texture,
		     draw_all);
		break;
	default: assert(false);
	}

	unset_subwin();
}
