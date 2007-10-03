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
	PicMod_UI = 1,
	PicMod_Menu = 2,
	PicMod_Game = 4,
	PicMod_Font = 8,
};

struct Picture {
	Picture() : mod(0), surface(0) {u.fname=0;}
	/// 0 if unused, -1 for surfaces, PicMod_* bitmask for pictures
	int32_t mod;
	Surface* surface;

	//WTF ?!?! A union between char* (to be used as filename) and class* ?!?
	//Why ?!?! #fweber
	union {
		char * fname;
		RenderTarget * rendertarget;
	} u;
};

#endif
