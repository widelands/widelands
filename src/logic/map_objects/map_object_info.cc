/*
 * Copyright (C) 2016 by the Widelands Development Team
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

#include <memory>

#include <SDL.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "config.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/world.h"
#include "sound/sound_handler.h"

using namespace Widelands;

namespace  {

/*
 ==========================================================
 SETUP
 ==========================================================
 */

const std::string kDirectory = "map_object_info";

// Setup the static objects Widelands needs to operate and initializes systems.
void initialize() {
	i18n::set_locale("en");

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw wexception("Unable to initialize SDL: %s", SDL_GetError());
	}

	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));
	g_fs->ensure_directory_exists(kDirectory);

	// We don't really need graphics or sound here, but we will get error messages
	// when they aren't initialized
	g_gr = new Graphic();
	g_gr->initialize(Graphic::TraceGl::kNo, 1, 1, false);

	g_sound_handler.init();
	g_sound_handler.nosound_ = true;
}

/*
 ==========================================================
 SPECIALIZED FILEWRITE
 ==========================================================
 */

// Defines some convenience writing functions for the JSOn format
class JSONFileWrite : public FileWrite {
public:
	JSONFileWrite() : FileWrite(), level_(0) {}

	void write_string(const std::string& s, bool use_indent = false) {
		std::string writeme = s;
		if (use_indent) {
			for (int i = 0; i < level_; ++i) {
				writeme = (boost::format("   %s") % writeme).str();
			}
		}
		data(writeme.c_str(), writeme.size());
	}
	void write_key(const std::string& key) {
		write_string((boost::format("\"%s\":\n") % key).str(), true);
	}
	void write_value_string(const std::string& quoted_value) {
		write_string((boost::format("\"%s\"") % quoted_value).str(), true);
	}
	void write_key_value(const std::string& key, const std::string& quoted_value) {
		write_string((boost::format("\"%s\": %s") % key % quoted_value).str(), true);
	}
	void write_key_value_string(const std::string& key, const std::string& value) {
		std::string quoted_value = value;
		boost::replace_all(quoted_value, "\"", "\\\"");
		write_key_value(key, "\"" + value + "\"");
	}
	void write_key_value_int(const std::string& key, const int value) {
		write_key_value(key, boost::lexical_cast<std::string>(value));
	}
	void open_brace() {
		write_string("{\n", true);
		++level_;
	}
	// JSON hates a final comma. This defaults to having NO comma.
	void close_brace(bool precede_newline = false, int current = 0, int total = 0) {
		--level_;
		if (precede_newline) {
			write_string("\n");
		}
		if (current < total - 1) {
			write_string("},\n", true);
		} else {
			write_string("}", true);
		}
	}
	void open_array(const std::string& name) {
		write_string((boost::format("\"%s\":[\n") % name).str(), true);
		++level_;
	}
	// JSON hates a final comma. This defaults to having NO comma.
	void close_array(int current = 0, int total = 0) {
		--level_;
		write_string("\n");
		if (current < total - 1) {
			write_string("],\n", true);
		} else {
			write_string("]\n", true);
		}
	}
	// JSON hates a final comma. This defaults to having a comma.
	void close_element(int current = -2, int total = 0) {
		if (current < total - 1) {
			write_string(",\n");
		}
	}
private:
	int level_;
};

/*
 ==========================================================
 BUILDINGS
 ==========================================================
 */

