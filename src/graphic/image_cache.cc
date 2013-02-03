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
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

#include "log.h"
#include "picture.h"
#include "picture_impl.h"
#include "image_loader.h"

#include "image_cache.h"

using namespace std;

namespace  {

// NOCOM(#sirver): documentation
class FromDiskImage : public IPicture {
public:
	FromDiskImage(const string& hash, bool alpha, IImageLoader* image_loader, SurfaceCache* surface_cache) :
		hash_(hash),
		alpha_(alpha),
		image_loader_(image_loader),
		surface_cache_(surface_cache) {
			reload_image_();
		}

	// Implements IPicture.
	virtual uint32_t get_w() const { return w_; }
	virtual uint32_t get_h() const { return h_; }
	virtual Surface* surface() const {
		Surface* surf = surface_cache_.get(hash_);
		if (surf)
			return surf;
		return reload_image_();
	}

private:
	Surface* reload_image_() {
		log("FromDiskImage: Loading %s.\n", hash_.c_str());
		Surface* surf = surface_cache_.insert(hash_, image_loader_->load(hash_, alpha_));

		w_ = surf->get_w();
		h_ = surf->get_h();
		return surf;
	}
	uint32_t w_, h_;

	const string hash_;
	const bool alpha_;
	// Nothing owned
	// NOCOM(#sirver): image_loader is really loading surfaces. Rename that sucker.
	IImageLoader* const image_loader_;
	SurfaceCache* const surface_cache_;
};

// NOCOM(#sirver): documnent me
class InMemoryImage : public IPicture {
public:
	InMemoryImage(Surface* surf) :
		surf_(surf) {}

	// Implements IPicture.
	virtual uint32_t get_w() const { return surf_->get_w(); }
	virtual uint32_t get_h() const { return surf_->get_h(); }
	virtual Surface* surface() const { return surf_; }

private:
	boost::scoped_ptr<Surface> surf_;
};

// NOCOM(#sirver): docu
class DerivedImage : public IPicture {
public:
	DerivedImage(const string& hash, const IPicture& original, SurfaceCache* surface_cache) :
		hash_(hash), original_(original), surface_cache_(surface_cache) {}
	virtual ~DerivedImage() {}

	// Implements IPicture.
	virtual uint32_t get_w() const {return original_->get_w();}
	virtual uint32_t get_h() const {return original_->get_h();}
	virtual Surface* surface() const {
		Surface* surf = surface_cache_.get(hash_);
		if (surf)
			return surf;

		surf = recalculate_surface();
		surface_cache_.insert(hash_, surf);
		return surf;
	}

	// To be implemented by child class.
	Surface* recalculate_surface() = 0;

protected:
	const string hash_;
	const IPicture original_;
	SurfaceCache* const surface_cache_;  // not owned
};
// NOCOM(#sirver): docu
class ResizedImage : public DerivedImage {
public:
	// NOCOM(#sirver): reconsider arguments.
	ResizedImage(const string& hash, const IPicture& original, uint32_t w, uint32_t h, SurfaceCache* surface_cache) :
		DerivedImage(hash, original_, surface_cache), w_(w), h_(h) {
			assert(w != original.get_w());
			assert(h != original.get_h());
	}

	// Implements IPicture.
	virtual uint32_t get_w() const { return w_; }
	virtual uint32_t get_h() const { return h_; }

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() {
		return g_gr->resize_surface(original_.surface(), w_, h_);
	}

private:
	uint32_t w_, h_;
}

class GrayedOutPic : public DerivedImage {
public:
	GrayedOutPic(const string& original_hash, const IPicture& original, SurfaceCache* surface_cache) :
		DerivedImage(hash, original_, surface_cache)
	{}

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() {
		return g_gr->gray_out_surface(original_.surface(), w_, h_);

	}
}

class ChangeLuminosityPic : public DerivedImage {
public:
	ChangeLuminosityPic(const string& hash, const IPicture& original, SurfaceCache* surface_cache, float factor, bool halve_alpha) :
		DerivedImage(original_, surface_cache), factor_(factor), halve_alpha_(halve_alpha)
	{}

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() {
		return g_gr->change_luminosity_of_surface(original_.surface(), factor_, halve_alpha_);
	}

private:
	float factor_;
	bool halve_alpha_;
}


class ImageCacheImpl : public ImageCache {
public:
	// No ownership is taken here.
	ImageCacheImpl(IImageLoader* loader, SurfaceCache* surface_cache)
		: image_loader_(loader), surface_cache_(surface_cache) {}
	~ImageCacheImpl();

	// Implements ImageCache
	virtual const IPicture& get(const string& hash, bool alpha);
	virtual const IPicture& new_permanent_picture(const string& hash, Surface*);

private:
	typedef map<string, IPicture*> ImageMap;

	// hash of cached filename/picture pairs
	ImageMap images_;
	IImageLoader* image_loader_;
	SurfaceCache* surface_cache_;
};

ImageCacheImpl::~ImageCacheImpl() {
	BOOST_FOREACH(ImageMap::value_type& p, images_)
		delete p.second;
	images_.clear();
}

const IPicture& ImageCacheImpl::get(const string& hash) {
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash, new FromDiskImage(hash, bool alpha, image_loader_, surface_cache_)));
		return get(hash);
	}
	return it->second.picture;
}

virtual const IPicture& ImageCacheImpl::get_resized(const std::string& hash, uint32_t w, uint32_t h) {
	const string new_hash = (boost::format("%s:%i:%i") % hash % w_ % h_).str();
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		const IPicture& original = get(hash);
		if (original->get_w() == w and original.get_h() == h) {
			return original;
		}
		images_insert(make_pair(new_hash, new ResizedImage(new_hash, original, surface_cache_, w, h)));
	}
	return get(new_hash);
}

virtual const IPicture& ImageCacheImpl::get_grayed_out(const std::string& hash) {
	const string new_hash = hash + ":greyed_out";
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		const IPicture& original = get(hash);
		images_insert(make_pair(new_hash, GrayedOutPic(new_hash, original, surface_cache_)));
	}
	return get(new_hash);
}
virtual const IPicture& get_changed_luminosity(const std::string& hash, float factor, bool halve_alpha) {
	const string new_hash = (boost::format("%s:%i:%i") % hash % static_cast<int>(factor*1000) % halve_alpha).str();
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		const IPicture& original = get(hash);
		images_insert(make_pair(new_hash, ChangeLuminosityPic(new_hash, original, surface_cache_, factor, halve_alpha)));
	}
	return get(new_hash);
}

const IPicture& ImageCacheImpl::new_permanent_picture(const string& hash, Surface* surf) {
	assert(images_.find(hash) == images_.end());
	images_.insert(make_pair(hash, new InMemoryImage(surf)));
	return get(hash);
}

}  // namespace

ImageCache* create_image_cache(IImageLoader* loader, SurfaceCache* surface_cache) {
	return new ImageCacheImpl(loader, surface_cache);
}

