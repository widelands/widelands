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

#include "compositemode.h"
#include "picture_id.h"
#include "point.h"
#include "rect.h"

#include <boost/noncopyable.hpp>

/**
 * Interface to a basic surfaces that can be used as destination for blitting.
 */
struct IBlitableSurface : boost::noncopyable {
	virtual ~IBlitableSurface() {}

	//@{
	/// Get width and height
	virtual uint32_t get_w() = 0; // TODO(sirver): Should only be w()
	virtual uint32_t get_h() = 0;
	//@}

	/// This draws a part aother surface to this surface
	virtual void blit(Point, PictureID, Rect srcrc, Composite cm = CM_Normal) = 0;
};

#endif
