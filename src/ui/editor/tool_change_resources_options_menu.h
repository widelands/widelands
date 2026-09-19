/*
 * Copyright (C) 2002-2026 by the Widelands Development Team
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

#ifndef WL_UI_EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU_H
#define WL_UI_EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU_H

#include "ui/basic/box.h"
#include "ui/basic/radiobutton.h"
#include "ui/basic/spinbox.h"
#include "ui/basic/textarea.h"
#include "ui/editor/tool_options_menu.h"

class EditorInteractive;
struct EditorIncreaseResourcesTool;

struct EditorToolChangeResourcesOptionsMenu : public EditorToolOptionsMenu {
	EditorToolChangeResourcesOptionsMenu(EditorInteractive&,
	                                     EditorIncreaseResourcesTool&,
	                                     UI::UniqueWindow::Registry&);

	void update_window() override;

	[[nodiscard]] bool uses_picker() const override {
		return true;
	}
	bool pick_from_field(const Widelands::Map& map,
	                     const Widelands::NodeAndTriangle<>& center,
	                     bool multiselect) override;

private:
	EditorInteractive& eia();
	void change_resource();
	void update_change_by();
	void update_set_to();
	void update();

	EditorIncreaseResourcesTool& increase_tool_;
	UI::Box box_;
	UI::SpinBox change_by_;
	UI::SpinBox set_to_;
	UI::Box resources_box_;
	UI::Radiogroup radiogroup_;
	UI::Textarea cur_selection_;
	UI::Button picker_;
};

#endif  // end of include guard: WL_UI_EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU_H
