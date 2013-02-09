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

/**
 * Interface to a bitmap that can act as the source of a rendering
 * operation.
 */
class Surface; // // NOCOM(#sirver): remove again?
// TODO(#sirver): rename to Image ?
// NOCOM(#sirver): move to image_cache!
class IPicture : boost::noncopyable {
public:
	virtual ~IPicture() {}

	virtual uint16_t width() const = 0;
	virtual uint16_t height() const = 0;

	// NOCOM(#sirver): this somewhat leaks the implementation.
	// NOCOM(#sirver): Check if some casts become unnecessary now.
	virtual Surface* surface() const = 0;
	virtual const std::string& hash() const = 0;
};


#endif
