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

#include "logic/map_objects/tribes/worker_program.h"

#include "base/log.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/findnode.h"
#include "sound/sound_handler.h"

namespace Widelands {
/* RST
.. _tribes_worker_programs:

Worker Programs
===============

Worker programs are defined in the ``programs`` subtable specified in the worker's
:ref:`lua_tribes_workers_common`. Each worker program is a Lua table in itself and defined as a
series of command strings. Commands can also have parameters, which are separated from each other by
a blank space. These parameters can also have values, which are separated from the parameter name by
a colon (:). Finally, programs can call other programs. The table looks like this::

   programs = {
      program_name1 = {
         "program_name2",
         "program_name3",
      }
      program_name2 = {
         "action1=parameter1:value1 parameter2:value2",
         "action2=parameter1",
      },
      program_name3 = {
         "action3",
         "action4=parameter1 parameter2 parameter3",
      }
   },


For general information about the format, see :ref:`map_object_programs_syntax`.

Available actions are:

- `createware`_
- `mine`_
- `breed`_
- `findobject`_
- `findspace`_
- `walk`_
- `animate`_
- `return`_
- `callobject`_
- `plant`_
- `createbob`_
- `buildferry`_
- `removeobject`_
- `repeatsearch`_
- `findresources`_
- `scout`_
- `playsound`_
- `construct`_
- `terraform`_
*/

const WorkerProgram::ParseMap WorkerProgram::parsemap_[] = {
   {"mine", &WorkerProgram::parse_mine},
   {"breed", &WorkerProgram::parse_breed},
   {"createware", &WorkerProgram::parse_createware},
   {"findobject", &WorkerProgram::parse_findobject},
   {"findspace", &WorkerProgram::parse_findspace},
   {"walk", &WorkerProgram::parse_walk},
   {"animate", &WorkerProgram::parse_animate},
   {"return", &WorkerProgram::parse_return},
   {"callobject", &WorkerProgram::parse_callobject},
   {"plant", &WorkerProgram::parse_plant},
   {"createbob", &WorkerProgram::parse_createbob},
   {"buildferry", &WorkerProgram::parse_buildferry},
   {"removeobject", &WorkerProgram::parse_removeobject},
   {"repeatsearch", &WorkerProgram::parse_repeatsearch},
   {"findresources", &WorkerProgram::parse_findresources},
   {"scout", &WorkerProgram::parse_scout},
   {"playsound", &WorkerProgram::parse_playsound},
   {"construct", &WorkerProgram::parse_construct},
   {"terraform", &WorkerProgram::parse_terraform},

   {nullptr, nullptr}};

/**
 * Parse a program
 */
WorkerProgram::WorkerProgram(const std::string& init_name,
                             const LuaTable& actions_table,
                             const WorkerDescr& worker,
                             Descriptions& descriptions)
   : MapObjectProgram(init_name), worker_(worker), descriptions_(descriptions) {

	for (const std::string& line : actions_table.array_entries<std::string>()) {
		if (line.empty()) {
			throw GameDataError("Empty line");
		}
		try {

			ProgramParseInput parseinput = parse_program_string(line);

			// Find the appropriate parser
			Worker::Action act;
			uint32_t mapidx;

			for (mapidx = 0; parsemap_[mapidx].name != nullptr; ++mapidx) {
				if (parseinput.name == parsemap_[mapidx].name) {
					break;
				}
			}

			if (parsemap_[mapidx].name == nullptr) {
				throw GameDataError(
				   "Unknown command '%s' in line '%s'", parseinput.name.c_str(), line.c_str());
			}

			(this->*parsemap_[mapidx].function)(&act, parseinput.arguments);

			actions_.push_back(act);
		} catch (const std::exception& e) {
			throw GameDataError("Error reading line '%s': %s", line.c_str(), e.what());
		}
	}
	if (actions_.empty()) {
		throw GameDataError("No actions found");
	}
}

/* RST
createware
^^^^^^^^^^
.. function:: createware=\<ware_name\>

   :arg string ware_name: The ware type to create, e.g. ``wheat``.

   The worker will create and carry a ware of the given type. Example::

      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe 220",
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:4s",
         "createware=wheat", -- Create 1 wheat and start carrying it
         "return"
      },
*/
/**
 * iparam1 = ware index
 */
void WorkerProgram::parse_createware(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1) {
		throw wexception("Usage: createware=<ware type>");
	}

