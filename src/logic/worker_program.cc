/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "logic/worker_program.h"

#include "graphic/graphic.h"
#include "helper.h"
#include "logic/findnode.h"
#include "logic/game_data_error.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"

namespace Widelands {

const WorkerProgram::ParseMap WorkerProgram::s_parsemap[] = {
	{"mine",              &WorkerProgram::parse_mine},
	{"breed",             &WorkerProgram::parse_breed},
	{"createware",        &WorkerProgram::parse_createware},
	{"setdescription",    &WorkerProgram::parse_plant},
	{"setbobdescription", &WorkerProgram::parse_setbobdescription},
	{"findobject",        &WorkerProgram::parse_findobject},
	{"findspace",         &WorkerProgram::parse_findspace},
	{"walk",              &WorkerProgram::parse_walk},
	{"animation",         &WorkerProgram::parse_animation},
	{"return",            &WorkerProgram::parse_return},
	{"object",            &WorkerProgram::parse_object},
	{"plant",             &WorkerProgram::parse_plant},
	{"create_bob",        &WorkerProgram::parse_create_bob},
	{"removeobject",      &WorkerProgram::parse_removeobject},
	{"geologist",         &WorkerProgram::parse_geologist},
	{"geologist-find",    &WorkerProgram::parse_geologist_find},
	{"scout",             &WorkerProgram::parse_scout},
	{"playFX",            &WorkerProgram::parse_playFX},
	{"construct",         &WorkerProgram::parse_construct},

	{nullptr, nullptr}
};


/**
 * Parse a program
 */
void WorkerProgram::parse
	(Worker_Descr * descr, Parser * parser, char const * const name)
{
	Section & program_s = parser->prof->get_safe_section(name);

	for (uint32_t idx = 0;; ++idx) {
		try
		{
			char buf[32];

			snprintf(buf, sizeof(buf), "%i", idx);
			char const * const string = program_s.get_string(buf, nullptr);
			if (!string)
				break;

			const std::vector<std::string> cmd(split_string(string, " \t\r\n"));
			if (cmd.empty())
				continue;

			// Find the appropriate parser
			Worker::Action act;
			uint32_t mapidx;

			for (mapidx = 0; s_parsemap[mapidx].name; ++mapidx)
				if (cmd[0] == s_parsemap[mapidx].name)
					break;

			if (!s_parsemap[mapidx].name)
				throw wexception("unknown command type \"%s\"", cmd[0].c_str());

			(this->*s_parsemap[mapidx].function)(descr, &act, parser, cmd);

			m_actions.push_back(act);
		} catch (const std::exception & e) {
			throw wexception("Line %i: %s", idx, e.what());
		}
	}

	// Check for line numbering problems
	if (program_s.get_num_values() != m_actions.size())
		throw wexception("Line numbers appear to be wrong");
}


/**
 * createware \<waretype\>
 *
 * The worker will create and carry a ware of the given type.
 *
 * iparam1 = ware index
 */
void WorkerProgram::parse_createware
	(Worker_Descr                   * descr,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 2)
		throw wexception("Usage: createware <ware type>");

	act->function = &Worker::run_createware;
	act->iparam1 = descr->tribe().safe_ware_index(cmd[1]).value();
}

/**
 * mine \<resource\> \<area\>
 *
 * Mine on the current coordinates (from walk or so) for resources decrease,
 * go home
 *
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_mine
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 3)
		throw wexception("Usage: mine <ware type> <area>");

	act->function = &Worker::run_mine;
	act->sparam1 = cmd[1];
	char * endp;
	act->iparam1 = strtol(cmd[2].c_str(), &endp, 0);

	if (*endp)
		throw wexception("Bad area '%s'", cmd[2].c_str());
}

/**
 * breed \<resource\> \<area\>
 *
 * Breed on the current coordinates (from walk or so) for resource increase,
 * go home
 *
 * iparam1 = area
 * sparam1 = resource
 */
void WorkerProgram::parse_breed
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 3)
		throw wexception("Usage: breed <ware type> <area>");

	act->function = &Worker::run_breed;
	act->sparam1 = cmd[1];
	char * endp;
	act->iparam1 = strtol(cmd[2].c_str(), &endp, 0);

	if (*endp)
		throw wexception("Bad area '%s'", cmd[2].c_str());
}


/**
 * OUTDATED - SHOULD NOT BE USED ANYMORE AND DOES NOT DO ANYTHING VALUABLE
 *    just kept here for savegame compatibility for Build15 and earlier
 *
 * setdescription \<immovable name\> \<immovable name\> ...
 *
 * sparamv = possible bobs
 */
void WorkerProgram::parse_setdescription
	(Worker_Descr                   *,
	 Worker::Action                 *,
	 Parser                         *,
	 const std::vector<std::string> &)
{
}


/**
 * setbobdescription \<bob name\> \<bob name\> ...
 *
 * Randomly select a bob name that can be used in subsequent commands
 * (e.g. create_bob).
 *
 * sparamv = possible bobs
 */
