/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef PICTURE_H
#define PICTURE_H

#include "rendertarget.h"
#include "surface.h"

/// picture module flags
enum {
	PicMod_UI   = 0x01,
	PicMod_Menu = 0x02,
	PicMod_Game = 0x04,
	PicMod_Font = 0x40,
	PicSurface  = 0x80
};

struct Picture {
	Picture() : module(0), surface(0) {u.fname = 0;}
	/// 0 if unused, PicSurface for surfaces, PicMod_* bitmask for pictures
	uint8_t   module;
	Surface * surface;

	//WTF ?!?! A union between char* (to be used as filename) and class* ?!?
	//Why ?!?! #fweber
	union {
		char         * fname; //  module & (PicMod_UI|PicMod_Menu|PicMod_Game)
		RenderTarget * rendertarget; //  module & (PicMod_Font | PicSurface)
	} u;
};

#endif