	const DescriptionIndex ware_index = descriptions_.load_ware(cmd[0]);

	act->function = &Worker::run_createware;
	act->iparam1 = ware_index;
	produced_ware_types_.insert(ware_index);
}

/* RST
mine
^^^^
.. function:: mine=\<resource_name\> radius:\<number\>

   :arg string resource_name: The map resource to mine, e.g. ``fish``.

   :arg int radius: After the worker has found a spot, the radius that is scanned for decreasing the
      map resource, e.g. ``1``.

   Mine on the current coordinates that the worker has walked to for resources decrease.
   Example::

      fish = {
         "findspace=size:any radius:7 resource:resource_fish",
         "walk=coords",
         "playsound=sound/fisher/fisher_throw_net 192",
         "mine=resource_fish radius:1", -- Remove a fish in an area of 1
         "animate=fishing duration:3s",
         "playsound=sound/fisher/fisher_pull_net 192",
         "createware=fish",
         "return"
      },
*/
/**
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_mine(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2) {
		throw GameDataError("Usage: mine=<resource_name> radius:<number>");
	}

	act->function = &Worker::run_mine;

	if (read_key_value_pair(cmd[1], ':').second.empty()) {
		// TODO(GunChleoc): Compatibility, remove this option after v1.0
		log_warn("'mine' program without parameter names is deprecated, please use "
		         "'mine=<resource_name> radius:<number>' in %s\n",
		         worker_.name().c_str());
		act->sparam1 = cmd[0];
		act->iparam1 = read_positive(cmd[1]);
	} else {
		for (const std::string& argument : cmd) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
			if (item.first == "radius") {
				act->iparam1 = read_positive(item.second);
			} else if (item.second.empty()) {
				act->sparam1 = item.first;
			} else {
				throw GameDataError(
				   "Unknown parameter '%s'. Usage: mine=<resource_name> radius:<number>",
				   item.first.c_str());
			}
		}
	}
	Notifications::publish(
	   NoteMapObjectDescription(act->sparam1, NoteMapObjectDescription::LoadType::kObject));
}

/* RST
breed
^^^^^
.. function:: breed=\<resource_name\> radius:\<number\>

   :arg string resource_name: The map resource to breed, e.g. ``fish``.

   :arg int radius: After the worker has found a spot, the radius that is scanned for increasing the
      map resource, e.g. ``1``.

   Breed a resource on the current coordinates that the worker has walked to for
   resources increase. Example::

      breed = {
         "findspace=size:any radius:7 breed resource:resource_fish",
         "walk=coords",
         "animate=freeing duration:3s",
         "breed=resource_fish radius:1", -- Add a fish in an area of 1
         "return"
      },
*/
/**
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_breed(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2) {
		throw GameDataError("Usage: breed=<resource_name> radius:<number>");
	}

	act->function = &Worker::run_breed;

	if (read_key_value_pair(cmd[1], ':').second.empty()) {
		// TODO(GunChleoc): Compatibility, remove this option after v1.0
		log_warn("'breed' program without parameter names is deprecated, please use "
		         "'breed=<resource_name> radius:<number>' in %s\n",
		         worker_.name().c_str());
		act->sparam1 = cmd[0];
		act->iparam1 = read_positive(cmd[1]);
	} else {
		for (const std::string& argument : cmd) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
			if (item.first == "radius") {
				act->iparam1 = read_positive(item.second);
			} else if (item.second.empty()) {
				act->sparam1 = item.first;
			} else {
				throw GameDataError(
				   "Unknown parameter '%s'. Usage: breed=<resource_name> radius:<number>",
				   item.first.c_str());
			}
		}
	}
	Notifications::publish(
	   NoteMapObjectDescription(act->sparam1, NoteMapObjectDescription::LoadType::kObject));
}

/* RST
findobject
^^^^^^^^^^
.. function:: findobject=radius:\<distance\> [type:\<map_object_type\>] [attrib:\<attribute\>]
   [no_notify]

   :arg int radius: Search for an object within the given radius around the worker.
   :arg string type: The type of map object to search for. Defaults to ``immovable``.
   :arg string attrib: The attribute that the map object should possess.
   :arg empty no_notify: Do not send a message to the player if this step fails.

   Find and select an object based on a number of predicates, which can be specified
   in arbitrary order. The object can then be used in other commands like ``walk``
   or ``callobject``. Examples::

      cut_granite = {
         "findobject=attrib:rocks radius:6", -- Find rocks on the map within a radius of 6 from your
         building
         "walk=object", -- Now walk to those rocks
         "playsound=sound/atlanteans/cutting/stonecutter 192",
         "animate=hacking duration:12s",
         "callobject=shrink",
         "createware=granite",
         "return"
      },

      hunt = {
         "findobject=type:bob radius:13 attrib:eatable", -- Find an eatable bob (animal) within a
         radius of 13 from your building
         "walk=object", -- Walk to where the animal is
         "animate=idle duration:1s500ms",
         "removeobject",
         "createware=meat",
         "return"
      },
*/
/**
 * iparam1 = radius predicate
 * iparam2 = attribute predicate (if >= 0)
 * iparam3 = send message on failure (if != 0)
 * sparam1 = type
 */
