/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "cmd_queue.h"
#include "critter_bob.h"
#include "filesystem.h"
#include "game.h"
#include "graphic.h"
#include "player.h"
#include "profile.h"
#include "rendertarget.h"
#include "soldier.h"
#include "transport.h"
#include "tribe.h"
#include "util.h"
#include "warehouse.h"
#include "wexception.h"
#include "worker.h"
#include "world.h"


class Cmd_Incorporate:public BaseCommand {
    private:
	    Worker* worker;
	    
    public:
	    Cmd_Incorporate (int t, Worker* w) : BaseCommand (t)
	    {
		    worker=w;
	    }
	    
	    void execute (Game* g)
	    {
		    worker->incorporate (g);
	    }
};


/*
==============================================================================

class WorkerProgram

==============================================================================
*/

struct WorkerAction {
	typedef bool (Worker::*execute_t)(Game* g, Bob::State* state, const WorkerAction* act);

	enum {
		walkObject,			// walk to objvar1
		walkCoords,			// walk to coords
	};

	execute_t		function;
	int				iparam1;
	int				iparam2;
	std::string		sparam1;

	std::vector<std::string>	sparamv;
};

class WorkerProgram {
public:
	struct Parser {
		Worker_Descr*		descr;
		std::string			directory;
		Profile*				prof;
      const EncodeData* encdata;
	};

	typedef void (WorkerProgram::*parse_t)(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);

public:
	WorkerProgram(std::string name);

	std::string get_name() const { return m_name; }
	int get_size() const { return m_actions.size(); }
	const WorkerAction* get_action(int idx) const {
		assert(idx >= 0 && (uint)idx < m_actions.size());
		return &m_actions[idx];
	}

