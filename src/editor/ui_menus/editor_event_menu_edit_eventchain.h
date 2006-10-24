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

#ifndef __S__EDITOR_EVENT_MENU_EDIT_EVENTCHAIN_H
#define __S__EDITOR_EVENT_MENU_EDIT_EVENTCHAIN_H

#include "ui_window.h"

class Editor_Interactive;
class UIListselect;
class UIButton;
class UICheckbox;
class UIEdit_Box;
class EventChain;

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Editor_Event_Menu_Edit_EventChain : public UIWindow {
   public:
      Editor_Event_Menu_Edit_EventChain(Editor_Interactive*, EventChain* );
      ~Editor_Event_Menu_Edit_EventChain();

      bool handle_mouseclick(const Uint8 btn, const bool down, int mx, int my);
      void think( void );

   private:
      void clicked(int);
      void tl_selected(int);
      void tl_double_clicked(int);
      void cs_selected(int);
      void cs_double_clicked(int);

      UIListselect       *m_available_events;
      UIListselect       *m_events;
      Editor_Interactive *m_parent;
      UIButton           *m_insert_btn;
      UIButton           *m_delete_btn;
      UIButton           *m_mvup_btn;
      UIButton           *m_mvdown_btn;
      UICheckbox         *m_morethanonce;
      UIEdit_Box         *m_name;
      EventChain         *m_event_chain;
      bool                m_edit_trigcond;
};

#endif
