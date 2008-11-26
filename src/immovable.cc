/*
 * Copyright (C) 2002-2003, 2006-2008 by the Widelands Development Team
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
#include "field.h"
#include "game.h"
#include "helper.h"
#include "immovable_program.h"
#include "player.h"
#include "map.h"
#include "mapfringeregion.h"
#include "profile.h"
#include "rendertarget.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "worker.h"

#include "upcast.h"

#include <stdio.h>

namespace Widelands {

BaseImmovable::BaseImmovable(const Map_Object_Descr & mo_descr) :
Map_Object(&mo_descr)
{}

BaseImmovable::~BaseImmovable() {}

static std::string const base_immovable_name = "unknown";
std::string const & BaseImmovable::name() const throw () {
	return base_immovable_name;
}

/**
 * Associate the given field with this immovable. Recalculate if necessary.
 *
 * Only call this during init.
*/
void BaseImmovable::set_position(Editor_Game_Base * egbase, Coords const c)
{
	assert(c);

	Map & map = egbase->map();
	FCoords f = map.get_fcoords(c);
	if (f.field->immovable && f.field->immovable != this) {
		assert(f.field->immovable->get_size() == NONE);

		f.field->immovable->cleanup(egbase);
		delete f.field->immovable;
	}

	f.field->immovable = this;

	if (get_size() >= SMALL)
		map.recalc_for_field_area(Area<FCoords>(f, 2));
}

/**
 * Remove the link to the given field.
 *
 * Only call this during cleanup.
*/
void BaseImmovable::unset_position(Editor_Game_Base * egbase, Coords const c)
{
	Map & map = egbase->map();
	FCoords f = map.get_fcoords(c);

	assert(f.field->immovable == this);

	f.field->immovable = 0;
	egbase->inform_players_about_immovable(f.field - &map[0], 0);

	if (get_size() >= SMALL)
		map.recalc_for_field_area(Area<FCoords>(f, 2));
}


/*
==============================================================================

ImmovableProgram IMPLEMENTATION

==============================================================================
*/

/// Command name -> parser function mapping
const ImmovableProgram::ParseMap ImmovableProgram::s_parsemap[] = {
	{"animation", &ImmovableProgram::parse_animation},
	{"transform", &ImmovableProgram::parse_transform},
	{"remove",    &ImmovableProgram::parse_remove},
	{"playFX",    &ImmovableProgram::parse_playFX},
	{"seed",      &ImmovableProgram::parse_seed},
	{0, 0}
};


ImmovableProgram::ImmovableProgram(std::string name)
{
	m_name = name;
}


/**
 * Append the given action
*/
void ImmovableProgram::add_action(const ImmovableAction& act)
{
	m_actions.push_back(act);
}


