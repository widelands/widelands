/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef EDITOR_EVENT_MENU_H
#define EDITOR_EVENT_MENU_H

#include "ui/ui_basic/ui_unique_window.h"

struct Editor_Interactive;
namespace Widelands {
struct Event;
struct EventChain;
struct Trigger;
};
namespace UI {
template <typename T> struct Callback_Button;
template <typename T> struct Listselect;
};

struct Editor_Event_Menu : public UI::UniqueWindow {
	Editor_Event_Menu(Editor_Interactive &, UI::UniqueWindow::Registry *);

private:
	Editor_Interactive & eia();

	UI::Listselect<Widelands::Event      *> * m_event_list;
	UI::Listselect<Widelands::EventChain *> * m_eventchain_list;
	UI::Listselect<Widelands::Trigger    *> * m_trigger_list;
	UI::Callback_Button<Editor_Event_Menu>  * m_btn_del_event;
	UI::Callback_Button<Editor_Event_Menu>  * m_btn_edit_event;
	UI::Callback_Button<Editor_Event_Menu>  * m_btn_del_trigger;
	UI::Callback_Button<Editor_Event_Menu>  * m_btn_edit_trigger;
	UI::Callback_Button<Editor_Event_Menu>  * m_btn_del_eventchain;
	UI::Callback_Button<Editor_Event_Menu>  * m_btn_edit_eventchain;
	void trigger_list_selected    (uint32_t);
	void event_list_selected      (uint32_t);
	void eventchain_list_selected (uint32_t);
	void trigger_double_clicked   (uint32_t);
	void event_double_clicked     (uint32_t);
	void eventchain_double_clicked(uint32_t);
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
