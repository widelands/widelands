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

#ifndef TRIGGER_NULL_OPTION_MENU_H
#define TRIGGER_NULL_OPTION_MENU_H

#include "ui_window.h"

class Editor_Interactive;
namespace Widelands {class Trigger_Null;};
namespace UI {struct EditBox;};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Trigger_Null_Option_Menu : public UI::Window {
	Trigger_Null_Option_Menu(Editor_Interactive &, Widelands::Trigger_Null &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive & eia();
	void clicked_ok();

	Widelands::Trigger_Null & m_trigger;
	UI::EditBox * m_name;
};

#endif
