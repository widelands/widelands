/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_EDITOR_TOOL_MENU_H
#define WL_EDITOR_UI_MENUS_EDITOR_TOOL_MENU_H

#include "editor/editorinteractive.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/unique_window.h"

/// The tool selection window/menu.
struct EditorToolMenu : public UI::UniqueWindow {
	EditorToolMenu(EditorInteractive &, UI::UniqueWindow::Registry &);

private:
	UI::Radiogroup radioselect_;

	void changed_to();
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_TOOL_MENU_H
