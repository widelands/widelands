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

#ifndef __S__EDITOR_EVENT_MENU_H
#define __S__EDITOR_EVENT_MENU_H

#include "ui_unique_window.h"

class Editor_Interactive;
class UIListselect;

/*
=============================

class Editor_Event_Menu

=============================
*/
class Editor_Event_Menu : public UIUniqueWindow {
   public:
      Editor_Event_Menu(Editor_Interactive*, UIUniqueWindowRegistry*);
      virtual ~Editor_Event_Menu();

   private:
      Editor_Interactive *m_parent;
		UIUniqueWindowRegistry		m_window_readme;

      UIListselect* m_event_list, *m_trigger_list;

      void clicked(int);
      void update();
};


#endif
