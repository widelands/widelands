/*
 * Copyright (C) 2002 by Holger Rapp
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

#include "input.h"
#include "cursor.h"
#include "graphic.h"
#include "ui.h"
#include "fileloc.h"
#include "font.h"
#include "setup.h"
#include "counter.h"
#include "output.h"
#include "singlepmenue.h"
#include "menuecommon.h"
#include "criterr.h"

#ifndef VERSION
#include "config.h"
#endif /* VERSION */

#include <string.h>

/*
==============================================================================

MainMenu

==============================================================================
*/

enum {
	mm_singleplayer,
	//mm_multiplayer, // BIG TODO
	//mm_options,
	//mm_readme,
	//mm_about,
	mm_exit
};

class MainMenu : public BaseMenu {
public:
	MainMenu();

	void not_supported();
};

MainMenu::MainMenu()
	: BaseMenu("splash.bmp")
{
	// Buttons
	Button *b;

	b = new Button(this, 60, 150, 174, 24, 1, mm_singleplayer);
	b->clickedid.set(this, &MainMenu::end_modal);
	b->set_pic(g_fh.get_string("Single Player", 0));

	b = new Button(this, 60, 190, 174, 24, 1);
	b->clicked.set(this, &MainMenu::not_supported);
	b->set_pic(g_fh.get_string("Multi Player", 0));

	b = new Button(this, 60, 230, 174, 24, 1);
	b->clicked.set(this, &MainMenu::not_supported);
	b->set_pic(g_fh.get_string("Options", 0));

	b = new Button(this, 60, 270, 174, 24, 1);
	b->clicked.set(this, &MainMenu::not_supported);
	b->set_pic(g_fh.get_string("View Readme", 0));

	b = new Button(this, 60, 310, 174, 24, 1);
	b->clicked.set(this, &MainMenu::not_supported);
	b->set_pic(g_fh.get_string("About", 0));


	b = new Button(this, 60, 370, 174, 24, 0, mm_exit);
	b->clickedid.set(this, &MainMenu::end_modal);
	b->set_pic(g_fh.get_string("Exit Game", 0));

	// Text
	new Textarea(this, MENU_XRES, MENU_YRES, "Version " VERSION, Textarea::BOTTOMRIGHT);
	new Textarea(this, 0, MENU_YRES, "(C) 2002 by the Widelands Development Team", Textarea::V_BOTTOM);
}

void MainMenu::not_supported()
{
	critical_error("This is not yet supported. You can savly click on continue.");
}

/** void main_menue(void);
 *
 * This functions runs the main menu. There, you can select
 * between different playmodes, exit and so on.
 *
 * This is mainly a wrapper of MainMenu creation, running and
 * deletion, so MainMenu doesn't have to be in a header file.
 */
void main_menue(void)
{
	for(;;) {
		MainMenu *mm = new MainMenu;
		int code = mm->run();
		delete mm;

		switch(code) {
		case mm_singleplayer:
			single_player_menue();
			break;

		default:
		case mm_exit:
			return;
		}
	}
}
