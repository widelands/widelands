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

#ifndef __S__EDITOR_EVENT_MENU_CHOOSE_TRIGGER_H
#define __S__EDITOR_EVENT_MENU_CHOOSE_TRIGGER_H

#include "ui_window.h"

class Editor_Interactive;
class UIListselect;
class UIButton;
class Event;
class Trigger;

/*
=============================

class Editor_Event_Menu_Choose_Trigger

=============================
*/
class Editor_Event_Menu_Choose_Trigger : public UIWindow {
   public:
      Editor_Event_Menu_Choose_Trigger(Editor_Interactive*, Event*);
      virtual ~Editor_Event_Menu_Choose_Trigger();

      bool handle_mouseclick(uint btn, bool down, int mx, int my);


   private:
      struct Trigger_Data {
         Trigger*      trig;
         bool          run_enabled;
      };

      Editor_Interactive *m_parent;
      UIListselect* m_available, *m_selected;
      UIButton *m_btn_ltor, *m_btn_rtol, *m_btn_toggle_event;
      Event* m_event;
      void update(void);
      void available_list_selected(int);
      void selected_list_selected(int);
      void selected_list_double_clicked(int);
      void available_list_double_clicked(int);
      void clicked(int);
};


#endif
