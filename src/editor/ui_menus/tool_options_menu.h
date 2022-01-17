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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_EDITOR_UI_MENUS_TOOL_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_OPTIONS_MENU_H

#include "editor/editorinteractive.h"
#include "ui_basic/unique_window.h"

struct EditorToolOptionsMenu : public UI::UniqueWindow {
	EditorToolOptionsMenu(EditorInteractive& parent,
	                      UI::UniqueWindow::Registry&,
	                      const uint32_t width,
	                      const uint32_t height,
	                      const std::string& title,
	                      EditorTool& tool);

	/**
	 * Selects the correct tool from the parent.
	 * This is needed when a selection was made in the options menus.
	 */
	void select_correct_tool();

	uint32_t spacing() const {
		return 5;
	}
	uint32_t hspacing() const {
		return spacing();
	}
	uint32_t vspacing() const {
		return spacing();
	}
	uint32_t hmargin() const {
		return spacing();
	}
	uint32_t vmargin() const {
		return spacing();
	}

private:
	EditorInteractive& parent_;
	EditorTool& current_tool_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_OPTIONS_MENU_H