void write_buildings(const TribeDescr& tribe, EditorGameBase& egbase) {

	log("\n==================\nWriting buildings:\n==================\n");
	JSONFileWrite fw;
	fw.open_brace(); // Main
	fw.open_array("buildings"); // Buildings

	// We don't want any partially finished buildings
	std::vector<const BuildingDescr*> buildings;
	for (const DescriptionIndex& index : tribe.buildings()) {
		const BuildingDescr* building = tribe.get_building_descr(index);
		if (building->type() != MapObjectType::CONSTRUCTIONSITE &&
			 building->type() != MapObjectType::DISMANTLESITE) {
			buildings.push_back(building);
		}
	}

	// Now write
	for (size_t i = 0; i < buildings.size(); ++i) {
		const BuildingDescr& building = *buildings[i];
		log(" %s", building.name().c_str());
		fw.open_brace(); // Building

		fw.write_key_value_string("name", building.name());
		fw.close_element();
		fw.write_key_value_string("descname", building.descname());
		fw.close_element();
		fw.write_key_value_string("icon", building.representative_image_filename());
		fw.close_element();

		// Conditional stuff in between, so we won't run into trouble with the commas.

		// Buildcost
		if (building.is_buildable()) {
			fw.open_array("buildcost"); // Buildcost
			size_t buildcost_counter = 0;
			for (WareAmount buildcost : building.buildcost()) {
				const WareDescr& ware = *tribe.get_ware_descr(buildcost.first);
				fw.open_brace(); // Buildcost
				fw.write_key_value_string("name", ware.name());
				fw.close_element();
				fw.write_key_value_int("amount", buildcost.second);
				fw.close_brace(true, buildcost_counter, building.buildcost().size()); // Buildcost
				++buildcost_counter;
			}
			fw.close_array(1, 5); // Buildcost - we need a comma
		}

		if (building.is_enhanced()) {
			fw.write_key_value_string("enhanced", tribe.get_building_descr(building.enhanced_from())->name());
			fw.close_element();
		}

		if (upcast(ProductionSiteDescr const, productionsite, &building)) {
			// Produces
			if (productionsite->output_ware_types().size() > 0) {
				fw.open_array("produced_wares"); // Produces
				size_t produces_counter = 0;
				for (DescriptionIndex ware_index : productionsite->output_ware_types()) {
					const WareDescr& ware = *tribe.get_ware_descr(ware_index);
					fw.write_value_string(ware.name());
					fw.close_element(produces_counter, productionsite->output_ware_types().size());
					++produces_counter;
				}
				fw.close_array(1, 5); // Produces - we need a comma
			}
			if (productionsite->output_worker_types().size() > 0) {
				fw.open_array("produced_workers"); // Produces
				size_t produces_counter = 0;
				for (DescriptionIndex worker_index : productionsite->output_worker_types()) {
					const WorkerDescr& worker = *tribe.get_worker_descr(worker_index);
					fw.write_value_string(worker.name());
					fw.close_element(produces_counter, productionsite->output_worker_types().size());
					++produces_counter;
				}
				fw.close_array(1, 5); // Produces - we need a comma
			}

			// Consumes
			if (productionsite->inputs().size() > 0) {
				fw.open_array("stored_wares"); // Consumes
				size_t consumes_counter = 0;
				for (WareAmount input : productionsite->inputs()) {
					const WareDescr& ware = *tribe.get_ware_descr(input.first);
					fw.open_brace(); // Input
					fw.write_key_value_string("name", ware.name());
					fw.close_element();
					fw.write_key_value_int("amount", input.second);
					fw.close_brace(true, consumes_counter, productionsite->inputs().size()); // Input
					++consumes_counter;
				}
				fw.close_array(1, 5); // Consumes - we need a comma
			}

			fw.open_array("workers"); // Workers
			size_t worker_counter = 0;
			for (WareAmount input : productionsite->working_positions()) {
				const WorkerDescr& worker = *tribe.get_worker_descr(input.first);
				fw.open_brace(); // Worker
				fw.write_key_value_string("name", worker.name());
				fw.close_element();
				fw.write_key_value_int("amount", input.second);
				fw.close_brace(true, worker_counter, productionsite->working_positions().size()); // Worker
				++worker_counter;
			}
			fw.close_array(1, 5); // Workers - we need a comma
		}

		// Type NOCOM do we need this?
		switch (building.type()) {
		case MapObjectType::PRODUCTIONSITE:
			fw.write_key_value_string("type", "productionsite");
			break;
		case MapObjectType::WAREHOUSE:
			fw.write_key_value_string("type", "warehouse");
			break;
		case MapObjectType::MILITARYSITE:
			fw.write_key_value_string("type", "militarysite");
			break;
		case MapObjectType::TRAININGSITE:
			fw.write_key_value_string("type", "trainingsite");
			break;
		default:
			NEVER_HERE();
		}
		fw.close_element();

		// Size
		if (building.type() == MapObjectType::WAREHOUSE &&
			 !building.is_buildable() && !building.is_enhanced()) {
				fw.write_key_value_string("size", "headquarters");
		} else if (building.get_ismine()) {
			fw.write_key_value_string("size", "mine");
		} else if (building.get_isport()) {
			fw.write_key_value_string("size", "port");
		} else {
			fw.write_key_value_string("size", BaseImmovable::size_to_string(building.get_size()));
		}
		fw.close_element();

		// Helptext
		try {
			std::unique_ptr<LuaTable> table(
				egbase.lua().run_script("tribes/scripting/mapobject_info/building_helptext.lua"));
			std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
			cr->push_arg(&building);
			cr->resume();
			const std::string help_text = cr->pop_string();
			fw.write_key_value_string("helptext", help_text);
		} catch (LuaError& err) {
			fw.write_key_value_string("helptext", err.what());
		}

		fw.close_brace(true, i, buildings.size()); // Building
	}
	fw.close_array(); // Buildings
	fw.close_brace(); // Main
	fw.write(*g_fs, (boost::format("%s/%s_buildings.json") % kDirectory % tribe.name()).str().c_str());
	log("\n");
}

