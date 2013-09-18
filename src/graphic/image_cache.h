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

#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include <string>

#include <boost/utility.hpp>

#include "graphic/image.h"

class IImageLoader;
class SurfaceCache;

// For historic reasons, most part of the Widelands code base expect that an
// Image stays valid for the whole duration of the program run. This class is
// the one that keeps ownership of all Images to ensure that this is true. Also
// for historic reasons, this class will try to load in Image from disk when
// its hash is not found. Other parts of Widelands will create images when they
// do not exist in the cache yet and then put it into the cache and therefore
// releasing their ownership.
class ImageCache : boost::noncopyable {
public:
	virtual ~ImageCache() {}

	// Insert the given Image into the cache. The hash is defined by Image's hash()
	// function. Ownership of the Image is taken. Will return a pointer to the freshly inserted
	// image for convenience.
	virtual const Image* insert(const Image*) = 0;

	// Returns the image associated with the given hash. If no image by this
	// hash is known, it will try to load one from disk with the filename =
	// hash. If this fails, it will throw an error.
	virtual const Image* get(const std::string& hash) = 0;

	// Returns true if the given hash is stored in the cache.
	virtual bool has(const std::string& hash) const = 0;

};

// Create a new ImageCache. Takes no ownership.
ImageCache* create_image_cache(IImageLoader*, SurfaceCache*);

#endif /* end of include guard: IMAGE_CACHE_H */

