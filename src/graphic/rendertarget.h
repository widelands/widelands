/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
#include "base/times.h"
#include "graphic/align.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "logic/widelands_geometry.h"

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
	explicit RenderTarget(Surface*);
	void set_window(const Recti& rc, const Vector2i& ofs);
	bool enter_window(const Recti& rc, Recti* previous, Vector2i* prevofs);

	int32_t width() const;
	int32_t height() const;

	void draw_line_strip(const std::vector<Vector2f>& points, const RGBColor& color, float width);
	void draw_rect(const Recti&, const RGBColor&);
	void fill_rect(const Recti&, const RGBAColor&, BlendMode blend_mode = BlendMode::Copy);
	void brighten_rect(const Recti&, int32_t factor);

	void blit(const Vector2i& dst,
	          const Image* image,
	          BlendMode blend_mode = BlendMode::UseAlpha,
	          UI::Align = UI::Align::kLeft);

	// Like blit. See MonochromeBlitProgram for details.
	void blit_monochrome(const Vector2i& dst,
	                     const Image* image,
	                     const RGBAColor& blend_mode,
	                     UI::Align = UI::Align::kLeft);

	void blitrect(const Vector2i& dst,
	              const Image* image,
	              const Recti& src,
	              BlendMode blend_mode = BlendMode::UseAlpha);

	// Blits the 'source_rect' from 'image' into the
	// 'destination_rect' in this rendertarget. All alpha values are
	// multiplied with 'opacity' before blitting. The 'blend_mode'
	// defines if values are blended with whats already there or just
	// copied over.
	// Takes by value on purpose.
	void blitrect_scale(Rectf destination_rect,
	                    const Image* image,
	                    Recti source_rect,
	                    float opacity,
	                    BlendMode blend_mode);

	// Like blitrect_scale. See MonochromeBlitProgram for details. Takes by
	// value on purpose.
	void blitrect_scale_monochrome(Rectf destination_rect,
	                               const Image* image,
	                               Recti source_rect,
	                               const RGBAColor& blend);

	void tile(const Recti&,
	          const Image* image,
	          const Vector2i& ofs,
	          BlendMode blend_mode = BlendMode::UseAlpha);

	// Draw the 'animation' as it should appear at 'time' in this target at
	// 'dst'. Optionally, the animation is tinted with 'player_color' and
	// cropped to 'source_rect'.
	// Any sound effects are played with stereo position according to 'coords'.
	// If 'coords' == Widelands::Coords::null(), skip playing any sound effects.
	// The `opacity` parameter is ignored unless `player_color` is nullptr.
	void blit_animation(const Vector2f& dst,
	                    const Widelands::Coords& coords,
	                    const float scale,
	                    uint32_t animation_id,
	                    const Time& time,
	                    const RGBColor* player_color = nullptr,
	                    float opacity = 1.0f,
	                    const int percent_from_bottom = 100);

	void reset();

	const Surface& get_surface() const {
		return *surface_;
	}
	const Recti& get_rect() const {
		return rect_;
	}
	const Vector2i& get_offset() const {
		return offset_;
	}

protected:
	bool clip(Rectf& r) const;
	bool to_surface_geometry(Rectf* destination_rect, Rectf* source_rect) const;

	/// The target surface
	Surface* const surface_;
	/// The current clip rectangle
	Recti rect_;
	/// Drawing offset
	Vector2i offset_ = Vector2i::zero();
};

#endif  // end of include guard: WL_GRAPHIC_RENDERTARGET_H
