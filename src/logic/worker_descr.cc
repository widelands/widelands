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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "worker_descr.h"

#include "carrier.h"

#include "graphic/graphic.h"
#include "helper.h"
#include "i18n.h"
#include "profile/profile.h"
#include "soldier.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"
#include "worker_program.h"

namespace Widelands {

Worker_Descr::Worker_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _tribe, EncodeData const * const encdata)
	:
	Bob::Descr(_name, _descname, directory, prof, global_s, &_tribe, encdata),
	m_helptext(global_s.get_string("help", "")),
	m_icon_fname(directory + "/menu.png"),
	m_icon(g_gr->get_no_picture()),
	m_becomes (Ware_Index::Null())
{
	add_attribute(Map_Object::WORKER);

	if (Section * const s = prof.get_section("buildcost"))
		while (Section::Value const * const val = s->get_next_val())
			try {
				std::string const input = val->get_name();
				if (m_buildcost.count(input))
					throw wexception
						("a buildcost item of this ware type has already been "
						 "defined");
				if (not (tribe().ware_index(input) or tribe().worker_index(input)))
					throw wexception
						("\"%s\" has not beed defined as a ware/worker type (wrong "
						 "declaration order?)",
						 input.c_str());
				int32_t const value = val->get_int();
				uint8_t const count = value;
				if (count != value)
					throw wexception("count is out of range 1 .. 255");
				m_buildcost.insert(std::pair<std::string, uint8_t>(input, value));
			} catch (_wexception const & e) {
				throw wexception
					("[buildcost] \"%s=%s\": %s",
					 val->get_name(), val->get_string(), e.what());
			}

	// Read the walking animations
	m_walk_anims.parse
		(*this, directory, prof, "walk_??", prof.get_section("walk"), encdata);

	//  Soldiers have no walkload.
	if (not global_s.has_val("max_hp_level"))
		m_walkload_anims.parse
			(*this,
			 directory,
			 prof,
			 "walkload_??",
			 prof.get_section("walkload"),
			 encdata);

	while (Section::Value const * const v = global_s.get_next_val("soundfx"))
		g_sound_handler.load_fx(directory, v->get_string());

	// Read the becomes and experience
	if (char const * const becomes_name = global_s.get_string("becomes")) {
		m_becomes = tribe().safe_worker_index(becomes_name);
		std::string const exp = global_s.get_string("experience", "");
		m_min_experience = m_max_experience = -1;
		if (exp.size()) {
			std::vector<std::string> list(split_string(exp, "-"));
			if (list.size() != 2)
				throw wexception
					("Parse error in experience string: \"%s\" (must be "
					 "\"min-max\")",
					 exp.c_str());
			remove_spaces(list[0]);
			remove_spaces(list[1]);

			char * endp;
			m_min_experience = strtol(list[0].c_str(), &endp, 0);
			if (*endp)
				throw wexception
					("Parse error in experience string: %s is a bad value",
					 list[0].c_str());
			m_max_experience = strtol(list[1].c_str(), &endp, 0);
			if (*endp)
				throw wexception
					("Parse error in experience string: %s is a bad value",
					 list[1].c_str());
		}
	}

	// Read programs
	while (Section::Value const * const v = global_s.get_next_val("program")) {
		std::string const program_name = v->get_string();
		WorkerProgram * program = 0;

		try {
			if (m_programs.count(program_name))
				throw wexception("this program has already been declared");
			WorkerProgram::Parser parser;

			parser.descr = this;
			parser.directory = directory;
			parser.prof = &prof;
			parser.encdata = encdata;

			program = new WorkerProgram(program_name);
			program->parse(this, &parser, program_name.c_str());
			m_programs[program_name.c_str()] = program;
		}

		catch (std::exception const & e) {
			delete program;
			throw wexception("program %s: %s", program_name.c_str(), e.what());
		}
	}
}


Worker_Descr::~Worker_Descr()
{
	while (m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/**
 * Load graphics (other than animations).
 */
void Worker_Descr::load_graphics()
{
	m_icon = g_gr->get_picture(PicMod_Game, m_icon_fname.c_str());
}


/**
 * Get a program from the workers description.
 */
WorkerProgram const * Worker_Descr::get_program
	(std::string const & programname) const
{
	Programs::const_iterator it = m_programs.find(programname);

	if (it == m_programs.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), programname.c_str());

	return it->second;
}


/**
 * Custom creation routing that accounts for the location.
 */
Worker & Worker_Descr::create
	(Editor_Game_Base &       egbase,
	 Player           &       owner,
	 PlayerImmovable  * const location,
	 Coords             const coords)
const
{
	Worker & worker = dynamic_cast<Worker &>(create_object());
	worker.set_owner(&owner);
	worker.set_location(location);
	worker.set_position(egbase, coords);
	worker.init(egbase);
	return worker;
}


uint32_t Worker_Descr::movecaps() const throw () {return MOVECAPS_WALK;}


/**
 * Create a generic worker of this type.
 */
Bob & Worker_Descr::create_object() const
{
	return *new Worker(*this);
}


/**
* check if worker can be substitute for a requested worker type
 */
bool Worker_Descr::can_act_as(Ware_Index const index) const {
	assert(index < tribe().get_nrworkers());
	if (index == worker_index())
		return true;

	// if requested worker type can be promoted, compare with that type
	Worker_Descr const & descr = *tribe().get_worker_descr(index);
	Ware_Index const becomes_index = descr.becomes();
	return becomes_index ? can_act_as(becomes_index) : false;
}

Ware_Index Worker_Descr::worker_index() const throw () {
	return tribe().worker_index(name().c_str());
}

};