void WorkerProgram::parse_setbobdescription
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() < 2)
		throw wexception("Usage: setbobdescription <bob name> <bob name> ...");

	act->function = &Worker::run_setbobdescription;

	for (uint32_t i = 1; i < cmd.size(); ++i)
		act->sparamv.push_back(cmd[i]);
}


/**
 * findobject key:value key:value ...
 *
 * Find and select an object based on a number of predicates.
 * The object can be used in other commands like walk or object.
 *
 * Predicates:
 * radius:\<dist\>
 * Find objects within the given radius
 *
 * attrib:\<attribute\>  (optional)
 * Find objects with the given attribute
 *
 * type:\<what\>         (optional, defaults to immovable)
 * Find only objects of this type
 *
 * iparam1 = radius predicate
 * iparam2 = attribute predicate (if >= 0)
 * sparam1 = type
 */
void WorkerProgram::parse_findobject
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	uint32_t i;

	act->function = &Worker::run_findobject;
	act->iparam1 = -1;
	act->iparam2 = -1;
	act->sparam1 = "immovable";

	// Parse predicates
	for (i = 1; i < cmd.size(); ++i) {
		uint32_t idx = cmd[i].find(':');
		std::string const key   = cmd[i].substr(0, idx);
		std::string const value = cmd[i].substr(idx + 1);

		if (key == "radius") {
			char * endp;

			act->iparam1 = strtol(value.c_str(), &endp, 0);
			if (*endp)
				throw wexception("Bad findobject radius '%s'", value.c_str());

		} else if (key == "attrib") {
			act->iparam2 = Map_Object_Descr::get_attribute_id(value);
		} else if (key == "type") {
			act->sparam1 = value;
		} else
			throw wexception
				("Bad findobject predicate %s:%s", key.c_str(), value.c_str());
	}

	if (act->iparam1 <= 0)
		throw wexception("findobject: must specify radius");

	m_workarea_info[act->iparam1].insert(" findobject");
}


/**
 * findspace key:value key:value ...
 *
 * Find a field based on a number of predicates.
 * The field can later be used in other commands, e.g. walk.
 *
 * Predicates:
 * radius:\<dist\>
 * Search for fields within the given radius around the worker.
 *
 * size:[any|build|small|medium|big|mine|port]
 * Search for fields with the given amount of space.
 *
 * breed
 * in resource:\<resname\>, also accept fields where the resource has been
 * depleted. Use this when looking for a place for breeding. Should be used
 * before resource:\<resname\>
 *
 * resource:\<resname\>
 * Resource to search for. This is mainly intended for fisher and
 * therelike (non detectable Resources and default resources)
 *
 * avoid:\<immovable attribute>
 * a field containing an immovable with that immovable should not be used
 *
 * space
 * Find only fields that are walkable such that all neighbours
 * are also walkable (an exception is made if one of the neighbouring
 * fields is owned by this worker's location).
 *
 * iparam1 = radius
 * iparam2 = FindNodeSize::sizeXXX
 * iparam3 = whether the "space" flag is set
 * iparam4 = whether the "breed" flag is set
 * iparam5 = Immovable attribute id
 * sparam1 = Resource
 */
void WorkerProgram::parse_findspace
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
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
			char * endp;

			act->iparam1 = strtol(value.c_str(), &endp, 0);
			if (*endp)
				throw wexception("Bad findspace radius '%s'", value.c_str());

		} else if (key == "size") {
			static const struct {
				char const * name;
				int32_t val;
			} sizenames[] = {
				{"any",    FindNodeSize::sizeAny},
				{"build",  FindNodeSize::sizeBuild},
				{"small",  FindNodeSize::sizeSmall},
				{"medium", FindNodeSize::sizeMedium},
				{"big",    FindNodeSize::sizeBig},
				{"mine",   FindNodeSize::sizeMine},
				{"port",   FindNodeSize::sizePort},
				{nullptr, 0}
			};

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
			act->iparam5 = Map_Object_Descr::get_attribute_id(value);
		} else
			throw wexception
				("Bad findspace predicate %s:%s", key.c_str(), value.c_str());
	}

	if (act->iparam1 <= 0)
		throw wexception("findspace: must specify radius");
	if (act->iparam2 < 0)
		throw wexception("findspace: must specify size");
	m_workarea_info[act->iparam1].insert(" findspace");
}

/**
 * walk \<where\>
 *
 * Walk to a previously selected destination. where can be one of:
 * object  walk to a previously found and selected object
 * coords  walk to a previously found and selected field/coordinate
 * object-or-coords  walk to a previously found and selected object if
 *         present; otherwise to previously found and selected coordinate
 *
 * iparam1 = walkXXX
 */
void WorkerProgram::parse_walk
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
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

/**
 * animation \<name\> \<duration\>
 *
 * Play the given animation for the given amount of time.
 *
 * iparam1 = anim id
 * iparam2 = duration
 *
 */
