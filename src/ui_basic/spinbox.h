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

#ifndef WL_UI_BASIC_SPINBOX_H
#define WL_UI_BASIC_SPINBOX_H

#include <cstring>
#include <list>

#include "graphic/align.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "graphic/graphic.h"

namespace UI {

struct SpinBoxImpl;
struct IntValueTextReplacement;

/// A spinbox is an UI element for setting the integer value of a variable.
/// w is the overall width of the SpinBox and must be wide enough to fit 2 labels and the buttons.
/// unit_w is the width alotted for all buttons and the text between them (the actual spinbox).
/// label_text is a text that precedes the actual spinbox.
class SpinBox : public Panel {
public:
	enum class Type {
		kSmall,    // Displays buttons for small steps
		kBig,      // Displays buttons for small and big steps
		kValueList // Uses the values that are set by set_value_list().
	};

	enum class Units {
		kNone,
		kPixels,
		kMinutes,
		kPercent
	};

	SpinBox
		(Panel*,
		 int32_t x, int32_t y, uint32_t w, uint32_t unit_w,
		 int32_t startval, int32_t minval, int32_t maxval,
		 const std::string& label_text = std::string(),
		 const Units& unit = Units::kNone,
		 const Image* buttonbackground = g_gr->images().get("images/ui_basic/but3.png"),
		 SpinBox::Type = SpinBox::Type::kSmall,
		  // The amount by which units are increased/decreased for small and big steps when a button is pressed.
		 int32_t step_size = 1, int32_t big_step_size = 10);
	~SpinBox();

	void set_value(int32_t);
	// For spinboxes of type kValueList. The vector needs to be sorted in ascending order,
	// otherwise you will confuse the user.
	void set_value_list(const std::vector<int32_t>&);
	void set_interval(int32_t min, int32_t max);
	int32_t get_value() const;
	void add_replacement(int32_t, const std::string&);
	const std::vector<UI::Button*>& get_buttons() {return buttons_;}

private:
	void update();
	void change_value(int32_t);
	int32_t find_replacement(int32_t value) const;
	const std::string unit_text(int32_t value) const;

	const SpinBox::Type type_;
	SpinBoxImpl* sbi_;
	std::vector<UI::Button*> buttons_;
	UI::Box* box_;
};

}

#endif  // end of include guard: WL_UI_BASIC_SPINBOX_H
