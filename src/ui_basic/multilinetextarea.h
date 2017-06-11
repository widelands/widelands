/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_MULTILINETEXTAREA_H
#define WL_UI_BASIC_MULTILINETEXTAREA_H

#include <memory>

#include "graphic/align.h"
#include "graphic/color.h"
#include "graphic/richtext.h"
#include "graphic/text_layout.h"
#include "ui_basic/panel.h"
#include "ui_basic/scrollbar.h"

namespace UI {
struct Scrollbar;

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
	   UI::Panel::Style style,
	   const std::string& text = std::string(),
	   const Align = UI::Align::kLeft,
	   MultilineTextarea::ScrollMode scroll_mode = MultilineTextarea::ScrollMode::kScrollNormal);

	const std::string& get_text() const {
		return text_;
	}

	void set_text(const std::string&);
	uint32_t get_eff_w() const {
		return scrollbar_.is_enabled() ? get_w() - Scrollbar::kSize : get_w();
	}

	void set_color(RGBColor fg) {
		color_ = fg;
	}

	// Most MultilineTextareas that contain richtext markup still use the old
	// font renderer, but some are already switched over the the new font
	// renderer. The markup is incompatible, so we need to be able to tell the
	// MultilineTextarea which one to use. MultilineTextareas without markup
	// automatically use the new font renderer.
	// TODO(GunChleoc): Remove this function once the switchover to the new font
	// renderer is complete.
	void force_new_renderer(bool force = true) {
		force_new_renderer_ = force;
	}

	// Drawing and event handlers
	void draw(RenderTarget&) override;

	bool handle_mousewheel(uint32_t which, int32_t x, int32_t y) override;
	void scroll_to_top();

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
	RGBColor color_;
	const Align align_;

	bool force_new_renderer_;
	bool use_old_renderer_;
	RichText rt;

	Scrollbar scrollbar_;
	ScrollMode scrollmode_;
};
}

#endif  // end of include guard: WL_UI_BASIC_MULTILINETEXTAREA_H
