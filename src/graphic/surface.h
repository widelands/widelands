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

	// Converts the given pixel into an OpenGl point. This might
	// need some flipping of axis, depending if you want to render
	// on the screen or not.
	virtual void pixel_to_gl(float* x, float* y) const = 0;

	// Setups OpenGL to render to this surface.
	virtual void setup_gl() = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(Surface);
};

/// Draws a rect (frame only) to the surface.
void draw_rect(const Rect&, const RGBColor&, Surface* destination);

/// This draws a part of 'texture' to 'surface'.
void blit
	(const Rect& dst, const Image&, const Rect& srcrc, const float opacity,
	 BlendMode blend_mode, Surface* destination);

/// This draws a grayed out version. See MonochromeBlitProgram.
void
blit_monochrome
	(const Rect& dst, const Image&, const Rect& srcrc,
	 const RGBAColor& multiplier, Surface* destination);

/// This draws a playercolor blended image. See BlendedBlitProgram.
void blit_blended
	(const Rect& dst, const Image& image, const Image& mask, const Rect&
	 srcrc, const RGBColor& blend, Surface* destination);

/// Draws a filled rect to the destination. No blending takes place, the values
// in the target are just replaced (i.e. / BlendMode would be BlendMode::Copy).
void fill_rect(const Rect&, const RGBAColor&, Surface* destination);

/// draw a line to the destination
void draw_line
	(int x1, int y1, int x2, int y2, const RGBColor& color,
	 int width, Surface* destination);

/// makes a rectangle on the destination brighter (or darker).
void brighten_rect(const Rect&, int factor, Surface* destination);

#endif  // end of include guard: WL_GRAPHIC_SURFACE_H
