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

/**
 * The type of a surface. This make it possible to check a bit before drawing.
 */
enum SurfaceType {
	SURFACE_INVALID,
	SURFACE_SOURCE,       ///< This sourface is used as source only
	SURFACE_OFFSCREEN,    ///< Use surface as source and destinantion
	SURFACE_SCREEN        ///< This draws to screen directly
};

/// A virtual base for rendering. Objects of Surface are used as Source or
/// destination for drawing. Surfaces are created with the
/// Graphic::create_surface() functions.
struct Surface {
	virtual ~Surface() {}

	//@{
	/// Get width and height
	virtual uint32_t get_w() const {return m_w;}
	virtual uint32_t get_h() const {return m_h;}
	//@}

	/// Update the screen. This is only useful for the screen surface.
	virtual void update() = 0;

	//@{
	/// For the slowest: Indirect pixel access.
	/// A safe function to get and set single pixels. lock() must be called
	/// before pixel access can be used. get_pixel() and set_pixel() are easier
	/// and safer to use but also much slower than direct pixel access.
	virtual uint32_t get_pixel(uint32_t x, uint32_t y) {
		//  FIXME make abstract or move body out of declaration
		throw wexception("get_pixel() not implemented");
	}
	virtual void set_pixel(uint32_t x, uint32_t y, Uint32 clr) {
		//  FIXME make abstract or move body out of declaration
		throw wexception("set_pixel() not implemented");
	}
	//@}

	//@{
	/**
	 * Locking and unlocking the surface for pixel access. This may be slow. So
	 * use it with care.
	 */

	virtual void lock() {};
	virtual void unlock() {};
	//@}

	/// This returns the pixel format for direct pixel access.
	virtual SDL_PixelFormat const & format() const {
		throw wexception("format() not implemented");
	}

	//@{
	/**
	 * Direct pixel access. lock() must be called before pixles may be access.
	 * This is faster than indirect pixel access but also more dangerous.
	 * get_pixels() gives a pointer to the pixel data. get_pitch() returns an
	 * integer where the next row begins.
	 */
	virtual uint16_t get_pitch() const {
		throw wexception("get_pitch() not implemented");
	}
	virtual uint8_t * get_pixels() const {
		throw wexception("get_pixels() not implemented");
	}
	//@}

	/// Clears the complete surface to black.
	virtual void clear() {
		fill_rect
			(Rect(Point(0, 0), get_w(), get_h()), RGBAColor(255, 255, 255, 255));
	}

	/// Draws a rect (frame only) to the surface.
	virtual void draw_rect(Rect, RGBColor) = 0;

	/// Draws a filled rect to the surface.
	virtual void fill_rect(Rect, RGBAColor) = 0;

	/// draw a line to the surface
	virtual void draw_line
		(int32_t x1,
		 int32_t y1,
		 int32_t x2,
		 int32_t y2,
		 RGBColor color,
		 Rect const * clip = 0)
	{
		throw wexception("draw_line() not implemented");
	}

	/// makes a rectangle on the surface brighter (or darker).
	/// @note this is slow in SDL mode. Use with care
	virtual void brighten_rect(Rect, int32_t factor) = 0;

	/// This draws a part aother surface to this surface
	virtual void blit(Point, Surface *, Rect srcrc, bool enable_alpha = false) = 0;
	/// This draws another surface completely in the left
	/// upper corner of this surface
	virtual void fast_blit(Surface * surface) {
		blit
			(Point(0, 0),
			 surface,
			 Rect(Point(0, 0), surface->get_w(), surface->get_h()));
	}

	/// set the type of the surface
	virtual void set_type(SurfaceType const type) {m_surf_type = type;}

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
