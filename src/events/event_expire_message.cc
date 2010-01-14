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

#include "event_expire_message.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "profile/profile.h"

#include "upcast.h"

namespace Widelands {

Event_Expire_Message::Event_Expire_Message
	(char const * const Name, State const S)
	: Event(Name, S), player(0)
{}


Event_Expire_Message::Event_Expire_Message
	(Section & s, Editor_Game_Base & egbase)
	: Event(s)
{
	try {
		if
			((player =
			  	s.get_Player_Number("player", egbase.map().get_nrplayers(), 0)))
		{
			Player const * const p = egbase.get_player(player);
			if (not p)
				throw game_data_error(_("player %u does not exist"), player);

			message = Message_Id(s.get_safe_positive("message"));
			Message const * const m = p->messages()[message];

			if (not m)
				throw game_data_error
					(_("player %u's message %u does not exist"),
					 player, message.value());

			if (m->duration() != Forever())
				throw game_data_error
					(_
					 	("player %u's message %u has duration %u; (it must have "
					 	 "infinite duration, so that only this event can exipre "
					 	 "it)"),
					 player, message.value(), m->duration());

			Manager<Event> const & mem = egbase.map().mem();
			Manager<Event>::Index const nr_events = mem.size();
			for (Manager<Event>::Index i = 0; i < nr_events; ++i)
				if (upcast(Event_Expire_Message const, other, &mem[i]))
					if (other->player == player and other->message == message)
						throw game_data_error
							(_
							 	("refers to the same message id (player %u's %u) as "
							 	 "another expire message event (\"%s\"); (only one "
							 	 "event can expire each message)"),
							 player, message.value(), other->name().c_str());
		}
	} catch (_wexception const & e) {
		throw game_data_error(_("(expire message): %s"), e.what());
	}
}


void Event_Expire_Message::Write
	(Section                    & s,
	 Editor_Game_Base     const & egbase,
	 Map_Map_Object_Saver const & mos)
	const
{
	s.set_string("type",   "expire_message");
	if (player) {
		assert(message);
		assert(egbase.get_player(player));
		s.set_int("player",  player);
		s.set_int("message", mos.message_savers[player - 1][message].value());
	} else
		assert(not message);
}


Event::State Event_Expire_Message::run(Game & game) {
	if (player) {
		assert(message);
		assert(game.player(player).messages()[message]);
		game.player(player).messages().expire_message(message);
		message = Message_Id::Null();
		player  = 0;
	} else {
		assert(not message);
		log
			("SCENARIO ERROR: running event expire message \"%s\" without "
			 "message set!",
			 name().c_str());
	}

	return m_state = DONE;
}

}
