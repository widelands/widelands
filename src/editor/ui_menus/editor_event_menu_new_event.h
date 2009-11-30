/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef EDITOR_EVENT_MENU_NEW_EVENT_H
#define EDITOR_EVENT_MENU_NEW_EVENT_H

#include "ui_basic/window.h"

struct Editor_Interactive;
namespace Widelands {struct Event_Descr;}
namespace UI {
template <typename T> struct Callback_Button;
struct BaseListselect;
struct Multiline_Textarea;
}

/**
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Editor_Event_Menu_New_Event : public UI::Window {
	Editor_Event_Menu_New_Event(Editor_Interactive &);

private:
	Editor_Interactive & eia();
	void clicked_ok();
	void selected      (uint32_t);
	void double_clicked(uint32_t);

	UI::BaseListselect * m_event_type_list;
	UI::Multiline_Textarea        * m_description;
	UI::Callback_Button<Editor_Event_Menu_New_Event> * m_ok_button;
};

#endif