/**
 * Actually parse a program
*/
void ImmovableProgram::parse
	(Immovable_Descr & descr, std::string const & directory, Profile & prof)
{
	ProgramParser p;
	Section & s = prof.get_safe_section(m_name.c_str());
	uint32_t line=0;

	p.descr = &descr;
	p.directory = directory;
	p.prof = &prof;

	for (line = 0;; ++line) {
		try
		{
			ImmovableAction action;
			char buffer[256];
			const char* string;
			uint32_t mapidx;

			snprintf(buffer, sizeof(buffer), "%i", line);
			string = s.get_string(buffer, 0);

			if (!string)
				break;

			const std::vector<std::string> command
				(split_string(string, " \t\r\n"));
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

	if (s.get_num_values() != m_actions.size())
		log
			("WARNING: %s:%s: program line numbers appear to be wrong\n",
			 directory.c_str(), m_name.c_str());

	// Fallback (default) program
	if (!m_actions.size())
	{
		ImmovableAction act;

		log("WARNING: %s: [%s] is empty, using default\n", directory.c_str(), m_name.c_str());

		act.function = &Immovable::run_animation;
		act.iparam1 = descr.parse_animation(directory, prof, "idle");
		act.iparam2 = -1;

		m_actions.push_back(act);
	}
}


/*
==============================================================================

Immovable_Descr IMPLEMENTATION

==============================================================================
*/

/**
 * Parse an immovable from its conf file.
 *
 * Section [global]:
 * picture (default = $NAME_00.bmp): name of picture used in editor
 * size = none|small|medium|big (default = none): influences build options
 * EncodeData (default for all animations)
 *
 * Section [program] (optional)
 * step = animation [animation name] [duration]
 *        transform [immovable name]
 *
 * Default:
 * 0=animation idle -1
*/
Immovable_Descr::Immovable_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const * const owner_tribe)
:
	Map_Object_Descr(_name, _descname),
	m_picture       (directory + global_s.get_string("picture", "menu.png")),
	m_size          (BaseImmovable::NONE),
	m_owner_tribe   (owner_tribe)
{
	m_default_encodedata.clear();
	m_default_encodedata.parse(global_s);

	if (char const * const string = global_s.get_string("size")) {
		if      (!strcasecmp(string, "small"))
			m_size = BaseImmovable::SMALL;
		else if (!strcasecmp(string, "medium"))
			m_size = BaseImmovable::MEDIUM;
		else if (!strcasecmp(string, "big"))
			m_size = BaseImmovable::BIG;
		else
			throw wexception("size=\"%s\": expected {small|medium|big}", string);
	}


	//  parse attributes
	while (Section::Value const * const v = global_s.get_next_val("attrib")) {
		uint32_t attrib = get_attribute_id(v->get_string());
		if (attrib < Map_Object::HIGHEST_FIXED_ATTRIBUTE)
			if (attrib != Map_Object::RESI)
				throw wexception("Bad attribute '%s'", v->get_string());
		add_attribute(attrib);
	}


	//  parse the programs
	while (Section::Value const * const v = global_s.get_next_val("program"))
		parse_program(directory, prof, v->get_string());

	if (m_programs.find("program") == m_programs.end()) {
		if (prof.get_section("program")) {
			log
				("WARNING: %s: obsolete implicit [program] section; use "
				 "program=program in [global]\n",
				 directory.c_str());
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


/**
 * Cleanup
*/
Immovable_Descr::~Immovable_Descr()
{
	while (m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/**
 * Find the program of the given name.
*/
const ImmovableProgram* Immovable_Descr::get_program
	(std::string programname) const
{
	Programs::const_iterator it = m_programs.find(programname);

	if (it == m_programs.end())
		throw wexception
			("Immovable %s has no program '%s'",
			 name().c_str(), programname.c_str());

	return it->second;
}


/**
 * Parse a program.
*/
void Immovable_Descr::parse_program
	(std::string const & directory,
	 Profile           & prof,
	 std::string const & programname)
{
	ImmovableProgram* prog = 0;

	if (m_programs.find(programname) != m_programs.end())
		throw wexception("Duplicate program '%s'", programname.c_str());

	try
	{
		prog = new ImmovableProgram(programname);
		prog->parse(*this, directory, prof);
		m_programs[programname] = prog;
	}
	catch (...)
	{
		delete prog;

		throw;
	}
}


/**
 * Parse the animation of the given name.
*/
uint32_t Immovable_Descr::parse_animation
	(std::string const & directory,
	 Profile           & prof,
	 std::string const & animation_name)
{
	// Load the animation
	Section & anim = prof.get_safe_section(animation_name.c_str());
	char picname[256];
	uint32_t animid=0;

	snprintf(picname, sizeof(picname), "%s.png", animation_name.c_str());

	if (not is_animation_known(animation_name.c_str())) {
		animid =
			g_anim.get(directory.c_str(), anim, picname, &m_default_encodedata);
		add_animation(animation_name.c_str(), animid);
	} else
		animid = get_animation(animation_name.c_str());

	return animid;
}


/**
 * Create an immovable of this type
*/
Immovable & Immovable_Descr::create
	(Editor_Game_Base & egbase, Coords const coords) const
{
	Immovable & result = *new Immovable(*this);
	result.m_position = coords;
	result.init(&egbase);
	return result;
}


/*
==============================

IMPLEMENTATION

==============================
*/

Immovable::Immovable(const Immovable_Descr & imm_descr) :
BaseImmovable (imm_descr),
m_anim        (0),
m_program     (0),
m_program_ptr (0),
m_program_step(0)
{}

Immovable::~Immovable() {}

int32_t Immovable::get_type() const throw ()
{
	return IMMOVABLE;
}

int32_t Immovable::get_size() const throw ()
{
	return descr().get_size();
}

bool Immovable::get_passable() const throw ()
{
	return descr().get_size() < BIG;
}


std::string const & Immovable::name() const throw () {return descr().name();}


/**
 * Actually initialize the immovable.
*/
void Immovable::init(Editor_Game_Base *g)
{
	BaseImmovable::init(g);

	set_position(g, m_position);

	set_program_animation(g);

	if (upcast(Game, game, g))
		switch_program(game, "program");
}


/**
 * Cleanup before destruction
*/
void Immovable::cleanup(Editor_Game_Base *g)
{
	unset_position(g, m_position);

	BaseImmovable::cleanup(g);
}


/**
 * Set animation data according to current program state.
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


/**
 * Switch the currently running program.
*/
void Immovable::switch_program(Game* g, std::string programname)
{
	m_program = descr().get_program(programname);
	m_program_ptr = 0;

	run_program(g, false);
}


/**
 * Run program timer.
*/
void Immovable::act(Game *g, uint32_t data)
{
	BaseImmovable::act(g, data);

	if (g->get_gametime() - m_program_step >= 0)
		run_program(g, true); // This might delete itself!
}


/**
 * Execute the next step(s) in the program until we need to schedule_act().
 * If killable is true, the immovable could kill itself in this function.
*/
void Immovable::run_program(Game* g, bool killable)
{
	uint32_t origptr = m_program_ptr; // avoid infinite loops

	do
	{
		const ImmovableAction& action = m_program->get_action(m_program_ptr);

		if ((this->*action.function)(g, killable, action))
			return;
	}
	while (origptr != m_program_ptr);

	molog
		("WARNING: %s has infinite loop in program %s\n",
		 descr().name().c_str(), m_program->get_name().c_str());
}

/**
 * Draw the immovable at the given position.
 * coords is the field that draw() was called for.
*/
void Immovable::draw
	(Editor_Game_Base const &       game,
	 RenderTarget           &       dst,
	 FCoords,
	 Point                    const pos)
{
	if (m_anim)
		dst.drawanim(pos, m_anim, game.get_gametime() - m_animstart, 0);
}


/*
==============================

Load/save support

==============================
*/

#define IMMOVABLE_SAVEGAME_VERSION 1

void Immovable::Loader::load(FileRead& fr)
{
	BaseImmovable::Loader::load(fr);

	Immovable & imm = dynamic_cast<Immovable &>(*get_object());

	// Position
	imm.m_position = fr.Coords32(egbase().map().extent());
	imm.set_position(&egbase(), imm.m_position);

	// Animation
	const char* animname = fr.CString();
	try {
		imm.m_anim = imm.descr().get_animation(animname);
	}
	catch (Map_Object_Descr::Animation_Nonexistent&) {
		imm.m_anim = imm.descr().main_animation();
		log
			("Warning: Animation \"%s\" not found, using animation %s).\n",
			 animname, imm.descr().get_animation_name(imm.m_anim).c_str());
	}
	imm.m_animstart = fr.Signed32();

	//  program
	imm.m_program =
		imm.descr().get_program(fr.Unsigned8() ? fr.CString() : "program");
	imm.m_program_ptr = fr.Unsigned32();

	if (!imm.m_program) {
		imm.m_program_ptr = 0;
	} else {
		if (imm.m_program_ptr >= imm.m_program->get_size()) {
			// Try to not fail if the program of some immovable has changed
			// significantly.
			// Note that in some cases, the immovable may end up broken despite
			// the fixup, but there isn't really anything we can do against that.
			log
				("Warning: Immovable '%s', size of program '%s' seems to have "
				 "changed.\n",
				 imm.descr().name().c_str(), imm.m_program->get_name().c_str());
			imm.m_program_ptr = 0;
		}
	}

	imm.m_program_step = fr.Signed32();
}

void Immovable::Loader::load_pointers()
{
	BaseImmovable::Loader::load_pointers();
}

void Immovable::Loader::load_finish()
{
	BaseImmovable::Loader::load_finish();

	upcast(Immovable, imm, get_object());
	if (upcast(Game, game, &egbase()))
		imm->schedule_act(game, 1);

	egbase().inform_players_about_immovable
			(Map::get_index(imm->m_position, egbase().map().get_width()),
			 &imm->descr());
}

void Immovable::save
	(Editor_Game_Base * egbase, Map_Map_Object_Saver * mos, FileWrite & fw)
{
	// This is in front because it is required to obtain the descriptiong
	// necessary to create the Immovable
	fw.Unsigned8(header_Immovable);
	fw.Unsigned8(IMMOVABLE_SAVEGAME_VERSION);

	if (const Tribe_Descr * const tribe = get_owner_tribe())
		fw.String(tribe->name());
	else
		fw.CString("world");

	fw.String(name());

	// The main loading data follows
	BaseImmovable::save(egbase, mos, fw);

	fw.Coords32(m_position);

	// Animations
	fw.String(descr().get_animation_name(m_anim));
	fw.Signed32(m_animstart);

	// Program Stuff
	if (m_program) {
		fw.Unsigned8(1);
		fw.String(m_program->get_name());
	} else {
		fw.Unsigned8(0);
	}

	fw.Unsigned32(m_program_ptr);
	fw.Signed32(m_program_step);
}

Map_Object::Loader* Immovable::load
	(Editor_Game_Base * egbase, Map_Map_Object_Loader * mol, FileRead & fr)
{
	Loader* loader = new Loader;

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (version != IMMOVABLE_SAVEGAME_VERSION)
			throw wexception("unknown/unhandled version %u", version);

		const char * const owner = fr.CString ();
		const char * const name  = fr.CString ();
		Immovable* imm = 0;

		if (strcmp(owner, "world")) {
			// It is a tribe immovable
			egbase->manually_load_tribe(owner);

			if (const Tribe_Descr * const tribe = egbase->get_tribe(owner)) {
				const int32_t idx = tribe->get_immovable_index(name);
				if (idx != -1)
					imm = new Immovable(*tribe->get_immovable_descr(idx));
				else
					throw wexception
						("Unknown tribe-immovable %s in map, asked for tribe: %s!",
						 name, owner);
			} else
				throw wexception("Unknown tribe %s!", owner);
		} else {
			// World immovable
			int32_t const idx = egbase->map().world().get_immovable_index(name);
			if (idx == -1)
				throw wexception("Unknown world immovable %s in map!", name);

			imm = new Immovable(*egbase->map().world().get_immovable_descr(idx));
		}

		loader->init(egbase, mol, imm);
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


/**
 * animation \<name\> \<duration\>
*/
void ImmovableProgram::parse_animation
	(ImmovableAction                * act,
	 ProgramParser            const * parser,
	 std::vector<std::string> const & cmd)
{
	if (cmd.size() != 3)
		throw wexception("Syntax: animation <name> <duration>");

	act->function = &Immovable::run_animation;

	act->iparam1 =
		parser->descr->parse_animation(parser->directory, *parser->prof, cmd[1]);
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


void ImmovableProgram::parse_playFX
	(ImmovableAction                * act,
	 ProgramParser            const *,
	 std::vector<std::string> const & cmd)
{
	if (cmd.size()<2 || cmd.size()>3)
		throw wexception("Syntax: playFX <fxname> [priority]");

	act->function = &Immovable::run_playFX;

	act->sparam1 = cmd[1];

	act->iparam1 = cmd.size() < 3 ? 127 : atoi(cmd[2].c_str());
}

/** Demand from the g_sound_handler to play a certain sound effect. Whether the effect actually gets played
 * is decided only by the sound server*/
bool Immovable::run_playFX(Game * game, bool, const ImmovableAction & action)
{
	g_sound_handler.play_fx(action.sparam1, get_position(), action.iparam1);

	schedule_act(game, 1);
	m_program_ptr = (m_program_ptr + 1) % m_program->get_size();

	return true;
}


void ImmovableProgram::parse_transform
	(ImmovableAction                * act,
	 ProgramParser            const *,
	 std::vector<std::string> const & cmd)
{
	if (cmd.size() < 2 or 3 < cmd.size())
		throw wexception("Syntax: transform <immovable type> [probability]");

	act->function = &Immovable::run_transform;

	const std::vector<std::string> list(split_string(cmd[1], ":"));
	if (list.size() == 1) {
		act->sparam1 = cmd[1];
		act->sparam2 = "world";
	} else {
		act->sparam1 = list[1];
		act->sparam2 = list[0];
	}

	if (cmd.size() < 3)
		act->iparam1 = 0;
	else {
		act->iparam1 = atoi(cmd[2].c_str());
		if (act->iparam1 < 1 or 255 < act->iparam1)
			throw wexception
				("probability out of range (1, 255) \"%s\"", cmd[2].c_str());
	}
}

bool Immovable::run_transform(Game* g, bool killable, const ImmovableAction& action)
{
	if
		(action.iparam1 == 0
		 or
		 g->logic_rand() % 256 < static_cast<uint32_t>(action.iparam1))
	{
		Coords c = m_position;

		if (!descr().get_owner_tribe() && (action.sparam2 != "world"))
			throw wexception
				("Should create tribe-immovable %s, but we are no tribe immovable!"
				 "\n",
				 action.sparam1.c_str());

		if (!killable) { //  we need to reschedule and remove self from act()
			m_program_step = schedule_act(g, 1);
			return true;
		}


		const Tribe_Descr* tribe=0;

		if (action.sparam2 != "world")
			tribe = descr().get_owner_tribe(); // Not a world bob?

		remove(g);
		//  Only use variables on the stack below this point!
		g->create_immovable(c, action.sparam1, tribe);
	} else {
		schedule_act(g, 1);
		m_program_ptr = (m_program_ptr + 1) % m_program->get_size();
	}
	return true;
}


/**
 * remove
*/
void ImmovableProgram::parse_remove
	(ImmovableAction                * act,
	 ProgramParser            const *,
	 std::vector<std::string> const & cmd)
{
	if (cmd.size() < 1 or 2 < cmd.size())
		throw wexception("Syntax: remove [probability]");

	act->function = &Immovable::run_remove;

	if (cmd.size() < 2)
		act->iparam1 = 0;
	else {
		act->iparam1 = atoi(cmd[1].c_str());
		if (act->iparam1 < 1 or 255 < act->iparam1)
			throw wexception
				("probability out of range (1, 255) \"%s\"", cmd[1].c_str());
	}
}

bool Immovable::run_remove
	(Game * g, bool const killable, ImmovableAction const & action)
{
	if
		(action.iparam1 == 0
		 or
		 g->logic_rand() % 256 < static_cast<uint32_t>(action.iparam1))
	{
		if (!killable) {
			m_program_step = schedule_act(g, 1);
			return true;
		}

		remove(g);
	} else {
		schedule_act(g, 1);
		m_program_ptr = (m_program_ptr + 1) % m_program->get_size();
	}
	return true;
}

void ImmovableProgram::parse_seed
	(ImmovableAction                * act,
	 ProgramParser            const *,
	 std::vector<std::string> const & cmd)
{
	if (cmd.size() != 3)
		throw wexception
			("Syntax: seed <immovable type> <probability of spreading one step "
			 "away (1 .. 255)>");

	act->function = &Immovable::run_seed;

	std::vector<std::string> const list(split_string(cmd[1], ":"));
	if (list.size() == 1) {
		act->sparam1 = cmd[1];
		act->sparam2 = "world";
	} else {
		act->sparam1 = list[1];
		act->sparam2 = list[0];
	}

	long const probability = atoi(cmd[2].c_str());
	if (probability < 1 or 255 < probability)
		throw wexception
			("probability out of range (1, 255) \"%s\"", cmd[2].c_str());
	act->iparam1 = probability;
}

bool Immovable::run_seed(Game* g, bool, const ImmovableAction & action)
{
	Map const & map = g->map();
	MapFringeRegion<> mr(map, Area<>(get_position(), 0));
	uint32_t fringe_size = 0;
	do {
		mr.extend(map);
		fringe_size += 6;
	} while (g->logic_rand() % 256 < static_cast<uint32_t>(action.iparam1));
	for (uint32_t n = g->logic_rand() % fringe_size; n; --n)
		mr.advance(map);
	{
		Field const & f = map[mr.location()];
		if (not f.get_immovable() and f.get_caps() & MOVECAPS_WALK)
			g->create_immovable
				(mr.location(),
				 action.sparam1,
				 action.sparam2 == "world" ?
				 0 : g->get_tribe(action.sparam2.c_str()));
	}

	m_program_step = schedule_act(g, 1);
	m_program_ptr = (m_program_ptr + 1) % m_program->get_size();
	return true;
}

/*
==============================================================================

PlayerImmovable IMPLEMENTATION

==============================================================================
*/

/**
 * Zero-initialize
*/
PlayerImmovable::PlayerImmovable(const Map_Object_Descr & mo_descr) :
	BaseImmovable(mo_descr), m_owner(0), m_economy(0)
{}

/**
 * Cleanup
*/
PlayerImmovable::~PlayerImmovable()
{
	if (m_workers.size())
		log
			("Building::~Building: %lu workers left!\n",
			 static_cast<long unsigned int>(m_workers.size()));
}

/**
 * Change the economy, transfer the workers
*/
void PlayerImmovable::set_economy(Economy *e)
{
	if (m_economy == e)
		return;

	for (uint32_t i = 0; i < m_workers.size(); ++i)
		m_workers[i]->set_economy(e);

	m_economy = e;
}

/**
 * Associate the given worker with this immovable.
 * The worker will be transferred along to another economy, and it will be
 * released when the immovable is destroyed.
 *
 * This should only be called from Worker::set_location.
*/
void PlayerImmovable::add_worker(Worker *w)
{
	m_workers.push_back(w);
}

/**
 * Disassociate the given worker with this building.
 *
 * This should only be called from Worker::set_location.
*/
void PlayerImmovable::remove_worker(Worker *w)
{
	for (uint32_t i = 0; i < m_workers.size(); ++i) {
		if (m_workers[i] == w) {
			if (i < m_workers.size()-1)
				m_workers[i] = m_workers[m_workers.size()-1];
			m_workers.pop_back();
			return;
		}
	}

	throw wexception("PlayerImmovable::remove_worker: not in list");
}


/**
 * Set the immovable's owner. Currently, it can only be set once.
*/
void PlayerImmovable::set_owner(Player * const new_owner) {
	// Change these asserts once you've made really sure that changing owners
	// works (necessary for military building)
	/*THIS IS A TEST
   assert(!m_owner);
	assert(new_owner);*/

	m_owner = new_owner;

	m_owner->egbase().receive(NoteImmovable(this, GAIN));
}

/**
 * Initialize the immovable.
*/
void PlayerImmovable::init(Editor_Game_Base *g)
{
	BaseImmovable::init(g);
}

/**
 * Release workers
*/
void PlayerImmovable::cleanup(Editor_Game_Base *g)
{
	while (m_workers.size())
		m_workers[0]->set_location(0);

	if (m_owner)
		m_owner->egbase().receive(NoteImmovable(this, LOSE));

	BaseImmovable::cleanup(g);
}

/**
 * Dump general information
 */
void PlayerImmovable::log_general_info(Editor_Game_Base* egbase)
{
	BaseImmovable::log_general_info(egbase);

	molog("this: %p\n", this);
	molog("m_owner: %p\n", m_owner);
	molog("* player nr: %i\n", m_owner->get_player_number());
	molog("m_economy: %p\n", m_economy);
}

};