void WorkerProgram::parse_findobject(Worker::Action* act, const std::vector<std::string>& cmd) {
	act->function = &Worker::run_findobject;
	act->iparam1 = -1;
	act->iparam2 = -1;
	act->iparam3 = 1;
	act->sparam1 = "immovable";

	// Parse predicates
	for (const std::string& argument : cmd) {
		if (argument == "no_notify") {
			act->iparam3 = 0;
			continue;
		}

		const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');

		if (item.first == "radius") {
			act->iparam1 = read_positive(item.second);
		} else if (item.first == "attrib") {
			Notifications::publish(
			   NoteMapObjectDescription(item.second, NoteMapObjectDescription::LoadType::kAttribute));
			act->iparam2 = MapObjectDescr::get_attribute_id(item.second);
		} else if (item.first == "type") {
			act->sparam1 = item.second;
		} else {
			throw GameDataError("Unknown findobject predicate %s", argument.c_str());
		}
	}

	if (act->iparam2 >= 0) {
		needed_attributes_.insert(
		   std::make_pair(act->sparam1 == "immovable" ? MapObjectType::IMMOVABLE : MapObjectType::BOB,
		                  act->iparam2));
	}

	workarea_info_[act->iparam1].insert(" findobject");
}

/* RST
findspace
^^^^^^^^^
.. function:: findspace=size:\<plot\> radius:\<distance\> [breed] [resource:\<name\>]
   [avoid:\<immovable_attribute\>] [saplingsearches:\<number\>] [space] [ferry]
   [terraform:\<category\>] [no_notify]

   :arg string size: The size or building plot type of the free space.
      The possible values are:

      * ``any``: Any size will do.
      * ``build``: Any building plot.
      * ``small``: Small building plots only.
      * ``medium``: Medium building plots only.
      * ``big``: Big building plots only.
      * ``mine``: Mining plots only.
      * ``port``: Port spaces only.
      * ``swim``: Anything on the coast.

   :arg int radius: Search for map fields within the given radius around the worker.

   :arg empty breed: Used in front of ``resource`` only: Also accept fields where the
      resource has been depleted. Use this when looking for a place for breeding.

   :arg string resource: A resource to search for. This is mainly intended for
      fishers and suchlike, for non-detectable resources and default resources.

   :arg string avoid: A field containing an immovable that has this attribute will
      not be used.

   :arg int saplingsearches: The higher the number, the better the accuracy
      for finding a better spot for immovables that have terrain affinity, e.g. trees.

   :arg empty space: Find only fields that are walkable in such a way that all
      neighbors are also walkable (an exception is made if one of the neighboring
      fields is owned by this worker's location).

   :arg empty ferry: Find only fields reachable by a ferry.

   :arg string terraform: Find only nodes where at least one adjacent triangle has
      terrain that can be enhanced.

   :arg empty no_notify: Do not send a message to the player if this step fails.

   Find a map field based on a number of predicates.
   The field can then be used in other commands like ``walk``. Examples::

      breed = {
         -- Find any field that can have fish in it for adding a fish to it below
         "findspace=size:any radius:7 breed resource:resource_fish",
         "walk=coords",
         "animate=freeing duration:3s",
         "breed=resource_fish 1",
         "return"
      },

      plant = {
         -- Don't get in the way of the farmer's crops when planting trees. Retry 8 times.
         "findspace=size:any radius:5 avoid:field saplingsearches:8",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:tree_sapling",
         "animate=water duration:2s",
         "return"
      },

      plant = {
         -- The farmer will want to walk to this field again later for harvesting his crop
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=planting duration:4s",
         "plant=attrib:seed_wheat",
         "animate=planting duration:4s",
         "return",
      },
*/
/**
 * iparam1 = radius
 * iparam2 = FindNodeSize::sizeXXX
 * iparam3 = Bitset of "space" (1st bit), "no_notify" (2nd bit), and "ferry" (3rd bit).
 * iparam4 = whether the "breed" flag is set
 * iparam5 = Immovable attribute id
 * iparam6 = Forester retries
 * sparam1 = Resource
 * sparamv = The terraform category (if any)
 */
