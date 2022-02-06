/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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
 */

#ifndef WL_GRAPHIC_WORDWRAP_H
#define WL_GRAPHIC_WORDWRAP_H

#include <memory>

#include <base/scoped_timer.h>
#include <unicode/uchar.h>

#include "base/vector.h"
#include "graphic/align.h"
#include "graphic/color.h"
#include "graphic/text/sdl_ttf_font.h"

class RenderTarget;

namespace UI {

/**
 * Helper struct that provides word wrapping and related functionality.
 */
struct WordWrap {
	static constexpr int kLineMargin = 1;

	explicit WordWrap(int fontsize, const RGBColor& color, uint32_t wrapwidth);

	void set_wrapwidth(uint32_t wrapwidth);

	uint32_t wrapwidth() const;

	void wrap(const std::string& text);

	uint32_t width() const;
	uint32_t height() const;
	void set_draw_caret(bool draw_it) {
		draw_caret_ = draw_it;
	}

	void draw(RenderTarget& dst,
	          Vector2i where,
	          Align align = UI::Align::kLeft,
	          uint32_t caret = std::numeric_limits<uint32_t>::max(),
	          bool with_selection = false,
	          uint32_t selection_start = 0,
	          uint32_t selection_end = 0,
	          uint32_t scrollbar_position = 0,
	          const std::string& caret_image_path = std::string());

	void calc_wrapped_pos(uint32_t caret, uint32_t& line, uint32_t& pos) const;
	uint32_t nrlines() const {
		return lines_.size();
	}
	uint32_t line_offset(uint32_t line) const;
	uint32_t offset_of_line_at(int32_t y) const;
	std::string text_of_line_at(int32_t y) const;
	int text_width_of(std::string& text) const;

	void focus();

private:
	struct LineData {
		/// Textual content of the line
		std::string text;

		/// Starting offset of this line within the original un-wrapped text
		size_t start;
	};

	void compute_end_of_line(const std::string& text,
	                         std::string::size_type line_start,
	                         std::string::size_type& line_end,
	                         std::string::size_type& next_line_start,
	                         uint32_t safety_margin);

	bool line_fits(const std::string& text, uint32_t safety_margin) const;

	uint32_t quick_width(const UChar& c) const;
	uint32_t quick_width(const std::string& text) const;

	uint32_t wrapwidth_;
	bool draw_caret_;

	// TODO(GunChleoc): We can tie these to constexpr once the old font renderer is gone.
	const int fontsize_;
	RGBColor color_;

	// Editor font is sans bold.
	std::unique_ptr<RT::IFont> font_;

	std::vector<LineData> lines_;
	void highlight_selection(RenderTarget& dst,
	                         uint32_t scrollbar_position,
	                         uint32_t selection_start_line,
	                         uint32_t selection_start_x,
	                         uint32_t selection_end_line,
	                         uint32_t selection_end_x,
	                         const int fontheight,
	                         uint32_t line,
	                         const Vector2i& point) const;
	uint32_t line_index(int32_t y) const;
	ScopedTimer caret_timer_;
	uint32_t caret_ms;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_WORDWRAP_H
