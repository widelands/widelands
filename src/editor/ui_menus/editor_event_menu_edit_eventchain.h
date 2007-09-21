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

#include <stdint.h>
#include "ui_window.h"

class Editor_Interactive;
class Event;
class EventChain;
namespace UI {
template <typename T> struct Listselect;
template <typename T> struct Button;
struct Checkbox;
struct Edit_Box;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Editor_Event_Menu_Edit_EventChain : public UI::Window {
      Editor_Event_Menu_Edit_EventChain(Editor_Interactive*, EventChain*);
      ~Editor_Event_Menu_Edit_EventChain();

	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);
      void think();

private:
	void clicked_ok                      ();
	void clicked_new_event               ();
	void clicked_edit_trigger_contitional();
	void clicked_ins_event               ();
	void clicked_del_event               ();
	void clicked_move_up                 ();
	void clicked_move_down               ();
	void tl_selected      (uint32_t);
	void tl_double_clicked(uint32_t);
	void cs_selected      (uint32_t);
	void cs_double_clicked(uint32_t);

	UI::Listselect<Event &> *m_available_events;
	UI::Listselect<Event &> *m_events;
      Editor_Interactive *m_parent;
	UI::Button<Editor_Event_Menu_Edit_EventChain> * m_insert_btn;
	UI::Button<Editor_Event_Menu_Edit_EventChain> * m_delete_btn;
	UI::Button<Editor_Event_Menu_Edit_EventChain> * m_mvup_btn;
	UI::Button<Editor_Event_Menu_Edit_EventChain> * m_mvdown_btn;
      UI::Checkbox         *m_morethanonce;
      UI::Edit_Box         *m_name;
      EventChain         *m_event_chain;
      bool                m_edit_trigcond;
};

#endif
