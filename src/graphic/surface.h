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
 *
 */

#ifndef WL_GRAPHIC_SURFACE_H
#define WL_GRAPHIC_SURFACE_H

#include <memory>

#include "base/macros.h"
#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/image.h"

class Texture;

// Interface to a basic surfaces that can be used as destination for blitting
// and drawing. It also allows low level pixel access.
class Surface {
public:
	Surface() = default;
	virtual ~Surface() {}

	/// Dimensions.
	virtual int width() const = 0;
	virtual int height() const = 0;

	/// This draws a part of 'texture'.
	void blit(const Rect& dst,
	          const Image&,
	          const Rect& srcrc,
	          const float opacity,
	          BlendMode blend_mode);

	/// This draws a playercolor blended image.
	void blit_blended(const Rect& dst,
	                  const Image& image,
	                  const Image& texture_mask,
	                  const Rect& srcrc,
	                  const RGBColor& blend);

	/// This draws a grayed out version.
	void
	blit_monochrome(const Rect& dst, const Image&, const Rect& srcrc, const RGBAColor& multiplier);

	/// Draws a filled rect to the destination. No blending takes place, the values
	// in the target are just replaced (i.e. / BlendMode would be BlendMode::Copy).
	void fill_rect(const Rect&, const RGBAColor&);

	/// draw a line to the destination
	void draw_line(const Point& start, const Point& end, const RGBColor& color, int width);

	/// makes a rectangle on the destination brighter (or darker).
	void brighten_rect(const Rect&, int factor);

private:
	/// The actual implementation of the methods below.
	virtual void do_blit(const FloatRect& dst_rect,
	                     const BlitData& texture,
	                     float opacity,
	                     BlendMode blend_mode) = 0;

	virtual void do_blit_blended(const FloatRect& dst_rect,
	                             const BlitData& texture,
	                             const BlitData& mask,
	                             const RGBColor& blend) = 0;

	virtual void do_blit_monochrome(const FloatRect& dst_rect,
	                                const BlitData& texture,
	                                const RGBAColor& blend) = 0;

	virtual void
	do_draw_line(const FloatPoint& start, const FloatPoint& end, const RGBColor& color, int width) = 0;

	virtual void
	do_fill_rect(const FloatRect& dst_rect, const RGBAColor& color, BlendMode blend_mode) = 0;

	DISALLOW_COPY_AND_ASSIGN(Surface);
};

/// Draws a rect (frame only) to the surface.
void draw_rect(const Rect&, const RGBColor&, Surface* destination);

#endif  // end of include guard: WL_GRAPHIC_SURFACE_H
