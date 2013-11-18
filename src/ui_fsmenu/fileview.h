/*
 * Copyright (C) 2002, 2006, 2008-2009 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef FULLSCREEN_MENU_FILEVIEW_H
#define FULLSCREEN_MENU_FILEVIEW_H

#include <cstring>
#include <string>

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

void fileview_window
	(UI::Panel                  & parent,
	 UI::UniqueWindow::Registry & reg,
	 const std::string          & filename);

/**
 * Shows a Text in a Fullscreen Menu. Can automatically handle Lua files and
 * plain text files. Waits for the button Ok to be clicked.
*/
struct Fullscreen_Menu_TextView : public Fullscreen_Menu_Base {
	Fullscreen_Menu_TextView(const std::string & filename);

protected:
	void set_text(const std::string & text);

private:
	UI::Textarea                                title;
	UI::Multiline_Textarea                      textview;
	UI::Button                     close_button;
};

/**
 * Shows an ASCII-File in a Fullscreen Menu. Waits for the button Ok
 * to be clicked.
 */
struct Fullscreen_Menu_FileView : public Fullscreen_Menu_TextView {
	Fullscreen_Menu_FileView(const std::string & filename);
};



#endif
