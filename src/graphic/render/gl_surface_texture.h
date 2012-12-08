/*
 * Copyright 2010-2011 by the Widelands Development Team
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

#ifndef GL_SURFACE_TEXTURE_H
#define GL_SURFACE_TEXTURE_H

#include "gl_surface.h"

struct SDL_Surface;

class GLSurfaceTexture : public GLSurface {
public:
	// Call this once before using any instance of this class and Cleanup once
	// before the program exits.
	static void Initialize();
	static void Cleanup();

	GLSurfaceTexture(SDL_Surface * surface);
	GLSurfaceTexture(int w, int h);
	~GLSurfaceTexture();

	/// Interface implementation
	//@{
	virtual void lock(LockMode);
	virtual void unlock(UnlockMode);
	virtual uint16_t get_pitch() const;

	GLuint get_gl_texture() const {return m_texture;}
	uint32_t get_tex_w() const {return m_tex_w;}
	uint32_t get_tex_h() const {return m_tex_h;}

private:
	void init(uint32_t w, uint32_t h);

private:
	virtual void setup_gl();
	virtual void reset_gl();

	static GLuint gl_framebuffer_id_;
	GLuint m_texture;

	/// Keep the size of the opengl texture. This is necessary because some
	/// systems support only a power of two for texture sizes.
	uint32_t m_tex_w, m_tex_h;

};

#endif //GL_SURFACE_TEXTURE_H
