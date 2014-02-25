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

#ifndef UI_MULTILINE_TEXTAREA_H
#define UI_MULTILINE_TEXTAREA_H

#include "align.h"
#include "ui_basic/panel.h"
#include "rgbcolor.h"
#include "ui_basic/scrollbar.h"

namespace UI {
struct Scrollbar;

/**
 * This defines an area, where a text can easily be printed.
 * The textarea transparently handles explicit line-breaks and word wrapping.
 */
struct Multiline_Textarea : public Panel {
	enum ScrollMode {
		ScrollNormal = 0, ///< (default) only explicit or forced scrolling
		ScrollLog = 1,    ///< follow the bottom of the text
	};

	Multiline_Textarea
		(Panel * const parent,
		 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
		 const std::string & text         = std::string(),
		 const Align                      = Align_Left,
		 const bool always_show_scrollbar = false);
	~Multiline_Textarea();

	const std::string & get_text() const {return m_text;}
	ScrollMode get_scrollmode() const {return m_scrollmode;}

	void set_text(const std::string &);
	void set_scrollmode(ScrollMode mode);

	void set_font(std::string name, int32_t size, RGBColor fg);

	uint32_t scrollbar_w() const {return 24;}
	uint32_t get_eff_w() const {return get_w() - scrollbar_w();}

	void set_color(RGBColor fg) {m_fcolor = fg;}

	// Drawing and event handlers
	void draw(RenderTarget &) override;

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y) override;

	const char *  get_font_name() {return m_fontname.c_str();}
	int32_t       get_font_size() {return m_fontsize;}
	RGBColor &    get_font_clr () {return m_fcolor;}

private:
	struct Impl;

	std::unique_ptr<Impl> m;

	void recompute();
	void scrollpos_changed(int32_t pixels);

	std::string m_text;
	Scrollbar   m_scrollbar;
	ScrollMode  m_scrollmode;

protected:
	virtual void layout() override;

	Align        m_align;
	std::string  m_fontname;
	int32_t  m_fontsize;
	RGBColor m_fcolor;
};

}

#endif
