/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#include "graphic/surface.h"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <SDL.h>

#include "base/macros.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/draw_rect_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/utils.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"


uint16_t Surface::width() const {
	return m_w;
}

uint16_t Surface::height() const {
	return m_h;
}

uint8_t * Surface::get_pixels() const
{
	return m_pixels.get();
}

uint32_t Surface::get_pixel(uint16_t x, uint16_t y) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[y * get_pitch() + 4 * x];
	return *(reinterpret_cast<uint32_t *>(data));
}

uint16_t Surface::get_pitch() const {
	return 4 * m_w;
}

const SDL_PixelFormat & Surface::format() const {
	return Gl::gl_rgba_format();
}


void Surface::set_pixel(uint16_t x, uint16_t y, uint32_t clr) {
	assert(m_pixels);
	assert(x < m_w);
	assert(y < m_h);

	uint8_t * data = &m_pixels[y * get_pitch() + 4 * x];
	*(reinterpret_cast<uint32_t *>(data)) = clr;
}

FloatRect Surface::to_opengl(const Rect& rect, ConversionMode mode) {
	const float delta = mode == ConversionMode::kExact ? 0. : 0.5;
	float x1 = rect.x + delta;
	float y1 = rect.y + delta;
	pixel_to_gl(&x1, &y1);
	float x2 = rect.x + rect.w - delta;
	float y2 = rect.y + rect.h - delta;
	pixel_to_gl(&x2, &y2);

	return FloatRect(x1, y1, x2 - x1, y2 - y1);
}

void Surface::draw_rect(const Rect& rc, const RGBColor& clr)
{
	glViewport(0, 0, width(), height());
	DrawRectProgram::instance().draw(to_opengl(rc, ConversionMode::kMidPoint), clr);
}

/**
 * Draws a filled rectangle
 */
void Surface::fill_rect(const Rect& rc, const RGBAColor& clr) {
	glViewport(0, 0, width(), height());

	glBlendFunc(GL_ONE, GL_ZERO);

	FillRectProgram::instance().draw(to_opengl(rc, ConversionMode::kExact), clr);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * Change the brightness of the given rectangle
 */
void Surface::brighten_rect(const Rect& rc, const int32_t factor)
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

void Surface::draw_line
	(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor& color, uint8_t gwidth)
{
	glViewport(0, 0, width(), height());

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

void Surface::blit
	(const Point& dst, const Texture* texture, const Rect& srcrc, BlendMode blend_mode)
{
	glViewport(0, 0, width(), height());

	// Source Rectangle. We have to take into account that the texture might be
	// a subtexture in another bigger texture. So we first figure out the pixel
	// coordinates given it is a full texture (values between 0 and 1) and then
	// adjust these for the texture coordinates in the parent texture.
	FloatRect gl_src_rect;
	{
		const FloatRect& texture_coordinates = texture->texture_coordinates();

		float x1 = srcrc.x;
		float y1 = srcrc.y;
		pixel_to_gl_texture(texture->width(), texture->height(), &x1, &y1);
		x1 = texture_coordinates.x + x1 * texture_coordinates.w;
		y1 = texture_coordinates.y + y1 * texture_coordinates.h;

		float x2 = srcrc.x + srcrc.w;
		float y2 = srcrc.y + srcrc.h;
		pixel_to_gl_texture(texture->width(), texture->height(), &x2, &y2);
		x2 = texture_coordinates.x + x2 * texture_coordinates.w;
		y2 = texture_coordinates.y + y2 * texture_coordinates.h;

		gl_src_rect.x = x1;
		gl_src_rect.y = y1;
		gl_src_rect.w = x2 - x1;
		gl_src_rect.h = y2 - y1;
	}

	const FloatRect gl_dst_rect = to_opengl(Rect(dst.x, dst.y, srcrc.w, srcrc.h), ConversionMode::kExact);

	BlitProgram::instance().draw(gl_dst_rect, gl_src_rect, texture->get_gl_texture(), blend_mode);
}
