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

#ifndef EDITOR_TOOL_PLACE_IMMOVABLE_OPTIONS_MENU_H
#define EDITOR_TOOL_PLACE_IMMOVABLE_OPTIONS_MENU_H

#include "editor_tool_options_menu.h"

#include "ui_basic/tabpanel.h"

#include <vector>

struct Editor_Interactive;
struct Editor_Place_Immovable_Tool;
namespace UI {struct Checkbox;}

struct Editor_Tool_Place_Immovable_Options_Menu :
	public Editor_Tool_Options_Menu
{
	Editor_Tool_Place_Immovable_Options_Menu
		(Editor_Interactive          &,
		 Editor_Place_Immovable_Tool &,
		 UI::UniqueWindow::Registry  &);
	virtual ~Editor_Tool_Place_Immovable_Options_Menu();

private:
	std::vector<UI::Checkbox *> m_checkboxes;
	UI::Tab_Panel m_tabpanel;
	Editor_Place_Immovable_Tool & m_pit;
	void clicked(int32_t, bool);
};

#endif
