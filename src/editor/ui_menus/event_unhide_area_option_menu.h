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

#ifndef EVENT_UNHIDE_AREA_OPTION_MENU_H
#define EVENT_UNHIDE_AREA_OPTION_MENU_H

#include "player_area.h"

#include "ui_window.h"

class Editor_Interactive;
namespace Widelands {struct Event_Unhide_Area;};

namespace UI {
struct EditBox;
struct Textarea;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Event_Unhide_Area_Option_Menu : public UI::Window {
	Event_Unhide_Area_Option_Menu
		(Editor_Interactive &, Widelands::Event_Unhide_Area &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive & eia();
	void update();
	void clicked_ok();
	void clicked(int32_t);

	Widelands::Event_Unhide_Area & m_event;
	UI::Textarea * m_player_ta, * m_area_ta, * m_x_ta, * m_y_ta;
	UI::EditBox * m_name;
	Widelands::Player_Area<>       m_player_area;
};

#endif
