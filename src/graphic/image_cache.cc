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

#include "image_loader.h"
#include "log.h"
#include "picture.h"
#include "graphic.h"   // NOCOM(#sirver): cyclic dependency ftw.
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
	virtual uint16_t width() const { return w_; }
	virtual uint16_t height() const { return h_; }
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
		Surface* surf = surface_cache_->insert(filename_, image_loader_->load(filename_));
		return surf;
	}
	uint16_t w_, h_;

	const string filename_;
	// Nothing owned
	IImageLoader* const image_loader_;
	SurfaceCache* const surface_cache_;
};

// NOCOM(#sirver): documnent me
class InMemoryImage : public IPicture {
public:
	InMemoryImage(const string& hash, Surface* surf) :
		hash_(hash), surf_(surf) {}

	// Implements IPicture.
	virtual uint16_t width() const { return surf_->width(); }
	virtual uint16_t height() const { return surf_->height(); }
	virtual const string& hash() const { return hash_; }
	virtual Surface* surface() const { return surf_.get(); }

private:
	const string hash_;
	boost::scoped_ptr<Surface> surf_;
};

// NOCOM(#sirver): docu
class DerivedImage : public IPicture {
public:
	DerivedImage(const string& hash, const IPicture& original, SurfaceCache* surface_cache) :
		hash_(hash), original_(original), surface_cache_(surface_cache) {}
	virtual ~DerivedImage() {}

	// Implements IPicture.
	virtual uint16_t width() const {return original_.width();}
	virtual uint16_t height() const {return original_.height();}
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
	const IPicture& original_;
	SurfaceCache* const surface_cache_;  // not owned
};
// NOCOM(#sirver): docu
class ResizedImage : public DerivedImage {
public:
	// NOCOM(#sirver): reconsider arguments.
	ResizedImage(const string& hash, const IPicture& original, SurfaceCache* surface_cache, uint16_t w, uint16_t h) :
		DerivedImage(hash, original, surface_cache), w_(w), h_(h) {
			assert(w != original.width() || h != original.height());
	}

	// Overwrites DerivedImage.
	virtual uint16_t width() const { return w_; }
	virtual uint16_t height() const { return h_; }

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() const {
		Surface* rv = g_gr->resize_surface(original_.surface(), w_, h_);
		return rv;
	}

private:
	uint16_t w_, h_;
};

// NOCOM(#sirver): Mix between Pic and Image again.
class GrayedOutPic : public DerivedImage {
public:
	GrayedOutPic(const string& hash, const IPicture& original, SurfaceCache* surface_cache) :
		DerivedImage(hash, original, surface_cache)
	{}

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() const {
		return g_gr->gray_out_surface(original_.surface());
	}
};

class ChangeLuminosityPic : public DerivedImage {
public:
	ChangeLuminosityPic(const string& hash, const IPicture& original, SurfaceCache* surface_cache, float factor, bool halve_alpha) :
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

class PlayerColoredImage : public DerivedImage {
public:
	PlayerColoredImage(const string& hash, const IPicture& original, SurfaceCache* surface_cache, const RGBColor& color, const IPicture& mask) :
		DerivedImage(hash, original, surface_cache), color_(color), mask_(mask) {}

	// Implements DerivedImage.
	virtual Surface* recalculate_surface() const {
		// Encodes the given Image into the corresponding image for player color.
		// Takes the neutral set of images and the player color mask.
		Surface& orig_surface = *original_.surface();
		Surface& pcmask_surface = *mask_.surface();

		Surface* new_surface = g_gr->create_surface(orig_surface.width(), orig_surface.height());

		const SDL_PixelFormat & fmt = orig_surface.format();
		const SDL_PixelFormat & fmt_pc = pcmask_surface.format();
		const SDL_PixelFormat & destfmt = new_surface->format();

		orig_surface.lock(Surface::Lock_Normal);
		pcmask_surface.lock(Surface::Lock_Normal);
		new_surface->lock(Surface::Lock_Discard);
		// This could be done significantly faster, but since we
		// cache the result, let's keep it simple for now.
		for (uint32_t y = 0; y < orig_surface.height(); ++y) {
			for (uint32_t x = 0; x < orig_surface.width(); ++x) {
				RGBAColor source;
				RGBAColor mask;
				RGBAColor product;

				source.set(fmt, orig_surface.get_pixel(x, y));
				mask.set(fmt_pc, pcmask_surface.get_pixel(x, y));

				if
					(uint32_t const influence =
					 static_cast<uint32_t>(mask.r) * mask.a)
					{
						uint32_t const intensity =
							(luminance_table_r[source.r] +
							 luminance_table_g[source.g] +
							 luminance_table_b[source.b] +
							 8388608U) //  compensate for truncation:  .5 * 2^24
							>> 24;
						RGBAColor plrclr;

						plrclr.r = (color_.r * intensity) >> 8;
						plrclr.g = (color_.g * intensity) >> 8;
						plrclr.b = (color_.b * intensity) >> 8;

						product.r =
							(plrclr.r * influence + source.r * (65536 - influence)) >> 16;
						product.g =
							(plrclr.g * influence + source.g * (65536 - influence)) >> 16;
						product.b =
							(plrclr.b * influence + source.b * (65536 - influence)) >> 16;
						product.a = source.a;
					} else {
						product = source;
					}

				new_surface->set_pixel(x, y, product.map(destfmt));
			}
		}
		orig_surface.unlock(Surface::Unlock_NoChange);
		pcmask_surface.unlock(Surface::Unlock_NoChange);
		new_surface->unlock(Surface::Unlock_Update);

		return new_surface;
	}

private:
	const RGBColor& color_;
	const IPicture& mask_;
};

// NOCOM(#sirver): rename to ImageTrove
class RTImage : public IPicture {
public:
	RTImage(const string& hash, SurfaceCache* surface_cache, RT::IRenderer* rt_renderer,
			const string& text, uint16_t width) :
		hash_(hash), surface_cache_(surface_cache), rt_renderer_(rt_renderer),
		text_(text), width_(width)
	{}

