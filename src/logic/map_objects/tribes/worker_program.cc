/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "logic/map_objects/tribes/worker_program.h"

#include <memory>
#include <string>

#include "base/log.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "logic/findnode.h"
#include "logic/game_data_error.h"
#include "sound/sound_handler.h"

namespace Widelands {
/* RST
.. _tribes_worker_programs:

Worker Programs
===============

Worker programs are defined in the ``programs`` subtable specified in calls to
:ref:`tribes:new_worker_type <lua_tribes_workers_common>`.
Each worker program is a Lua table in itself and defined as a series of command strings.
Commands can also have parameters, which are separated from each other by a blank space.
These parameters can also have values, which are separated from the parameter name by a colon (:).
The table looks like this::

   programs = {
      program_name1 = {
         "command1 parameter1:value1 parameter2:value2",
         "command2 parameter1",
         "return"
      },
      program_name2 = {
         "command3",
         "command4 parameter1 parameter2 parameter3",
         "return"
      }
   },

The available programs are:

- `createware`_
- `mine`_
- `breed`_
- `setbobdescription`_
- `findobject`_
- `findspace`_
- `walk`_
- `animation`_
- `return`_
- `object`_
- `plant`_
- `create_bob`_
- `object remove`_
- `geologist`_
- `geologist_find`_
- `scout`_
- `play_sound`_
- `construct`_
*/

const WorkerProgram::ParseMap WorkerProgram::parsemap_[] = {
   {"mine", &WorkerProgram::parse_mine},
   {"breed", &WorkerProgram::parse_breed},
   {"createware", &WorkerProgram::parse_createware},
   {"setbobdescription", &WorkerProgram::parse_setbobdescription},
   {"findobject", &WorkerProgram::parse_findobject},
   {"findspace", &WorkerProgram::parse_findspace},
   {"walk", &WorkerProgram::parse_walk},
   {"animation", &WorkerProgram::parse_animation},
   {"return", &WorkerProgram::parse_return},
   {"object", &WorkerProgram::parse_object},
   {"plant", &WorkerProgram::parse_plant},
   {"create_bob", &WorkerProgram::parse_create_bob},
   {"removeobject", &WorkerProgram::parse_removeobject},
   {"geologist", &WorkerProgram::parse_geologist},
   {"geologist_find", &WorkerProgram::parse_geologist_find},
   {"scout", &WorkerProgram::parse_scout},
   {"play_sound", &WorkerProgram::parse_play_sound},
   {"construct", &WorkerProgram::parse_construct},

   {nullptr, nullptr}};

/**
 * Parse a program
 */
void WorkerProgram::parse(const LuaTable& table) {
	for (const std::string& string : table.array_entries<std::string>()) {
		if (string.empty()) {
			log("Worker program %s for worker %s contains empty string\n", name_.c_str(),
			    worker_.name().c_str());
			break;
		}
		try {
			const std::vector<std::string> cmd(split_string(string, " \t\r\n"));
			if (cmd.empty()) {
				continue;
			}

			// Find the appropriate parser
			Worker::Action act;
			uint32_t mapidx;

			for (mapidx = 0; parsemap_[mapidx].name; ++mapidx) {
				if (cmd[0] == parsemap_[mapidx].name) {
					break;
				}
			}

			if (!parsemap_[mapidx].name) {
				throw wexception("unknown command type \"%s\"", cmd[0].c_str());
			}

			(this->*parsemap_[mapidx].function)(&act, cmd);

			actions_.push_back(act);
		} catch (const std::exception& e) {
			throw wexception("Error reading line '%s' in worker program %s for worker %s: %s",
			                 string.c_str(), name_.c_str(), worker_.name().c_str(), e.what());
		}
	}
}

/* RST
createware
^^^^^^^^^^
.. function:: createware \<ware_name\>

   :arg string ware_name: The ware type to create, e.g. ``wheat``.

   The worker will create and carry a ware of the given type. Example::

      harvest = {
         "findobject attrib:ripe_wheat radius:2",
         "walk object",
         "play_sound sound/farm scythe 220",
         "animation harvesting 10000",
         "object harvest",
         "animation gathering 4000",
         "createware wheat", -- Create 1 wheat and start carrying it
         "return"
      },
*/
/**
 * iparam1 = ware index
 */
void WorkerProgram::parse_createware(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2)
		throw wexception("Usage: createware <ware type>");

