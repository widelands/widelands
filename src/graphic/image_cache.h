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

#include "picture.h"

class IImageLoader;
class IPicture;
class Surface;
class SurfaceCache;

// NOCOM(#sirver): replace permanent image through a function that takes a CachableImage which
// is an image implementation.

// NOCOM(#sirver): check comment
// This class can load and cache images, so that they are already available
// when requested the next time. When another part of the program generates a
// new picture, it can also choose to insert it into this cache. The pictures
// in the cache are associated with a Picture Module. When a part of the
// program knows that it no longer needs pictures from a certain module, it can
// call flush to delete the images from the cache. The cache holds ownership
// over each image it contains.
class ImageCache : boost::noncopyable {
public:
	virtual ~ImageCache() {}

	/// Inserts this entry into the ImageCache. Overwrites existing entries /
	//without freeing the image first, so be careful. Returns the picture just
	//inserted / for convenience.
	// // NOCOM(#sirver): ownership is taken.
	// // NOCOM(#sirver): all methods return object and ownership is retained
	virtual const IPicture* new_permanent_picture(const std::string& hash, Surface*) = 0;
	// NOCOM(#sirver): maybe new_temporary_picture that might get deleted for rich text.

	/// Returns the image associated with the given hash. If no image by this hash is known,
	/// it will try to load one from disk with the filename = hash. If this fails, it will throw an
	/// error.
	// NOCOM(#sirver): document this
	virtual const IPicture* get(const std::string& hash, bool alpha = true) = 0;
	virtual const IPicture* render_text(const std::string& text, uint32_t w) = 0;
	virtual const IPicture* resize(const IPicture*, uint32_t w, uint32_t h) = 0;
	virtual const IPicture* gray_out(const IPicture*) = 0;
	virtual const IPicture* change_luminosity(const IPicture*, float factor, bool halve_alpha) = 0;
};

// NOCOM(#sirver): Should not take owernshi
//  of nothing
// Takes ownership of img_loader, but not of SurfaceCache.
ImageCache* create_image_cache(IImageLoader*, SurfaceCache*);


#endif /* end of include guard: IMAGE_CACHE_H */

