/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "fullscreen_menu_fileview.h"

#include "constants.h"
#include "filesystem.h"
#include "i18n.h"
#include "profile.h"


Fullscreen_Menu_TextView::Fullscreen_Menu_TextView(const std::string & filename)
:
Fullscreen_Menu_Base("fileviewmenu.jpg"),
title               (this, 50, 50, std::string(), Align_Left),
textview            (this, 30, 170, 735, 330),
close_button
	(this,
	 300, 545, 200, 26,
	 0,
	 &Fullscreen_Menu_TextView::end_modal, this, 0,
	 _("Close"))
{
	{
		i18n::Textdomain textdomain("texts");

		Profile prof(filename.c_str(), "global"); //  section-less file
		Section & section = prof.get_safe_section("global");

		title   .set_text(section.get_safe_string("title"));
		textview.set_text(section.get_safe_string("text"));
	}

	title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	title.set_pos(Point((get_inner_w() - title.get_w()) / 2, 100));

	textview.set_font(PROSA_FONT, PROSA_FONT_CLR_FG);
}

void Fullscreen_Menu_TextView::set_text(const std::string & text)
{textview.set_text(text);}

Fullscreen_Menu_FileView::Fullscreen_Menu_FileView(const std::string & filename)
: Fullscreen_Menu_TextView(filename)
{}


struct FileViewWindow : public UI::UniqueWindow {
	FileViewWindow
		(UI::Panel * parent,
		 UI::UniqueWindow::Registry * reg,
		 std::string filename);
private:
	UI::Multiline_Textarea textview;
};

FileViewWindow::FileViewWindow(UI::Panel* parent, UI::UniqueWindow::Registry* reg, std::string filename)
: UI::UniqueWindow(parent, reg, 0, 0, ""), textview(this, 0, 0, 560, 240)
{
	i18n::grab_textdomain("texts");

	Profile prof(filename.c_str(), "global"); // section-less file
	Section & s = prof.get_safe_section("global");
	i18n::release_textdomain();

	set_title(s.get_safe_string("title"));

	textview.set_text(s.get_safe_string("text"));
	textview.set_font(PROSA_FONT, PROSA_FONT_CLR_FG);

	set_inner_size(560, 240);

	if (get_usedefaultpos())
		center_to_parent();
}


/**
 * Display the contents of a text file in a scrollable window.
*/
void fileview_window(UI::Panel* parent, UI::UniqueWindow::Registry* reg, std::string filename)
{
	new FileViewWindow(parent, reg, filename);
}
