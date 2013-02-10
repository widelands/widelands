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

#ifndef IGRAPHIC_H
#define IGRAPHIC_H

#include <string>

#include "iblitable_surface.h"
#include "picture.h"

struct SDL_Surface;
class ImageCache;

class IGraphic {
public:
	virtual ~IGraphic() {};

	virtual IPicture* convert_sdl_surface_to_picture
		(SDL_Surface*,
		 bool alpha = false) const = 0;
	virtual IBlitableSurface * create_surface(int32_t w, int32_t h, bool alpha = false) const = 0;

	virtual ImageCache& imgcache() const = 0;
};


#endif /* end of include guard: IGRAPHIC_H */

