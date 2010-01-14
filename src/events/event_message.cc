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

#include "event_message.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "profile/profile.h"

#include "upcast.h"

#define EVENT_VERSION 1

namespace Widelands {

Event_Message::Event_Message
	(char const * const Name, State const S)
	:
	Event                  (Name, S),
	m_expire_event         (0),
	m_status_change_trigger(0),
	m_duration             (Forever()),
	m_status               (New),
	m_player               (1),
	m_position             (Coords::Null())
{}


Event_Message::Event_Message
	(Section & s, Editor_Game_Base & egbase)
	:
	Event                  (s),
	m_expire_event         (0),
	m_status_change_trigger(0),
	m_duration             (Forever()),
	m_status               (New),
	m_player               (1),
	m_position             (Coords::Null())
{
	try {
		uint32_t const event_version = s.get_safe_positive("version");
		if (event_version <= EVENT_VERSION) {
			Map & map = egbase.map();
			m_player = s.get_Player_Number("player", map.get_nrplayers());
			if (char const * const evname = s.get_string("expire")) {
				if (upcast(Event_Expire_Message, ev, map.mem()[evname]))
					set_expire_event(ev);
				else
					throw game_data_error
						(_("expire: \"%s\" is not an expire message event"), evname);
			} else if (Section::Value const * const v = s.get_val("duration"))
				if ((m_duration = v->get_positive()) == Forever())
					throw game_data_error
						(_
						 	("duration: the value %u is not allowed as duration; it "
						 	 "is a specal value meaning forever, which is the "
						 	 "default; omit the duration key to make the message "
						 	 "exist forever"),
						 Forever());
			m_title    = s.get_safe_string("title");
			m_body     = s.get_safe_string("body");
			m_position = s.get_Coords("position", map.extent(), Coords::Null());
			if (char const * const status_string = s.get_string("status"))
				try {
					if      (not strcmp(status_string, "popup"))
						m_status = Popup;
					else if (not strcmp(status_string, "read"))
						m_status = Read;
					else if (not strcmp(status_string, "archived"))
						m_status = Archived;
					else
						throw game_data_error
							(_("expected %s but found \"%s\""),
							 "{read|archived}", status_string);
				} catch (_wexception const & e) {
					throw game_data_error("status: %s", e.what());
				}
			else if (char const * const trname = s.get_string("status_change")) {
				if
					(upcast
					 	(Trigger_Message_Is_Read_Or_Archived, tr, map.mtm()[trname]))
					set_status_change_trigger(tr);
				else
					throw game_data_error
						(_
						 	("status_change: \"%s\" is not a "
						 	 "message_is_read_or_archived trigger"),
						 trname);
			}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), event_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("(expire message): %s"), e.what());
	}
}

static char const * const status_names[] =
	{"new", "read", "archived", "popup"};

void Event_Message::Write
	(Section & s, Editor_Game_Base const &, Map_Map_Object_Saver const &) const
{
	s.set_string   ("type",          "message");
	s.set_int      ("version",       EVENT_VERSION);
	if (m_player != 1)
		s.set_int   ("player",        m_player);
	if (m_expire_event) {
		assert(m_duration == Forever());
		s.set_string("expire",        m_expire_event->name());
	} else if (m_duration != Forever())
		s.set_int   ("duration",      m_duration);
	if (m_status_change_trigger) {
		assert(m_status == New);
		s.set_string("status_change", m_status_change_trigger->name());
	}
	s.set_string   ("title",         m_title);
	s.set_string   ("body",          m_body);
	s.set_Coords   ("position",      m_position);
	if (m_status != New) //  The default status. Do not write.
		s.set_string("status",   status_names[m_status]);
}


Event::State Event_Message::run(Game & game) {
	assert(m_duration == Forever() or not m_expire_event);
	assert(m_status   == New       or not m_status_change_trigger);
	bool const popup = m_status == Popup;
	if (popup)
		m_status = New;
	Message_Id const message =
		game.player(m_player).add_message
			(game,
			 *new Message
			 	(identifier(),
			 	 game.get_gametime(),
			 	 m_duration,
			 	 m_title,
			 	 m_body,
			 	 m_position,
			 	 static_cast<Message::Status>(m_status)),
			 popup);
	if (m_expire_event)
		m_expire_event         ->set_message(m_player, message);
	if (m_status_change_trigger)
		m_status_change_trigger->set_message(m_player, message);

	return m_state = DONE;
}

}
