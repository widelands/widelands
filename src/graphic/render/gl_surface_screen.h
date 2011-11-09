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
 */

#ifndef GL_SURFACE_SCREEN_H
#define GL_SURFACE_SCREEN_H

#include <boost/scoped_array.hpp>

#include "graphic/picture_id.h"
#include "graphic/pixelaccess.h"
#include "graphic/surface.h"

/**
 * This surface represents the screen in OpenGL mode.
 */
struct GLSurfaceScreen : Surface, IPixelAccess {
	GLSurfaceScreen(uint32_t w, uint32_t h);

	/// Interface implementations
	//@{
	virtual bool valid();

	virtual uint32_t get_w();
	virtual uint32_t get_h();

	virtual void update();

	virtual const SDL_PixelFormat & format() const;
	virtual void lock(LockMode);
	virtual void unlock(UnlockMode);
	virtual uint16_t get_pitch() const;
	virtual uint8_t * get_pixels() const;
	virtual void set_pixel(uint32_t x, uint32_t y, Uint32 clr);
	virtual uint32_t get_pixel(uint32_t x, uint32_t y);
	virtual IPixelAccess & pixelaccess() {return *this;}

	virtual void clear();
	virtual void draw_rect(Rect, RGBColor);
	virtual void fill_rect(Rect, RGBAColor);
	virtual void brighten_rect(Rect, int32_t factor);

	virtual void draw_line
		(int32_t x1, int32_t y1,
		 int32_t x2, int32_t y2,
		 RGBColor);

	virtual void blit(Point, PictureID, Rect srcrc, Composite cm);
	virtual void fast_blit(PictureID);
	//@}

private:
	void swap_rows();

	/// Size of the screen
	uint32_t m_w, m_h;

	/// Pixel data while locked
	boost::scoped_array<uint8_t> m_pixels;
};

#endif // GL_SURFACE_SCREEN_H
