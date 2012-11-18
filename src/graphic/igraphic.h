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

#ifndef IGRAPHIC_H
#define IGRAPHIC_H

#include <string>

#include "graphic/iblitable_surface.h"
#include "graphic/picture.h"

struct SDL_Surface;

/**
 * Picture caches (modules).
 *
 * \ref Graphic maintains a cache of \ref IPicture to avoid continuous re-loading of
 * pictures that may not be referenced all the time (e.g. UI elements).
 *
 * This cache is separated into different modules, and can be flushed per-module.
 */
enum PicMod {
	PicMod_UI = 0,
	PicMod_Menu,
	PicMod_Game,

	// Must be last
	PicMod_Last
};


class IGraphic {
public:
	virtual ~IGraphic() { };
	virtual IPicture* convert_sdl_surface_to_picture(SDL_Surface*, bool alpha = false) = 0;
	virtual IPicture* load_image(std::string const &, bool alpha = false) = 0;
	virtual const IPicture* get_picture(PicMod, std::string const &, bool alpha = true) = 0;
	virtual void add_picture_to_cache(PicMod, const std::string &, IPicture*) = 0; // TODO(sirver): Really needed?
	virtual IBlitableSurface * create_surface(int32_t w, int32_t h) = 0;
};


#endif /* end of include guard: IGRAPHIC_H */

