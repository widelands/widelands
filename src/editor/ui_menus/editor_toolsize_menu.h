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

#ifndef EDITOR_TOOLSIZE_MENU_H
#define EDITOR_TOOLSIZE_MENU_H

#include "ui/ui_basic/ui_button.h"
#include "ui/ui_basic/ui_textarea.h"
#include "ui/ui_basic/ui_unique_window.h"


struct Editor_Interactive;


/// The tool size window/menu.
struct Editor_Toolsize_Menu : public UI::UniqueWindow {
	Editor_Toolsize_Menu(Editor_Interactive *, UI::UniqueWindow::Registry *);
	void update(uint32_t);

private:
	Editor_Interactive & eia();
	void decrease_radius();
	void increase_radius();

	UI::Textarea                                   m_textarea;
	UI::Callback_Button<Editor_Toolsize_Menu> m_increase, m_decrease;
};


#endif
