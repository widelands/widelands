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
#include "graphic.h"
#include "ui.h"
#include "fullscreen_menu_base.h"
#include "fullscreen_menu_main.h"

/*
==============================================================================

Fullscreen_Menu_Main

==============================================================================
*/
Fullscreen_Menu_Main::Fullscreen_Menu_Main()
	: Fullscreen_Menu_Base("mainmenu.jpg")
{
	// Buttons
	Button *b;

	b = new Button(this, 60, 100, 174, 24, 3, mm_singleplayer);
	b->clickedid.set(this, &Fullscreen_Menu_Main::end_modal);
	b->set_title("Single Player");

	b = new Button(this, 60, 140, 174, 24, 3);
	b->clicked.set(this, &Fullscreen_Menu_Main::not_supported);
	b->set_title("Multi Player");

	b = new Button(this, 60, 180, 174, 24, 3, mm_options);
	b->clickedid.set(this, &Fullscreen_Menu_Main::end_modal);
	b->set_title("Options");

	b = new Button(this, 60, 220, 174, 24, 3, mm_editor);
	b->clickedid.set(this, &Fullscreen_Menu_Main::end_modal);
	b->set_title("Editor");

   b = new Button(this, 60, 260, 174, 24, 3, mm_readme);
	b->clickedid.set(this, &Fullscreen_Menu_Main::end_modal);
	b->set_title("View Readme");

	b = new Button(this, 60, 300, 174, 24, 3, mm_license);
	b->clickedid.set(this, &Fullscreen_Menu_Main::end_modal);
	b->set_title("License");


	b = new Button(this, 60, 370, 174, 24, 0, mm_exit);
	b->clickedid.set(this, &Fullscreen_Menu_Main::end_modal);
	b->set_title("Exit Game");

	// Text
	new Textarea(this, MENU_XRES-25, MENU_YRES-29, "Version " VERSION, Align_Right);
	new Textarea(this, 15, MENU_YRES-29, "(C) 2002-2003 by the Widelands Development Team", Align_TopLeft);
}

void Fullscreen_Menu_Main::not_supported()
{
	critical_error("This is not yet supported. You can safely click on continue.");
}
