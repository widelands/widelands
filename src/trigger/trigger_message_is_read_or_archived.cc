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

#include "trigger_message_is_read_or_archived.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/message.h"
#include "logic/player.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "profile/profile.h"

namespace Widelands {

Trigger_Message_Is_Read_Or_Archived::Trigger_Message_Is_Read_Or_Archived
	(char const * const _name, bool const set)
	: Trigger(_name, set), player(0)
{}


void Trigger_Message_Is_Read_Or_Archived::Read
	(Section & s, Editor_Game_Base & egbase)
{
	try {
		if
			((player =
			  	s.get_Player_Number("player", egbase.map().get_nrplayers(), 0)))
		{
			if (Player const * const p = egbase.get_player(player)) {
				message = Message_Id(s.get_safe_positive("message"));

				//  The message queue should be continuous after loading it and
				//  before starting to run the simulation. Therefore we can check
				//  that a nonzero (as returned by Section::get_safe_positive) id
				//  is valid simply by comparing it to the queue's highest id
				//  (in constant time) instead of looking it up in the map (in
				//  logarithmic time).
				assert(p->messages().is_continuous());
				Message_Id const highest_id = p->messages().current_message_id();
				if (highest_id < message)
					throw game_data_error
						(_("player %u's message %u does not exist (%u is highest)"),
						 player, message.value(), highest_id.value());
			} else
				throw game_data_error(_("player %u does not exist"), player);
		}
	} catch (_wexception const & e) {
		throw game_data_error(_("(message_is_read_or_archived): %s"), e.what());
	}
}

void Trigger_Message_Is_Read_Or_Archived::Write
	(Section                    & s,
	 Editor_Game_Base     const & egbase,
	 Map_Map_Object_Saver const & mos)
	const
{
	s.set_string("type",    "message_is_read_or_archived");
	if (player) {
		assert(egbase.get_player(player));
		if (not egbase.player(player).messages()[message]) //  message expired
			return warn_when_message_expired(); //  must not write player and id
		s.set_int("player",  player);
		s.set_int("message", mos.message_savers[player - 1][message].value());
	}
}


void Trigger_Message_Is_Read_Or_Archived::check_set_conditions
	(Game const & game)
{
	//  \todo simplify this when Time is unsigned
	if (player) {
		if (Message const * const m = game.player(player).messages()[message]) {
			if (m->status() != Message::New)
				set_trigger(true);
		} else
			warn_when_message_expired();
	}
}


void Trigger_Message_Is_Read_Or_Archived::reset_trigger       (Game const &) {
	message = Message_Id::Null();
	player  = 0;

	set_trigger(false);
}

void Trigger_Message_Is_Read_Or_Archived::warn_when_message_expired() const {
	log
		("SCENARIO ERROR: \"%s\" (trigger message_is_read_or_archived) refers "
		 "to player %u's nonexistent message %u. It has expired.",
		 name().c_str(), player, message.value());
}

}
