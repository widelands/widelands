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

struct MapObjectProgram {
	std::string name() const;

	explicit MapObjectProgram(const std::string& init_name);
	virtual ~MapObjectProgram() = default;

protected:
	/// Split a string by separators.
	/// \note This ignores empty elements, so do not use this for example to split
	/// a string with newline characters into lines, because it would ignore empty
	/// lines.
	static std::vector<std::string> split_string(const std::string&, char const* separators);

	static unsigned int read_number(const std::string& input, int min_value, int max_value = std::numeric_limits<int32_t>::max());
	static unsigned int read_positive(const std::string& input, int max_value = std::numeric_limits<int32_t>::max());

	static const std::pair<std::string, std::string> read_key_value_pair(const std::string& input, const char separator, const std::string& default_value = "", const std::string& expected_key = "");

	struct ProgramParseInput {
		std::string name;
		std::vector<std::string> arguments;
	};
	static ProgramParseInput parse_program_string(const std::string& line);

	struct AnimationParameters {
		uint32_t animation = 0;
		Duration duration = 0; //  forever
	};
	static AnimationParameters parse_act_animate(const std::vector<std::string>& arguments, const MapObjectDescr& descr, bool is_idle_allowed);

	struct PlaySoundParameters {
		std::string name;
		uint8_t priority = 0;
	};
	static PlaySoundParameters parse_act_play_sound(const std::vector<std::string>& arguments, uint8_t default_priority);

private:
	const std::string name_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_PROGRAM_H
