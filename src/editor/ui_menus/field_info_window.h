/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_TOOL_INFO_WINDOW_H
#define WL_EDITOR_UI_MENUS_TOOL_INFO_WINDOW_H

#include "editor/editorinteractive.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"

struct FieldInfoWindow : public UI::UniqueWindow {
	FieldInfoWindow(EditorInteractive& parent,
	                UI::UniqueWindow::Registry&,
	                int32_t x,
	                int32_t y,
	                const Widelands::NodeAndTriangle<>& center,
	                const Widelands::Field& f,
	                const Widelands::Field& tf,
	                Widelands::Map* map);

private:
	void add_node_info(std::string& buf);
	void add_caps_info(std::string& buf);
	void add_owner_info(std::string& buf);
	void add_terrain_info(std::string& buf);
	void add_mapobject_info(std::string& buf);
	void add_resources_info(std::string& buf);
	void add_map_info(std::string& buf);
	EditorInteractive& parent_;
	const Widelands::NodeAndTriangle<>& center_;
	const Widelands::Field& f_;
	const Widelands::Field& tf_;
	Widelands::Map* map_;
	UI::MultilineTextarea multiline_textarea_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_INFO_WINDOW_H
