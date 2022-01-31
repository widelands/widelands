/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_FIELD_INFO_WINDOW_H
#define WL_EDITOR_UI_MENUS_FIELD_INFO_WINDOW_H

#include "editor/editorinteractive.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/unique_window.h"

struct FieldInfoWindow : public UI::UniqueWindow {
	static constexpr int16_t width = 400;
	static constexpr int16_t total_width = width + 2 * UI::Window::kVerticalBorderThickness;
	static constexpr int16_t height = 200;
	static constexpr int16_t total_height =
	   height + UI::Window::kTopBorderThickness + UI::Window::kBottomBorderThickness;
	FieldInfoWindow(EditorInteractive& parent,
	                UI::UniqueWindow::Registry&,
	                int32_t x,
	                int32_t y,
	                const Widelands::NodeAndTriangle<>& center,
	                const Widelands::Field& f,
	                const Widelands::Field& tf,
	                Widelands::Map* map);

protected:
	void think() override;

private:
	void update();

	void add_node_info(std::string& buf) const;
	void add_caps_info(std::string& buf) const;
	void add_owner_info(std::string& buf) const;
	void add_terrain_info(std::string& buf) const;
	void add_mapobject_info(std::string& buf) const;
	void add_resources_info(std::string& buf) const;
	void add_map_info(std::string& buf) const;
	EditorInteractive& parent_;
	const Widelands::NodeAndTriangle<> center_;
	const Widelands::Field& f_;
	const Widelands::Field& tf_;
	Widelands::Map* map_;
	UI::MultilineTextarea multiline_textarea_;
	Time lastupdate_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_FIELD_INFO_WINDOW_H
