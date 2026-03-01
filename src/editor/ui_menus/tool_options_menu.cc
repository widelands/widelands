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

#include "editor/ui_menus/tool_options_menu.h"

EditorToolOptionsMenu::EditorToolOptionsMenu(EditorInteractive& parent,
                                             UI::UniqueWindow::Registry& registry,
                                             uint32_t const width,
                                             uint32_t const height,
                                             const std::string& title,
                                             EditorTool& tool)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "tool_options_menu", &registry, width, height, title),
     parent_(parent),
     current_tool_(tool) {
	select_correct_tool();
	clicked.connect([this] { select_correct_tool(); });
}

EditorToolOptionsMenu::~EditorToolOptionsMenu() {
	if (picker_is_active()) {
		deactivate_picker();
	}
	assert(!picker_is_active());
}

/**
 * Handle message menu hotkeys.
 */
bool EditorToolOptionsMenu::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		if (uses_picker() && matches_shortcut(KeyboardShortcut::kEditorPicker, code)) {
			toggle_picker();
			return true;
		}
	}
	return UI::UniqueWindow::handle_key(down, code);
}

void EditorToolOptionsMenu::select_correct_tool() {
	parent_.select_tool(current_tool_, EditorTool::First);
}

bool EditorToolOptionsMenu::picker_is_active() const {
	return parent_.tools() != nullptr && &parent_.tools()->current() == &parent_.tools()->picker &&
	       parent_.tools()->picker.get_linked_tool() == this;
}

void EditorToolOptionsMenu::activate_picker() {
	parent_.tools()->picker.set_linked_tool(this);
	parent_.select_tool(parent_.tools()->picker, EditorTool::First);
}

void EditorToolOptionsMenu::deactivate_picker() {
	assert(picker_is_active());
	parent_.tools()->picker.set_linked_tool(nullptr);
	select_correct_tool();
}

void EditorToolOptionsMenu::toggle_picker() {
	if (picker_is_active()) {
		deactivate_picker();
	} else {
		activate_picker();
	}
}