	act->function = &Worker::run_createware;
	act->iparam1 = tribes_.safe_ware_index(cmd[1]);
}

/* RST
mine
^^^^
.. function:: mine \<resource_name\> \<area\>

   :arg string resource_name: The map resource to mine, e.g. ``fish``.

   :arg int area: The radius that is scanned for decreasing the map resource, e.g. ``1``.

   Mine on the current coordinates that the worker has walked to for resources decrease.
   Example::

      fish = {
         "findspace size:any radius:7 resource:fish",
         "walk coords",
         "play_sound sound/fisher fisher_throw_net 192",
         "mine fish 1", -- Remove a fish in an area of 1
         "animation fishing 3000",
         "play_sound sound/fisher fisher_pull_net 192",
         "createware fish",
         "return"
      },
*/
/**
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_mine(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 3)
		throw wexception("Usage: mine <ware type> <area>");

	act->function = &Worker::run_mine;
	act->sparam1 = cmd[1];
	char* endp;
	act->iparam1 = strtol(cmd[2].c_str(), &endp, 0);

	if (*endp)
		throw wexception("Bad area '%s'", cmd[2].c_str());
}

/* RST
breed
^^^^^
.. function:: breed \<resource_name\> \<area\>

   :arg string resource_name: The map resource to breed, e.g. ``fish``.

   :arg int area: The radius that is scanned for increasing the map resource, e.g. ``1``.

   Breed a resource on the current coordinates that the worker has walked to for
   resources increase. Example::

      breed = {
         "findspace size:any radius:7 breed resource:fish",
         "walk coords",
         "animation freeing 3000",
         "breed fish 1", -- Add a fish in an area of 1
         "return"
      },
*/
/**
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_breed(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 3)
		throw wexception("Usage: breed <ware type> <area>");

	act->function = &Worker::run_breed;
	act->sparam1 = cmd[1];
	char* endp;
	act->iparam1 = strtol(cmd[2].c_str(), &endp, 0);

	if (*endp)
		throw wexception("Bad area '%s'", cmd[2].c_str());
}

/* RST
setbobdescription
^^^^^^^^^^^^^^^^^
.. function:: setbobdescription \<bob_name\> [\<bob_name\> ...]

   :arg string bob_name: The bob type to add to the selection. Specify as many bob
      types as you want.

   Randomly select a bob name that can be used in subsequent commands,
   e.g. create_bob. Used for releasing animals. Example::

      release = {
         "setbobdescription wildboar stag sheep", -- A wildboar, stag or sheep will be selected
         "findspace size:any radius:3",
         "walk coords",
         "animation releasein 2000",
         "create_bob",
         "animation releaseout 2000",
         "return"
      },
*/
/**
 * sparamv = possible bobs
 */
void WorkerProgram::parse_setbobdescription(Worker::Action* act,
                                            const std::vector<std::string>& cmd) {
	if (cmd.size() < 2)
		throw wexception("Usage: setbobdescription <bob name> <bob name> ...");

	act->function = &Worker::run_setbobdescription;

	for (uint32_t i = 1; i < cmd.size(); ++i)
		act->sparamv.push_back(cmd[i]);
}

