/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include "widelands.h"
#include "graphic.h"
#include "profile.h"
#include "game.h"
#include "map.h"
#include "immovable.h"
#include "graphic.h"
#include "sw16_graphic.h"

/*
==============================================================================

BaseImmovable IMPLEMENTATION

==============================================================================
*/

/*
===============
BaseImmovable::BaseImmovable
BaseImmovable::~BaseImmovable

Base immovable creation and destruction
===============
*/
BaseImmovable::BaseImmovable(Map_Object_Descr *descr)
	: Map_Object(descr)
{
}

BaseImmovable::~BaseImmovable()
{
}


/*
===============
BaseImmovable::set_position

Associate the given field with this immovable. Recalculate if necessary.
Only call this during init.
===============
*/
void BaseImmovable::set_position(Editor_Game_Base *g, Coords c)
{
	Field *f = g->get_map()->get_field(c);

	if (f->immovable) {
		BaseImmovable *other = f->immovable;

		assert(other->get_size() == NONE);

		other->cleanup(g);
		delete other;
	}

	f->immovable = this;

	if (get_size() >= SMALL)
		g->recalc_for_field(c);
}

/*
===============
BaseImmovable::unset_position

Remove the link to the given field.
Only call this during cleanup.
===============
*/
void BaseImmovable::unset_position(Editor_Game_Base *g, Coords c)
{
	Field *f = g->get_map()->get_field(c);

	assert(f->immovable == this);

	f->immovable = 0;

	if (get_size() >= SMALL)
		g->recalc_for_field(c);
}


/*
==============================================================================

ImmovableProgram IMPLEMENTATION

==============================================================================
*/

// Additional parameters for op parsing routines
struct ProgramParser {
	Immovable_Descr*		descr;
	std::string				directory;
	Profile* 				prof;
};

// One action of a program
struct ImmovableAction {
	typedef bool (Immovable::*execute_t)(Game* g, bool killable, const ImmovableAction& action);

	execute_t	function;
	int			iparam1;
	int			iparam2;
	std::string	sparam;
};

// The ImmovableProgram
class ImmovableProgram {
	typedef void (ImmovableProgram::*parse_t)(ImmovableAction* act, const ProgramParser* parser,
																				const std::vector<std::string>& cmd);

public:
	ImmovableProgram(std::string name);

	std::string get_name() const { return m_name; }
	uint get_size() const { return m_actions.size(); }
	const ImmovableAction& get_action(uint idx) const { assert(idx < m_actions.size()); return m_actions[idx]; }

	void add_action(const ImmovableAction& act);
	void parse(Immovable_Descr* descr, std::string directory, Profile* prof);

private:
	void parse_animation(ImmovableAction* act, const ProgramParser* parser, const std::vector<std::string>& cmd);
	void parse_transform(ImmovableAction* act, const ProgramParser* parser, const std::vector<std::string>& cmd);
	void parse_remove(ImmovableAction* act, const ProgramParser* parser, const std::vector<std::string>& cmd);

private:
	struct ParseMap {
		const char*	name;
		parse_t		function;
	};

private:
	std::string							m_name;
	std::vector<ImmovableAction>	m_actions;

private:
	static const ParseMap			s_parsemap[];
};

// Command name -> parser function mapping
const ImmovableProgram::ParseMap ImmovableProgram::s_parsemap[] = {
	{ "animation",		&ImmovableProgram::parse_animation },
	{ "transform",		&ImmovableProgram::parse_transform },
	{ "remove",			&ImmovableProgram::parse_remove },

	{ 0, 0 }
};


ImmovableProgram::ImmovableProgram(std::string name)
{
	m_name = name;
}


/*
===============
ImmovableProgram::add_action

Append the given action
===============
*/
void ImmovableProgram::add_action(const ImmovableAction& act)
{
	m_actions.push_back(act);
}


/*
===============
ImmovableProgram::parse

Actually parse a program
===============
*/
void ImmovableProgram::parse(Immovable_Descr* descr, std::string directory, Profile* prof)
{
	ProgramParser p;
	Section* s = prof->get_safe_section(m_name.c_str());
	uint line;

	p.descr = descr;
	p.directory = directory;
	p.prof = prof;

	for(line = 0; ; line++)
	{
		try
		{
			std::vector<std::string> command;
			ImmovableAction action;
			char buf[256];
			const char* string;
			uint mapidx;

			snprintf(buf, sizeof(buf), "%i", line);
			string = s->get_string(buf, 0);

			if (!string)
				break;

			split_string(string, &command, " \t\r\n");
			if (!command.size())
				continue;

			for(mapidx = 0; s_parsemap[mapidx].name; ++mapidx)
				if (command[0] == s_parsemap[mapidx].name)
					break;

			if (!s_parsemap[mapidx].name)
				throw wexception("Unknown instruction '%s'", command[0].c_str());

			(this->*s_parsemap[mapidx].function)(&action, &p, command);

			m_actions.push_back(action);
		}
		catch(std::exception& e)
		{
			log("WARNING: %s:%s:%i: %s\n", directory.c_str(), m_name.c_str(), line, e.what());
		}
	}

	// Fallback (default) program
	if (!m_actions.size())
	{
		ImmovableAction act;

		log("WARNING: %s: [%s] is empty, using default\n", directory.c_str(), m_name.c_str());

		act.function = &Immovable::run_animation;
		act.iparam1 = descr->parse_animation(directory, prof, "idle");
		act.iparam2 = -1;

		m_actions.push_back(act);
	}
}


