/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_EDITOR_SCRIPTING_SCRIPTING_H
#define WL_EDITOR_SCRIPTING_SCRIPTING_H

#include <list>
#include <string>

#include "base/macros.h"

const std::string kMainFunction = "mission_thread";

// TODO saveloading for all this in some editor-only packet

struct Variable {
	std::string type; // type of the value (supported: nil, string, bool, int)
	std::string value;

	Variable(const std::string&);
	Variable(const std::string&, const std::string&);
	~Variable() {
	}
};

struct FunctionStatement {



};

struct Function {
	Function(bool a) : autostart(a) {
	}
	~Function() {
	}

	bool autostart;
	std::list<std::string> parameters;
	std::list<FunctionStatement> body;
};

#endif  // end of include guard: WL_EDITOR_SCRIPTING_SCRIPTING_H
