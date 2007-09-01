/*
 * Copyright (C) 2002-2003, 2006-2007 by the Widelands Development Team
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

#include "immovable.h"

#include "editor_game_base.h"
#include "error.h"
#include "field.h"
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "helper.h"
#include "immovable_program.h"
#include "player.h"
#include "map.h"
#include "profile.h"
#include "rendertarget.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"

#include <stdio.h>


/*
===============
BaseImmovable::BaseImmovable
BaseImmovable::~BaseImmovable

Base immovable creation and destruction
===============
*/
BaseImmovable::BaseImmovable(const Map_Object_Descr & mo_descr) :
Map_Object(&mo_descr)
{}

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
   if (f->immovable && f->immovable!=this) {
      BaseImmovable *other = f->immovable;

	   assert(other->get_size() == NONE);

      other->cleanup(g);
      delete other;
	}

	f->immovable = this;

	if (get_size() >= SMALL) {
		Map & map = g->map();
		map.recalc_for_field_area(Area<FCoords>(map.get_fcoords(c), 2));
	}
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
	g->inform_players_about_immovable
		(Map::get_index(c, g->map().get_width()), 0);

	if (get_size() >= SMALL) {
		Map & map = g->map();
		map.recalc_for_field_area(Area<FCoords>(map.get_fcoords(c), 2));
	}
}


/*
==============================================================================

ImmovableProgram IMPLEMENTATION

==============================================================================
*/

