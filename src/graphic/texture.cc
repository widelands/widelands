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


}  // namespace

Texture::Texture(int w, int h)
{
	init(w, h);

	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	glTexImage2D
		(GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA), m_w, m_h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, nullptr);
}

Texture::Texture(SDL_Surface * surface, bool intensity)
{
	init(surface->w, surface->h);

	// Convert image data. BGR Surface support is an extension for
	// OpenGL ES 2, which we rather not rely on. So we convert our
	// surfaces in software.
	// TODO(sirver): SDL_TTF returns all data in BGR format. If we
	// use freetype directly we might be able to avoid that.
	uint8_t bpp = surface->format->BytesPerPixel;

	if (surface->format->palette || m_w != surface->w || m_h != surface->h ||
	    (bpp != 3 && bpp != 4) || is_bgr_surface(*surface->format)) {
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
        (GL_TEXTURE_2D, 0, static_cast<GLint>(intensity ? GL_INTENSITY : GL_RGBA), m_w, m_h, 0,
		 pixels_format, GL_UNSIGNED_BYTE, surface->pixels);

	SDL_UnlockSurface(surface);
	SDL_FreeSurface(surface);
}

Texture::Texture(const GLuint texture, const Rect& subrect, int parent_w, int parent_h) {
	if (parent_w == 0 || parent_h == 0) {
		throw wexception("Created a sub Texture with zero height and width parent.");
	}

	m_w = subrect.w;
	m_h = subrect.h;

	m_texture = texture;
	m_owns_texture = false;

	m_texture_coordinates.w = static_cast<float>(m_w - 1) / parent_w;
	m_texture_coordinates.h = static_cast<float>(m_h - 1) / parent_h;
	m_texture_coordinates.x = (static_cast<float>(subrect.x) + 0.5) / parent_w;
	m_texture_coordinates.y = (static_cast<float>(subrect.y) + 0.5) / parent_h;
}

Texture::~Texture()
{
	if (m_owns_texture) {
		glDeleteTextures(1, &m_texture);
	}
}

int Texture::width() const {
	return m_w;
}

int Texture::height() const {
	return m_h;
}

int Texture::get_gl_texture() const {
	return m_texture;
}

const FloatRect& Texture::texture_coordinates() const {
	return m_texture_coordinates;
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

	m_owns_texture = true;
	m_texture_coordinates.x = 0.f;
	m_texture_coordinates.y = 0.f;
	m_texture_coordinates.w = 1.f;
	m_texture_coordinates.h = 1.f;

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	// set texture filter to use linear filtering. This looks nicer for resized
	// texture. Most textures and images are not resized so the filtering
	// makes no difference
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
}

void Texture::lock() {
	if (m_w <= 0 || m_h <= 0) {
		return;
	}

	if (m_pixels) {
		throw wexception("Called lock() on locked surface.");
	}
	if (!m_owns_texture) {
		throw wexception("A surface that does not own its pixels can not be locked..");
	}

	m_pixels.reset(new uint8_t[m_w * m_h * 4]);

	glBindTexture(GL_TEXTURE_2D, m_texture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::unlock(UnlockMode mode) {
	if (m_w <= 0 || m_h <= 0) {
		return;
	}
	assert(m_pixels);

	if (mode == Unlock_Update) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D
            (GL_TEXTURE_2D, 0, static_cast<GLint>(GL_RGBA), m_w, m_h, 0, GL_RGBA,
			 GL_UNSIGNED_BYTE,  m_pixels.get());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_pixels.reset(nullptr);
}

uint8_t * Texture::get_pixels() const
{
	return m_pixels.get();
}

uint32_t Texture::get_pixel(uint16_t x, uint16_t y) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[y * get_pitch() + 4 * x];
	return *(reinterpret_cast<uint32_t *>(data));
}

uint16_t Texture::get_pitch() const {
	return 4 * m_w;
}

const SDL_PixelFormat & Texture::format() const {
	return Gl::gl_rgba_format();
}


void Texture::set_pixel(uint16_t x, uint16_t y, uint32_t clr) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[y * get_pitch() + 4 * x];
	*(reinterpret_cast<uint32_t *>(data)) = clr;
}


void Texture::setup_gl() {
	glBindFramebuffer(GL_FRAMEBUFFER, GlFramebuffer::instance().id());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
}
