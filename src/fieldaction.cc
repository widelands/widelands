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
#include "game.h"
#include "map.h"
#include "IntPlayer.h"
#include "player.h"
#include "transport.h"
#include "fieldaction.h"
#include "watchwindow.h"


class FieldActionWindow : public Window {
public:
	FieldActionWindow(Interactive_Player *plr, UniqueWindow *registry);
	~FieldActionWindow();

	// Action handlers
	void act_watch();
	void act_buildflag();
	void act_ripflag();

private:
	void add_button(const char *name, void (FieldActionWindow::*fn)());
	void okdialog();
	
	Interactive_Player	*m_player;
	Map						*m_map;
	UniqueWindow			*m_registry;

	int		m_nbuttons;
	
	FCoords	m_field;
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
	m_map = m_player->get_game()->get_map();
	m_registry = registry;
	if (registry->window)
		delete registry->window;
	registry->window = this;
	
	Field *f = m_map->get_field(m_player->get_fieldsel());
	m_field = FCoords(m_player->get_fieldsel(), f);

	m_player->set_fieldsel_freeze(true);
	
	// Add actions
	std::vector<Map_Object*> objs;
	
	m_nbuttons = 0;
	
	if (m_field.field->get_owned_by() == m_player->get_player_number())
	{
		if (m_map->find_objects(m_field, 0, Map_Object::FLAG, &objs))
		{
			// Add flag actions
			Flag *flag = (Flag*)objs[0];

			//add_button("ROAD", &FieldActionWindow::act_buildroad);

			Building *building = flag->get_building();

			if (!building || strcasecmp(building->get_name(), "headquarters"))
				add_button("RIP", &FieldActionWindow::act_ripflag);
		}
		else
		{
			// Add build actions
			int buildcaps = m_player->get_player()->get_buildcaps(m_field);

			if (buildcaps & BUILDCAPS_FLAG)
				add_button("FLAG", &FieldActionWindow::act_buildflag);
		}
	}
	
	// Common to all fields
	add_button("WATCH", &FieldActionWindow::act_watch);

	set_inner_size(m_nbuttons*34, 34);

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

/*
===============
FieldActionWindow::~FieldActionWindow

Free allocated resources, remove from registry.
===============
*/
FieldActionWindow::~FieldActionWindow()
{
	m_player->set_fieldsel_freeze(false);
	m_registry->window = 0;
}


/*
===============
FieldActionWindow::add_button
===============
*/
void FieldActionWindow::add_button(const char *name, void (FieldActionWindow::*fn)())
{
	Button *b = new Button(this, m_nbuttons*34, 0, 34, 34, 2);
	b->clicked.set(this, fn);
	b->set_pic(g_fh.get_string(name, 0));
	
	m_nbuttons++;
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

/*
===============
FieldActionWindow::act_watch()

Open a watch window for the given field and delete self.
===============
*/
void FieldActionWindow::act_watch()
{
	show_watch_window(m_player, m_field.x, m_field.y);
	okdialog();
}

/*
===============
FieldActionWindow::act_buildflag

Build a flag at this field
===============
*/
void FieldActionWindow::act_buildflag()
{
	Game *g = m_player->get_game();
	
	g->send_player_command(m_player->get_player_number(), CMD_BUILD_FLAG, m_field.x, m_field.y);
	
	okdialog();
}

/*
===============
FieldActionWindow::act_ripflag

Remove the flag at this field
===============
*/
void FieldActionWindow::act_ripflag()
{
	Game *g = m_player->get_game();
	
	g->send_player_command(m_player->get_player_number(), CMD_RIP_FLAG, m_field.x, m_field.y);

	okdialog();
}

/*
===============
show_field_action

Perform a field action (other than building options).
Bring up a field action window or continue road building.
===============
*/
void show_field_action(Interactive_Player *parent, UniqueWindow *registry)
{
	new FieldActionWindow(parent, registry);
}
