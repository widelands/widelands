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

#include <iostream> // TODO(sirver): remove me

#include <string>
#include <map>

#include <SDL.h>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include "graphic/pixelaccess.h"
#include "graphic/text/rt_errors.h"
#include "graphic/text/sdl_helper.h"
#include "upcast.h"

#include "lodepng.h"

#include "thin_graphic.h"

using namespace std;
using namespace boost;


#include <iostream> // TODO(sirver): Remove me

class ThinSDLSurfacePA : boost::noncopyable, public virtual IPixelAccess {
public:
	ThinSDLSurfacePA(SDL_Surface* surf) : surf_(surf) {}
	virtual ~ThinSDLSurfacePA() {}

	 // TODO(sirver): Should only be w, should be const
	virtual uint32_t get_w() {return surf_->w;}
	virtual uint32_t get_h() {return surf_->h;}
	virtual SDL_PixelFormat const & format() const {return *surf_->format;}
	virtual uint16_t get_pitch() const {return surf_->pitch;}
	virtual uint8_t * get_pixels() const {
   cout << "w: " << surf_->w << endl;
   cout << "h: " << surf_->h << endl;
   cout << "pitch: " << get_pitch() << endl;
   cout << "BitsPerPixel: " << (int)format().BitsPerPixel << endl;
   cout << "BytesPerPixel: " << (int)format().BytesPerPixel << endl;
   cout << "Rmask: " << format().Rmask << endl;
   cout << "Gmask: " << format().Gmask << endl;
   cout << "Bmask: " << format().Bmask << endl;
   cout << "Amask: " << format().Amask << endl;
   // 5   Uint8  Rloss, Gloss, Bloss, Aloss;
   // 6   Uint8  Rshift, Gshift, Bshift, Ashift;
   // 7   Uint32 Rmask, Gmask, Bmask, Amask;
   // 8   Uint32 colorkey;
   // 9   Uint8  alpha;
		return static_cast<uint8_t*>(surf_->pixels);
	}

	virtual void lock(LockMode) {SDL_LockSurface(surf_);}
	virtual void unlock(UnlockMode) {SDL_UnlockSurface(surf_);}

	// TODO(sirver): These functions are wrong
	virtual uint32_t get_pixel(uint32_t x, uint32_t y) {
		return static_cast<uint32_t*>(surf_->pixels)[y*surf_->pitch + x];
	}
	virtual void set_pixel(uint32_t x, uint32_t y, uint32_t clr) {
		static_cast<uint32_t*>(surf_->pixels)[y*surf_->pitch + x] = clr;
	}


private:
	SDL_Surface* surf_;
};

class ThinSDLSurface : public virtual IBlitableSurface {
public:
	ThinSDLSurface(SDL_Surface * surf, bool free_pixels) :
		surf_(surf), pa_(surf), free_pixels_(free_pixels)
	{}
	virtual ~ThinSDLSurface() {
		if (surf_) {
			cout << "~ThinSDLSurface: before SDL_FreeSurface" << endl;
			// TODO(sirver): Leaking left and right
			// SDL_FreeSurface(surf_);
			if (free_pixels_) {
				cout << "~ThinSDLSurface: before freeing" << endl;
				free(surf_->pixels);
			}
		}
		surf_ = 0;
		cout << "~ThinSDLSurface: all done" << endl;
	}

	virtual bool valid() {return true;}
	virtual uint32_t get_w() {return surf_->w;}
	virtual uint32_t get_h() {return surf_->h;}

	virtual IPixelAccess & pixelaccess() {return pa_;}

	void blit(Point const dst, PictureID src, Rect const srcrc, Composite cm)
	{
		upcast(ThinSDLSurface, sdlsurf, src.get());
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

	void fill_rect(Rect rc, RGBAColor clr) {
		const uint32_t color = clr.map(*surf_->format);
		SDL_Rect r = {rc.x, rc.y, rc.w, rc.h};
		SDL_FillRect(surf_, &r, color);
	}

private:
	SDL_Surface * surf_;
	ThinSDLSurfacePA pa_;
	bool free_pixels_;
};


// TODO(sirver): Rename to TinyGraphics
class ThinGraphic : boost::noncopyable, virtual public IGraphic {
public:
	virtual ~ThinGraphic();
	virtual PictureID convert_sdl_surface_to_picture(SDL_Surface *, bool alpha = false);
	virtual PictureID load_image(const std::string &, bool alpha = false);
	virtual const PictureID & get_picture(PicMod, std::string const &, bool alpha = true);
	virtual void add_picture_to_cache(PicMod, const std::string &, PictureID);
	IBlitableSurface * create_surface(int32_t w, int32_t h);

private:
	typedef std::pair<const string, PictureID> MapEntry;
	typedef map<string, PictureID> GraphicMap;
	GraphicMap m_imgcache;
};


ThinGraphic::~ThinGraphic() {
	// TODO(sirver): This leaks memory due to the shared_ptr approach
}

PictureID ThinGraphic::convert_sdl_surface_to_picture(SDL_Surface * surf, bool alpha)
{
	PictureID rv(new ThinSDLSurface(surf, false));
	// TODO(sirver): We are leaking left and right here
	// SDL_FreeSurface(surf);
	return rv;
}

// TODO(sirver): Pass a PicMod and add to cache immediately
// TODO: understand the semantic of these functions. Is load_image
// caching already. Is get_picture calling load image?
PictureID ThinGraphic::load_image(const std::string & s, bool alpha) {
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
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(image, w, h, 32, w*4, rmask, gmask, bmask, amask);
	if (!surf)
		throw RT::BadImage
			((format("Problem creating surface for image %s: %s\n") % s % SDL_GetError()).str());

	ThinSDLSurface* rv = new ThinSDLSurface(surf, true);
	return m_imgcache[s] = PictureID(rv);
}

// TODO(sirver): Why is this exposed?
void ThinGraphic::add_picture_to_cache(PicMod /* module */, const std::string & name, PictureID pic) {
	// We ignore module completely here.
	m_imgcache[name] = pic;
}

const PictureID & ThinGraphic::get_picture
	(PicMod const module, const std::string & fname, bool alpha)
{
	GraphicMap::iterator i = m_imgcache.find(fname);
	if (i != m_imgcache.end())
		return i->second;

	load_image(fname, alpha);
	return m_imgcache[fname];
}

IBlitableSurface * ThinGraphic::create_surface(int32_t w, int32_t h) {
	return new ThinSDLSurface(RT::empty_sdl_surface(w,h), false);
}


IGraphic * create_thin_graphic() {
	return new ThinGraphic();
}

