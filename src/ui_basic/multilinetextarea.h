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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_UI_BASIC_MULTILINETEXTAREA_H
#define WL_UI_BASIC_MULTILINETEXTAREA_H

#include <memory>

#include "graphic/align.h"
#include "graphic/styles/font_style.h"
#include "graphic/styles/panel_styles.h"
#include "graphic/text/rendered_text.h"
#include "ui_basic/panel.h"
#include "ui_basic/scrollbar.h"

namespace UI {

/**
 * This defines an area, where a text can easily be printed.
 * The textarea transparently handles explicit line-breaks and word wrapping.
 */
struct MultilineTextarea : public Panel {
	enum class ScrollMode {
		kNoScrolling,         // Expand the height instead of showing a scroll bar
		kScrollNormal,        // (default) only explicit scrolling
		kScrollNormalForced,  // forced scrolling
		kScrollLog,           // follow the bottom of the text
		kScrollLogForced      // follow the bottom of the text, and forced
	};

	MultilineTextarea(
	   Panel* const parent,
	   const int32_t x,
	   const int32_t y,
	   const uint32_t w,
	   const uint32_t h,
	   UI::PanelStyle style,
	   const std::string& text = std::string(),
	   const Align = UI::Align::kLeft,
	   MultilineTextarea::ScrollMode scroll_mode = MultilineTextarea::ScrollMode::kScrollNormal);

	const std::string& get_text() const {
		return text_;
	}

	void set_text(const std::string&);
	// int instead of uint because of overflow situations
	int32_t get_eff_w() const {
		return scrollbar_.is_enabled() ? get_w() - Scrollbar::kSize : get_w();
	}

	void set_style(FontStyle);
	void set_font_scale(float scale);

	// Drawing and event handlers
	void draw(RenderTarget&) override;

	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;
	bool handle_key(bool down, SDL_Keysym code) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	void scroll_to_top();

	void set_scrollmode(MultilineTextarea::ScrollMode scroll_mode);

protected:
	void layout() override;

private:
	void recompute();
	void scrollpos_changed(int32_t pixels);

	/**
	 * This prepares a non-richtext text for rendering. It escapes the source text and
	 * turns '\\n' into '<br>' tags as needed, then creates the richtext style wrappers.
	 */
	std::string make_richtext();
	std::string text_;
	std::string tooltip_before_hyperlink_tooltip_;

	std::shared_ptr<const UI::RenderedText> rendered_text_;
	Vector2i render_anchor_;

	FontStyle font_style_;
	const FontStyleInfo& font_style() const;
	float font_scale_;

	const Align align_;

	Scrollbar scrollbar_;
	ScrollMode scrollmode_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_MULTILINETEXTAREA_H
