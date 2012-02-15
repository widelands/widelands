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

#include "gl_surface_screen.h"

#include <cmath>

#include "gl_picture_texture.h"
#include "gl_utils.h"
#include "graphic/graphic.h"
#include "upcast.h"

GLSurfaceScreen::GLSurfaceScreen(uint32_t w, uint32_t h)
{
	m_w = w;
	m_h = h;
}

bool GLSurfaceScreen::valid()
{
	return true;
}

uint32_t GLSurfaceScreen::get_w()
{
	return m_w;
}

uint32_t GLSurfaceScreen::get_h()
{
	return m_h;
}

void GLSurfaceScreen::update()
{
	assert(g_opengl);
	SDL_GL_SwapBuffers();
}

const SDL_PixelFormat & GLSurfaceScreen::format() const
{
	return gl_rgba_format();
}

/**
 * Swap order of rows in m_pixels, to compensate for the upside-down nature of the
 * OpenGL coordinate system.
 */
void GLSurfaceScreen::swap_rows()
{
	uint8_t * begin_row = m_pixels.get();
	uint8_t * end_row = m_pixels.get() + (m_w * (m_h - 1) * 4);

	while (begin_row < end_row) {
		for (uint32_t x = 0; x < m_w * 4; ++x)
			std::swap(begin_row[x], end_row[x]);

		begin_row += m_w * 4;
		end_row -= m_w * 4;
	}
}

void GLSurfaceScreen::lock(IPixelAccess::LockMode mode)
{
	assert(!m_pixels);

	m_pixels.reset(new uint8_t[m_w * m_h * 4]);

	if (mode == Lock_Normal) {
		glReadPixels(0, 0, m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
		swap_rows();
	}
}

void GLSurfaceScreen::unlock(IPixelAccess::UnlockMode mode)
{
	assert(m_pixels);

	if (mode == Unlock_Update) {
		swap_rows();
		glDrawPixels(m_w, m_h, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.get());
	}

	m_pixels.reset(0);
}

uint16_t GLSurfaceScreen::get_pitch() const
{
	return 4 * m_w;
}

uint8_t * GLSurfaceScreen::get_pixels() const
{
	return m_pixels.get();
}

uint32_t GLSurfaceScreen::get_pixel(uint32_t x, uint32_t y)
{
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[4 * (y * m_w + x)];
	return *(reinterpret_cast<uint32_t *>(data));
}

void GLSurfaceScreen::set_pixel(uint32_t x, uint32_t y, Uint32 clr)
{
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[4 * (y * m_w + x)];
	*(reinterpret_cast<uint32_t *>(data)) = clr;
}

/**
 * Draws the outline of a rectangle
 */
void GLSurfaceScreen::draw_rect(const Rect rc, const RGBColor clr)
{
	assert(g_opengl);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1);

	glBegin(GL_LINE_LOOP); {
		glColor3ub(clr.r(), clr.g(), clr.b());
		glVertex2f(rc.x + 0.5f,        rc.y + 0.5f);
		glVertex2f(rc.x + rc.w - 0.5f, rc.y + 0.5f);
		glVertex2f(rc.x + rc.w - 0.5f, rc.y + rc.h - 0.5f);
		glVertex2f(rc.x + 0.5f,        rc.y + rc.h - 0.5f);
	} glEnd();
	glEnable(GL_TEXTURE_2D);
}


/**
 * Draws a filled rectangle
 */
void GLSurfaceScreen::fill_rect(const Rect rc, const RGBAColor clr) {
	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 0);
	assert(rc.h >= 0);
	assert(g_opengl);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glBegin(GL_QUADS); {
		glColor4ub(clr.r, clr.g, clr.b, clr.a);
		glVertex2f(rc.x,        rc.y);
		glVertex2f(rc.x + rc.w, rc.y);
		glVertex2f(rc.x + rc.w, rc.y + rc.h);
		glVertex2f(rc.x,        rc.y + rc.h);
	} glEnd();
	glEnable(GL_TEXTURE_2D);
}

