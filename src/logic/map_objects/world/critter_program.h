/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_PROGRAM_H
#define WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_PROGRAM_H

#include "logic/map_objects/bob.h"

namespace Widelands {

struct CritterAction {
	using CritterExecuteActionFn = bool (Critter::*)(Game&, Bob::State&, const CritterAction&);

	enum {
		walkObject,  //  walk to objvar1
		walkCoords,  //  walk to coords
	};

	CritterExecuteActionFn function;
	int32_t iparam1;
	int32_t iparam2;
	std::string sparam1;

	std::vector<std::string> sparamv;
};

struct CritterProgram : public BobProgramBase {
	CritterProgram(const std::string& name) : name_(name) {
	}
	virtual ~CritterProgram() {
	}

	std::string get_name() const override {
		return name_;
	}
	int32_t get_size() const {
		return actions_.size();
	}
	const CritterAction& operator[](size_t const idx) const {
		assert(idx < actions_.size());
		return actions_[idx];
	}

	void parse(const std::vector<std::string>& lines);

private:
	std::string name_;
	std::vector<CritterAction> actions_;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WORLD_CRITTER_PROGRAM_H
