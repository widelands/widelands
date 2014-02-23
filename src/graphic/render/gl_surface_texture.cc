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

#include "graphic/render/gl_surface_texture.h"

#include <cassert>

#include "gl_surface.h"  // for glew.h
#include "graphic/graphic.h"
#include "graphic/render/gl_utils.h"
#include "wexception.h"

GLuint GLSurfaceTexture::gl_framebuffer_id_;
bool use_arb_;

/**
 * Initial global resources needed for fast offscreen rendering.
 */
void GLSurfaceTexture::Initialize(bool use_arb) {
	use_arb_ = use_arb;

	// Generate the framebuffer for Offscreen rendering.
	if (use_arb) {
		glGenFramebuffers(1, &gl_framebuffer_id_);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	} else {
		glGenFramebuffersEXT(1, &gl_framebuffer_id_);
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
	}
}

/**
 * Free global resources.
 */
void GLSurfaceTexture::Cleanup() {
	if (use_arb_)
		glDeleteFramebuffers(1, &gl_framebuffer_id_);
	else
		glDeleteFramebuffersEXT(1, &gl_framebuffer_id_);
}

/**
 * Initialize an OpenGL texture of the given dimensions.
 *
 * The initial data of the texture is undefined.
 */
GLSurfaceTexture::GLSurfaceTexture(int w, int h)
{
	init(w, h);

	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	glTexImage2D
		(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_w, m_tex_h, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, nullptr);
}

/**
 * Initialize an OpenGL texture with the contents of the given surface.
 *
 * \note Takes ownership of the given surface.
 */
GLSurfaceTexture::GLSurfaceTexture(SDL_Surface * surface, bool intensity)
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
		SDL_BlitSurface(surface, nullptr, converted, nullptr);
		SDL_FreeSurface(surface);
		surface = converted;
		bpp = surface->format->BytesPerPixel;
	}

	const SDL_PixelFormat & fmt = *surface->format;
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
		(GL_TEXTURE_2D, 0, intensity ? GL_INTENSITY : GL_RGBA, m_tex_w, m_tex_h, 0,
		 pixels_format, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);

	glPopAttrib();
	handle_glerror();
}

GLSurfaceTexture::~GLSurfaceTexture()
{
	glDeleteTextures(1, &m_texture);
}

void GLSurfaceTexture::init(uint16_t w, uint16_t h)
{
	handle_glerror();
	m_w = w;
	m_h = h;
	if (m_w <= 0 || m_h <= 0) {
		return;
	}

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

const SDL_PixelFormat & GLSurfaceTexture::format() const {
	return gl_rgba_format();
}

void GLSurfaceTexture::lock(LockMode mode) {
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	assert(!m_pixels);

	m_pixels.reset(new uint8_t[m_tex_w * m_tex_h * 4]);

	if (mode == Lock_Normal) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}
}

void GLSurfaceTexture::unlock(UnlockMode mode) {
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	assert(m_pixels);

	if (mode == Unlock_Update) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D
			(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_w, m_tex_h, 0, GL_RGBA,
			 GL_UNSIGNED_BYTE,  m_pixels.get());
	}

	m_pixels.reset(nullptr);
}

uint16_t GLSurfaceTexture::get_pitch() const {
	return 4 * m_tex_w;
}

void GLSurfaceTexture::draw_rect(const Rect& rc, const RGBColor clr)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	setup_gl();
	GLSurface::draw_rect(rc, clr);
	reset_gl();
}


/**
 * Draws a filled rectangle
 */
void GLSurfaceTexture::fill_rect(const Rect& rc, const RGBAColor clr)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	setup_gl();
	GLSurface::fill_rect(rc, clr);
	reset_gl();
}

/**
 * Change the brightness of the given rectangle
 */
void GLSurfaceTexture::brighten_rect(const Rect& rc, const int32_t factor)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	setup_gl();
	GLSurface::brighten_rect(rc, factor);
	reset_gl();
}

void GLSurfaceTexture::draw_line
	(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor& color, uint8_t gwidth)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	setup_gl();
	GLSurface::draw_line(x1, y1, x2, y2, color, gwidth);
	reset_gl();
}

void GLSurfaceTexture::blit
	(const Point& dst, const Surface* src, const Rect& srcrc, Composite cm)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	setup_gl();
	GLSurface::blit(dst, src, srcrc, cm);
	reset_gl();
}

void GLSurfaceTexture::setup_gl() {
	if (use_arb_) {
		glBindFramebuffer(GL_FRAMEBUFFER, gl_framebuffer_id_);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	} else {
		glBindFramebufferEXT(GL_FRAMEBUFFER, gl_framebuffer_id_);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	}

	// Note: we do not want to reverse y for textures, we only want this for
	// the screen.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, get_tex_w(), 0, get_tex_h(), -1, 1);

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, get_tex_w(), get_tex_h());
}

void GLSurfaceTexture::reset_gl() {
	if (use_arb_)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	else
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();
}
