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
#include "singlepmenue.h"
#include "menuecommon.h"
#include "optionsmenu.h"


/*
==============================================================================

FileViewScreen

==============================================================================
*/

class FileViewScreen : public BaseMenu {
public:
	FileViewScreen(const char *text);
};

FileViewScreen::FileViewScreen(const char *text)
	: BaseMenu("fileviewmenu.bmp")
{
	// Text view
	new Multiline_Textarea(this, 40, 150, 560, 240, text);

	// Close button
	Button *b;

	b = new Button(this, 233, 420, 174, 24, 3);
	b->clickedid.set(this, &FileViewScreen::end_modal);
	b->set_title("Close");
}

/** fileview_screen(const char *fname)
 *
 * Display the contents of a text file in a menu screen
 */
static void fileview_screen(const char *fname)
{
	FileRead f;
	f.Open(g_fs, fname);
	FileViewScreen *fvs = new FileViewScreen((char*)f.Data(0,0));
	fvs->run();
	delete fvs;
}

/*
==============================================================================

MainMenu

==============================================================================
*/

enum {
	mm_singleplayer,
	//mm_multiplayer, // BIG TODO
	mm_options,
	mm_readme,
	mm_license,
	mm_exit
};

class MainMenu : public BaseMenu {
public:
	MainMenu();

	void not_supported();
};

MainMenu::MainMenu()
	: BaseMenu("mainmenu.bmp")
{
	// Buttons
	Button *b;

	b = new Button(this, 60, 150, 174, 24, 3, mm_singleplayer);
	b->clickedid.set(this, &MainMenu::end_modal);
	b->set_title("Single Player");

	b = new Button(this, 60, 190, 174, 24, 3);
	b->clicked.set(this, &MainMenu::not_supported);
	b->set_title("Multi Player");

	b = new Button(this, 60, 230, 174, 24, 3, mm_options);
	b->clickedid.set(this, &MainMenu::end_modal);
	b->set_title("Options");

	b = new Button(this, 60, 270, 174, 24, 3, mm_readme);
	b->clickedid.set(this, &MainMenu::end_modal);
	b->set_title("View Readme");

	b = new Button(this, 60, 310, 174, 24, 3, mm_license);
	b->clickedid.set(this, &MainMenu::end_modal);
	b->set_title("License");


	b = new Button(this, 60, 370, 174, 24, 0, mm_exit);
	b->clickedid.set(this, &MainMenu::end_modal);
	b->set_title("Exit Game");

	// Text
	new Textarea(this, MENU_XRES, MENU_YRES, "Version " VERSION, Align_BottomRight);
	new Textarea(this, 0, MENU_YRES, "(C) 2002 by the Widelands Development Team", Align_Bottom);
}

void MainMenu::not_supported()
{
	critical_error("This is not yet supported. You can savly click on continue.");
}

/** void main_menue(void)
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

		case mm_options:
			options_menu();
			break;

		case mm_readme:
			fileview_screen("README");
			break;

		case mm_license:
			fileview_screen("COPYING");
			break;

		default:
		case mm_exit:
			return;
		}
	}
}
