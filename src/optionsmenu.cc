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
#include "options.h"
#include "ui.h"
#include "menuecommon.h"
#include "optionsmenu.h"
#include "IntPlayer.h"

/*
==============================================================================

OptionsMenu

==============================================================================
*/

enum {
	om_cancel = 0,
	om_ok = 1
};

#define NUM_RESOLUTIONS		3

class OptionsMenu : public BaseMenu {
public:
	OptionsMenu();

	inline bool get_fullscreen() const { return m_fullscreen->get_state(); }
	inline bool get_inputgrab() const { return m_inputgrab->get_state(); }
	inline uint get_xres() const { return resolutions[m_resolution.get_state()].width; }
	inline uint get_yres() const { return resolutions[m_resolution.get_state()].height; }

private:
	Checkbox*	m_fullscreen;
	Checkbox*	m_inputgrab;
	Radiogroup	m_resolution;

	struct res {
		int width;
		int height;
	};
	static res resolutions[NUM_RESOLUTIONS];
};

OptionsMenu::res OptionsMenu::resolutions[NUM_RESOLUTIONS] = {
	{ 640, 480 },
	{ 800, 600 },
	{ 1024, 768 }
};

OptionsMenu::OptionsMenu()
	: BaseMenu("optionsmenu.jpg")
{
	Section *s = g_options.pull_section("global");

	// Menu title
	new Textarea(this, MENU_XRES/2, 140, "Options", Align_HCenter);

	// Buttons
	Button* b;

	b = new Button(this, 330, 420, 174, 24, 0, om_cancel);
	b->clickedid.set(this, &OptionsMenu::end_modal);
	b->set_title("Cancel");

	b = new Button(this, 136, 420, 174, 24, 2, om_ok);
	b->clickedid.set(this, &OptionsMenu::end_modal);
	b->set_title("Apply");

	// Fullscreen mode
	m_fullscreen = new Checkbox(this, 100, 180);
	m_fullscreen->set_state(s->get_bool("fullscreen", false));
	new Textarea(this, 125, 190, "Fullscreen", Align_VCenter);

	// input grab
	m_inputgrab = new Checkbox(this, 100, 205);
	m_inputgrab->set_state(s->get_bool("inputgrab", true));
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

		if (Interactive_Player::get_xres() == resolutions[i].width)
			m_resolution.set_state(i);
	}
	if (m_resolution.get_state() < 0)
		m_resolution.set_state(0);
}

/** options_menu()
 *
 * Display the options menu and apply settings if OK was pressed
 */
void options_menu()
{
	OptionsMenu *om = new OptionsMenu;
	int code = om->run();

	if (code == om_ok) {
		Section *s = g_options.pull_section("global");

		s->set_int("xres", om->get_xres());
		s->set_int("yres", om->get_yres());
		s->set_bool("fullscreen", om->get_fullscreen());
		s->set_bool("inputgrab", om->get_inputgrab());
		Sys_SetInputGrab(om->get_inputgrab());
	}

	delete om;
}
