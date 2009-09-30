/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "i18n.h"
#include "map.h"

#include <cassert>
#include <string>
#include <cstring>

namespace Widelands {
	struct Coords;

	///the Message manager keeps track of messages
	///based of the objective system

struct Message {
	Message()
		:
		m_sender    ("unknown sender"),
		m_time      (0),
		m_title     ("unknown message"),
		m_descr     (_("no descr")),
		m_is_visible(true)
	{}
	Message
		(std::string const &       sender,
		 uint32_t                  time,
		 std::string const &       t,
		 Widelands::Coords   const c,
		 std::string const &       description)
		:
		m_sender(sender),
		m_time(time),
		m_title(t),
		m_descr(description),
		m_coords(c),
		m_is_visible(true)
	{}

	std::string identifier() const {
		return "Message: " + m_title + '|' + m_descr;
	}

	const std::string & title() const throw ()  {return m_title;}
	const std::string & descr() const throw ()    {return m_descr;}
	void set_title(std::string const & new_name)  {m_title = new_name;}
	void set_descr  (std::string const & new_descr) {m_descr   = new_descr;}
	bool get_is_visible()       const throw ()    {return m_is_visible;}
	void set_is_visible(const bool t) throw ()    {m_is_visible = t;}
	Widelands::Coords get_coords() const {return m_coords;}

private:
	std::string m_sender;
	uint32_t m_time;

	std::string m_title;
	std::string m_descr;
	Widelands::Coords m_coords;
	bool        m_is_visible;

};

}

#endif