/*
 ==========================================================
 WARES
 ==========================================================
 */

void write_wares(const TribeDescr& tribe, EditorGameBase& egbase) {
	log("\n===============\nWriting wares:\n===============\n");
	JSONFileWrite fw;
	fw.open_brace(); // Main

	fw.open_array("wares"); // Wares
	size_t counter = 0;
	const size_t no_of_wares = tribe.wares().size();
	for (DescriptionIndex ware_index : tribe.wares()) {
		const WareDescr& ware = *tribe.get_ware_descr(ware_index);
		log(" %s", ware.name().c_str());
		fw.open_brace();
		fw.write_key_value_string("name", ware.name());
		fw.close_element();
		fw.write_key_value_string("descname", ware.descname());
		fw.close_element();
		fw.write_key_value_string("icon", ware.icon_filename());
		fw.close_element();

		// Helptext
		try {
			std::unique_ptr<LuaTable> table(
				egbase.lua().run_script("tribes/scripting/mapobject_info/ware_helptext.lua"));
			std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
			cr->push_arg(tribe.name());
			cr->push_arg(&ware);
			cr->resume();
			const std::string help_text = cr->pop_string();
			fw.write_key_value_string("helptext", help_text);
		} catch (LuaError& err) {
			fw.write_key_value_string("helptext", err.what());
		}
		fw.close_element();

		fw.open_array("producers"); // Producers
		size_t prod_counter = 0;
		const size_t no_of_producers = ware.producers().size();
		for (DescriptionIndex building_index : ware.producers()) {
			const BuildingDescr& building = *tribe.get_building_descr(building_index);
			fw.open_brace();
			fw.write_key_value_string("name", building.name());
			fw.close_element();
			fw.write_key_value_string("descname", building.descname());
			fw.close_element();
			fw.write_key_value_string("icon", building.icon_filename());
			fw.close_element();
			fw.write_key_value_string("representative_image", building.representative_image_filename());
			fw.close_brace(true, prod_counter, no_of_producers); // Building
			++prod_counter;
		}
		fw.close_array(1, 5); // Producers - and we need a comma

		fw.open_array("consumers"); // Consumers
		size_t consumer_counter = 0;
		const size_t no_of_consumers = ware.consumers().size();
		for (DescriptionIndex building_index : ware.consumers()) {
			const BuildingDescr& building = *tribe.get_building_descr(building_index);
			fw.open_brace();
			fw.write_key_value_string("name", building.name());
			fw.close_element();
			fw.write_key_value_string("descname", building.descname());
			fw.close_element();
			fw.write_key_value_string("icon", building.icon_filename());
			fw.close_element();
			fw.write_key_value_string("representative_image", building.representative_image_filename());
			fw.close_brace(true, consumer_counter, no_of_consumers); // Building
			++consumer_counter;
		}
		fw.close_array(); // Consumers

		fw.close_brace(false, counter, no_of_wares); // Ware
		++counter;
	}
	fw.close_array(); // Wares

	fw.close_brace(); // Main
	fw.write(*g_fs, (boost::format("%s/%s_wares.json") % kDirectory % tribe.name()).str().c_str());
	log("\n");
}

/*
 ==========================================================
 WORKERS
 ==========================================================
 */

