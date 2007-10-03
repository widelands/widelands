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

#ifndef __S__EVENT_MOVE_VIEW_OPTION_MENU_H
#define __S__EVENT_MOVE_VIEW_OPTION_MENU_H

#include "ui_window.h"

class Editor_Interactive;
class Event_Move_View;
namespace UI {
struct Edit_Box;
struct Textarea;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Event_Move_View_Option_Menu : public UI::Window {
      Event_Move_View_Option_Menu(Editor_Interactive*, Event_Move_View*);
      ~Event_Move_View_Option_Menu();

	bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);

private:
      void update();
	void clicked_ok();
      void clicked(int32_t);

      Event_Move_View* m_event;
      Editor_Interactive* m_parent;

      UI::Edit_Box* m_name;
	Coords m_location;
      UI::Textarea* m_x_ta, *m_y_ta;
};

#endif
