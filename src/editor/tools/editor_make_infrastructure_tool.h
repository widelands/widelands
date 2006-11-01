/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__EDITOR_MAKE_INFRASTRUCTURE_TOOL_H
#define __S__EDITOR_MAKE_INFRASTRUCTURE_TOOL_H

#include "editor_tool.h"
#include "ui_unique_window.h"

/*
=============================
class Editor_Make_Infrastructure_Tool

this places immovables on the map
=============================
*/
class Editor_Make_Infrastructure_Tool : public Editor_Tool {
   public:
      Editor_Make_Infrastructure_Tool(void);
      ~Editor_Make_Infrastructure_Tool(void);

      void set_player(int n) { m_player=n; }
      int  get_player(void)  { return m_player; }

      virtual int handle_click_impl(FCoords&, Map*, Editor_Interactive*);
      virtual const char* get_fsel_impl(void) { return "pics/fsel.png"; } // Standart fsel icon, most complex tool of all

   private:
      int m_player;
      UI::UniqueWindow::Registry m_registry;
};

int Editor_Make_Infrastructure_Tool_Callback(const TCoords, void *, int);

#endif
