/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#include "base/math.h"

#include <regex>

#include "base/warning.h"
#include "base/wexception.h"

namespace math {

#define TO_NUMBER(identifier, type, function)                                                      \
	type identifier(const std::string& str) {                                                       \
		try {                                                                                        \
			return function(str);                                                                     \
		} catch (...) {                                                                              \
			throw WLWarning("", "Expected a " #type ", received: %s", str.c_str());                   \
		}                                                                                            \
	}

TO_NUMBER(to_int, int, stoi)
TO_NUMBER(to_long, long long, stoll)
#undef TO_NUMBER

// This function has RST documentation in logic/map_objects/map_object_program
unsigned read_percent_to_int(const std::string& input) {
	std::smatch match;
	std::regex re("^\\d+([.](\\d{1,2})){0,1}%$");
	if (std::regex_search(input, match, re)) {
		// Convert to range

		uint64_t result = 100U * std::stoul(match[0]) +
		                  // Match[1] are the outer () around ([.](\\d{1,2})), but we're interested in
		                  // (\\d{1,2}) here, which is match[2].
		                  (match[2].str().empty()     ? 0U :
		                   match[2].str().size() == 1 ? 10U * std::stoul(match[2]) :
                                                      std::stoul(match[2]));

		if (result > k100PercentAsInt) {
			throw wexception(
			   "Given percentage of '%s' is greater than the 100%% allowed", input.c_str());
		}
		return result;
	}
	throw wexception(
	   "Wrong format for percentage '%s'. Must look like '25%%', '25.4%%' or '25.26%%'.",
	   input.c_str());
}

}  // namespace math
