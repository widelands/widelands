/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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

#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "named.h"
#include "i18n.h"

#include <cassert>
#include <string>
#include <cstring>

namespace Widelands {

/// The Map Objective manager keeps all objectives in order.
///
/// A Map (or scenario) objective is an objective that has to be fulfilled to
/// end a scenario successfully.
/// But note, the objectives itself doesn't check it's conditions, the map
/// designer is responsible for checking it and setting it's trigger up.
// TODO: SirVer, Lua: replace is and get here!
struct Objective : public Named {
	Objective()
		:
		m_visname   (name()),
		m_descr     (_("no descr")),
		m_is_done   (false),
		m_is_visible(true)
	{}
	virtual ~Objective() {}

	std::string identifier() const {return "Objective: " + name();}

	const std::string & visname() const throw ()  {return m_visname;}
	const std::string & descr() const throw ()    {return m_descr;}
	void set_visname(std::string const & new_name)  {m_visname = new_name;}
	void set_descr  (std::string const & new_descr) {m_descr   = new_descr;}
	void set_done(bool t) {m_is_done = t;}
	bool get_is_visible()       const throw ()    {return m_is_visible;}
	void set_is_visible(const bool t) throw ()    {m_is_visible = t;}
	bool done() const throw() {return m_is_done;}

private:
	std::string m_visname;
	std::string m_descr;
	bool        m_is_visible;
	bool        m_is_done;
};

}

#endif
