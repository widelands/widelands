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

#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <SDL.h>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include "thin_graphic.h"
#include "lodepng.h"
#include "rt_errors.h"

using namespace std;
using namespace boost;

class LodePngImageLoader {
private:
	typedef std::pair<const string, SDL_Surface *> MapEntry;
	map<string, SDL_Surface*> m_imgcache;

public:
	virtual ~LodePngImageLoader() {
		BOOST_FOREACH(MapEntry & s,  m_imgcache) {
			free(s.second->pixels);
			SDL_FreeSurface(s.second);
		}
	}
	SDL_Surface * load(std::string s) {
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

		SDL_Surface * sur = SDL_CreateRGBSurfaceFrom(image, w, h, 32, w*4, rmask, gmask, bmask, amask);
		if (!sur)
			throw RT::BadImage
				((format("Problem creating surface for image %s: %s\n") % s % SDL_GetError()).str());

		m_imgcache[s] = sur;
		return sur;
	}
};


// TODO(sirver): Rename to TinyGraphics
class ThinGraphic : boost::noncopyable {
public:
	virtual ~ThinGraphic() {}
	virtual PictureID convert_sdl_surface_to_picture(SDL_Surface *, bool alpha = false);
	virtual PictureID load_image(std::string const &, bool alpha = false) = 0;
	virtual const PictureID & get_picture(PicMod, std::string const &, bool alpha = true);
	virtual void add_picture_to_cache(PicMod, const std::string &, PictureID);

private:
	LodePngImageLoader m_imgl;
};


ThinGraphic::ThinGraphic() {
}

PictureID ThinGraphic::convert_sdl_surface_to_picture(SDL_Surface * surf, bool alpha)
{
	// TODO(sirver): ImplementThis
}

PictureID ThinGraphic::load_image(std::string const & fname, bool const alpha) {
	// TODO(sirver): implement this
}

const PictureID & ThinGraphic::get_picture
	(PicMod const module, const std::string & fname, bool alpha)
{
	// TODO(sirver): Implement this
}

void ThinGraphic::add_picture_to_cache(PicMod module, const std::string & name, PictureID pic) {
	// TODO(sirver): Implement this
}


IGraphic * create_thin_graphic() {
	return new ThinGraphic();
}

