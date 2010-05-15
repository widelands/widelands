/*
 * Copyright 2010 by the Widelands Development Team
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

#ifndef SURFACE_H
#define SURFACE_H

#include "rgbcolor.h"
#include "rect.h"
#include "wexception.h"

enum SurfaceType {
	SURFACE_INVALID,
	SURFACE_SOURCE,		// This sourface is used as source only
	SURFACE_OFFSCREEN,	// Use surface as source and destinantion
	SURFACE_SCREEN		// This draws to screen directly
};
	

/**
 * This represents a simple bitmap without managing its memory. The rendering
 * functions do NOT perform any clipping; this is up to the caller.
*/
class Surface {

public:
	virtual ~Surface() {}
	/// Get width and height
	virtual uint32_t get_w() const {return m_w;}
	virtual uint32_t get_h() const {return m_h;}
	virtual void update() = 0;

	/// For the slowest: Indirect pixel access
	virtual uint32_t get_pixel(uint32_t x, uint32_t y)
		{ throw wexception("get_pixel() not implemented"); }
	virtual void set_pixel(uint32_t x, uint32_t y, Uint32 clr)
		{ throw wexception("set_pixel() not implemented"); }

	virtual void lock() {};
	virtual void unlock() {};

	virtual const SDL_PixelFormat& format() const
		{ throw wexception("format() not implemented"); }
	virtual uint16_t get_pitch() const
		{ throw wexception("get_pitch() not implemented"); }
	virtual uint8_t * get_pixels() const
		{ throw wexception("get_pixels() not implemented"); }

	virtual void clear() {
		fill_rect
			(Rect(Point(0,0),get_w(), get_h()), 
			 RGBAColor(255, 255, 255, 255));
	}
	
	virtual void draw_rect(Rect, RGBColor) = 0;
	virtual void fill_rect(Rect, RGBAColor) = 0;
	virtual void brighten_rect(Rect, int32_t factor) = 0;

	virtual void blit(Point, Surface *, Rect srcrc, bool enable_alpha = false) = 0;
	virtual void fast_blit(Surface * surface) {
		blit(Point(0,0), surface, Rect(Point(0, 0), surface->get_w(), surface->get_h()));
	}

	virtual void set_type(SurfaceType type)
		{ m_surf_type = type; }
/*
	virtual void draw_minimap
		(Widelands::Editor_Game_Base const &,
		 Widelands::Player           const *,
		 Rect, Point, uint32_t flags) = 0;
*/
	virtual SurfaceType get_surface_type() { return m_surf_type; }
	
	
	
protected:
	int32_t m_offsx;
	int32_t m_offsy;
	uint32_t m_w, m_h;

	Surface():
		m_offsx(0), m_offsy(0),
		m_w(0), m_h(0),
		m_surf_type(SURFACE_INVALID)
	{}
	Surface(int w, int h, SurfaceType t):
		m_offsx(0), m_offsy(0),
		m_w(w), m_h(h),
		m_surf_type(t)
	{}
	SurfaceType m_surf_type;
private:
	Surface & operator= (Surface const &);
};

#endif
