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

#ifndef __S__EDITOR_TOOL_MENU_H
#define __S__EDITOR_TOOL_MENU_H

#include "ui_unique_window.h"
#include "editorinteractive.h"

class UIRadiogroup;

/*
=============================

class Editor_Tool_Menu

This class is the tool selection window/menu.
Here, you can select the tool you wish to use the next time

=============================
*/
class Editor_Tool_Menu : public UIUniqueWindow {
   public:
      Editor_Tool_Menu(Editor_Interactive*, UIUniqueWindowRegistry*, Editor_Interactive::Editor_Tools*);
      virtual ~Editor_Tool_Menu();

   private:
      UIUniqueWindowRegistry m_options;

      Editor_Interactive::Editor_Tools* m_tools;
      Editor_Interactive* m_parent;
      UIRadiogroup* m_radioselect;

      void changed_to(void);
};

#endif