/* RST
findobject
^^^^^^^^^^
.. function:: findobject radius:\<distance\> [type:\<map_object_type\>] [attrib:\<attribute\>]

   :arg int radius: Search for an object within the given radius around the worker.
   :arg string type: The type of map object to search for. Defaults to ``immovable``.
   :arg string attrib: The attribute that the map object should possess.

   Find and select an object based on a number of predicates, which can be specified
   in arbitrary order. The object can then be used in other commands like ``walk``
   or ``object``. Examples::

      cut_granite = {
         "findobject attrib:rocks radius:6", -- Find rocks on the map within a radius of 6 from your building
         "walk object", -- Now walk to those rocks
         "play_sound sound/atlanteans/cutting stonecutter 192",
         "animation hacking 12000",
         "object shrink",
         "createware granite",
         "return"
      },

      hunt = {
         "findobject type:bob radius:13 attrib:eatable", -- Find an eatable bob (animal) within a radius of 13 from your building
         "walk object", -- Walk to where the animal is
         "animation idle 1500",
         "object remove",
         "createware meat",
         "return"
      },
*/
/**
 * iparam1 = radius predicate
 * iparam2 = attribute predicate (if >= 0)
 * sparam1 = type
 */
void WorkerProgram::parse_findobject(Worker::Action* act, const std::vector<std::string>& cmd) {
	uint32_t i;

	act->function = &Worker::run_findobject;
	act->iparam1 = -1;
	act->iparam2 = -1;
	act->sparam1 = "immovable";

	// Parse predicates
	for (i = 1; i < cmd.size(); ++i) {
		uint32_t idx = cmd[i].find(':');
		std::string const key = cmd[i].substr(0, idx);
		std::string const value = cmd[i].substr(idx + 1);

		if (key == "radius") {
			char* endp;

			act->iparam1 = strtol(value.c_str(), &endp, 0);
			if (*endp)
				throw wexception("Bad findobject radius '%s'", value.c_str());

		} else if (key == "attrib") {
			act->iparam2 = MapObjectDescr::get_attribute_id(value);
		} else if (key == "type") {
			act->sparam1 = value;
		} else
			throw wexception("Bad findobject predicate %s:%s", key.c_str(), value.c_str());
	}

	if (act->iparam1 <= 0)
		throw wexception("findobject: must specify radius");

	workarea_info_[act->iparam1].insert(" findobject");
}

/* RST
findspace
^^^^^^^^^
.. function:: findspace size:\<plot\> radius:\<distance\> [breed] [resource:\<name\>] [avoid:\<immovable_attribute\>] [space]

   :arg string size: The size or building plot type of the free space.
      The possible values are:

      * ``any``: Any size will do.
      * ``build``: Any building plot.
      * ``small``: Small building plots only.
      * ``medium``: Medium building plots only.
      * ``big``: Big building plots only.
      * ``mine``: Mining plots only.
      * ``port``: Port spaces only.

   :arg int radius: Search for map fields within the given radius around the worker.

   :arg empty breed: Used in front of ``resource`` only: Also accept fields where the
      resource has been depleted. Use this when looking for a place for breeding.

   :arg string resource: A resource to search for. This is mainly intended for
      fishers and suchlike, for non-detectable resources and default resources.

   :arg string avoid: A field containing an immovable that has this attribute will
      not be used.

   :arg empty space: Find only fields that are walkable in such a way that all
      neighbors are also walkable (an exception is made if one of the neighboring
      fields is owned by this worker's location).

   Find a map field based on a number of predicates.
   The field can then be used in other commands like ``walk``. Examples::

      breed = {
         "findspace size:any radius:7 breed resource:fish", -- Find any field that can have fish in it for adding a fish to it below
         "walk coords",
         "animation freeing 3000",
         "breed fish 1",
         "return"
      },

      plant = {
         "findspace size:any radius:5 avoid:field", -- Don't get in the way of the farmer's crops when planting trees
         "walk coords",
         "animation dig 2000",
         "animation planting 1000",
         "plant attrib:tree_sapling",
         "animation water 2000",
         "return"
      },

      plant = {
         "findspace size:any radius:2 space", -- The farmer will want to walk to this field again later for harvesting his crop
         "walk coords",
         "animation planting 4000",
         "plant tribe:field_tiny",
         "animation planting 4000",
         "return",
      },
*/
/**
 * iparam1 = radius
 * iparam2 = FindNodeSize::sizeXXX
 * iparam3 = whether the "space" flag is set
 * iparam4 = whether the "breed" flag is set
 * iparam5 = Immovable attribute id
 * sparam1 = Resource
 */
