/*
 * Copyright (C) 2010-2011 by the Widelands Development Team
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

#ifndef SDL_SURFACE_H
#define SDL_SURFACE_H

#include "rgbcolor.h"
#include "rect.h"

#include "graphic/surface.h"

/**
* This implements SDL rendering. Do not use this class directly. The right
* way is to use the base struct Surface wherever possible. Everything which
* needs to know about the underlying renderer should go to the graphics
* subdirectory.
* Surfaces are created through Graphic::create_surface() functions.
*/
class SDLSurface : public Surface {
public:
	SDLSurface(SDL_Surface & surface) :
		m_surface(&surface),
		m_offsx(0), m_offsy(0),
		m_w(surface.w), m_h(surface.h)
	{}
	virtual ~SDLSurface();

	// Implements IPicture
	virtual uint32_t get_w() const {return m_w;}
	virtual uint32_t get_h() const {return m_h;}

	// Implements IBlitableSurface
	virtual void blit(const Point&, const IPicture*, const Rect& srcrc, Composite cm);
	virtual void fill_rect(const Rect&, RGBAColor);

	// Implements Surface
	virtual void draw_rect(const Rect&, RGBColor);
	virtual void draw_line
		(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor&, uint8_t width);
	virtual void brighten_rect(const Rect&, int32_t factor);

	virtual SDL_PixelFormat const & format() const;
	virtual void lock(LockMode);
	virtual void unlock(UnlockMode);
	virtual uint32_t get_pixel(uint32_t x, uint32_t y);
	virtual void set_pixel(uint32_t x, uint32_t y, Uint32 clr);
	virtual uint16_t get_pitch() const {return m_surface->pitch;}
	virtual uint8_t * get_pixels() const;

	/// Set surface, only call once
	void set_sdl_surface(SDL_Surface & surface);
	SDL_Surface * get_sdl_surface() const {return m_surface;}

	void set_subwin(const Rect& r);
	void unset_subwin();

protected:
	SDL_Surface * m_surface;
	int32_t m_offsx;
	int32_t m_offsy;
	uint32_t m_w, m_h;
};


#endif /* end of include guard: SDL_SURFACE_H */