void write_workers(const TribeDescr& tribe, EditorGameBase& egbase) {
	log("\n================\nWriting workers:\n================\n");
	JSONFileWrite fw;
	fw.open_brace(); // Main

	fw.open_array("workers"); // Workers
	size_t counter = 0;
	const size_t no_of_workers = tribe.workers().size();
	for (DescriptionIndex worker_index : tribe.workers()) {
		const WorkerDescr& worker = *tribe.get_worker_descr(worker_index);
		log(" %s", worker.name().c_str());
		fw.open_brace();
		fw.write_key_value_string("name", worker.name());
		fw.close_element();
		fw.write_key_value_string("descname", worker.descname());
		fw.close_element();
		fw.write_key_value_string("icon", worker.icon_filename());
		fw.close_element();

		// Helptext
		try {
			std::unique_ptr<LuaTable> table(
				egbase.lua().run_script("tribes/scripting/mapobject_info/worker_helptext.lua"));
			std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
			cr->push_arg(&worker);
			cr->resume();
			const std::string help_text = cr->pop_string();
			fw.write_key_value_string("helptext", help_text);
		} catch (LuaError& err) {
			fw.write_key_value_string("helptext", err.what());
		}

		if (worker.becomes() != INVALID_INDEX) {
			fw.close_element();
			const WorkerDescr& becomes = *tribe.get_worker_descr(worker.becomes());
			fw.write_key("becomes");
			fw.open_brace();
			fw.write_key_value_string("name", becomes.name());
			fw.close_element();
			fw.write_key_value_string("descname", worker.descname());
			fw.close_element();
			fw.write_key_value_string("icon", worker.icon_filename());
			fw.close_brace(true);
		}
		fw.close_brace(true, counter, no_of_workers); // Worker
		++counter;
	}
	fw.close_array(); // Workers

	fw.close_brace(); // Main
	fw.write(*g_fs, (boost::format("%s/%s_workers.json") % kDirectory % tribe.name()).str().c_str());
	log("\n");
}

/*
 ==========================================================
 TRIBES
 ==========================================================
 */

void add_tribe_info(const TribeBasicInfo& tribe_info, JSONFileWrite* fw) {
		fw->write_key_value_string("name", tribe_info.name);
		fw->close_element();
		fw->write_key_value_string("descname", tribe_info.descname);
		fw->close_element();
		fw->write_key_value_string("author", tribe_info.author);
		fw->close_element();
		fw->write_key_value_string("tooltip", tribe_info.tooltip);
		fw->close_element();
		fw->write_key_value_string("icon", tribe_info.icon);
}

void write_tribes(EditorGameBase& egbase) {
	JSONFileWrite fw;
	fw.open_brace(); // Main
	fw.open_array("tribes"); // Tribes

	/// Tribes
	egbase.mutable_tribes()->postload(); // Make sure that all values have been set.
	const Tribes& tribes = egbase.tribes();

	std::vector<TribeBasicInfo> tribeinfos = tribes.get_all_tribeinfos();
	for (size_t tribe_index = 0; tribe_index < tribeinfos.size(); ++tribe_index) {
		const TribeBasicInfo& tribe_info = tribeinfos[tribe_index];
		log("\n\n=========================\nWriting tribe: %s\n=========================\n",
			 tribe_info.name.c_str());

		fw.open_brace(); // TribeDescr
		add_tribe_info(tribe_info, &fw);
		fw.close_brace(true, tribe_index, tribeinfos.size()); // TribeDescr

		 // These go in separate files

		JSONFileWrite fw_tribe;
		fw_tribe.open_brace(); // TribeDescr
		add_tribe_info(tribe_info, &fw_tribe);
		fw_tribe.close_brace(true); // TribeDescr
		fw_tribe.write(*g_fs, (boost::format("%s/tribe_%s.json") % kDirectory % tribe_info.name).str().c_str());

		const TribeDescr& tribe =
				*tribes.get_tribe_descr(tribes.tribe_index(tribe_info.name));

		write_buildings(tribe, egbase);
		write_wares(tribe, egbase);
		write_workers(tribe, egbase);
	}
	fw.close_array(); // Tribes
	fw.close_brace(); // Main
	fw.write(*g_fs, (boost::format("%s/tribes.json") % kDirectory).str().c_str());
}

}  // namespace

/*
 ==========================================================
 MAIN
 ==========================================================
 */

int main()
{

	try {

		initialize();

		EditorGameBase egbase(nullptr);
		// NOCOM const World& world = egbase.world();
		write_tribes(egbase);
	}
	catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		g_sound_handler.shutdown();
		return 1;
	}
	g_sound_handler.shutdown();
	return 0;
}
