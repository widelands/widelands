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

#ifndef WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_PROGRAM_H
#define WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_PROGRAM_H

#include <limits>
#include <string>
#include <vector>

#include "base/times.h"
#include "sound/constants.h"

namespace Widelands {

class MapObjectDescr;

/// Superclass for Worker, Immovable and Productionsite programs. Includes a program name and
/// diverse parsing convenience functions. The creation and execution of program actions is left to
/// the sub-classes.
struct MapObjectProgram {
	static constexpr const char* const kMainProgram = "main";

	const std::string& name() const;

	explicit MapObjectProgram(const std::string& init_name);
	virtual ~MapObjectProgram() = default;

protected:
	/// Splits a string by separators.
	/// \note This ignores empty elements, so do not use this for example to split
	/// a string with newline characters into lines, because it would ignore empty
	/// lines.
	static std::vector<std::string> split_string(const std::string&, char const* separators);

	/// Reads an int value from a string. Throws a GameDataError if 'min_value' or 'max_value' are
	/// exceeded
	static unsigned int read_int(const std::string& input,
	                             int min_value,
	                             int64_t max_value = std::numeric_limits<int32_t>::max());
	/// Same as 'read_int', with 'min_value' == 1
	static unsigned int read_positive(const std::string& input,
	                                  int64_t max_value = std::numeric_limits<int32_t>::max());

	/**
	 * @brief Reads a key-value pair from a string using the given separator, e.g. "attrib:tree",
	 * "meat:2", "return=skipped unless economy needs meal"
	 * @param input The string to parse
	 * @param separator The separator for splitting the string, e.g. ':' or '='
	 * @param default_value A default to assign to the right-hand value if the separator is not found
	 * @param expected_key If this is not empty, the left-hand key must match this string
	 * @return A key, value pair
	 */
	static const std::pair<std::string, std::string>
	read_key_value_pair(const std::string& input,
	                    const char separator,
	                    const std::string& default_value = "",
	                    const std::string& expected_key = "");

	/**
	 * @brief Reads time duration with units from a string
	 * @param input: A positive integer, followed by 'ms' (milliseconds), 's' (seconds) or 'm'
	 * (minutes). This can be repeated to form units like '1m20s500ms'.
	 * @param descr: For error messages
	 * @return The duration in SDL ticks (milliseconds)
	 */
	static Duration read_duration(const std::string& input, const MapObjectDescr& descr);

	/// Left-hand and right-hand elements of a line in a program, e.g. parsed from "return=skipped
	/// unless economy needs meal"
	struct ProgramParseInput {
		/// Program name, e.g. "return"
		std::string name;
		/// Program arguments, e.g. { "skipped", "unless", "economy", "needs", "meal" }
		std::vector<std::string> arguments;
	};
	/// Reads the program name and arguments from a string
	static ProgramParseInput parse_program_string(const std::string& line);

	/// Animation information
	struct AnimationParameters {
		/// Animation ID
		uint32_t animation = 0;
		/// Animation duration before the next action will be called by the program.
		Duration duration = Duration(0);
	};
	/// Parses the arguments for an animation action, e.g. { "working", "24000" }. If
	/// 'is_idle_allowed' == false, throws a GameDataError if the animation is called "idle".
	static AnimationParameters parse_act_animate(const std::vector<std::string>& arguments,
	                                             const MapObjectDescr& descr,
	                                             bool is_idle_allowed);

	/// Sound effect information
	struct PlaySoundParameters {
		/// Sound effect ID
		FxId fx;
		/// Sound effect priority
		uint16_t priority = 0;
		/// Whether the sound can be played by different map objects at the same time
		bool allow_multiple;
	};
	/// Parses the arguments for a play_sound action, e.g. { "sound/smiths/sharpening", "120" }
	static PlaySoundParameters parse_act_play_sound(const std::vector<std::string>& arguments,
	                                                const MapObjectDescr& descr);

private:
	const std::string name_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_MAP_OBJECT_PROGRAM_H
