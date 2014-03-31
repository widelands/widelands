/*
 * Copyright (C) 2002-2004, 2006-2010, 2012-2013 by the Widelands Development Team
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

#include "logic/worker_descr.h"

#include "graphic/graphic.h"
#include "helper.h"
#include "i18n.h"
#include "logic/carrier.h"
#include "logic/nodecaps.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "logic/worker_program.h"
#include "profile/profile.h"
#include "ref_cast.h"
#include "sound/sound_handler.h"
#include "wexception.h"

namespace Widelands {

Worker_Descr::Worker_Descr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Tribe_Descr & _tribe)
	:
	BobDescr(_name, _descname, directory, prof, global_s, &_tribe),
	m_helptext(global_s.get_string("help", "")),
	m_ware_hotspot(global_s.get_Point("ware_hotspot", Point(0, 15))),
	m_icon_fname(directory + "/menu.png"),
	m_icon(nullptr),
	m_buildable     (false),
	m_level_experience(-1),
	m_becomes (Ware_Index::Null())
{
	add_attribute(Map_Object::WORKER);

	m_default_target_quantity =
		global_s.get_positive("default_target_quantity", std::numeric_limits<uint32_t>::max());

	if (Section * const s = prof.get_section("buildcost")) {
		m_buildable = true;
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
			} catch (const _wexception & e) {
				throw wexception
					("[buildcost] \"%s=%s\": %s",
					 val->get_name(), val->get_string(), e.what());
			}
	}

	// If worker has a work animation load and add it.
	Section * work_s = prof.get_section("work");
	if (work_s)
		add_animation("work", g_gr->animations().load(directory, *work_s));

	// Read the walking animations
	m_walk_anims.parse(*this, directory, prof, "walk");
	m_walkload_anims.parse(*this, directory, prof, "walkload", true);

	// Read the becomes and experience
	if (char const * const becomes_name = global_s.get_string("becomes")) {
		m_becomes = tribe().safe_worker_index(becomes_name);
		m_level_experience = global_s.get_safe_positive("experience");
	}

	// Read programs
	while (Section::Value const * const v = global_s.get_next_val("program")) {
		std::string program_name = v->get_string();
		std::transform
			(program_name.begin(), program_name.end(), program_name.begin(),
			 tolower);
		WorkerProgram * program = nullptr;

		try {
			if (m_programs.count(program_name))
				throw wexception("this program has already been declared");
			WorkerProgram::Parser parser;

			parser.descr = this;
			parser.directory = directory;
			parser.prof = &prof;

			program = new WorkerProgram(program_name);
			program->parse(this, &parser, program_name.c_str());
			m_programs[program_name.c_str()] = program;
		}

		catch (const std::exception & e) {
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
	m_icon = g_gr->images().get(m_icon_fname);
}


/**
 * Get a program from the workers description.
 */
WorkerProgram const * Worker_Descr::get_program
	(const std::string & programname) const
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
	Worker & worker = ref_cast<Worker, Map_Object>(create_object());
	worker.set_owner(&owner);
	worker.set_location(location);
	worker.set_position(egbase, coords);
	worker.init(egbase);
	return worker;
}


uint32_t Worker_Descr::movecaps() const {return MOVECAPS_WALK;}


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
	const Worker_Descr & descr = *tribe().get_worker_descr(index);
	Ware_Index const becomes_index = descr.becomes();
	return becomes_index ? can_act_as(becomes_index) : false;
}

Ware_Index Worker_Descr::worker_index() const {
	return tribe().worker_index(name().c_str());
}

}
