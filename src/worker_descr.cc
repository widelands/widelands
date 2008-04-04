/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "carrier.h"
#include "graphic.h"
#include "helper.h"
#include "i18n.h"
#include "profile.h"
#include "soldier.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "wexception.h"
#include "worker.h"
#include "worker_descr.h"
#include "worker_program.h"

namespace Widelands {

Worker_Descr::Worker_Descr
	(Tribe_Descr const & tribe_descr, std::string const & worker_name)
	:
	Bob::Descr        (&tribe_descr, worker_name),
	m_menu_pic_fname  (0),
	m_menu_pic        (0)
{
	add_attribute(Map_Object::WORKER);
}


Worker_Descr::~Worker_Descr()
{
	if (m_menu_pic_fname)
		free(m_menu_pic_fname);

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
	m_menu_pic = g_gr->get_picture(PicMod_Game,  m_menu_pic_fname);
}


/**
 * Get a program from the workers description.
 */
const WorkerProgram* Worker_Descr::get_program(std::string programname) const
{
	ProgramMap::const_iterator it = m_programs.find(programname);

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
	 PlayerImmovable  &       location,
	 Coords             const coords)
const
{
	Worker & worker = dynamic_cast<Worker &>(*create_object());
	worker.set_owner(&owner);
	worker.set_location(&location);
	worker.set_position(&egbase, coords);
	worker.init(&egbase);
	return worker;
}


/**
 * Parse the worker data from configuration
 */
void Worker_Descr::parse
	(char const       * const directory,
	 Profile          * const prof,
	 becomes_map_t    &       becomes_map,
	 EncodeData const * const encdata)
{
	char buffer[256];
	char fname[256];
	Section* sglobal;

	Bob::Descr::parse(directory, prof, encdata);

	sglobal = prof->get_safe_section("global");

	m_descname = sglobal->get_string("descname", name().c_str());
	m_helptext =
		sglobal->get_string("help", _("Doh... someone forgot the help text!"));

	snprintf(buffer, sizeof(buffer), "%s_menu.png", name().c_str());
	const char * string = sglobal->get_string("menu_pic", buffer);
	snprintf(fname, sizeof(fname), "%s/%s", directory, string);
	m_menu_pic_fname = strdup(fname);

	// Read the costs of building
	if (get_worker_type() == CARRIER || get_worker_type() == SOLDIER)
		m_buildable = sglobal->get_bool("buildable", false);
	else
		m_buildable = sglobal->get_bool("buildable", true);

	if (m_buildable) {
		Section *s;

		// Get the buildcost
		s = prof->get_safe_section("buildcost");

		Section::Value *val;

		while ((val = s->get_next_val(0)))
			m_buildcost.push_back (CostItem(val->get_name(), val->get_int()));
	}

	// Read the walking animations
	m_walk_anims.parse
		(this, directory, prof, "walk_??", prof->get_section("walk"), encdata);

	if (get_worker_type() != SOLDIER) // Soldier have no walkload
		m_walkload_anims.parse
			(this,
			 directory,
			 prof,
			 "walkload_??",
			 prof->get_section("walkload"),
			 encdata);

	// Read the sound effects
	while (sglobal->get_next_string("soundfx", &string))
		g_sound_handler.load_fx(directory, string);

	// Read the becomes and experience
	if (char const * const becomes_name = sglobal->get_string("becomes"))
		becomes_map[this] = becomes_name;
	std::string exp=sglobal->get_string("experience", "");
	m_min_experience=m_max_experience=-1;
	if (exp.size()) {
		std::vector<std::string> list(split_string(exp, "-"));
		if (list.size()!=2)
			throw wexception("Parse error in experience string: \"%s\" (must be \"min-max\")", exp.c_str());
		remove_spaces(list[0]);
		remove_spaces(list[1]);

		char* endp;
		m_min_experience = strtol(list[0].c_str(), &endp, 0);
		if (endp && *endp)
			throw wexception("Parse error in experience string: %s is a bad value", list[0].c_str());
		m_max_experience = strtol(list[1].c_str(), &endp, 0);
		if (endp && *endp)
			throw wexception("Parse error in experience string: %s is a bad value", list[1].c_str());
	}

	// Read programs
	while (sglobal->get_next_string("program", &string)) {
		WorkerProgram* prog = 0;

		try {
			WorkerProgram::Parser parser;

			parser.descr = this;
			parser.directory = directory;
			parser.prof = prof;
			parser.encdata = encdata;

			prog = new WorkerProgram(string);
			prog->parse(this, &parser, string);
			m_programs[prog->get_name()] = prog;
		}

		catch (std::exception& e) {
			delete prog;
			throw wexception("Parse error in program %s: %s", string, e.what());
		}
	}
}

/**
 * Create a generic worker of this type.
 */
Bob * Worker_Descr::create_object() const
{
	return new Worker(*this);
}


/**
 * Automatically create the appropriate Worker_Descr type from the given
 * config data.
 * \note May return 0.
 */
Worker_Descr * Worker_Descr::create_from_dir
	(Tribe_Descr const &       tribe,
	 becomes_map_t     &       becomes_map,
	 char        const * const directory,
	 EncodeData  const * const encdata)
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
			descr = new Carrier::Descr(tribe, name);
		else if (!strcasecmp(type, "soldier"))
			descr = new Soldier_Descr(tribe, name);
		else
			throw wexception("Unknown worker type '%s' [supported: carrier, soldier]", type);

		descr->parse(directory, &prof, becomes_map, encdata);
	}
	catch (std::exception &e) {
		delete descr;
		throw wexception("Error reading worker %s: %s", name, e.what());
	}
	catch (...) {
		delete descr;
		throw;
	}

	return descr;
}


/**
* check if worker can be substitute for a requested worker type
 */
bool Worker_Descr::can_act_as(Ware_Index const index) const {
	if (index == tribe().worker_index(name().c_str()))
		return true;

	// if requested worker type can be promoted, compare with that type
	Worker_Descr const & descr = *tribe().get_worker_descr(index);
	Ware_Index const becomes_index = descr.becomes();
	return becomes_index ? can_act_as(becomes_index) : false;
}

};
