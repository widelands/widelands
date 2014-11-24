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

#include "graphic/texture.h"

#include <cassert>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/utils.h"
#include "graphic/graphic.h"
#include "graphic/sdl_utils.h"
#include "graphic/surface.h"

namespace  {

class GlFramebuffer {
public:
	static GlFramebuffer& instance() {
		static GlFramebuffer gl_framebuffer;
		return gl_framebuffer;
	}

	~GlFramebuffer() {
		glDeleteFramebuffers(1, &gl_framebuffer_id_);
	}

	GLuint id() const {
		return gl_framebuffer_id_;
	}

private:
	GlFramebuffer() {
		// Generate the framebuffer for Offscreen rendering.
		glGenFramebuffers(1, &gl_framebuffer_id_);
	}

	GLuint gl_framebuffer_id_;

	DISALLOW_COPY_AND_ASSIGN(GlFramebuffer);
};

bool is_bgr_surface(const SDL_PixelFormat& fmt) {
	return (fmt.Bmask == 0x000000ff && fmt.Gmask == 0x0000ff00 && fmt.Rmask == 0x00ff0000);
}

inline void setup_gl(const GLuint texture) {
	glBindFramebuffer(GL_FRAMEBUFFER, GlFramebuffer::instance().id());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
}

inline void reset_gl() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace

/**
 * Initialize an OpenGL texture of the given dimensions.
 *
 * The initial data of the texture is undefined.
 */
Texture::Texture(int w, int h)
{
	init(w, h);

	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	glTexImage2D
		(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, nullptr);
}

/**
 * Initialize an OpenGL texture with the contents of the given surface.
 *
 * \note Takes ownership of the given surface.
 */
Texture::Texture(SDL_Surface * surface, bool intensity)
{
	init(surface->w, surface->h);

	// Convert image data. BGR Surface support is an extension for
	// OpenGL ES 2, which we rather not rely on. So we convert our
	// surfaces in software.
	// TODO(sirver): SDL_TTF returns all data in BGR format. If we
	// use freetype directly we might be able to avoid that.
	uint8_t bpp = surface->format->BytesPerPixel;

	if (surface->format->palette || m_w != static_cast<uint32_t>(surface->w) ||
	    m_h != static_cast<uint32_t>(surface->h) || (bpp != 3 && bpp != 4) ||
	    is_bgr_surface(*surface->format)) {
		SDL_Surface* converted = empty_sdl_surface(m_w, m_h);
		assert(converted);
		SDL_SetSurfaceAlphaMod(converted,  SDL_ALPHA_OPAQUE);
		SDL_SetSurfaceBlendMode(converted, SDL_BLENDMODE_NONE);
		SDL_SetSurfaceAlphaMod(surface,  SDL_ALPHA_OPAQUE);
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
		SDL_BlitSurface(surface, nullptr, converted, nullptr);
		SDL_FreeSurface(surface);
		surface = converted;
		bpp = surface->format->BytesPerPixel;
	}

	const GLenum pixels_format = bpp == 4 ? GL_RGBA : GL_RGB;

	SDL_LockSurface(surface);

	glTexImage2D
		(GL_TEXTURE_2D, 0, intensity ? GL_INTENSITY : GL_RGBA, m_w, m_h, 0,
		 pixels_format, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_texture);
}

void Texture::pixel_to_gl(float* x, float* y) const {
	*x = (*x / m_w) * 2. - 1.;
	*y = (*y / m_h) * 2. - 1.;
}

void Texture::init(uint16_t w, uint16_t h)
{
	m_w = w;
	m_h = h;
	if (m_w <= 0 || m_h <= 0) {
		return;
	}

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	// set texture filter to use linear filtering. This looks nicer for resized
	// texture. Most textures and images are not resized so the filtering
	// makes no difference
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::lock(LockMode mode) {
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	assert(!m_pixels);

	m_pixels.reset(new uint8_t[m_w * m_h * 4]);

	if (mode == Lock_Normal) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}
}

void Texture::unlock(UnlockMode mode) {
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	assert(m_pixels);

	if (mode == Unlock_Update) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D
			(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA,
			 GL_UNSIGNED_BYTE,  m_pixels.get());
	}

	m_pixels.reset(nullptr);
}

void Texture::draw_rect(const Rect& rectangle, const RGBColor& clr)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	setup_gl(m_texture);
	Surface::draw_rect(rectangle, clr);
	reset_gl();
}


/**
 * Draws a filled rectangle
 */
void Texture::fill_rect(const Rect& rectangle, const RGBAColor& clr)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}

	setup_gl(m_texture);
	Surface::fill_rect(rectangle, clr);
	reset_gl();
}

/**
 * Change the brightness of the given rectangle
 */
void Texture::brighten_rect(const Rect& rectangle, const int32_t factor)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}

	setup_gl(m_texture);
	Surface::brighten_rect(rectangle, factor);
	reset_gl();
}

void Texture::draw_line
	(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor& color, uint8_t gwidth)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}

	setup_gl(m_texture);
	Surface::draw_line(x1, y1, x2, y2, color, gwidth);
	reset_gl();
}

void Texture::blit
	(const Point& dst, const Texture* src, const Rect& srcrc, Composite cm)
{
	if (m_w <= 0 || m_h <= 0) {
		return;
	}

	setup_gl(m_texture);
	Surface::blit(dst, src, srcrc, cm);
	reset_gl();
}
