/*
 * Copyright (C) 2016-2020 by the Widelands Development Team
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

#include <boost/algorithm/string.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "config.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/world.h"
#include "website/json/json.h"
#include "website/website_common.h"

using namespace Widelands;

namespace {

/*
 ==========================================================
 BUILDINGS
 ==========================================================
 */

void write_buildings(const TribeDescr& tribe, EditorGameBase& egbase, FileSystem* out_filesystem) {
	log_info_notimestamp("\n==================\nWriting buildings:\n==================\n");

	// We don't want any partially finished buildings
	std::vector<const BuildingDescr*> buildings;
	for (const DescriptionIndex& index : tribe.buildings()) {
		const BuildingDescr* building = tribe.get_building_descr(index);
		if (building->type() != MapObjectType::CONSTRUCTIONSITE &&
		    building->type() != MapObjectType::DISMANTLESITE) {
			buildings.push_back(building);
		}
	}

	std::unique_ptr<JSON::Element> json(new JSON::Element());
	JSON::Array* json_buildings_array = json->add_array("buildings");
	for (size_t i = 0; i < buildings.size(); ++i) {
		const BuildingDescr& building = *buildings[i];
		log_info_notimestamp(" %s", building.name().c_str());

		JSON::Object* json_building = json_buildings_array->add_object();
		json_building->add_string("name", building.name());
		json_building->add_string("descname", building.descname());
		json_building->add_string("icon", building.icon_filename());

		// Buildcost
		if (building.is_buildable()) {
			JSON::Array* json_builcost_array = json_building->add_array("buildcost");
			for (WareAmount buildcost : building.buildcost()) {
				const WareDescr& ware = *tribe.get_ware_descr(buildcost.first);
				JSON::Object* json_builcost = json_builcost_array->add_object();
				json_builcost->add_string("name", ware.name());
				json_builcost->add_int("amount", buildcost.second);
			}
		}

		if (building.is_enhanced()) {
			json_building->add_string(
			   "enhanced", tribe.get_building_descr(building.enhanced_from())->name());
		}

		if (building.enhancement() != INVALID_INDEX) {
			json_building->add_string(
			   "enhancement", tribe.get_building_descr(building.enhancement())->name());
		}

		if (upcast(ProductionSiteDescr const, productionsite, &building)) {
			// Produces wares
			if (productionsite->output_ware_types().size() > 0) {
				JSON::Array* json_wares_array = json_building->add_array("produced_wares");
				for (DescriptionIndex ware_index : productionsite->output_ware_types()) {
					json_wares_array->add_empty(tribe.get_ware_descr(ware_index)->name());
				}
			}
			// Produces workers
			if (productionsite->output_worker_types().size() > 0 ||
			    productionsite->type() == Widelands::MapObjectType::TRAININGSITE) {
				JSON::Array* json_workers_array = json_building->add_array("produced_workers");
				for (DescriptionIndex worker_index : productionsite->output_worker_types()) {
					json_workers_array->add_empty(tribe.get_worker_descr(worker_index)->name());
				}
				// Trainingsites
				if (productionsite->type() == Widelands::MapObjectType::TRAININGSITE) {
					json_workers_array->add_empty(tribe.get_worker_descr(tribe.soldier())->name());
				}
			}

			// Consumes
			if (productionsite->input_wares().size() > 0) {
				JSON::Array* json_wares_array = json_building->add_array("stored_wares");
				for (WareAmount input : productionsite->input_wares()) {
					const WareDescr& ware = *tribe.get_ware_descr(input.first);
					JSON::Object* json_input = json_wares_array->add_object();
					json_input->add_string("name", ware.name());
					json_input->add_int("amount", input.second);
				}
			}

			// Workers
			JSON::Array* json_workers_array = json_building->add_array("workers");
			for (WareAmount input : productionsite->working_positions()) {
				const WorkerDescr& worker = *tribe.get_worker_descr(input.first);
				JSON::Object* json_input = json_workers_array->add_object();
				json_input->add_string("name", worker.name());
				json_input->add_int("amount", input.second);
			}
		} else if (upcast(MilitarySiteDescr const, militarysite, &building)) {
			json_building->add_int("conquers", militarysite->get_conquers());
			json_building->add_int("max_soldiers", militarysite->get_max_number_of_soldiers());
			json_building->add_int("heal_per_second", militarysite->get_heal_per_second());
		}

		json_building->add_string("type", to_string(building.type()));

		// Size
		if (building.type() == MapObjectType::WAREHOUSE && !building.is_buildable() &&
		    !building.is_enhanced()) {
			json_building->add_string("size", "headquarters");
		} else if (building.get_ismine()) {
			json_building->add_string("size", "mine");
		} else if (building.get_isport()) {
			json_building->add_string("size", "port");
		} else {
			json_building->add_string("size", BaseImmovable::size_to_string(building.get_size()));
		}

		// Helptext
		try {
			std::unique_ptr<LuaTable> table(
			   egbase.lua().run_script("tribes/scripting/mapobject_info/building_helptext.lua"));
			std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
			cr->push_arg(building.helptext_script());
			cr->resume();
			const std::string help_text = cr->pop_string();
			json_building->add_string("helptext", help_text);
		} catch (LuaError& err) {
			json_building->add_string("helptext", err.what());
		}
	}

	json->write_to_file(
	   *out_filesystem, (boost::format("%s_buildings.json") % tribe.name()).str().c_str());
	log_info_notimestamp("\n");
}

/*
 ==========================================================
 WARES
 ==========================================================
 */

