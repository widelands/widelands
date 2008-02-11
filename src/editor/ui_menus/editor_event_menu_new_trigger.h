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

#ifndef EDITOR_EVENT_MENU_NEW_TRIGGER_H
#define EDITOR_EVENT_MENU_NEW_TRIGGER_H

#include "ui_window.h"

class Editor_Interactive;
namespace Widelands {namespace Trigger_Factory {struct Type_Descr;};};
namespace UI {
struct BaseListselect;
struct Multiline_Textarea;
template <typename T> struct Button;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Editor_Event_Menu_New_Trigger : public UI::Window {
	Editor_Event_Menu_New_Trigger(Editor_Interactive &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive & eia();
	void clicked_ok();
	void selected      (uint32_t);
	void double_clicked(uint32_t);

	UI::BaseListselect * m_trigger_type_list;
	UI::Multiline_Textarea                    * m_description;
	UI::Button<Editor_Event_Menu_New_Trigger> * m_ok_button;
};

#endif
