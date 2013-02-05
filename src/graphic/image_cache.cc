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

// NOCOM(#sirver): check all includes
#include <string>
#include <map>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "wexception.h"

#include "text/rt_render.h"
#include "text/rt_errors.h"
#include "io/filesystem/layered_filesystem.h"
#include "text/sdl_ttf_font.h"

#include "image_loader.h"
#include "log.h"
#include "picture.h"
#include "picture_impl.h"
#include "graphic.h"   // NOCOM(#sirver): cyclic dependency ftw.
#include "surface.h"
#include "surface_cache.h"

#include "image_cache.h"

using namespace std;


namespace  {

// NOCOM(#sirver): documentation
class FromDiskImage : public ImageImpl {
public:
	FromDiskImage(const string& filename, SurfaceCache* surface_cache, bool alpha, IImageLoader* image_loader) :
		filename_(filename),
		alpha_(alpha),
		image_loader_(image_loader),
		surface_cache_(surface_cache) {
			Surface* surf = reload_image_();
			w_ = surf->get_w();
			h_ = surf->get_h();
		}

	// Implements ImageImpl.
	virtual uint32_t get_w() const { return w_; }
	virtual uint32_t get_h() const { return h_; }
	virtual const string& hash() const { return filename_; }
	virtual Surface* surface() const {
		Surface* surf = surface_cache_->get(filename_);
		if (surf)
			return surf;
		return reload_image_();
	}

private:
	Surface* reload_image_() const {
		log("FromDiskImage: Loading %s.\n", filename_.c_str());
		Surface* surf = surface_cache_->insert(filename_, image_loader_->load(filename_, alpha_));
		return surf;
	}
	uint32_t w_, h_;

	const string filename_;
	const bool alpha_;
	// Nothing owned
	// NOCOM(#sirver): image_loader is really loading surfaces. Rename that sucker.
	IImageLoader* const image_loader_;
	SurfaceCache* const surface_cache_;
};

// NOCOM(#sirver): documnent me
class InMemoryImage : public ImageImpl {
public:
	InMemoryImage(const string& hash, Surface* surf) :
		hash_(hash), surf_(surf) {}

	// Implements ImageImpl.
	virtual uint32_t get_w() const { return surf_->get_w(); }
	virtual uint32_t get_h() const { return surf_->get_h(); }
	virtual const string& hash() const { return hash_; }
	virtual Surface* surface() const { return surf_.get(); }

private:
	const string hash_;
	boost::scoped_ptr<Surface> surf_;
};

// NOCOM(#sirver): docu
class DerivedImage : public ImageImpl {
public:
	DerivedImage(const string& hash, const ImageImpl& original, SurfaceCache* surface_cache) :
		hash_(hash), original_(original), surface_cache_(surface_cache) {}
	virtual ~DerivedImage() {}

	// Implements ImageImpl.
	virtual uint32_t get_w() const {return original_.get_w();}
	virtual uint32_t get_h() const {return original_.get_h();}
	virtual const string& hash() const { return hash_; }
	virtual Surface* surface() const {
		Surface* surf = surface_cache_->get(hash_);
		if (surf)
			return surf;

		surf = recalculate_surface();
		surface_cache_->insert(hash_, surf);
		return surf;
	}

	virtual Surface* recalculate_surface() const = 0;

protected:
	const string hash_;
	const ImageImpl& original_;
	SurfaceCache* const surface_cache_;  // not owned
};
// NOCOM(#sirver): docu
class ResizedImage : public DerivedImage {
public:
	// NOCOM(#sirver): reconsider arguments.
	ResizedImage(const string& hash, const ImageImpl& original, SurfaceCache* surface_cache, uint32_t w, uint32_t h) :
		DerivedImage(hash, original, surface_cache), w_(w), h_(h) {
			log("#sirver original_.hash(): %s\n", original_.hash().c_str());
			assert(w != original.get_w());
			assert(h != original.get_h());
	}

	// Overwrites DerivedImage.
	virtual uint32_t get_w() const { return w_; }
	virtual uint32_t get_h() const { return h_; }

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() const {
		log("#sirver hash: %s\n", hash().c_str());
		log("#sirver trying to resize: %s\n", original_.hash().c_str());
		Surface* rv = g_gr->resize_surface(original_.surface(), w_, h_);
		log("#sirver done\n");
		return rv;
	}

private:
	uint32_t w_, h_;
};

// NOCOM(#sirver): Mix between Pic and Image again.
class GrayedOutPic : public DerivedImage {
public:
	GrayedOutPic(const string& hash, const ImageImpl& original, SurfaceCache* surface_cache) :
		DerivedImage(hash, original, surface_cache)
	{}

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() const {
		return g_gr->gray_out_surface(original_.surface());

	}
};

class ChangeLuminosityPic : public DerivedImage {
public:
	ChangeLuminosityPic(const string& hash, const ImageImpl& original, SurfaceCache* surface_cache, float factor, bool halve_alpha) :
		DerivedImage(hash, original, surface_cache), factor_(factor), halve_alpha_(halve_alpha)
	{}

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() const {
		return g_gr->change_luminosity_of_surface(original_.surface(), factor_, halve_alpha_);
	}

private:
	float factor_;
	bool halve_alpha_;
};

class RTImage : public ImageImpl {
public:
	RTImage(const string& hash, SurfaceCache* surface_cache, RT::IRenderer* rt_renderer,
			const string& text, uint32_t width) :
		hash_(hash), surface_cache_(surface_cache), rt_renderer_(rt_renderer),
		text_(text), width_(width)
	{}

