/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#include "map.h"
#include "IntPlayer.h"
#include "fieldaction.h"
#include "watchwindow.h"

class FieldActionWindow : public Window {
public:
	FieldActionWindow(Interactive_Player *plr, UniqueWindow *registry);
	~FieldActionWindow();

	// Action handlers
	void act_watch();

private:
	void okdialog();
	
	void act(int action);

	Interactive_Player	*m_player;
	UniqueWindow			*m_registry;
	
	Coords m_field;
};

/*
===============
FieldActionWindow::FieldActionWindow

Initialize a field action window, creating the appropriate buttons.
===============
*/
FieldActionWindow::FieldActionWindow(Interactive_Player *plr, UniqueWindow *registry)
	: Window(plr, plr->get_w()/2, plr->get_h()/2, 68, 34, "Action")
{
	m_player = plr;
	m_field = m_player->get_fieldsel();
	m_registry = registry;

	if (registry->window)
		delete registry->window;
		
	registry->window = this;

	m_player->set_fieldsel_freeze(true);
	
	// Buttons
	Button *b;

	b = new Button(this, 0, 0, 34, 34, 2);
	b->clicked.set(this, &FieldActionWindow::act_watch);
	b->set_pic(g_fh.get_string("WATCH", 0));

	b = new Button(this, 34, 0, 34, 34, 2);

	// Move the window away from the current mouse position, i.e.
	// where the field is, to allow better view
	int mousex = get_mouse_x();
	int mousey = get_mouse_y();
	
	if (mousex >= 0 && mousex < get_w() &&
	    mousey >= 0 && mousey < get_h()) {
		if (mousey < get_h()/2)
			set_pos(get_x(), get_y()+get_h());
		else
			set_pos(get_x(), get_y()-get_h());
	}
	
	// Now force the mouse onto the first button
	// TODO: adapt this automatically as more buttons are added
	set_mouse_pos(17, 17);
}

/** FieldActionWindow::~FieldActionWindow()
 *
 * Free allocated resources, remove from registry.
 */
FieldActionWindow::~FieldActionWindow()
{
	m_player->set_fieldsel_freeze(false);
	m_registry->window = 0;
}

/*
===============
FieldActionWindow::okdialog

Call this from the button handlers.
It resets the mouse to its original position and closes the window
===============
*/
void FieldActionWindow::okdialog()
{
	m_player->warp_mouse_to_field(m_field);
	die();
}

/** FieldActionWindow::act_watch()
 *
 * Open a watch window for the given field and delete self.
 */
void FieldActionWindow::act_watch()
{
	show_watch_window(m_player, m_field.x, m_field.y);
	okdialog();
}

/*
===============
show_field_action

Bring up a field action window
===============
*/
void show_field_action(Interactive_Player *parent, UniqueWindow *registry)
{
	new FieldActionWindow(parent, registry);
}
