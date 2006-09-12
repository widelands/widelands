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

#include "types.h"
#include "ui_checkbox.h"
#include "ui_radiobutton.h"

/*
==============================================================================

Radiobutton

==============================================================================
*/

class UIRadiobutton : public UIStatebox {
	friend class UIRadiogroup;

public:
	UIRadiobutton(UIPanel *parent, int x, int y, uint picid, UIRadiogroup *group, int id);
	~UIRadiobutton();

private:
	void clicked();

	UIRadiobutton*	m_nextbtn;
	UIRadiogroup*		m_group;
	int				m_id;
};

/**
Initialize the radiobutton and link it into the group's linked list
*/
UIRadiobutton::UIRadiobutton(UIPanel *parent, int x, int y, uint picid, UIRadiogroup *group, int id)
	: UIStatebox(parent, x, y, picid)
{
	m_group = group;
	m_id = id;

	m_nextbtn = group->m_buttons;
	group->m_buttons = this;
}

/**
 * Unlink the radiobutton from its group
 */
UIRadiobutton::~UIRadiobutton()
{
	for(UIRadiobutton **pp = &m_group->m_buttons; *pp; pp = &(*pp)->m_nextbtn) {
		if (*pp == this) {
			*pp = m_nextbtn;
			break;
		}
	}
}

/**
 * Inform the radiogroup about the click; the group is responsible of setting
 * button states.
 */
void UIRadiobutton::clicked()
{
   m_group->set_state(m_id);
	play_click();
}


/*
==============================================================================

Radiogroup

==============================================================================
*/

/**
 * Initialize an empty radiogroup
 */
UIRadiogroup::UIRadiogroup()
{
	m_buttons = 0;
	m_highestid = -1;
	m_state = -1;
}

/**
 * Free all associated buttons.
 */
UIRadiogroup::~UIRadiogroup()
{
	while(m_buttons)
		delete m_buttons;
}


/**
Create a new radio button with the given attributes
Returns the ID of the new button.
*/
int UIRadiogroup::add_button(UIPanel *parent, int x, int y, uint picid)
{
	m_highestid++;
	new UIRadiobutton(parent, x, y, picid, this, m_highestid);
	return m_highestid;
}


/**
 * Change the state and set button states to reflect the change.
 *
 * Args: state	the ID of the checked button (-1 means don't check any button)
 */
void UIRadiogroup::set_state(int state)
{
	if (state == m_state) {
      clicked.call();
		return;
   }

	for(UIRadiobutton *btn = m_buttons; btn; btn = btn->m_nextbtn)
		btn->set_state(btn->m_id == state);
	m_state = state;
	changed.call();
	changedto.call(state);
}
