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

#include "editor/scripting.h"

#include "base/wexception.h"

Variable::Variable(const std::string& t)
	: type(t) {
	if (type == "nil") {
		value = "nil";
	} else if (type == "bool") {
		value = "false";
	} else if (type == "int") {
		value = "0";
	} else if (type == "string") {
		value = "";
	} else {
		NEVER_HERE();
	}
}

Variable::Variable(const std::string& t, const std::string& v)
	: type(t), value(v) {
}

