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

#include "graphic/image_cache.h"

#include <cassert>
#include <string>

#include "base/log.h"
#include "graphic/image.h"
#include "graphic/image_io.h"
#include "graphic/surface.h"
#include "graphic/surface_cache.h"

namespace  {

// Image Implementation that loads images from disc when they should be drawn.
// Uses SurfaceCache. These images are meant to be cached in ImageCache.
class FromDiskImage : public Image {
public:
	FromDiskImage(const std::string& filename, SurfaceCache* surface_cache) :
		filename_(filename),
		surface_cache_(surface_cache) {
			Surface* surf = reload_image_();
			w_ = surf->width();
			h_ = surf->height();
		}
	virtual ~FromDiskImage() {}

	// Implements Image.
	uint16_t width() const override {return w_; }
	uint16_t height() const override {return h_;}
	const std::string& hash() const override {return filename_;}
	Surface* surface() const override {
		Surface* surf = surface_cache_->get(filename_);
		if (surf)
			return surf;
		return reload_image_();
	}

private:
	Surface* reload_image_() const {
		Surface* surf = surface_cache_->insert(filename_, load_image(filename_), false);
		return surf;
	}
	uint16_t w_, h_;
	const std::string filename_;

	SurfaceCache* const surface_cache_;  // Not owned.
};

}  // namespace

ImageCache::ImageCache(SurfaceCache* const surface_cache) : surface_cache_(surface_cache) {
}

ImageCache::~ImageCache() {
	for (ImageMap::value_type& p : images_) {
		delete p.second;
	}
	images_.clear();
}

bool ImageCache::has(const std::string& hash) const {
	return images_.count(hash);
}

const Image* ImageCache::insert(const Image* image) {
	assert(!has(image->hash()));
	images_.insert(make_pair(image->hash(), image));
	return image;
}

const Image* ImageCache::get(const std::string& hash) {
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash, new FromDiskImage(hash, surface_cache_)));
		return get(hash);
	}
	return it->second;
}
