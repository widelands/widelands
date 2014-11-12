/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/gl/surface.h"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include "base/macros.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/draw_rect_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/surface_texture.h"
#include "graphic/graphic.h"

uint16_t GLSurface::width() const {
	return m_w;
}

uint16_t GLSurface::height() const {
	return m_h;
}

uint8_t * GLSurface::get_pixels() const
{
	return m_pixels.get();
}

uint32_t GLSurface::get_pixel(uint16_t x, uint16_t y) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[y * get_pitch() + 4 * x];
	return *(reinterpret_cast<uint32_t *>(data));
}

void GLSurface::set_pixel(uint16_t x, uint16_t y, uint32_t clr) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[y * get_pitch() + 4 * x];
	*(reinterpret_cast<uint32_t *>(data)) = clr;
}

FloatRect GLSurface::to_opengl(const Rect& rect, ConversionMode mode) {
	const float delta = mode == ConversionMode::kExact ? 0. : 0.5;
	float x1 = rect.x + delta;
	float y1 = rect.y + delta;
	pixel_to_gl(&x1, &y1);
	float x2 = rect.x + rect.w - delta;
	float y2 = rect.y + rect.h - delta;
	pixel_to_gl(&x2, &y2);

	return FloatRect(x1, y1, x2 - x1, y2 - y1);
}

void GLSurface::draw_rect(const Rect& rc, const RGBColor& clr)
{
	glViewport(0, 0, width(), height());
	DrawRectProgram::instance().draw(to_opengl(rc, ConversionMode::kMidPoint), clr);
}

/**
 * Draws a filled rectangle
 */
void GLSurface::fill_rect(const Rect& rc, const RGBAColor& clr) {
	glViewport(0, 0, width(), height());

	glBlendFunc(GL_ONE, GL_ZERO);

	FillRectProgram::instance().draw(to_opengl(rc, ConversionMode::kExact), clr);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * Change the brightness of the given rectangle
 */
void GLSurface::brighten_rect(const Rect& rc, const int32_t factor)
{
	if (!factor)
		return;

	glViewport(0, 0, width(), height());

	// The simple trick here is to fill the rect, but using a different glBlendFunc that will sum
	// src and target (or subtract them if factor is negative).
	if (factor < 0) {
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
	}

	glBlendFunc(GL_ONE, GL_ONE);

	const int delta = std::abs(factor);
	FillRectProgram::instance().draw(
	   to_opengl(rc, ConversionMode::kExact), RGBAColor(delta, delta, delta, 0));

	if (factor < 0) {
		glBlendEquation(GL_FUNC_ADD);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GLSurface::draw_line
	(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor& color, uint8_t gwidth)
{
	float gl_x1 = x1 + 0.5;
	float gl_y1 = y1 + 0.5;
	pixel_to_gl(&gl_x1, &gl_y1);

	float gl_x2 = x2 + 0.5;
	float gl_y2 = y2 + 0.5;
	pixel_to_gl(&gl_x2, &gl_y2);

	DrawLineProgram::instance().draw(gl_x1, gl_y1, gl_x2, gl_y2, color, gwidth);
}

// Converts the pixel (x, y) in a texture to a gl coordinate in [0, 1].
inline void pixel_to_gl_texture(const int width, const int height, float* x, float* y) {
	*x = (*x / width);
	*y = (*y / height);
}

void GLSurface::blit
	(const Point& dst, const Surface* image, const Rect& srcrc, Composite cm)
{
	glViewport(0, 0, width(), height());

	// Source Rectangle.
	const GLSurfaceTexture* const texture = static_cast<const GLSurfaceTexture*>(image);
	FloatRect gl_src_rect;
	{
		float x1 = srcrc.x;
		float y1 = srcrc.y;
		pixel_to_gl_texture(texture->width(), texture->height(), &x1, &y1);
		float x2 = srcrc.x + srcrc.w;
		float y2 = srcrc.y + srcrc.h;
		pixel_to_gl_texture(texture->width(), texture->height(), &x2, &y2);
		gl_src_rect.x = x1;
		gl_src_rect.y = y1;
		gl_src_rect.w = x2 - x1;
		gl_src_rect.h = y2 - y1;
	}

	const FloatRect gl_dst_rect = to_opengl(Rect(dst.x, dst.y, srcrc.w, srcrc.h), ConversionMode::kExact);

	BlitProgram::instance().draw(gl_dst_rect, gl_src_rect, texture->get_gl_texture(), cm);
}
