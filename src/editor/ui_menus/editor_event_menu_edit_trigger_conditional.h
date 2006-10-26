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

#ifndef __S__EDITOR_EVENT_MENU_EDIT_TRIGGERCONDITIONAL_H
#define __S__EDITOR_EVENT_MENU_EDIT_TRIGGERCONDITIONAL_H

#include "trigger_conditional.h"
#include "ui_window.h"

class Editor_Interactive;
class EventChain;
template <typename T> struct UIListselect;
class UIButton;
class TriggerConditional;

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Editor_Event_Menu_Edit_TriggerConditional : public UIWindow {
   public:
      Editor_Event_Menu_Edit_TriggerConditional(Editor_Interactive*, TriggerConditional*, EventChain*  );
      ~Editor_Event_Menu_Edit_TriggerConditional();

      bool handle_mouseclick(const Uint8 btn, const bool down, int mx, int my);
      TriggerConditional* get_trigcond( void ) { return m_given_cond; }

   private:
	void clicked_cancel     ();
	void clicked_ok         ();
	void clicked_operator   (int i);
	void clicked_ins_trigger();
	void clicked_delete     ();
	void clicked_move_up    ();
	void clicked_move_down  ();
      void tl_selected(int);
      void tl_double_clicked(int);
      void cs_selected(int);
      void cs_double_clicked(int);

	UIListselect<Trigger &> *m_trigger_list;
	UIListselect<TriggerConditional_Factory::Token &> *m_construction;
      Editor_Interactive *m_parent;
      UIButton           *m_insert_btn;
      UIButton           *m_delete_btn;
      UIButton           *m_mvup_btn;
      UIButton           *m_mvdown_btn;
      TriggerConditional *m_given_cond;
      EventChain         *m_event_chain;
};

#endif