// TODO(Nordfriese): All boolean flags (space. breed, no_notify) should be placed in
// just one iparam. Unfortunately there is no way to have saveloading versioning here.
// Refactor this when we next break savegame compatibility completely.
void WorkerProgram::parse_findspace(Worker::Action* act, const std::vector<std::string>& cmd) {
	act->function = &Worker::run_findspace;
	act->iparam1 = -1;
	act->iparam2 = -1;
	act->iparam3 = 0;
	act->iparam4 = 0;
	act->iparam5 = -1;
	act->iparam6 = 1;
	act->iparam7 = 0;
	act->sparam1 = "";
	act->sparamv = {};

	// Parse predicates
	for (const std::string& argument : cmd) {
		try {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');

			if (item.first == "radius") {
				act->iparam1 = read_positive(item.second);
			} else if (item.first == "size") {
				static const std::map<std::string, FindNodeSize::Size> sizenames{
				   {"any", FindNodeSize::sizeAny},     {"build", FindNodeSize::sizeBuild},
				   {"small", FindNodeSize::sizeSmall}, {"medium", FindNodeSize::sizeMedium},
				   {"big", FindNodeSize::sizeBig},     {"mine", FindNodeSize::sizeMine},
				   {"port", FindNodeSize::sizePort},   {"swim", FindNodeSize::sizeSwim}};

				if (sizenames.count(item.second) != 1) {
					throw GameDataError("Bad findspace size '%s'", item.second.c_str());
				}
				act->iparam2 = sizenames.at(item.second);
			} else if (item.first == "breed") {
				act->iparam4 = 1;
			} else if (item.first == "terraform") {
				act->sparamv.push_back(item.second);
			} else if (item.first == "resource") {
				act->sparam1 = item.second;
			} else if (item.first == "space") {
				act->iparam3 |= 1 << 0;
			} else if (item.first == "no_notify") {
				act->iparam3 |= 1 << 1;
			} else if (item.first == "ferry") {
				act->iparam3 |= 1 << 2;
			} else if (item.first == "avoid") {
				act->iparam5 = MapObjectDescr::get_attribute_id(item.second);
			} else if (item.first == "saplingsearches") {
				act->iparam6 = read_int(item.second, 2);
			} else {
				throw GameDataError("Unknown findspace predicate %s", item.first.c_str());
			}
		} catch (const GameDataError& e) {
			throw GameDataError("Malformed findspace argument %s: %s", argument.c_str(), e.what());
		}
	}

	if (act->iparam1 <= 0) {
		throw GameDataError("findspace: must specify radius");
	}
	if (act->iparam2 < 0) {
		throw GameDataError("findspace: must specify size");
	}
	workarea_info_[act->iparam1].insert(" findspace");

	if (!act->sparam1.empty()) {
		Notifications::publish(
		   NoteMapObjectDescription(act->sparam1, NoteMapObjectDescription::LoadType::kObject));
		if (act->iparam4 == 1) {
			// breeds
			created_resources_.insert(act->sparam1);
		} else {
			collected_resources_.insert(act->sparam1);
		}
	}
}

