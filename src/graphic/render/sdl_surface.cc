/*
 * Copyright (C) 2002-2004, 2007-2013 by the Widelands Development Team
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

#include "graphic/render/sdl_surface.h"

#include <cassert>

#include <SDL.h>

SDLSurface::~SDLSurface() {
	assert(m_surface);

	if (m_free_surface_on_delete)
		SDL_FreeSurface(m_surface);
}

const SDL_PixelFormat & SDLSurface::format() const {
	assert(m_surface);
	return *m_surface->format;
}

uint8_t * SDLSurface::get_pixels() const {
	assert(m_surface);

	return
		static_cast<uint8_t *>(m_surface->pixels)
		+
		m_offsy * m_surface->pitch
		+
		m_offsx * m_surface->format->BytesPerPixel;
}

void SDLSurface::lock(LockMode) {
	if (SDL_MUSTLOCK(m_surface))
		SDL_LockSurface(m_surface);
}

void SDLSurface::unlock(UnlockMode) {
	if (SDL_MUSTLOCK(m_surface))
		SDL_UnlockSurface(m_surface);
}

uint32_t SDLSurface::get_pixel(uint16_t x, uint16_t y) {
	x += m_offsx;
	y += m_offsy;

	assert(x < width());
	assert(y < height());
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
	default:
		assert(false);
	}

	return 0; // Should never be here
}

void SDLSurface::set_pixel(uint16_t x, uint16_t y, const Uint32 clr) {
	x += m_offsx;
	y += m_offsy;

	if (x >= width() || y >= height())
		return;
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
	default: break;
	};

	if (SDL_MUSTLOCK(m_surface))
		SDL_UnlockSurface(m_surface);
}

void SDLSurface::set_subwin(const Rect& r) {
	m_offsx = r.x;
	m_offsy = r.y;
	m_w = r.w;
	m_h = r.h;
}

void SDLSurface::unset_subwin() {
	m_offsx = 0;
	m_offsy = 0;
	m_w = m_surface->w;
	m_h = m_surface->h;
}

/*
===============
Draws the outline of a rectangle
===============
*/
void SDLSurface::draw_rect(const Rect& rc, const RGBColor clr) {
	assert(m_surface);
	assert(rc.x >= 0);
	assert(rc.y >= 0);

	const uint32_t color = clr.map(format());

	const Point bl = rc.bottom_right() - Point(1, 1);

	for (int32_t x = rc.x + 1; x < bl.x; ++x) {
		set_pixel(x, rc.y, color);
		set_pixel(x, bl.y, color);
	}
	for (int32_t y = rc.y; y <= bl.y; ++y) {
		set_pixel(rc.x, y, color);
		set_pixel(bl.x, y, color);
	}
}


/*
===============
Draws a filled rectangle
===============
*/
void SDLSurface::fill_rect(const Rect& rc, const RGBAColor clr) {
	assert(m_surface);
	assert(rc.x >= 0);
	assert(rc.y >= 0);

	const uint32_t color = clr.map(format());

	SDL_Rect r = {
		static_cast<Sint16>(rc.x), static_cast<Sint16>(rc.y),
		static_cast<Uint16>(rc.w), static_cast<Uint16>(rc.h)
		};
	SDL_FillRect(m_surface, &r, color);
}


