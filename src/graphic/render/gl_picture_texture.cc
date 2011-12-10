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

#include "gl_picture_texture.h"

#include <SDL_video.h>

#include "gl_utils.h"
#include "graphic/graphic.h"
#include "wexception.h"


/**
 * Initialize an OpenGL texture of the given dimensions.
 *
 * The initial data of the texture is undefined.
 */
GLPictureTexture::GLPictureTexture(int w, int h)
{
	init(w, h);
}

/**
 * Initialize an OpenGL texture with the contents of the given surface.
 *
 * \note Takes ownership of the given surface.
 */
GLPictureTexture::GLPictureTexture(SDL_Surface * surface)
{
	init(surface->w, surface->h);

	// Convert image data
	uint8_t bpp = surface->format->BytesPerPixel;

	if
		(surface->format->palette or (surface->format->colorkey > 0) or
		 m_tex_w != static_cast<uint32_t>(surface->w) or
		 m_tex_h != static_cast<uint32_t>(surface->h) or
		 (bpp != 3 && bpp != 4))
	{
		SDL_Surface * converted = SDL_CreateRGBSurface
			(SDL_SWSURFACE, m_tex_w, m_tex_h,
			 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		assert(converted);
		SDL_SetAlpha(converted, 0, 0);
		SDL_SetAlpha(surface, 0, 0);
		SDL_BlitSurface(surface, 0, converted, 0);
		SDL_FreeSurface(surface);
		surface = converted;
		bpp = surface->format->BytesPerPixel;
	}

	SDL_PixelFormat const & fmt = *surface->format;
	GLenum pixels_format;

	glPushAttrib(GL_PIXEL_MODE_BIT);

	if (bpp == 4) {
		if
			(fmt.Rmask == 0x000000ff and fmt.Gmask == 0x0000ff00 and
			 fmt.Bmask == 0x00ff0000)
		{
			if (fmt.Amask == 0xff000000) {
				pixels_format = GL_RGBA;
			} else {
				pixels_format = GL_RGBA;
				// Read four bytes per pixel but ignore the alpha value
				glPixelTransferi(GL_ALPHA_SCALE, 0.0f);
				glPixelTransferi(GL_ALPHA_BIAS, 1.0f);
			}
		} else if
			(fmt.Bmask == 0x000000ff and fmt.Gmask == 0x0000ff00 and
			 fmt.Rmask == 0x00ff0000)
		{
			if (fmt.Amask == 0xff000000) {
				pixels_format = GL_BGRA;
			} else {
				pixels_format = GL_BGRA;
				// Read four bytes per pixel but ignore the alpha value
				glPixelTransferi(GL_ALPHA_SCALE, 0.0f);
				glPixelTransferi(GL_ALPHA_BIAS, 1.0f);
			}
		} else
			throw wexception("OpenGL: Unknown pixel format");
	} else  if (bpp == 3) {
		if
			(fmt.Rmask == 0x000000ff and fmt.Gmask == 0x0000ff00 and
			 fmt.Bmask == 0x00ff0000)
		{
			pixels_format = GL_RGB;
		} else if
			(fmt.Bmask == 0x000000ff and fmt.Gmask == 0x0000ff00 and
			 fmt.Rmask == 0x00ff0000)
		{
			pixels_format = GL_BGR;
		} else
			throw wexception("OpenGL: Unknown pixel format");
	} else
		throw wexception("OpenGL: Unknown pixel format");

	SDL_LockSurface(surface);

	glTexImage2D
		(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_w, m_tex_h, 0,
		 pixels_format, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);

	glPopAttrib();
	handle_glerror();
}

GLPictureTexture::~GLPictureTexture()
{
	glDeleteTextures(1, &m_texture);
}

void GLPictureTexture::init(uint32_t w, uint32_t h)
{
	handle_glerror();

	m_w = w;
	m_h = h;

	if (g_gr->caps().gl.tex_power_of_two) {
		m_tex_w = next_power_of_two(w);
		m_tex_h = next_power_of_two(h);
	} else {
		m_tex_w = w;
		m_tex_h = h;
	}

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	// set texture filter to use linear filtering. This looks nicer for resized
	// texture. Most textures and images are not resized so the filtering
	// makes no difference
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	handle_glerror();
}

bool GLPictureTexture::valid()
{
	return true;
}

uint32_t GLPictureTexture::get_w()
{
	return m_w;
}

uint32_t GLPictureTexture::get_h()
{
	return m_h;
}

const SDL_PixelFormat & GLPictureTexture::format() const
{
	return gl_rgba_format();
}

void GLPictureTexture::lock(LockMode mode)
{
	assert(!m_pixels);

	m_pixels.reset(new uint8_t[m_tex_w * m_tex_h * 4]);

	if (mode == Lock_Normal) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}
}

void GLPictureTexture::unlock(UnlockMode mode)
{
	assert(m_pixels);

	if (mode == Unlock_Update) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D
			(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_w, m_tex_h, 0, GL_RGBA,
			 GL_UNSIGNED_BYTE,  m_pixels.get());
	}

	m_pixels.reset(0);
}

uint16_t GLPictureTexture::get_pitch() const
{
	return 4 * m_tex_w;
}

uint8_t * GLPictureTexture::get_pixels() const
{
	return m_pixels.get();
}

uint32_t GLPictureTexture::get_pixel(uint32_t x, uint32_t y)
{
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[(y * m_tex_w + x) * 4];
	return *(reinterpret_cast<uint32_t *>(data));
}

void GLPictureTexture::set_pixel(uint32_t x, uint32_t y, uint32_t clr)
{
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[(y * m_tex_w + x) * 4];
	*(reinterpret_cast<uint32_t *>(data)) = clr;
}
