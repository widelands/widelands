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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/map_objects/map_object_program.h"

#include <cstdlib>
#include <regex>

#include "base/log.h"
#include "base/math.h"
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
* :ref:`map_object_programs_actions`

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

Map object programs are put in a Lua table, like this:

.. code-block:: lua

   programs = {
      main = {
         "action1=parameter1:value1 parameter2:value2",
         "action2=value1",
      },
      program_name2 = {
         "action3",
         "action4=value1 value2 value3",
      },
      program_name3 = {
         "action5=value1 value2 parameter:value3",
      }
   },

For productionsites, there is a nested ``actions`` table, so that we can give them a descname for
the tooltips:

.. code-block:: lua

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start doing something because ...
         descname = _"doing something",
         actions = {
            "call=program_name2",
            "call=program_name3",
         }
         ...
     }
   }

* Named parameters of the form ``parameter:value`` can be given in any order, but we recommend using
  the order from the documentation for consistency. It will make your code easier to read.
* Values without parameter name need to be given in the correct order.
* Some actions combine both named and unnamed values, see ``action5`` in our example.

If there is a program called ``"main"``, this is the default program.
For :ref:`productionsites <productionsite_programs>`, having a main program is mandatory.
For :ref:`immovables <immovable_programs>`, having a main program is optional, because their
programs can also be triggered by a productionsite or by a worker. :ref:`Workers
<tribes_worker_programs>` have no default program, because their individual programs are always
called from their production site.

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
	std::string::size_type endpos;
	for (std::string::size_type pos = 0;
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
	int64_t const value = strtol(input.c_str(), &endp, 0);
	result = value;
	if (*endp || static_cast<int64_t>(result) != value) {
		throw GameDataError("Expected a number but found \"%s\"", input.c_str());
	}
	if (value < min_value) {
		throw GameDataError("Expected a number >= %d but found \"%s\"", min_value, input.c_str());
	}
	if (value > max_value) {
		throw GameDataError(
		   "Expected a number <= %" PRIi64 " but found \"%s\"", max_value, input.c_str());
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
	auto as_ms = [](uint32_t number, const std::string& unit) {
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
		std::smatch match;
		std::regex one_unit("^(\\d+)(s|m|ms)$");
		if (std::regex_search(input, match, one_unit)) {
			return Duration(as_ms(read_positive(match[1], Duration().get()), match[2]));
		}
		std::regex two_units("^(\\d+)(m|s)(\\d+)(s|ms)$");
		if (std::regex_search(input, match, two_units)) {
			if (match[2] == match[4]) {
				std::string unit(match[2]);
				throw GameDataError("has duplicate unit '%s'", unit.c_str());
			}
			const Duration part1(as_ms(read_positive(match[1], Duration().get()), match[2]));
			const Duration part2(as_ms(read_positive(match[3], Duration().get()), match[4]));
			return part1 + part2;
		}
		std::regex three_units("^(\\d+)(m)(\\d+)(s)(\\d+)(ms)$");
		if (std::regex_search(input, match, three_units)) {
			const Duration part1(as_ms(read_positive(match[1], Duration().get()), match[2]));
			const Duration part2(as_ms(read_positive(match[3], Duration().get()), match[4]));
			const Duration part3(as_ms(read_positive(match[5], Duration().get()), match[6]));
			return part1 + part2 + part3;
		}
		// TODO(GunChleoc): Compatibility, remove unitless option after v1.0
		std::regex without_unit("^(\\d+)$");
		if (std::regex_match(input, without_unit)) {
			log_warn("Duration '%s' without unit in %s's program is deprecated", input.c_str(),
			         descr.name().c_str());
			return Duration(read_positive(input, Duration().get()));
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

// Percent is implemented in base/math.h so that we can use it in animations too
/* RST

.. _map_object_programs_datatypes_percent:

Percent
^^^^^^^

A percent value. Valid unit is:

* ``%`` (percent)

Maximum value is ``100%``. Examples:

* ``25%``
* ``25.1%``
* ``25.13%``

*/

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
		if (idx == std::string::npos) {
			throw GameDataError("Empty value in '%s' for separator '%c'\n", input.c_str(), separator);
		}
		if (key != expected_key) {
			throw GameDataError("Expected key '%s' but found '%s' in '%s'\n", expected_key.c_str(),
			                    key.c_str(), input.c_str());
		}
	}

	return std::make_pair(key, idx == std::string::npos ? default_value : input.substr(idx + 1));
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

   Example for a worker:

.. code-block:: lua

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
	if (arguments.empty() || arguments.size() > 2) {
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
			log_warn("'animate' program without parameter name is deprecated, please use "
			         "'animate=<animation_name> duration:<duration>' in %s\n",
			         descr.name().c_str());
		} else {
			throw GameDataError("Unknown argument '%s'. Usage: <animation_name> [duration:<duration>]",
			                    arguments.at(1).c_str());
		}
	}
	return result;
}

/* RST

.. _map_object_programs_playsound:

playsound
^^^^^^^^^^
.. function:: playsound=\<sound_dir/sound_name\> priority:<\percent\> \[allow_multiple\]

   :arg string sound_dir/sound_name: The directory (folder) that the sound files are in,
      relative to the data directory, followed by the name of the particular sound to play.
      There can be multiple sound files to select from at random, e.g.
      for `sound/farm/scythe`, we can have `sound/farm/scythe_00.ogg`, `sound/farm/scythe_01.ogg`
      ...

   :arg percent priority: The priority to give this sound,
      in :ref:`map_object_programs_datatypes_percent`. Maximum priority is ``100%``.

   :arg allow_multiple: When this parameter is given, the sound can be played by different map
      objects at the same time.

   Trigger a sound effect. Whether the sound effect is actually played is determined by the
   sound handler.

   Examples:

.. code-block:: lua

      -- Worker
      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         -- Almost certainly play a swishy harvesting sound
         "playsound=sound/farm/scythe priority:95%",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=wheat",
         "return"
      }

      -- Production site
     produce_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an ax because ...
         descname = _"forging an ax",
         actions = {
            "return=skipped unless economy needs ax",
            "consume=coal iron",
            "sleep=duration:26s",
            -- Play a banging sound 50% of the time.
            -- Other buildings can also play this sound at the same time.
            "playsound=sound/smiths/smith priority:50% allow_multiple",
            "animate=working duration:22s",
            -- Play a sharpening sound 50% of the time,
            -- but not if another building is already playing it right now.
            "playsound=sound/smiths/sharpening priority:90%",
            "sleep=duration:9s",
            "produce=ax"
         }
      }
*/
MapObjectProgram::PlaySoundParameters
MapObjectProgram::parse_act_play_sound(const std::vector<std::string>& arguments,
                                       const MapObjectDescr& descr) {
	if (arguments.size() != 2 && arguments.size() != 3) {
		throw GameDataError(
		   "Usage: playsound=<sound_dir/sound_name> priority:<percent> [allow_multiple]");
	}
	PlaySoundParameters result;
	result.fx = SoundHandler::register_fx(SoundType::kAmbient, arguments.at(0));
	result.allow_multiple = false;

	const std::pair<std::string, std::string> item = read_key_value_pair(arguments.at(1), ':');
	if (item.first == "priority") {
		result.priority = math::read_percent_to_int(item.second);
	} else if (item.second.empty()) {
		if (item.first == "allow_multiple") {
			result.allow_multiple = true;
		} else {
			// TODO(GunChleoc): Compatibility, remove this option after v1.0
			result.priority = (read_positive(arguments.at(1)) * math::k100PercentAsInt * 2U) / 256;
			log_warn("Deprecated usage in %s. Please convert playsound's 'priority' option to "
			         "percentage, like this: "
			         "playsound=<sound_dir/sound_name> priority:<percent> [allow_multiple]\n",
			         descr.name().c_str());
		}
	} else {
		throw GameDataError("Unknown argument '%s'. Usage: playsound=<sound_dir/sound_name> "
		                    "priority:<percent> [allow_multiple]",
		                    arguments.at(1).c_str());
	}

	if (result.priority < kFxPriorityLowest) {
		throw GameDataError("Minimum priority for sounds is %d, but only %d was specified for %s",
		                    kFxPriorityLowest, result.priority, arguments.at(0).c_str());
	}
	return result;
}

}  // namespace Widelands