/* RST
walk
^^^^
.. function:: walk=\<destination_type\>

   :arg string destination_type: Defines where to walk to. Possible destinations are:

      * ``object``: Walk to a previously found and selected object.
      * ``coords``: Walk to a previously found and selected field/coordinate.
      * ``object-or-coords``: Walk to a previously found and selected object if present;
        otherwise to previously found and selected field/coordinate.

   Walk to a previously selected destination. Examples::

      plant = {
         "findspace=size:any radius:2",
         "walk=coords", -- Walk to the space found by the command above
         "animate=planting duration:4s",
         "plant=attrib:seed_blackroot",
         "animate=planting duration:4s",
         "return"
      },

      harvest = {
         "findobject=attrib:ripe_blackroot radius:2",
         "walk object", -- Walk to the blackroot field found by the command above
         "animate=harvesting duration:10s",
         "callobject=harvest",
         "animate=gathering duration:2s",
         "createware=blackroot",
         "return"
      },

      buildship = {
         "walk=object-or-coords", -- Walk to coordinates from 1. or to object from 2.
         -- 2. This will create an object for us if we don't have one yet
         "plant=attrib:atlanteans_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=work duration:500ms",
         "construct", -- 1. This will find a space for us if no object has been planted yet
         "animate=work duration:5s",
         "return"
      },
*/
/**
 * iparam1 = walkXXX
 */
void WorkerProgram::parse_walk(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1) {
		throw GameDataError("Usage: walk=object|coords|object-or-coords");
	}

	act->function = &Worker::run_walk;

	if (cmd[0] == "object") {
		act->iparam1 = Worker::Action::walkObject;
	} else if (cmd[0] == "coords") {
		act->iparam1 = Worker::Action::walkCoords;
	} else if (cmd[0] == "object-or-coords") {
		act->iparam1 = Worker::Action::walkObject | Worker::Action::walkCoords;
	} else {
		throw GameDataError("Bad walk destination '%s'", cmd[0].c_str());
	}
}

/* RST
animate
^^^^^^^
Runs an animation. See :ref:`map_object_programs_animate`.
*/
/**
 * iparam1 = anim id
 * iparam2 = duration
 */
void WorkerProgram::parse_animate(Worker::Action* act, const std::vector<std::string>& cmd) {
	AnimationParameters parameters = MapObjectProgram::parse_act_animate(cmd, worker_, true);

	act->function = &Worker::run_animate;
	// If the second parameter to MapObjectDescr::get_animation is ever used for anything other than
	// level-dependent soldier animations, or we want to write a worker program for a soldier,
	// we will need to store the animation name as a string in an sparam
	act->iparam1 = parameters.animation;
	act->iparam2 = parameters.duration.get();
}

/* RST
return
^^^^^^
.. function:: return

   Return home and then drop any ware we're carrying onto our building's flag. Example::

      scout = {
         "scout=15 75000",
         "return" -- Go home
      }
*/
/**
 * iparam1 = 0: don't drop ware on flag, 1: do drop ware on flag
 */
void WorkerProgram::parse_return(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (!cmd.empty()) {
		throw GameDataError("Usage: return");
	}
	act->function = &Worker::run_return;
	act->iparam1 = 1;  // drop a ware on our owner's flag
}