void WorkerProgram::parse_animation
	(Worker_Descr                   * descr,
	 Worker::Action                 * act,
	 Parser                         * parser,
	 const std::vector<std::string> & cmd)
{
	char * endp;

	if (cmd.size() != 3)
		throw wexception("Usage: animation <name> <time>");

	act->function = &Worker::run_animation;

	if (!descr->is_animation_known(cmd[1].c_str())) {
		// dynamically allocate animations here
		descr->add_animation
			(cmd[1].c_str(),
			 (g_gr->animations().load(parser->directory,
			  	 parser->prof->get_safe_section(cmd[1].c_str()))));
	}
	act->iparam1 = descr->get_animation(cmd[1].c_str());

	act->iparam2 = strtol(cmd[2].c_str(), &endp, 0);
	if (*endp)
		throw wexception("Bad duration '%s'", cmd[2].c_str());

	if (act->iparam2 <= 0)
		throw wexception("animation duration must be positive");
}


/**
 * Return home, drop an ware we're carrying onto our building's flag.
 *
 * iparam1 = 0: don't drop ware on flag, 1: do drop ware on flag
 */
void WorkerProgram::parse_return
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> &)
{
	act->function = &Worker::run_return;
	act->iparam1 = 1; // drop a ware on our owner's flag
}


/**
 * object \<command\>
 *
 * Cause the currently selected object to execute the given program.
 *
 * sparam1 = object command name
 */
void WorkerProgram::parse_object
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 2)
		throw wexception("Usage: object <program name>");

	act->function = &Worker::run_object;
	act->sparam1 = cmd[1];
}


/**
 * plant \<immmovable type\> \<immovable type\> ... [unless object]
 *
 * Plant one of the given immovables on the current position.
 * Decision is made with inclusion of the terrain affinity.
 *
 * sparamv  list of object names
 * iparam1  one of plantXXX
 */
void WorkerProgram::parse_plant
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() < 2)
		throw wexception("Usage: plant <immovable type> <immovable type> ... [unless object]");

	act->function = &Worker::run_plant;
	act->iparam1 = Worker::Action::plantAlways;
	for (uint32_t i = 1; i < cmd.size(); ++i) {
		if (i >= 2 && cmd[i] == "unless") {
			++i;
			if (i >= cmd.size())
				throw game_data_error("plant: something expected after unless");
			if (cmd[i] == "object")
				act->iparam1 = Worker::Action::plantUnlessObject;
			else
				throw game_data_error("plant: 'unless %s' not understood", cmd[i].c_str());

			continue;
		}
		act->sparamv.push_back(cmd[i]);
	}
}


/**
 * Plants a bob (critter usually, maybe also worker later on). The immovable
 * type must have been selected by a previous command (i.e. setbobdescription).
 */
void WorkerProgram::parse_create_bob
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> &)
{
	act->function = &Worker::run_create_bob;
}


/**
 * Simply remove the currently selected object - make no fuss about it.
 */
void WorkerProgram::parse_removeobject
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> &)
{
	act->function = &Worker::run_removeobject;
}


/**
 * geologist \<repeat #\> \<radius\> \<subcommand\>
 *
 * Walk around the starting point randomly within a certain radius,
 * and execute the subcommand for some of the fields.
 *
 * iparam1 = maximum repeat #
 * iparam2 = radius
 * sparam1 = subcommand
 */
void WorkerProgram::parse_geologist
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	char * endp;

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


/**
 * Check resources at the current position, and plant a marker object
 * when possible.
 */
void WorkerProgram::parse_geologist_find
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 1)
		throw wexception("Usage: geologist-find");

	act->function = &Worker::run_geologist_find;
}

/**
 * scout \<radius\> \<time\>
 *
 * Sends the scout out to run around scouting the area
 *
 * iparam1 = radius
 * iparam2 = time
 */
void WorkerProgram::parse_scout
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 3)
		throw wexception("Usage: scout <radius> <time>");

	act->iparam1 = atoi(cmd[1].c_str());
	act->iparam2 = atoi(cmd[2].c_str());
	act->function = &Worker::run_scout;
}

void WorkerProgram::parse_playFX
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         * parser,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() < 2 || cmd.size() > 3)
		throw wexception("Usage: playFX <fx_name> [priority]");

	act->sparam1 = parser->directory + "/" + cmd[1];

	g_sound_handler.load_fx_if_needed(parser->directory, cmd[1], act->sparam1);

	act->function = &Worker::run_playFX;
	act->iparam1 =
		cmd.size() == 2 ?
		64 : //  50% chance to play, only one instance at a time
		atoi(cmd[2].c_str());
}

/**
 * construct
 *
 * Give the currently held ware of the worker to the \ref objvar1 immovable
 * for construction. This is used in ship building.
 */
void WorkerProgram::parse_construct
	(Worker_Descr                   *,
	 Worker::Action                 * act,
	 Parser                         *,
	 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 1)
		throw wexception("Usage: construct");

	act->function = &Worker::run_construct;
}


}
