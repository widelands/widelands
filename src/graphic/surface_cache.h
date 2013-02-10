/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef SURFACE_CACHE_H
#define SURFACE_CACHE_H

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <string>

class Surface;

// Caches Surfaces that are expensive to create. Is free to delete them at any
// time when it feels like it uses too much memory. It will then delete the
// surfaces that have been used the longest time ago. Users of this class
// should therefore not hold onto the Surface they get back, instead, they
// should use it only temporarily and request it whenever they need it.
class SurfaceCache : boost::noncopyable {
public:
	SurfaceCache() {};
	virtual ~SurfaceCache() {};

	/// Returns an entry if it is cached, NULL otherwise.
	virtual Surface* get(const std::string& hash) = 0;

	/// Inserts this entry into the SurfaceCache. Overwrites existing entries /
	/// without freeing them so be careful.
	/// // NOCOM(#sirver): crashes if hash is present.
	virtual Surface* insert(const std::string& hash, Surface*) = 0;
};

SurfaceCache* create_surface_cache(uint32_t memory_in_bytes);

#endif /* end of include guard: SURFACE_CACHE_H */