void WorkerProgram::parse_findspace(Worker::Action* act, const std::vector<std::string>& cmd) {
	uint32_t i;

	act->function = &Worker::run_findspace;
	act->iparam1 = -1;
	act->iparam2 = -1;
	act->iparam3 = 0;
	act->iparam4 = 0;
	act->iparam5 = -1;
	act->sparam1 = "";

	// Parse predicates
	for (i = 1; i < cmd.size(); ++i) {
		uint32_t idx = cmd[i].find(':');
		std::string key = cmd[i].substr(0, idx);
		std::string value = cmd[i].substr(idx + 1);

		if (key == "radius") {
			char* endp;

			act->iparam1 = strtol(value.c_str(), &endp, 0);
			if (*endp)
				throw wexception("Bad findspace radius '%s'", value.c_str());

		} else if (key == "size") {
			static const struct {
				char const* name;
				int32_t val;
			} sizenames[] = {{"any", FindNodeSize::sizeAny},     {"build", FindNodeSize::sizeBuild},
			                 {"small", FindNodeSize::sizeSmall}, {"medium", FindNodeSize::sizeMedium},
			                 {"big", FindNodeSize::sizeBig},     {"mine", FindNodeSize::sizeMine},
			                 {"port", FindNodeSize::sizePort},   {nullptr, 0}};

			int32_t index;

			for (index = 0; sizenames[index].name; ++index)
				if (value == sizenames[index].name)
					break;

			if (!sizenames[index].name)
				throw wexception("Bad findspace size '%s'", value.c_str());

			act->iparam2 = sizenames[index].val;
		} else if (key == "breed") {
			act->iparam4 = 1;
		} else if (key == "resource") {
			act->sparam1 = value;
		} else if (key == "space") {
			act->iparam3 = 1;
		} else if (key == "avoid") {
			act->iparam5 = MapObjectDescr::get_attribute_id(value);
		} else
			throw wexception("Bad findspace predicate %s:%s", key.c_str(), value.c_str());
	}

	if (act->iparam1 <= 0)
		throw wexception("findspace: must specify radius");
	if (act->iparam2 < 0)
		throw wexception("findspace: must specify size");
	workarea_info_[act->iparam1].insert(" findspace");
}

/* RST
walk
^^^^
.. function:: walk \<destination_type\>

   :arg string destination_type: Defines where to walk to. Possible destinations are:

      * ``object``: Walk to a previously found and selected object.
      * ``coords``: Walk to a previously found and selected field/coordinate.
      * ``object-or-coords``: Walk to a previously found and selected object if present;
        otherwise to previously found and selected field/coordinate.

   Walk to a previously selected destination. Examples::

      plant = {
         "findspace size:any radius:2",
         "walk coords", -- Walk to the space found by the command above
         "animation planting 4000",
         "plant tribe:blackrootfield_tiny",
         "animation planting 4000",
         "return"
      },

      harvest = {
         "findobject attrib:ripe_blackroot radius:2",
         "walk object", -- Walk to the blackroot field found by the command above
         "animation harvesting 10000",
         "object harvest",
         "animation gathering 2000",
         "createware blackroot",
         "return"
      },

      buildship = {
         "walk object-or-coords", -- Walk to coordinates from 1. or to object from 2.
         "plant tribe:barbarians_shipconstruction unless object", -- 2. This will create an object for us if we don't have one yet
         "play_sound sound/sawmill sawmill 230",
         "animation work 500",
         "construct", -- 1. This will find a space for us if no object has been planted yet
         "animation work 5000",
         "return"
      },
*/
/**
 * iparam1 = walkXXX
 */
