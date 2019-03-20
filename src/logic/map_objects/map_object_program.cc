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

#include "logic/map_objects/map_object_program.h"

#include <boost/algorithm/string.hpp>

#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/map_object.h"
#include "sound/sound_handler.h"

namespace Widelands {

char* next_word(char*& p, bool& reached_end, char const terminator) {
	assert(terminator);
	char* const result = p;
	for (; *p != terminator; ++p)
		if (*p == '\0') {
			reached_end = true;
			goto end;
		}
	reached_end = false;
	*p = '\0';  //  terminate the word
	++p;        //  move past the terminator
end:
	if (result < p)
		return result;
	throw wexception("expected word");
}

std::vector<std::string> split_string(const std::string& s, const char* const separators) {
	std::vector<std::string> result;
	for (std::string::size_type pos = 0, endpos;
	     (pos = s.find_first_not_of(separators, pos)) != std::string::npos; pos = endpos) {
		endpos = s.find_first_of(separators, pos);
		result.push_back(s.substr(pos, endpos - pos));
	}
	return result;
}

unsigned int read_positive(std::string input, unsigned int max_value) {
	unsigned int  result = 0U;
	char* endp;
	long long int const value = strtoll(input.c_str(), &endp, 0);
	result = value;
	if (*endp || result != value) {
		throw GameDataError("Expected a number but found \"%s\"", input.c_str());
	}
	if (value <= 0) {
		throw GameDataError("Expected a number > 0 but found \"%s\"", input.c_str());
	}
	if (value > max_value) {
		throw GameDataError("Expected a number <= %d but found \"%s\"", max_value, input.c_str());
	}
	return result;
}

ProgramParseInput parse_program_string(const std::string& action_string) {
	ProgramParseInput result;
	std::vector<std::string> parts;
	boost::split(parts, action_string, boost::is_any_of("="));
	if (parts.size() < 1 || parts.size() > 2) {
		throw GameDataError("Invalid line: \"%s\" in program", action_string.c_str());
	}

	result.name = parts.at(0);
	if (parts.size() == 2) {
		boost::split(result.arguments, parts.at(1), boost::is_any_of(" \t\n"));
	}
	return result;
}


AnimationParameters parse_act_animate(const std::vector<std::string>& arguments, const MapObjectDescr& descr, bool is_idle_allowed) {
	AnimationParameters result;
	if (arguments.size() < 1) {
		throw GameDataError("%s: Animation without name. Usage: animate=<name> <duration>",
							descr.name().c_str());
	}

	const std::string animation_name = arguments.at(0);

	if (arguments.size() > 2) {
		throw GameDataError("%s: Animation %s has too many parameters. Usage: animate=<name> <duration>",
							descr.name().c_str(), animation_name.c_str());
	}
	if (!is_idle_allowed && animation_name == "idle") {
		throw GameDataError("%s: 'idle' animation is default; calling is not allowed",
							descr.name().c_str());
	}
	if (!descr.is_animation_known(animation_name)) {
		throw GameDataError("%s: Unknown animation '%s'",
							descr.name().c_str(), animation_name.c_str());
	}
	result.animation = descr.get_animation(animation_name);

	if (arguments.size() == 2) {  //  The next parameter is the duration.
		result.duration = read_positive(arguments.at(1));
	}
	return result;
}

PlaySoundParameters parse_act_play_sound(const std::vector<std::string>& arguments, const MapObjectDescr& descr, uint8_t default_priority) {
	PlaySoundParameters result;

	if (arguments.size() < 2 || arguments.size() > 3) {
		throw GameDataError("%s: Wrong number of parameters. Usage: playsound <sound_dir> <sound_name> [priority]", descr.name().c_str());
	}

	result.name = arguments.at(0)+ g_fs->file_separator() + arguments.at(1);

	g_sound_handler.load_fx_if_needed(arguments.at(0), arguments.at(1), result.name);

	result.priority = default_priority;
	if (arguments.size() == 3) {
		char* endp;
		long long int const value = strtoll(arguments.at(2).c_str(), &endp, 0);
		result.priority = value;
		if (*endp || result.priority != value) {
			throw GameDataError("%s: Playsound expected a priority but found \"%s\"", descr.name().c_str(), arguments.at(2).c_str());
		}
	}
	return result;
}

}  // namespace Widelands
