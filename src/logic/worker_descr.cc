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

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "logic/carrier.h"
#include "logic/game_data_error.h"
#include "logic/nodecaps.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "logic/worker_program.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"

namespace Widelands {

WorkerDescr::WorkerDescr
	(const MapObjectType object_type, char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const TribeDescr & _tribe)
	:
	BobDescr(object_type, _name, _descname, &_tribe),
	helptext_          (global_s.get_string("help", "")),
	ware_hotspot_      (global_s.get_point("ware_hotspot", Point(0, 15))),
	icon_fname_        (directory + "/menu.png"),
	icon_              (nullptr),
	needed_experience_  (-1),
	becomes_           (INVALID_INDEX)
{
	{ //  global options
		Section & idle_s = prof.get_safe_section("idle");
		add_animation("idle", g_gr->animations().load(directory, idle_s));
	}

	add_attribute(MapObject::Attribute::WORKER);

	default_target_quantity_ =
		global_s.get_positive("default_target_quantity", std::numeric_limits<uint32_t>::max());

	if (Section * const s = prof.get_section("buildcost")) {
		while (Section::Value const * const val = s->get_next_val())
			try {
				std::string const input = val->get_name();
				if (buildcost_.count(input))
					throw wexception("a buildcost item of this ware type has already been defined");
				if (tribe().ware_index(input) == INVALID_INDEX &&
				    tribe().worker_index(input) == INVALID_INDEX)
					throw wexception
						("\"%s\" has not been defined as a ware/worker type (wrong "
						 "declaration order?)",
						 input.c_str());
				int32_t const value = val->get_int();
				uint8_t const count = value;
				if (count != value)
					throw wexception("count is out of range 1 .. 255");
				buildcost_.insert(std::pair<std::string, uint8_t>(input, value));
			} catch (const WException & e) {
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
	walk_anims_.parse(*this, directory, prof, "walk");
	walkload_anims_.parse(*this, directory, prof, "walkload", true);

	// Read the becomes and experience
	if (char const * const becomes_name = global_s.get_string("becomes")) {
		becomes_ = tribe().safe_worker_index(becomes_name);
		needed_experience_ = global_s.get_safe_positive("experience");
	}

	// Read programs
	while (Section::Value const * const v = global_s.get_next_val("program")) {
		std::string program_name = v->get_string();
		std::transform
			(program_name.begin(), program_name.end(), program_name.begin(),
			 tolower);
		WorkerProgram * program = nullptr;

		try {
			if (programs_.count(program_name))
				throw wexception("this program has already been declared");
			WorkerProgram::Parser parser;

			parser.descr = this;
			parser.directory = directory;
			parser.prof = &prof;
			parser.table = nullptr;

			program = new WorkerProgram(program_name);
			program->parse(this, &parser, program_name.c_str());
			programs_[program_name.c_str()] = program;
		}

		catch (const std::exception & e) {
			delete program;
			throw wexception("program %s: %s", program_name.c_str(), e.what());
		}
	}
}

WorkerDescr::WorkerDescr(const LuaTable& table) :
	BobDescr(MapObjectType::WORKER, table),
	icon_fname_        (directory + "/menu.png"),
	icon_              (nullptr),
	ware_hotspot_      (Point(0, 15)),
	needed_experience_ (-1),
	becomes_           (INVALID_INDEX)
{
	LuaTable items_table = table.get_table("buildcost");
	for (const std::string& key : items_table.keys()) {
		try {
			if (buildcost_.count(key)) {
				throw wexception("a buildcost item of this ware type has already been defined: %s", key.c_str());
			}
			// NOCOM(GunChleoc): Do the check with the new tribes object
			/*
			if (tribe().ware_index(key) == INVALID_INDEX &&
				 tribe().worker_index(key) == INVALID_INDEX) {
				throw wexception
					("\"%s\" has not been defined as a ware/worker type (wrong "
					 "declaration order?)",
					 key.c_str());
			}
			*/
			int32_t const value = items_table.get_int(key);
			uint8_t const count = value;
			if (count != value)
				throw wexception("count is out of range 1 .. 255");
			buildcost_.insert(std::pair<std::string, uint8_t>(key, count));
		} catch (const WException & e) {
			throw wexception
				("[buildcost] \"%s=%d\": %s",
				 key.c_str(), value, e.what());
		}
	}

	helptext_ = table.get_string("helptext");

	items_table = table.get_table("animations");
	for (const std::string& key : items_table.keys()) {
		const LuaTable anims_table = items_table.get_table(key);
		for (const std::string& anim_key : anims_table.keys()) {
			// NOCOM(GunChleoc): And the hotspot + fps?
			add_animation(anim_key, g_gr->animations().load(anims_table.get_string("pictures")));
		}
	}

	// If worker has a work animation load and add it.
	if(table.has_key("work")) {
		items_table = table.get_table("work");
		for (const std::string& key : items_table.keys()) {
			// NOCOM(GunChleoc): And the hotspot + fps? Also check anims below.
			add_animation(key, g_gr->animations().load(items_table.get_string("pictures")));
		}
	}

	// Read the walking animations
	add_directional_animation(&walk_anims_, "walk");
	add_directional_animation(&walkload_anims_, "walkload");

	// Read the becomes and experience
	if(table.has_key("becomes")) {
		becomes_ = table.get_string("becomes");
		needed_experience_ = table.get_int("experience");
	}

	// Read programs
	if(table.has_key("programs")) {
		items_table = table.get_table("programs");
		for (std::string program_name : items_table.keys()) {
			std::transform
				(program_name.begin(), program_name.end(), program_name.begin(),
				 tolower);
			WorkerProgram * program = nullptr;

			try {
				if (programs_.count(program_name))
					throw wexception("this program has already been declared");

				// NOCOM(GunChleoc): Redefine the parser for Lua Tables.
				WorkerProgram::Parser parser;

				parser.descr = this;
				parser.directory = directory;
				parser.prof = nullptr;
				parser.table = &items_table;

				program = new WorkerProgram(program_name);
				program->parse(this, &parser, program_name.c_str());
				programs_[program_name.c_str()] = program;
			}

			catch (const std::exception & e) {
				delete program;
				throw wexception("program %s: %s", program_name.c_str(), e.what());
			}
		}
	}

	// For carriers
	if(table.has_key("default_target_quantity")) {
		default_target_quantity_ = items_table.get_int("default_target_quantity");
	}
	if(table.has_key("ware_hotspot")) {
		items_table = table.get_table("ware_hotspot");
		ware_hotspot_(Point(items_table.get_int("1"),items_table.get_int("2")));
	}
}


WorkerDescr::~WorkerDescr()
{
	while (programs_.size()) {
		delete programs_.begin()->second;
		programs_.erase(programs_.begin());
	}
}

// NOCOM(GunChleoc): WorkerDescr, won't know their tribe, only the workers.
const TribeDescr& WorkerDescr::tribe() const {
	const TribeDescr* owner_tribe = get_owner_tribe();
	assert(owner_tribe != nullptr);
	return *owner_tribe;
}

/**
 * Load graphics (other than animations).
 */
void WorkerDescr::load_graphics()
{
	icon_ = g_gr->images().get(icon_fname_);
}


/**
 * Get a program from the workers description.
 */
WorkerProgram const * WorkerDescr::get_program
	(const std::string & programname) const
{
	Programs::const_iterator it = programs_.find(programname);

	if (it == programs_.end())
		throw wexception
			("%s has no program '%s'", name().c_str(), programname.c_str());

	return it->second;
}

/**
 * Custom creation routing that accounts for the location.
 */
Worker & WorkerDescr::create
	(EditorGameBase &       egbase,
	 Player           &       owner,
	 PlayerImmovable  * const location,
	 Coords             const coords)
const
{
	Worker & worker = dynamic_cast<Worker&>(create_object());
	worker.set_owner(&owner);
	worker.set_location(location);
	worker.set_position(egbase, coords);
	worker.init(egbase);
	return worker;
}


uint32_t WorkerDescr::movecaps() const {return MOVECAPS_WALK;}


/**
 * Create a generic worker of this type.
 */
Bob & WorkerDescr::create_object() const
{
	return *new Worker(*this);
}


/**
* check if worker can be substitute for a requested worker type
 */
bool WorkerDescr::can_act_as(WareIndex const index) const {
	assert(index < tribe().get_nrworkers());
	if (index == worker_index())
		return true;

	// if requested worker type can be promoted, compare with that type
	const WorkerDescr & descr = *tribe().get_worker_descr(index);
	WareIndex const becomes_index = descr.becomes();
	return becomes_index != INVALID_INDEX ? can_act_as(becomes_index) : false;
}

WareIndex WorkerDescr::worker_index() const {
	return tribe().worker_index(name());
}

}
