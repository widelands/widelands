/*
 * Copyright (C) 2002-2004, 2006-2008, 2010-2011 by the Widelands Development Team
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

#ifndef EDITOR_TOOL_PLACE_IMMOVABLE_OPTIONS_MENU_H
#define EDITOR_TOOL_PLACE_IMMOVABLE_OPTIONS_MENU_H

#include <memory>
#include <vector>

#include "editor/ui_menus/categorized_item_selection_menu.h"
#include "editor/ui_menus/editor_tool_options_menu.h"
#include "editor/tools/editor_place_immovable_tool.h"

struct Editor_Interactive;

struct Editor_Tool_Place_Immovable_Options_Menu : public Editor_Tool_Options_Menu {
	Editor_Tool_Place_Immovable_Options_Menu(Editor_Interactive&,
	                                         Editor_Place_Immovable_Tool&,
	                                         UI::UniqueWindow::Registry&);
	virtual ~Editor_Tool_Place_Immovable_Options_Menu();

private:
	std::unique_ptr<CategorizedItemSelectionMenu<Widelands::Immovable_Descr, Editor_Place_Immovable_Tool>>
	multi_select_menu_;
};

#endif