void WorkerProgram::parse_walk(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2)
		throw wexception("Usage: walk <where>");

	act->function = &Worker::run_walk;

	if (cmd[1] == "object")
		act->iparam1 = Worker::Action::walkObject;
	else if (cmd[1] == "coords")
		act->iparam1 = Worker::Action::walkCoords;
	else if (cmd[1] == "object-or-coords")
		act->iparam1 = Worker::Action::walkObject | Worker::Action::walkCoords;
	else
		throw wexception("Bad walk destination '%s'", cmd[1].c_str());
}

/* RST
animation
^^^^^^^^^
.. function:: animation \<name\> \<duration\>

   :arg string name: The name of the animation.
   :arg int duration: The time in milliseconds for which the animation will be played.

   Play the given animation for the given duration. Example::

      plantvine = {
         "findspace size:any radius:1",
         "walk coords",
         "animation dig 2000", -- Play a digging animation for 2 seconds.
         "plant tribe:grapevine_tiny",
         "animation planting 3000", -- Play a planting animation for 3 seconds.
         "return"
      },
*/
/**
 * iparam1 = anim id
 * iparam2 = duration
 */
void WorkerProgram::parse_animation(Worker::Action* act, const std::vector<std::string>& cmd) {
	char* endp;

	if (cmd.size() != 3)
		throw GameDataError("Usage: animation <name> <time>");

	if (!worker_.is_animation_known(cmd[1])) {
		throw GameDataError("unknown animation \"%s\" in worker program for worker \"%s\"",
		                    cmd[1].c_str(), worker_.name().c_str());
	}

	act->function = &Worker::run_animation;
	act->iparam1 = worker_.get_animation(cmd[1]);

	act->iparam2 = strtol(cmd[2].c_str(), &endp, 0);
	if (*endp)
		throw GameDataError("Bad duration '%s'", cmd[2].c_str());

	if (act->iparam2 <= 0)
		throw GameDataError("animation duration must be positive");
}

/* RST
return
^^^^^^
.. function:: return

   Return home and then drop any ware we're carrying onto our building's flag. Example::

      scout = {
         "scout 15 75000",
         "return" -- Go home
      }
*/
/**
 * iparam1 = 0: don't drop ware on flag, 1: do drop ware on flag
 */
void WorkerProgram::parse_return(Worker::Action* act, const std::vector<std::string>&) {
	act->function = &Worker::run_return;
	act->iparam1 = 1;  // drop a ware on our owner's flag
}

/* RST
object
^^^^^^
.. function:: object \<program_name\>

   :arg string program_name: The name of the program to be executed.

   Cause the currently selected object to execute its given program. Example::

      chop = {
         "findobject attrib:tree radius:10",
         "walk object",
         "play_sound sound/woodcutting fast_woodcutting 250",
         "animation hacking 10000",
         "play_sound sound/woodcutting tree-falling 130",
         "object fall", -- Cause the tree to fall
         "animation idle 2000",
         "createware log",
         "return"
      }
*/
/**
 * sparam1 = object command name
 */
void WorkerProgram::parse_object(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 2)
		throw wexception("Usage: object <program name>");

	act->function = &Worker::run_object;
	act->sparam1 = cmd[1];
}

/* RST
plant
^^^^^
.. function:: plant \<immmovable_type\> [\<immovable_type\> ...] [unless object]

   :arg string immovable_type: The immovable to be planted. This can be specified
      in two different ways:

      * ``attrib:<attribute>``: Select at random any immovable that has this attribute.
      * ``tribe:<immovable_name>``: Name a specific immovable to be planted.

   :arg empty unless object: Do not plant the immovable if it already exists at
      the current position.

   Plant one of the given immovables on the current position, taking into account
   the fertility of the area. Examples::

      plant = {
         "findspace size:any radius:5 avoid:field",
         "walk coords",
         "animation dig 2000",
         "animation planting 1000",
         "plant attrib:tree_sapling", -- Plant any random sapling tree
         "animation water 2000",
         "return"
      },

      plant = {
         "findspace size:any radius:2 space",
         "walk coords",
         "animation planting 4000",
         "plant tribe:field_tiny", -- Plant the tiny field immovable that the worker's tribe knows about
         "animation planting 4000",
         "return",
      },

      buildship = {
         "walk object-or-coords",
         "plant tribe:empire_shipconstruction unless object", -- Only create a shipconstruction if we don't already have one
         "play_sound sound/sawmill sawmill 230",
         "animation work 500",
         "construct",
         "animation work 5000",
         "return"
      }
*/
/**
 * sparamv  list of object names
 * iparam1  one of plantXXX
 */
