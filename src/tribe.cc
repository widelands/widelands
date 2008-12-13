/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "tribe.h"

#include "carrier.h"
#include "constructionsite.h"
#include "critter_bob.h"
#include "editor_game_base.h"
#include "game.h"
#include "helper.h"
#include "i18n.h"
#include "immovable.h"
#include "layered_filesystem.h"
#include "militarysite.h"
#include "profile.h"
#include "soldier.h"
#include "trainingsite.h"
#include "warehouse.h"
#include "wexception.h"
#include "worker.h"
#include "widelands_fileread.h"
#include "world.h"

#include "disk_filesystem.h"

#include "upcast.h"

#include <iostream>

using namespace std;

namespace Widelands {

//
// Tribe_Descr class
//
Tribe_Descr::Tribe_Descr(const std::string & tribename, const World & the_world)
: m_name(tribename), m_world(the_world)
{
	assert(&the_world);
	try {
		std::string path = "tribes/";
		path            += tribename;

		// Grab the localization textdomain.
		// 'path' must be without final /
		i18n::Textdomain textdomain(path);

		path            += '/';
		std::string::size_type const base_path_size = path.size();

		m_default_encdata.clear();

		path += "conf";
		Profile root_conf(path.c_str());
		path.resize(base_path_size);

		{
			std::set<std::string> descnames; //  To enforce descname uniqueness.

			Section & section = root_conf.get_safe_section("map object types");
			while (Section::Value const * const v = section.get_next_val(0)) {
				char const * const     _name = v->get_name  ();
				char const * const _descname = v->get_string();
				if (descnames.count(_name))
					throw wexception
						("object name \"%s\" is already used", v->get_string());
				descnames.insert(v->get_string());
				path += _name;
				path += "/conf";
				try {
					Profile prof(path.c_str(), "global");
					path.resize(path.size() - strlen("conf"));
					Section & global_s = prof.get_safe_section("global");
					if (char const * const type = global_s.get_string("type")) {
						//  Compares are sorted by frequence:
						//    grep ^type= {global,tribes,worlds}/*/*/conf|grep -v .svn|sed "s@.*type=\([^# ]*\).*@type=\1@"|sort|uniq -c|sort -gr
						//  But for even better performance, gperf should be used.
						if      (not strcasecmp(type, "ware"))
							m_wares.add
								(new Item_Ware_Descr
								 	(_name, _descname, path, prof, global_s));
						else if (not strcasecmp(type, "production"))
							m_buildings.add
								(new ProductionSite_Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else if (not strcasecmp(type, "generic"))
							m_workers.add
								(new Worker_Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else if (not strcasecmp(type, "critter"))
							m_bobs.add
								(new Critter_Bob_Descr
								 	(_name, _descname, path, prof, global_s, this, &m_default_encdata));
						else if (not strcasecmp(type, "military"))
							m_buildings.add
								(new MilitarySite_Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else if (not strcasecmp(type, "training"))
							m_buildings.add
								(new TrainingSite_Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else if (not strcasecmp(type, "warehouse"))
							m_buildings.add
								(new Warehouse_Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else if (not strcasecmp(type, "carrier"))
							m_workers.add
								(new Carrier::Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else if (not strcasecmp(type, "construction"))
							m_buildings.add
								(new ConstructionSite_Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else if (not strcasecmp(type, "soldier"))
							m_workers.add
								(new Soldier_Descr
								 	(_name, _descname, path, prof, global_s, *this, &m_default_encdata));
						else
							throw wexception("unknown map object type");
					} else
						m_immovables.add
							(new Immovable_Descr
							 	(_name, _descname, path, prof, global_s, this));
					prof.check_used();
				} catch (std::exception const & e) {
					throw wexception("%s=\"%s\": %s", _name, _descname, e.what());
				}
				path.resize(base_path_size);
			}
		}

		try {
			{
				Section & tribe_s = root_conf.get_safe_section("tribe");
				tribe_s.get_string("author");
				tribe_s.get_string("name"); // descriptive name
				tribe_s.get_string("descr"); // long description
				m_bob_vision_range = tribe_s.get_int("bob_vision_range");
			}

			if (Section * const defaults_s = root_conf.get_section("defaults"))
				m_default_encdata.parse(*defaults_s);

			m_anim_frontier =
				g_anim.get(path, root_conf.get_safe_section("frontier"), 0, &m_default_encdata);
			m_anim_flag     =
				g_anim.get(path, root_conf.get_safe_section("flag"),     0, &m_default_encdata);

			try { //  FIXME eliminate
				Section & swa_s = root_conf.get_safe_section("startwares");
				while (Section::Value const * const value = swa_s.get_next_val(0))
				{
					if (not m_wares.exists(value->get_name()))
						throw wexception
							("In section [startwares], ware %s is not known!",
							 value->get_name());
					m_startwares[value->get_name()] = value->get_int();
				}
			} catch (_wexception const & e) {
				throw wexception("section [startwares]: %s", e.what());
			}

			try { //  FIXME eliminate
				Section & swo_s = root_conf.get_safe_section("startworkers");
				while (Section::Value const * const value = swo_s.get_next_val(0))
				{
					if (strcmp(value->get_name(), "soldier")) { // Ignore soldiers
						if (not m_workers.exists(value->get_name()))
							throw wexception
								("\"%s\" is not known!",
								 value->get_name());
						m_startworkers[value->get_name()] = value->get_int();
					} else
						throw wexception
							("\"soldier\" not allowed here, should be in "
							 "[startsoldiers]");
				}
			} catch (_wexception const & e) {
				throw wexception("section [startworkers]: %s", e.what());
			}

			{ //  FIXME eliminate
				Section & sso_s = root_conf.get_safe_section("startsoldiers");
				while (Section::Value const * const value = sso_s.get_next_val(0))
				//  NOTE no check here; we do not know about max levels and so on
					m_startsoldiers[value->get_name()] = value->get_int();
			}
		} catch (std::exception const & e) {
			throw wexception("root conf: %s", e.what());
		}
#ifdef WRITE_GAME_DATA_AS_HTML
		m_ware_references     = new HTMLReferences[get_nrwares    ().value()];
		m_worker_references   = new HTMLReferences[get_nrworkers  ().value()];
		m_building_references = new HTMLReferences[get_nrbuildings().value()];
		writeHTMLBuildings(path);
		writeHTMLWorkers  (path);
		writeHTMLWares    (path);
		delete[] m_building_references;
		delete[] m_worker_references;
		delete[] m_ware_references;
#endif
	}
	catch (std::exception &e)
	{throw wexception("Error loading tribe %s: %s", tribename.c_str(), e.what());}
}


/*
===============
Tribe_Descr::postload

Load all logic data
===============
*/
void Tribe_Descr::postload(Editor_Game_Base *) {
	// TODO: move more loads to postload
}

/*
===============
Tribe_Descr::load_graphics

Load tribe graphics
===============
*/
void Tribe_Descr::load_graphics()
{
	for (Ware_Index i = Ware_Index::First(); i < m_workers.get_nitems(); ++i)
		m_workers.get(i)->load_graphics();

	for (Ware_Index i = Ware_Index::First(); i < m_wares.get_nitems  (); ++i)
		m_wares.get(i)->load_graphics();

	for
		(Building_Index i = Building_Index::First();
		 i < m_buildings.get_nitems();
		 ++i)
		m_buildings.get(i)->load_graphics();
}


/*
===========
This loads a warehouse with the given start wares as defined in
the conf files
===========
*/
void Tribe_Descr::load_warehouse_with_start_wares
	(Editor_Game_Base & egbase, Warehouse & wh) const
{
	container_iterate_const(starting_resources_map, m_startwares,    j)
		wh.insert_wares
			(safe_ware_index  (j.current->first.c_str()), j.current->second);
	container_iterate_const(starting_resources_map, m_startworkers,  j)
		wh.insert_workers
			(safe_worker_index(j.current->first.c_str()), j.current->second);
	container_iterate_const(starting_resources_map, m_startsoldiers, j) {
		std::vector<std::string> const list(split_string(j.current->first, "/"));

		if (list.size() != 4)
			throw wexception
				("Error in tribe (%s), startsoldier %s is not valid!",
				 name().c_str(), j.current->first.c_str());

		char * endp;
		long int const hplvl      = strtol(list[0].c_str(), &endp, 0);
		if (*endp)
			throw wexception("Bad hp level '%s'", list[0].c_str());
		long int const attacklvl  = strtol(list[1].c_str(), &endp, 0);
		if (*endp)
			throw wexception("Bad attack level '%s'", list[1].c_str());
		long int const defenselvl = strtol(list[2].c_str(), &endp, 0);
		if (*endp)
			throw wexception("Bad defense level '%s'", list[2].c_str());
		long int const evadelvl   = strtol(list[3].c_str(), &endp, 0);
		if (*endp)
			throw wexception("Bad evade level '%s'", list[3].c_str());

		if (upcast(Game, game, &egbase))
			for (int32_t i = 0; i < j.current->second; ++i) {
				Soldier & soldier =
					static_cast<Soldier &>
						(dynamic_cast<Soldier_Descr const &>
						 	(*get_worker_descr(worker_index("soldier")))
						 .create(*game, wh.owner(), wh, wh.get_position()));
				soldier.set_level(hplvl, attacklvl, defenselvl, evadelvl);
				wh.incorporate_worker(game, &soldier);
			}
	}
}


/*
 * does this tribe exist?
 */
bool Tribe_Descr::exists_tribe(const std::string & name, TribeBasicInfo* info) {
	std::string buf = "tribes/";
	buf            += name;
	buf            += "/conf";

	FileRead f;
	if (f.TryOpen(*g_fs, buf.c_str())) {
		if (info) {
			Profile prof(buf.c_str());
			info->name = name;
			info->uiposition =
				prof.get_safe_section("tribe").get_int("uiposition", 0);
		}

		return true;
	}

	return false;
}

struct TribeBasicComparator {
	bool operator()(const TribeBasicInfo& t1, const TribeBasicInfo& t2) {
		return t1.uiposition < t2.uiposition;
	}
};

/**
 * Fills the given string vector with the names of all tribes that exist.
 */
void Tribe_Descr::get_all_tribenames(std::vector<std::string> & target) {
	assert(target.empty());

	//  get all tribes
	std::vector<TribeBasicInfo> tribes;
	filenameset_t m_tribes;
	g_fs->FindFiles("tribes", "*", &m_tribes);
	for
		(filenameset_t::iterator pname = m_tribes.begin();
		 pname != m_tribes.end();
		 ++pname)
	{
		const std::string name = pname->substr(7);
		TribeBasicInfo info;
		if (Tribe_Descr::exists_tribe(name, &info))
			tribes.push_back(info);
	}

	std::sort(tribes.begin(), tribes.end(), TribeBasicComparator());
	for
		(std::vector<TribeBasicInfo>::const_iterator it = tribes.begin();
		 it != tribes.end();
		 ++it)
	{
		target.push_back(it->name);
	}
}

/*
==============
Resource_Descr::get_indicator

Find the best matching indicator for the given amount.
==============
*/
uint32_t Tribe_Descr::get_resource_indicator
	(Resource_Descr const * const res, uint32_t const amount) const
{
	if (not res or not amount) {
		int32_t idx = get_immovable_index("resi_none");
		if (idx == -1)
			throw wexception
				("Tribe %s doesn't declare a resource indicator resi_none!",
				 name().c_str());
		return idx;
	}

	char buffer[256];

	int32_t i = 1;
	int32_t num_indicators = 0;
	for (;;) {
		snprintf(buffer, sizeof(buffer), "resi_%s%i", res->name().c_str(), i);
		if (get_immovable_index(buffer) == -1)
			break;
		++i;
		++num_indicators;
	}

	if (not num_indicators)
		throw wexception
			("Tribe %s doesn't declar a resource indicator for resource %s!",
			 name().c_str(),
			 res->name().c_str());

	uint32_t bestmatch =
		static_cast<uint32_t>
		((static_cast<float>(amount)/res->get_max_amount()) * num_indicators);
	if (static_cast<int32_t>(amount) < res->get_max_amount())
		bestmatch += 1; // Resi start with 1, not 0

	snprintf
		(buffer, sizeof(buffer), "resi_%s%i", res->name().c_str(), bestmatch);

	// NoLog("Resource(%s): Indicator '%s' for amount = %u\n",
	//res->get_name(), buffer, amount);



	return get_immovable_index(buffer);
}

/*
 * Return the given ware or die trying
 */
Ware_Index Tribe_Descr::safe_ware_index(std::string const & warename) const {
	if (Ware_Index const result = ware_index(warename))
		return result;
	else
		throw wexception
			("tribe %s does not define ware type \"%s\"",
			 name().c_str(), warename.c_str());
}
Ware_Index Tribe_Descr::safe_ware_index(const char * const warename) const {
	if (Ware_Index const result = ware_index(warename))
		return result;
	else
		throw wexception
			("tribe %s does not define ware type \"%s\"",
			 name().c_str(), warename);
}

/*
 * Return the given worker or die trying
 */
Ware_Index Tribe_Descr::safe_worker_index(std::string const & workername) const
{
	if (Ware_Index const result = worker_index(workername))
		return result;
	else
		throw wexception
			("tribe %s does not define worker type \"%s\"",
			 name().c_str(), workername.c_str());
}
Ware_Index Tribe_Descr::safe_worker_index(const char * const workername) const {
	if (Ware_Index const result = worker_index(workername))
		return result;
	else
		throw wexception
			("tribe %s does not define worker type \"%s\"",
			 name().c_str(), workername);
}

/*
 * Return the given building or die trying
 */
Building_Index Tribe_Descr::safe_building_index(const char *buildingname) const {
	Building_Index const result = building_index(buildingname);

	if (not result)
		throw wexception
			("tribe %s does not define building type \"%s\"",
			 name().c_str(), buildingname);
	return result;
}

};
