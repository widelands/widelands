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

/* RST
Programs
========
.. _map_object_programs:

Many map objects have special programs that define their behavior.
You can describe these programs in their ``init.lua`` files, in the ``programs``
table.

* :ref:`map_object_programs_syntax`
* :ref:`map_object_programs_datatypes`
* :ref:`map_object_programs_actions`.

Map objects that can have programs are:

.. toctree::
   :maxdepth: 1

   Immovables <autogen_immovable_programs>
   Production Sites <autogen_tribes_productionsite_programs>
   Workers <autogen_tribes_worker_programs>

Critters all run the same built-in program, so you don't need to define any programs for them.

.. _map_object_programs_syntax:

Syntax
------

Map object programs are put in a Lua table, like this::

   programs = {
      default_program = {
         "program_name2",
         "program_name3",
      }
      program_name2 = {
         "action1=parameter1:value1 parameter2:value2",
         "action2=value1",
      },
      program_name3 = {
         "action3",
         "action4=value1 value2 value3",
      },
      program_name4 = {
         "action5=value1 value2 parameter:value3",
      }
   },

* Named parameters of the form ``parameter:value`` can be given in any order, but we recommend using
  the order from the documentation for consistency. It will make your code easier to read.
* Values without parameter name need to be given in the correct order.
* Some actions combine both named and unnamed values, see ``action5`` in our example.

The first program is the default program that calls all the other programs. For productionsites,
this is ``"work"``, and for immovables, this is ``"program"``. Workers have no default program,
because their individual programs are called from their production site.


.. _map_object_programs_datatypes:

Data Types
----------

Some numerical action parameters use units of measure to clarify their meaning.
*/

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

// Using int64_t in input so we can get the full range of unsigned int in the output while still
// checking for negative integers.
unsigned int
MapObjectProgram::read_int(const std::string& input, int min_value, int64_t max_value) {
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

// Using int64_t in input so we can get the full range of unsigned int in the output while still
// checking for negative integers.
unsigned int MapObjectProgram::read_positive(const std::string& input, int64_t max_value) {
	return read_int(input, 1, max_value);
}

/* RST

.. _map_object_programs_datatypes_duration:

Duration
^^^^^^^^

Temporal duration is specified with an accompanying unit. Valid units are:

* ``m`` (minutes)
* ``s`` (seconds)
* ``ms`` (milliseconds)

You can combine these units in descending order as you please. Examples:

* ``4m``
* ``12s``
* ``500ms``
* ``4m12s``
* ``12s500ms``
* ``4m500ms``
* ``4m12s500ms``
* ``1m500s100000ms`` will work too, but is not recommended (unreadable)

*/
Duration MapObjectProgram::read_duration(const std::string& input, const MapObjectDescr& descr) {
	// Convert unit part into milliseconds
	auto as_ms = [](Duration number, const std::string& unit) {
		if (unit == "s") {
			return number * 1000;
		}
		if (unit == "m") {
			return number * 60000;
		}
		if (unit == "ms") {
			return number;
		}
		throw GameDataError("has unknown unit '%s'", unit.c_str());
	};

	try {
		boost::smatch match;
		boost::regex one_unit("^(\\d+)(s|m|ms)$");
		if (boost::regex_search(input, match, one_unit)) {
			return as_ms(read_positive(match[1], endless()), match[2]);
		}
		boost::regex two_units("^(\\d+)(m|s)(\\d+)(s|ms)$");
		if (boost::regex_search(input, match, two_units)) {
			if (match[2] == match[4]) {
				std::string unit(match[2]);
				throw GameDataError("has duplicate unit '%s'", unit.c_str());
			}
			const Duration part1 = as_ms(read_positive(match[1], endless()), match[2]);
			const Duration part2 = as_ms(read_positive(match[3], endless()), match[4]);
			return part1 + part2;
		}
		boost::regex three_units("^(\\d+)(m)(\\d+)(s)(\\d+)(ms)$");
		if (boost::regex_search(input, match, three_units)) {
			const Duration part1 = as_ms(read_positive(match[1], endless()), match[2]);
			const Duration part2 = as_ms(read_positive(match[3], endless()), match[4]);
			const Duration part3 = as_ms(read_positive(match[5], endless()), match[6]);
			return part1 + part2 + part3;
		}
		// TODO(GunChleoc): Compatibility, remove unitless option after v1.0
		boost::regex without_unit("^(\\d+)$");
		if (boost::regex_match(input, without_unit)) {
			log("WARNING: Duration '%s' without unit in %s's program is deprecated\n", input.c_str(),
			    descr.name().c_str());
			return read_positive(input, endless());
		}
	} catch (const WException& e) {
		throw GameDataError(
		   "Duration '%s' %s. Usage: <numbers>{m|s|ms}[<numbers>{s|ms}][<numbers>ms]", input.c_str(),
		   e.what());
	}
	throw GameDataError(
	   "Illegal duration: %s. Usage: <numbers>{m|s|ms}[<numbers>{s|ms}][<numbers>ms]",
	   input.c_str());
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

/* RST

.. _map_object_programs_actions:

Actions
-------

The actions documented in this section are available to all map object types.

.. _map_object_programs_animate:

animate
^^^^^^^
.. function:: animate=\<name\> [duration:\<duration\>]

   Switch to new animation and pause program execution for the given duration.

   :arg string name: The name of the animation to be played.
   :arg duration duration: The time :ref:`map_object_programs_datatypes_duration` for which the
      program will wait before continuing on to the next action. If omitted, the program will
      continue to the next step immediately.

   Example for a worker::

      plantvine = {
         "findspace=size:any radius:1",
         "walk=coords",
         "animate=dig duration:2s500ms", -- Play a digging animation for 2.5 seconds.
         "plant=attrib:seed_grapes",
         "animate=planting duration:3s", -- Play a planting animation for 3 seconds.
         "return"
      },

The animate action will trigger a new animation, then wait for the specified duration before moving
on to the next action in the program. The animation will continue playing and loop around until the
program ends or another ``animate=`` action is called. The given duration does not have to equal the
length of the animation.

When the program ends, the map object will switch back to the default ``idle`` animation. Some
actions also have an animation associated with them that will be played instead, e.g.
``"walk=coords"`` will play the walking animation for the direction the worker is walking in.
*/
MapObjectProgram::AnimationParameters MapObjectProgram::parse_act_animate(
   const std::vector<std::string>& arguments, const MapObjectDescr& descr, bool is_idle_allowed) {
	if (arguments.size() < 1 || arguments.size() > 2) {
		throw GameDataError("Usage: animate=<animation_name> [duration:<duration>]");
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
			result.duration = read_duration(item.second, descr);
		} else if (item.second.empty()) {
			// TODO(GunChleoc): Compatibility, remove this option after v1.0
			result.duration = read_duration(item.first, descr);
			log("WARNING: 'animate' program without parameter name is deprecated, please use "
			    "'animate=<animation_name> duration:<duration>' in %s\n",
			    descr.name().c_str());
		} else {
			throw GameDataError("Unknown argument '%s'. Usage: <animation_name> [duration:<duration>]",
			                    arguments.at(1).c_str());
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
