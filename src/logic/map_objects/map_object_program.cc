/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "logic/map_objects/map_object_program.h"

#include <boost/regex.hpp>

#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/map_object.h"
#include "sound/sound_handler.h"

namespace Widelands {

MapObjectProgram::MapObjectProgram(const std::string& init_name) : name_(init_name) {
}

const std::string& MapObjectProgram::name() const {
	return name_;
}

std::vector<std::string> MapObjectProgram::split_string(const std::string& s,
                                                        const char* const separators) {
	std::vector<std::string> result;
	for (std::string::size_type pos = 0, endpos;
	     (pos = s.find_first_not_of(separators, pos)) != std::string::npos; pos = endpos) {
		endpos = s.find_first_of(separators, pos);
		result.push_back(s.substr(pos, endpos - pos));
	}
	return result;
}

// Using int64_t in input so we can get the full range of unsigned int in the output while still checking for negative integers.
unsigned int MapObjectProgram::read_int(const std::string& input, int min_value, int64_t max_value) {
	unsigned int result = 0U;
	char* endp;
	long int const value = strtol(input.c_str(), &endp, 0);
	result = value;
	if (*endp || static_cast<long>(result) != value) {
		throw GameDataError("Expected a number but found \"%s\"", input.c_str());
	}
	if (value < min_value) {
		throw GameDataError("Expected a number >= %d but found \"%s\"", min_value, input.c_str());
	}
	if (value > max_value) {
		throw GameDataError("Expected a number <= %ld but found \"%s\"", max_value, input.c_str());
	}
	return result;
}

// Using int64_t in input so we can get the full range of unsigned int in the output while still checking for negative integers.
unsigned int MapObjectProgram::read_positive(const std::string& input, int64_t max_value) {
	return read_int(input, 1, max_value);
}

Duration MapObjectProgram::read_duration(const std::string& input) {
	boost::regex with_unit("^(\\d+)(ms|s|m)$");
	boost::smatch match;
	if (boost::regex_search(input, match, with_unit)) {
		Duration result = read_positive(match[1], endless());
		if (match[2] == 's') {
			result *= 1000;
		} else if (match[2] == 'm') {
			result *= 60000;
		}
		return result;
	}
	boost::regex without_unit("^(\\d+)$");
	if (boost::regex_match(input, without_unit)) {
		return read_positive(input, endless());
	}
	throw GameDataError("Illegal duration: %s. Usage: numbers{ms|s|m}", input.c_str());
}

MapObjectProgram::ProgramParseInput
MapObjectProgram::parse_program_string(const std::string& line) {
	const std::pair<std::string, std::string> key_values =
	   MapObjectProgram::read_key_value_pair(line, '=');
	return ProgramParseInput{key_values.first, split_string(key_values.second, " \t\n")};
}

const std::pair<std::string, std::string>
MapObjectProgram::read_key_value_pair(const std::string& input,
                                      const char separator,
                                      const std::string& default_value,
                                      const std::string& expected_key) {
	const size_t idx = input.find(separator);
	const std::string key = input.substr(0, idx);

	if (!expected_key.empty()) {
		if (idx == input.npos) {
			throw GameDataError("Empty value in '%s' for separator '%c'\n", input.c_str(), separator);
		}
		if (key != expected_key) {
			throw GameDataError("Expected key '%s' but found '%s' in '%s'\n", expected_key.c_str(),
			                    key.c_str(), input.c_str());
		}
	}

	return std::make_pair(key, idx == input.npos ? default_value : input.substr(idx + 1));
}

MapObjectProgram::AnimationParameters MapObjectProgram::parse_act_animate(
   const std::vector<std::string>& arguments, const MapObjectDescr& descr, bool is_idle_allowed) {
	if (arguments.size() < 1 || arguments.size() > 2) {
		throw GameDataError("Usage: animate=animation_name [duration:numbers{ms|s|m}]");
	}

	AnimationParameters result;
	const std::string& animation_name = arguments.at(0);

	if (!is_idle_allowed && animation_name == "idle") {
		throw GameDataError("'idle' animation is default; calling is not allowed");
	}
	if (!descr.is_animation_known(animation_name)) {
		throw GameDataError("Unknown animation '%s'", animation_name.c_str());
	}
	result.animation = descr.get_animation(animation_name, nullptr);

	if (arguments.size() == 2) {
		const std::pair<std::string, std::string> item = read_key_value_pair(arguments.at(1), ':');
		if (item.first == "duration") {
			result.duration = read_duration(item.second);
		} else if (item.second.empty()) {
			result.duration = read_duration(item.first);
		} else {
			throw GameDataError(
			   "Unknown argument '%s'. Usage: animation_name [duration:numbers{ms|s|m}]", arguments.at(1).c_str());
		}
	}
	return result;
}

MapObjectProgram::PlaySoundParameters
MapObjectProgram::parse_act_play_sound(const std::vector<std::string>& arguments,
                                       uint8_t default_priority) {
	std::string filepath = "";
	PlaySoundParameters result;

	// TODO(GunChleoc): Savegame compabitility. Remove after Build 21.
	if (arguments.size() == 3) {
		filepath = arguments.at(0) + "/" + arguments.at(1);
		result.priority = read_positive(arguments.at(2));
	} else {
		if (arguments.size() < 1 || arguments.size() > 2) {
			throw GameDataError("Usage: playsound=<sound_dir/sound_name> [priority]");
		}
		filepath = arguments.at(0);
		result.priority = arguments.size() == 2 ? read_positive(arguments.at(1)) : default_priority;
	}

	result.fx = SoundHandler::register_fx(SoundType::kAmbient, filepath);

	if (result.priority < kFxPriorityLowest) {
		throw GameDataError("Minmum priority for sounds is %d, but only %d was specified for %s",
		                    kFxPriorityLowest, result.priority, filepath.c_str());
	}
	return result;
}

}  // namespace Widelands
