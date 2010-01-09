/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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
#include "events/event_allow_building_types.h"
#include "events/event_allow_retreat_change.h"
#include "events/event_allow_worker_types.h"
#include "events/event_building.h"
#include "events/event_conquer_area.h"
#include "events/event_forbid_building_types.h"
#include "events/event_forbid_retreat_change.h"
#include "events/event_forbid_worker_types.h"
#include "events/event_player_seeall.h"
#include "events/event_set_player_frontier_style.h"
#include "events/event_set_player_flag_style.h"
#include "events/event_unhide_area.h"
#include "events/event_retreat_change.h"
#include "game.h"
#include "game_data_error.h"
#include "helper.h"
#include "i18n.h"
#include "immovable.h"
#include "io/filesystem/layered_filesystem.h"
#include "militarysite.h"
#include "parse_map_object_types.h"
#include "profile/profile.h"
#include "soldier.h"
#include "trainingsite.h"
#include "warehouse.h"
#include "worker.h"
#include "widelands_fileread.h"
#include "world.h"

#include "io/filesystem/disk_filesystem.h"

#include "upcast.h"

#include <iostream>

using namespace std;

namespace Widelands {

Tribe_Descr::Tribe_Descr
	(std::string const & tribename, Editor_Game_Base & egbase)
	: m_name(tribename), m_world(egbase.map().world())
{
	assert(&m_world);
	std::string path = "tribes/";
	try {
		path            += tribename;

		// Grab the localization textdomain.
		// 'path' must be without final /
		i18n::Textdomain textdomain(path);

		path            += '/';
		std::string::size_type base_path_size = path.size();

		m_default_encdata.clear();

		path += "conf";
		Profile root_conf(path.c_str());
		path.resize(base_path_size);

		{
			std::set<std::string> names; //  To enforce name uniqueness.

			PARSE_MAP_OBJECT_TYPES_BEGIN("immovable")
				m_immovables.add
					(new Immovable_Descr
					 	(_name, _descname, path, prof, global_s, m_world, this));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("critter bob")
				m_bobs.add
					(new Critter_Bob_Descr
					 	(_name, _descname, path, prof, global_s,  this,
					 	 &m_default_encdata));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("ware")
				m_wares.add
					(new Item_Ware_Descr
					 	(_name, _descname, path, prof, global_s));
			PARSE_MAP_OBJECT_TYPES_END;

#define PARSE_WORKER_TYPES(name, descr_type)                                  \
         PARSE_MAP_OBJECT_TYPES_BEGIN(name)                                   \
            descr_type & worker_descr =                                       \
               *new descr_type                                                \
                  (_name, _descname, path, prof, global_s, *this,             \
                   &m_default_encdata);                                       \
            Ware_Index const worker_idx = m_workers.add(&worker_descr);       \
            if                                                                \
               (worker_descr.is_buildable() and                               \
                worker_descr.buildcost().empty())                             \
               m_worker_types_without_cost.push_back(worker_idx);             \
         PARSE_MAP_OBJECT_TYPES_END;

			PARSE_WORKER_TYPES("carrier", Carrier::Descr);
			PARSE_WORKER_TYPES("soldier", Soldier_Descr);
			PARSE_WORKER_TYPES("worker",  Worker_Descr);

			PARSE_MAP_OBJECT_TYPES_BEGIN("constructionsite")
				m_buildings.add
					(new ConstructionSite_Descr
					 	(_name, _descname, path, prof, global_s, *this,
					 	 &m_default_encdata));
			PARSE_MAP_OBJECT_TYPES_END;
			if (not safe_building_index("constructionsite"))
				throw game_data_error
					(_("constructionsite type \"constructionsite\" is missing"));

			PARSE_MAP_OBJECT_TYPES_BEGIN("militarysite")
				m_buildings.add
					(new MilitarySite_Descr
					 	(_name, _descname, path, prof, global_s, *this,
					 	 &m_default_encdata));
			PARSE_MAP_OBJECT_TYPES_END;


			// global militarysites are in /global not in /tribes
			std::string temp                = path;
			std::string::size_type sizetemp = base_path_size;
			path           = "global/militarysites/";
			base_path_size = path.size();

			PARSE_MAP_OBJECT_TYPES_BEGIN("global militarysite")
				m_buildings.add
					(new MilitarySite_Descr
					 	(_name, _descname, path, prof, global_s, *this,
					 	 &m_default_encdata));
			PARSE_MAP_OBJECT_TYPES_END;

			// Reset path and base_path_size
			path           = temp;
			base_path_size = sizetemp;


			PARSE_MAP_OBJECT_TYPES_BEGIN("trainingsite")
				m_buildings.add
					(new TrainingSite_Descr
					 	(_name, _descname, path, prof, global_s, *this,
					 	 &m_default_encdata));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("warehouse")
				m_buildings.add
					(new Warehouse_Descr
					 	(_name, _descname, path, prof, global_s, *this,
					 	 &m_default_encdata));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("productionsite")
				m_buildings.add
					(new ProductionSite_Descr
					 	(_name, _descname, path, prof, global_s, *this,
					 	 &m_default_encdata));
			PARSE_MAP_OBJECT_TYPES_END;
		}

		{
			/// Loads military data
			Section * military_data_s = root_conf.get_section("military_data");
			if (military_data_s)
				m_military_data.parse(*military_data_s);
		}

		try {
			{
				Section & tribe_s = root_conf.get_safe_section("tribe");
				tribe_s.get_string("author");
				tribe_s.get_string("name"); // descriptive name
				tribe_s.get_string("descr"); // long description
				m_bob_vision_range = tribe_s.get_int("bob_vision_range");
				m_carrier2         = tribe_s.get_string("carrier2");
			}

			if (Section * const defaults_s = root_conf.get_section("defaults"))
				m_default_encdata.parse(*defaults_s);

			try {
				while (Section * const s = root_conf.get_next_section("frontier"))
				{
					char const * const style_name = s->get_safe_string("name");
					try {
						if (m_anim_frontier.empty())
							throw Nonexistent();
						frontier_style_index(style_name);
						throw game_data_error(_("\"%s\" is duplicated"), style_name);
					} catch (Nonexistent) {
						m_anim_frontier.push_back
							(std::pair<std::string, uint32_t>
							 	(style_name,
							 	 g_anim.get(path, *s, 0, &m_default_encdata)));
					}
				}
				if (m_anim_frontier.empty())
					throw game_data_error(_("none found"));
			} catch (_wexception const & e) {
				throw game_data_error(_("frontier styles: %s"), e.what());
			}
			try {
				while (Section * const s = root_conf.get_next_section("flag"))
				{
					char const * const style_name = s->get_safe_string("name");
					try {
						if (m_anim_flag.empty())
							throw Nonexistent();
						flag_style_index(style_name);
						throw game_data_error(_("\"%s\" is duplicated"), style_name);
					} catch (Nonexistent) {
						m_anim_flag.push_back
							(std::pair<std::string, uint32_t>
							 	(style_name,
							 	 g_anim.get(path, *s, 0, &m_default_encdata)));
					}
				}
				if (m_anim_flag.empty())
					throw game_data_error(_("none found"));
			} catch (_wexception const & e) {
				throw game_data_error(_("flag styles: %s"), e.what());
			}

			if
				(Section * const inits_s =
				 	root_conf.get_section("initializations"))
				while (Section::Value const * const v = inits_s->get_next_val()) {
					m_initializations.resize(m_initializations.size() + 1);
					Initialization & init = m_initializations.back();
					init.    name = v->get_name  ();
					init.descname = v->get_string();
					try {
						for
							(Initialization const * i = &m_initializations.front();
							 i < &init;
							 ++i)
							if (i->name == init.name)
								throw game_data_error("duplicated");
						path += init.name;
						Profile init_prof(path.c_str());
						path.resize(base_path_size);
						while
							(Section * const event_s = init_prof.get_next_section())
						{
							char const * const event_name = event_s->get_name();
							Event * event;
							if      (event_s->get_string("type"))
								throw game_data_error("type key is not allowed");
							else if   (event_s->get_string("player"))
								throw game_data_error("player key is not allowed");
							else if   (event_s->get_string("point"))
								throw game_data_error("point key is not allowed");
							else if   (not strcmp(event_name, "allow_worker_types")) {
								event_s->set_int("version", 1);
								event =
									new Event_Allow_Worker_Types
										(*event_s, egbase, this);
							} else if (not strcmp(event_name, "forbid_worker_types"))
							{
								event_s->set_int("version", 1);
								event =
									new Event_Forbid_Worker_Types
										(*event_s, egbase, this);
							} else if (not strcmp(event_name, "allow_building_types"))
							{
								event_s->set_int("version", 3);
								event =
									new Event_Allow_Building_Types
										(*event_s, egbase, this);
							} else if
								(not strcmp(event_name, "forbid_building_types"))
							{
								event_s->set_int("version", 3);
								event =
									new Event_Forbid_Building_Types
										(*event_s, egbase, this);
							} else if
								(Building_Index const building =
								 	building_index(event_name))
							{
								event_s->set_int("version", 2);
								event =
									new Event_Building
										(*event_s, egbase, this, building);
							} else if (not strcmp(event_name, "conquer_area"))   {
								event_s->set_int("version", 2);
								event_s->set_string("point", "0 0");
								event = new Event_Conquer_Area(*event_s, egbase);
							} else if (not strcmp(event_name, "unhide_area"))    {
								event_s->set_int("version", 2);
								event_s->set_string("point", "0 0");
								event = new Event_Unhide_Area(*event_s, egbase);
							} else if
								(not strcmp(event_name, "seeall"))
							{
								event_s->set_int("version", 1);
								event =
									new Event_Player_See_All
										(*event_s, egbase);
							} else if
								(not strcmp(event_name, "set_player_frontier_style"))
							{
								event_s->set_int("version", 1);
								event =
									new Event_Set_Player_Frontier_Style
										(*event_s, egbase);
							} else if
								(not strcmp(event_name, "set_player_flag_style"))
							{
								event_s->set_int("version", 1);
								event =
									new Event_Set_Player_Flag_Style
										(*event_s, egbase);
							} else if
								(not strcmp(event_name, "allow_retreat_change"))
							{
								event_s->set_int("version", 1);
								event =
									new Event_Allow_Retreat_Change
										(*event_s, egbase);
							} else if
								(not strcmp(event_name, "forbid_retreat_change"))
							{
								event_s->set_int("version", 1);
								event =
									new Event_Forbid_Retreat_Change
										(*event_s, egbase);
							} else if
								(not strcmp(event_name, "retreat_change"))
							{
								event_s->set_int("version", 1);
								event =
									new Event_Retreat_Change(*event_s, egbase);
							} else
								throw game_data_error
									("\"%s\" is invalid as player initialization event "
									 "type for this tribe",
									 event_name);
							init.events.push_back(event);
						}
					} catch (_wexception const & e) {
						throw game_data_error
							("[initializations] \"%s=%s\": %s",
							 init.name.c_str(), v->get_string(), e.what());
					}
				}
		} catch (std::exception const & e) {
			throw game_data_error("root conf: %s", e.what());
		}
	} catch (_wexception const & e) {
		throw game_data_error(_("tribe %s: %s"), tribename.c_str(), e.what());
	}
#ifdef WRITE_GAME_DATA_AS_HTML
	if (g_options.pull_section("global").get_bool("write_HTML", false)) {
		m_ware_references     = new HTMLReferences[get_nrwares    ().value()];
		m_worker_references   = new HTMLReferences[get_nrworkers  ().value()];
		m_building_references = new HTMLReferences[get_nrbuildings().value()];
		writeHTMLBuildings(path);
		writeHTMLWorkers  (path);
		writeHTMLWares    (path);
		delete[] m_building_references;
		delete[] m_worker_references;
		delete[] m_ware_references;
	}
#endif
}


/*
===============
Load all logic data
===============
*/
void Tribe_Descr::postload(Editor_Game_Base &) {
	// TODO: move more loads to postload
}

/*
===============
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
 * does this tribe exist?
 */
bool Tribe_Descr::exists_tribe
	(std::string const & name, TribeBasicInfo * const info)
{
	std::string buf = "tribes/";
	buf            += name;
	buf            += "/conf";

	FileRead f;
	if (f.TryOpen(*g_fs, buf.c_str())) {
		if (info)
			try {
				Profile prof(buf.c_str());
				info->name = name;
				info->uiposition =
					prof.get_safe_section("tribe").get_int("uiposition", 0);
				Section & inits_s = prof.get_safe_section("initializations");
				while (Section::Value const * const v = inits_s.get_next_val())
					info->initializations.push_back
						(TribeBasicInfo::Initialization
						 	(v->get_name(), v->get_string()));
			} catch (_wexception const & e) {
				throw game_data_error
					("reading basic info for tribe \"%s\": %s",
					 name.c_str(), e.what());
			}

		return true;
	}

	return false;
}

struct TribeBasicComparator {
	bool operator()(TribeBasicInfo const & t1, TribeBasicInfo const & t2) {
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
		TribeBasicInfo info;
		if (Tribe_Descr::exists_tribe(pname->substr(7), &info))
			tribes.push_back(info);
	}

