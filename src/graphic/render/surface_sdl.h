/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef SURFACE_SDL_H
#define SURFACE_SDL_H

#include "rgbcolor.h"
#include "rect.h"

#include "graphic/surface.h"

/**
* This implements SDL rendering. Do not use this class directly. The right
* way is to use the base class Surface wherever possible. Everything which
* needs to know about the underlying renderer should go to the graphics
* subdirectory.
* Surfaces are created through Graphic::create_surface() functions.
*/
class SurfaceSDL : public Surface {

public:
	SurfaceSDL(SDL_Surface & surface) : 
		Surface(surface.w, surface.h, SURFACE_OFFSCREEN),
		m_surface(&surface)
	{}
	SurfaceSDL():
		Surface(),
		m_surface(0)
	{}
	~SurfaceSDL();

	/// Set surface, only call once
	void set_sdl_surface(SDL_Surface & surface);
	SDL_Surface * get_sdl_surface() {return m_surface;}

	/// Get width and height
	void update();

	/// Save a bitmap of this to a file
	void save_bmp(const char & fname) const;

/*
	// For the bravest: Direct Pixel access. Use carefully
	/// Needed if you want to blit directly to the screen by memcpy
	void force_disable_alpha();
	const SDL_PixelFormat * get_format() const;
	uint16_t get_pitch() const {return m_surface->pitch;}
*/

	const SDL_PixelFormat& format() const;
	uint8_t * get_pixels() const;
	uint16_t get_pitch() const
		{ return m_surface->pitch; }
	
	/// Lock
	void lock();
	void unlock();

	/// For the slowest: Indirect pixel access
	uint32_t get_pixel(uint32_t x, uint32_t y);
	void set_pixel(uint32_t x, uint32_t y, Uint32 clr);

	void clear();
	void draw_rect(Rect, RGBColor);
	void fill_rect(Rect, RGBAColor);
	void brighten_rect(Rect, int32_t factor);

	void draw_line
		(int32_t x1, int32_t y1,
		 int32_t x2, int32_t y2,
		 RGBColor color, const Rect * clip = NULL);

	void blit(Point, Surface *, Rect srcrc, bool enable_alpha = true);
	void fast_blit(Surface *);

	void set_subwin(Rect r);
	void unset_subwin();

private: 
	SurfaceSDL & operator= (SurfaceSDL const &);
        explicit SurfaceSDL(SurfaceSDL const &);

	SDL_Surface * m_surface;
};

#endif
