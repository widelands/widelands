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

#ifndef __S__EVENT_MESSAGE_BOX_OPTION_MENU_H
#define __S__EVENT_MESSAGE_BOX_OPTION_MENU_H

#include <string>
#include <vector>
#include "ui_window.h"

class Editor_Interactive;
class UIButton;
class Event_Message_Box;
class UITextarea;
class UIMultiline_Editbox;
class UIEdit_Box;
class UICheckbox;
class UIListselect;
class Trigger;

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Event_Message_Box_Option_Menu : public UIWindow {
   public:
      Event_Message_Box_Option_Menu(Editor_Interactive*, Event_Message_Box*);
      ~Event_Message_Box_Option_Menu();

      bool handle_mouseclick(uint btn, bool down, int mx, int my);

   private:
      static const int MAX_BUTTONS=4;

      void update(void);
      void clicked(int);
      void ls_selected(int);
      void edit_box_edited(int);

      Event_Message_Box* m_event;
      Editor_Interactive* m_parent;

      struct Button_Descr {
         std::string name;
         int trigger;
      } m_buttons[MAX_BUTTONS];

      UICheckbox* m_is_modal;
      UIEdit_Box* m_caption;
      UIEdit_Box* m_name;
      UIEdit_Box* m_window_title;
      UIEdit_Box* m_button_name;
      UITextarea* m_nr_buttons_ta;
      int         m_nr_buttons;
      int         m_ls_selected;
      UIMultiline_Editbox* m_text;
      UITextarea* m_current_trigger_ta;
      UIListselect* m_buttons_ls;

      std::vector<int> m_null_triggers;
};

#endif
