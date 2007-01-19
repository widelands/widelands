/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__EDITOR_TOOL_MENU_H
#define __S__EDITOR_TOOL_MENU_H

#include "editorinteractive.h"

#include "ui_radiobutton.h"
#include "ui_unique_window.h"

namespace UI {struct Radiogroup;};

/*
=============================

class Editor_Tool_Menu

This class is the tool selection window/menu.
Here, you can select the tool you wish to use the next time

=============================
*/
struct Editor_Tool_Menu : public UI::UniqueWindow {
      Editor_Tool_Menu(Editor_Interactive*, UI::UniqueWindow::Registry*, Editor_Interactive::Editor_Tools*, std::vector<UI::UniqueWindow::Registry>* );

   private:
      std::vector<UI::UniqueWindow::Registry>* m_options_menus;
      Editor_Interactive::Editor_Tools* m_tools;
      Editor_Interactive* m_parent;
	UI::Radiogroup m_radioselect;

      void changed_to(void);
};

#endif
