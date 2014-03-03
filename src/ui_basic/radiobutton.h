/*
 * Copyright (C) 2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef UI_RADIOBUTTON_H
#define UI_RADIOBUTTON_H

#include <stdint.h>

#include "ui_basic/checkbox.h"
#include "point.h"

namespace UI {

struct Panel;

struct Radiogroup;

struct Radiobutton : public Statebox {
	friend struct Radiogroup;

	Radiobutton
		(Panel * parent, Point, const Image* pic, Radiogroup &, int32_t id);
	~Radiobutton();

	Radiobutton * next_button() {return m_nextbtn;}

private:
	void clicked() override;

	Radiobutton * m_nextbtn;
	Radiogroup  & m_group;
	int32_t           m_id;
};

/**
 * A group of radiobuttons. At most one of them is checked at any time.  State
 * is -1 if none is checked, otherwise it's the index of the checked button.
 */
struct Radiogroup {
	friend struct Radiobutton;

	Radiogroup();
	~Radiogroup();

	boost::signals2::signal<void ()> changed;
	boost::signals2::signal<void (int32_t)> changedto;
	boost::signals2::signal<void ()> clicked; //  clicked without things changed

	int32_t add_button
		(Panel * parent, Point, const Image* pic, const std::string& tooltip = "", Radiobutton ** = nullptr);

	int32_t get_state() const {return m_state;}
	void set_state(int32_t state);
	void set_enabled(bool);
	Radiobutton * get_first_button() {return m_buttons;}
private:
	Radiobutton * m_buttons; //  linked list of buttons (not sorted)
	int32_t           m_highestid;
	int32_t           m_state;   //  -1: none
};

}

#endif
