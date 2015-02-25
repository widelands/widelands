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

#include <algorithm>
#include <iostream>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "io/fileread.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/carrier.h"
#include "logic/constructionsite.h"
#include "logic/dismantlesite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/immovable.h"
#include "logic/militarysite.h"
#include "logic/parse_map_object_types.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "logic/world/resource_description.h"
#include "logic/world/world.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"


using namespace std;

namespace Widelands {

TribeDescr::TribeDescr
	(const std::string & tribename, EditorGameBase & egbase)
	: m_name(tribename)
{
	std::string path = "data/tribes/";
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

			PARSE_MAP_OBJECT_TYPES_BEGIN("ship")
				m_bobs.add
					(new ShipDescr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("ware")
				m_wares.add
					(new WareDescr
					 	(*this, _name, _descname, path, prof, global_s));
			PARSE_MAP_OBJECT_TYPES_END;

			const World& world = egbase.world();

			PARSE_MAP_OBJECT_TYPES_BEGIN("immovable")
				m_immovables.add
					(new ImmovableDescr
					 	(_name, _descname, path, prof, global_s, this));
			PARSE_MAP_OBJECT_TYPES_END;

#define PARSE_SPECIAL_WORKER_TYPES(name, descr_type)                                               \
	PARSE_MAP_OBJECT_TYPES_BEGIN(name)                                                              \
	auto& worker_descr = *new descr_type(_name, _descname, path, prof, global_s, *this);      \
	WareIndex const worker_idx = m_workers.add(&worker_descr);                                     \
	if (worker_descr.is_buildable() && worker_descr.buildcost().empty())                            \
		m_worker_types_without_cost.push_back(worker_idx);                                           \
	PARSE_MAP_OBJECT_TYPES_END;

			PARSE_SPECIAL_WORKER_TYPES("carrier", CarrierDescr);
			PARSE_SPECIAL_WORKER_TYPES("soldier", SoldierDescr);

#define PARSE_WORKER_TYPES(name)                                                                   \
	PARSE_MAP_OBJECT_TYPES_BEGIN(name)                                                              \
	auto& worker_descr =                                                                      \
		*new WorkerDescr(MapObjectType::WORKER, _name, _descname, path, prof, global_s, *this);   \
	WareIndex const worker_idx = m_workers.add(&worker_descr);                                     \
	if (worker_descr.is_buildable() && worker_descr.buildcost().empty())                            \
		m_worker_types_without_cost.push_back(worker_idx);                                           \
	PARSE_MAP_OBJECT_TYPES_END;

			PARSE_WORKER_TYPES("worker");

			PARSE_MAP_OBJECT_TYPES_BEGIN("constructionsite")
				m_buildings.add
					(new ConstructionSiteDescr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;
			safe_building_index("constructionsite"); // Check that it is defined.

			PARSE_MAP_OBJECT_TYPES_BEGIN("dismantlesite")
				m_buildings.add
					(new DismantleSiteDescr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;
			safe_building_index("dismantlesite"); // Check that it is defined.

			PARSE_MAP_OBJECT_TYPES_BEGIN("warehouse")
				m_buildings.add
					(new WarehouseDescr
					 	(_name, _descname, path, prof, global_s, *this));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("productionsite")
				m_buildings.add(new ProductionSiteDescr(
					MapObjectType::PRODUCTIONSITE, _name, _descname, path, prof, global_s, *this, world));
			PARSE_MAP_OBJECT_TYPES_END;

			PARSE_MAP_OBJECT_TYPES_BEGIN("militarysite")
				m_buildings.add
					(new MilitarySiteDescr
					 	(_name, _descname, path, prof, global_s, *this, world));
			PARSE_MAP_OBJECT_TYPES_END;


			// global militarysites are in /data/global not in /tribes
			std::string temp                = path;
			std::string::size_type sizetemp = base_path_size;
			path           = "data/global/militarysites/";
			base_path_size = path.size();

			PARSE_MAP_OBJECT_TYPES_BEGIN("global militarysite")
				m_buildings.add
					(new MilitarySiteDescr
					 	(_name, _descname, path, prof, global_s, *this, world));
			PARSE_MAP_OBJECT_TYPES_END;

			// Reset path and base_path_size
			path           = temp;
			base_path_size = sizetemp;


			PARSE_MAP_OBJECT_TYPES_BEGIN("trainingsite")
				m_buildings.add
					(new TrainingSiteDescr
					 	(_name, _descname, path, prof, global_s, *this, world));
			PARSE_MAP_OBJECT_TYPES_END;

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
			vector<WareIndex> column; \
			std::string column_s = boost::copy_range<std::string>(*It1); \
			for (boost::split_iterator<string::iterator> It2 = \
				boost::make_split_iterator \
					(column_s, boost::token_finder(boost::is_any_of(","))); \
				It2 != boost::split_iterator<string::iterator>(); \
				++It2) { \
				std::string instance_name = boost::copy_range<std::string>(*It2); \
				boost::trim(instance_name); \
				WareIndex id = safe_##w##_index(instance_name); \
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
			(WareIndex id = 0; \
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

			{
				Section road_s = root_conf.get_safe_section("roads");
				const auto load_roads = [&road_s, this](
				   const std::string& prefix, std::vector<std::string>* images) {
					for (int i = 0; i < 99; ++i) {
						const char* img =
						   road_s.get_string((boost::format("%s_%02i") % prefix % i).str().c_str(), nullptr);
						if (img == nullptr) {
							break;
						}
						if (!g_fs->file_exists(img)) {
							throw new GameDataError("File %s for roadtype %s in tribe %s does not exist",
							                        img,
							                        prefix.c_str(),
							                        m_name.c_str());
						}
						images->emplace_back(img);
					}
					if (images->empty()) {
						throw new GameDataError(
						   "No %s roads defined in tribe %s.", prefix.c_str(), m_name.c_str());
					}
				};
				load_roads("normal", &m_normal_road_paths);
				load_roads("busy", &m_busy_road_paths);
			}

			m_frontier_animation_id =
			   g_gr->animations().load(path, root_conf.get_safe_section("frontier"));
			m_flag_animation_id =
			   g_gr->animations().load(path, root_conf.get_safe_section("flag"));

			{
				// Read initializations -- all scripts are initializations currently
				for (const std::string& script :
					  filter(g_fs->list_directory(path + "scripting"),
				            [](const string& fn) {return boost::ends_with(fn, ".lua");})) {
					std::unique_ptr<LuaTable> t = egbase.lua().run_script(script);
					t->do_not_warn_about_unaccessed_keys();

					m_initializations.resize(m_initializations.size() + 1);
					Initialization& init = m_initializations.back();
					init.script = script;
					init.descname = t->get_string("name");
				}
			}
		} catch (const std::exception & e) {
			throw GameDataError("root conf: %s", e.what());
		}
	} catch (const WException & e) {
		throw GameDataError("tribe %s: %s", tribename.c_str(), e.what());
	}
}


/*
===============
Load all logic data
===============
*/
void TribeDescr::postload(EditorGameBase &) {
	// TODO(unknown): move more loads to postload
}

/*
===============
Load tribe graphics
===============
*/
void TribeDescr::load_graphics()
{
	for (WareIndex i = 0; i < m_workers.get_nitems(); ++i)
		m_workers.get(i)->load_graphics();

	for (WareIndex i = 0; i < m_wares.get_nitems  (); ++i)
		m_wares.get(i)->load_graphics();

	for
		(BuildingIndex i = 0;
		 i < m_buildings.get_nitems();
		 ++i)
		m_buildings.get(i)->load_graphics();
}

const std::vector<std::string>& TribeDescr::normal_road_paths() const {
	return m_normal_road_paths;
}

const std::vector<std::string>& TribeDescr::busy_road_paths() const {
	return m_busy_road_paths;
}


void TribeDescr::add_normal_road_texture(std::unique_ptr<Texture> texture) {
	m_road_textures.add_normal_road_texture(std::move(texture));
}

void TribeDescr::add_busy_road_texture(std::unique_ptr<Texture> texture) {
	m_road_textures.add_busy_road_texture(std::move(texture));
}

const RoadTextures& TribeDescr::road_textures() const {
	return m_road_textures;
}


/*
 * does this tribe exist?
 */
bool TribeDescr::exists_tribe
	(const std::string & name, TribeBasicInfo * const info)
{
	std::string buf = "data/tribes/";
	buf            += name;
	buf            += "/conf";

	LuaInterface lua;
	FileRead f;
	if (f.try_open(*g_fs, buf)) {
		if (info)
			try {
				Profile prof(buf.c_str());
				info->name = name;
				info->uiposition =
					prof.get_safe_section("tribe").get_int("uiposition", 0);

				std::string path = "data/tribes/" + name + "/scripting";
				for (const std::string& script :
					  filter(g_fs->list_directory(path),
				            [](const string& fn) {return boost::ends_with(fn, ".lua");})) {
					std::unique_ptr<LuaTable> t = lua.run_script(script);
					t->do_not_warn_about_unaccessed_keys();
					info->initializations.push_back(
					   TribeBasicInfo::Initialization(script, t->get_string("name")));
				}
			} catch (const WException & e) {
				throw GameDataError
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
std::vector<std::string> TribeDescr::get_all_tribenames() {
	std::vector<std::string> tribenames;

	//  get all tribes
	std::vector<TribeBasicInfo> tribes;
	FilenameSet m_tribes = g_fs->list_directory("data/tribes");
	for
		(FilenameSet::iterator pname = m_tribes.begin();
		 pname != m_tribes.end();
		 ++pname)
	{
		TribeBasicInfo info;
		if (TribeDescr::exists_tribe(g_fs->fs_filename(pname->c_str()), &info))
			tribes.push_back(info);
	}

	std::sort(tribes.begin(), tribes.end(), TribeBasicComparator());
	for (const TribeBasicInfo& tribe : tribes) {
		tribenames.push_back(tribe.name);
	}
	return tribenames;
}


std::vector<TribeBasicInfo> TribeDescr::get_all_tribe_infos() {
	std::vector<TribeBasicInfo> tribes;

	//  get all tribes
	FilenameSet m_tribes = g_fs->list_directory("data/tribes");
	for
		(FilenameSet::iterator pname = m_tribes.begin();
		 pname != m_tribes.end();
		 ++pname)
	{
		TribeBasicInfo info;

		if (TribeDescr::exists_tribe(g_fs->fs_filename(pname->c_str()), &info))
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
uint32_t TribeDescr::get_resource_indicator
	(ResourceDescription const * const res, uint32_t const amount) const
{
	if (!res || !amount) {
		int32_t idx = get_immovable_index("resi_none");
		if (idx == -1)
			throw GameDataError
				("tribe %s does not declare a resource indicator resi_none!",
				 name().c_str());
		return idx;
	}

	int32_t i = 1;
	int32_t num_indicators = 0;
	for (;;) {
		const std::string resi_filename = (boost::format("resi_%s%i") % res->name().c_str() % i).str();
		if (get_immovable_index(resi_filename) == -1)
			break;
		++i;
		++num_indicators;
	}

	if (!num_indicators)
		throw GameDataError
			("tribe %s does not declare a resource indicator for resource %s",
			 name().c_str(),
			 res->name().c_str());

	int32_t bestmatch =
		static_cast<int32_t>
			((static_cast<float>(amount) / res->max_amount())
			 *
			 num_indicators);
	if (bestmatch > num_indicators)
		throw GameDataError
			("Amount of %s is %i but max amount is %i",
			 res->name().c_str(),
			 amount,
			 res->max_amount());
	if (static_cast<int32_t>(amount) < res->max_amount())
		bestmatch += 1; // Resi start with 1, not 0

	return get_immovable_index((boost::format("resi_%s%i")
										 % res->name().c_str()
										 % bestmatch).str());
}

/*
 * Return the given ware or die trying
 */
WareIndex TribeDescr::safe_ware_index(const std::string & warename) const {
	const WareIndex result = ware_index(warename);
	if (result == INVALID_INDEX) {
		throw GameDataError("tribe %s does not define ware type \"%s\"", name().c_str(), warename.c_str());
	}
	return result;
}

WareIndex TribeDescr::ware_index(const std::string & warename) const {
	WareIndex const wi = m_wares.get_index(warename);
	return wi;
}

/*
 * Return the given worker or die trying
 */
WareIndex TribeDescr::safe_worker_index(const std::string& workername) const {
const WareIndex result = worker_index(workername);
	if (result == INVALID_INDEX) {
		throw GameDataError(
		   "tribe %s does not define worker type \"%s\"", name().c_str(), workername.c_str());
	}
	return result;
}

/*
 * Return the given building or die trying
 */
BuildingIndex TribeDescr::safe_building_index(char const* const buildingname) const {
	const BuildingIndex result = building_index(buildingname);
	if (result == INVALID_INDEX) {
		throw GameDataError(
		   "tribe %s does not define building type \"%s\"", name().c_str(), buildingname);
	}
	return result;
}

void TribeDescr::resize_ware_orders(size_t maxLength) {
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
			new_wares_order.push_back(std::vector<Widelands::WareIndex>());
			for (std::vector<Widelands::WareIndex>::iterator it2 = it->begin(); it2 != it->end(); ++it2) {
				if (new_wares_order.rbegin()->size() >= maxLength) {
					new_wares_order.push_back(std::vector<Widelands::WareIndex>());
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
