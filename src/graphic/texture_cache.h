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

#ifndef WL_GRAPHIC_TEXTURE_CACHE_H
#define WL_GRAPHIC_TEXTURE_CACHE_H

#include <string>

#include <boost/utility.hpp>

#include "base/macros.h"

class Texture;

// Caches Surfaces. It contains surfaces which must not be deleted and
// transient surfaces that are always free to be deleted - somebody else must
// then recreate them when they are needed again.
//
// Nobody in Widelands should hold onto a Surface they get from this class,
// instead, they should use it only temporarily and rerequest it whenever they
// need it.
class TextureCache {
public:
	TextureCache() {}
	virtual ~TextureCache() {}

	/// Deletes all surfaces in the cache leaving it as if it were just created.
	virtual void flush() = 0;

	/// Returns an entry if it is cached, nullptr otherwise.
	virtual Texture* get(const std::string& hash) = 0;

	// Inserts this entry into the TextureCache. asserts() that there is no
	// entry with this hash already cached. Returns the given Surface for
	// convenience. If 'transient' is false, this surface will not be deleted
	// automatically - use this if surfaces are around for a long time and
	// recreation is expensive (i.e. images loaded from disk).
	virtual Texture* insert(const std::string& hash, Texture*, bool transient) = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(TextureCache);
};

// Create a new Cache whichs combined pixels in all transient surfaces are
// always below the given limit (Note: there is overhead for class members
// which is not counted as the pixels make up the bulk of the size of a
// surface).
TextureCache* create_texture_cache(uint32_t transient_memory_in_bytes);

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_CACHE_H
