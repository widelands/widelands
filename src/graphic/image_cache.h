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

class IPicture;

/**
 * Picture caches (modules).
 *
 * This cache is separated into different modules, and can be flushed
 * per-module.
 */
enum PicMod {
	PicMod_UI = 0,
	PicMod_Menu,
	PicMod_Game,
	PicMod_Text,
	PicMod_RichText,

	// Must be last
	PicMod_Last
};

/// This only knows how to load an image. It is the only thing that is
//implementation dependant in the ImageCache below.
class IImageLoader {
public:
	virtual ~IImageLoader() {}

	virtual IPicture* load(const std::string& fn, bool alpha) = 0;
};


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

	/// Returns an entry if it is cached, NULL otherwise.
	virtual const IPicture* get(PicMod, const std::string& hash) const = 0;

	/// Inserts this entry into the ImageCache. Overwrites existing entries /
	//without freeing the image first, so be careful. Returns the picture just
	//inserted / for convenience.
	virtual const IPicture* insert(PicMod, const std::string& hash, const IPicture*) = 0;

	/// Loads an Image from disk and caches it. If it was already
	/// cached, it is simply returned.
	virtual const IPicture* load(PicMod, const std::string& fn, bool alpha) = 0;

	/// Clears the Cache for the given PicMod
	virtual void flush(PicMod) = 0;
};

// Takes ownership of img_loader
ImageCache* create_image_cache(IImageLoader*);

#endif /* end of include guard: IMAGE_CACHE_H */

