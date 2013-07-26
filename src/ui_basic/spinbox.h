/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#ifndef UI_SPINBOX_H
#define UI_SPINBOX_H

#include <cstring>

#include "align.h"
#include "ui_basic/button.h"
#include "graphic/graphic.h"

namespace UI {

struct SpinBoxImpl;
struct IntValueTextReplacement;
struct TextStyle;

/// A spinbox is an UI element for setting the integer value of a variable.
struct SpinBox : public Panel {
	SpinBox
		(Panel *,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 int32_t startval, int32_t minval, int32_t maxval,
		 const std::string & unit             = std::string(),
		 const Image* buttonbackground =
		 	g_gr->images().get("pics/but2.png"),
		 bool big = false,
		 Align align = Align_Center);
	~SpinBox();

	void setValue(int32_t);
	void setInterval(int32_t min, int32_t max);
	void setUnit(const std::string &);
	int32_t getValue();
	std::string getUnit();
	Align align() const;
	void setAlign(Align);
	void set_font(const std::string &, int32_t, RGBColor);
	void set_textstyle(const TextStyle & style);
	void add_replacement(int32_t, std::string);
	void remove_replacement(int32_t);
	bool has_replacement(int32_t);

private:
	void update();
	void changeValue(int32_t);
	int32_t findReplacement(int32_t value);

	const bool  m_big;

	SpinBoxImpl * sbi;
};

}

#endif
