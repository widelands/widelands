/*
 * Copyright (C) 2002-2004, 2006, 2008 by the Widelands Development Team
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

#ifndef COLORMAP_H
#define COLORMAP_H

#include <SDL_video.h>

/**
 * Colormap contains a palette and lookup table for use with ground textures.
*/
class Colormap {
	SDL_Color palette[256];

	/// maps 8 bit color and brightness value to the shaded color.
	/// \note Brightness is currently 8 bits. Restricting brightness to 64 or
	/// less shades would greatly reduce the size of this table, and thus
	/// improve memory cache impact inside the renderer.
	void * colormap;

public:
	Colormap (const SDL_Color &, const SDL_PixelFormat & fmt);
	~Colormap ();

	SDL_Color* get_palette() {return palette;}

	void* get_colormap () const {return colormap;}
};

#endif
