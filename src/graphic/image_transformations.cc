/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#include "graphic/image_transformations.h"

#include <string>

#include <SDL.h>
#include <SDL_rotozoom.h>
#include <boost/format.hpp>

#include "graphic/graphic.h"
#include "graphic/render/sdl_surface.h"
#include "graphic/surface.h"
#include "graphic/surface_cache.h"
#include "rgbcolor.h"
#include "upcast.h"

using namespace std;

namespace {

// This table is used to transform colors.
uint32_t luminance_table_r[0x100];
uint32_t luminance_table_g[0x100];
uint32_t luminance_table_b[0x100];

/**
 * Create and return an \ref SDL_Surface that contains the given sub-rectangle
 * of the given pixel region.
 */
SDL_Surface* extract_sdl_surface(Surface & surf, Rect srcrect)
{
	assert(srcrect.x >= 0);
	assert(srcrect.y >= 0);
	assert(srcrect.x + srcrect.w <= surf.width());
	assert(srcrect.y + srcrect.h <= surf.height());

	const SDL_PixelFormat & fmt = surf.format();
	SDL_Surface * dest = SDL_CreateRGBSurface
		(SDL_SWSURFACE, srcrect.w, srcrect.h,
		 fmt.BitsPerPixel, fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);

	surf.lock(Surface::Lock_Normal);
	SDL_LockSurface(dest);

	uint32_t srcpitch = surf.get_pitch();
	uint32_t rowsize = srcrect.w * fmt.BytesPerPixel;
	uint8_t * srcpix = surf.get_pixels() + srcpitch * srcrect.y + fmt.BytesPerPixel * srcrect.x;
	uint8_t * dstpix = static_cast<uint8_t *>(dest->pixels);

	for (uint32_t y = 0; y < srcrect.h; ++y) {
		memcpy(dstpix, srcpix, rowsize);
		srcpix += srcpitch;
		dstpix += dest->pitch;
	}

	SDL_UnlockSurface(dest);
	surf.unlock(Surface::Unlock_NoChange);

	return dest;
}

/**
 * Produces a resized version of the specified image
 */
Surface* resize_surface(Surface* src, uint32_t w, uint32_t h) {
	assert(w != src->width() || h != src->height());

	// First step: compute scaling factors
	Rect srcrect = Rect(Point(0, 0), src->width(), src->height());

	// Second step: get source material
	SDL_Surface * srcsdl = nullptr;
	bool free_source = true;
	if (upcast(const SDLSurface, sdlsrcsurf, src)) {
		srcsdl = sdlsrcsurf->get_sdl_surface();
		free_source = false;
	} else {
		// This is in OpenGL
		srcsdl = extract_sdl_surface(*src, srcrect);
	}

	// If we actually shrink a surface, ballpark the zoom so that the shrinking
	// effect is weakened.
	int factor = 1;
	while ((static_cast<double>(w) * factor / srcsdl->w) < 1. ||
	       (static_cast<double>(h) * factor / srcsdl->h) < 1.) {
		++factor;
	}
	if (factor > 2) {
		SDL_Surface* temp = shrinkSurface(srcsdl, factor - 1, factor - 1);
		if (free_source) {
			SDL_FreeSurface(srcsdl);
		}
		srcsdl = temp;
		free_source = true;
	}

	// Third step: perform the zoom and placement
	SDL_Surface* zoomed = zoomSurface(srcsdl, double(w) / srcsdl->w, double(h) / srcsdl->h, 1);

	if (free_source)
		SDL_FreeSurface(srcsdl);

	if (uint32_t(zoomed->w) != w || uint32_t(zoomed->h) != h) {
		const SDL_PixelFormat & fmt = *zoomed->format;
		SDL_Surface * placed = SDL_CreateRGBSurface
			(SDL_SWSURFACE, w, h,
			 fmt.BitsPerPixel, fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
		SDL_Rect srcrc =
			{0, 0,
			 static_cast<Uint16>(zoomed->w), static_cast<Uint16>(zoomed->h)
			};  // For some reason SDL_Surface and SDL_Rect express w,h in different types
		SDL_Rect dstrc = {0, 0, 0, 0};
		SDL_SetAlpha(zoomed, 0, 0);
		SDL_BlitSurface(zoomed, &srcrc, placed, &dstrc); // Updates dstrc

		Uint32 fillcolor = SDL_MapRGBA(zoomed->format, 0, 0, 0, 255);

		if (zoomed->w < placed->w) {
			dstrc.x = zoomed->w;
			dstrc.y = 0;
			dstrc.w = placed->w - zoomed->w;
			dstrc.h = zoomed->h;
			SDL_FillRect(placed, &dstrc, fillcolor);
		}
		if (zoomed->h < placed->h) {
			dstrc.x = 0;
			dstrc.y = zoomed->h;
			dstrc.w = placed->w;
			dstrc.h = placed->h - zoomed->h;
			SDL_FillRect(placed, &dstrc, fillcolor);
		}

		SDL_FreeSurface(zoomed);
		zoomed = placed;
	}

	return Surface::create(zoomed);
}

/**
 * Create a grayed version of the given surface.
 */
Surface* gray_out_surface(Surface* surf) {
	assert(surf);

	uint16_t w = surf->width();
	uint16_t h = surf->height();
	const SDL_PixelFormat & origfmt = surf->format();

	Surface* dest = Surface::create(w, h);
	const SDL_PixelFormat & destfmt = dest->format();

	surf->lock(Surface::Lock_Normal);
	dest->lock(Surface::Lock_Discard);
	for (uint32_t y = 0; y < h; ++y) {
		for (uint32_t x = 0; x < w; ++x) {
			RGBAColor color;

			color.set(origfmt, surf->get_pixel(x, y));

			//  Halve the opacity to give some difference for image that are
			//  grayscale to begin with.
			color.a >>= 1;

			color.r = color.g = color.b =
				(luminance_table_r[color.r] +
				 luminance_table_g[color.g] +
				 luminance_table_b[color.b] +
				 8388608U) //  compensate for truncation:  .5 * 2^24
				>> 24;

			dest->set_pixel(x, y, color.map(destfmt));
		}
	}
	surf->unlock(Surface::Unlock_NoChange);
	dest->unlock(Surface::Unlock_Update);

	return dest;
}

/**
 * Creates an image with changed luminosity from the given surface.
 */
Surface* change_luminosity_of_surface(Surface* surf, float factor, bool halve_alpha) {
	assert(surf);

	uint16_t w = surf->width();
	uint16_t h = surf->height();
	const SDL_PixelFormat & origfmt = surf->format();

	Surface* dest = Surface::create(w, h);
	const SDL_PixelFormat & destfmt = dest->format();

	surf->lock(Surface::Lock_Normal);
	dest->lock(Surface::Lock_Discard);
	for (uint32_t y = 0; y < h; ++y) {
		for (uint32_t x = 0; x < w; ++x) {
			RGBAColor color;

			color.set(origfmt, surf->get_pixel(x, y));

			if (halve_alpha)
				color.a >>= 1;

			color.r = color.r * factor > 255 ? 255 : color.r * factor;
			color.g = color.g * factor > 255 ? 255 : color.g * factor;
			color.b = color.b * factor > 255 ? 255 : color.b * factor;

			dest->set_pixel(x, y, color.map(destfmt));
		}
	}
	surf->unlock(Surface::Unlock_NoChange);
	dest->unlock(Surface::Unlock_Update);

	return dest;
}

// Encodes the given Image into the corresponding image for player color.
// Takes the neutral set of images and the player color mask.
Surface* make_playerclr_surface(Surface& orig_surface, Surface& pcmask_surface, const RGBColor& color) {
	Surface* new_surface = Surface::create(orig_surface.width(), orig_surface.height());

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

					plrclr.r = (color.r * intensity) >> 8;
					plrclr.g = (color.g * intensity) >> 8;
					plrclr.b = (color.b * intensity) >> 8;

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

// An Image implementation that is the transformation of another Image. Uses
// the SurfaceCache to avoid recalculating the transformation too often. No
// ownerships are taken.
class TransformedImage : public Image {
public:
	TransformedImage(const string& ghash, const Image& original, SurfaceCache* surface_cache) :
		hash_(ghash), original_(original), surface_cache_(surface_cache) {}
	virtual ~TransformedImage() {}

	// Implements Image.
	virtual uint16_t width() const override {return original_.width();}
	virtual uint16_t height() const override {return original_.height();}
	virtual const string& hash() const override {return hash_;}
	virtual Surface* surface() const override {
		Surface* surf = surface_cache_->get(hash_);
		if (surf)
			return surf;

		surf = recalculate_surface();
		surface_cache_->insert(hash_, surf, true);
		return surf;
	}

	virtual Surface* recalculate_surface() const = 0;

protected:
	const string hash_;
	const Image& original_;
	SurfaceCache* const surface_cache_;  // not owned
};

// A resized copy of an Image.
class ResizedImage : public TransformedImage {
public:
	ResizedImage
		(const string& ghash, const Image& original,
		 SurfaceCache* surface_cache, uint16_t w, uint16_t h)
		: TransformedImage(ghash, original, surface_cache), w_(w), h_(h) {
			assert(w != original.width() || h != original.height());
	}
	virtual ~ResizedImage() {}

	// Overwrites TransformedImage.
	virtual uint16_t width() const override {return w_;}
	virtual uint16_t height() const override {return h_;}

	// Implements TransformedImage.
	virtual Surface* recalculate_surface() const override {
		Surface* rv = resize_surface(original_.surface(), w_, h_);
		return rv;
	}

private:
	uint16_t w_, h_;
};

// A grayed out copy of an Image.
class GrayedOutImage : public TransformedImage {
public:
	GrayedOutImage(const string& ghash, const Image& original, SurfaceCache* surface_cache) :
		TransformedImage(ghash, original, surface_cache)
	{}
	virtual ~GrayedOutImage() {}

	// Implements TransformedImage.
	virtual Surface* recalculate_surface() const override {
		return gray_out_surface(original_.surface());
	}
};

// A copy with another luminosity and maybe half the opacity.
class ChangeLuminosityImage : public TransformedImage {
public:
	ChangeLuminosityImage
		(const string& ghash, const Image& original,
		 SurfaceCache* surface_cache, float factor, bool halve_alpha)
		: TransformedImage(ghash, original, surface_cache),
		  factor_(factor),
		  halve_alpha_(halve_alpha)
	{}
	virtual ~ChangeLuminosityImage() {}

	// Implements TransformedImage.
	virtual Surface* recalculate_surface() const override {
		return change_luminosity_of_surface(original_.surface(), factor_, halve_alpha_);
	}

private:
	float factor_;
	bool halve_alpha_;
};

// A copy with applied player colors. Also needs a mask - ownership is not
// taken.
class PlayerColoredImage : public TransformedImage {
public:
	PlayerColoredImage
		(const string& ghash, const Image& original,
		 SurfaceCache* surface_cache, const RGBColor& color, const Image& mask)
		: TransformedImage(ghash, original, surface_cache), color_(color), mask_(mask)
		{}
	virtual ~PlayerColoredImage() {}

	// Implements TransformedImage.
	virtual Surface* recalculate_surface() const override {
		return make_playerclr_surface(*original_.surface(), *mask_.surface(), color_);
	}

private:
	const RGBColor& color_;
	const Image& mask_;
};

}

namespace ImageTransformations {

void initialize() {
	// Initialize the table used to create grayed image
	for
		(uint32_t i = 0, r = 0, g = 0, b = 0;
		 i < 0x100;
		 ++i, r += 5016388U, g += 9848226U, b += 1912603U)
		{
			luminance_table_r[i] = r;
			luminance_table_g[i] = g;
			luminance_table_b[i] = b;
		}
}

const Image* resize(const Image* original, uint16_t w, uint16_t h) {
	if (original->width() == w and original->height() == h)
		return original;

	const string new_hash = (boost::format("%s:%i:%i") % original->hash() % w % h).str();
	if (g_gr->images().has(new_hash))
		return g_gr->images().get(new_hash);
	return
		g_gr->images().insert(new ResizedImage(new_hash, *original, &g_gr->surfaces(), w, h));
}

const Image* gray_out(const Image* original) {
	const string new_hash = original->hash() + ":greyed_out";
	if (g_gr->images().has(new_hash))
		return g_gr->images().get(new_hash);
	return
		g_gr->images().insert(new GrayedOutImage(new_hash, *original, &g_gr->surfaces()));
}

const Image* change_luminosity(const Image* original, float factor, bool halve_alpha) {
	const string new_hash =
		(boost::format("%s:%i:%i") % original->hash() % static_cast<int>(factor * 1000) % halve_alpha).str();
	if (g_gr->images().has(new_hash))
		return g_gr->images().get(new_hash);
	return
		g_gr->images().insert
			(new ChangeLuminosityImage(new_hash, *original, &g_gr->surfaces(), factor, halve_alpha));
}

const Image* player_colored(const RGBColor& clr, const Image* original, const Image* mask) {
	const string new_hash =
		(boost::format("%s:%02x%02x%02x") % original->hash() % static_cast<int>(clr.r) %
		 static_cast<int>(clr.g) % static_cast<int>(clr.b))
			.str();
	if (g_gr->images().has(new_hash))
		return g_gr->images().get(new_hash);
	return
		g_gr->images().insert
			(new PlayerColoredImage(new_hash, *original, &g_gr->surfaces(), clr, *mask));
}

}  // namespace ImageTransformations
