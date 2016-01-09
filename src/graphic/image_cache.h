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

#ifndef WL_GRAPHIC_IMAGE_CACHE_H
#define WL_GRAPHIC_IMAGE_CACHE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include "base/macros.h"
#include "graphic/image.h"
#include "graphic/texture_atlas.h"

// For historic reasons, most part of the Widelands code base expect that an
// Image stays valid for the whole duration of the program run. This class is
// the one that keeps ownership of all Images to ensure that this is true.
// Other parts of Widelands will create images when they do not exist in the
// cache yet and then put it into the cache and therefore releasing their
// ownership.
class ImageCache {
public:
	ImageCache();
	~ImageCache();

	// Insert the given Image into the cache.
	// Will return a pointer to the freshly inserted image for convenience.
	const Image* insert(const std::string& hash, std::unique_ptr<const Image> image);

	// Returns the image associated with the given hash. If no image by this
	// hash is known, it will try to load one from disk with the filename =
	// hash. If this fails, it will throw an error.
	const Image* get(const std::string& hash);

	// Returns true if the given hash is stored in the cache.
	bool has(const std::string& hash) const;

	// Loads the cache from texture atlases on disk.
	void fill_with_texture_atlas();

private:
	std::vector<std::unique_ptr<Texture>> texture_atlases_;
	std::map<std::string, std::unique_ptr<const Image>> images_;

	DISALLOW_COPY_AND_ASSIGN(ImageCache);
};

#endif  // end of include guard: WL_GRAPHIC_IMAGE_CACHE_H
