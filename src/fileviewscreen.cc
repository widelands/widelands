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
#include "menuecommon.h"
#include "fileviewscreen.h"

/*
==============================================================================

FileViewScreen

==============================================================================
*/

class FileViewScreen : public BaseMenu {
public:
	FileViewScreen(std::string title, std::string text);
};

FileViewScreen::FileViewScreen(std::string title, std::string text)
	: BaseMenu("fileviewmenu.jpg")
{
	// Text view
	new Multiline_Textarea(this, 40, 150, 560, 240, text.c_str());

   // Title
   Textarea* ta= new Textarea(this, 50, 50, title, Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 113);

	// Close button
	Button *b;

	b = new Button(this, 233, 420, 174, 24, 0);
	b->clickedid.set(this, &FileViewScreen::end_modal);
	b->set_title("Close");
}


/*
===============
textview_screen

Display the contents of a text in a menu screen
===============
*/
void textview_screen(std::string title, std::string text)
{
	FileViewScreen *fvs = new FileViewScreen(title, text);
	fvs->run();
	delete fvs;
}


/*
===============
fileview_screen

Display the contents of a text file in a menu screen
===============
*/
void fileview_screen(std::string title, std::string fname)
{
	FileRead f;
	f.Open(g_fs, fname);
	textview_screen(title, (const char*)f.Data(0,0));
}


/*
==============================================================================

TextViewWindow

==============================================================================
*/

class TextViewWindow : public UniqueWindow {
public:
	TextViewWindow(Panel* parent, UniqueWindowRegistry* reg, std::string title, std::string text);
};

TextViewWindow::TextViewWindow(Panel* parent, UniqueWindowRegistry* reg, std::string title, std::string text)
	: UniqueWindow(parent, reg, 0, 0, title)
{
	Multiline_Textarea* mt = new Multiline_Textarea(this, 0, 0, 560, 240, text.c_str());

	fit_inner(mt);

	if (get_usedefaultpos())
		center_to_parent();
}


/*
===============
textview_window

Display the text in a scrollable window.
===============
*/
void textview_window(Panel* parent, UniqueWindowRegistry* reg, std::string title, std::string text)
{
	new TextViewWindow(parent, reg, title, text);
}


/*
===============
fileview_screen

Display the contents of a text file in a scrollable window.
===============
*/
void fileview_window(Panel* parent, UniqueWindowRegistry* reg, std::string title, std::string fname)
{
	FileRead f;
	f.Open(g_fs, fname);
	textview_window(parent, reg, title, (const char*)f.Data(0,0));
}
