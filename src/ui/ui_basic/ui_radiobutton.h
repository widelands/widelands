/*
 * Copyright (C) 2004 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef included_ui_radiobutton_h
#define included_ui_radiobutton_h

#include "types.h"
#include "ui_signal.h"

class UIPanel;

/**
 * A group of radiobuttons. At most one of them is checked at any time.
 * State is -1 if none is checked, otherwise it's the index of the checked button.
 */
class UIRadiobutton;

class UIRadiogroup {
	friend class UIRadiobutton;

public:
	UIRadiogroup();
	~UIRadiogroup();

	UISignal changed;
	UISignal1<int> changedto;
   UISignal clicked; // clicked without things changed

	int add_button(UIPanel* parent, int x, int y, uint picid = 0);

	inline int get_state() const { return m_state; }
	void set_state(int state);

private:
	UIRadiobutton*	m_buttons; // linked list of buttons (not sorted)
	int				m_highestid;
	int				m_state; // -1: none
};

#endif 
