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

#ifndef __S__EVENT_MESSAGE_BOX_OPTION_MENU_PIC_OPTIONS_H
#define __S__EVENT_MESSAGE_BOX_OPTION_MENU_PIC_OPTIONS_H

#include "ui_window.h"

class Editor_Interactive;
class Event_Message_Box;
class Event_Message_Box_Option_Menu;
class UICheckbox;
class UIEdit_Box;
class UIRadiogroup;
class UITextarea;

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Event_Message_Box_Option_Menu_Picture_Options : public UIWindow {
   public:
      Event_Message_Box_Option_Menu_Picture_Options(Editor_Interactive*, bool*, uint*, int*);
      ~Event_Message_Box_Option_Menu_Picture_Options();

      bool handle_mouseclick(uint btn, bool down, int mx, int my);

   private:
      void update(void);
      void clicked(int);

      UIPanel* m_parent;

      bool *m_clrkey_var;
      uint* m_picid_var;
      int* m_pos_var;
      int  m_picid;
      UICheckbox* m_clrkey;
      UIEdit_Box* m_filename;
      UIRadiogroup *m_radiogroup;
      UITextarea* m_pic_assigned;
};







#endif