/*
===============
Change the brightness of the given rectangle
This function is slow as hell.

* This function is a possible point to optimize on
  slow system. It takes a lot of cpu time atm and is
  not needed. It is used by the ui_basic stuff to
  highlight things.
===============
*/
void SDLSurface::brighten_rect(const Rect& rc, const int32_t factor) {
	if (!factor)
		return;
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);

	const Point bl = rc.bottom_right();

	lock(Surface::Lock_Normal);

	if (m_surface->format->BytesPerPixel == 4)
	{
		for (int32_t y = rc.y; y < bl.y; ++y)
			for (int32_t x = rc.x; x < bl.x; ++x)
		{

			Uint8 * const pix =
				static_cast<Uint8 *>(m_surface->pixels) +
				(y + m_offsy) * m_surface->pitch + (x + m_offsx) * 4;

			uint32_t const clr = *reinterpret_cast<const Uint32 *>(pix);
			uint8_t gr, gg, gb;
			SDL_GetRGB(clr, m_surface->format, &gr, &gg, &gb);
			int16_t r = gr + factor;
			int16_t g = gg + factor;
			int16_t b = gb + factor;

			if (b & 0xFF00)
				b = ~b >> 24;
			if (g & 0xFF00)
				g = ~g >> 24;
			if (r & 0xFF00)
				r = ~r >> 24;

			*reinterpret_cast<Uint32 *>(pix) =
				SDL_MapRGB(m_surface->format, r, g, b);
		}
	} else if (m_surface->format->BytesPerPixel == 2) {
		for (int32_t y = rc.y; y < bl.y; ++y)
			for (int32_t x = rc.x; x < bl.x; ++x)
		{
			Uint8 * const pix =
				static_cast<Uint8 *>(m_surface->pixels) +
				(y + m_offsy) * m_surface->pitch + (x + m_offsx) * 2;

			uint32_t const clr = *reinterpret_cast<const Uint16 *>(pix);
			uint8_t gr, gg, gb;
			SDL_GetRGB(clr, m_surface->format, &gr, &gg, &gb);
			int16_t r = gr + factor;
			int16_t g = gg + factor;
			int16_t b = gb + factor;

			if (b & 0xFF00)
				b = ~b >> 24;
			if (g & 0xFF00)
				g = ~g >> 24;
			if (r & 0xFF00)
				r = ~r >> 24;

			*reinterpret_cast<Uint16 *>(pix) =
				SDL_MapRGB(m_surface->format, r, g, b);
		}
	}
	unlock(Surface::Unlock_Update);
}

/**
* This functions draws a (not horizontal or vertical)
* line in the target, using Bresenham's algorithm
*
* This function could be faster by using direct pixel
* access instead of the set_pixel() function
*/
void SDLSurface::draw_line
	(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor& color, uint8_t gwidth)
{
	int32_t dx = x2 - x1;      /* the horizontal distance of the line */
	int32_t dy = y2 - y1;      /* the vertical distance of the line */
	const uint32_t dxabs = abs(dx);
	const uint32_t dyabs = abs(dy);
	int32_t sdx = dx < 0 ? -1 : 1;
	int32_t sdy = dy < 0 ? -1 : 1;
	uint32_t x = dyabs / 2;
	uint32_t y = dxabs / 2;
	Point p(x1, y1);

	set_pixel(p.x, p.y, color.map(format()));

	if (dxabs >= dyabs) //  the line is more horizontal than vertical
		for (uint32_t i = 0; i < dxabs; ++i) {
			y += dyabs;

			if (y >= dxabs) {
				y   -= dxabs;
				p.y += sdy;
			}

			p.x += sdx;
			for (int32_t w = 0; w < gwidth; ++w) {
				set_pixel(p.x, p.y + w, color.map(format()));
			}
		}
	else                //  the line is more vertical than horizontal
		for (uint32_t i = 0; i < dyabs; ++i) {
			x += dxabs;

			if (x >= dyabs) {
				x   -= dyabs;
				p.x += sdx;
			}

			p.y += sdy;
			for (int32_t w = 0; w < gwidth; ++w) {
				set_pixel(p.x + w, p.y, color.map(format()));
			}
		}
}


void SDLSurface::blit
	(const Point& dst, const Surface* src, const Rect& srcrc, Composite cm)
{
	SDL_Surface* sdlsurf = static_cast<const SDLSurface*>(src)->get_sdl_surface();
	SDL_Rect srcrect = {
		static_cast<Sint16>(srcrc.x), static_cast<Sint16>(srcrc.y),
		static_cast<Uint16>(srcrc.w), static_cast<Uint16>(srcrc.h)
		};
	SDL_Rect dstrect = {
		static_cast<Sint16>(dst.x), static_cast<Sint16>(dst.y),
		0, 0
		};

	bool alpha;
	uint8_t alphaval;
	if (cm == CM_Solid || cm == CM_Copy) {
		alpha = sdlsurf->flags & SDL_SRCALPHA;
		alphaval = sdlsurf->format->alpha;
		SDL_SetAlpha(sdlsurf, 0, 0);
	}

	SDL_BlitSurface(sdlsurf, &srcrect, m_surface, &dstrect);

	if (cm == CM_Solid || cm == CM_Copy) {
		SDL_SetAlpha(sdlsurf, alpha?SDL_SRCALPHA:0, alphaval);
	}
}
