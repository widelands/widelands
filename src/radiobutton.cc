/*
 * Copyright (C) 2002 Widelands Development Team
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

#include "widelands.h"
#include "ui.h"


/*
==============================================================================

Radiobutton

==============================================================================
*/

class Radiobutton : public Statebox {
	friend class Radiogroup;

public:
	Radiobutton(Panel *parent, int, int, Radiogroup *group, int id);
	~Radiobutton();

private:
	void clicked();

	Radiobutton *_nextbtn;
	Radiogroup *_group;
	int _id;
};

/** Radiobutton::Radiobutton(Panel *parent, int x, int y, Radiogroup *group, int id)
 *
 * Initialize the radiobutton and link it into the group's linked list
 */
Radiobutton::Radiobutton(Panel *parent, int x, int y, Radiogroup *group, int id)
	: Statebox(parent, x, y)
{
	_group = group;
	_id = id;

	_nextbtn = group->_buttons;
	group->_buttons = this;
}

/** Radiobutton::~Radiobutton()
 *
 * Unlink the radiobutton from its group
 */
Radiobutton::~Radiobutton()
{
	for(Radiobutton **pp = &_group->_buttons; *pp; pp = &(*pp)->_nextbtn) {
		if (*pp == this) {
			*pp = _nextbtn;
			break;
		}
	}
}

/** Radiobutton::clicked()
 *
 * Inform the radiogroup about the click; the group is responsible of setting
 * button states.
 */
void Radiobutton::clicked()
{
	if (!get_state())
		_group->set_state(_id);
}


/*
==============================================================================

Radiogroup

==============================================================================
*/

/** Radiogroup::Radiogroup()
 *
 * Initialize an empty radiogroup
 */
Radiogroup::Radiogroup()
{
	_buttons = 0;
	_highestid = -1;
	_state = -1;
}

/** Radiogroup::~Radiogroup()
 *
 * Free all associated buttons.
 */
Radiogroup::~Radiogroup()
{
	while(_buttons)
		delete _buttons;
}

/** Radiogroup::add_button(Panel *parent, int x, int y)
 *
 * Create a new radio button with the given attributes
 *
 * Returns: the ID of the new button
 */
int Radiogroup::add_button(Panel *parent, int x, int y)
{
	_highestid++;
	new Radiobutton(parent, x, y, this, _highestid);
	return _highestid;
}

/** Radiogroup::set_state(int state)
 *
 * Change the state and set button states to reflect the change.
 *
 * Args: state	the ID of the checked button (-1 means don't check any button)
 */
void Radiogroup::set_state(int state)
{
	if (state == _state)
		return;

	for(Radiobutton *btn = _buttons; btn; btn = btn->_nextbtn)
		btn->set_state(btn->_id == state);
	_state = state;
	changed.call();
	changedto.call(state);
}

/*
=============================

class Radiogroup_with_Buttons

This class is exactly the same logic as Radiogroup above, 
but it uses buttons insted of checkboxes as markers

============================
*/
class Radiobutton_Button : public Button {
	friend class Radiogroup_with_Buttons;

public:
	Radiobutton_Button(Panel *parent, int x, int y, int w, int h, bool flat, Radiogroup_with_Buttons *group, int bg, int id);
	~Radiobutton_Button();

private:
	void clicked();

	Radiobutton_Button *_nextbtn;
	Radiogroup_with_Buttons *_group;
	int _id;
};

/** Radiobutton_Button::Radiobutton_Button(Panel *parent, int x, int y, Radiogroup *group, int id)
 *
 * Initialize the radiobutton and link it into the group's linked list
 */
Radiobutton_Button::Radiobutton_Button(Panel *parent, int x, int y, int w, int h, bool flat, Radiogroup_with_Buttons *group, int bg, int id)
	: Button(parent, x, y, w, h, bg, id, true, flat)
{
	_group = group;
	_id = id;

	_nextbtn = group->_buttons;
	group->_buttons = this;

   Button::clicked.set(this, &Radiobutton_Button::clicked);
}

/** Radiobutton_Button::~Radiobutton_Button()
 *
 * Unlink the radiobutton from its group
 */
Radiobutton_Button::~Radiobutton_Button()
{
	for(Radiobutton_Button **pp = &_group->_buttons; *pp; pp = &(*pp)->_nextbtn) {
		if (*pp == this) {
			*pp = _nextbtn;
			break;
		}
	}
}

/** Radiobutton_Button::clicked()
 *
 * Inform the radiogroup about the click; the group is responsible of setting
 * button states.
 */
void Radiobutton_Button::clicked()
{
   if (!get_pressed())
		_group->set_state(_id);
}


/*
==============================================================================

Radiogroup_with_Buttons

==============================================================================
*/

/** Radiogroup_with_Buttons::Radiogroup_with_Buttons()
 *
 * Initialize an empty radiogroup
 */
Radiogroup_with_Buttons::Radiogroup_with_Buttons(int w, int h, bool flat)
{
	_buttons = 0;
	_highestid = -1;
	_state = -1;
   m_flat=flat;
   m_button_width=w;
   m_button_height=h;
}

/** Radiogroup_with_Buttons::~Radiogroup_with_Buttons()
 *
 * Free all associated buttons.
 */
Radiogroup_with_Buttons::~Radiogroup_with_Buttons()
{
	while(_buttons)
		delete _buttons;
}

/** Radiogroup_with_Buttons::add_button(Panel *parent, int x, int y)
 *
 * Create a new radio button with the given attributes
 *
 * Returns: the ID of the new button
 */
int Radiogroup_with_Buttons::add_button(Panel *parent, int x, int y, int bg, const char* text)
{
	_highestid++;
	Button* b=new Radiobutton_Button(parent, x, y, m_button_width, m_button_height, m_flat, this, bg, _highestid);
	b->set_title(text);
   return _highestid;
}
int Radiogroup_with_Buttons::add_button(Panel *parent, int x, int y, int bg, uint pic)
{
	_highestid++;
	Button* b=new Radiobutton_Button(parent, x, y, m_button_width, m_button_height, m_flat, this, bg, _highestid);
	b->set_pic(pic);
   return _highestid;
}

/** Radiogroup_with_Buttons::set_state(int state)
 *
 * Change the state and set button states to reflect the change.
 *
 * Args: state	the ID of the checked button (-1 means don't check any button)
 */
void Radiogroup_with_Buttons::set_state(int state)
{
	if (state == _state)
		return;

	for(Radiobutton_Button *btn = _buttons; btn; btn = btn->_nextbtn)
		btn->set_pressed(btn->_id == state);
	_state = state;
	changed.call();
	changedto.call(state);
}
