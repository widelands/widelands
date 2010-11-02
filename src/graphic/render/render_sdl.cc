/*
 * Copyright (C) 2002-2004, 2007-2010 by the Widelands Development Team
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

/*
Rendering functions of the software renderer.
*/


#include "log.h"
#include "upcast.h"
#include "wexception.h"
#include "logic/building.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/world.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "wui/minimap.h"

#include "surface_sdl.h"
#include "graphic/graphic.h"

#include <SDL.h>

using Widelands::Flag;
using Widelands::PlayerImmovable;
using Widelands::Road;

/*
 * Updating the whole Surface
 */
void SurfaceSDL::update() {
	if (m_surf_type == SURFACE_SCREEN) {
		//flip defaults to SDL_UpdateRect(m_surface, 0, 0, 0, 0);
		SDL_Flip(m_surface);
		//log("SurfaceSDL::update(): update complete screen\n");
	} else {
		SDL_UpdateRect(m_surface, m_offsx, m_offsy, m_w, m_h);
		//log("SurfaceSDL::update()\n");
	}
}

/*
===============
Draws the outline of a rectangle
===============
*/
void SurfaceSDL::draw_rect(const Rect rc, const RGBColor clr) {
	assert(m_surface);
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);
	//log("SurfaceSDL::draw_rect()\n");
	const uint32_t color = clr.map(format());

	const Point bl = rc.bottom_left() - Point(1, 1);

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
void SurfaceSDL::fill_rect(const Rect rc, const RGBAColor clr) {
	assert(m_surface);
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);
	//log("SurfaceSDL::fill_rect()\n");
	const uint32_t color = clr.map(format());

	SDL_Rect r = {rc.x, rc.y, rc.w, rc.h};
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
void SurfaceSDL::brighten_rect(const Rect rc, const int32_t factor) {
	if (!factor)
		return;
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);

	//log("SurfaceSDL::brighten_rect()\n");

	const Point bl = rc.bottom_left();

	lock();

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
	unlock();
}

#define draw_pixel(p, r, clr)                                                 \
   if                                                                         \
      ((p).x >= (r).x and (p).x < static_cast<int32_t>((r).x + (r).w) and     \
       (p).y >= (r).y and (p).y < static_cast<int32_t>((r).y + (r).h))        \
      set_pixel((p).x, (p).y, (clr).map(format()))                            \

/**
* This functions draws a (not horizontal or vertical)
* line in the target, using Bresenham's algorithm
*
* This function could be faster by using direct pixel
* access instead of the set_pixel() function
*/
void SurfaceSDL::draw_line
		(int32_t x1,
		 int32_t y1,
		 int32_t x2,
		 int32_t y2,
		 RGBColor color,
		 const Rect * clip)
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

	draw_pixel(p, *clip, color);

	if (dxabs >= dyabs) //  the line is more horizontal than vertical
		for (uint32_t i = 0; i < dxabs; ++i) {
			y += dyabs;

			if (y >= dxabs) {
				y   -= dxabs;
				p.y += sdy;
			}

			p.x += sdx;
			draw_pixel(p, *clip, color);
		}
	else                //  the line is more vertical than horizontal
		for (uint32_t i = 0; i < dyabs; ++i) {
			x += dxabs;

			if (x >= dyabs) {
				x   -= dyabs;
				p.x += sdx;
			}

			p.y += sdy;
			draw_pixel(p, *clip, color);
		}
}


/*
===============
Clear the entire bitmap to black
===============
*/
void SurfaceSDL::clear() {
	SDL_FillRect(m_surface, 0, 0);
}


void SurfaceSDL::blit
	(Point const dst, Surface * const src, Rect const srcrc, bool enable_alpha)
{
	assert(src);
	assert(this);
	SDL_Rect srcrect = {srcrc.x, srcrc.y, srcrc.w, srcrc.h};
	SDL_Rect dstrect = {dst.x, dst.y, 0, 0};

	SDL_BlitSurface
		(dynamic_cast<SurfaceSDL *>(src)->get_sdl_surface(),
		 &srcrect, m_surface, &dstrect);
}


/*
 * Fast blit, simply copy the source to the destination
 */
void SurfaceSDL::fast_blit(Surface * const src) {
	SDL_BlitSurface
		(dynamic_cast<SurfaceSDL *>(src)->get_sdl_surface(),
		 0, m_surface, 0);
}
