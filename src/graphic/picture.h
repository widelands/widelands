/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

/***
 * Picture/PictureID
 *
 * PictureID is a reference to a picture
 * picmod is used to specify which buffer picture is loaded in.
 * warning: a picture can be loaded multiple times in multiple buffers
 * when buffer is flushed pictures will hang around till the last reference
 * is gone too
 ***/


struct RenderTarget;
struct Surface;

/// picture module flags
enum PicMod {
	INVALID     = 0,
	PicMod_UI   = 1,
	PicMod_Menu = 2,
	PicMod_Game = 3,
	PicMod_Font = 4,
	PicSurface  = 5,
	MaxModule   = 6 //MaxModule ALWAYS has to be the 'size' of picmod
};

struct Picture {
	Picture() : module(INVALID), surface(0), fname(0), rendertarget(0) {}
	~Picture();
	//void operator delete(void * p);

	//PicMod lists which 'buffer' to load the images in.
	// INVALID if unused, MaxModule not a legal module
	PicMod    module;
	Surface * surface;

	char         * fname; //  module & (PicMod_UI|PicMod_Menu|PicMod_Game)
	RenderTarget * rendertarget; //  module & (PicMod_Font | PicSurface)
};

#endif
