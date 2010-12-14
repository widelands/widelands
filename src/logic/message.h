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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include "widelands.h"
#include "widelands_geometry.h"

#include <string>

namespace Widelands {

struct Message {
	enum Status {New, Read, Archived};
	Message
		(std::string const &       msgsender,
		 uint32_t                  sent_time,
		 Duration                  d,
		 std::string const &       t,
		 std::string const &       b,
		 Widelands::Coords   const c = Coords::Null(),
		 Status                    s = New)
		:
		m_sender(msgsender),
		m_title(t),
		m_body    (b),
		m_sent    (sent_time),
		m_duration(d),
		m_position(c),
		m_status  (s)
	{}

	const std::string & sender() const throw ()     {return m_sender;}
	uint32_t            sent    () const            {return m_sent;}
	Duration            duration() const            {return m_duration;}
	const std::string & title() const throw ()      {return m_title;}
	std::string const & body () const               {return m_body;}
	Widelands::Coords   position() const            {return m_position;}
	Status              status  () const {return m_status;}
	Status set_status(Status const s) {return m_status = s;}

private:
	std::string m_sender;
	std::string m_title;
	std::string       m_body;
	uint32_t          m_sent;
	Duration          m_duration; /// will expire after this duration
	Widelands::Coords m_position;
	Status            m_status;
};

}

#endif
