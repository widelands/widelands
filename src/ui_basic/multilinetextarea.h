/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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
		kNoScrolling,        // Expand the height instead of showing a scroll bar
		kScrollNormal,       // (default) only explicit scrolling
		kScrollNormalForced, // forced scrolling
		kScrollLog,          // follow the bottom of the text
		kScrollLogForced     // follow the bottom of the text, and forced
	};

	MultilineTextarea
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const std::string& text          = std::string(),
		 const Align                      = UI::Align::kLeft,
		 MultilineTextarea::ScrollMode scroll_mode = MultilineTextarea::ScrollMode::kScrollNormal);

	const std::string& get_text() const {return text_;}

	void set_text(const std::string&);

	uint32_t scrollbar_w() const {return 24;}
	uint32_t get_eff_w() const {return scrollbar_.is_enabled() ? get_w() - scrollbar_w() : get_w();}

	void set_color(RGBColor fg) {color_ = fg;}

	// Drawing and event handlers
	void draw(RenderTarget&) override;

	bool handle_mousewheel(uint32_t which, int32_t x, int32_t y) override;
	void scroll_to_top();

protected:
	void layout() override;

private:
	void recompute();
	void scrollpos_changed(int32_t pixels);

	std::string text_;
	RGBColor color_;
	Align align_;

	bool isrichtext;
	RichText rt;

	Scrollbar   scrollbar_;
	ScrollMode  scrollmode_;
};

}

#endif  // end of include guard: WL_UI_BASIC_MULTILINETEXTAREA_H
