/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_EDITOR_UI_MENUS_TOOL_TOOLHISTORY_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_TOOLHISTORY_OPTIONS_MENU_H

#include "editor/tools/tool_action.h"
#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/box.h"
#include "ui_basic/listselect.h"

class EditorInteractive;
struct EditorHistoryTool;

struct EditorToolhistoryOptionsMenu : public EditorToolOptionsMenu {
	EditorToolhistoryOptionsMenu(EditorInteractive&,
	                             EditorHistoryTool&,
	                             UI::UniqueWindow::Registry&);
	void update();

private:
	EditorInteractive& eia() const;
	EditorHistoryTool& history_tool_;

	void list_item_clicked(const std::string& selected);
	std::string make_tooltip(const ToolConf& conf, const std::string& title);
	void rebuild_list();

	int32_t margin_;
	int32_t box_width_;
	UI::Box box_;
	UI::Listselect<std::string> list_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_TOOLHISTORY_OPTIONS_MENU_H
