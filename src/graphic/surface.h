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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_SURFACE_H
#define WL_GRAPHIC_SURFACE_H

#include "base/macros.h"
#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/image.h"

// Interface to a basic surfaces that can be used as destination for blitting
// and drawing. It also allows low level pixel access.
class Surface {
public:
	Surface() = default;
	virtual ~Surface() {
	}

	/// Dimensions.
	virtual int width() const = 0;
	virtual int height() const = 0;

	/// This draws a part of 'texture'.
	void blit(const Rectf& dst,
	          const Image&,
	          const Rectf& srcrc,
	          const float opacity,
	          BlendMode blend_mode);

	/// This draws a playercolor blended image.
	void blit_blended(const Rectf& dst,
	                  const Image& image,
	                  const Image& texture_mask,
	                  const Rectf& srcrc,
	                  const RGBColor& blend);

	/// This draws a grayed out version.
	void
	blit_monochrome(const Rectf& dst, const Image&, const Rectf& srcrc, const RGBAColor& multiplier);

	/// Draws a filled rect to the destination.
	void fill_rect(const Rectf& dst, const RGBAColor&, BlendMode blend_mode = BlendMode::Copy);

	// Draw a 'width' pixel wide line to the destination. 'points' are taken by
	// value on purpose.
	void draw_line_strip(const std::vector<Vector2f>& points, const RGBColor& color, float width);

	/// makes a rectangle on the destination brighter (or darker).
	void brighten_rect(const Rectf&, int factor);

private:
	/// The actual implementation of the methods below.
	virtual void
	do_blit(const Rectf& dst_rect, const BlitData& texture, float opacity, BlendMode blend_mode) = 0;

	virtual void do_blit_blended(const Rectf& dst_rect,
	                             const BlitData& texture,
	                             const BlitData& mask,
	                             const RGBColor& blend) = 0;

	virtual void
	do_blit_monochrome(const Rectf& dst_rect, const BlitData& texture, const RGBAColor& blend) = 0;

	// Takes argument by value for micro optimization: the argument might then
	// be moved by the compiler instead of copied.
	virtual void do_draw_line_strip(std::vector<DrawLineProgram::PerVertexData> vertices) = 0;

	virtual void
	do_fill_rect(const Rectf& dst_rect, const RGBAColor& color, BlendMode blend_mode) = 0;

	DISALLOW_COPY_AND_ASSIGN(Surface);
};

/// Draws a rect (frame only) to the surface. The width of the surrounding line
/// is 1 pixel, i.e. the transparent inner box of the drawn rectangle starts at
/// (x+1, y+1) and has dimension (w - 2, h - 2).
void draw_rect(const Rectf& rect, const RGBColor&, Surface* destination);

#endif  // end of include guard: WL_GRAPHIC_SURFACE_H