/**
 * Change the brightness of the given rectangle
 */
void GLSurfaceScreen::brighten_rect(const Rect rc, const int32_t factor)
{
	if (!factor)
		return;

	assert(rc.x >= 0);
	assert(rc.y >= 0);
	assert(rc.w >= 1);
	assert(rc.h >= 1);
	assert(g_opengl);

	if (factor < 0) {
		if (!g_gr->caps().gl.blendequation)
			return;

		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
	}

	/* glBlendFunc is a very nice feature of opengl. You can specify how the
	* color is calculated.
	*
	* glBlendFunc(GL_ONE, GL_ONE) means the following:
	* Rnew = Rdest + Rsrc
	* Gnew = Gdest + Gsrc
	* Bnew = Bdest + Bsrc
	* Anew = Adest + Asrc
	* where Xnew is the new calculated color for destination, Xdest is the old
	* color of the destination and Xsrc is the color of the source.
	*/
	glEnable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_ONE, GL_ONE);

	// And now simply draw a rect with facor as the color
	// (this is the source color) over the region
	glBegin(GL_QUADS); {
		glColor3f
			((fabsf(factor) / 256.0f),
			 (fabsf(factor) / 256.0f),
			 (fabsf(factor) / 256.0f));
		glVertex2f(rc.x,        rc.y);
		glVertex2f(rc.x + rc.w, rc.y);
		glVertex2f(rc.x + rc.w, rc.y + rc.h);
		glVertex2f(rc.x,        rc.y + rc.h);
	} glEnd();

	if (factor < 0)
		glBlendEquation(GL_FUNC_ADD);
}

void GLSurfaceScreen::draw_line
		(int32_t x1,
		 int32_t y1,
		 int32_t x2,
		 int32_t y2,
		 RGBColor color,
		 uint8_t width)
{
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(width);
	glBegin(GL_LINES); {
		glColor3ub(color.r(), color.g(), color.b());
		glVertex2f(x1 + 0.5f, y1 + 0.5f);
		glVertex2f(x2 + 0.5f, y2 + 0.5f);
	} glEnd();
}

/**
 * Clear the entire screen to black
 */
void GLSurfaceScreen::clear()
{
	assert(g_opengl);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}


void GLSurfaceScreen::blit
	(Point const dst, PictureID const src, Rect const srcrc, Composite cm)
{
	upcast(GLPictureTexture, oglsrc, src.get());
	assert(oglsrc);
	assert(g_opengl);

	/* Set a texture scaling factor. Normaly texture coordiantes
	* (see glBegin()...glEnd() Block below) are given in the range 0-1
	* to avoid the calculation (and let opengl do it) the texture
	* space is modified. glMatrixMode select which matrix to manipulate
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

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, oglsrc->get_gl_texture());

	glBegin(GL_QUADS); {
		//  set color white, otherwise textures get mixed with color
		glColor3f(1.0, 1.0, 1.0);
		//  top-left
		glTexCoord2i(srcrc.x,           srcrc.y);
		glVertex2i  (dst.x,             dst.y);
		//  top-right
		glTexCoord2i(srcrc.x + srcrc.w, srcrc.y);
		glVertex2f  (dst.x + srcrc.w,   dst.y);
		//  bottom-right
		glTexCoord2i(srcrc.x + srcrc.w, srcrc.y + srcrc.h);
		glVertex2f  (dst.x + srcrc.w,   dst.y + srcrc.h);
		//  bottom-left
		glTexCoord2i(srcrc.x,           srcrc.y + srcrc.h);
		glVertex2f  (dst.x,             dst.y + srcrc.h);
	} glEnd();

	glLoadIdentity();
}

void GLSurfaceScreen::fast_blit(PictureID src)
{
	blit(Point(0, 0), src, Rect(Point(0, 0), src->get_w(), src->get_h()), CM_Normal);
}
