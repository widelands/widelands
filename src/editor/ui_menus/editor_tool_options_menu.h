/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __S__EDITOR_TOOL_OPTIONS_MENU
#define __S__EDITOR_TOOL_OPTIONS_MENU

#include "ui_unique_window.h"

class Editor_Interactive;

struct Editor_Tool_Options_Menu : public UI::UniqueWindow {
         Editor_Tool_Options_Menu(Editor_Interactive*, int index, UI::UniqueWindow::Registry*, const char* title);
         ~Editor_Tool_Options_Menu();

         inline Editor_Interactive* get_parent() { return m_parent; }
         void select_correct_tool(void) ;

      private:
         int m_index;
         Editor_Interactive* m_parent;
};

#endif
