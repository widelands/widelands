/*
 * Copyright (C) 2006-2009 by the Widelands Development Team
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

#ifndef WL_HELPER_H
#define WL_HELPER_H

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include <SDL_keyboard.h>
#include <boost/utility.hpp>

#include "base/wexception.h"

/// Returns the word starting at the character that p points to and ending
/// before the first terminator character. Replaces the terminator with null.
// TODO(sirver): move into a logic/strings lib or so.
char* next_word(char*& p, bool& reached_end, char terminator = ' ');

/// Split a string by separators.
/// \note This ignores empty elements, so do not use this for example to split
/// a string with newline characters into lines, because it would ignore empty
/// lines.
std::vector<std::string> split_string(const std::string&, char const* separators);

// A functional container filtering (by copying the values). Returns a new
// ContainerType that  contains all values where 'test' returned true.
template <typename ContainerType, class UnaryPredicate>
ContainerType filter(const ContainerType& container, UnaryPredicate test) {
	ContainerType filtered;
	for (const auto& entry : container) {
		if (!test(entry)) {
			continue;
		}
		filtered.insert(entry);
	}
	return filtered;
}
#endif  // end of include guard: WL_HELPER_H
