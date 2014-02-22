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

#include "graphic/render/gl_surface.h"

/**
 * This surface represents the screen in OpenGL mode.
 */
class GLSurfaceScreen : public GLSurface {
public:
	GLSurfaceScreen(uint16_t w, uint16_t h);
	virtual ~GLSurfaceScreen() {}

	/// Interface implementations
	virtual void lock(LockMode) override;
	virtual void unlock(UnlockMode) override;
	virtual uint16_t get_pitch() const override;
	virtual const SDL_PixelFormat & format() const override;

private:
	void swap_rows();
};

#endif // GL_SURFACE_SCREEN_H
