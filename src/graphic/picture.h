/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include <stdint.h>
#include <string>

#include "surfaceptr.h"

struct RenderTarget;

struct PictureImpl;

/**
 * Interface to a bitmap that can act as the source of a rendering
 * operation.
 */
struct IPicture {
	IPicture() {}
	virtual ~IPicture() {}

	virtual uint32_t get_w() = 0;
	virtual uint32_t get_h() = 0;

	// to be removed
	virtual bool valid() = 0;
	virtual PictureImpl & impl() = 0;

private:
	// forbid copying
	IPicture(const IPicture &);
	IPicture & operator= (const IPicture &);
};

struct PictureImpl : IPicture {
	PictureImpl() : rendertarget(0) {}
	~PictureImpl();

	virtual bool valid();
	virtual uint32_t get_w();
	virtual uint32_t get_h();

	PictureImpl & impl() {return *this;}

	SurfacePtr surface;

	//TODO: descriptive name of the picture?
	std::string name;
	RenderTarget * rendertarget; //  module & (PicMod_Font | PicSurface)
};

#endif