/* RST
callobject
^^^^^^^^^^^
.. function:: callobject=\<program_name\>

   :arg string program_name: The name of the program to be executed.

   Cause the currently selected object to execute its given program. Example::

      harvest = {
         "findobject=attrib:tree radius:10",
         "walk=object",
         "playsound=sound/woodcutting/fast_woodcutting 250",
         "animate=hacking duration:10s",
         "playsound=sound/woodcutting/tree-falling 130",
         "callobject=fall", -- Cause the tree to fall
         "animate=idle duration:2s",
         "createware=log",
         "return"
      }

   See also :ref:`immovable_programs`.
*/
/**
 * sparam1 = callobject command name
 */
void WorkerProgram::parse_callobject(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1) {
		throw GameDataError("Usage: callobject=<program name>");
	}

	act->function = &Worker::run_callobject;
	act->sparam1 = cmd[0];

	// TODO(Gunchleoc): We might need to dig into the called object's program too, but this is good
	// enough for now.
	if (!needed_attributes_.empty()) {
		collected_attributes_.insert(needed_attributes_.begin(), needed_attributes_.end());
	}
}

/* RST
plant
^^^^^
.. function:: plant attrib:\<attribute\> [attrib:\<attribute\> ...] [unless object]

   :arg string attrib\:\<attribute\>: Select at random any immovable
      that has this attribute.

   :arg empty unless object: Do not plant the immovable if it already exists at
      the current position.

   Plant one of the given immovables on the current position, taking into account
   the fertility of the area. Examples::

      plant = {
         "findspace=size:any radius:5 avoid:field",
         "walk=coords",
         "animate=dig duration:2s",
         "animate=planting duration:1s",
         "plant=attrib:tree_sapling", -- Plant any random sapling tree
         "animate=water duration:2s",
         "return"
      },

      plant = {
         "findspace=size:any radius:2 space",
         "walk=coords",
         "animate=planting duration:4s",
         -- Plant the tiny field immovable that the worker's tribe knows about
         "plant=attrib:seed_wheat",
         "animate=planting duration:4s",
         "return",
      },

      buildship = {
         "walk=object-or-coords",
         -- Only create a shipconstruction if we don't already have one
         "plant=attrib:barbarians_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=work duration:500ms",
         "construct",
         "animate=work duration:5s",
         "return"
      }
*/
/**
 * sparamv  list of attributes
 * iparam1  one of plantXXX
 */
void WorkerProgram::parse_plant(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.empty()) {
		throw GameDataError(
		   "Usage: plant=attrib:<attribute> [attrib:<attribute> ...] [unless object]");
	}

	act->function = &Worker::run_plant;
	act->iparam1 = Worker::Action::plantAlways;
	for (uint32_t i = 0; i < cmd.size(); ++i) {
		if (cmd[i] == "unless") {
			++i;
			if (i >= cmd.size()) {
				throw GameDataError("plant: something expected after 'unless'");
			}
			if (cmd[i] == "object") {
				act->iparam1 = Worker::Action::plantUnlessObject;
			} else {
				throw GameDataError("plant: 'unless %s' not understood", cmd[i].c_str());
			}
			continue;
		}

		const std::string attrib_name = read_key_value_pair(cmd[i], ':', "", "attrib").second;
		Notifications::publish(
		   NoteMapObjectDescription(attrib_name, NoteMapObjectDescription::LoadType::kAttribute));
		act->sparamv.push_back(attrib_name);
		// get_attribute_id will throw a GameDataError if the attribute doesn't exist.
		created_attributes_.insert(
		   std::make_pair(MapObjectType::IMMOVABLE, ImmovableDescr::get_attribute_id(attrib_name)));
	}
}

/* RST
createbob
^^^^^^^^^
.. function:: createbob=\<bob_name\> [\<bob_name\> ...]

   :arg string bob_name: The bob type to add to the selection. Specify as many bob
      types as you want.

   Adds a bob (an animal or a worker, e.g. a deer or a ferry) to the map at the worker's current
   location. Randomly select from the list of ``bob_name``. Examples::

      release = {
         "findspace=size:any radius:3",
         "walk=coords",
         "animate=releasein duration:2s",
         "createbob=wildboar stag sheep", -- Release a wildboar, stag or sheep into the wild
         "animate=releaseout duration:2s",
         "return"
      },

      buildferry = {
         "findspace=size:swim radius:5",
         "walk=coords",
         "animate=work duration:10s",
         "createbob=frisians_ferry",
         "return"
      }
*/
// TODO(GunChleoc): attrib:eatable would be much better, then depend on terrain too
void WorkerProgram::parse_createbob(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.empty()) {
		throw GameDataError("Usage: createbob=<bob name> <bob name> ...");
	}

	act->function = &Worker::run_createbob;
	act->sparamv = cmd;

	// Register created bobs
	for (const std::string& bobname : act->sparamv) {
		created_bobs_.insert(bobname);
		Notifications::publish(
		   NoteMapObjectDescription(bobname, NoteMapObjectDescription::LoadType::kObject));
	}
}

