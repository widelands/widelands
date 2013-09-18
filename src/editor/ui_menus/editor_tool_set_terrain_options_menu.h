/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef EDITOR_TOOL_SET_TERRAIN_TOOL_H
#define EDITOR_TOOL_SET_TERRAIN_TOOL_H

#include <vector>

#include "editor/ui_menus/editor_tool_options_menu.h"
#include "ui_basic/textarea.h"


struct Editor_Interactive;
struct Editor_Set_Terrain_Tool;
namespace UI {struct Checkbox;}

struct Editor_Tool_Set_Terrain_Options_Menu : public Editor_Tool_Options_Menu {
	Editor_Tool_Set_Terrain_Options_Menu
		(Editor_Interactive         &,
		 Editor_Set_Terrain_Tool    &,
		 UI::UniqueWindow::Registry &);
	virtual ~Editor_Tool_Set_Terrain_Options_Menu();

private:
	// When we are using an offscreen surface, we have to delete it,
	// when we are closed. So we keep a pointer around.
	std::vector<const Image*>  offscreen_images_;
	UI::Textarea                m_cur_selection;
	Editor_Set_Terrain_Tool   & m_tool;
	void selected(int32_t, bool);
	std::vector<UI::Checkbox *> m_checkboxes;
	bool m_select_recursion_protect;
};


#endif
