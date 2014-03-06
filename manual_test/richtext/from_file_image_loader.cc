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

#include "/Users/sirver/Desktop/Programming/cpp/widelands/bzr_repo/test/richtext/from_file_image_loader.h"

#include <SDL.h>
#include <boost/format.hpp>

#include "graphic/render/sdl_surface.h"
#include "graphic/text/rt_errors.h"
#include "/Users/sirver/Desktop/Programming/cpp/widelands/bzr_repo/test/richtext/lodepng.h"

using namespace boost;
using namespace std;

class FromFileImageLoader : public IImageLoader {
public:
	virtual ~FromFileImageLoader() {}

	// Implements IImageLoader.
	virtual Surface* load(const string& s) const {
		unsigned w, h;
		unsigned char * image;

		unsigned error = lodepng_decode32_file
			(&image, &w, &h, ("imgs/" + s).c_str());
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

		SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(image, w, h, 32, w * 4, rmask, gmask, bmask, amask);
		if (!surf)
			throw RT::BadImage
				((format("Problem creating surface for image %s: %s\n") % s % SDL_GetError()).str());

		return new SDLSurface(surf);
	}
};

IImageLoader* create_from_file_image_loader() {
	return new FromFileImageLoader();
}
