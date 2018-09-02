/*
 * Copyright (C) 2018 by the Widelands Development Team
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

#include "website/json/value.h"

#include <boost/lexical_cast.hpp>

namespace JSON {

Boolean::Boolean(bool value) : bool_value(value) {
}
std::string Boolean::as_string() const {
	return bool_value ? "true" : "false";
}

Double::Double(double value) : double_value(value) {
}
std::string Double::as_string() const {
	// NOCOM this turns 0.815 into 0.81499999999999995, so we need something better
	return boost::lexical_cast<std::string>(double_value);
}

Int::Int(int value) : int_value(value) {}
std::string Int::as_string() const {
	return boost::lexical_cast<std::string>(int_value);
}

String::String(std::string value) : string_value(value) {}
std::string String::as_string() const {
	return "\"" + string_value + "\"";
}

}  // namespace JSON