	// Implements IPicture.
	virtual uint16_t width() const {return surface()->width();}
	virtual uint16_t height() const {return surface()->height();}
	virtual const string& hash() const { return hash_; }
	virtual Surface* surface() const {
		Surface* surf = surface_cache_->get(hash_);
		if (surf)
			return surf;

		try {
			surf = rt_renderer_->render(text_, width_);
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
	uint16_t width_;
};

// NOCOM(#sirver): try to fix some of the cycles in image_cache, graphic and image_loader.
class ImageCacheImpl : public ImageCache {
public:
	// No ownership is taken here.
	ImageCacheImpl(IImageLoader* loader, SurfaceCache* surface_cache, RT::IRenderer* rt_renderer)
		: image_loader_(loader), surface_cache_(surface_cache), rt_renderer_(rt_renderer) {
		}
	virtual ~ImageCacheImpl();

	// Implements ImageCache
	virtual const IPicture* new_permanent_picture(const std::string& hash, Surface*);
	virtual const IPicture* get(const std::string& hash);
	virtual const IPicture* render_text(const std::string& text, uint16_t w);
	virtual const IPicture* resize(const IPicture*, uint16_t w, uint16_t h);
	virtual const IPicture* gray_out(const IPicture*);
	virtual const IPicture* change_luminosity(const IPicture*, float factor, bool halve_alpha);
	virtual const IPicture* player_colored(const RGBColor&, const IPicture*, const IPicture*);

private:
	typedef map<string, IPicture*> ImageMap;

	// hash of cached filename/picture pairs
	ImageMap images_;

	// None of these are owned.
	IImageLoader* const image_loader_;
	SurfaceCache* const surface_cache_;
	RT::IRenderer* const rt_renderer_;
};

ImageCacheImpl::~ImageCacheImpl() {
	BOOST_FOREACH(ImageMap::value_type& p, images_)
		delete p.second;
	images_.clear();
}

const IPicture* ImageCacheImpl::get(const string& hash) {
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash, new FromDiskImage(hash, surface_cache_, image_loader_)));
		return get(hash);
	}
	return it->second;
}

const IPicture* ImageCacheImpl::resize(const IPicture* original, uint16_t w, uint16_t h) {
	const string new_hash = (boost::format("%s:%i:%i") % original->hash() % w % h).str();
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		if (original->width() == w and original->height() == h) {
			return original;
		}
		images_.insert(make_pair(new_hash, new ResizedImage(new_hash, *original, surface_cache_, w, h)));
	}
	return get(new_hash);
}

const IPicture* ImageCacheImpl::gray_out(const IPicture* original) {
	const string new_hash = original->hash() + ":greyed_out";
	log("#sirver original->hash: %s\n", original->hash().c_str());
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		images_.insert(make_pair(new_hash, new GrayedOutPic(new_hash, *original, surface_cache_)));
	}
	return get(new_hash);
}
const IPicture* ImageCacheImpl::change_luminosity(const IPicture* original, float factor, bool halve_alpha) {
	const string new_hash = (boost::format("%s:%i:%i") % original->hash() % static_cast<int>(factor*1000) % halve_alpha).str();
	// NOCOM(#sirver): a simple 'contains' would be good here. count?
	ImageMap::const_iterator it = images_.find(new_hash);
	if (it == images_.end()) {
		images_.insert(make_pair(new_hash, new ChangeLuminosityPic(new_hash, *original, surface_cache_, factor, halve_alpha)));
	}
	return get(new_hash);
}

const IPicture* ImageCacheImpl::player_colored(const RGBColor& clr, const IPicture* original, const IPicture* mask) {
	const string hash = (boost::format("%s:%02x%02x%02x") % original->hash() % clr.r % clr.g % clr.b).str();
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash,
					new PlayerColoredImage(hash, *original, surface_cache_, clr, *mask)));
	}
	return get(hash);
}

const IPicture* ImageCacheImpl::new_permanent_picture(const string& hash, Surface* surf) {
	assert(images_.find(hash) == images_.end());
	images_.insert(make_pair(hash, new InMemoryImage(hash, surf)));
	return get(hash);
}

// NOCOM(#sirver): maybe add a pruning step so that unused images are killed when possible.
const IPicture* ImageCacheImpl::render_text(const std::string& text, uint16_t width) {
	const string hash = boost::lexical_cast<string>(width) + text;
	ImageMap::const_iterator it = images_.find(hash);
	if (it == images_.end()) {
		images_.insert(make_pair(hash,
					new RTImage(hash, surface_cache_, rt_renderer_, text, width)));
	}
	return get(hash);
}



}  // namespace

ImageCache* create_image_cache(IImageLoader* loader, SurfaceCache* surface_cache, RT::IRenderer* rt_renderer) {
	return new ImageCacheImpl(loader, surface_cache, rt_renderer);
}

IPicture* new_uncached_image(Surface* surf) {
	return new InMemoryImage("", surf);
}

