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
 */

#ifndef GL_SURFACE_SCREEN_H
#define GL_SURFACE_SCREEN_H

#include <boost/scoped_array.hpp>

#include "gl_surface.h"

/**
 * This surface represents the screen in OpenGL mode.
 */
class GLSurfaceScreen : public GLSurface {
public:
	GLSurfaceScreen(uint32_t w, uint32_t h);

	/// Interface implementations
	virtual void lock(LockMode);
	virtual void unlock(UnlockMode);
	virtual uint16_t get_pitch() const;

private:
	virtual void setup_gl();
	void swap_rows();

	/// Pixel data while locked
	boost::scoped_array<uint8_t> m_pixels;
};

#endif // GL_SURFACE_SCREEN_H
