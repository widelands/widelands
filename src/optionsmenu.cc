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

	inline bool get_fullscreen() const { return fullscreen->get_state(); }
	inline uint get_xres() const { return resolutions[resolution.get_state()].x; }
	inline uint get_yres() const { return resolutions[resolution.get_state()].y; }

private:
	Checkbox *fullscreen;
	Radiogroup resolution;

	struct res {
		uint x, y;
	};
	static res resolutions[NUM_RESOLUTIONS];
};

OptionsMenu::res OptionsMenu::resolutions[NUM_RESOLUTIONS] = {
	{ 640, 480 },
	{ 800, 600 },
	{ 1024, 768 }
};

OptionsMenu::OptionsMenu()
	: BaseMenu("splash.bmp")
{
	// Menu title
	new Textarea(this, MENU_XRES/2, 140, "Options", Textarea::H_CENTER);

	// Buttons
	Button* b;

	b = new Button(this, 330, 420, 174, 24, 0, om_cancel);
	b->clickedid.set(this, &OptionsMenu::end_modal);
	b->set_pic(g_fh.get_string("Cancel", 0));

	b = new Button(this, 136, 420, 174, 24, 2, om_ok);
	b->clickedid.set(this, &OptionsMenu::end_modal);
	b->set_pic(g_fh.get_string("Apply", 0));

	// Fullscreen mode
	fullscreen = new Checkbox(this, 100, 180);
	fullscreen->set_state(g_gr.get_mode() == Graphic::MODE_FS);
	new Textarea(this, 125, 190, "Fullscreen", Textarea::V_CENTER);

	// In-game resolution
	new Textarea(this, 100, 230, "In-game resolution", Textarea::V_CENTER);

	int y = 240;
	int i;
	for(i = 0; i < NUM_RESOLUTIONS; i++, y+= 25) {
		char buf[16];
		resolution.add_button(this, 100, y);
		sprintf(buf, "%ix%i", resolutions[i].x, resolutions[i].y);
		new Textarea(this, 125, y+10, buf, Textarea::V_CENTER);

		if (Interactive_Player::get_xres() == resolutions[i].x)
			resolution.set_state(i);
	}
	if (resolution.get_state() < 0)
		resolution.set_state(0);
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
		g_gr.set_mode(g_gr.get_xres(), g_gr.get_yres(), om->get_fullscreen() ? Graphic::MODE_FS : Graphic::MODE_WIN);
		Interactive_Player::set_resolution(om->get_xres(), om->get_yres());
	}

	delete om;
}
