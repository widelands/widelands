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

#ifndef IMAGE_H
#define IMAGE_H

#include <string>

#include <boost/noncopyable.hpp>
#include <stdint.h>

/**
 * Interface to a bitmap that can act as the source of a rendering
 * operation.
 */
class Surface;

class Image : boost::noncopyable {
public:
	virtual ~Image() {}

	virtual uint16_t width() const = 0;
	virtual uint16_t height() const = 0;

	// Internal functions needed for caching.
	virtual Surface* surface() const = 0;
	virtual const std::string& hash() const = 0;
};


#endif
