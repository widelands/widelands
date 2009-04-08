/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef EDITOR_MAIN_MENU_SAVE_MAP_MAKE_DIRECTORY_H
#define EDITOR_MAIN_MENU_SAVE_MAP_MAKE_DIRECTORY_H

#include "ui_window.h"

#include <string>
#include <cstring>

namespace UI {
struct EditBox;
template <typename T, typename ID> struct Callback_IDButton;
};

/**
 * Show a small modal dialog allowing the user to enter
 * a directory name to be created
 *
 * \todo This should be moved to src/ui, it's not specific to the editor
 */
struct Main_Menu_Save_Map_Make_Directory : public UI::Window {
	Main_Menu_Save_Map_Make_Directory(UI::Panel *, char const *);

	char const * get_dirname() {return m_dirname.c_str();}

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	std::string                                                m_dirname;
	UI::EditBox                                              * m_edit;
	UI::Callback_IDButton<Main_Menu_Save_Map_Make_Directory, int32_t> *
		m_ok_button;
	void edit_changed();
};

#endif
