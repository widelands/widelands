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

#ifndef __S__EDITOR_EVENT_MENU_NEW_EVENT_H
#define __S__EDITOR_EVENT_MENU_NEW_EVENT_H

#include "ui_window.h"

class Editor_Interactive;
class Event_Descr;
namespace UI {
template <typename T> struct Button;
template <typename T> struct Listselect;
struct Multiline_Textarea;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Editor_Event_Menu_New_Event : public UI::Window {
      Editor_Event_Menu_New_Event(Editor_Interactive*);
      ~Editor_Event_Menu_New_Event();

	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private:
	void clicked_ok();
	void selected      (uint);
	void double_clicked(uint);

	UI::Listselect<Event_Descr &> * m_event_list;
	UI::Multiline_Textarea        * m_description;
      Editor_Interactive* m_parent;
	UI::Button<Editor_Event_Menu_New_Event> * m_ok_button;
};

#endif
