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

#ifndef WL_LOGIC_OBJECTIVE_H
#define WL_LOGIC_OBJECTIVE_H

#include <cassert>
#include <cstring>
#include <string>

#include "base/i18n.h"

namespace Widelands {

// A Map (or scenario) objective is an objective that has to be fulfilled to
// end a scenario successfully.
class Objective {
public:
	Objective(const std::string& init_name)
	   : m_name(init_name),
	     m_descname(init_name),
	     m_descr(_("This objective has no description.")),
	     m_visible(true),
	     m_done(false) {
	}

	// Unique internal name of the objective.
	const std::string& name() const {
		return m_name;
	}

	// User facing (translated) descriptive name.
	const std::string& descname() const {
		return m_descname;
	}
	void set_descname(const std::string& new_name) {
		m_descname = new_name;
	}

	// Description text of this name.
	const std::string& descr() const {
		return m_descr;
	}
	void set_descr(const std::string& new_descr) {
		m_descr = new_descr;
	}

	// True, if this objective is fulfilled.
	bool done() const {
		return m_done;
	}

	void set_done(bool t) {
		m_done = t;
	}

	// True, if this objective is visible to the user.
	bool visible() const {
		return m_visible;
	}
	void set_visible(const bool t) {
		m_visible = t;
	}

private:
	const std::string m_name;
	std::string m_descname;
	std::string m_descr;
	bool m_visible;
	bool m_done;
};
}

#endif  // end of include guard: WL_LOGIC_OBJECTIVE_H
