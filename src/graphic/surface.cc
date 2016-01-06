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
#include "base/rect.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"

namespace {

// Adjust 'original' so that only 'src_rect' is actually blitted.
BlitData adjust_for_src(BlitData blit_data, const Rect& src_rect) {
	blit_data.rect.x += src_rect.x;
	blit_data.rect.y += src_rect.y;
	blit_data.rect.w = src_rect.w;
	blit_data.rect.h = src_rect.h;
	return blit_data;
}

}  // namespace

void draw_rect(const Rect& rc, const RGBColor& clr, Surface* surface) {
	surface->draw_line(Point(rc.x, rc.y), Point(rc.x + rc.w, rc.y), clr, 1);
	surface->draw_line(Point(rc.x + rc.w, rc.y), Point(rc.x + rc.w, rc.y + rc.h), clr, 1);
	surface->draw_line(Point(rc.x + rc.w, rc.y + rc.h), Point(rc.x, rc.y + rc.h), clr, 1);
	surface->draw_line(Point(rc.x, rc.y + rc.h), Point(rc.x, rc.y), clr, 1);
}

void Surface::fill_rect(const Rect& rc, const RGBAColor& clr) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), rc);
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
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), rc);
	do_fill_rect(rect, color, blend_mode);
}

void Surface::draw_line
	(const Point& start, const Point& end, const RGBColor& color, int line_width)
{
	float gl_x1 = start.x;
	float gl_y1 = start.y;

	// Include the end pixel.
	float gl_x2 = end.x + 1.;
	float gl_y2 = end.y + 1.;
	pixel_to_gl_renderbuffer(width(), height(), &gl_x1, &gl_y1);
	pixel_to_gl_renderbuffer(width(), height(), &gl_x2, &gl_y2);

	do_draw_line(FloatPoint(gl_x1, gl_y1), FloatPoint(gl_x2, gl_y2), color, line_width);
}

void Surface::blit_monochrome(const Rect& dst_rect,
                              const Image& image,
                              const Rect& src_rect,
                              const RGBAColor& blend) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit_monochrome(rect, adjust_for_src(image.blit_data(), src_rect), blend);
}

void Surface::blit_blended(const Rect& dst_rect,
                           const Image& image,
                           const Image& texture_mask,
                           const Rect& src_rect,
                           const RGBColor& blend) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit_blended(rect, adjust_for_src(image.blit_data(), src_rect),
	                adjust_for_src(texture_mask.blit_data(), src_rect), blend);
}

void Surface::blit(const Rect& dst_rect,
                   const Image& image,
                   const Rect& src_rect,
                   float opacity,
                   BlendMode blend_mode) {
	const FloatRect rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit(rect, adjust_for_src(image.blit_data(), src_rect), opacity, blend_mode);
}
