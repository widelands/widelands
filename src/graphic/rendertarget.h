/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_RENDERTARGET_H
#define WL_GRAPHIC_RENDERTARGET_H

#include <vector>

#include "base/rect.h"
#include "graphic/align.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/image.h"

class Animation;
class Surface;

/**
 * This class represents anything that can be rendered to.
 *
 * It supports windows, which are composed of a clip rectangle and a drawing
 * offset:
 * The drawing offset will be added to all coordinates that are passed to
 * drawing routines. Therefore, the offset is usually negative. Then the
 * coordinates are interpreted as relative to the clip rectangle and the
 * primitives are clipped accordingly.
 * \ref enter_window() can be used to enter a sub-window of the current window.
 * When you're finished drawing in the window, restore the old window by calling
 * \ref set_window() with the values stored in previous and prevofs.
 * \note If the sub-window would be empty/invisible, \ref enter_window() returns
 * false and doesn't change the window state at all.
*/
// TODO(sirver): remove window functions and merge with surface once
// the old richtext renderer is gone.
class RenderTarget {
public:
	RenderTarget(Surface*);
	void set_window(const Rect& rc, const Point& ofs);
	bool enter_window(const Rect& rc, Rect* previous, Point* prevofs);

	int32_t width() const;
	int32_t height() const;

	void draw_line_strip(const std::vector<FloatPoint>& points, const RGBColor& color, float width);
	void draw_rect(const Rect&, const RGBColor&);
	void fill_rect(const Rect&, const RGBAColor&, BlendMode blend_mode = BlendMode::Copy);
	void brighten_rect(const Rect&, int32_t factor);

	void blit(const Point& dst,
	          const Image* image,
	          BlendMode blend_mode = BlendMode::UseAlpha,
	          UI::Align = UI::Align::kTopLeft);

	// Like blit. See MonochromeBlitProgram for details.
	void blit_monochrome(const Point& dst,
	                     const Image* image,
	                     const RGBAColor& blend_mode,
	                     UI::Align = UI::Align::kTopLeft);

	void blitrect(const Point& dst,
	              const Image* image,
	              const Rect& src,
	              BlendMode blend_mode = BlendMode::UseAlpha);

	// Blits the 'source_rect' from 'image' into the
	// 'destination_rect' in this rendertarget. All alpha values are
	// multiplied with 'opacity' before blitting. The 'blend_mode'
	// defines if values are blended with whats already there or just
	// copied over.
	// Rect's are taken by value on purpose.
	void blitrect_scale(Rect destination_rect,
	                    const Image* image,
	                    Rect source_rect,
	                    float opacity,
	                    BlendMode blend_mode);

	// Like blitrect_scale. See MonochromeBlitProgram for details. Rect's are
	// taken by value on purpose.
	void blitrect_scale_monochrome(Rect destination_rect,
	                               const Image* image,
	                               Rect source_rect,
	                               const RGBAColor& blend);

	void tile(const Rect&,
	          const Image* image,
	          const Point& ofs,
	          BlendMode blend_mode = BlendMode::UseAlpha);

	// Draw the 'animation' as it should appear at 'time' in this target at 'dst'. Optionally, the
	// animation is
	// tinted with 'player_color' and cropped to 'source_rect'.
	void blit_animation(const Point& dst, uint32_t animation, uint32_t time);
	void blit_animation(const Point& dst,
	                    uint32_t animation,
	                    uint32_t time,
	                    const RGBColor& player_color);
	void blit_animation(const Point& dst,
	                    uint32_t animation,
	                    uint32_t time,
	                    const RGBColor& player_color,
	                    const Rect& source_rect);

	void reset();

	Surface* get_surface() const {
		return surface_;
	}
	const Rect& get_rect() const {
		return rect_;
	}
	const Point& get_offset() const {
		return offset_;
	}

protected:
	bool clip(Rect& r) const;
	bool to_surface_geometry(Rect* destination_rect, Rect* source_rect) const;

	// Does the actual blitting.
	void do_blit_animation(const Point& dst,
	                       const Animation& animation,
	                       uint32_t time,
	                       const RGBColor* player_color,
	                       const Rect& source_rect);

	/// The target surface
	Surface* surface_;
	/// The current clip rectangle
	Rect rect_;
	/// Drawing offset
	Point offset_;
};

#endif  // end of include guard: WL_GRAPHIC_RENDERTARGET_H
