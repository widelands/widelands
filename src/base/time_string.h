/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#ifndef WL_BASE_TIME_STRING_H
#define WL_BASE_TIME_STRING_H

#include <string>

/// Get a string representation conforming to ISO 8601 of the current time (in
/// seconds since the Epoch). The return value points to a statically allocated
/// string which might be overwritten by subsequent calls.
char* timestring();

/// Turn a month number into a short, localized month string,
/// 0 = "Jan" ... 11 = "Dec"
std::string localize_month(int8_t month);

/// Get a string representation of the game time as hhh:mm:ss. If Time
/// represents more than 999 hours, it wraps around. Use this in table columns
/// for easy sorting.
char* gamestring_with_leading_zeros(uint32_t gametime);

/// Get a string representation of the game time as [hh]h:mm.
/// If show_seconds = true, this returns [hhh:]mm:ss instead.
/// If Time represents more than 999 hours, it wraps around
std::string gametimestring(uint32_t gametime, bool show_seconds = false);

#endif  // end of include guard: WL_BASE_TIME_STRING_H
