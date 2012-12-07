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

#include "log.h"

#include <SDL_video.h>

#include "gl_utils.h"
#include "graphic/graphic.h"
#include "upcast.h"
#include "wexception.h"

#include "gl_surface_texture.h"


/**
 * Initialize an OpenGL texture of the given dimensions.
 *
 * The initial data of the texture is undefined.
 */
GLSurfaceTexture::GLSurfaceTexture(int w, int h)
{
	init(w, h);
}

/**
 * Initialize an OpenGL texture with the contents of the given surface.
 *
 * \note Takes ownership of the given surface.
 */
GLSurfaceTexture::GLSurfaceTexture(SDL_Surface * surface)
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

GLSurfaceTexture::~GLSurfaceTexture()
{
	glDeleteTextures(1, &m_texture);
}

void GLSurfaceTexture::init(uint32_t w, uint32_t h)
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
	// TODO(sirver): Was liner
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// TODO(sirver): this might leak memory
		glTexImage2D
			(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_w, m_tex_h, 0, GL_RGBA,
			 GL_UNSIGNED_BYTE, 0);

	handle_glerror();
}

uint32_t GLSurfaceTexture::get_w() const {
	return m_w;
}

uint32_t GLSurfaceTexture::get_h() const {
	return m_h;
}

const SDL_PixelFormat & GLSurfaceTexture::format() const {
	return gl_rgba_format();
}

void GLSurfaceTexture::lock(LockMode mode) {
	assert(!m_pixels);

	m_pixels.reset(new uint8_t[m_tex_w * m_tex_h * 4]);

	if (mode == Lock_Normal) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}
}

void GLSurfaceTexture::unlock(UnlockMode mode) {
	assert(m_pixels);

	if (mode == Unlock_Update) {
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D
			(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_w, m_tex_h, 0, GL_RGBA,
			 GL_UNSIGNED_BYTE,  m_pixels.get());
	}

	m_pixels.reset(0);
}

uint16_t GLSurfaceTexture::get_pitch() const {
	return 4 * m_tex_w;
}

uint8_t * GLSurfaceTexture::get_pixels() const {
	return m_pixels.get();
}

// TODO(sirver): should be const
uint32_t GLSurfaceTexture::get_pixel(uint32_t x, uint32_t y) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[(y * m_tex_w + x) * 4];
	return *(reinterpret_cast<uint32_t *>(data));
}

void GLSurfaceTexture::set_pixel(uint32_t x, uint32_t y, uint32_t clr) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[(y * m_tex_w + x) * 4];
	*(reinterpret_cast<uint32_t *>(data)) = clr;
}

void GLSurfaceTexture::blit(const Point& dst, const IPicture* src,
		const Rect& srcrc, Composite cm) {
	upcast(const GLSurfaceTexture, const_oglsrc, src);
	assert(const_oglsrc);
	GLSurfaceTexture* oglsrc = const_cast<GLSurfaceTexture*>(const_oglsrc);
	assert(g_opengl);

	// TODO(sirver): when used like this, it should be a scoped_ptr. Maybe this can be
	// used globally though.
	GLuint id;
	glGenFramebuffers(1, &id);

	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		m_texture, // TODO(sirver): set to 0 to detach again
		0);

	// TODO(sirver): one to one copy from screen
	/* Set a texture scaling factor. Normaly texture coordiantes
	* (see glBegin()...glEnd() Block below) are given in the range 0-1
	* to avoid the calculation (and let opengl do it) the texture
	* space is modified. glMatrixMode select which matrixconst  to manipulate
	* (the texture transformation matrix in this case). glLoadIdentity()
	* resets the (selected) matrix to the identity matrix. And finally
	* glScalef() calculates the texture matrix.
	*/
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef
		(1.0f / static_cast<GLfloat>(oglsrc->get_tex_w()),
		 1.0f / static_cast<GLfloat>(oglsrc->get_tex_h()), 1);

	// Enable Alpha blending
	if (cm == CM_Normal) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}

	glViewport(0, 0, get_tex_w(), get_tex_h());

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, oglsrc->get_gl_texture());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, get_tex_w(), 0, get_tex_h(), -1, 1);

	glBegin(GL_QUADS); {
		//  set color white, otherwise textures get mixed with color
		glColor3f(1.0, 1.0, 1.0);
		//  top-left
		glTexCoord2i(srcrc.x,           srcrc.y);
		glVertex2i  (dst.x,             dst.y);
		//  top-right
		glTexCoord2i(srcrc.x + srcrc.w, srcrc.y);
		glVertex2i  (dst.x + srcrc.w,   dst.y);
		//  bottom-right
		glTexCoord2i(srcrc.x + srcrc.w, srcrc.y + srcrc.h);
		glVertex2i  (dst.x + srcrc.w,   dst.y + srcrc.h);
		//  bottom-left
		glTexCoord2i(srcrc.x,           srcrc.y + srcrc.h);
		glVertex2i  (dst.x,             dst.y + srcrc.h);
	} glEnd();
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	// glBindTexture(GL_TEXTURE_2D, m_texture);
	// glGenerateMipmap(GL_TEXTURE_2D);
	// glBindTexture(GL_TEXTURE_2D, 0);

	// TODO(sirver): this might be expensive
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteFramebuffers(1, &id);

	// TODO(sirver): ugly!!
		glViewport(0, 0, 1680, 1000);

}
void GLSurfaceTexture::fill_rect(const Rect&, RGBAColor) {
	// TODO(sirver): implement me
}
// TODO(sirver): the next four methods seem to be unused. remove them and refactor
void GLSurfaceTexture::draw_rect(const Rect&, RGBColor) {
	// TODO(sirver): when gl supports offscreen rendering, caching is also done for OpenGL
	// TODO(sirver): implement me
}
void GLSurfaceTexture::draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
		const RGBColor& color, uint8_t width) {
	assert(0); // Never here!
}
void GLSurfaceTexture::brighten_rect(const Rect&, int32_t factor) {
	// TODO(sirver): when gl supports offscreen rendering, caching is also done for OpenGL
	// TODO(sirver): implement me
}

