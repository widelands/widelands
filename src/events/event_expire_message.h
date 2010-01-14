/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef EVENT_EXPIRE_MESSAGE_H
#define EVENT_EXPIRE_MESSAGE_H

#include "event.h"
#include "logic/message_id.h"

namespace Widelands {

struct Editor_Game_Base;

/// Expires a message for a player. A player number and message id should be
/// set for this event when a message is created with an Event_Message.
///
/// Assumes that the message that is set for this expire event still exists
/// when the expire event is run. This is guaranteed by Event_Message, which
/// does not create a message with both a (finite) constant duration and an
/// expire event. Nor does it create several expire events for the same
/// message.
///
/// \note Stores a message id, which must be translated when saving to file in
/// the Write function. This is done with a Map_Message_Saver. There is one
/// such object for each player in Map_Map_Object_Saver. This will of course
/// only work if messages have been written before events are written.
struct Event_Expire_Message : public Event {
	Event_Expire_Message(char const * name, State);
	Event_Expire_Message(Section &, Editor_Game_Base &);

	std::string identifier() const {return "Event (expire message): " + name();}

	int32_t option_menu(Editor_Interactive &);

	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	State run(Game &);

	void set_message(Player_Number const p, Message_Id const m) {
		player  = p;
		message = m;
	}

private:
	Player_Number player;
	Message_Id    message;
};

}

#endif
