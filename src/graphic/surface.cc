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
#include "graphic/gl/utils.h"

namespace  {

// Convert the 'rect' in pixel space into opengl space.
enum class ConversionMode {
	// Convert the rect as given.
	kExact,

	// Convert the rect so that the borders are in the center
	// of the pixels.
	kMidPoint,
};

FloatRect to_opengl(const Surface& surface, const Rect& rect, ConversionMode mode) {
	const float delta = mode == ConversionMode::kExact ? 0. : 0.5;
	float x1 = rect.x + delta;
	float y1 = rect.y + delta;
	surface.pixel_to_gl(&x1, &y1);
	float x2 = rect.x + rect.w - delta;
	float y2 = rect.y + rect.h - delta;
	surface.pixel_to_gl(&x2, &y2);
	return FloatRect(x1, y1, x2 - x1, y2 - y1);
}

// Converts the pixel (x, y) in a texture to a gl coordinate in [0, 1].
inline void pixel_to_gl_texture(const int width, const int height, float* x, float* y) {
	*x = (*x / width);
	*y = (*y / height);
}

// Convert 'srcrc' from pixel space into opengl space, taking into account that
// it might be a subtexture in a bigger texture.
// NOCOM(#sirver): KILL
FloatRect source_rect_to_gl(const Image& image, const Rect& src_rect) {
	// Source Rectangle. We have to take into account that the texture might be
	// a subtexture in another bigger texture. So we first figure out the pixel
	// coordinates given it is a full texture (values between 0 and 1) and then
	// adjust these for the texture coordinates in the parent texture.
	const FloatRect& texture_coordinates = image.texture_coordinates();

	float x1 = src_rect.x;
	float y1 = src_rect.y;
	pixel_to_gl_texture(image.width(), image.height(), &x1, &y1);
	x1 = texture_coordinates.x + x1 * texture_coordinates.w;
	y1 = texture_coordinates.y + y1 * texture_coordinates.h;

	float x2 = src_rect.x + src_rect.w;
	float y2 = src_rect.y + src_rect.h;
	pixel_to_gl_texture(image.width(), image.height(), &x2, &y2);
	x2 = texture_coordinates.x + x2 * texture_coordinates.w;
	y2 = texture_coordinates.y + y2 * texture_coordinates.h;

	return FloatRect(x1, y1, x2 - x1, y2 - y1);
}

// Convert 'srcrc' from pixel space into opengl space, taking into account that
// it might be a subtexture in a bigger texture.
BlitSource to_blit_source(const Image& image, const Rect& src_rect) {
	// Source Rectangle. We have to take into account that the texture might be
	// a subtexture in another bigger texture. So we first figure out the pixel
	// coordinates given it is a full texture (values between 0 and 1) and then
	// adjust these for the texture coordinates in the parent texture.
	const FloatRect& texture_coordinates = image.texture_coordinates();

	float x1 = src_rect.x;
	float y1 = src_rect.y;
	pixel_to_gl_texture(image.width(), image.height(), &x1, &y1);
	x1 = texture_coordinates.x + x1 * texture_coordinates.w;
	y1 = texture_coordinates.y + y1 * texture_coordinates.h;

	float x2 = src_rect.x + src_rect.w;
	float y2 = src_rect.y + src_rect.h;
	pixel_to_gl_texture(image.width(), image.height(), &x2, &y2);
	x2 = texture_coordinates.x + x2 * texture_coordinates.w;
	y2 = texture_coordinates.y + y2 * texture_coordinates.h;

	return BlitSource {
		FloatRect(x1, y1, x2 - x1, y2 - y1), image.get_gl_texture(),
	};
}

}  // namespace

void draw_rect(const Rect& rc, const RGBColor& clr, Surface* surface) {
	surface->draw_line(rc.x, rc.y, rc.x + rc.w, rc.y, clr, 1);
	surface->draw_line(rc.x + rc.w, rc.y, rc.x + rc.w, rc.y + rc.h, clr, 1);
	surface->draw_line(rc.x + rc.w, rc.y + rc.h, rc.x, rc.y + rc.h, clr, 1);
	surface->draw_line(rc.x, rc.y + rc.h, rc.x, rc.y, clr, 1);
}

void Surface::fill_rect(const Rect& rc, const RGBAColor& clr) {
	const FloatRect rect = to_opengl(*this, rc, ConversionMode::kExact);
	do_fill_rect(rect, clr, BlendMode::Copy);
}

void Surface::brighten_rect(const Rect& rc, const int32_t factor)
{
	if (!factor) {
		return;
	}

	const BlendMode blend_mode = factor < 0 ? BlendMode::Subtract : BlendMode::UseAlpha;
	const int abs_factor = std::abs(factor);
	const RGBAColor color(abs_factor, abs_factor, abs_factor, 0);
	const FloatRect rect = to_opengl(*this, rc, ConversionMode::kExact);
	do_fill_rect(rect, color, blend_mode);
}

void Surface::draw_line
	(int x1, int y1, int x2, int y2, const RGBColor& color, int gwidth)
{
	float gl_x1 = x1 + 0.5;
	float gl_y1 = y1 + 0.5;
	pixel_to_gl(&gl_x1, &gl_y1);

	float gl_x2 = x2 + 0.5;
	float gl_y2 = y2 + 0.5;
	pixel_to_gl(&gl_x2, &gl_y2);

	do_draw_line(FloatPoint(gl_x1, gl_y1), FloatPoint(gl_x2, gl_y2), color);
}

void Surface::blit_monochrome(const Rect& dst_rect,
                              const Image& image,
                              const Rect& src_rect,
                              const RGBAColor& blend) {
	const FloatRect gl_dst_rect = to_opengl(*this, dst_rect, ConversionMode::kExact);
	const BlitSource texture = to_blit_source(image, src_rect);
	do_blit_monochrome(gl_dst_rect, texture, blend);
}

void Surface::blit_blended(const Rect& dst_rect,
                           const Image& image,
                           const Image& texture_mask,
                           const Rect& src_rect,
                           const RGBColor& blend) {
	const BlitSource texture = to_blit_source(image, src_rect);
	const BlitSource mask = to_blit_source(texture_mask, src_rect);
	const FloatRect gl_dst_rect = to_opengl(*this, dst_rect, ConversionMode::kExact);
	do_blit_blended(gl_dst_rect, texture, mask, blend);
}

void Surface::blit(const Rect& dst_rect,
                   const Image& image,
                   const Rect& src_rect,
                   float opacity,
                   BlendMode blend_mode) {
	const BlitSource texture = to_blit_source(image, src_rect);
	const FloatRect gl_dst_rect = to_opengl(*this, dst_rect, ConversionMode::kExact);
	do_blit(gl_dst_rect, texture, opacity, blend_mode);
}
