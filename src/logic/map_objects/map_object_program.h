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

#ifndef WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_PROGRAM_H
#define WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_PROGRAM_H

#include <cstring>
#include <string>

#include "base/macros.h"

#include "logic/map_objects/buildcost.h"

namespace Widelands {
struct MapObjectDescr;

// NOCOM move into anonymous namespace in cc when converting is finished
/// Returns the word starting at the character that p points to and ending
/// before the first terminator character. Replaces the terminator with null.
char* next_word(char*& p, bool& reached_end, char terminator = ' ');

/// Split a string by separators.
/// \note This ignores empty elements, so do not use this for example to split
/// a string with newline characters into lines, because it would ignore empty
/// lines.
std::vector<std::string> split_string(const std::string&, char const* separators);

unsigned int read_positive(std::string input, unsigned int max_value = std::numeric_limits<uint32_t>::max());

const std::pair<std::string, std::string> parse_key_value_pair(const std::string& input, const char separator, const std::string& expected_key = "", bool allow_empty = false);

struct ProgramParseInput {
	std::string name;
	std::vector<std::string> arguments;
};
ProgramParseInput parse_program_string(const std::string& line);

struct AnimationParameters {
	uint32_t animation = 0;
	Duration duration = 0; //  forever
};
AnimationParameters parse_act_animate(const std::vector<std::string>& arguments, const MapObjectDescr& descr, bool is_idle_allowed);

struct PlaySoundParameters {
	std::string name;
	uint8_t priority = 0;
};
PlaySoundParameters parse_act_play_sound(const std::vector<std::string>& arguments, const MapObjectDescr& descr, uint8_t default_priority);

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_PROGRAM_H