/* RST
buildferry
^^^^^^^^^^
.. function:: buildferry

   **DEPRECATED** use ``createbob=TRIBENAME_ferry`` instead.
*/
void WorkerProgram::parse_buildferry(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() > 1) {
		throw wexception("buildferry takes no arguments");
	}
	// TODO(GunChleoc): API compatibility - remove after v1.0
	log_warn("%s: Worker program 'buildferry' is deprecated. Use createbob=TRIBENAME_ferry instead.",
	         worker_.name().c_str());
	act->function = &Worker::run_buildferry;
}

/* RST
terraform
^^^^^^^^^
.. function:: terraform=\<category\>

   Turns the terrain of one of the triangles around the current node into its
   enhancement terrain. Example::

      terraform = {
         "findspace=size:any radius:6 terraform:amazons",
         "walk=coords",
         "animate=dig duration:2s",
         "terraform=amazons",
         "return"
      }
*/
void WorkerProgram::parse_terraform(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1) {
		throw wexception("Usage: terraform=<category>");
	}
	act->function = &Worker::run_terraform;
	act->sparam1 = cmd[0];
}

/* RST
removeobject
^^^^^^^^^^^^
.. function:: removeobject

   Remove the currently selected object. Example::

      hunt = {
         "findobject=type:bob radius:13 attrib:eatable", -- Select an object to remove
         "walk=object",
         "animate=idle duration:1s",
         -- The selected eatable map object has been hunted, so remove it from the map
         "removeobject",
         "createware=meat",
         "return"
      }
*/
void WorkerProgram::parse_removeobject(Worker::Action* act,
                                       const std::vector<std::string>& /* cmd */) {
	act->function = &Worker::run_removeobject;
	if (!needed_attributes_.empty()) {
		collected_attributes_.insert(needed_attributes_.begin(), needed_attributes_.end());
	}
}

/* RST
repeatsearch
^^^^^^^^^^^^
.. function:: repeatsearch=\<program_name\> repetitions:\<number\> radius:\<number\>

   :arg string program_name: The name of the program to repeatedly call from this program.

   :arg int repetitions: The number of times that the worker will move to a
      different spot on the map to execute ``program_name``. Used by geologists.

   :arg int radius: The radius of map fields for the worker not to stray from.

   Walk around the starting point randomly within a certain radius, and execute
   your ``program_name`` for some of the fields. Example::

      expedition = {
         "repeatsearch=search repetitions:15 radius:5"
      },
*/
/**
 * iparam1 = maximum repeat #
 * iparam2 = radius
 * sparam1 = subcommand
 */
void WorkerProgram::parse_repeatsearch(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 3) {
		throw GameDataError("Usage: repeatsearch=<program_name> repetitions<number> radius:<number>");
	}

	act->function = &Worker::run_repeatsearch;

	if (read_key_value_pair(cmd[1], ':').second.empty()) {
		// TODO(GunChleoc): Compatibility, remove this option after v1.0
		log_warn("'repeatsearch' program without parameter names is deprecated, please use "
		         "'repeatsearch=<program_name> repetitions:<number> radius:<number>' in %s\n",
		         worker_.name().c_str());
		act->iparam1 = read_positive(cmd[0]);
		act->iparam2 = read_positive(cmd[1]);
		act->sparam1 = cmd[2];
	} else {
		for (const std::string& argument : cmd) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
			if (item.first == "repetitions") {
				act->iparam1 = read_positive(item.second);
			} else if (item.first == "radius") {
				act->iparam2 = read_positive(item.second);
			} else if (item.second.empty()) {
				act->sparam1 = item.first;
			} else {
				throw GameDataError("Unknown parameter '%s'. Usage: repeatsearch=<program_name> "
				                    "repetitions:<number> radius:<number>",
				                    item.first.c_str());
			}
		}
	}
}

