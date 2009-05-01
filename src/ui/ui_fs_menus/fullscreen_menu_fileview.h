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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef FULLSCREEN_MENU_FILEVIEW_H
#define FULLSCREEN_MENU_FILEVIEW_H

#include "fullscreen_menu_base.h"

#include "ui_button.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

#include <string>
#include <cstring>

void fileview_window
	(UI::Panel                  & parent,
	 UI::UniqueWindow::Registry & reg,
	 std::string          const & filename);

/**
 * Shows a Text in a Fullscreen Menu. Waits for the button Ok to be clicked.
*/
struct Fullscreen_Menu_TextView : public Fullscreen_Menu_Base {
	Fullscreen_Menu_TextView(const std::string & filename);

protected:
	void set_text(const std::string & text);

private:
	UI::Textarea                                title;
	UI::Multiline_Textarea                      textview;
	UI::Callback_IDButton<Fullscreen_Menu_TextView, int32_t> close_button;
};

/**
 * Shows an ASCII-File in a Fullscreen Menu. Waits for the button Ok
 * to be clicked.
 */
struct Fullscreen_Menu_FileView : public Fullscreen_Menu_TextView
{Fullscreen_Menu_FileView(const std::string & filename);};



#endif
