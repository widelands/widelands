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
	Radiobutton(Panel *parent, int x, int y, uint picid, Radiogroup *group, int id);
	~Radiobutton();

private:
	void clicked();

	Radiobutton*	m_nextbtn;
	Radiogroup*		m_group;
	int				m_id;
};

/*
===============
Radiobutton::Radiobutton

Initialize the radiobutton and link it into the group's linked list
===============
*/
Radiobutton::Radiobutton(Panel *parent, int x, int y, uint picid, Radiogroup *group, int id)
	: Statebox(parent, x, y, picid)
{
	m_group = group;
	m_id = id;

	m_nextbtn = group->m_buttons;
	group->m_buttons = this;
}

/** Radiobutton::~Radiobutton()
 *
 * Unlink the radiobutton from its group
 */
Radiobutton::~Radiobutton()
{
	for(Radiobutton **pp = &m_group->m_buttons; *pp; pp = &(*pp)->m_nextbtn) {
		if (*pp == this) {
			*pp = m_nextbtn;
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
   m_group->set_state(m_id);
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
	m_buttons = 0;
	m_highestid = -1;
	m_state = -1;
}

/** Radiogroup::~Radiogroup()
 *
 * Free all associated buttons.
 */
Radiogroup::~Radiogroup()
{
	while(m_buttons)
		delete m_buttons;
}


/*
===============
Radiogroup::add_button

Create a new radio button with the given attributes
Returns the ID of the new button.
===============
*/
int Radiogroup::add_button(Panel *parent, int x, int y, uint picid)
{
	m_highestid++;
	new Radiobutton(parent, x, y, picid, this, m_highestid);
	return m_highestid;
}


/** Radiogroup::set_state(int state)
 *
 * Change the state and set button states to reflect the change.
 *
 * Args: state	the ID of the checked button (-1 means don't check any button)
 */
void Radiogroup::set_state(int state)
{
	if (state == m_state) {
      clicked.call();
		return;
   }

	for(Radiobutton *btn = m_buttons; btn; btn = btn->m_nextbtn)
		btn->set_state(btn->m_id == state);
	m_state = state;
	changed.call();
	changedto.call(state);
}

