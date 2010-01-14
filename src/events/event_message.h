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

#ifndef EVENT_MESSAGE_H
#define EVENT_MESSAGE_H

#include "event_expire_message.h"
#include "logic/message.h"
#include "trigger/trigger_message_is_read_or_archived.h"

namespace Widelands {

struct Editor_Game_Base;
struct Event_Expire_Message;

/// Creates a message for a player. There are 3 alternatives for the lifetime
/// of the message:
///   1. It exists forever.
///   2. It exists for a specified duration.
///   3. It exists until a particular Event_Expire_Message is run.
///
/// If the message will be created with status New (makes sense in most cases),
/// it is possible to define that a particular trigger should be set when the
/// player has changed the status of the message (to Read or Archived).
struct Event_Message :
	public Event,
	public Referencer<Event>,
	public Referencer<Trigger>
{
	Event_Message(char const * name, State);
	Event_Message(Section &, Editor_Game_Base &);
	~Event_Message() {set_expire_event(0); set_status_change_trigger(0);}

	std::string identifier() const {return "Event (message): " + name();}

	bool has_option_menu() const {return false;}
	int32_t option_menu(Editor_Interactive &) __attribute__ ((noreturn));

	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	State run(Game &);

	void reorigin(Coords const new_origin, Extent const extent) {
		m_position.reorigin(new_origin, extent);
	}

	void set_expire_event(Event_Expire_Message * const new_expire_event) {
		if (new_expire_event != m_expire_event) {
			if   (m_expire_event)
				Referencer<Event>::unreference  (*m_expire_event);
			if (new_expire_event)
				Referencer<Event>::  reference(*new_expire_event);
			m_expire_event = new_expire_event;
		}
	}
	Event_Expire_Message * expire_event() const {return m_expire_event;}

	void set_status_change_trigger
		(Trigger_Message_Is_Read_Or_Archived * const new_status_change_trigger)
	{
		if (new_status_change_trigger != m_status_change_trigger) {
			if   (m_status_change_trigger)
				Referencer<Trigger>::unreference  (*m_status_change_trigger);
			if (new_status_change_trigger)
				Referencer<Trigger>::  reference(*new_status_change_trigger);
			m_status_change_trigger = new_status_change_trigger;
		}
	}
	Trigger_Message_Is_Read_Or_Archived * status_change_trigger() const {
		return m_status_change_trigger;
	}

	void set_duration(Duration const d) {m_duration = d;}
	Duration          duration() const {return m_duration;}
	std::string const & title () const {return m_title;}
	void set_title(std::string const & t) {m_title = t;}
	std::string const & body  () const {return m_body;}
	void set_body (std::string const & b) {m_body = b;}

private:
	Event_Expire_Message                * m_expire_event;
	Trigger_Message_Is_Read_Or_Archived * m_status_change_trigger;
	Duration                              m_duration;
	enum {New, Read, Archived, Popup}     m_status;
	Player_Number                         m_player;
	Coords                                m_position;
	std::string                           m_title;
	std::string                           m_body;
};

}

#endif
