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

#ifndef EVENT_MESSAGE_BOX_MESSAGE_BOX_H
#define EVENT_MESSAGE_BOX_MESSAGE_BOX_H

#include "ui_basic/window.h"

#include <vector>

namespace Widelands {
struct Event_Message_Box;
struct Game;
struct Trigger_Time;
};

struct Message_Box_Event_Message_Box : public UI::Window {
	Message_Box_Event_Message_Box
		(Widelands::Game &, Widelands::Event_Message_Box *,
		 int32_t, int32_t, int32_t, int32_t);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Widelands::Game                      & m_game;
	void clicked(int32_t);
	bool                                   m_is_modal;
	std::vector<Widelands::Trigger_Time *> m_trigger;
};

#endif
