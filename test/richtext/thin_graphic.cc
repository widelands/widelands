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

#include <SDL.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include "graphic/image_cache.h"
#include "graphic/image_loader.h"
#include "graphic/text/rt_errors.h"
#include "graphic/text/sdl_helper.h"
#include "upcast.h"

#include "lodepng.h"

#include "thin_graphic.h"

using namespace std;
using namespace boost;

namespace {
class ImageLoader : public IImageLoader {
public:
	virtual ~ImageLoader() {}

	virtual IPicture* load(const string& s, bool alpha) const {
		unsigned w, h;
		unsigned char * image;

		unsigned error = lodepng_decode32_file(&image, &w, &h,
				("imgs/" + s).c_str());
		if (error)
			throw RT::BadImage
				((format("Problem loading image %s: %s\n") % s % lodepng_error_text(error)).str());

		Uint32 rmask, gmask, bmask, amask;
		/* SDL interprets each pixel as a 32-bit number, so our masks must depend
			on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = alpha ? 0x000000ff : 0;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = alpha ? 0xff000000 : 0;
#endif

		SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(image, w, h, 32, w*4, rmask, gmask, bmask, amask);
		if (!surf)
			throw RT::BadImage
				((format("Problem creating surface for image %s: %s\n") % s % SDL_GetError()).str());

		return new ThinSDLSurface(surf, true);
	}
};
}  // namespace

ThinSDLSurface::ThinSDLSurface(SDL_Surface * surf, bool free_pixels) :
		surf_(surf), free_pixels_(free_pixels)
	{}

ThinSDLSurface::~ThinSDLSurface() {
	if (surf_) {
		// TODO(sirver): Leaking left and right
		// SDL_FreeSurface(surf_);
		if (free_pixels_) {
			free(surf_->pixels);
		}
	}
	surf_ = 0;
}

void ThinSDLSurface::blit(const Point& dst, const IPicture* src, const Rect& srcrc, Composite cm)
{
	upcast(const ThinSDLSurface, sdlsurf, src);
	assert(sdlsurf);
	assert(this);
	SDL_Rect srcrect = {srcrc.x, srcrc.y, srcrc.w, srcrc.h};
	SDL_Rect dstrect = {dst.x, dst.y, 0, 0};

	bool alpha;
	uint8_t alphaval;
	if (cm == CM_Solid || cm == CM_Copy) {
		alpha = sdlsurf->surf_->flags & SDL_SRCALPHA;
		alphaval = sdlsurf->surf_->format->alpha;
		SDL_SetAlpha(sdlsurf->surf_, 0, 0);
	}

	SDL_BlitSurface(sdlsurf->surf_, &srcrect, surf_, &dstrect);

	if (cm == CM_Solid || cm == CM_Copy) {
		SDL_SetAlpha(sdlsurf->surf_, alpha?SDL_SRCALPHA:0, alphaval);
	}
}

void ThinSDLSurface::fill_rect(const Rect& rc, RGBAColor clr) {
	const uint32_t color = clr.map(*surf_->format);
	SDL_Rect r = {rc.x, rc.y, rc.w, rc.h};
	SDL_FillRect(surf_, &r, color);
}

class ThinGraphic : boost::noncopyable, virtual public IGraphic {
public:
	ThinGraphic() :
		img_loader_(new ImageLoader()),
		img_cache_(create_image_cache(img_loader_.get())) {}
	virtual ~ThinGraphic() {}

	virtual IPicture* convert_sdl_surface_to_picture(SDL_Surface* surf, bool alpha = false) {
		return new ThinSDLSurface(surf, false);
	}
	IBlitableSurface * create_surface(int32_t w, int32_t h, bool alpha = false) {
		return new ThinSDLSurface(RT::empty_sdl_surface(w,h,alpha), false);
	}

	ImageCache& imgcache() const {return *img_cache_.get();}

private:
	boost::scoped_ptr<ImageCache> img_cache_;
	boost::scoped_ptr<ImageLoader> img_loader_;
};


IGraphic * create_thin_graphic() {
	return new ThinGraphic();
}

