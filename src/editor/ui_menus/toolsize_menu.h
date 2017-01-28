/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_TOOLSIZE_MENU_H
#define WL_EDITOR_UI_MENUS_TOOLSIZE_MENU_H

#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

/// The tool size window/menu.
struct EditorToolsizeMenu : public UI::UniqueWindow {
	EditorToolsizeMenu(EditorInteractive&, UI::UniqueWindow::Registry&);
	void update(uint32_t);
	void set_buttons_enabled(bool enable);
	uint32_t value() {
		return value_;
	}

private:
	EditorInteractive& eia();
	void decrease_radius();
	void increase_radius();

	UI::Textarea textarea_;
	UI::Button increase_, decrease_;
	uint32_t value_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOLSIZE_MENU_H