	void parse(Parser* parser, std::string name);

private:
	struct ParseMap {
		const char*		name;
		parse_t			function;
	};

private:
	void parse_mine(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_createitem(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_setdescription(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_setbobdescription(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_findobject(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_findspace(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_walk(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_animation(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_return(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_object(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_plant(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_create_bob(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_removeobject(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_geologist(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);
	void parse_geologist_find(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd);

private:
	std::string						m_name;
	std::vector<WorkerAction>	m_actions;

private:
	static const ParseMap		s_parsemap[];
};

const WorkerProgram::ParseMap WorkerProgram::s_parsemap[] = {
	{ "mine",		      &WorkerProgram::parse_mine },
	{ "createitem",		&WorkerProgram::parse_createitem },
	{ "setdescription",	&WorkerProgram::parse_setdescription },
	{ "setbobdescription", &WorkerProgram::parse_setbobdescription },
	{ "findobject",		&WorkerProgram::parse_findobject },
	{ "findspace",			&WorkerProgram::parse_findspace },
	{ "walk",				&WorkerProgram::parse_walk },
	{ "animation",			&WorkerProgram::parse_animation },
	{ "return",				&WorkerProgram::parse_return },
	{ "object",				&WorkerProgram::parse_object },
	{ "plant",				&WorkerProgram::parse_plant },
   { "create_bob",		&WorkerProgram::parse_create_bob },
	{ "removeobject",		&WorkerProgram::parse_removeobject },
	{ "geologist",			&WorkerProgram::parse_geologist },
	{ "geologist-find",	&WorkerProgram::parse_geologist_find },

	{ 0, 0 }
};


/*
===============
WorkerProgram::WorkerProgram

Initialize a program
===============
*/
WorkerProgram::WorkerProgram(std::string name)
{
	m_name = name;
}


/*
===============
WorkerProgram::parse

Parse a program
===============
*/
void WorkerProgram::parse(Parser* parser, std::string name)
{
	Section* sprogram = parser->prof->get_safe_section(name.c_str());

	for(uint idx = 0; ; ++idx) {
		try
		{
			char buf[32];
			const char* string;
			std::vector<std::string> cmd;

			snprintf(buf, sizeof(buf), "%i", idx);
			string = sprogram->get_string(buf, 0);
			if (!string)
				break;

			split_string(string, &cmd, " \t\r\n");
			if (!cmd.size())
				continue;

			// Find the appropriate parser
			WorkerAction act;
			uint mapidx;

			for(mapidx = 0; s_parsemap[mapidx].name; ++mapidx)
				if (cmd[0] == s_parsemap[mapidx].name)
					break;

			if (!s_parsemap[mapidx].name)
				throw wexception("unknown command '%s'", cmd[0].c_str());

			(this->*s_parsemap[mapidx].function)(&act, parser, cmd);

			m_actions.push_back(act);
		}
		catch(std::exception& e)
		{
			throw wexception("Line %i: %s", idx, e.what());
		}
	}

	// Check for line numbering problems
	if (sprogram->get_num_values() != m_actions.size())
		throw wexception("Line numbers appear to be wrong");
}



/*
==============================================================================

WorkerProgram Commands

==============================================================================
*/


/*
==============================

createitem <waretype>

The worker will create and carry an item of the given type.

sparam1 = ware name

==============================
*/
void WorkerProgram::parse_createitem(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	if (cmd.size() != 2)
		throw wexception("Usage: createitem <ware type>");

	act->function = &Worker::run_createitem;
	act->sparam1 = cmd[1];
}

bool Worker::run_createitem(Game* g, State* state, const WorkerAction* act)
{
	WareInstance* item;
	int wareid;

	molog("  CreateItem(%s)\n", act->sparam1.c_str());

	item = fetch_carried_item(g);
	if (item) {
		molog("  Still carrying an item! Delete it.\n");
		item->schedule_destroy(g);
	}

	wareid = g->get_safe_ware_id(act->sparam1.c_str());
	item = new WareInstance(wareid);
	item->init(g);

	set_carried_item(g, item);

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}

/*
==============================

mine <resource> <area>

Mine on the current coordinates (from walk or so) for resources
decrease, go home

iparam1 = area
sparam1 = resource 

==============================
*/
void WorkerProgram::parse_mine(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
   if (cmd.size() != 3)
      throw wexception("Usage: mine <ware type> <area>");

   act->function = &Worker::run_mine;
   act->sparam1 = cmd[1];
   char* endp;
   act->iparam1 = strtol(cmd[2].c_str(),&endp, 0);

   if(endp && *endp)
      throw wexception("Bad area '%s'", cmd[2].c_str());

}

bool Worker::run_mine(Game* g, State* state, const WorkerAction* act)
{
   molog("  Mine(%s,%i)\n", act->sparam1.c_str(), act->iparam1);

   Map* map = g->get_map();
   MapRegion mr;
   uchar res;


   res=map->get_world()->get_resource(act->sparam1.c_str());
   if(static_cast<char>(res)==-1)
      throw wexception(" Worker::run_mine: Should mine resource %s, which doesn't exist in world. Tribe is not compatible"
            " with world!!\n",  act->sparam1.c_str());

   // Select one of the fields randomly
   uint totalres = 0;
   uint totalchance = 0;
   int pick;
   Field* f;

   mr.init(map, get_position(), act->iparam1);

   while((f = mr.next())) {
      uchar fres = f->get_resources();
      uint amount = f->get_resources_amount();

      // In the future, we might want to support amount = 0 for
      // fields that can produce an infinite amount of resources.
      // Rather -1 or something similar. not 0
      if (fres != res)
         amount = 0;

      totalres += amount;
      totalchance += 8 * amount;

      // Add penalty for fields that are running out
      if (amount == 0)
         // we already know it's completely empty, so punish is less
         totalchance += 1;
      else if (amount <= 2)
         totalchance += 6;
      else if (amount <= 4)
         totalchance += 4;
      else if (amount <= 6)
         totalchance += 2;
   }

   if (totalres == 0) {
      molog("  Run out of resources\n");
      return false;
   }

   // Second pass through fields
   pick = g->logic_rand() % totalchance;

   mr.init(map, get_position(), act->iparam1);

   while((f = mr.next())) {
      uchar fres = f->get_resources();
      uint amount = f->get_resources_amount();;

      if (fres != res)
         amount = 0;

      pick -= 8*amount;
      if (pick < 0) {
         assert(amount > 0);

         amount--;

         f->set_resources(res,amount);
         break;
      }
   }

   if (pick >= 0) {
      molog("  Not successful this time\n");
      return false;
   }

   molog("  Mined one item\n");

   state->ivar1++;
   schedule_act(g, 10);
   return true;
}

/*
==============================

setdescription <immovable name> <immovable name> ...

Randomly select an immovable name that can be used in subsequent commands
(e.g. plant).

sparamv = possible bobs

==============================
*/
void WorkerProgram::parse_setdescription(WorkerAction* act, Parser* parser,
															const std::vector<std::string>& cmd)
{
	if (cmd.size() < 2)
		throw wexception("Usage: setdescription <bob name> <bob name> ...");

	act->function = &Worker::run_setdescription;

	for(uint i = 1; i < cmd.size(); i++)
		act->sparamv.push_back(cmd[i]);
}

bool Worker::run_setdescription(Game* g, State* state, const WorkerAction* act)
{
	int idx = g->logic_rand() % act->sparamv.size();

	molog("  SetDescription: %s\n", act->sparamv[idx].c_str());

   std::vector<std::string> list;
   split_string(act->sparamv[idx], &list, ":");
   std::string bob;
   if(list.size()==1) {
      state->svar1 = "world";
      bob=list[0];
   } else {
      state->svar1 = "tribe";
      bob=list[1];
   }
      
   if(state->svar1 == "world") { 
	state->ivar2 = g->get_map()->get_world()->get_immovable_index(bob.c_str());
   } else {
	state->ivar2 = get_descr()->get_tribe()->get_immovable_index(bob.c_str());
   }
	if (state->ivar2 < 0) {
		molog("  WARNING: Unknown immovable %s\n", act->sparamv[idx].c_str());
		set_signal("fail");
		pop_task(g);
		return true;
	}

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}

/*
==============================

setbobdescription <bob name> <bob name> ...

Randomly select a bob name that can be used in subsequent commands
(e.g. create_bob).

sparamv = possible bobs

==============================
*/
void WorkerProgram::parse_setbobdescription(WorkerAction* act, Parser* parser,
															const std::vector<std::string>& cmd)
{
	if (cmd.size() < 2)
		throw wexception("Usage: setbobdescription <bob name> <bob name> ...");

	act->function = &Worker::run_setbobdescription;

	for(uint i = 1; i < cmd.size(); i++)
		act->sparamv.push_back(cmd[i]);
}

bool Worker::run_setbobdescription(Game* g, State* state, const WorkerAction* act)
{
	int idx = g->logic_rand() % act->sparamv.size();

	molog("  SetBobDescription: %s\n", act->sparamv[idx].c_str());

   std::vector<std::string> list;
   split_string(act->sparamv[idx], &list, ":");
   std::string bob;
   if(list.size()==1) {
      state->svar1 = "world";
      bob=list[0];
   } else {
      state->svar1 = "tribe";
      bob=list[1];
   }
      
   if(state->svar1 == "world") { 
	state->ivar2 = g->get_map()->get_world()->get_bob(bob.c_str());
   } else {
	state->ivar2 = get_descr()->get_tribe()->get_bob(bob.c_str());
   }
	if (state->ivar2 < 0) {
		molog("  WARNING: Unknown bob %s\n", act->sparamv[idx].c_str());
		set_signal("fail");
		pop_task(g);
		return true;
	}

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}

/*
==============================

findobject key:value key:value ...

Find and select an object based on a number of predicates.
The object can be used in other commands like walk or object.

Predicates:
radius:<dist>
	Find objects within the given radius

attrib:<attribute>  (optional)
	Find objects with the given attribute

type:<what>         (optional, defaults to immovable)
   Find only objects of this type
   
iparam1 = radius predicate
iparam2 = attribute predicate (if >= 0)
sparam1 = type

==============================
*/
void WorkerProgram::parse_findobject(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	uint i;

	act->function = &Worker::run_findobject;
	act->iparam1 = -1;
	act->iparam2 = -1;
   act->sparam1 = "immovable";
      
	// Parse predicates
	for(i = 1; i < cmd.size(); i++) {
		uint idx = cmd[i].find(':');
		std::string key = cmd[i].substr(0, idx);
		std::string value = cmd[i].substr(idx+1);

		if (key == "radius") {
			char* endp;

			act->iparam1 = strtol(value.c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Bad findobject radius '%s'", value.c_str());
		} else if (key == "attrib") {
			act->iparam2 = Map_Object_Descr::get_attribute_id(value);
		} else if (key == "type") {
         act->sparam1 = value;
      } else
			throw wexception("Bad findobject predicate %s:%s", key.c_str(), value.c_str());
	}

	if (act->iparam1 <= 0)
		throw wexception("findobject: must specify radius");
}

bool Worker::run_findobject(Game* g, State* state, const WorkerAction* act)
{
   Coords pos = get_position();
   Map* map = g->get_map();

   molog("  FindObject(%i, %i,%s)\n", act->iparam1, act->iparam2, act->sparam1.c_str());


   PlayerImmovable* location = get_location(g);
   Building* owner;

   assert(location);
   assert(location->get_type() == BUILDING);

   owner = (Building*)location;

   CheckStepWalkOn cstep(get_movecaps(), false);

   if (pos == owner->get_position())
      pos = owner->get_base_flag()->get_position();

   if(act->sparam1=="immovable") {
      std::vector<ImmovableFound> list;
      if (act->iparam2 < 0)
         map->find_reachable_immovables(pos, act->iparam1, &list, &cstep);
      else
         map->find_reachable_immovables(pos, act->iparam1, &list, &cstep,
               FindImmovableAttribute(act->iparam2));

      if (!list.size()) {
         set_signal("fail"); // no object found, cannot run program
         pop_task(g);
         return true;
      }

      int sel = g->logic_rand() % list.size();
      state->objvar1 = list[sel].object;
      molog("  %i found\n", list.size());
   } else {
      std::vector<Bob*> list;
      log("BOB: searching bob with attribute (%i)\n", act->iparam2);
      if (act->iparam2 < 0)
         map->find_reachable_bobs(pos, act->iparam1, &list, &cstep);
      else
         map->find_reachable_bobs(pos, act->iparam1, &list, &cstep,
               FindBobAttribute(act->iparam2));

      if (!list.size()) {
         set_signal("fail"); // no object found, cannot run program
         pop_task(g);
         return true;
      }
      int sel = g->logic_rand() % list.size();
      state->objvar1 = list[sel];
      molog("  %i found\n", list.size());
   }

   state->ivar1++;
   schedule_act(g, 10);
   return true;
}



/*
==============================

findspace key:value key:value ...

Find a field based on a number of predicates.
The field can later be used in other commands, e.g. walk.

Predicates:
radius:<dist>
	Search for fields within the given radius around the worker.

size:[any|build|small|medium|big|mine|port]
	Search for fields with the given amount of space.

resource:<resname>
   Resource to search for. This is mainly intended for fisher and 
   therelike (non detectable Resources and default resources)

iparam1 = radius
iparam2 = FindFieldSize::sizeXXX
sparam1 = Resource

==============================
*/
void WorkerProgram::parse_findspace(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	uint i;

	act->function = &Worker::run_findspace;
	act->iparam1 = -1;
	act->iparam2 = -1;
   act->sparam1 = "";
   
	// Parse predicates
	for(i = 1; i < cmd.size(); i++) {
		uint idx = cmd[i].find(':');
		std::string key = cmd[i].substr(0, idx);
		std::string value = cmd[i].substr(idx+1);

		if (key == "radius") {
			char* endp;

			act->iparam1 = strtol(value.c_str(), &endp, 0);
			if (endp && *endp)
				throw wexception("Bad findspace radius '%s'", value.c_str());
		} else if (key == "size") {
			static const struct {
				const char* name;
				int val;
			} sizenames[] = {
				{ "any",		FindFieldSize::sizeAny },
				{ "build",	FindFieldSize::sizeBuild },
				{ "small",	FindFieldSize::sizeSmall },
				{ "medium",	FindFieldSize::sizeMedium },
				{ "big",		FindFieldSize::sizeBig },
				{ "mine",	FindFieldSize::sizeMine },
				{ "port",	FindFieldSize::sizePort },
				{ 0, 0 }
			};

			int idx;

			for(idx = 0; sizenames[idx].name; ++idx)
				if (value == sizenames[idx].name)
					break;

			if (!sizenames[idx].name)
				throw wexception("Bad findspace size '%s'", value.c_str());

			act->iparam2 = sizenames[idx].val;
		} else if(key == "resource") {
         act->sparam1 = value; 
      } else
			throw wexception("Bad findspace predicate %s:%s", key.c_str(), value.c_str());
	}

	if (act->iparam1 <= 0)
		throw wexception("findspace: must specify radius");
	if (act->iparam2 < 0)
		throw wexception("findspace: must specify size");
}

bool Worker::run_findspace(Game* g, State* state, const WorkerAction* act)
{
	std::vector<Coords> list;
	Map* map = g->get_map();
	World* w = map->get_world();
	
   CheckStepDefault cstep(get_movecaps());

   int res=-1;
   if(act->sparam1.size()) 
     res=w->get_resource(act->sparam1.c_str());

   int retval=0;
   if(res!=-1) {
      retval=map->find_reachable_fields(get_position(), act->iparam1, &list, &cstep,
               FindFieldSizeResource((FindFieldSize::Size)act->iparam2,res)); 
   } else {
      retval=map->find_reachable_fields(get_position(), act->iparam1, &list, &cstep,
               FindFieldSize((FindFieldSize::Size)act->iparam2)); 
   }

   if(!retval) {
		molog("  no space found\n");
		set_signal("fail");
		pop_task(g);
		return true;
	}

	// Pick a location at random
	int sel = g->logic_rand() % list.size();

	state->coords = list[sel];

	molog("  selected %i,%i\n", state->coords.x, state->coords.y);

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}


/*
==============================

walk <where>

Walk to a previously selected destination. where can be one of:
	object	Walk to a previously found and selected object
	coords	Walk to a previously found and selected field/coordinate

iparam1 = walkXXX

==============================
*/
void WorkerProgram::parse_walk(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	if (cmd.size() != 2)
		throw wexception("Usage: walk <where>");

	act->function = &Worker::run_walk;

	if (cmd[1] == "object")
		act->iparam1 = WorkerAction::walkObject;
	else if (cmd[1] == "coords")
		act->iparam1 = WorkerAction::walkCoords;
	else
		throw wexception("Bad walk destination '%s'", cmd[1].c_str());
}

bool Worker::run_walk(Game* g, State* state, const WorkerAction* act)
{
	BaseImmovable* imm = g->get_map()->get_immovable(get_position());
	Coords dest;
	bool forceonlast = false;
	PlayerImmovable* location = get_location(g);
	Building* owner;
   int max_steps=-1;
   
	assert(location);
	assert(location->get_type() == BUILDING);

	owner = (Building*)location;

	molog("  Walk(%i)\n", act->iparam1);

	// First of all, make sure we're outside
	if (imm == owner) {
		start_task_leavebuilding(g, false);
		return true;
	}

	// Determine the coords we need to walk towards
	switch(act->iparam1) {
	case WorkerAction::walkObject:
		{
			Map_Object* obj = state->objvar1.get(g);

			if (!obj) {
				molog("  object(nil)\n");
				set_signal("fail");
				pop_task(g);
				return true;
			}

			molog("  object(%u): type = %i\n", obj->get_serial(), obj->get_type());

			if (obj->get_type() == BOB)
				dest = ((Bob*)obj)->get_position();
			else if (obj->get_type() == IMMOVABLE)
				dest = ((Immovable*)obj)->get_position();
			else
				throw wexception("MO(%u): [actWalk]: bad object type = %i", get_serial(), obj->get_type());

         max_steps=1; // Only take one step, then rethink (object may have moved)
			forceonlast = true;
			break;
		}

	case WorkerAction::walkCoords:
		molog("  coords(%i,%i)\n", state->coords.x, state->coords.y);
		dest = state->coords;
		break;

	default:
		throw wexception("MO(%u): [actWalk]: bad act->iparam1 = %i", get_serial(), act->iparam1);
	}

	// If we've already reached our destination, that's cool
	if (get_position() == dest) {
		molog("  reached\n");
		state->ivar1++;
		return false; // next instruction
	}

	// Walk towards it
	if (!start_task_movepath(g, dest, 10, get_descr()->get_right_walk_anims(does_carry_ware()), forceonlast, max_steps)) {
		molog("  couldn't find path\n");
		set_signal("fail");
		pop_task(g);
		return true;
	}

	return true;
}


/*
==============================

animation <name> <duration>

Play the given animation for the given amount of time.

iparam1 = anim id
iparam2 = duration

==============================
*/
void WorkerProgram::parse_animation(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	char* endp;

	if (cmd.size() != 3)
		throw wexception("Usage: animation <name> <time>");

	act->function = &Worker::run_animation;

	if (cmd[1] == "idle")
		act->iparam1 = parser->descr->get_idle_anim();
	else {
      // dynamically allocate animations here
      Section* s = parser->prof->get_safe_section(cmd[1].c_str());
      act->iparam1 = g_anim.get(parser->directory.c_str(), s, 0, parser->encdata);
   }

	act->iparam2 = strtol(cmd[2].c_str(), &endp, 0);
	if (endp && *endp)
		throw wexception("Bad duration '%s'", cmd[2].c_str());

	if (act->iparam2 <= 0)
		throw wexception("animation duration must be positive");
}

bool Worker::run_animation(Game* g, State* state, const WorkerAction* act)
{
	molog("  Animation(%i, %i)\n", act->iparam1, act->iparam2);
	set_animation(g, act->iparam1);

	state->ivar1++;
	schedule_act(g, act->iparam2);
	return true;
}



/*
==============================

return

Return home, drop any item we're carrying onto our building's flag.

iparam1 = 0: don't drop item on flag, 1: do drop item on flag

==============================
*/
void WorkerProgram::parse_return(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	act->function = &Worker::run_return;
	act->iparam1 = 1; // drop any item on our owner's flag
}

bool Worker::run_return(Game* g, State* state, const WorkerAction* act)
{
	molog("  Return(%i)\n", act->iparam1);

	state->ivar1++;
	start_task_return(g, act->iparam1);
	return true;
}


/*
==============================

object <command>

Cause the currently selected object to execute the given program.

sparam1 = object command name

==============================
*/
void WorkerProgram::parse_object(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	if (cmd.size() != 2)
		throw wexception("Usage: object <program name>");

	act->function = &Worker::run_object;
	act->sparam1 = cmd[1];
}

bool Worker::run_object(Game* g, State* state, const WorkerAction* act)
{
	Map_Object* obj;

	molog("  Object(%s)\n", act->sparam1.c_str());

	obj = state->objvar1.get(g);

	if (!obj) {
		molog("  object(nil)\n");
		set_signal("fail");
		pop_task(g);
		return true;
	}

	molog("  object(%u): type = %i\n", obj->get_serial(), obj->get_type());

	if (obj->get_type() == IMMOVABLE)
		((Immovable*)obj)->switch_program(g, act->sparam1);
   else if(obj->get_type() == BOB) {
      Bob* bob=((Bob*)(obj));
      if(bob->get_bob_type() == Bob::CRITTER) {
         Critter_Bob* crit= ((Critter_Bob*)bob);
         crit->send_signal(g, "interrupt_now");
         crit->start_task_program(g, act->sparam1);
      } else if(bob->get_type() == Bob::WORKER) {
         Worker* w= ((Worker*)bob);
         w->send_signal(g, "interrupt_now");
         w->start_task_program(g, act->sparam1);
      } else 
         throw wexception("MO(%i): [actObject]: bab bob type = %i", get_serial(), bob->get_bob_type());
   }
   else 
		throw wexception("MO(%u): [actObject]: bad object type = %i", get_serial(), obj->get_type());

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}


/*
==============================

plant

Plant an immovable on the current position. The immovable type must have been
selected by a previous command (i.e. setdescription)

==============================
*/
void WorkerProgram::parse_plant(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	act->function = &Worker::run_plant;
}

bool Worker::run_plant(Game* g, State* state, const WorkerAction* act)
{
	Coords pos = get_position();

	molog("  Plant: %i at %i,%i\n", state->ivar2, pos.x, pos.y);

	// Check if the map is still free here
	BaseImmovable* imm = g->get_map()->get_immovable(pos);

	if (imm && imm->get_size() >= BaseImmovable::SMALL) {
		molog("  field no longer free\n");
		set_signal("fail");
		pop_task(g);
		return true;
	}

   if(state->svar1 == "world") 
      g->create_immovable(pos, state->ivar2, 0);
   else 
      g->create_immovable(pos, state->ivar2, get_descr()->get_tribe());

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}

/*
==============================

create_bob

Plants a bob (critter usually, maybe also worker later on). The immovable type must have been
selected by a previous command (i.e. setbobdescription).

==============================
*/
void WorkerProgram::parse_create_bob(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	act->function = &Worker::run_create_bob;
}

bool Worker::run_create_bob(Game* g, State* state, const WorkerAction* act)
{
	Coords pos = get_position();

	molog("  Create Bob: %i at %i,%i\n", state->ivar2, pos.x, pos.y);

   if(state->svar1 == "world") 
      g->create_bob(pos, state->ivar2, 0);
   else 
      g->create_bob(pos, state->ivar2, get_descr()->get_tribe());

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}



/*
==============================

removeobject

Simply remove the currently selected object - make no fuss about it.

==============================
*/
void WorkerProgram::parse_removeobject(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	act->function = &Worker::run_removeobject;
}

bool Worker::run_removeobject(Game* g, State* state, const WorkerAction* act)
{
	Map_Object* obj;

	obj = state->objvar1.get(g);
	if (obj) {
		obj->remove(g);
		state->objvar1.set(0);
	}

	state->ivar1++;
	schedule_act(g, 10);
	return true;
}


/*
==============================

geologist <repeat #> <radius> <subcommand>

Walk around the starting point randomly within a certain radius,
and execute the subcommand for some of the fields.

iparam1 = maximum repeat #
iparam2 = radius
sparam1 = subcommand

==============================
*/
void WorkerProgram::parse_geologist(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	char* endp;

	if (cmd.size() != 4)
		throw wexception("Usage: geologist <repeat #> <radius> <subcommand>");

	act->function = &Worker::run_geologist;

	act->iparam1 = strtol(cmd[1].c_str(), &endp, 0);
	if (endp && *endp)
		throw wexception("Bad repeat count '%s'", cmd[1].c_str());

	act->iparam2 = strtol(cmd[2].c_str(), &endp, 0);
	if (endp && *endp)
		throw wexception("Bad radius '%s'", cmd[2].c_str());

	act->sparam1 = cmd[3];
}

bool Worker::run_geologist(Game* g, State* state, const WorkerAction* act)
{
	#ifdef DEBUG
	PlayerImmovable* location = get_location(g);
	#endif

	assert(location);
	assert(location->get_type() == FLAG);

	molog("  Start Geologist (%i attempts, %i radius -> %s)\n",
		act->iparam1, act->iparam2, act->sparam1.c_str());

	state->ivar1++;
	start_task_geologist(g, act->iparam1, act->iparam2, act->sparam1);
	return true;
}


/*
==============================

geologist-find

Check resources at the current position, and plant a marker object
when possible.

==============================
*/
void WorkerProgram::parse_geologist_find(WorkerAction* act, Parser* parser, const std::vector<std::string>& cmd)
{
	if (cmd.size() != 1)
		throw wexception("Usage: geologist-find");

	act->function = &Worker::run_geologist_find;
}

bool Worker::run_geologist_find(Game* g, State* state, const WorkerAction* act)
{
	FCoords position = g->get_map()->get_fcoords(get_position());
	BaseImmovable* imm = position.field->get_immovable();

	if (imm && imm->get_size() > BaseImmovable::NONE)
	{
//		NoLog("  Field is no longer empty\n");
	}
	else
	{
		uint res = position.field->get_resources();
      uint amount = position.field->get_resources_amount();

      int idx;
      Resource_Descr* rdescr=g->get_map()->get_world()->get_resource(res);

      if(rdescr->is_detectable() && amount) {
         idx = get_descr()->get_tribe()->get_resource_indicator(rdescr, amount);
      } else {
         idx = get_descr()->get_tribe()->get_resource_indicator(0, 0); // not detectable
      }

//		NoLog("  Resource: %02X -> plant indicator '%s'\n", res, get_descr()->get_tribe()->get_immovable_descr(idx)->get_name());

		g->create_immovable(position, idx, get_descr()->get_tribe());
	}

	state->ivar1++;
	return false;
}



/*
==============================================================================

IdleWorkerSupply IMPLEMENTATION

==============================================================================
*/

class IdleWorkerSupply : public Supply {
public:
	IdleWorkerSupply(Worker* w);
	~IdleWorkerSupply();

	void set_economy(Economy* e);

public:
	virtual PlayerImmovable* get_position(Game* g);
	virtual int get_amount(Game* g, int ware);
	virtual bool is_active(Game* g);

	virtual WareInstance* launch_item(Game* g, int ware);
	virtual Worker* launch_worker(Game* g, int ware);

private:
	Worker*		m_worker;
	Economy*		m_economy;
};


/*
===============
IdleWorkerSupply::IdleWorkerSupply

Automatically register with the worker's economy.
===============
*/
IdleWorkerSupply::IdleWorkerSupply(Worker* w)
{
	m_worker = w;
	m_economy = 0;

	set_economy(w->get_economy());
}


/*
===============
IdleWorkerSupply::~IdleWorkerSupply

Automatically unregister from economy.
===============
*/
IdleWorkerSupply::~IdleWorkerSupply()
{
	set_economy(0);
}


/*
===============
IdleWorkerSupply::set_economy

Add/remove this supply from the Economy as appropriate.
===============
*/
void IdleWorkerSupply::set_economy(Economy* e)
{
	if (m_economy == e)
		return;

	if (m_economy)
		m_economy->remove_supply(m_worker->get_ware_id(), this);

	m_economy = e;

	if (m_economy)
		m_economy->add_supply(m_worker->get_ware_id(), this);
}


/*
===============
IdleWorkerSupply::get_position

Return the worker's position.
===============
*/
PlayerImmovable* IdleWorkerSupply::get_position(Game* g)
{
	return m_worker->get_location(g);
}


/*
===============
IdleWorkerSupply::get_amount

It's just the one worker.
===============
*/
int IdleWorkerSupply::get_amount(Game* g, int ware)
{
	if (ware == m_worker->get_ware_id())
		return 1;

	return 0;
}


/*
===============
IdleWorkerSupply::is_active

Idle workers are always active supplies, because they need to get into a
Warehouse ASAP.
===============
*/
bool IdleWorkerSupply::is_active(Game* g)
{
	return true;
}


/*
===============
IdleWorkerSupply::launch_item
===============
*/
WareInstance* IdleWorkerSupply::launch_item(Game* g, int ware)
{
	throw wexception("IdleWorkerSupply::launch_item() makes no sense.");
}


/*
===============
IdleWorkerSupply::launch_worker

No need to explicitly launch the worker.
===============
*/
Worker* IdleWorkerSupply::launch_worker(Game* g, int ware)
{
	assert(ware == m_worker->get_ware_id());

	return m_worker;
}


/*
==============================================================================

Worker IMPLEMENTATION

==============================================================================
*/

/*
===============
Worker_Descr::Worker_Descr
Worker_Descr::~Worker_Descr
===============
*/
Worker_Descr::Worker_Descr(Tribe_Descr *tribe, const char *name)
	: Bob_Descr(name, tribe)
{
	m_tribe = tribe;
	m_menu_pic = 0;
	m_menu_pic_fname = 0;
	m_ware_id = -1;
	add_attribute(Map_Object::WORKER);
}

Worker_Descr::~Worker_Descr(void)
{
	if (m_menu_pic_fname)
		free(m_menu_pic_fname);

	while(m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/*
===============
Worker_Descr::load_graphics

Load graphics (other than animations).
===============
*/
void Worker_Descr::load_graphics()
{
	m_menu_pic = g_gr->get_picture(PicMod_Game, m_menu_pic_fname, false);
}


/*
===============
Worker_Descr::set_ware_id

Set the worker's ware id. Called by Game::init_wares.
===============
*/
void Worker_Descr::set_ware_id(int idx)
{
	m_ware_id = idx;
}


/*
===============
Worker_Descr::get_program

Get a program from the workers description.
===============
*/
const WorkerProgram* Worker_Descr::get_program(std::string name) const
{
	ProgramMap::const_iterator it = m_programs.find(name);

	if (it == m_programs.end())
		throw wexception("%s has no program '%s'", get_name(), name.c_str());

	return it->second;
}


/*
===============
Worker_Descr::create

Custom creation routing that accounts for the location.
===============
*/
Worker *Worker_Descr::create(Editor_Game_Base *gg, Player *owner, PlayerImmovable *location, Coords coords)
{
   Worker *worker = (Worker*)create_object();
	worker->set_owner(owner);
	worker->set_location(location);
	worker->set_position(gg, coords);
	worker->init(gg);
	return worker;
}


/*
===============
Worker_Descr::parse

Parse the worker data from configuration
===============
*/
void Worker_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	const char *string;
	char buf[256];
	char fname[256];
	Section* sglobal;

	Bob_Descr::parse(directory, prof, encdata);

	sglobal = prof->get_safe_section("global");

	m_descname = sglobal->get_string("descname", get_name());
	m_helptext = sglobal->get_string("help", "Doh... someone forgot the help text!");

	snprintf(buf, sizeof(buf),	"%s_menu.bmp", get_name());
	string = sglobal->get_string("menu_pic", buf);
	snprintf(fname, sizeof(fname), "%s/%s", directory, string);
	m_menu_pic_fname = strdup(fname);

	// Read the costs of building
   if(get_worker_type() == CARRIER || get_worker_type() == SOLDIER) 
      m_buildable = sglobal->get_bool("buildable", false);
   else 
      m_buildable = sglobal->get_bool("buildable", true);

	if (m_buildable)
	{
		Section *s;

		// Get the buildcost
		s = prof->get_safe_section("buildcost");

		Section::Value *val;
		
		while ((val = s->get_next_val(0)))
			m_buildcost.push_back (CostItem(val->get_name(), val->get_int()));
	}

	// Read the walking animations
	m_walk_anims.parse(directory, prof, "walk_??", prof->get_section("walk"), encdata);
   if(get_worker_type()!=SOLDIER) // Soldier have no walkload
      m_walkload_anims.parse(directory, prof, "walkload_??", prof->get_section("walkload"), encdata);

   // Read the becomes and experience
   m_becomes = sglobal->get_string("becomes","");
   std::string exp=sglobal->get_string("experience", "");
   m_min_experience=m_max_experience=-1;
   if(exp.size()) {
      std::vector<std::string> list;
      split_string(exp, &list, "-");
      if(list.size()!=2) 
         throw wexception("Parse error in experience string: \"%s\" (must be \"min-max\")", exp.c_str());
      uint i=0;
      for(i=0; i<list.size(); i++)
         remove_spaces(&list[i]);

      char* endp;
      m_min_experience = strtol(list[0].c_str(),&endp, 0);
      if(endp && *endp)
         throw wexception("Parse error in experience string: %s is a bad value", list[0].c_str());
      m_max_experience = strtol(list[1].c_str(),&endp, 0);
      if(endp && *endp)
         throw wexception("Parse error in experience string: %s is a bad value", list[1].c_str());
   }

   // Read programs
	while(sglobal->get_next_string("program", &string)) {
		WorkerProgram* prog = 0;

		try
		{
			WorkerProgram::Parser parser;

			parser.descr = this;
			parser.directory = directory;
			parser.prof = prof;
         parser.encdata = encdata;

			prog = new WorkerProgram(string);
			prog->parse(&parser, string);
			m_programs[prog->get_name()] = prog;
		}
		catch(std::exception& e)
		{
			if (prog)
				delete prog;

			throw wexception("Parse error in program %s: %s", string, e.what());
		}
	}
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Worker::Worker
Worker::~Worker
===============
*/
Worker::Worker(Worker_Descr *descr)
	: Bob(descr)
{
	m_economy = 0;
	m_location = 0;
	m_supply = 0;
   m_needed_exp = 0;
   m_current_exp = 0;
}

Worker::~Worker()
{
}


/*
===============
Worker::get_movecaps
===============
*/
uint Worker::get_movecaps()
{
	return MOVECAPS_WALK;
}


/*
===============
Worker::set_location

Change the location. This should be called in the following situations:
- worker creation (usually, location is a warehouse)
- worker moves along a route (location is a road and finally building)
- current location is destroyed (building burnt down etc...)
===============
*/
void Worker::set_location(PlayerImmovable *location)
{
	PlayerImmovable *oldlocation = get_location(get_owner()->get_game());

	if (oldlocation == location)
		return;

	if (oldlocation)
	{
		// Note: even though we have an oldlocation, m_economy may be zero
		// (oldlocation got deleted)

		oldlocation->remove_worker(this);
	}
	else
	{
		assert(!m_economy);
	}

	m_location = location;

	if (location)
	{
		Economy *eco = location->get_economy();

		if (!m_economy)
			set_economy(eco);
		else if (m_economy != eco)
			throw wexception("Worker::set_location changes economy");

		location->add_worker(this);
	}
	else
	{
		// Our location has been destroyed, we are now fugitives.
		// Interrupt whatever we've been doing.
		set_economy(0);

		send_signal((Game*)get_owner()->get_game(), "location");
	}
}


/*
===============
Worker::set_economy

Change the worker's current economy. This is called:
- by set_location() when appropriate
- by the current location, when the location's economy changes
===============
*/
void Worker::set_economy(Economy *economy)
{
	if (economy == m_economy)
		return;

	if (m_economy)
		m_economy->remove_wares(get_descr()->get_ware_id(), 1);

	m_economy = economy;

	WareInstance* item = get_carried_item(get_owner()->get_game());

	if (item)
		item->set_economy(m_economy);
	if (m_supply)
		m_supply->set_economy(m_economy);

	if (m_economy)
		m_economy->add_wares(get_descr()->get_ware_id(), 1);
}


/*
===============
Worker::init

Initialize the worker
===============
*/
void Worker::init(Editor_Game_Base *g)
{
	Bob::init(g);

	// a worker should always start out at a fixed location
	assert(get_location(g));
  
   if(g->is_game())
      create_needed_experience(static_cast<Game*>(g)); // Set his experience
}


/*
===============
Worker::cleanup

Remove the worker.
===============
*/
void Worker::cleanup(Editor_Game_Base *g)
{
	WareInstance* item = get_carried_item(g);

	if (m_supply) {
		delete m_supply;
		m_supply = 0;
	}

	if (item)
		item->destroy(g);

	if (get_location(g))
		set_location(0);

	assert(!get_economy());

	Bob::cleanup(g);
}


/*
===============
Worker::set_carried_item

Set the item we carry.
If we carry an item right now, it will be destroyed (see fetch_carried_item()).
===============
*/
void Worker::set_carried_item(Game* g, WareInstance* item)
{
	WareInstance* olditem = get_carried_item(g);

	if (olditem) {
		olditem->cleanup(g);
		delete olditem;
	}

	m_carried_item = item;
	item->set_location(g, this);
	item->update(g);
}


/*
===============
Worker::fetch_carried_item

Stop carrying the current item, and return a pointer to it.
===============
*/
WareInstance* Worker::fetch_carried_item(Game* g)
{
	WareInstance* item = get_carried_item(g);

	if (item) {
		item->set_location(g, 0);
		m_carried_item = 0;
	}

	return item;
}


/*
===============
Worker::schedule_incorporate

Schedule an immediate CMD_INCORPORATE, which will integrate this worker into
the warehouse he is standing on.
===============
*/
void Worker::schedule_incorporate(Game* g)
{
	g->get_cmdqueue()->enqueue (new Cmd_Incorporate(g->get_gametime(), this));
//	g->get_cmdqueue()->queue(g->get_gametime(), SENDER_MAPOBJECT, CMD_INCORPORATE, m_serial);
	force_skip_act(g);
}


/*
===============
Worker::incorporate

Incorporate the worker into the warehouse it's standing on immediately.
This will delete the worker.
===============
*/
void Worker::incorporate(Game *g)
{
	PlayerImmovable *location = get_location(g);

	if (location && location->has_attribute(WAREHOUSE)) {
		Warehouse *wh = (Warehouse*)location;

		wh->incorporate_worker(g, this);
		return;
	}

	// our location has been deleted from under us
	send_signal(g, "fail");
}

/*
 * Calculate needed experience. 
 * This sets the needed experience on a value between max and min
 */
void Worker::create_needed_experience(Game* g) {
   if(get_descr()->get_min_exp()==-1 && get_descr()->get_max_exp()==-1) {
      m_needed_exp=m_current_exp=-1;
      return;
   }
   
   int range=get_descr()->get_max_exp()-get_descr()->get_min_exp();
   int value=g->logic_rand() % range;
   m_needed_exp=value+get_descr()->get_min_exp();
   m_current_exp=0;
}

/*
 * Gain experience 
 * 
 * This function increases the experience
 * of the worker by one, if he reaches
 * needed_experience he levels
 */
void Worker::gain_experience(Game* g) {
   if(m_needed_exp==-1) return; // This worker can not level
   
   ++m_current_exp;
   
   if(m_current_exp==m_needed_exp) 
      level(g);
   
}

/*
 * Level this worker to the next higher 
 * level. this includes creating a new worker
 * with his propertys and removing this worker
 */
void Worker::level(Game* g) {
  
   // We do not really remove this worker, all we do 
   // is to overwrite his description with the new one and to 
   // reset his needed experience. Congratulations to promotion!
   // This silently expects that the new worker is the same type as the old
   // worker and can fullfill the same jobs (which should be given in all
   // circumstances)
   assert(get_becomes());
   int index=get_descr()->get_tribe()->get_worker_index(get_becomes());
   Worker_Descr* new_descr=get_descr()->get_tribe()->get_worker_descr(index);
		
   // Inform the economy, that something has changed
   m_economy->remove_wares(get_descr()->get_ware_id(), 1);
   m_economy->add_wares(new_descr->get_ware_id(),1);

   m_descr=new_descr;

   create_needed_experience(g);
}

/*
===============
Worker::init_auto_task

Set a fallback task.
===============
*/
void Worker::init_auto_task(Game* g)
{
	PlayerImmovable* location = get_location(g);

	if (location) {
		if (get_economy()->get_nr_warehouses()) {
			molog("init_auto_task: go warehouse\n");

			start_task_gowarehouse(g);
			return;
		}

		set_location(0);
	}

	molog("init_auto_task: become fugitive\n");

	start_task_fugitive(g);
}


/*
==============================

TRANSFER task

Follow the given transfer.
Signal "cancel" to cancel the transfer.

==============================
*/

Bob::Task Worker::taskTransfer = {
	"transfer",

	(Bob::Ptr)&Worker::transfer_update,
	(Bob::Ptr)&Worker::transfer_signal,
	(Bob::Ptr)&Worker::transfer_mask,
};


/*
===============
Worker::start_task_transfer

Tell the worker to follow the Transfer
===============
*/
void Worker::start_task_transfer(Game* g, Transfer* t)
{
	State* state;

	// hackish override for gowarehouse
	state = get_state(&taskGowarehouse);
	if (state) {
		assert(!state->transfer);

		state->transfer = t;
		send_signal(g, "transfer");
		return;
	}

	// just start a normal transfer
	push_task(g, &taskTransfer);

	state = get_state();
	state->transfer = t;
}


/*
===============
Worker::transfer_update
===============
*/
void Worker::transfer_update(Game* g, State* state)
{
	PlayerImmovable* location = get_location(g);

	assert(location); // 'location' signal expected otherwise

	// The request is no longer valid, the task has failed
	if (!state->transfer) {
		molog("[transfer]: Fail (without transfer)\n");

		set_signal("fail");
		pop_task(g);
		return;
	}

	// Figure out where to go
	bool success;
	PlayerImmovable* nextstep = state->transfer->get_next_step(location, &success);

	if (!nextstep) {
		Transfer* t = state->transfer;

		state->transfer = 0;

		if (success) {
			molog("[transfer]: Success\n");
			pop_task(g);

			t->finish();
			return;
		} else {
			molog("[transfer]: Failed\n");
			set_signal("fail");
			pop_task(g);

			t->fail();
			return;
		}
	}

	// Initiate the next step
	if (location->get_type() == BUILDING) {
		if (location->get_base_flag() != nextstep)
			throw wexception("MO(%u): [transfer]: in building, nextstep is not building's flag", get_serial());

		molog("[transfer]: move from building to flag\n");
		start_task_leavebuilding(g, true);
		return;
	}

	if (location->get_type() == FLAG) {
		// Flag to Building
		if (nextstep->get_type() == BUILDING) {
			if (nextstep->get_base_flag() != location)
				throw wexception("MO(%u): [transfer]: next step is building, but we're nowhere near", get_serial());

			molog("[transfer]: move from flag to building\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_right_walk_anims(does_carry_ware()));
			set_location(nextstep);
			return;
		}

		// Flag to Flag
		if (nextstep->get_type() == FLAG) {
			Road* road = ((Flag*)location)->get_road((Flag*)nextstep);

			molog("[transfer]: move to next flag via road %u\n", road->get_serial());

			Path path(road->get_path());

			if (nextstep != road->get_flag(Road::FlagEnd))
				path.reverse();

			start_task_movepath(g, path, get_descr()->get_right_walk_anims(does_carry_ware()));
			set_location(road);
			return;
		}

		// Flag to Road
		if (nextstep->get_type() == ROAD) {
			Road* road = (Road*)nextstep;

			if (road->get_flag(Road::FlagStart) != location && road->get_flag(Road::FlagEnd) != location)
				throw wexception("MO(%u): [transfer]: nextstep is road, but we're nowhere near", get_serial());

			molog("[transfer]: set location to road %u\n", road->get_serial());
			set_location(road);
			set_animation(g, get_descr()->get_idle_anim());
			schedule_act(g, 10); // wait a little
			return;
		}

		throw wexception("MO(%u): [transfer]: flag to bad nextstep %u", get_serial(), nextstep->get_serial());
	}

	if (location->get_type() == ROAD) {
		// Road to Flag
		if (nextstep->get_type() == FLAG) {
			Road* road = (Road*)location;
			const Path& path = road->get_path();
			int index;

			if (nextstep == road->get_flag(Road::FlagStart))
				index = 0;
			else if (nextstep == road->get_flag(Road::FlagEnd))
				index = path.get_nsteps();
			else
				index = -1;

			molog("[transfer]: on road %u, to flag %u, index is %i\n", road->get_serial(),
							nextstep->get_serial(), index);

			if (index >= 0)
			{
				if (start_task_movepath(g, path, index, get_descr()->get_right_walk_anims(does_carry_ware()))) {
					molog("[transfer]: from road %u to flag %u\n", get_serial(), road->get_serial(),
									nextstep->get_serial());
					return;
				}
			}
			else
			{
				if (nextstep != g->get_map()->get_immovable(get_position()))
					throw wexception("MO(%u): [transfer]: road to flag, but flag is nowhere near", get_serial());
			}

			molog("[transfer]: arrive at flag %u\n", nextstep->get_serial());
			set_location((Flag*)nextstep);
			set_animation(g, get_descr()->get_idle_anim());
			schedule_act(g, 10); // wait a little
			return;
		}

		throw wexception("MO(%u): [transfer]: from road to bad nextstep %u (type %u)", get_serial(),
					nextstep->get_serial(), nextstep->get_type());
	}

	throw wexception("MO(%u): location %u has bad type %u", get_serial(),
					location->get_serial(), location->get_type());
}


/*
===============
Worker::transfer_signal
===============
*/
void Worker::transfer_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	// The caller requested a route update, or the previously calulcated route
	// failed.
	// We will recalculate the route on the next update().
	if (signal == "road" || signal == "fail") {
		molog("[transfer]: Got signal '%s' -> recalculate\n", signal.c_str());

		set_signal("");
		return;
	}

	molog("[transfer]: Cancel due to signal '%s'\n", signal.c_str());
	pop_task(g);
}


/*
===============
Worker::transfer_mask
===============
*/
void Worker::transfer_mask(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "cancel")
		state->transfer = 0; // dont't call transfer_fail/finish when cancelled
}


/*
===============
Worker::cancel_task_transfer

Called by transport code when the transfer has been cancelled & destroyed.
===============
*/
void Worker::cancel_task_transfer(Game* g)
{
	send_signal(g, "cancel");
}


/*
==============================

BUILDINGWORK task

Endless loop, in which the worker calls the owning building's
get_building_work() function to intiate subtasks.
The signal "update" is used to wake the worker up after a sleeping time
(initiated by a false return value from get_building_work()).

ivar1 - 0: no task has failed; 1: currently in buildingwork;
        2: signal failure of buildingwork

==============================
*/

Bob::Task Worker::taskBuildingwork = {
	"buildingwork",

	(Bob::Ptr)&Worker::buildingwork_update,
	(Bob::Ptr)&Worker::buildingwork_signal,
	0
};


/*
===============
Worker::start_task_buildingwork

Begin work at a building.
===============
*/
void Worker::start_task_buildingwork(Game* g)
{
	push_task(g, &taskBuildingwork);

	get_state()->ivar1 = 0;
}


/*
===============
Worker::buildingwork_update
===============
*/
void Worker::buildingwork_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "location") {
		pop_task(g);
		return;
	}

	// Reset any other signals
	PlayerImmovable* location = get_location(g);

	assert(location);
	assert(location->get_type() == BUILDING);

	set_signal("");

	if (state->ivar1 == 1)
	{
		if (signal == "fail")
			state->ivar1 = 2;
		else
			state->ivar1 = 0;
	}

	// Return to building, if necessary
	BaseImmovable* position = g->get_map()->get_immovable(get_position());

	if (position != location) {
		molog("[buildingwork]: Something went wrong, return home.\n");

		start_task_return(g, false); // don't drop item
		return;
	}

	// Get the new job
	bool success = state->ivar1 != 2;

	// Set this *before* calling to get_building_work, because the
	// state pointer might become invalid
	state->ivar1 = 1;

	if (!((Building*)location)->get_building_work(g, this, success)) {
		molog("[buildingwork]: Nothing to be done.\n");
		set_animation(g, 0);
		skip_act(g);
	}
}


/*
===============
Worker::buildingwork_signal
===============
*/
void Worker::buildingwork_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "update")
		set_signal("");

	schedule_act(g, 1);
}


/*
===============
Worker::update_task_buildingwork

Wake up the buildingwork task if it was sleeping.
Otherwise, the buildingwork task will update as soon as the previous task
is finished.
===============
*/
void Worker::update_task_buildingwork(Game* g)
{
	State* state = get_state();

	if (state->task == &taskBuildingwork)
		send_signal(g, "update");
}


/*
==============================

RETURN task

Return to our owning building.
If dropitem (ivar1) is true, we'll drop our carried item (if any) on the
building's flag, if possible.
Blocks all signals except for "location".

==============================
*/

Bob::Task Worker::taskReturn = {
	"return",

	(Bob::Ptr)&Worker::return_update,
	(Bob::Ptr)&Worker::return_signal,
	0,
};


/*
===============
Worker::start_task_return

Return to our owning building.
===============
*/
void Worker::start_task_return(Game* g, bool dropitem)
{
	PlayerImmovable* location = get_location(g);

	molog("start_task_return\n");

	if (!location || location->get_type() != BUILDING)
		throw wexception("MO(%u): start_task_return(): not owned by building", get_serial());

	push_task(g, &taskReturn);

	molog("pushed task\n");

	get_state()->ivar1 = dropitem ? 1 : 0;

	molog("done\n");
}


/*
===============
Worker::return_update
===============
*/
void Worker::return_update(Game* g, State* state)
{
	PlayerImmovable* location = get_location(g);
	BaseImmovable* pos = g->get_map()->get_immovable(get_position());

	assert(location && location->get_type() == BUILDING); // expect signal "location"

	if (pos)
	{
		if (pos == location) {
			molog("[return]: Back home.\n");

			set_animation(g, 0);
			pop_task(g);
			return;
		}

		if (pos->get_type() == FLAG) {
			Flag* flag = (Flag*)pos;

			// Is this "our" flag?
			if (flag->get_building() == location) {
				if (state->ivar1 && flag->has_capacity()) {
					WareInstance* item = fetch_carried_item(g);

					if (item) {
						molog("[return]: Drop item on flag\n");

						flag->add_item(g, item);

						set_animation(g, get_descr()->get_idle_anim());
						schedule_act(g, 20); // rest a while
						return;
					}
				}

				molog("[return]: Move back into building\n");

				start_task_forcemove(g, WALK_NW, get_descr()->get_right_walk_anims(does_carry_ware()));
				return;
			}
		}
	}

	// Determine the building's flag and move to it
	Flag* flag = location->get_base_flag();

	molog("[return]: Move to building's flag\n");

	if (!start_task_movepath(g, flag->get_position(), 15, get_descr()->get_right_walk_anims(does_carry_ware()))) {
		molog("[return]: Failed to return\n");

		set_location(0);
		return;
	}
}


/*
===============
Worker::return_signal
===============
*/
void Worker::return_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "location") {
		molog("[return]: Interrupted by signal '%s'\n", signal.c_str());
		pop_task(g);
		return;
	}

	molog("[return]: Blocking signal '%s'\n", signal.c_str());
	set_signal("");
}


/*
==============================

PROGRAM task

Follow the steps of a configuration-defined program.
ivar1 is the next action to be performed.
ivar2 is used to store description indices selected by setdescription
objvar1 is used to store objects found by findobject
coords is used to store target coordinates found by findspace

==============================
*/

Bob::Task Worker::taskProgram = {
	"program",

	(Bob::Ptr)&Worker::program_update,
	(Bob::Ptr)&Worker::program_signal,
	0
};


/*
===============
Worker::start_task_program

Start the given program.
===============
*/
void Worker::start_task_program(Game* g, std::string name)
{
	State* state;

	push_task(g, &taskProgram);

	state = get_state();
	state->program = get_descr()->get_program(name);
	state->ivar1 = 0;
}


/*
===============
Worker::program_update
===============
*/
void Worker::program_update(Game* g, State* state)
{
	const WorkerAction* act;

	for(;;)
	{
      const WorkerProgram* program=static_cast<const WorkerProgram*>(state->program);
		molog("[program]: %s#%i\n", program->get_name().c_str(), state->ivar1);

		if (state->ivar1 >= program->get_size()) {
			molog("  End of program\n");
			pop_task(g);
			return;
		}

		act = program->get_action(state->ivar1);

		if ((this->*(act->function))(g, state, act))
			return;
	}
}


/*
===============
Worker::program_signal
===============
*/
void Worker::program_signal(Game* g, State* state)
{
	molog("[program]: Interrupted by signal '%s'\n", get_signal().c_str());
	pop_task(g);
}


/*
==============================

GOWAREHOUSE task

==============================
*/

Bob::Task Worker::taskGowarehouse = {
	"gowarehouse",

	(Bob::Ptr)&Worker::gowarehouse_update,
	(Bob::Ptr)&Worker::gowarehouse_signal,
	0,
};

/*
===============
Worker::start_task_gowarehouse

Get the worker to move to the nearest warehouse.
The worker is added to the list of usable wares, so he may be reassigned to
a new task immediately.
===============
*/
void Worker::start_task_gowarehouse(Game* g)
{
	assert(!m_supply);

	push_task(g, &taskGowarehouse);

	m_supply = new IdleWorkerSupply(this);
}


/*
===============
Worker::gowarehouse_update
===============
*/
void Worker::gowarehouse_update(Game* g, State* state)
{
	PlayerImmovable *location = get_location(g);

	assert(location); // 'location' signal expected otherwise

	if (location->get_type() == BUILDING && location->has_attribute(WAREHOUSE)) {
		molog("[gowarehouse]: Back in warehouse, schedule incorporate\n");

		delete m_supply;
		m_supply = 0;

		schedule_incorporate(g);
		return;
	}

	// If we got a transfer, use it
	if (state->transfer) {
		Transfer* t = state->transfer;

		molog("[gowarehouse]: Got a Transfer\n");

		state->transfer = 0;
		delete m_supply;
		m_supply = 0;

		pop_task(g);
		start_task_transfer(g, t);
		return;
	}

	if (!get_economy()->get_nr_warehouses()) {
		molog("[gowarehouse]: No warehouse left in Economy\n");

		delete m_supply;
		m_supply = 0;

		pop_task(g);
		return;
	}

	// Idle until we are assigned a transfer.
	// The delay length is rather arbitrary, but we need some kind of
	// check against disappearing warehouses, or the worker will just
	// idle on a flag until the end of days (actually, until either the
	// flag is removed or a warehouse connects to the Economy).
	molog("[gowarehouse]: Idle\n");

	start_task_idle(g, get_idle_anim(), 1000);
}


/*
===============
Worker::gowarehouse_signal
===============
*/
void Worker::gowarehouse_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	// if routing has failed, try a different warehouse/route on next update()
	if (signal == "fail") {
		molog("[gowarehouse]: caught 'fail'\n");

		set_signal("");
		return;
	}

	if (signal == "transfer") {
		molog("[gowarehouse]: transfer signal\n");

		delete m_supply;
		m_supply = 0;

		schedule_act(g, 1);
		set_signal("");
		return;
	}

	molog("[gowarehouse]: cancel for signal '%s'\n", signal.c_str());

	delete m_supply;
	m_supply = 0;
	pop_task(g);
}


/*
==============================

DROPOFF task

==============================
*/

Bob::Task Worker::taskDropoff = {
	"dropoff",

	(Bob::Ptr)&Worker::dropoff_update,
	0,
	0,
};

/*
===============
Worker::set_job_dropoff

Walk to the building's flag, drop the given item, and walk back inside.
===============
*/
void Worker::start_task_dropoff(Game* g, WareInstance* item)
{
	assert(item);

	set_carried_item(g, item);

	push_task(g, &taskDropoff);
}


/*
===============
Worker::dropoff_update
===============
*/
void Worker::dropoff_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal.size()) {
		molog("[dropoff]: Interrupted by signal '%s'\n", signal.c_str());
		pop_task(g);
		return;
	}

	WareInstance* item = get_carried_item(g);
	BaseImmovable* location = g->get_map()->get_immovable(get_position());
	#ifdef DEBUG
	PlayerImmovable* ploc = get_location(g);
	#endif

	assert(ploc && ploc->get_type() == BUILDING);
	assert(location && (ploc == location || ploc->get_base_flag() == location));

	// Deliver the item
	if (item)
	{
		// We're in the building, walk onto the flag
		if (location->get_type() == Map_Object::BUILDING)
		{
			Flag* flag = ((PlayerImmovable*)location)->get_base_flag();

			if (start_task_waitforcapacity(g, flag))
				return;

			// Exit throttle
			molog("[dropoff]: move from building to flag\n");
			start_task_leavebuilding(g, false);
			return;
		}

		// We're on the flag, drop the item and pause a little
		if (location->get_type() == Map_Object::FLAG)
		{
			Flag* flag = (Flag*)location;

			if (flag->has_capacity()) {
				molog("[dropoff]: dropping the item\n");

				item = fetch_carried_item(g);
				flag->add_item(g, item);

				set_animation(g, get_descr()->get_idle_anim());
				schedule_act(g, 50);
				return;
			}

			molog("[dropoff]: flag is overloaded\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_right_walk_anims(does_carry_ware()));
			return;
		}

		throw wexception("MO(%u): [dropoff]: not on building or on flag - fishy", get_serial());
	}

	// We don't have the item any more, return home
	if (location->get_type() == Map_Object::FLAG) {
		start_task_forcemove(g, WALK_NW, get_descr()->get_right_walk_anims(does_carry_ware()));
		return;
	}

	if (location->get_type() != Map_Object::BUILDING)
		throw wexception("MO(%u): [dropoff]: not on building on return", get_serial());

	if (location->has_attribute(WAREHOUSE)) {
		schedule_incorporate(g);
		return;
	}

	// Our parent task should know what to do
	molog("[dropoff]: back in building\n");
	pop_task(g);
}


/*
==============================

FETCHFROMFLAG task

ivar1 is set to 0 if we should move to the flag and fetch the item, and it is
set to 1 if we should move into the building.

==============================
*/

Bob::Task Worker::taskFetchfromflag = {
	"fetchfromflag",

	(Bob::Ptr)&Worker::fetchfromflag_update,
	0,
	0,
};


/*
===============
Worker::start_task_fetchfromflag

Walk to the building's flag, fetch an item from the flag that is destined for
the building, and walk back inside.
===============
*/
void Worker::start_task_fetchfromflag(Game* g)
{
	push_task(g, &taskFetchfromflag);

	get_state()->ivar1 = 0;
}


/*
===============
Worker::fetchfromflag_update
===============
*/
void Worker::fetchfromflag_update(Game *g, State* state)
{
	PlayerImmovable* owner = get_location(g);
	WareInstance* item = get_carried_item(g);

	assert(owner); // expect 'location' signal

	BaseImmovable* location = g->get_map()->get_immovable(get_position());

	// If we haven't got the item yet, walk onto the flag
	if (!item && !state->ivar1) {
		if (location->get_type() == BUILDING) {
			molog("[fetchfromflag]: move from building to flag\n");
			start_task_leavebuilding(g, false);
			return;
		}

		// This can't happen because of the owner check above
		if (location->get_type() != FLAG)
			throw wexception("MO(%u): [fetchfromflag]: flag disappeared", get_serial());

		item = ((Flag*)location)->fetch_pending_item(g, owner);
		state->ivar1 = 1; // force return to building

		// The item has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if (item)
			set_carried_item(g, item);

		set_animation(g, get_descr()->get_idle_anim());
		schedule_act(g, 20);
		return;
	}

	// Go back into the building
	if (location->get_type() == FLAG) {
		molog("[fetchfromflag]: return to building\n");

		start_task_forcemove(g, WALK_NW, get_descr()->get_right_walk_anims(does_carry_ware()));
		return;
	}

	if (location->get_type() != BUILDING)
		throw wexception("MO(%u): [fetchfromflag]: building disappeared", get_serial());

	assert(location == owner);

	molog("[fetchfromflag]: back home\n");

	item = fetch_carried_item(g);
	item->set_location(g, location);
	item->update(g); // this might remove the item and ack any requests

	// We're back!
	if (location->has_attribute(WAREHOUSE)) {
		schedule_incorporate(g);
		return;
	}

	pop_task(g); // assume our parent task knows what to do
}


/*
==============================

WAITFORCAPACITY task

Wait for available capacity on a flag.

==============================
*/

Bob::Task Worker::taskWaitforcapacity = {
	"waitforcapacity",

	(Bob::Ptr)&Worker::waitforcapacity_update,
	(Bob::Ptr)&Worker::waitforcapacity_signal,
	0,
};

/*
===============
Worker::start_task_waitforcapacity

Checks the capacity of the flag.
If there is none, a wait task is pushed, and the worker is added to the flag's
wait queue. The function returns true in this case.
If the flag still has capacity, the function returns false and doesn't act at
all.
===============
*/
bool Worker::start_task_waitforcapacity(Game* g, Flag* flag)
{
	if (flag->has_capacity())
		return false;

	push_task(g, &taskWaitforcapacity);

	get_state()->objvar1 = flag;

	flag->wait_for_capacity(g, this);

	return true;
}


/*
===============
Worker::waitforcapacity_update
===============
*/
void Worker::waitforcapacity_update(Game* g, State* state)
{
	skip_act(g); // wait indefinitely
}


/*
===============
Worker::waitforcapacity_signal
===============
*/
void Worker::waitforcapacity_signal(Game* g, State* state)
{
	// The 'wakeup' signal is to be expected; don't propagate it
	if (get_signal() == "wakeup")
		set_signal("");

	pop_task(g);
}

/*
===============
Worker::wakeup_flag_capacity

Called when the flag we waited on has now got capacity left.
Return true if we actually woke up due to this.
===============
*/
bool Worker::wakeup_flag_capacity(Game* g, Flag* flag)
{
	State* state = get_state();

	if (state && state->task == &taskWaitforcapacity) {
		molog("[waitforcapacity]: Wake up: flag capacity.\n");

		if (state->objvar1.get(g) != flag)
			throw wexception("MO(%u): wakeup_flag_capacity: Flags don't match.", get_serial());

		send_signal(g, "wakeup");
		return true;
	}

	return false;
}


/*
==============================

WAITLEAVEBUILDING task

ivar1 - 0: don't change location; 1: change location to the flag
objvar1 - the building we're leaving

==============================
*/

Bob::Task Worker::taskLeavebuilding = {
	"leavebuilding",

	(Bob::Ptr)&Worker::leavebuilding_update,
	(Bob::Ptr)&Worker::leavebuilding_signal,
	0,
};


/*
===============
Worker::start_task_leavebuilding

Leave the current building.
Waits on the buildings leave wait queue if necessary.

If changelocation is true, change the location to the flag once we're outside.
===============
*/
void Worker::start_task_leavebuilding(Game* g, bool changelocation)
{
	PlayerImmovable* location = get_location(g);

	assert(location && location->get_type() == BUILDING);

	Building* building = (Building*)location;

	// Set the wait task
	push_task(g, &taskLeavebuilding);

	get_state()->ivar1 = changelocation ? 1 : 0;
	get_state()->objvar1 = building;
}


/*
===============
Worker::leavebuilding_update
===============
*/
void Worker::leavebuilding_update(Game* g, State* state)
{
	BaseImmovable* position = g->get_map()->get_immovable(get_position());

	if (!position || position->get_type() != BUILDING) {
		molog("[leavebuilding]: Left the building successfully\n");
		pop_task(g);
		return;
	}

	Building* building = (Building*)position;

	assert(building == state->objvar1.get(g));

	if (!building->leave_check_and_wait(g, this)) {
		molog("[leavebuilding]: Wait\n");
		skip_act(g);
		return;
	}

	molog("[leavebuilding]: Leave (%s)\n", state->ivar1 ? "change location" : "stay in location");

	if (state->ivar1)
	{
		set_location(building->get_base_flag());
	}

	start_task_forcemove(g, WALK_SE, get_descr()->get_right_walk_anims(does_carry_ware()));
}


/*
===============
Worker::leavebuilding_signal
===============
*/
void Worker::leavebuilding_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "wakeup") {
		molog("[leavebuilding]: Wake up\n");
		set_signal("");
		schedule_act(g, 1);
		return;
	}

	molog("[leavebuilding]: Interrupted by signal '%s'\n", signal.c_str());
	pop_task(g);
}


/*
===============
Worker::wakeup_leave_building

Called when the given building allows us to leave it.
Return true if we actually woke up due to this.
===============
*/
bool Worker::wakeup_leave_building(Game* g, Building* building)
{
	State* state = get_state();

	molog("wakeup_leave_building called\n");

	if (state && state->task == &taskLeavebuilding) {
		molog("[leavebuilding]: wakeup\n");

		if (state->objvar1.get(g) != building)
			throw wexception("MO(%u): [waitleavebuilding]: buildings don't match", get_serial());

		send_signal(g, "wakeup");
		return true;
	}

	return false;
}



/*
==============================

FUGITIVE task

Run around aimlessly until we find a warehouse.

==============================
*/

Bob::Task Worker::taskFugitive = {
	"fugitive",

	(Bob::Ptr)&Worker::fugitive_update,
	(Bob::Ptr)&Worker::fugitive_signal,
	0,
};


/*
===============
Worker::start_task_fugitive
===============
*/
void Worker::start_task_fugitive(Game* g)
{
	push_task(g, &taskFugitive);

	// Fugitives survive for two to four minutes
	get_state()->ivar1 = g->get_gametime() + 120000 + 200*(g->logic_rand() % 600);
}


/*
===============
Worker::fugitive_update
===============
*/
void Worker::fugitive_update(Game* g, State* state)
{
	Map *map = g->get_map();
	PlayerImmovable *location = get_location(g);

	if (location && location->get_owner() == get_owner()) {
		molog("[fugitive]: we're on location\n");

		if (location->has_attribute(WAREHOUSE)) {
			schedule_incorporate(g);
			return;
		}

		set_location(0);
		location = 0;
	}

	// check whether we're on a flag and it's time to return home
	BaseImmovable *imm = map->get_immovable(get_position());

	if (imm && imm->get_type() == FLAG) {
		Flag *flag = (Flag*)imm;
		Building *building = flag->get_building();

		if (building && building->has_attribute(WAREHOUSE) && building->get_owner() == get_owner()) {
			molog("[fugitive]: move into warehouse\n");
			start_task_forcemove(g, WALK_NW, get_descr()->get_right_walk_anims(does_carry_ware()));
			set_location(building);
			return;
		}
	}

	// time to die?
	if (g->get_gametime() - state->ivar1 >= 0) {
		molog("[fugitive]: die\n");

		schedule_destroy(g);
		return;
	}

	// Try to find a warehouse we can return to
	std::vector<ImmovableFound> warehouses;

	if (map->find_immovables(get_position(), 15, &warehouses, FindImmovableAttribute(WAREHOUSE))) {
		int bestdist = -1;
		Warehouse *best = 0;

		molog("[fugitive]: found warehouse(s)\n");

		for(uint i = 0; i < warehouses.size(); i++) {
			Warehouse *wh = (Warehouse*)warehouses[i].object;

			// Only walk into one of our warehouses
			if (wh->get_owner() != get_owner())
				continue;

			int dist = map->calc_distance(get_position(), warehouses[i].coords);

			if (!best || dist < bestdist) {
				best = wh;
				bestdist = dist;
			}
		}

		if (best)
		{
			bool use = false;

			if ((g->logic_rand() % 30) <= (30 - bestdist))
				use = true;

			// okay, move towards the flag of this warehouse
			if (use) {
				Flag *flag = best->get_base_flag();

				molog("[fugitive]: try to move to warehouse\n");

				// the warehouse could be on a different island, so check for failure
				if (start_task_movepath(g, flag->get_position(), 0, get_descr()->get_right_walk_anims(does_carry_ware())))
					return;
			}
		}
	}

	// just walk into a random direction
	Coords dst;

	molog("[fugitive]: wander randomly\n");

	dst.x = get_position().x + (g->logic_rand()%11) - 5;
	dst.y = get_position().y + (g->logic_rand()%11) - 5;

	if (start_task_movepath(g, dst, 4, get_descr()->get_right_walk_anims(does_carry_ware())))
		return;

	start_task_idle(g, get_descr()->get_idle_anim(), 50);
}


/*
===============
Worker::fugitive_signal
===============
*/
void Worker::fugitive_signal(Game* g, State* state)
{
	molog("[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
	pop_task(g);
}


/*
==============================

GEOLOGIST task

Walk in a circle around our owner, calling a subprogram on currently
empty fields.

ivar1 - number of attempts
ivar2 - radius to search
svar1 - name of subcommand

Failure of path movement is caught, all other signals terminate this task.

==============================
*/

Bob::Task Worker::taskGeologist = {
	"geologist",

	(Bob::Ptr)&Worker::geologist_update,
	0,
	0,
};


/*
===============
Worker::start_task_geologist
===============
*/
void Worker::start_task_geologist(Game* g, int attempts, int radius, std::string subcommand)
{
	push_task(g, &taskGeologist);

	State* s = get_state();

	s->ivar1 = attempts;
	s->ivar2 = radius;
	s->svar1 = subcommand;
}


/*
===============
Worker::geologist_update
===============
*/
void Worker::geologist_update(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "fail")
	{
		molog("[geologist]: Caught signal '%s'\n", signal.c_str());
		set_signal("");
	}
	else if (signal.size())
	{
		molog("[geologist]: Interrupted by signal '%s'\n", signal.c_str());
		pop_task(g);
		return;
	}

	//
	Map* map = g->get_map();
	PlayerImmovable* location = get_location(g);
	Flag* owner;
	Coords center;

	assert(location);
	assert(location->get_type() == FLAG);

	owner = (Flag*)location;
	center = owner->get_position();

	// Check if it's time to go home
	if (state->ivar1 > 0)
	{
		// Check to see if we're on suitable terrain
		BaseImmovable* imm = map->get_immovable(get_position());

		if (!imm ||
		    (imm->get_size() == BaseImmovable::NONE && !imm->has_attribute(RESI)))
		{
			molog("[geologist]: Starting program '%s'\n", state->svar1.c_str());

			state->ivar1--;
			start_task_program(g, state->svar1);
			return;
		}

		// Find a suitable field and walk towards it
		std::vector<Coords> list;
		CheckStepDefault cstep(get_movecaps());
		FindFieldImmovableSize ffis(FindFieldImmovableSize::sizeNone);
		FindFieldImmovableAttribute ffia(RESI);
		FindFieldAnd ffa;

		ffa.add(&ffis, false);
		ffa.add(&ffia, true);

		if (map->find_reachable_fields(center, state->ivar2, &list, &cstep, ffa))
      {
         Coords target;

         // is center a mountain piece?
         bool is_center_mountain= (map->get_field(center)->get_terd()->get_is() & TERRAIN_MOUNTAIN) |
            (map->get_field(center)->get_terr()->get_is() & TERRAIN_MOUNTAIN);
         // Only run towards fields that are on a mountain (or not)
         // depending on position of center
         bool is_target_mountain;
         uint n=list.size();
         uint i=g->logic_rand() % list.size();
         do {
            molog("[geologist] Searching for a suitable field!\n");
            target = list[g->logic_rand() % list.size()];
            is_target_mountain = (map->get_field(target)->get_terd()->get_is() & TERRAIN_MOUNTAIN) |
               (map->get_field(target)->get_terr()->get_is() & TERRAIN_MOUNTAIN);
            if(i==0) i=list.size();
            --i;
            --n;
         } while( (is_center_mountain != is_target_mountain) && n );

         if(!n) {
            // no suitable field found, this is no fail, there's just nothing else to do
            // so let's go home
            // FALLTHROUGH TO RETURN HOME
         } else {
            molog("[geologist]: Walk towards free field\n");
            if (!start_task_movepath(g, target, 0, get_descr()->get_right_walk_anims(does_carry_ware())))
            {
               molog("[geologist]: BUG: couldn't find path\n");
               set_signal("fail");
               pop_task(g);
               return;
            }
            return;
         }
      }

		molog("[geologist]: Found no applicable field, going home\n");
		state->ivar1 = 0;
	}

	if (get_position() == center) {
		molog("[geologist]: We're home\n");
		pop_task(g);
		return;
	}

	molog("[geologist]: Return home\n");
	if (!start_task_movepath(g, center, 0, get_descr()->get_right_walk_anims(does_carry_ware())))
	{
		molog("[geologist]: Couldn't find path home\n");
		set_signal("fail");
		pop_task(g);
		return;
	}
}


/*
===============
Worker::draw

Draw the worker, taking the carried item into account.
===============
*/
void Worker::draw(Editor_Game_Base* g, RenderTarget* dst, Point pos)
{
	uint anim = get_current_anim();

	if (!anim)
		return;

	const RGBColor* playercolors = 0;
	Point drawpos;

	calc_drawpos(g, pos, &drawpos);

	if (get_owner())
		playercolors = get_owner()->get_playercolor();

	dst->drawanim(drawpos.x, drawpos.y, anim, g->get_gametime() - get_animstart(), playercolors);

	// Draw the currently carried item
	WareInstance* item = get_carried_item(g);

	if (item) {
		uint itemanim = item->get_ware_descr()->get_idle_anim();

		dst->drawanim(drawpos.x, drawpos.y - 15, itemanim, 0, playercolors);
	}
}


/*
===============
Worker_Descr::create_object

Create a generic worker of this type.
===============
*/
Bob* Worker_Descr::create_object()
{
	return new Worker(this);
}


/*
==============================================================================

Carrier IMPLEMENTATION

==============================================================================
*/

/*
===============
Carrier_Descr::Carrier_Descr
Carrier_Descr::~Carrier_Descr
===============
*/
Carrier_Descr::Carrier_Descr(Tribe_Descr *tribe, const char *name)
	: Worker_Descr(tribe, name)
{
}

Carrier_Descr::~Carrier_Descr(void)
{
}

/*
===============
Carrier_Descr::parse

Parse carrier-specific configuration data
===============
*/
void Carrier_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Worker_Descr::parse(directory, prof, encdata);
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Carrier::Carrier
Carrier::~Carrier
===============
*/
Carrier::Carrier(Carrier_Descr *descr)
	: Worker(descr)
{
	m_acked_ware = -1;
}

Carrier::~Carrier()
{
}


/*
==============================

ROAD task

Signal "road" on road split.
Signal "ware" when a ware has arrived.

==============================
*/

Bob::Task Carrier::taskRoad = {
	"road",

	(Bob::Ptr)&Carrier::road_update,
	(Bob::Ptr)&Carrier::road_signal,
	0,
};


/*
===============
Carrier::start_task_road

Work on the given road, assume the location is correct.
===============
*/
void Carrier::start_task_road(Game* g, Road* road)
{
	assert(get_location(g) == road);

	push_task(g, &taskRoad);

	get_state()->ivar1 = 0;

	m_acked_ware = -1;
}


/*
===============
Carrier::update_task_road

Called by Road code when the road is split.
===============
*/
void Carrier::update_task_road(Game* g)
{
	send_signal(g, "road");
}


/*
===============
Carrier::road_update
===============
*/
void Carrier::road_update(Game* g, State* state)
{
	Road* road = (Road*)get_location(g);

	assert(road); // expect 'location' signal

	// Check for pending items
	if (m_acked_ware < 0)
		find_pending_item(g);

	if (m_acked_ware >= 0) {
		if (state->ivar1)
		{
			molog("[road]: Go pick up item from %i\n", m_acked_ware);

			state->ivar1 = 0;
			start_task_transport(g, m_acked_ware);
		}
		else
		{
			// Short delay before we move to pick up
			molog("[road]: delay (acked for %i)\n", m_acked_ware);
			state->ivar1 = 1;

			set_animation(g, get_descr()->get_idle_anim());
			schedule_act(g, 50);
		}

		return;
	}

	// Move into idle position if necessary
	if (start_task_movepath(g, road->get_path(), road->get_idle_index(), get_descr()->get_right_walk_anims(does_carry_ware())))
		return;

	// Be bored. There's nothing good on TV, either.
	molog("[road]: Idle.\n");

	// TODO: idle animations

	set_animation(g, get_descr()->get_idle_anim());
	skip_act(g); // wait until signal
	state->ivar1 = 1; // we're available immediately after an idle phase
}


/*
===============
Carrier::road_signal
===============
*/
void Carrier::road_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "road" || signal == "ware") {
		set_signal(""); // update() will do the rest
		schedule_act(g, 10);
		return;
	}

	molog("[road]: Terminated by signal '%s'\n", signal.c_str());
	pop_task(g);
}


/*
==============================

TRANSPORT task

Fetch an item from a flag, drop it on the other flag.
ivar1 is the flag we fetch from, or -1 when we're in the target building.

Signal "update" when the road has been split etc.

==============================
*/

Bob::Task Carrier::taskTransport = {
	"transport",

	(Bob::Ptr)&Carrier::transport_update,
	(Bob::Ptr)&Carrier::transport_signal,
	0,
};


/*
===============
Carrier::start_task_transport

Begin the transport task.
===============
*/
void Carrier::start_task_transport(Game* g, int fromflag)
{
	State* state;

	assert(!get_carried_item(g));

	push_task(g, &taskTransport);

	state = get_state();
	state->ivar1 = fromflag;
}


/*
===============
Carrier::transport_update
===============
*/
void Carrier::transport_update(Game* g, State* state)
{
	WareInstance* item;
	Road* road = (Road*)get_location(g);

	assert(road); // expect 'location' signal

	//molog("[transport]\n");

	// If we're "in" the target building, special code applies
	if (state->ivar1 == -1)
	{
		BaseImmovable* pos = g->get_map()->get_immovable(get_position());

		// tough luck, the building has disappeared
		if (!pos || (pos->get_type() != BUILDING && pos->get_type() != FLAG)) {
			molog("[transport]: Building disappeared while in building.\n");

			set_location(0);
			return;
		}

		// Drop the item, indicating success
		if (pos->get_type() == Map_Object::BUILDING) {
			item = fetch_carried_item(g);

			if (item) {
				molog("[transport]: Arrived in building.\n");
				item->set_location(g, (Building*)pos);
				item->update(g);

				set_animation(g, get_descr()->get_idle_anim());
				schedule_act(g, 20);
				return;
			}

			// Now walk back onto the flag
			molog("[transport]: Move out of building.\n");
			start_task_forcemove(g, WALK_SE, get_descr()->get_right_walk_anims(does_carry_ware()));
			return;
		}

		// We're done
		pop_task(g);
		return;
	}

	// If we don't carry something, walk to the flag
	if (!get_carried_item(g)) {
		Flag* flag;
		Flag* otherflag;

		if (start_task_walktoflag(g, state->ivar1))
			return;

		molog("[transport]: pick up from flag.\n");
		m_acked_ware = -1;

		flag = road->get_flag((Road::FlagId)state->ivar1);
		otherflag = road->get_flag((Road::FlagId)(state->ivar1 ^ 1));
		item = flag->fetch_pending_item(g, otherflag);

		if (!item) {
			molog("[transport]: Nothing on flag.\n");
			pop_task(g);
			return;
		}

		set_carried_item(g, item);

		set_animation(g, get_descr()->get_idle_anim());
		schedule_act(g, 20);
		return;
	}

	// If the item should go to the building attached to our flag, walk directly
	// into said building
	Flag* flag;

	item = get_carried_item(g);
	flag = road->get_flag((Road::FlagId)(state->ivar1 ^ 1));

	assert(item->get_location(g) == this);

	// A sanity check is necessary, in case the building has been destroyed
	PlayerImmovable* next = item->get_next_move_step(g);

	if (next && next != flag && next->get_base_flag() == flag) {
		if (start_task_walktoflag(g, state->ivar1 ^ 1))
			return;

		molog("[transport]: Move into building.\n");
		start_task_forcemove(g, WALK_NW, get_descr()->get_right_walk_anims(does_carry_ware()));
		state->ivar1 = -1;
		return;
	}

	// Move into waiting position if the flag is overloaded
	if (!flag->has_capacity())
	{
		Flag *otherflag = road->get_flag((Road::FlagId)state->ivar1);

		if (m_acked_ware == (state->ivar1 ^ 1))
		{
			// All is well, we already acked an item that we can pick up
			// from this flag
		}
		else if (flag->has_pending_item(g, otherflag))
		{
			if (!flag->ack_pending_item(g, otherflag))
				throw wexception("MO(%u): transport: overload exchange: flag %u is fucked up",
							get_serial(), flag->get_serial());

			m_acked_ware = state->ivar1 ^ 1;
		}
		else
		{
			if (start_task_walktoflag(g, state->ivar1 ^ 1, true))
				return;

			// Wait one field away
			start_task_waitforcapacity(g, flag);
			return;
		}
	}

	// If there is capacity, walk to the flag
	if (start_task_walktoflag(g, state->ivar1 ^ 1))
		return;

	// Drop the item, possible exchanging it with another one
	WareInstance* otheritem = 0;

	if (m_acked_ware == (state->ivar1 ^ 1)) {
		otheritem = flag->fetch_pending_item(g, road->get_flag((Road::FlagId)state->ivar1));

		if (!otheritem && !flag->has_capacity()) {
			molog("[transport]: strange: acked ware from busy flag no longer present.\n");

			m_acked_ware = -1;
			set_animation(g, get_descr()->get_idle_anim());
			schedule_act(g, 20);
			return;
		}

		state->ivar1 = m_acked_ware;
		m_acked_ware = -1;
	}

	item = fetch_carried_item(g);
	flag->add_item(g, item);

	if (otheritem)
	{
		molog("[transport]: return trip.\n");

		set_carried_item(g, otheritem);

		set_animation(g, get_descr()->get_idle_anim());
		schedule_act(g, 20);
		return;
	}
	else
	{
		molog("[transport]: back to idle.\n");
		pop_task(g);
	}
}


/*
===============
Carrier::transport_signal
===============
*/
void Carrier::transport_signal(Game* g, State* state)
{
	std::string signal = get_signal();

	if (signal == "road") {
		set_signal("");
		schedule_act(g, 10);
		return;
	}

	molog("[transport]: Interrupted by signal '%s'\n", signal.c_str());
	pop_task(g);
}


/*
===============
Carrier::notify_ware

Called by Road code to indicate that a new item has arrived on a flag
(0 = start, 1 = end).
Returns true if the carrier is going to fetch it.
===============
*/
bool Carrier::notify_ware(Game* g, int flag)
{
	State* state = get_state();

	// Check if we've already acked something
	if (m_acked_ware >= 0) {
		molog("notify_ware(%i): already acked %i\n", flag, m_acked_ware);
		return false;
	}

	// If we are currently in a transport.
	// Explanation:
	//  a) a different carrier / road may be better suited for this ware
	//     (the transport code does not have priorities for the actual
	//     carrier that is notified)
	//  b) the transport task has logic that allows it to
	//     drop an item on an overloaded flag iff it can pick up an item
	//     at the same time.
	//     We should ack said item to avoid more confusion before we move
	//     onto the flag, but we can't do that if we have already acked
	//     something.
	//  c) we might ack for a flag that we are actually moving away from;
	//     this will get us into trouble if items have arrived on the other
	//     flag while we couldn't ack them.
	//
	// (Maybe the need for this lengthy explanation is proof that the
	// ack system needs to be reworked.)
	State* transport = get_state(&taskTransport);

	if (transport) {
		if ((transport->ivar1 == -1 && find_closest_flag(g) != flag) || flag == transport->ivar1)
			return false;
	}

	// Ack it if we haven't
	molog("notify_ware(%i)\n", flag);
	m_acked_ware = flag;

	if (state->task == &taskRoad)
		send_signal(g, "ware");
	else if (state->task == &taskWaitforcapacity)
		send_signal(g, "wakeup");

	return true;
}


/*
===============
Carrier::find_pending_item

Find a pending item on one of the road's flags, ack it and set m_acked_ware
accordingly.
===============
*/
void Carrier::find_pending_item(Game* g)
{
	Road* road = (Road*)get_location(g);
	uint haveitembits = 0;

	assert(m_acked_ware < 0);

	if (road->get_flag(Road::FlagStart)->has_pending_item(g, road->get_flag(Road::FlagEnd)))
		haveitembits |= 1;

	if (road->get_flag(Road::FlagEnd)->has_pending_item(g, road->get_flag(Road::FlagStart)))
		haveitembits |= 2;

	// If both roads have an item, we pick the one closer to us
	if (haveitembits == 3)
		haveitembits = (1 << find_closest_flag(g));

	// Ack our decision
	if (haveitembits == 1)
	{
		bool ok = false;

		molog("find_pending_item: flag %i\n", 0);
		m_acked_ware = 0;

		ok = road->get_flag(Road::FlagStart)->ack_pending_item(g, road->get_flag(Road::FlagEnd));
		if (!ok)
			throw wexception("Carrier::find_pending_item: start flag is messed up");

		return;
	}

	if (haveitembits == 2)
	{
		bool ok = false;

		molog("find_pending_item: flag %i\n", 1);
		m_acked_ware = 1;

		ok = road->get_flag(Road::FlagEnd)->ack_pending_item(g, road->get_flag(Road::FlagStart));
		if (!ok)
			throw wexception("Carrier::find_pending_item: end flag is messed up");

		return;
	}
}


/*
===============
Carrier::find_closest_flag

Find the flag we are closest to (in walking time).
===============
*/
int Carrier::find_closest_flag(Game* g)
{
	Road* road = (Road*)get_location(g);
	CoordPath startpath;
	CoordPath endpath;
	int startcost, endcost;
	int curidx = -1;

	startpath = road->get_path();
	curidx = startpath.get_index(get_position());

	// Apparently, we're in a building
	if (curidx < 0) {
		Coords pos = get_position();

		g->get_map()->get_brn(pos, &pos);

		if (pos == startpath.get_start())
			curidx = 0;
		else if (pos == startpath.get_end())
			curidx = startpath.get_nsteps();
		else
			throw wexception("MO(%u): Carrier::find_closest_flag: not on road, not on building",
						get_serial());
	}

	// Calculate the paths and their associated costs
	endpath = startpath;

	startpath.truncate(curidx);
	startpath.reverse();

	endpath.starttrim(curidx);

	g->get_map()->calc_cost(startpath, &startcost, 0);
	g->get_map()->calc_cost(endpath, &endcost, 0);

	if (startcost <= endcost)
		return 0;
	else
		return 1;
}


/*
===============
Carrier::start_task_walktoflag

Walk to the given flag, or one field before it if offset is true.
Returns true if a move task has been started, or false if we're already on
the target field.
===============
*/
bool Carrier::start_task_walktoflag(Game* g, int flag, bool offset)
{
	Road* road = (Road*)get_location(g);
	const Path& path = road->get_path();
	int idx;

	if (!flag) {
		idx = 0;
		if (offset)
			idx++;
	} else {
		idx = path.get_nsteps();
		if (offset)
			idx--;
	}

	return start_task_movepath(g, path, idx, get_descr()->get_right_walk_anims(does_carry_ware()));
}


/*
===============
Carrier_Descr::create_object

Create a carrier of this type.
===============
*/
Bob *Carrier_Descr::create_object()
{
	return new Carrier(this);
}


/*
==============================================================================

Worker factory

==============================================================================
*/

/*
===============
Worker_Descr::create_from_dir [static]

Automatically create the appropriate Worker_Descr type from the given
config data.
May return 0.
===============
*/
Worker_Descr *Worker_Descr::create_from_dir(Tribe_Descr *tribe, const char *directory, const EncodeData *encdata)
{
	const char *name;

	// name = last element of path
	const char *slash = strrchr(directory, '/');
	const char *backslash = strrchr(directory, '\\');

	if (backslash && (!slash || backslash > slash))
		slash = backslash;

	if (slash)
		name = slash+1;
	else
		name = directory;

	// Open the config file
	Worker_Descr *descr = 0;
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/conf", directory);

	if (!g_fs->FileExists(fname))
		return 0;

	try
	{
		Profile prof(fname);
		Section *s = prof.get_safe_section("global");
		const char *type = s->get_safe_string("type");

		if (!strcasecmp(type, "generic"))
			descr = new Worker_Descr(tribe, name);
		else if (!strcasecmp(type, "carrier"))
			descr = new Carrier_Descr(tribe, name);
		else if (!strcasecmp(type, "soldier"))
			descr = new Soldier_Descr(tribe, name);
		else
			throw wexception("Unknown worker type '%s' [supported: carrier, soldier]", type);

		descr->parse(directory, &prof, encdata);
	}
	catch(std::exception &e) {
		if (descr)
			delete descr;
		throw wexception("Error reading worker %s: %s", name, e.what());
	}
	catch(...) {
		if (descr)
			delete descr;
		throw;
	}

	return descr;
}

