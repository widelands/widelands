/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef PICTURE_H
#define PICTURE_H

#include <stdint.h>
#include <string>

#include <boost/noncopyable.hpp>

#include "picture_id.h"

struct IPixelAccess;
struct Surface;

/**
 * Interface to a bitmap that can act as the source of a rendering
 * operation.
 */
struct IPicture : boost::noncopyable {
	IPicture() {}
	virtual ~IPicture() {}

	virtual bool valid() = 0;

	virtual uint32_t get_w() = 0;
	virtual uint32_t get_h() = 0;

	virtual IPixelAccess & pixelaccess() = 0;

public:
	static const PictureID & null();
};

#endif
