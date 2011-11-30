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

#ifndef GL_PICTURE_TEXTURE_H
#define GL_PICTURE_TEXTURE_H

#include <boost/scoped_array.hpp>

#define NO_SDL_GLEXT
#include <GL/glew.h>
#include <SDL_opengl.h>

#include "graphic/picture.h"
#include "graphic/pixelaccess.h"

struct SDL_Surface;

struct GLPictureTexture : IPicture, IPixelAccess {
	GLPictureTexture(SDL_Surface * surface);
	GLPictureTexture(int w, int h);
	~GLPictureTexture();

	/// Interface implementation
	//@{
	virtual bool valid();

	virtual uint32_t get_w();
	virtual uint32_t get_h();

	virtual const SDL_PixelFormat & format() const;
	virtual void lock(LockMode);
	virtual void unlock(UnlockMode);
	virtual uint16_t get_pitch() const;
	virtual uint8_t * get_pixels() const;
	virtual void set_pixel(uint32_t x, uint32_t y, uint32_t clr);
	virtual uint32_t get_pixel(uint32_t x, uint32_t y);

	virtual IPixelAccess & pixelaccess() {return *this;}
	//@}

	GLuint get_gl_texture() const {return m_texture;}
	uint32_t get_tex_w() const {return m_tex_w;}
	uint32_t get_tex_h() const {return m_tex_h;}

private:
	void init(uint32_t w, uint32_t h);

private:
	GLuint m_texture;

	/// Logical width and height of the surface
	uint32_t m_w, m_h;

	/// Keep the size of the opengl texture. This is necessary because some
	/// systems support only a power of two for texture sizes.
	uint32_t m_tex_w, m_tex_h;

	/// Pixel data, while the texture is locked
	boost::scoped_array<uint8_t> m_pixels;
};

#endif //GL_PICTURE_TEXTURE_H
