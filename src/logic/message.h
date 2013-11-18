/*
 * Copyright (C) 2002-2010 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <boost/signals2.hpp>

#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct Message {
	enum Status {New, Read, Archived};
	/**
	 * A new message to be displayed to the player
	 * \param msgsender The message sender
	 * \param sent_time The (game) time at which the message is sent
	 * \param d The duration after which the message will expire
	 * \param t The message title
	 * \param b The message body
	 * \param c The message coords. The player will be able to taken there.
	 * Defaults to Coords::Null()
	 * \param ser A Map_Object serial. If non null, the message will expire once
	 * the object is removed from the game. Defaults to 0
	 * \param s The message status. Defaults to Status::New
	 */
	Message
		(const std::string &       msgsender,
		 uint32_t                  sent_time,
		 Duration                  d,
		 const std::string &       t,
		 const std::string &       b,
		 Widelands::Coords   const c = Coords::Null(),
		 Widelands::Serial         ser = 0,
		 Status                    s = New)
		:
		m_sender(msgsender),
		m_title(t),
		m_body    (b),
		m_sent    (sent_time),
		m_duration(d),
		m_position(c),
		m_serial  (ser),
		m_status  (s)
	{}

	const std::string & sender() const     {return m_sender;}
	uint32_t            sent    () const            {return m_sent;}
	Duration            duration() const            {return m_duration;}
	const std::string & title() const      {return m_title;}
	const std::string & body () const               {return m_body;}
	Widelands::Coords   position() const            {return m_position;}
	Widelands::Serial   serial() const              {return m_serial;}
	Status              status  () const {return m_status;}
	Status set_status(Status const s) {return m_status = s;}

private:
	std::string m_sender;
	std::string m_title;
	std::string       m_body;
	uint32_t          m_sent;
	Duration          m_duration; /// will expire after this duration
	Widelands::Coords m_position;
	Widelands::Serial m_serial; // serial to map object
	Status            m_status;
};

}

#endif