	// Implements ImageImpl.
	virtual uint32_t get_w() const {return surface()->get_w();}
	virtual uint32_t get_h() const {return surface()->get_h();}
	virtual const string& hash() const { return hash_; }
	virtual Surface* surface() const {
		Surface* surf = surface_cache_->get(hash_);
		if (surf)
			return surf;

		try {
			surf = static_cast<Surface*>(rt_renderer_->render(text_, width_));
			surface_cache_->insert(hash_, surf);
		} catch (RT::Exception& e) {
			throw wexception("Richtext rendering error: %s", e.what());
		}
		return surf;
	}

private:
	const string hash_;
	SurfaceCache* surface_cache_;
	RT::IRenderer* rt_renderer_;
	const string text_;
	uint32_t width_;
};

// NOCOM(#sirver): try to fix some of the cycles in image_cache, graphic and image_loader.
class ImageCacheImpl : public ImageCache {
public:
	// No ownership is taken here.
	ImageCacheImpl(IImageLoader* loader, SurfaceCache* surface_cache)
		: image_loader_(loader), surface_cache_(surface_cache) {
			// NOCOM(#sirver): fs should be passed in
			RT::IFontLoader * floader = RT::ttf_fontloader_from_filesystem(g_fs);
			rt_renderer_.reset(RT::setup_renderer(*g_gr, floader));
		}
	~ImageCacheImpl();

	// Implements ImageCache
	virtual const IPicture* new_permanent_picture(const std::string& hash, Surface*);
	virtual const IPicture* get(const std::string& hash, bool alpha = true);
	virtual const IPicture* render_text(const std::string& text, uint32_t w);
	virtual const IPicture* resize(const IPicture*, uint32_t w, uint32_t h);
	virtual const IPicture* gray_out(const IPicture*);
	virtual const IPicture* change_luminosity(const IPicture*, float factor, bool halve_alpha);

private:
	typedef map<string, ImageImpl*> ImageMap;

	// hash of cached filename/picture pairs
	ImageMap images_;
	IImageLoader* image_loader_;
	SurfaceCache* surface_cache_;
	boost::scoped_ptr<RT::IRenderer> rt_renderer_;
};

ImageCacheImpl::~ImageCacheImpl() {
	BOOST_FOREACH(ImageMap::value_type& p, images_)
		delete p.second;
	images_.clear();
}

const IPicture* ImageCacheImpl::get(const string& hash, bool alpha) {
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash, new FromDiskImage(hash, surface_cache_, alpha, image_loader_)));
		return get(hash);
	}
	return it->second;
}

const IPicture* ImageCacheImpl::resize(const IPicture* goriginal, uint32_t w, uint32_t h) {
	const ImageImpl *original = static_cast<const ImageImpl*>(goriginal);
	const string new_hash = (boost::format("%s:%i:%i") % original->hash() % w % h).str();
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ALIVE();
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
	ALIVE();
		if (goriginal->get_w() == w and goriginal->get_h() == h) {
	ALIVE();
			return goriginal;
	ALIVE();
		}
	ALIVE();
		images_.insert(make_pair(new_hash, new ResizedImage(new_hash, *original, surface_cache_, w, h)));
	ALIVE();
	}
	ALIVE();
	return get(new_hash);
}

const IPicture* ImageCacheImpl::gray_out(const IPicture* goriginal) {
	const ImageImpl *original = static_cast<const ImageImpl*>(goriginal);
	const string new_hash = original->hash() + ":greyed_out";
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		images_.insert(make_pair(new_hash, new GrayedOutPic(new_hash, *original, surface_cache_)));
	}
	return get(new_hash);
}
const IPicture* ImageCacheImpl::change_luminosity(const IPicture* goriginal, float factor, bool halve_alpha) {
	const ImageImpl *original = static_cast<const ImageImpl*>(goriginal);
	const string new_hash = (boost::format("%s:%i:%i") % original->hash() % static_cast<int>(factor*1000) % halve_alpha).str();
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		images_.insert(make_pair(new_hash, new ChangeLuminosityPic(new_hash, *original, surface_cache_, factor, halve_alpha)));
	}
	return get(new_hash);
}

const IPicture* ImageCacheImpl::new_permanent_picture(const string& hash, Surface* surf) {
	assert(images_.find(hash) == images_.end());
	images_.insert(make_pair(hash, new InMemoryImage(hash, surf)));
	return get(hash);
}

// NOCOM(#sirver): maybe add a pruning step so that unused images are killed when possible.
const IPicture* ImageCacheImpl::render_text(const std::string& text, uint32_t width) {
	const string hash = boost::lexical_cast<string>(width) + text;
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash,
					new RTImage(hash, surface_cache_, rt_renderer_.get(), text, width)));
	}
	return get(hash);
}

}  // namespace

ImageCache* create_image_cache(IImageLoader* loader, SurfaceCache* surface_cache) {
	return new ImageCacheImpl(loader, surface_cache);
}