/* RST
findresources
^^^^^^^^^^^^^
.. function:: findresources

   Check the current position for map resources (e.g. coal or water), and plant
   a marker object when possible. Example::

      search = {
         "animate=hacking duration:5s",
         "animate=idle duration:2s",
         "playsound=sound/hammering/geologist_hammer 192",
         "animate=hacking duration:3s",
         -- Plant a resource marker at the current location, according to what has been found.
         "findresources"
      }
*/
void WorkerProgram::parse_findresources(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (!cmd.empty()) {
		throw GameDataError("Usage: findresources");
	}

	act->function = &Worker::run_findresources;
}

/* RST
scout
^^^^^
.. function:: scout=radius:\<number\> duration:\<duration\>

   :arg int radius: The radius of map fields for the scout to explore.

   :arg duration duration: The time :ref:`map_object_programs_datatypes_duration` that the scout
      will spend scouting.

   Sends a scout out to run around scouting the area. Example::

      scout = {
         "scout=radius:15 duration:1m15s",
         "return"
      },
*/
/**
 * iparam1 = radius
 * iparam2 = time
 */
void WorkerProgram::parse_scout(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2) {
		throw GameDataError("Usage: scout=radius:<number> duration:<duration>");
	}
	act->function = &Worker::run_scout;

	if (read_key_value_pair(cmd[0], ':').second.empty()) {
		// TODO(GunChleoc): Compatibility, remove this option after v1.0
		log_warn("'scout' program without parameter names is deprecated, please use "
		         "'scout=radius:<number> duration:<duration>' in %s\n",
		         worker_.name().c_str());
		act->iparam1 = read_positive(cmd[0]);
		act->iparam2 = read_positive(cmd[1]);
	} else {
		for (const std::string& argument : cmd) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
			if (item.first == "radius") {
				act->iparam1 = read_positive(item.second);
			} else if (item.first == "duration") {
				act->iparam2 = read_duration(item.second, worker_).get();
			} else {
				throw GameDataError(
				   "Unknown parameter '%s'. Usage: scout=radius:<number> duration:<duration>",
				   item.first.c_str());
			}
		}
	}
}

/* RST
playsound
^^^^^^^^^^
Plays a sound effect. See :ref:`map_object_programs_playsound`.
*/
void WorkerProgram::parse_playsound(Worker::Action* act, const std::vector<std::string>& cmd) {
	//  50% chance to play, only one instance at a time
	PlaySoundParameters parameters = MapObjectProgram::parse_act_play_sound(cmd, worker_);

	act->iparam1 = parameters.priority;
	act->iparam2 = parameters.fx;
	act->iparam3 = parameters.allow_multiple ? 1 : 0;
	act->function = &Worker::run_playsound;
}

/* RST
construct
^^^^^^^^^
.. function:: construct

   Give the ware currently held by the worker to the immovable object for construction.
   This is used in ship building. Example::

      buildship = {
         "walk=object-or-coords", -- Walk to coordinates from 1. or to object from 2.
         -- 2. This will create an object for us if we don't have one yet
         "plant=attrib:atlanteans_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill 230",
         "animate=work duration:5s",
         -- 1. Add the current ware to the shipconstruction. This will find a space for us if no
         -- shipconstruction object has been planted yet
         "construct",
         "animate=work duration:5s",
         "return"
      },
*/
/**
 * construct
 *
 * Give the currently held ware of the worker to the \ref objvar1 immovable
 * for construction. This is used in ship building.
 */
void WorkerProgram::parse_construct(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (!cmd.empty()) {
		throw GameDataError("Usage: construct");
	}

	act->function = &Worker::run_construct;
}
}  // namespace Widelands
