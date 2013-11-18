/*
 * Copyright (C) 2002-2008, 2011 by the Widelands Development Team
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

#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include <cassert>
#include <cstring>
#include <string>

#include "i18n.h"
#include "named.h"

namespace Widelands {

/// The Map Objective manager keeps all objectives in order.
///
/// A Map (or scenario) objective is an objective that has to be fulfilled to
/// end a scenario successfully.
/// But note, the objectives itself doesn't check it's conditions, the map
/// designer is responsible for checking it and setting its done property up.
struct Objective : public Named {
	Objective()
		:
		m_descname   (name()),
		m_descr     (_("no descr")),
		m_visible(true),
		m_done   (false)
	{}
	virtual ~Objective() {}

	std::string identifier() const {return "Objective: " + name();}

	const std::string & descname() const  {return m_descname;}
	const std::string & descr() const    {return m_descr;}
	bool visible() const {return m_visible;}
	bool done() const {return m_done;}
	void set_descname(const std::string & new_name) {
		m_descname = new_name;
	}
	void set_descr  (const std::string & new_descr) {m_descr   = new_descr;}
	void set_visible(const bool t)    {m_visible = t;}
	void set_done(bool t) {m_done = t;}

private:
	std::string m_descname;
	std::string m_descr;
	bool        m_visible;
	bool        m_done;
};

}

#endif
