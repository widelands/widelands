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

#include "iblitable_surface.h"
#include "rect.h"
#include "rgbcolor.h"
#include "wexception.h"

#include <boost/noncopyable.hpp>

/**
 * Interface to a basic surfaces that can be used as destination for blitting and drawing.
 * It also allows low level pixel access.
 */
class Surface : public virtual IBlitableSurface {
public:
	virtual ~Surface() {}

	/// Update the screen. This is only useful for the screen surface.
	virtual void update() = 0;

	/// Draws a rect (frame only) to the surface.
	virtual void draw_rect(const Rect&, RGBColor) = 0;

	/// draw a line to the surface
	// TODO(sirver): why not pure virtual?
	virtual void draw_line
		(int32_t /* x1 */,
		 int32_t /* y1 */,
		 int32_t /* x2 */,
		 int32_t /* y2 */,
		 RGBColor /* color */,
		 uint8_t /* width = 1 */)
	{
		throw wexception("draw_line() not implemented");
	}

	/// makes a rectangle on the surface brighter (or darker).
	/// @note this is slow in SDL mode. Use with care
	virtual void brighten_rect(const Rect&, int32_t factor) = 0;

	virtual IPixelAccess & pixelaccess() = 0;
};

#endif
