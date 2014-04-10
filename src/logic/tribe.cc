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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/tribe.h"

#include <iostream>

#include <boost/algorithm/string.hpp>

#include "graphic/graphic.h"
#include "helper.h"
#include "i18n.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/carrier.h"
#include "logic/constructionsite.h"
#include "logic/critter_bob.h"
#include "logic/dismantlesite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/immovable.h"
#include "logic/militarysite.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/warehouse.h"
#include "logic/widelands_fileread.h"
#include "logic/worker.h"
#include "logic/world.h"
#include "parse_map_object_types.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "upcast.h"


using namespace std;

namespace Widelands {

Tribe_Descr::Tribe_Descr
	(const std::string & tribename, Editor_Game_Base & egbase)
	: m_name(tribename), m_world(egbase.map().world())
{
	assert(&m_world);
	std::string path = "tribes/";
	try {
		path            += tribename;

		// Grab the localization textdomain.
		// 'path' must be without final /
		i18n::Textdomain textdomain(std::string("tribe_") + tribename);

		path            += '/';
		std::string::size_type base_path_size = path.size();

		path += "conf";
		Profile root_conf(path.c_str());
		path.resize(base_path_size);

		{
			std::set<std::string> names; //  To enforce name uniqueness.

			PARSE_MAP_OBJECT_TYPES_BEGIN("critter bob")
				m_bobs.add
					(new Critter_Bob_Descr
					 	(_name, _descname, path, prof, global_s,  this));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("ship")
				m_bobs.add
					(new Ship_Descr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("ware")
				m_wares.add
					(new WareDescr
					 	(*this, _name, _descname, path, prof, global_s));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("immovable")
				m_immovables.add
					(new Immovable_Descr
					 	(_name, _descname, path, prof, global_s, m_world, this));
			PARSE_MAP_OBJECT_TYPES_END;

#define PARSE_WORKER_TYPES(name, descr_type)                                  \
         PARSE_MAP_OBJECT_TYPES_BEGIN(name)                                   \
            descr_type & worker_descr =                                       \
               *new descr_type                                                \
                  (_name, _descname, path, prof, global_s, *this);            \
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
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;
			if (not safe_building_index("constructionsite"))
				throw game_data_error
					("constructionsite type \"constructionsite\" is missing");

			PARSE_MAP_OBJECT_TYPES_BEGIN("dismantlesite")
				m_buildings.add
					(new DismantleSite_Descr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;
			if (not safe_building_index("dismantlesite"))
				throw game_data_error
					("dismantlesite type \"distmantlesite\" is missing");

			PARSE_MAP_OBJECT_TYPES_BEGIN("warehouse")
				m_buildings.add
					(new Warehouse_Descr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("productionsite")
				m_buildings.add
					(new ProductionSite_Descr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("militarysite")
				m_buildings.add
					(new MilitarySite_Descr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;


			// global militarysites are in /global not in /tribes
			std::string temp                = path;
			std::string::size_type sizetemp = base_path_size;
			path           = "global/militarysites/";
			base_path_size = path.size();

			PARSE_MAP_OBJECT_TYPES_BEGIN("global militarysite")
				m_buildings.add
					(new MilitarySite_Descr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;

			// Reset path and base_path_size
			path           = temp;
			base_path_size = sizetemp;


			PARSE_MAP_OBJECT_TYPES_BEGIN("trainingsite")
				m_buildings.add
					(new TrainingSite_Descr
					 	(_name, _descname, path, prof, global_s, *this));
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

				/// Load and parse ware and worker categorization
#define PARSE_ORDER_INFORMATION(w) /* w is ware or worker */ \
	{ \
		m_##w##s_order_coords.resize(m_##w##s.get_nitems()); \
\
		std::string categories_s \
			(tribe_s.get_safe_string(#w "s_order")); \
		for (boost::split_iterator<string::iterator> It1 = \
			boost::make_split_iterator \
				(categories_s, boost::token_finder(boost::is_any_of(";"))); \
			It1 != boost::split_iterator<string::iterator>(); \
			++It1) { \
			vector<Ware_Index> column; \
			std::string column_s = boost::copy_range<std::string>(*It1); \
			for (boost::split_iterator<string::iterator> It2 = \
				boost::make_split_iterator \
					(column_s, boost::token_finder(boost::is_any_of(","))); \
				It2 != boost::split_iterator<string::iterator>(); \
				++It2) { \
				std::string instance_name = boost::copy_range<std::string>(*It2); \
				boost::trim(instance_name); \
				Ware_Index id = safe_##w##_index(instance_name); \
				column.push_back(id); \
				/* it has been added to the column, but the column not */ \
				/* yet to the array */ \
				m_ ## w ## s_order_coords[id] = std::pair<uint32_t, uint32_t> \
					(m_ ## w ## s_order.size(), column.size() - 1); \
			} \
			if (!column.empty()) m_##w##s_order.push_back(column); \
		} \
\
		/* Check that every ##w## has been added */ \
		for \
			(Ware_Index id = Ware_Index::First(); \
			 id < m_##w##s.get_nitems(); ++id) { \
			if (id != m_ ## w ## s_order[m_ ## w ## s_order_coords[id].first] \
					[m_##w##s_order_coords[id].second]) { \
				log("Didn't find " #w " %s in " #w "s_order field of tribe %s!\n", \
					  get_##w##_descr(id)->name().c_str(), name().c_str()); \
			} \
		} \
	}

				PARSE_ORDER_INFORMATION(ware);
				PARSE_ORDER_INFORMATION(worker);
			}

			try {
				while (Section * const s = root_conf.get_next_section("frontier"))
				{
					char const * const style_name = s->get_safe_string("name");
					try {
						if (m_anim_frontier.empty())
							throw Nonexistent();
						frontier_style_index(style_name);
						throw game_data_error("\"%s\" is duplicated", style_name);
					} catch (Nonexistent) {
						m_anim_frontier.push_back
							(std::pair<std::string, uint32_t>
							 	(style_name, g_gr->animations().load(path, *s)));
					}
				}
				if (m_anim_frontier.empty())
					throw game_data_error("none found");
			} catch (const _wexception & e) {
				throw game_data_error("frontier styles: %s", e.what());
			}
			try {
				while (Section * const s = root_conf.get_next_section("flag"))
				{
					char const * const style_name = s->get_safe_string("name");
					try {
						if (m_anim_flag.empty())
							throw Nonexistent();
						flag_style_index(style_name);
						throw game_data_error("\"%s\" is duplicated", style_name);
					} catch (Nonexistent) {
						m_anim_flag.push_back
							(std::pair<std::string, uint32_t>
							 	(style_name,
							 	 g_gr->animations().load(path, *s)));
					}
				}
				if (m_anim_flag.empty())
					throw game_data_error("none found");
			} catch (const _wexception & e) {
				throw game_data_error("flag styles: %s", e.what());
			}

			{
				// Read initializations -- all scripts are initializations currently
				for (const std::string& script :
				     filter(g_fs->ListDirectory(path + "scripting"),
				            [](const string& fn) {return boost::ends_with(fn, ".lua");})) {
					std::unique_ptr<LuaTable> t = egbase.lua().run_script(script);

					m_initializations.resize(m_initializations.size() + 1);
					Initialization& init = m_initializations.back();
					init.script = script;
					init.descname = t->get_string("name");
				}
			}
		} catch (const std::exception & e) {
			throw game_data_error("root conf: %s", e.what());
		}
	} catch (const _wexception & e) {
		throw game_data_error("tribe %s: %s", tribename.c_str(), e.what());
	}
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
	(const std::string & name, TribeBasicInfo * const info)
{
	std::string buf = "tribes/";
	buf            += name;
	buf            += "/conf";

	LuaInterface lua;
	FileRead f;
	if (f.TryOpen(*g_fs, buf.c_str())) {
		if (info)
			try {
				Profile prof(buf.c_str());
				info->name = name;
				info->uiposition =
					prof.get_safe_section("tribe").get_int("uiposition", 0);

				std::string path = "tribes/" + name + "/scripting";
				for (const std::string& script :
				     filter(g_fs->ListDirectory(path),
				            [](const string& fn) {return boost::ends_with(fn, ".lua");})) {
					std::unique_ptr<LuaTable> t = lua.run_script(script);
					info->initializations.push_back(
					   TribeBasicInfo::Initialization(script, t->get_string("name")));
				}
			} catch (const _wexception & e) {
				throw game_data_error
					("reading basic info for tribe \"%s\": %s",
					 name.c_str(), e.what());
			}
		return true;
	}
	return false;
}

struct TribeBasicComparator {
	bool operator()(const TribeBasicInfo & t1, const TribeBasicInfo & t2) {
		return t1.uiposition < t2.uiposition;
	}
};

/**
 * Fills the given string vector with the names of all tribes that exist.
 */
std::vector<std::string> Tribe_Descr::get_all_tribenames() {
	std::vector<std::string> tribenames;

	//  get all tribes
	std::vector<TribeBasicInfo> tribes;
	filenameset_t m_tribes = g_fs->ListDirectory("tribes");
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
		tribenames.push_back(i.current->name);
	return tribenames;
}


std::vector<TribeBasicInfo> Tribe_Descr::get_all_tribe_infos() {
	std::vector<TribeBasicInfo> tribes;

	//  get all tribes
	filenameset_t m_tribes = g_fs->ListDirectory("tribes");
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
	return tribes;
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

	int32_t bestmatch =
		static_cast<int32_t>
			((static_cast<float>(amount) / res->get_max_amount())
			 *
			 num_indicators);
	if (bestmatch > num_indicators)
		throw game_data_error
			("Amount of %s is %i but max amount is %i",
			 res->name().c_str(),
			 amount,
			 res->get_max_amount());
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
Ware_Index Tribe_Descr::safe_ware_index(const std::string & warename) const {
	if (Ware_Index const result = ware_index(warename))
		return result;
	else
		throw game_data_error("tribe %s does not define ware type \"%s\"", name().c_str(), warename.c_str());
}
Ware_Index Tribe_Descr::safe_ware_index(const char * const warename) const {
	if (Ware_Index const result = ware_index(warename))
		return result;
	else
		throw game_data_error("tribe %s does not define ware type \"%s\"", name().c_str(), warename);
}

Ware_Index Tribe_Descr::ware_index(const std::string & warename) const {
	Ware_Index const wi = m_wares.get_index(warename);
	return wi;
}
Ware_Index Tribe_Descr::ware_index(char const * const warename) const {
	Ware_Index const wi = m_wares.get_index(warename);
	return wi;
}


/*
 * Return the given worker or die trying
 */
Ware_Index Tribe_Descr::safe_worker_index(const std::string & workername) const
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

void Tribe_Descr::resize_ware_orders(size_t maxLength) {
	bool need_resize = false;

	//check if we actually need to resize
	for (WaresOrder::iterator it = m_wares_order.begin(); it != m_wares_order.end(); ++it) {
		if (it->size() > maxLength) {
			need_resize = true;
		  }
	 }

	//resize
	if (need_resize) {

		//build new smaller wares_order
		WaresOrder new_wares_order;
		for (WaresOrder::iterator it = m_wares_order.begin(); it != m_wares_order.end(); ++it) {
			new_wares_order.push_back(std::vector<Widelands::Ware_Index>());
			for (std::vector<Widelands::Ware_Index>::iterator it2 = it->begin(); it2 != it->end(); ++it2) {
				if (new_wares_order.rbegin()->size() >= maxLength) {
					new_wares_order.push_back(std::vector<Widelands::Ware_Index>());
				}
				new_wares_order.rbegin()->push_back(*it2);
				m_wares_order_coords[*it2].first = new_wares_order.size() - 1;
				m_wares_order_coords[*it2].second = new_wares_order.rbegin()->size() - 1;
			}
		}

		//remove old array
		m_wares_order.clear();
		m_wares_order = new_wares_order;
	}
}

}
