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

#ifndef __S__EVENT_UNHIDE_AREA_OPTION_MENU_H
#define __S__EVENT_UNHIDE_AREA_OPTION_MENU_H

#include <string>
#include "ui_window.h"

class Editor_Interactive;
class Event_Unhide_Area;
class UIEdit_Box;
class UICheckbox;
class UITextarea;

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Event_Unhide_Area_Option_Menu : public UIWindow {
   public:
      Event_Unhide_Area_Option_Menu(Editor_Interactive*, Event_Unhide_Area*);
      ~Event_Unhide_Area_Option_Menu();

      bool handle_mouseclick(uint btn, bool down, int mx, int my);

   private:
      void update(void);
      void clicked(int);

      Event_Unhide_Area* m_event;
      Editor_Interactive* m_parent;
      UITextarea *m_player_ta, *m_area_ta, *m_x_ta, *m_y_ta;
      UIEdit_Box* m_name;
      int         m_x, m_y, m_player, m_area;
};

#endif
