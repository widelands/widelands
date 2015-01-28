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
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"

namespace  {

// Convert 'srcrc' from pixel space into opengl space, taking into account that
// it might be a subtexture in a bigger texture.
BlitSource to_blit_source(const Image& image, const Rect& src_rect) {
	// Source Rectangle. We have to take into account that the texture might be
	// a subtexture in another bigger texture. So we first figure out the pixel
	// coordinates given it is a full texture (values between 0 and 1) and then
	// adjust these for the texture coordinates in the parent texture.

	float left = src_rect.x;
	float top = src_rect.y;
	float right = src_rect.x + src_rect.w;
	float bottom = src_rect.y + src_rect.h;
	pixel_to_gl_texture(image.width(), image.height(), &right, &bottom);
	pixel_to_gl_texture(image.width(), image.height(), &left, &top);

	const FloatRect& texture_coordinates = image.texture_coordinates();
	left = texture_coordinates.x + left * texture_coordinates.w;
	top = texture_coordinates.y + top * texture_coordinates.h;
	right = texture_coordinates.x + right * texture_coordinates.w;
	bottom = texture_coordinates.y + bottom * texture_coordinates.h;

	return BlitSource{
	   FloatRect(left, bottom, right - left, top - bottom), image.get_gl_texture(),
	};
}

}  // namespace

void draw_rect(const Rect& rc, const RGBColor& clr, Surface* surface) {
	surface->draw_line(Point(rc.x, rc.y), Point(rc.x + rc.w, rc.y), clr, 1);
	surface->draw_line(Point(rc.x + rc.w, rc.y), Point(rc.x + rc.w, rc.y + rc.h), clr, 1);
	surface->draw_line(Point(rc.x + rc.w, rc.y + rc.h), Point(rc.x, rc.y + rc.h), clr, 1);
	surface->draw_line(Point(rc.x, rc.y + rc.h), Point(rc.x, rc.y), clr, 1);
}

void Surface::fill_rect(const Rect& rc, const RGBAColor& clr) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), rc, ConversionMode::kExact);
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
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), rc, ConversionMode::kExact);
	do_fill_rect(rect, color, blend_mode);
}

void Surface::draw_line
	(const Point& start, const Point& end, const RGBColor& color, int gwidth)
{
	float gl_x1 = start.x + 0.5;
	float gl_y1 = start.y + 0.5;
	pixel_to_gl_renderbuffer(width(), height(), &gl_x1, &gl_y1);

	float gl_x2 = end.x + 0.5;
	float gl_y2 = end.y + 0.5;
	pixel_to_gl_renderbuffer(width(), height(), &gl_x2, &gl_y2);

	do_draw_line(FloatPoint(gl_x1, gl_y1), FloatPoint(gl_x2, gl_y2), color);
}

void Surface::blit_monochrome(const Rect& dst_rect,
                              const Image& image,
                              const Rect& src_rect,
                              const RGBAColor& blend) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect, ConversionMode::kExact);
	const BlitSource texture = to_blit_source(image, src_rect);
	do_blit_monochrome(rect, texture, blend);
}

void Surface::blit_blended(const Rect& dst_rect,
                           const Image& image,
                           const Image& texture_mask,
                           const Rect& src_rect,
                           const RGBColor& blend) {
	const BlitSource texture = to_blit_source(image, src_rect);
	const BlitSource mask = to_blit_source(texture_mask, src_rect);
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect, ConversionMode::kExact);
	do_blit_blended(rect, texture, mask, blend);
}

void Surface::blit(const Rect& dst_rect,
                   const Image& image,
                   const Rect& src_rect,
                   float opacity,
                   BlendMode blend_mode) {
	const BlitSource texture = to_blit_source(image, src_rect);
	const FloatRect rect =
	   rect_to_gl_renderbuffer(width(), height(), dst_rect, ConversionMode::kExact);
	do_blit(rect, texture, opacity, blend_mode);
}