	std::sort(tribes.begin(), tribes.end(), TribeBasicComparator());
	container_iterate_const(std::vector<TribeBasicInfo>, tribes, i)
		target.push_back(i.current->name);
}


void Tribe_Descr::get_all_tribe_infos(std::vector<TribeBasicInfo> & tribes) {
	assert(tribes.empty());

	//  get all tribes
	filenameset_t m_tribes;
	g_fs->FindFiles("tribes", "*", &m_tribes);
	for
		(filenameset_t::iterator pname = m_tribes.begin();
		 pname != m_tribes.end();
		 ++pname)
	{
		TribeBasicInfo info;
		if (Tribe_Descr::exists_tribe(pname->substr(7), &info))
			tribes.push_back(info);
	}

	std::sort(tribes.begin(), tribes.end(), TribeBasicComparator());
}


/*
==============
Find the best matching indicator for the given amount.
==============
*/
uint32_t Tribe_Descr::get_resource_indicator
	(Resource_Descr const * const res, uint32_t const amount) const
{
	if (not res or not amount) {
		int32_t idx = get_immovable_index("resi_none");
		if (idx == -1)
			throw game_data_error
				("tribe %s does not declare a resource indicator resi_none!",
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
		throw game_data_error
			("tribe %s does not declare a resource indicator for resource %s",
			 name().c_str(),
			 res->name().c_str());

	uint32_t bestmatch =
		static_cast<uint32_t>
			((static_cast<float>(amount) / res->get_max_amount())
			 *
			 num_indicators);
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
		throw game_data_error
			("tribe %s does not define ware type \"%s\"",
			 name().c_str(), warename.c_str());
}
Ware_Index Tribe_Descr::safe_ware_index(const char * const warename) const {
	if (Ware_Index const result = ware_index(warename))
		return result;
	else
		throw game_data_error
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
		throw game_data_error
			("tribe %s does not define worker type \"%s\"",
			 name().c_str(), workername.c_str());
}
Ware_Index Tribe_Descr::safe_worker_index(const char * const workername) const {
	if (Ware_Index const result = worker_index(workername))
		return result;
	else
		throw game_data_error
			("tribe %s does not define worker type \"%s\"",
			 name().c_str(), workername);
}

/*
 * Return the given building or die trying
 */
Building_Index Tribe_Descr::safe_building_index
	(char const * const buildingname) const
{
	Building_Index const result = building_index(buildingname);

	if (not result)
		throw game_data_error
			("tribe %s does not define building type \"%s\"",
			 name().c_str(), buildingname);
	return result;
}

}
