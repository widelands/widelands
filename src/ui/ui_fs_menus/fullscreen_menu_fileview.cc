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

#include "constants.h"
#include "filesystem.h"
#include "fullscreen_menu_fileview.h"
#include "i18n.h"
#include "profile.h"
#include "ui_button.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

Fullscreen_Menu_TextView::Fullscreen_Menu_TextView(std::string filename)
	: Fullscreen_Menu_Base("fileviewmenu.jpg")
{


	i18n::grab_textdomain( "texts" );

   Profile prof(filename.c_str(), "global"); // section-less file
   Section* s = prof.get_section("global");

   std::string title = s->get_safe_string("title");
   std::string text = s->get_safe_string("text");
   i18n::release_textdomain();

   // Text view
	mlta=new UIMultiline_Textarea(this, 30, 170, 735, 330, text.c_str());
   mlta->set_font(PROSA_FONT, PROSA_FONT_CLR_FG);

   // Menu title
   UITextarea* ta= new UITextarea(this, 50, 50, title, Align_Left);
   ta->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 100);

	// Close button
	UIButton *b;

	b = new UIButton(this, 300, 545, 200, 26, 0);
	b->clickedid.set(this, &Fullscreen_Menu_TextView::end_modal);
	b->set_title(_("Close").c_str());
}

void Fullscreen_Menu_TextView::set_text(std::string text) {
   mlta->set_text(text.c_str());
}

Fullscreen_Menu_FileView::Fullscreen_Menu_FileView(std::string filename) :
   Fullscreen_Menu_TextView( filename ) {
   }

/*
==============================================================================

TextViewWindow

==============================================================================
*/

class FileViewWindow : public UIUniqueWindow {
public:
	FileViewWindow(UIPanel* parent, UIUniqueWindowRegistry* reg, std::string filename);
};

FileViewWindow::FileViewWindow(UIPanel* parent, UIUniqueWindowRegistry* reg, std::string filename)
	: UIUniqueWindow(parent, reg, 0, 0, "")
{
	i18n::grab_textdomain( "texts" );

   Profile prof(filename.c_str(), "global"); // section-less file
   Section* s = prof.get_section("global");
   i18n::release_textdomain();

   std::string title = s->get_safe_string("title");
   std::string text = s->get_safe_string("text");

   set_title(title.c_str());

	UIMultiline_Textarea* mlta=new UIMultiline_Textarea(this, 0, 0, 560, 240, text.c_str());
   mlta->set_font(PROSA_FONT, PROSA_FONT_CLR_FG);

	set_inner_size(560, 240);

	if (get_usedefaultpos())
		center_to_parent();
}


/*
===============
fileview_screen

Display the contents of a text file in a scrollable window.
===============
*/
void fileview_window(UIPanel* parent, UIUniqueWindowRegistry* reg, std::string filename)
{
   new FileViewWindow(parent, reg, filename);
}
