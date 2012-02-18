/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "picture.h"

#include "wexception.h"

struct NullPicture : IPicture {
	virtual bool valid() {return false;}

	virtual uint32_t get_w() {return 0;}
	virtual uint32_t get_h() {return 0;}

	virtual IPixelAccess & pixelaccess() {throw wexception("pixelaccess() attempted on null picture");}
	virtual Surface & surface() {throw wexception("surface() attempted on null picture");}
};

/**
 * \return an invalid, null picture
 *
 * \see Graphic::get_no_picture
 */
const PictureID & IPicture::null()
{
	static PictureID thenull(new NullPicture);
	return thenull;
}
