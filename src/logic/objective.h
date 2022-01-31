/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_OBJECTIVE_H
#define WL_LOGIC_OBJECTIVE_H

#include "base/i18n.h"

namespace Widelands {

// A Map (or scenario) objective is an objective that has to be fulfilled to
// end a scenario successfully.
class Objective {
public:
	explicit Objective(const std::string& init_name)
	   : name_(init_name),
	     descname_(init_name),
	     descr_(_("This objective has no description.")),
	     visible_(true),
	     done_(false) {
	}

	// Unique internal name of the objective.
	const std::string& name() const {
		return name_;
	}

	// User facing (translated) descriptive name.
	const std::string& descname() const {
		return descname_;
	}
	void set_descname(const std::string& new_name) {
		descname_ = new_name;
	}

	// Description text of this name.
	const std::string& descr() const {
		return descr_;
	}
	void set_descr(const std::string& new_descr) {
		descr_ = new_descr;
	}

	// True, if this objective is fulfilled.
	bool done() const {
		return done_;
	}

	void set_done(bool t) {
		done_ = t;
	}

	// True, if this objective is visible to the user.
	bool visible() const {
		return visible_;
	}
	void set_visible(const bool t) {
		visible_ = t;
	}

private:
	const std::string name_;
	std::string descname_;
	std::string descr_;
	bool visible_;
	bool done_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_OBJECTIVE_H
