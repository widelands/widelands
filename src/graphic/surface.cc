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
#include "base/point.h"
#include "base/rect.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"
#include "graphic/line_strip_mode.h"

namespace {

// Adjust 'original' so that only 'src_rect' is actually blitted.
BlitData adjust_for_src(BlitData blit_data, const Rect& src_rect) {
	blit_data.rect.x += src_rect.x;
	blit_data.rect.y += src_rect.y;
	blit_data.rect.w = src_rect.w;
	blit_data.rect.h = src_rect.h;
	return blit_data;
}

// Get the normal of the line between 'start' and 'end'.
template <typename PointType>
FloatPoint calculate_line_normal(const PointType& start, const PointType& end) {
	const float dx = end.x - start.x;
	const float dy = end.y - start.y;
	const float len = std::hypot(dx, dy);
	return FloatPoint(-dy / len, dx / len);
}

// Finds the pseudo-normal of a point at the join of two lines. We construct
// this like a miter joint from woodworks. The best explanation I found for
// this algorithm is here
// https://forum.libcinder.org/topic/smooth-thick-lines-using-geometry-shader#23286000001269127
FloatPoint calculate_pseudo_normal(const Point& p0, const Point& p1, const Point& p2) {
	FloatPoint tangent = normalize(normalize(p2 - p1) + normalize(p1 - p0));
	FloatPoint miter(-tangent.y, tangent.x);
	float len = 1. / dot(miter, calculate_line_normal(p0, p1));
	return FloatPoint(miter.x * len, miter.y * len);
}

}  // namespace

void draw_rect(const Rect& rc, const RGBColor& clr, Surface* surface) {
	surface->draw_line_strip(LineStripMode::kClose, {
		Point(rc.x, rc.y),
		Point(rc.x + rc.w, rc.y),
		Point(rc.x + rc.w, rc.y + rc.h),
		Point(rc.x, rc.y + rc.h),
		}, clr, 1);
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

void Surface::draw_line_strip(const LineStripMode& line_strip_mode,
                     std::vector<Point> points,
                     const RGBColor& color,
                     float line_width) {
	assert(points.size() > 1);

	// Figure out the tesselation for the line. The normal for a point is the
	// average of the up to two lines it is part of.
	std::vector<FloatPoint> normals;
	for (size_t i = 0; i < points.size(); ++i) {
		if (i == 0) {
			switch (line_strip_mode) {
			case LineStripMode::kOpen:
				normals.push_back(calculate_line_normal(points.front(), points[1]));
				break;

			case LineStripMode::kClose:
				normals.push_back(calculate_pseudo_normal(points.back(), points.front(), points[1]));
				break;
			}
		} else if (i == points.size() - 1) {
			switch (line_strip_mode) {
			case LineStripMode::kOpen:
				normals.push_back(calculate_line_normal(points[points.size() - 2], points.back()));
				break;

			case LineStripMode::kClose:
				normals.push_back(
				   calculate_pseudo_normal(points[points.size() - 2], points.back(), points.front()));
				break;
			}
		} else {
			normals.push_back(calculate_pseudo_normal(points[i - 1], points[i], points[i + 1]));
		}
	}
	if (line_strip_mode == LineStripMode::kClose) {
		// Push the first point as the last point again. For drawing we do not
		// need to special case closed or open lines anymore.
		normals.push_back(normals.front());
		points.push_back(points.front());
	}
	assert(points.size() == normals.size());
	assert(!points.empty());

	std::vector<FloatPoint> gl_points;

	// Iterate over each line segment, i.e. all points but the last, convert
	// them from pixel space to gl space and draw them.
	const auto w = width();
	const auto h = height();
	for (size_t i = 0; i < points.size() - 1; ++i) {
		const FloatPoint p1 = FloatPoint(points[i].x, points[i].y);
		const FloatPoint p2 = FloatPoint(points[i + 1].x, points[i + 1].y);
		const FloatPoint scaled_n1(0.5 * line_width * normals[i].x, 0.5 * line_width * normals[i].y);
		const FloatPoint scaled_n2(
		   0.5 * line_width * normals[i + 1].x, 0.5 * line_width * normals[i + 1].y);

		// Quad points are created in rendering order for OpenGL.
		FloatPoint quad_a = p1 - scaled_n1;
		pixel_to_gl_renderbuffer(w, h, &quad_a.x, &quad_a.y);
		gl_points.emplace_back(quad_a);

		FloatPoint quad_b = p2 - scaled_n2;
		pixel_to_gl_renderbuffer(w, h, &quad_b.x, &quad_b.y);
		gl_points.emplace_back(quad_b);

		FloatPoint quad_c = p1 + scaled_n1 + scaled_n1;
		pixel_to_gl_renderbuffer(w, h, &quad_c.x, &quad_c.y);
		gl_points.emplace_back(quad_c);

		FloatPoint quad_d = p2 + scaled_n2 + scaled_n2;
		pixel_to_gl_renderbuffer(w, h, &quad_d.x, &quad_d.y);
		gl_points.emplace_back(quad_d);
	}
	do_draw_line_strip(gl_points, color);
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
