/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <string>

#include <boost/utility.hpp>

#include "base/macros.h"
#include "graphic/text/rendered_text.h"

class Image;

// Caches transient Surfaces, i.e. those that are always free to be deleted
// because they can be regenerated - somebody else must then recreate them when
// they are needed again.
//
// Nothing in Widelands should hold onto a Surface they get from this class,
// instead, they should use it only temporarily and rerequest it whenever they
// need it.
class TextureCache {
public:
	// Create a new Cache whichs combined pixels data in all transient surfaces
	// are always below the 'max_size_in_bytes'.
	TextureCache(uint32_t max_size_in_bytes);
	~TextureCache();

	/// Deletes all surfaces in the cache leaving it as if it were just created.
	void flush();

	/// Returns an entry if it is cached, nullptr otherwise.
	const Image* get(const std::string& hash);

	// Inserts this entry into the TextureCache. asserts() that there is no
	// entry with this hash already cached. Returns the given Surface for
	// convenience. If 'transient' is false, this surface will not be deleted
	// automatically - use this if surfaces are around for a long time and
	// recreation is expensive (i.e. images loaded from disk).
	const Image* insert(const std::string& hash, std::unique_ptr<const Image> texture);

private:
	void drop();

	using AccessHistory = std::list<std::string>;
	struct Entry {
		std::unique_ptr<const Image> texture;
		uint32_t last_access;  // Mainly for debugging and analysis.
		const AccessHistory::iterator list_iterator;
	};

	uint32_t max_size_in_bytes_;
	uint32_t size_in_bytes_;
	std::map<std::string, Entry> entries_;
	AccessHistory access_history_;

	DISALLOW_COPY_AND_ASSIGN(TextureCache);
};

#endif  // end of include guard: WL_GRAPHIC_TEXTURE_CACHE_H
