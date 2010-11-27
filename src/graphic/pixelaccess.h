/*
 * Copyright 2010 by the Widelands Development Team
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

#ifndef PIXELACCESS_H
#define PIXELACCESS_H

#include <stdint.h>

/**
 * Interface that provides direct pixel access to a picture or surface.
 *
 * \note Direct pixel access should be used only carefully, since it can be
 * extremely slow.
 */
struct IPixelAccess {
	IPixelAccess() {}
	virtual ~IPixelAccess() {}

	/// Get width and height
	//@{
	virtual uint32_t get_w() = 0;
	virtual uint32_t get_h() = 0;
	//@}

	/// This returns the pixel format for direct pixel access.
	virtual SDL_PixelFormat const & format() const = 0;

	/**
	 * Lock/Unlock pairs must guard any of the direct pixel access using the
	 * functions below.
	 *
	 * \note Lock/Unlock pairs cannot be nested.
	 */
	//@{
	virtual void lock() = 0;
	virtual void unlock() = 0;
	//@}

	//@{
	virtual uint32_t get_pixel(uint32_t x, uint32_t y) = 0;
	virtual void set_pixel(uint32_t x, uint32_t y, Uint32 clr) = 0;
	//@}

	/**
	 * \return Pitch of the raw pixel data, i.e. the number of bytes
	 * contained in each image row. This can be strictly larger than
	 * bytes per pixel times the width.
	 */
	virtual uint16_t get_pitch() const = 0;

	/**
	 * \return Pointer to the raw pixel data.
	 *
	 * \warning May only be called inside lock/unlock pairs.
	 */
	virtual uint8_t * get_pixels() const = 0;

private:
	IPixelAccess & operator= (const IPixelAccess &);
};

#endif // PIXELSURFACE_H
