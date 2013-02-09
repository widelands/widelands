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

#include <string>
#include <map>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

#include "image_loader.h"
#include "log.h"
#include "picture.h"
#include "surface.h"
#include "surface_cache.h"

#include "image_cache.h"

using namespace std;


namespace  {

// NOCOM(#sirver): documentation
class FromDiskImage : public IPicture {
public:
	FromDiskImage(const string& filename, SurfaceCache* surface_cache, IImageLoader* image_loader) :
		filename_(filename),
		image_loader_(image_loader),
		surface_cache_(surface_cache) {
			Surface* surf = reload_image_();
			w_ = surf->width();
			h_ = surf->height();
		}

	// Implements IPicture.
	virtual uint16_t width() const {return w_; }
	virtual uint16_t height() const {return h_;}
	virtual const string& hash() const {return filename_;}
	virtual Surface* surface() const {
		Surface* surf = surface_cache_->get(filename_);
		if (surf)
			return surf;
		return reload_image_();
	}

private:
	Surface* reload_image_() const {
		log("Loading image %s.\n", filename_.c_str());
		Surface* surf = surface_cache_->insert(filename_, image_loader_->load(filename_));
		return surf;
	}
	uint16_t w_, h_;
	const string filename_;

	// Nothing owned
	IImageLoader* const image_loader_;
	SurfaceCache* const surface_cache_;
};

// NOCOM(#sirver): rename to ImageTrove

// NOCOM(#sirver): try to fix some of the cycles in image_cache, graphic and image_loader.
class ImageCacheImpl : public ImageCache {
public:
	// No ownership is taken here.
	ImageCacheImpl(IImageLoader* loader, SurfaceCache* surface_cache)
		: image_loader_(loader), surface_cache_(surface_cache) {
		}
	virtual ~ImageCacheImpl();

	// Implements ImageCache
	const IPicture* insert(const IPicture*);
	bool has(const std::string& hash) const;
	virtual const IPicture* get(const std::string& hash);

private:
	typedef map<string, const IPicture*> ImageMap;

	// hash of cached filename/picture pairs
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
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	return images_.find(hash) != images_.end();
}

const IPicture* ImageCacheImpl::insert(const IPicture* pic) {
	assert(!has(pic->hash()));
	images_.insert(make_pair(pic->hash(), pic));
	return pic;
}

const IPicture* ImageCacheImpl::get(const string& hash) {
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash, new FromDiskImage(hash, surface_cache_, image_loader_)));
		return get(hash);
	}
	return it->second;
}

// NOCOM(#sirver): maybe add a pruning step so that unused images are killed when possible.

}  // namespace

ImageCache* create_image_cache(IImageLoader* loader, SurfaceCache* surface_cache) {
	return new ImageCacheImpl(loader, surface_cache);
}