// Command name -> parser function mapping
const ImmovableProgram::ParseMap ImmovableProgram::s_parsemap[] = {
	{"animation", &ImmovableProgram::parse_animation},
	{"transform", &ImmovableProgram::parse_transform},
	{"remove",    &ImmovableProgram::parse_remove},
	{"playFX",    &ImmovableProgram::parse_playFX},

	{0, 0}
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
	uint line=0;

	p.descr = descr;
	p.directory = directory;
	p.prof = prof;

	for (line = 0; ; line++)
	{
		try
		{
			std::vector<std::string> command;
			ImmovableAction action;
			char buffer[256];
			const char* string;
			uint mapidx;

			snprintf(buffer, sizeof(buffer), "%i", line);
			string = s->get_string(buffer, 0);

			if (!string)
				break;

			split_string(string, command, " \t\r\n");
			if (!command.size())
				continue;

			for (mapidx = 0; s_parsemap[mapidx].name; ++mapidx)
				if (command[0] == s_parsemap[mapidx].name)
					break;

			if (!s_parsemap[mapidx].name)
				throw wexception("Unknown instruction '%s'", command[0].c_str());

			(this->*s_parsemap[mapidx].function)(&action, &p, command);

			m_actions.push_back(action);
		}
		catch (std::exception& e)
		{
			log("WARNING: %s:%s:%i: %s\n", directory.c_str(), m_name.c_str(), line, e.what());
		}
	}

	if (s->get_num_values() != m_actions.size())
		log("WARNING: %s:%s: program line numbers appear to be wrong\n",
			directory.c_str(), m_name.c_str());

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
Immovable_Descr::Immovable_Descr
(const Tribe_Descr * const owner_tribe, const std::string & immovable_name)
:
m_name(immovable_name), m_size(BaseImmovable::NONE), m_owner_tribe(owner_tribe)
{m_default_encodedata.clear();}


/*
===============
Immovable_Descr::~Immovable_Descr

Cleanup
===============
*/
Immovable_Descr::~Immovable_Descr()
{
	while (m_programs.size()) {
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
const ImmovableProgram* Immovable_Descr::get_program(std::string programname)
		const
{
	ProgramMap::const_iterator it = m_programs.find(programname);

	if (it == m_programs.end())
		throw wexception("Immovable %s has no program '%s'", name().c_str(),
							  programname.c_str());

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
	char buffer [256];
	char picname[256];

	// Global options
	snprintf(buffer, sizeof(buffer), "%s_00.png", m_name.c_str());
	snprintf
		(picname, sizeof(picname),
		 "%s/%s", directory, global->get_string("picture", buffer));
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
	while (global->get_next_string("attrib", &string)) {
		uint attrib = get_attribute_id(string);

		if (attrib < Map_Object::HIGHEST_FIXED_ATTRIBUTE)
		{
			if (attrib != Map_Object::RESI)
				throw wexception("Bad attribute '%s'", string);
		}

		add_attribute(attrib);
	}


	// Parse the programs
	while (global->get_next_string("program", &string))
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
void Immovable_Descr::parse_program(std::string directory, Profile* prof,
												std::string programname)
{
	ImmovableProgram* prog = 0;

	if (m_programs.find(programname) != m_programs.end())
		throw wexception("Duplicate program '%s'", programname.c_str());

	try
	{
		prog = new ImmovableProgram(programname);
		prog->parse(this, directory, prof);
		m_programs[programname] = prog;
	}
	catch (...)
	{
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
uint Immovable_Descr::parse_animation
(std::string directory, Profile* s, std::string animation_name)
{
	// Load the animation
	Section * anim = s->get_section(animation_name.c_str());
	char picname[256];
	uint animid=0;

	snprintf
		(picname, sizeof(picname),
		 "%s_%s_??.png",
		 m_name.c_str(),
		 animation_name.c_str());

	// kind of obscure, this is still needed for backwards compatibility
	if (animation_name == "idle" and not anim) {
		anim = s->get_section("global");

		snprintf(picname, sizeof(picname), "%s_??.png", m_name.c_str());
	}

	if (!anim) {
		log
			("%s: Animation %s not defined.\n",
			 directory.c_str(),
			 animation_name.c_str());
		return 0;
	}

	if (not is_animation_known(animation_name.c_str())) {
      animid = g_anim.get(directory.c_str(), anim, picname, &m_default_encodedata);
		add_animation(animation_name.c_str(), animid);
	} else animid = get_animation(animation_name.c_str());

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
	Immovable * im = new Immovable(*this);
	im->m_position = coords;
	im->init(gg);
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
Immovable::Immovable(const Immovable_Descr & imm_descr) :
BaseImmovable (imm_descr),
m_anim        (0),
m_program     (0),
m_program_ptr (0),
m_program_step(0)
{}

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
int Immovable::get_type() const throw () {return IMMOVABLE;}

int Immovable::get_size() const throw () {return descr().get_size();}

bool Immovable::get_passable() const throw ()
{return descr().get_size() < BIG;}

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

	if (Game * const game = dynamic_cast<Game *>(g))
		switch_program(game, "program");
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
		prog = descr().get_program("program");

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
void Immovable::switch_program(Game* g, std::string programname)
{
	m_program = descr().get_program(programname);
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
	while (origptr != m_program_ptr);

	molog("WARNING: %s has infinite loop in program %s\n", descr().name().c_str(),
					m_program->get_name().c_str());
}

/*
===============
Immovable::draw

Draw the immovable at the given position.
coords is the field that draw() was called for.
===============
*/
void Immovable::draw
(const Editor_Game_Base & game,
 RenderTarget & dst,
 const FCoords,
 const Point pos)
{if (m_anim) dst.drawanim(pos, m_anim, game.get_gametime() - m_animstart, 0);}


/*
==============================

Load/save support

==============================
*/

#define IMMOVABLE_SAVEGAME_VERSION 1

void Immovable::Loader::load(FileRead& fr)
{
	BaseImmovable::Loader::load(fr);

	Immovable* imm = dynamic_cast<Immovable*>(get_object());

	// Position
	imm->m_position = fr.Coords32(egbase().map().extent());
	imm->set_position(&egbase(), imm->m_position);

	// Animation
	const char* animname = fr.CString();
	try {
		imm->m_anim = imm->descr().get_animation(animname);
	}
	catch (Map_Object_Descr::Animation_Nonexistent&) {
		imm->m_anim = imm->descr().main_animation();
		log("Warning: Animation '%s' not found, using animation '%s').\n",
			animname, imm->descr().get_animation_name(imm->m_anim).c_str());
	}
	imm->m_animstart = fr.Signed32();

	// Programm
	if (fr.Unsigned8())
		imm->m_program = imm->descr().get_program(fr.CString());
	else
		imm->m_program = 0;
	imm->m_program_ptr = fr.Unsigned32();

	if (!imm->m_program) {
		imm->m_program_ptr = 0;
	} else {
		if (imm->m_program_ptr >= imm->m_program->get_size()) {
			// Try to not fail if the program of some immovable has changed
			// significantly.
			// Note that in some cases, the immovable may end up broken despite
			// the fixup, but there isn't really anything we can do against that.
			log("Warning: Immovable '%s', size of program '%s' seems to have changed.\n",
				imm->descr().name().c_str(), imm->m_program->get_name().c_str());
			imm->m_program_ptr = 0;
		}
	}

	imm->m_program_step = fr.Signed32();
}

void Immovable::Loader::load_pointers()
{
	BaseImmovable::Loader::load_pointers();
}

void Immovable::Loader::load_finish()
{
	BaseImmovable::Loader::load_finish();

	Immovable* imm = dynamic_cast<Immovable*>(get_object());

	egbase().inform_players_about_immovable
			(Map::get_index(imm->m_position, egbase().map().get_width()),
			 &imm->descr());
}

void Immovable::save(Editor_Game_Base* eg, Widelands_Map_Map_Object_Saver* mos, FileWrite& fw)
{
	// This is in front because it is required to obtain the descriptiong
	// necessary to create the Immovable
	fw.Unsigned8(header_Immovable);
	fw.Unsigned8(IMMOVABLE_SAVEGAME_VERSION);

	if (const Tribe_Descr * const tribe = get_owner_tribe())
		fw.CString(tribe->name().c_str());
	else
		fw.CString("world");

	fw.CString(name().c_str());

	// The main loading data follows
	BaseImmovable::save(eg, mos, fw);

	fw.Coords32(m_position);

	// Animations
	fw.CString(descr().get_animation_name(m_anim).c_str());
	fw.Signed32(m_animstart);

	// Program Stuff
	if (m_program) {
		fw.Unsigned8(1);
		fw.CString(m_program->get_name().c_str());
	} else {
		fw.Unsigned8(0);
	}

	fw.Unsigned32(m_program_ptr);
	fw.Signed32(m_program_step);
}

Map_Object::Loader* Immovable::load
		(Editor_Game_Base* eg, Widelands_Map_Map_Object_Loader* mol, FileRead& fr)
{
	Loader* loader = new Loader;

	try {
		// The header has been peeled away by the caller

		Uint8 version = fr.Unsigned8();
		if (version != IMMOVABLE_SAVEGAME_VERSION)
			throw wexception("Unknown version %u", version);

		const char * const owner = fr.CString ();
		const char * const name  = fr.CString ();
		Immovable* imm = 0;

		if (strcmp(owner, "world")) {
			// It is a tribe immovable
			eg->manually_load_tribe(owner);

			Tribe_Descr* tribe = eg->get_tribe(owner);
			if (!tribe)
				throw wexception("Unknown tribe %s!\n", owner);

			int idx = tribe->get_immovable_index(name);
			if (idx == -1)
				throw wexception
						("Unknown tribe-immovable %s in map, asked for tribe: %s!\n",
						 name, owner);

			imm = new Immovable(*tribe->get_immovable_descr(idx));
		} else {
			// World immovable
			int idx = eg->map().world().get_immovable_index(name);
			if (idx == -1)
				throw wexception("Unknown world immovable %s in map!\n", name);

			imm = new Immovable(*eg->map().world().get_immovable_descr(idx));
		}

		loader->init(eg, mol, imm);
		loader->load(fr);
	} catch (const std::exception & e) {
		delete loader;
		throw wexception("Loading Immovable: %s", e.what());
	} catch (...) {
		delete loader;
		throw;
	}

	return loader;
}


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

bool Immovable::run_animation(Game* g, bool, const ImmovableAction & action)
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

playFX <name>

===============
*/

void ImmovableProgram::parse_playFX
(ImmovableAction* act,
 const ProgramParser *,
 const std::vector<std::string> & cmd)
{
	if (cmd.size()<2 || cmd.size()>3)
		throw wexception("Syntax: playFX <fxname> [priority]");

	act->function = &Immovable::run_playFX;
	act->sparam1 = cmd[1];
	if (cmd.size()==2)
		act->iparam1=127;
	else
		act->iparam1=atoi(cmd[2].c_str());
}

/** Demand from the \ref g_sound_handler to play a certain sound effect. Whether the effect actually gets played
 * is decided only by the sound server*/
bool Immovable::run_playFX(Game *, bool, const ImmovableAction & action)
{
	g_sound_handler.play_fx(action.sparam1, get_position(), action.iparam1);

	m_program_ptr = (m_program_ptr+1) % m_program->get_size();

	return true;
}


/*
===============

transform <name of immovable>

===============
*/
void ImmovableProgram::parse_transform
(ImmovableAction* act,
 const ProgramParser *,
 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 2)
		throw wexception("Syntax: transform [bob name]");

   std::vector<std::string> list;

	split_string(cmd[1], list, ":");
   if (list.size()==1) {
      act->sparam1 = cmd[1];
      act->sparam2 = "world";
	} else {
      act->sparam1 = list[1];
      act->sparam2 = list[0];
	}

	act->function = &Immovable::run_transform;
}

bool Immovable::run_transform(Game* g, bool killable, const ImmovableAction& action)
{
	Coords c = m_position;

   if (!descr().get_owner_tribe() && (action.sparam2 != "world"))
      throw wexception("Should create tribe-immovable %s, but we are no tribe immovable!\n", action.sparam1.c_str());

	if (!killable) { // we need to reschedule and remove self from act()
		m_program_step = schedule_act(g, 1);
		return true;
	}


	const Tribe_Descr* tribe=0;

   if (action.sparam2 != "world")
      tribe=descr().get_owner_tribe(); // Not a world bob?

	remove(g);
	// Only use variables on the stack below this point!
	g->create_immovable(c, action.sparam1, tribe);
	return true;
}


/*
===============

remove

===============
*/
void ImmovableProgram::parse_remove
(ImmovableAction * act,
 const ProgramParser *,
 const std::vector<std::string> & cmd)
{
	if (cmd.size() != 1)
		throw wexception("Syntax: remove");

	act->function = &Immovable::run_remove;
}

bool Immovable::run_remove(Game* g, bool killable, const ImmovableAction &) {
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
PlayerImmovable::PlayerImmovable(const Map_Object_Descr & mo_descr) :
BaseImmovable(mo_descr), m_owner(0), m_economy(0)
{}

/*
===============
PlayerImmovable::~PlayerImmovable

Cleanup
===============
*/
PlayerImmovable::~PlayerImmovable()
{
	if (m_workers.size())
		log("Building::~Building: %u workers left!\n", m_workers.size());
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

	for (uint i = 0; i < m_workers.size(); i++)
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
	for (uint i = 0; i < m_workers.size(); i++) {
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
void PlayerImmovable::set_owner(Player * const new_owner) {
	// Change these asserts once you've made really sure that changing owners
	// works (necessary for military building)
	/*THIS IS A TEST
   assert(!m_owner);
	assert(new_owner);*/

	m_owner = new_owner;

	m_owner->egbase().player_immovable_notification
		(this, Editor_Game_Base::GAIN);
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
	while (m_workers.size())
		m_workers[0]->set_location(0);

	if (m_owner)
		m_owner->egbase().player_immovable_notification
			(this, Editor_Game_Base::LOSE);

	BaseImmovable::cleanup(g);
}

/*
 * Dump general information
 */
void PlayerImmovable::log_general_info(Editor_Game_Base* egbase)  {
   BaseImmovable::log_general_info(egbase);

   molog("this: %p\n", this);
   molog("m_owner: %p\n", m_owner);
   molog("* player nr: %i\n", m_owner->get_player_number());
   molog("m_economy: %p\n", m_economy);
}