/*
==============================================================================

Immovable_Descr IMPLEMENTATION

==============================================================================
*/

/*
===============
Immovable_Descr::Immovable_Descr

Initialize with sane defaults
===============
*/
Immovable_Descr::Immovable_Descr(const char *name)
{
	snprintf(m_name, sizeof(m_name), "%s", name);
	m_size = BaseImmovable::NONE;
   m_picture="";
	m_default_encodedata.clear();
}


/*
===============
Immovable_Descr::~Immovable_Descr

Cleanup
===============
*/
Immovable_Descr::~Immovable_Descr()
{
	while(m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/*
===============
Immovable_Descr::get_program

Find the program of the given name.
===============
*/
const ImmovableProgram* Immovable_Descr::get_program(std::string name) const
{
	ProgramMap::const_iterator it = m_programs.find(name);

	if (it == m_programs.end())
		throw wexception("Immovable %s has no program '%s'", get_name(), name.c_str());

	return it->second;
}


/*
===============
Immovable_Descr::parse

Parse an immovable from its conf file.

Section [global]:
picture (default = $NAME_00.bmp): name of picture used in editor
size = none|small|medium|big (default = none): influences build options
EncodeData (default for all animations)

Section [program] (optional)
step = animation [animation name] [duration]
       transform [immovable name]

Default:
0=animation idle -1
===============
*/
void Immovable_Descr::parse(const char *directory, Profile *prof)
{
	Section* global = prof->get_safe_section("global");
	const char* string;
	char buf[256];
	char picname[256];

	// Global options
	snprintf(buf, sizeof(buf), "%s_00.bmp", m_name);
	snprintf(picname, sizeof(picname), "%s/%s", directory, global->get_string("picture", buf));
   m_picture = picname;

	m_default_encodedata.parse(global);

	string = global->get_string("size", 0);
	if (string) {
		if (!strcasecmp(string, "volatile") || !strcasecmp(string, "none"))
		{
			m_size = BaseImmovable::NONE;
		}
		else if (!strcasecmp(string, "small"))
		{
			m_size = BaseImmovable::SMALL;
		}
		else if (!strcasecmp(string, "normal") || !strcasecmp(string, "medium"))
		{
			m_size = BaseImmovable::MEDIUM;
		}
		else if (!strcasecmp(string, "big"))
		{
			m_size = BaseImmovable::BIG;
		}
		else
			throw wexception("Unknown size '%s'. Possible values: none, small, medium, big", string);
	}


	// Parse attributes
	while(global->get_next_string("attrib", &string)) {
		add_attribute(get_attribute_id(string));
	}


	// Parse the programs
	while(global->get_next_string("program", &string))
		parse_program(directory, prof, string);

	if (m_programs.find("program") == m_programs.end()) {
		Section* program = prof->get_section("program");

		if (program)
		{
			log("WARNING: %s: obsolete implicit [program] section; use program=program in [global]\n",
						directory);
			parse_program(directory, prof, "program");
		}
		else
		{
			ImmovableProgram* prog = new ImmovableProgram("program");
			ImmovableAction act;

			act.function = &Immovable::run_animation;
			act.iparam1 = parse_animation(directory, prof, "idle");
			act.iparam2 = -1;
			prog->add_action(act);

			m_programs["program"] = prog;
		}
	}
}


/*
===============
Immovable_Descr::parse_program

Parse a program.
===============
*/
void Immovable_Descr::parse_program(std::string directory, Profile* prof, std::string name)
{
	ImmovableProgram* prog = 0;

	if (m_programs.find(name) != m_programs.end())
		throw wexception("Duplicate program '%s'", name.c_str());

	try
	{
		prog = new ImmovableProgram(name);
		prog->parse(this, directory, prof);
		m_programs[name] = prog;
	}
	catch(...)
	{
		if (prog)
			delete prog;

		throw;
	}
}


/*
===============
Immovable_Descr::parse_animation

Parse the animation of the given name.
===============
*/
uint Immovable_Descr::parse_animation(std::string directory, Profile* s, std::string name)
{
	AnimationMap::iterator it = m_animations.find(name);

	// Check if the animation has already been loaded
	if (it != m_animations.end())
		return it->second;

	// Load the animation
	Section* anim = s->get_section(name.c_str());
	char picname[256];
	uint animid;

	snprintf(picname, sizeof(picname), "%s_%s_??.bmp", m_name, name.c_str());

	// kind of obscure, this is still needed for backwards compatibility
	if (name == "idle" && !anim) {
		anim = s->get_section("global");

		snprintf(picname, sizeof(picname), "%s_??.bmp", m_name);
	}

	if (!anim) {
		log("%s: Animation %s not defined.\n", directory.c_str(), name.c_str());
		return 0;
	}

   animid = g_anim.get(directory.c_str(), anim, picname, &m_default_encodedata);

	m_animations[name] = animid;

	return animid;
}


/*
===============
Immovable_Descr::create

Create an immovable of this type
===============
*/
Immovable *Immovable_Descr::create(Editor_Game_Base *gg, Coords coords)
{
   Game* g=static_cast<Game*>(gg);
   Immovable *im = new Immovable(this);
	im->m_position = coords;
	im->init(g);
	return im;
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Immovable::Immovable
Immovable::~Immovable
===============
*/
Immovable::Immovable(Immovable_Descr *descr)
	: BaseImmovable(descr)
{
	m_anim = 0;
	m_program = 0;
	m_program_ptr = 0;
	m_program_step = 0;
}

Immovable::~Immovable()
{
}

/*
===============
Immovable::get_type
Immovable::get_size
Immovable::get_passable
===============
*/
int Immovable::get_type()
{
	return IMMOVABLE;
}

int Immovable::get_size()
{
	return get_descr()->get_size();
}

bool Immovable::get_passable()
{
	return get_descr()->get_size() < BIG;
}

/*
===============
Immovable::init

Actually initialize the immovable.
===============
*/
void Immovable::init(Editor_Game_Base *g)
{
	BaseImmovable::init(g);

	set_position(g, m_position);

	set_program_animation(g);

	if (g->is_game())
		switch_program((Game*)g, "program");
}


/*
===============
Immovable::cleanup

Cleanup before destruction
===============
*/
void Immovable::cleanup(Editor_Game_Base *g)
{
   unset_position(g, m_position);

	BaseImmovable::cleanup(g);
}


/*
===============
Immovable::set_program_animation

Set animation data according to current program state.
===============
*/
void Immovable::set_program_animation(Editor_Game_Base* g)
{
	const ImmovableProgram* prog = m_program;

	if (!prog)
		prog = get_descr()->get_program("program");

	const ImmovableAction& action = prog->get_action(m_program_ptr);

	if (action.function == &Immovable::run_animation) {
		m_anim = action.iparam1;
		m_animstart = g->get_gametime();
	}
}


/*
===============
Immovable::switch_program

Switch the currently running program.
===============
*/
void Immovable::switch_program(Game* g, std::string name)
{
	m_program = get_descr()->get_program(name);
	m_program_ptr = 0;

	run_program(g, false);
}


/*
===============
Immovable::act

Run program timer.
===============
*/
void Immovable::act(Game *g, uint data)
{
	BaseImmovable::act(g, data);

	if (g->get_gametime() - m_program_step >= 0)
		run_program(g, true); // This might delete itself!
}


/*
===============
Immovable::run_program

Execute the next step(s) in the program until we need to schedule_act().
If killable is true, the immovable could kill itself in this function.
===============
*/
void Immovable::run_program(Game* g, bool killable)
{
	uint origptr = m_program_ptr; // avoid infinite loops

	do
	{
		const ImmovableAction& action = m_program->get_action(m_program_ptr);

		if ((this->*action.function)(g, killable, action))
			return;
	}
	while(origptr != m_program_ptr);

	molog("WARNING: %s has infinite loop in program %s\n", get_descr()->get_name(),
					m_program->get_name().c_str());
}

/*
===============
Immovable::draw

Draw the immovable at the given position.
coords is the field that draw() was called for.
===============
*/
void Immovable::draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos)
{
	if (!m_anim)
		return;

	dst->drawanim(pos.x, pos.y, m_anim, game->get_gametime() - m_animstart, 0);
}


/*
==============================

Immovable commands

==============================
*/

/*
===============

animation <name> <duration>

===============
*/
void ImmovableProgram::parse_animation(ImmovableAction* act, const ProgramParser* parser,
																	const std::vector<std::string>& cmd)
{
	if (cmd.size() != 3)
		throw wexception("Syntax: animation [name] [duration]");

	act->function = &Immovable::run_animation;
	act->iparam1 = parser->descr->parse_animation(parser->directory, parser->prof, cmd[1]);
	act->iparam2 = atoi(cmd[2].c_str());

	if (act->iparam2 == 0 || act->iparam2 < -1)
		throw wexception("duration out of range (-1, 1..+inf) '%s'", cmd[2].c_str());
}

bool Immovable::run_animation(Game* g, bool killable, const ImmovableAction& action)
{
	m_anim = action.iparam1;
	m_animstart = g->get_gametime();

	if (action.iparam2 > 0)
		m_program_step = schedule_act(g, action.iparam2);

	m_program_ptr = (m_program_ptr+1) % m_program->get_size();

	return true;
}


/*
===============

transform <name of immovable>

===============
*/
void ImmovableProgram::parse_transform(ImmovableAction* act, const ProgramParser* parser,
															const std::vector<std::string>& cmd)
{
	if (cmd.size() != 2)
		throw wexception("Syntax: transform [bob name]");

	act->function = &Immovable::run_transform;
	act->sparam = cmd[1];
}

bool Immovable::run_transform(Game* g, bool killable, const ImmovableAction& action)
{
	Coords c = m_position;

	if (!killable) { // we need to reschedule and remove self from act()
		m_program_step = schedule_act(g, 1);
		return true;
	}

	remove(g);
	// Only use variables on the stack below this point!
	g->create_immovable(c, action.sparam);
	return true;
}


/*
===============

remove

===============
*/
void ImmovableProgram::parse_remove(ImmovableAction* act, const ProgramParser* parser,
																	const std::vector<std::string>& cmd)
{
	if (cmd.size() != 1)
		throw wexception("Syntax: remove");

	act->function = &Immovable::run_remove;
}

bool Immovable::run_remove(Game* g, bool killable, const ImmovableAction& action)
{
	if (!killable) {
		m_program_step = schedule_act(g, 1);
		return true;
	}

	remove(g);
	return true;
}


/*
==============================================================================

PlayerImmovable IMPLEMENTATION

==============================================================================
*/

/*
===============
PlayerImmovable::PlayerImmovable

Zero-initialize
===============
*/
PlayerImmovable::PlayerImmovable(Map_Object_Descr *descr)
	: BaseImmovable(descr)
{
	m_owner = 0;
	m_economy = 0;
}

/*
===============
PlayerImmovable::~PlayerImmovable

Cleanup
===============
*/
PlayerImmovable::~PlayerImmovable()
{
	if (m_workers.size())
		log("Building::~Building: %i workers left!\n", m_workers.size());
}

/*
===============
PlayerImmovable::set_economy

Change the economy, transfer the workers
===============
*/
void PlayerImmovable::set_economy(Economy *e)
{
	if (m_economy == e)
		return;

	for(uint i = 0; i < m_workers.size(); i++)
		m_workers[i]->set_economy(e);

	m_economy = e;
}

/*
===============
PlayerImmovable::add_worker

Associate the given worker with this immovable.
The worker will be transferred along to another economy, and it will be 
released when the immovable is destroyed.
This should only be called from Worker::set_location.
===============
*/
void PlayerImmovable::add_worker(Worker *w)
{
	m_workers.push_back(w);
}

/*
===============
PlayerImmovable::remove_worker

Disassociate the given worker with this building.
This should only be called from Worker::set_location.
===============
*/
void PlayerImmovable::remove_worker(Worker *w)
{
	for(uint i = 0; i < m_workers.size(); i++) {
		if (m_workers[i] == w) {
			if (i < m_workers.size()-1)
				m_workers[i] = m_workers[m_workers.size()-1];
			m_workers.pop_back();
			return;
		}
	}
	
	throw wexception("PlayerImmovable::remove_worker: not in list");
}


/*
===============
PlayerImmovable::set_owner

Set the immovable's owner. Currently, it can only be set once.
===============
*/
void PlayerImmovable::set_owner(Player *owner)
{
	// Change these asserts once you've made really sure that changing owners
	// works (necessary for military building)
	assert(!m_owner);
	assert(owner);
	
	m_owner = owner;
}

/*
===============
PlayerImmovable::init

Initialize the immovable.
===============
*/
void PlayerImmovable::init(Editor_Game_Base *g)
{
	BaseImmovable::init(g);
}

/*
===============
PlayerImmovable::cleanup

Release workers
===============
*/
void PlayerImmovable::cleanup(Editor_Game_Base *g)
{
   while(m_workers.size())
      m_workers[0]->set_location(0);

	BaseImmovable::cleanup(g);
}

