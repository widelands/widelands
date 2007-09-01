/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
class Event;
class EventChain;
class Trigger;
namespace UI {
template <typename T> struct Button;
template <typename T> struct Listselect;
};

struct Editor_Event_Menu : public UI::UniqueWindow {
      Editor_Event_Menu(Editor_Interactive*, UI::UniqueWindow::Registry*);
      virtual ~Editor_Event_Menu();

private:
      Editor_Interactive *m_parent;

	UI::Listselect<Event*      > *m_event_list;
	UI::Listselect<EventChain* > *m_eventchain_list;
	UI::Listselect<Trigger*    > *m_trigger_list;
	UI::Button<Editor_Event_Menu> * m_btn_del_event;
	UI::Button<Editor_Event_Menu> * m_btn_edit_event;
	UI::Button<Editor_Event_Menu> * m_btn_del_trigger;
	UI::Button<Editor_Event_Menu> * m_btn_edit_trigger;
	UI::Button<Editor_Event_Menu> * m_btn_del_eventchain;
	UI::Button<Editor_Event_Menu> * m_btn_edit_eventchain;
	void trigger_list_selected    (uint);
	void event_list_selected      (uint);
	void eventchain_list_selected (uint);
	void trigger_double_clicked   (uint);
	void event_double_clicked     (uint);
	void eventchain_double_clicked(uint);
	void clicked_new_event      ();
	void clicked_del_event      ();
	void clicked_edit_event     ();
	void clicked_new_trigger    ();
	void clicked_del_trigger    ();
	void clicked_edit_trigger   ();
	void clicked_new_eventchain ();
	void clicked_del_eventchain ();
	void clicked_edit_eventchain();
      void update();
};


#endif
