/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_TOOLSIZE_MENU_H
#define WL_EDITOR_UI_MENUS_TOOLSIZE_MENU_H

#include "ui_basic/spinbox.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

/// The tool size window/menu.
struct EditorToolsizeMenu : public UI::UniqueWindow {
	EditorToolsizeMenu(EditorInteractive&, UI::UniqueWindow::Registry&);
	void update(uint32_t radius, uint32_t gap);
	void set_buttons_enabled(bool enable);
	[[nodiscard]] uint32_t radius() const {
		return radius_;
	}
	[[nodiscard]] uint32_t gap_percent() const {
		return gap_percent_;
	}

private:
	EditorInteractive& eia() const;

	UI::Box box_;
	UI::SpinBox spinbox_radius_;
	UI::SpinBox spinbox_gap_;

	uint32_t radius_{0U};
	uint16_t gap_percent_{0U};

	void changed();
	bool is_updating_{false};
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOLSIZE_MENU_H
