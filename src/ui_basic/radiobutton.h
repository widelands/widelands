/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_RADIOBUTTON_H
#define WL_UI_BASIC_RADIOBUTTON_H

#include "base/vector.h"
#include "ui_basic/checkbox.h"

namespace UI {

class Panel;

struct Radiogroup;

struct Radiobutton : public Statebox {
	friend struct Radiogroup;

	Radiobutton(Panel* parent, PanelStyle, Vector2i, const Image* pic, Radiogroup&, int32_t id);
	~Radiobutton() override;

	Radiobutton* next_button() {
		return nextbtn_;
	}

private:
	void button_clicked() override;

	Radiobutton* nextbtn_;
	Radiogroup& group_;
	int32_t id_;
};

/**
 * A group of radiobuttons. At most one of them is checked at any time.  State
 * is -1 if none is checked, otherwise it's the index of the checked button.
 */
struct Radiogroup {
	friend struct Radiobutton;

	Radiogroup();
	~Radiogroup();

	Notifications::Signal<> changed;
	Notifications::Signal<int32_t> changedto;
	Notifications::Signal<> clicked;  //  clicked without things changed

	/**
	 * Text conventions: Sentence case for the 'tooltip'
	 */
	int32_t add_button(Panel* parent,
	                   PanelStyle,
	                   Vector2i,
	                   const Image* pic,
	                   const std::string& tooltip = "",
	                   Radiobutton** = nullptr);

	int32_t get_state() const {
		return state_;
	}
	void set_state(int32_t state, bool send_signal);
	void set_enabled(bool);
	Radiobutton* get_first_button() {
		return buttons_;
	}

private:
	Radiobutton* buttons_;  //  linked list of buttons (not sorted)
	int32_t highestid_;
	int32_t state_;  //  -1: none
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_RADIOBUTTON_H
