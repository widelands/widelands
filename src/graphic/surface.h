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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef SURFACE_H
#define SURFACE_H

#include "compositemode.h"
#include "rgbcolor.h"
#include "rect.h"
#include "surfaceptr.h"
#include "wexception.h"

#include <boost/noncopyable.hpp>

/**
 * Interface to a basic surfaces that can be used as destination for drawing.
 */
struct Surface : boost::noncopyable {
	Surface() {}
	virtual ~Surface() {}

	//@{
	/// Get width and height
	virtual uint32_t get_w() = 0;
	virtual uint32_t get_h() = 0;
	//@}

	/// Update the screen. This is only useful for the screen surface.
	virtual void update() = 0;

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
		 RGBColor color, uint8_t width = 1)
	{
		throw wexception("draw_line() not implemented");
	}

	/// makes a rectangle on the surface brighter (or darker).
	/// @note this is slow in SDL mode. Use with care
	virtual void brighten_rect(Rect, int32_t factor) = 0;

	/// This draws a part aother surface to this surface
	virtual void blit(Point, PictureID, Rect srcrc, Composite cm = CM_Normal) = 0;
	/// This draws another surface completely in the left
	/// upper corner of this surface
	virtual void fast_blit(PictureID surface) = 0;

	virtual IPixelAccess & pixelaccess() = 0;
};

#endif
