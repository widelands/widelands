/*
 * Copyright (C) 2002 by Widelands Development Team
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
#include "fullscreen_menu_options.h"

/*
==============================================================================

Fullscreen_Menu_Options

==============================================================================
*/

Fullscreen_Menu_Options::res Fullscreen_Menu_Options::resolutions[NUM_RESOLUTIONS] = {
	{ 640, 480 },
	{ 800, 600 },
	{ 1024, 768 }
};

Fullscreen_Menu_Options::Fullscreen_Menu_Options(int cur_x, int cur_y, bool fullscreen, bool inputgrab)
	: Fullscreen_Menu_Base("optionsmenu.jpg")
{

	// Menu title
	new Textarea(this, MENU_XRES/2, 140, "Options", Align_HCenter);

	// Buttons
	Button* b;

	b = new Button(this, 330, 420, 174, 24, 0, om_cancel);
	b->clickedid.set(this, &Fullscreen_Menu_Options::end_modal);
	b->set_title("Cancel");

	b = new Button(this, 136, 420, 174, 24, 2, om_ok);
	b->clickedid.set(this, &Fullscreen_Menu_Options::end_modal);
	b->set_title("Apply");

	// Fullscreen mode
	m_fullscreen = new Checkbox(this, 100, 180);
	m_fullscreen->set_state(fullscreen);
	new Textarea(this, 125, 190, "Fullscreen", Align_VCenter);

	// input grab
	m_inputgrab = new Checkbox(this, 100, 205);
	m_inputgrab->set_state(inputgrab);
	new Textarea(this, 125, 215, "Grab Input", Align_VCenter);

	// In-game resolution
	new Textarea(this, 100, 255, "In-game resolution", Align_VCenter);

	int y = 265;
	int i;
	for(i = 0; i < NUM_RESOLUTIONS; i++, y+= 25) {
		char buf[16];
		m_resolution.add_button(this, 100, y);
		sprintf(buf, "%ix%i", resolutions[i].width, resolutions[i].height);
		new Textarea(this, 125, y+10, buf, Align_VCenter);

		if (cur_x == resolutions[i].width)
			m_resolution.set_state(i);
	}
	if (m_resolution.get_state() < 0)
		m_resolution.set_state(0);
}
