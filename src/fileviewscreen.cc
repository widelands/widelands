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
#include "fileviewscreen.h"

/*
==============================================================================

FileViewScreen

==============================================================================
*/

FileViewScreen::FileViewScreen(const char* title, const char *text)
	: BaseMenu("fileviewmenu.bmp")
{
	// Text view
	new Multiline_Textarea(this, 40, 150, 560, 240, text);

   // Title
   Textarea* ta= new Textarea(this, 50, 50, title, Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 113);

	// Close button
	Button *b;

	b = new Button(this, 233, 420, 174, 24, 0);
	b->clickedid.set(this, &FileViewScreen::end_modal);
	b->set_title("Close");
}

/** textview_screen(const char *fname)
 *
 * Display the contents of a text in a menu screen
 */
void textview_screen(const char* title, const char *text)
{
	FileViewScreen *fvs = new FileViewScreen(title, text); 
	fvs->run();
	delete fvs;
}

/** fileview_screen(const char *fname)
 *
 * Display the contents of a text file in a menu screen
 */
void fileview_screen(const char* title, const char *fname)
{
	FileRead f;
	f.Open(g_fs, fname);
	FileViewScreen *fvs = new FileViewScreen(title, (char*)f.Data(0,0));
	fvs->run();
	delete fvs;
}
