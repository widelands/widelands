/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/surface.h"

#include <cstdlib>

#include "base/rect.h"
#include "base/vector.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"

namespace {

// Adjust 'original' so that only 'src_rect' is actually blitted.
BlitData adjust_for_src(BlitData blit_data, const Rectf& src_rect) {
	blit_data.rect.x += src_rect.x;
	blit_data.rect.y += src_rect.y;
	blit_data.rect.w = src_rect.w;
	blit_data.rect.h = src_rect.h;
	return blit_data;
}

// Get the normal of the line between 'start' and 'end'.
template <typename PointType>
Vector2f calculate_line_normal(const PointType& start, const PointType& end) {
	const float dx = end.x - start.x;
	const float dy = end.y - start.y;
	const float len = std::hypot(dx, dy);
	return Vector2f(-dy / len, dx / len);
}

// Tesselates the line made up of 'points' ino triangles and converts them into
// OpenGL space for a renderbuffer of dimensions 'w' and 'h'.
// We need for each line four points. We do not make sure that these quads are
// not properly joined at the corners which does not seem to matter a lot for
// the thin lines that we draw in Widelands.
void tesselate_line_strip(int w,
                          int h,
                          const RGBColor& color,
                          float line_width,
                          const std::vector<Vector2f>& points,
                          std::vector<DrawLineProgram::PerVertexData>* vertices) {
	const float r = color.r / 255.;
	const float g = color.g / 255.;
	const float b = color.b / 255.;

	// Iterate over each line segment, i.e. all points but the last, convert
	// them from pixel space to gl space and draw them.
	for (size_t i = 0; i < points.size() - 1; ++i) {
		const Vector2f p1 = Vector2f(points[i].x, points[i].y);
		const Vector2f p2 = Vector2f(points[i + 1].x, points[i + 1].y);

		const Vector2f normal = calculate_line_normal(p1, p2);
		const Vector2f scaled_normal(0.5f * line_width * normal.x, 0.5f * line_width * normal.y);

		// Quad points are created in rendering order for OpenGL.
		{
			Vector2f p = p1 - scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, 1.});
		}

		{
			Vector2f p = p2 - scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, 1.});
		}

		{
			Vector2f p = p1 + scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, -1.});
		}

		vertices->push_back(vertices->at(vertices->size() - 2));
		vertices->push_back(vertices->at(vertices->size() - 2));

		{
			Vector2f p = p2 + scaled_normal;
			pixel_to_gl_renderbuffer(w, h, &p.x, &p.y);
			vertices->emplace_back(DrawLineProgram::PerVertexData{p.x, p.y, 0.f, r, g, b, -1.});
		}
	}
}

}  // namespace

void Surface::fill_rect(const Rectf& rc, const RGBAColor& clr, BlendMode blend_mode) {
	const Rectf rect = rect_to_gl_renderbuffer(width(), height(), rc);
	do_fill_rect(rect, clr, blend_mode);
}

void Surface::brighten_rect(const Rectf& rc, const int32_t factor) {
	if (factor == 0) {
		return;
	}

	const BlendMode blend_mode = factor < 0 ? BlendMode::Subtract : BlendMode::UseAlpha;
	const int abs_factor = std::abs(factor);
	const RGBAColor color(abs_factor, abs_factor, abs_factor, 0);
	const Rectf rect = rect_to_gl_renderbuffer(width(), height(), rc);
	do_fill_rect(rect, color, blend_mode);
}

void Surface::draw_line_strip(const std::vector<Vector2f>& points,
                              const RGBColor& color,
                              float line_width) {
	if (points.size() < 2) {
		return;
	}

	std::vector<DrawLineProgram::PerVertexData> vertices;
	// Each line needs 2 triangles.
	vertices.reserve(3 * 2 * points.size());
	tesselate_line_strip(width(), height(), color, line_width, points, &vertices);
	do_draw_line_strip(std::move(vertices));
}

void Surface::blit_monochrome(const Rectf& dst_rect,
                              const Image& image,
                              const Rectf& src_rect,
                              const RGBAColor& blend) {
	const Rectf rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit_monochrome(rect, adjust_for_src(image.blit_data(), src_rect), blend);
}

void Surface::blit_blended(const Rectf& dst_rect,
                           const Image& image,
                           const Image& texture_mask,
                           const Rectf& src_rect,
                           const RGBColor& blend) {
	const Rectf rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit_blended(rect, adjust_for_src(image.blit_data(), src_rect),
	                adjust_for_src(texture_mask.blit_data(), src_rect), blend);
}

void Surface::blit(const Rectf& dst_rect,
                   const Image& image,
                   const Rectf& src_rect,
                   float opacity,
                   BlendMode blend_mode) {
	const Rectf rect = rect_to_gl_renderbuffer(width(), height(), dst_rect);
	do_blit(rect, adjust_for_src(image.blit_data(), src_rect), opacity, blend_mode);
}

void draw_rect(const Rectf& rc, const RGBColor& clr, Surface* surface) {
	const Vector2f top_left = Vector2f(rc.x + 0.5f, rc.y + 0.5f);
	const Vector2f top_right = Vector2f(rc.x + rc.w - 0.5f, rc.y + 0.5f);
	const Vector2f bottom_right = Vector2f(rc.x + rc.w - 0.5f, rc.y + rc.h - 0.5f);
	const Vector2f bottom_left = Vector2f(rc.x + 0.5f, rc.y + rc.h - 0.5f);

	surface->draw_line_strip({top_left, top_right, bottom_right}, clr, 1);
	// We need to split this up in order not to miss a pixel on the bottom right corner.
	surface->draw_line_strip(
	   {Vector2f(bottom_right.x + 1, bottom_right.y), bottom_left, top_left}, clr, 1);
}
