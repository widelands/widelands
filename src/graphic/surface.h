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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef SURFACE_H
#define SURFACE_H

#include <boost/noncopyable.hpp>

#include "graphic/compositemode.h"
#include "rect.h"
#include "rgbcolor.h"

/**
 * Interface to a basic surfaces that can be used as destination for blitting and drawing.
 * It also allows low level pixel access.
 */
class Surface : boost::noncopyable {
public:
	// Create a new surface from an SDL_Surface. Ownership is taken.
	static Surface* create(SDL_Surface*);

	// Create a new empty (that is randomly filled) Surface with the given
	// dimensions.
	static Surface* create(uint16_t w, uint16_t h);

	virtual ~Surface() {}

	/// Dimensions.
	virtual uint16_t width() const = 0;
	virtual uint16_t height() const = 0;

	/// This draws a part of another surface to this surface
	virtual void blit(const Point&, const Surface*, const Rect& srcrc, Composite cm = CM_Normal) = 0;

	/// Draws a filled rect to the surface.
	virtual void fill_rect(const Rect&, RGBAColor) = 0;

	/// Draws a rect (frame only) to the surface.
	virtual void draw_rect(const Rect&, RGBColor) = 0;

	/// draw a line to the surface
	virtual void draw_line
		(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor& color, uint8_t width = 1) = 0;

	/// makes a rectangle on the surface brighter (or darker).
	/// @note this is slow in SDL mode. Use with care
	virtual void brighten_rect(const Rect&, int32_t factor) = 0;

	/// The functions below are for direct pixel access. This should be used
	/// only very sparingly as / it is potentially expensive (especially for
	/// OpenGL). At the moment, only the code inside graphic / is actually using
	/// this.
	enum LockMode {
		/**
		 * Normal mode preserves pre-existing pixel data so that it can
		 * be read or modified.
		 */
		Lock_Normal = 0,

		/**
		 * Discard mode discards pre-existing pixel data. All pixels
		 * will be undefined unless they are re-written.
		 */
		Lock_Discard
	};

	enum UnlockMode {
		/**
		 * Update mode will ensure that any changes in the pixel data
		 * will appear in subsequent operations.
		 */
		Unlock_Update = 0,

		/**
		 * NoChange mode indicates that the caller changed no pixel data.
		 *
		 * \note If the caller did change pixel data but specifies NoChange
		 * mode, the results are undefined.
		 */
		Unlock_NoChange
	};

	/// This returns the pixel format for direct pixel access.
	virtual const SDL_PixelFormat & format() const = 0;

	/**
	 * Lock/Unlock pairs must guard any of the direct pixel access using the
	 * functions below.
	 *
	 * \note Lock/Unlock pairs cannot be nested.
	 */
	//@{
	virtual void lock(LockMode) = 0;
	virtual void unlock(UnlockMode) = 0;
	//@}

	//@{
	virtual uint32_t get_pixel(uint16_t x, uint16_t y) = 0;
	virtual void set_pixel(uint16_t x, uint16_t y, uint32_t clr) = 0;
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
};

#endif
