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
#include <map>
#include <string>

#include <boost/foreach.hpp>

#include "graphic/image.h"
#include "graphic/image_loader.h"
#include "graphic/surface.h"
#include "graphic/surface_cache.h"
#include "log.h"


using namespace std;


namespace  {

// Image Implementation that loads images from disc when they should be drawn.
// Uses SurfaceCache. These images are meant to be cached in ImageCache.
class FromDiskImage : public Image {
public:
	FromDiskImage(const string& filename, SurfaceCache* surface_cache, IImageLoader* image_loader) :
		filename_(filename),
		image_loader_(image_loader),
		surface_cache_(surface_cache) {
			Surface* surf = reload_image_();
			w_ = surf->width();
			h_ = surf->height();
		}
	virtual ~FromDiskImage() {}

	// Implements Image.
	virtual uint16_t width() const override {return w_; }
	virtual uint16_t height() const override {return h_;}
	virtual const string& hash() const override {return filename_;}
	virtual Surface* surface() const override {
		Surface* surf = surface_cache_->get(filename_);
		if (surf)
			return surf;
		return reload_image_();
	}

private:
	Surface* reload_image_() const {
		Surface* surf = surface_cache_->insert(filename_, image_loader_->load(filename_), false);
		return surf;
	}
	uint16_t w_, h_;
	const string filename_;

	// Nothing owned
	IImageLoader* const image_loader_;
	SurfaceCache* const surface_cache_;
};

class ImageCacheImpl : public ImageCache {
public:
	// No ownership is taken here.
	ImageCacheImpl(IImageLoader* loader, SurfaceCache* surface_cache)
		: image_loader_(loader), surface_cache_(surface_cache) {
		}
	virtual ~ImageCacheImpl();

	// Implements ImageCache.
	const Image* insert(const Image*) override;
	bool has(const std::string& hash) const override;
	virtual const Image* get(const std::string& hash) override;

private:
	typedef map<string, const Image*> ImageMap;

	// hash of cached filename/image pairs
	ImageMap images_;

	// None of these are owned.
	IImageLoader* const image_loader_;
	SurfaceCache* const surface_cache_;
};

ImageCacheImpl::~ImageCacheImpl() {
	BOOST_FOREACH(ImageMap::value_type& p, images_)
		delete p.second;
	images_.clear();
}

bool ImageCacheImpl::has(const string& hash) const {
	return images_.count(hash);
}

const Image* ImageCacheImpl::insert(const Image* image) {
	assert(!has(image->hash()));
	images_.insert(make_pair(image->hash(), image));
	return image;
}

const Image* ImageCacheImpl::get(const string& hash) {
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash, new FromDiskImage(hash, surface_cache_, image_loader_)));
		return get(hash);
	}
	return it->second;
}

}  // namespace

ImageCache* create_image_cache(IImageLoader* loader, SurfaceCache* surface_cache) {
	return new ImageCacheImpl(loader, surface_cache);
}

