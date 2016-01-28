/*
 * Copyright (C) 2002, 2006-2009, 2011 by the Widelands Development Team
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
	enum ScrollMode {
		ScrollNormal = 0, ///< (default) only explicit or forced scrolling
		ScrollLog = 1,    ///< follow the bottom of the text
	};

	MultilineTextarea
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const std::string& text          = std::string(),
		 const Align                      = Align_Left,
		 const bool always_show_scrollbar = false);

	const std::string& get_text() const {return m_text;}
	ScrollMode get_scrollmode() const {return m_scrollmode;}

	void set_text(const std::string&);
	void set_scrollmode(ScrollMode mode);

	uint32_t scrollbar_w() const {return 24;}
	uint32_t get_eff_w() const {return m_scrollbar.is_enabled() ? get_w() - scrollbar_w() : get_w();}

	void set_color(RGBColor fg) {m_style.fg = fg;}

	// Drawing and event handlers
	void draw(RenderTarget&) override;

	bool handle_mousewheel(uint32_t which, int32_t x, int32_t y) override;
	void scroll_to_top();

protected:
	void layout() override;

private:
	void recompute();
	void scrollpos_changed(int32_t pixels);

	std::string m_text;
	UI::TextStyle m_style;
	Align m_align;

	bool isrichtext;
	RichText rt;

	Scrollbar   m_scrollbar;
	ScrollMode  m_scrollmode;
};

}

#endif  // end of include guard: WL_UI_BASIC_MULTILINETEXTAREA_H
