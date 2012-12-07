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

#ifndef IBLITABLE_SURFACE_H
#define IBLITABLE_SURFACE_H

#include <boost/noncopyable.hpp>

#include "compositemode.h"
#include "picture.h"
#include "pixelaccess.h"
#include "point.h"
#include "rgbcolor.h"
#include "rect.h"

/**
 * Interface to a basic surfaces that can be used as destination for blitting.
 */
class IBlitableSurface : public virtual IPicture {
public:
	virtual ~IBlitableSurface() {}

	/// This draws a part of another surface to this surface
	virtual void blit(const Point&, const IPicture*, const Rect& srcrc, Composite cm = CM_Normal) = 0;

	/// Draws a filled rect to the surface.
	virtual void fill_rect(const Rect&, RGBAColor) = 0;
};

#endif