void write_wares(const TribeDescr& tribe, EditorGameBase& egbase, FileSystem* out_filesystem) {
	log_info_notimestamp("\n===============\nWriting wares:\n===============\n");

	std::unique_ptr<JSON::Element> json(new JSON::Element());
	JSON::Array* json_wares_array = json->add_array("wares");
	for (DescriptionIndex ware_index : tribe.wares()) {
		const WareDescr& ware = *tribe.get_ware_descr(ware_index);
		log_info_notimestamp(" %s", ware.name().c_str());
		JSON::Object* json_ware = json_wares_array->add_object();
		json_ware->add_string("name", ware.name());
		json_ware->add_string("descname", ware.descname());
		json_ware->add_string("icon", ware.icon_filename());

		// Helptext
		try {
			std::unique_ptr<LuaTable> table(
			   egbase.lua().run_script("tribes/scripting/mapobject_info/ware_helptext.lua"));
			std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
			cr->push_arg(tribe.name());
			cr->push_arg(ware.helptext_script());
			cr->resume();
			std::string help_text = cr->pop_string();
			boost::algorithm::trim(help_text);
			json_ware->add_string("helptext", help_text);
		} catch (LuaError& err) {
			json_ware->add_string("helptext", err.what());
		}
	}

	json->write_to_file(
	   *out_filesystem, (boost::format("%s_wares.json") % tribe.name()).str().c_str());
	log_info_notimestamp("\n");
}

/*
 ==========================================================
 WORKERS
 ==========================================================
 */

void write_workers(const TribeDescr& tribe, EditorGameBase& egbase, FileSystem* out_filesystem) {
	log_info_notimestamp("\n================\nWriting workers:\n================\n");

	std::unique_ptr<JSON::Element> json(new JSON::Element());
	JSON::Array* json_workers_array = json->add_array("workers");
	for (DescriptionIndex worker_index : tribe.workers()) {
		const WorkerDescr& worker = *tribe.get_worker_descr(worker_index);
		log_info_notimestamp(" %s", worker.name().c_str());
		JSON::Object* json_worker = json_workers_array->add_object();
		json_worker->add_string("name", worker.name());
		json_worker->add_string("descname", worker.descname());
		json_worker->add_string("icon", worker.icon_filename());

		// Helptext
		try {
			std::unique_ptr<LuaTable> table(
			   egbase.lua().run_script("tribes/scripting/mapobject_info/worker_helptext.lua"));
			std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
			cr->push_arg(worker.helptext_script());
			cr->resume();
			const std::string help_text = cr->pop_string();
			json_worker->add_string("helptext", help_text);
		} catch (LuaError& err) {
			json_worker->add_string("helptext", err.what());
		}

		if (worker.becomes() != INVALID_INDEX) {
			const WorkerDescr& becomes = *tribe.get_worker_descr(worker.becomes());
			JSON::Object* json_becomes = json_worker->add_object("becomes");
			json_becomes->add_string("name", becomes.name());
			json_becomes->add_int("experience", worker.get_needed_experience());
		}
	}

	json->write_to_file(
	   *out_filesystem, (boost::format("%s_workers.json") % tribe.name()).str().c_str());
	log_info_notimestamp("\n");
}

/*
 ==========================================================
 TRIBES
 ==========================================================
 */

void add_tribe_info(const Widelands::TribeBasicInfo& tribe_info, JSON::Element* json_tribe) {
	json_tribe->add_string("name", tribe_info.name);
	json_tribe->add_string("descname", tribe_info.descname);
	json_tribe->add_string("author", tribe_info.author);
	json_tribe->add_string("tooltip", tribe_info.tooltip);
	json_tribe->add_string("icon", tribe_info.icon);
}

void write_tribes(EditorGameBase& egbase, FileSystem* out_filesystem) {
	std::unique_ptr<JSON::Element> json(new JSON::Element());
	JSON::Array* json_tribes_array = json->add_array("tribes");

	/// Tribes
	const Tribes& tribes = egbase.tribes();

	std::vector<Widelands::TribeBasicInfo> tribeinfos = Widelands::get_all_tribeinfos();
	for (size_t tribe_index = 0; tribe_index < tribeinfos.size(); ++tribe_index) {
		const Widelands::TribeBasicInfo& tribe_info = tribeinfos[tribe_index];
		log_info_notimestamp("\n\n=========================\nWriting tribe: %s\n=========================\n",
		    tribe_info.name.c_str());

		// Main file
		JSON::Object* json_tribe = json_tribes_array->add_object();
		add_tribe_info(tribe_info, json_tribe);

		// These go in separate files
		std::unique_ptr<JSON::Object> json_tribe_for_file(new JSON::Object());
		add_tribe_info(tribe_info, json_tribe_for_file.get());
		json_tribe_for_file->write_to_file(
		   *out_filesystem, (boost::format("tribe_%s.json") % tribe_info.name).str().c_str());

		const TribeDescr& tribe = *tribes.get_tribe_descr(tribes.tribe_index(tribe_info.name));
		write_buildings(tribe, egbase, out_filesystem);
		write_wares(tribe, egbase, out_filesystem);
		write_workers(tribe, egbase, out_filesystem);
	}

	json->write_to_file(*out_filesystem, "tribes.json");
}

}  // namespace

/*
 ==========================================================
 MAIN
 ==========================================================
 */

int main(int argc, char** argv) {
	if (argc != 2) {
		log_err_notimestamp("Usage: %s <existing-output-path>\n", argv[0]);
		return 1;
	}

	const std::string output_path = argv[argc - 1];

	try {
		initialize();
		std::unique_ptr<FileSystem> out_filesystem(&FileSystem::create(output_path));
		EditorGameBase egbase(nullptr);
		egbase.load_all_tribes();
		write_tribes(egbase, out_filesystem.get());
	} catch (std::exception& e) {
		log_err_notimestamp("Exception: %s.\n", e.what());
		cleanup();
		return 1;
	}
	cleanup();
	return 0;
}