void WorkerProgram::parse_plant(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() < 2)
		throw wexception("Usage: plant <immovable type> <immovable type> ... [unless object]");

	act->function = &Worker::run_plant;
	act->iparam1 = Worker::Action::plantAlways;
	for (uint32_t i = 1; i < cmd.size(); ++i) {
		if (i >= 2 && cmd[i] == "unless") {
			++i;
			if (i >= cmd.size())
				throw GameDataError("plant: something expected after unless");
			if (cmd[i] == "object")
				act->iparam1 = Worker::Action::plantUnlessObject;
			else
				throw GameDataError("plant: 'unless %s' not understood", cmd[i].c_str());

			continue;
		}

		// Check if immovable type exists
		std::vector<std::string> const list(split_string(cmd[i], ":"));
		if (list.size() != 2) {
			throw GameDataError("plant takes either tribe:<immovable> or attrib:<attribute>");
		}

		if (list[0] == "attrib") {
			// This will throw a GameDataError if the attribute doesn't exist.
			ImmovableDescr::get_attribute_id(list[1]);
		} else {
			DescriptionIndex idx = tribes_.safe_immovable_index(list[1]);
			if (!tribes_.immovable_exists(idx)) {
				throw GameDataError(
				   "There is no tribe immovable %s for workers to plant.", list[1].c_str());
			}
		}
		act->sparamv.push_back(cmd[i]);
	}
}

/* RST
create_bob
^^^^^^^^^^
.. function:: create_bob

   Adds a bob (usually an animal) to the map at the worker's current location.
   The list of possible bobs must have been selected by a previous command. Example::

      release = {
         "setbobdescription wildboar stag sheep", -- We want to release a wildboar, stag or sheep into the wild
         "findspace size:any radius:3",
         "walk coords",
         "animation releasein 2000",
         "create_bob", -- Now release a wildboar, stag or sheep into the wild
         "animation releaseout 2000",
         "return"
      }
*/
void WorkerProgram::parse_create_bob(Worker::Action* act, const std::vector<std::string>&) {
	act->function = &Worker::run_create_bob;
}

/* RST
object remove
^^^^^^^^^^^^^
.. function:: object remove

   Remove the currently selected object. Example::

      hunt = {
         "findobject type:bob radius:13 attrib:eatable", -- Select an object to remove
         "walk object",
         "animation idle 1000",
         "object remove", -- The selected eatable map object has been hunted, so remove it from the map
         "createware meat",
         "return"
      }
*/
void WorkerProgram::parse_removeobject(Worker::Action* act, const std::vector<std::string>&) {
	act->function = &Worker::run_removeobject;
}

/* RST
geologist
^^^^^^^^^
.. function:: geologist \<repetitions\> \<radius\> \<program_name\>

   :arg int repetitions: The number of times that the geologist will move to a
      different spot on the map to execute ``program_name``.

   :arg int radius: The radius of map fields for the geologist not to stray from.

   Walk around the starting point randomly within a certain radius, and execute
   your ``program_name`` for some of the fields. Example::

      expedition = {
         "geologist 15 5 search"
      },
*/
/**
 * iparam1 = maximum repeat #
 * iparam2 = radius
 * sparam1 = subcommand
 */
void WorkerProgram::parse_geologist(Worker::Action* act, const std::vector<std::string>& cmd) {
	char* endp;

	if (cmd.size() != 4)
		throw wexception("Usage: geologist <repeat #> <radius> <subcommand>");

	act->function = &Worker::run_geologist;

	act->iparam1 = strtol(cmd[1].c_str(), &endp, 0);
	if (*endp)
		throw wexception("Bad repeat count '%s'", cmd[1].c_str());

	act->iparam2 = strtol(cmd[2].c_str(), &endp, 0);
	if (*endp)
		throw wexception("Bad radius '%s'", cmd[2].c_str());

	act->sparam1 = cmd[3];
}

/* RST
geologist_find
^^^^^^^^^^^^^^
.. function:: geologist_find

   Check the current position for map resources (e.g. coal or water), and plant
   a marker object when possible. Example::

      search = {
         "animation hacking 5000",
         "animation idle 2000",
         "play_sound sound/hammering geologist_hammer 192",
         "animation hacking 3000",
         "geologist_find" -- Plant a resource marker at the current location, according to what has been found.
      }
*/
void WorkerProgram::parse_geologist_find(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 1)
		throw wexception("Usage: geologist_find");

	act->function = &Worker::run_geologist_find;
}

/* RST
scout
^^^^^
.. function:: scout \<radius\> \<time\>

   :arg int radius: The radius of map fields for the scout to explore.

   :arg int time: The time in milliseconds that the scout will spend scouting.

   Sends a scout out to run around scouting the area. Example::

      scout = {
         "scout 15 75000", -- Scout within a radius of 15 for 75 seconds
         "return"
      },
*/
/**
 * iparam1 = radius
 * iparam2 = time
 */
void WorkerProgram::parse_scout(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() != 3)
		throw wexception("Usage: scout <radius> <time>");

	act->iparam1 = atoi(cmd[1].c_str());
	act->iparam2 = atoi(cmd[2].c_str());
	act->function = &Worker::run_scout;
}

/* RST
play_sound
^^^^^^^^^^
.. function:: play_sound \<sound_dir\> \<sound_name\> [priority]

   :arg string sound_dir: The directory (folder) that the sound files are in,
      relative to the data directory.
   :arg string sound_name: The name of the particular sound to play.
      There can be multiple sound files to select from at random, e.g.
      for `scythe`, we can have `scythe_00.ogg`, `scythe_01.ogg` ...

   :arg int priority: The priority to give this sound. Maximum priority is 255.

   Play a sound effect. Example::

      harvest = {
         "findobject attrib:ripe_wheat radius:2",
         "walk object",
         "play_sound sound/farm scythe 220", -- Almost certainly play a swishy harvesting sound
         "animation harvesting 10000",
         "object harvest",
         "animation gathering 4000",
         "createware wheat",
         "return"
      }
*/
void WorkerProgram::parse_play_sound(Worker::Action* act, const std::vector<std::string>& cmd) {
	if (cmd.size() < 3 || cmd.size() > 4)
		throw wexception("Usage: play_sound <sound_dir> <sound_name> [priority]");

	act->sparam1 = cmd[1] + "/" + cmd[2];

	g_sound_handler.load_fx_if_needed(cmd[1], cmd[2], act->sparam1);

	act->function = &Worker::run_play_sound;
	act->iparam1 = cmd.size() == 3 ? 64 :  //  50% chance to play, only one instance at a time
	                  atoi(cmd[3].c_str());
}

/* RST
construct
^^^^^^^^^
.. function:: construct

   Give the ware currently held by the worker to the immovable object for construction.
   This is used in ship building. Example::

      buildship = {
         "walk object-or-coords", -- Walk to coordinates from 1. or to object from 2.
         "plant tribe:barbarians_shipconstruction unless object", -- 2. This will create an object for us if we don't have one yet
         "play_sound sound/sawmill sawmill 230",
         "animation work 500",
         "construct", -- 1. Add the current ware to the shipconstruction. This will find a space for us if no shipconstruction object has been planted yet
         "animation work 5000",
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
	if (cmd.size() != 1)
		throw wexception("Usage: construct");

	act->function = &Worker::run_construct;
}
}
