/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/tribes/productionsite.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/ferry_fleet.h"
#include "economy/input_queue.h"
#include "economy/request.h"
#include "economy/ship_fleet.h"
#include "economy/ware_instance.h"
#include "economy/wares_queue.h"
#include "economy/workers_queue.h"
#include "graphic/style_manager.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/player.h"

namespace Widelands {

namespace {

// Parses the descriptions of the working positions from 'items_table' and
// fills in 'working_positions'. Throws an error if the table contains invalid
// values.
void parse_working_positions(const Descriptions& descriptions,
                             LuaTable* items_table,
                             BillOfMaterials* working_positions) {
	for (const std::string& worker_name : items_table->keys<std::string>()) {
		int amount = items_table->get_int(worker_name);
		try {
			if (amount < 1 || 255 < amount) {
				throw GameDataError("count is out of range 1 .. 255");
			}
			// Try to load the worker if an object with this name has been registered
			Notifications::publish(
			   NoteMapObjectDescription(worker_name, NoteMapObjectDescription::LoadType::kObject));

			// Ensure that we did indeed load a worker
			DescriptionIndex const woi = descriptions.worker_index(worker_name);
			if (!descriptions.worker_exists(woi)) {
				throw GameDataError("not a worker");
			}
			working_positions->push_back(std::make_pair(woi, amount));
		} catch (const WException& e) {
			throw GameDataError("%s=\"%d\": %s", worker_name.c_str(), amount, e.what());
		}
	}
}
}  // namespace

/*
==============================================================================

ProductionSite BUILDING

==============================================================================
*/

/**
 * The contents of 'table' are documented in
 * /data/tribes/buildings/productionsites/atlanteans/armorsmithy/init.lua
 */
ProductionSiteDescr::ProductionSiteDescr(const std::string& init_descname,
                                         MapObjectType init_type,
                                         const LuaTable& table,
                                         Descriptions& descriptions)
   : BuildingDescr(init_descname, init_type, table, descriptions),
     ware_demand_checks_(new std::set<DescriptionIndex>()),
     worker_demand_checks_(new std::set<DescriptionIndex>()),
     is_infinite_production_useful_(false),
     out_of_resource_productivity_threshold_(100) {

	std::unique_ptr<LuaTable> items_table;

	if (table.has_key("out_of_resource_notification")) {
		items_table = table.get_table("out_of_resource_notification");
		out_of_resource_title_ = items_table->get_string("title");
		out_of_resource_heading_ = items_table->get_string("heading");
		out_of_resource_message_ = items_table->get_string("message");
		if (items_table->has_key("productivity_threshold")) {
			out_of_resource_productivity_threshold_ = items_table->get_int("productivity_threshold");
		}
	}
	if (table.has_key("resource_not_needed_message")) {
		resource_not_needed_message_ = _(table.get_string("resource_not_needed_message"));
	}

	if (table.has_key("outputs")) {
		log_warn(
		   "The \"outputs\" table is no longer needed; you can remove it from %s\n", name().c_str());
	}

	if (table.has_key("inputs")) {
		std::vector<std::unique_ptr<LuaTable>> input_entries =
		   table.get_table("inputs")->array_entries<std::unique_ptr<LuaTable>>();
		for (std::unique_ptr<LuaTable>& entry_table : input_entries) {
			const std::string& ware_or_worker_name = entry_table->get_string("name");
			int amount = entry_table->get_int("amount");
			try {
				if (amount < 1 || 255 < amount) {
					throw GameDataError("amount is out of range 1 .. 255");
				}
				// Check if ware/worker exists already and if not, try to load it. Will throw a
				// GameDataError on failure.
				const std::pair<WareWorker, DescriptionIndex> wareworker =
				   descriptions.load_ware_or_worker(ware_or_worker_name);
				switch (wareworker.first) {
				case WareWorker::wwWARE: {
					for (const auto& temp_inputs : input_wares()) {
						if (temp_inputs.first == wareworker.second) {
							throw GameDataError(
							   "ware type '%s' was declared multiple times", ware_or_worker_name.c_str());
						}
					}
					input_wares_.push_back(WareAmount(wareworker.second, amount));
				} break;
				case WareWorker::wwWORKER: {
					for (const auto& temp_inputs : input_workers()) {
						if (temp_inputs.first == wareworker.second) {
							throw GameDataError("worker type '%s' was declared multiple times",
							                    ware_or_worker_name.c_str());
						}
					}
					input_workers_.push_back(WareAmount(wareworker.second, amount));
				} break;
				}
			} catch (const std::exception& e) {
				throw GameDataError(
				   "input \"%s=%d\": %s", ware_or_worker_name.c_str(), amount, e.what());
			}
		}
	}

	parse_working_positions(
	   descriptions, table.get_table("working_positions").get(), &working_positions_);

	// Get programs
	items_table = table.get_table("programs");
	for (std::string program_name : items_table->keys<std::string>()) {
		program_name = to_lower(program_name);
		if (programs_.count(program_name) != 0u) {
			throw GameDataError("Program '%s' has already been declared for productionsite '%s'",
			                    program_name.c_str(), name().c_str());
		}
		try {
			std::unique_ptr<LuaTable> program_table = items_table->get_table(program_name);

			if (program_name == "work") {
				log_warn("The main program for the building %s should be renamed from 'work' to "
				         "'main'",
				         name().c_str());
				programs_[MapObjectProgram::kMainProgram] =
				   std::unique_ptr<ProductionProgram>(new ProductionProgram(
				      MapObjectProgram::kMainProgram, *program_table, descriptions, this));
			} else {
				programs_[program_name] = std::unique_ptr<ProductionProgram>(
				   new ProductionProgram(program_name, *program_table, descriptions, this));
			}
		} catch (const std::exception& e) {
			throw GameDataError("%s: Error in productionsite program %s: %s", name().c_str(),
			                    program_name.c_str(), e.what());
		}
	}

	if (init_type == MapObjectType::PRODUCTIONSITE &&
	    programs_.count(MapObjectProgram::kMainProgram) == 0) {
		throw GameDataError(
		   "%s: Error in productionsite programs: no 'main' program defined", name().c_str());
	}

	// Check ActCall
	for (const auto& caller : programs_) {
		caller.second->validate_calls(*this);
	}

	if (table.has_key("indicate_workarea_overlaps")) {
		log_warn("The \"indicate_workarea_overlaps\" table in %s has been deprecated and can be "
		         "removed.\n",
		         name().c_str());
	}
}

ProductionSiteDescr::ProductionSiteDescr(const std::string& init_descname,
                                         const LuaTable& table,
                                         Descriptions& descriptions)
   : ProductionSiteDescr(init_descname, MapObjectType::PRODUCTIONSITE, table, descriptions) {
}

/**
 * Get the program of the given name.
 */
const ProductionProgram* ProductionSiteDescr::get_program(const std::string& program_name) const {
	Programs::const_iterator const it = programs().find(program_name);
	if (it == programs_.end()) {
		throw wexception("%s has no program '%s'", name().c_str(), program_name.c_str());
	}
	return it->second.get();
}

bool ProductionSiteDescr::highlight_overlapping_workarea_for(const std::string& productionsite,
                                                             bool* positive) const {
	if (competes_with_productionsite(productionsite)) {
		*positive = false;
		return true;
	}
	if (supports_productionsite(productionsite) || is_supported_by_productionsite(productionsite)) {
		*positive = true;
		return true;
	}
	return false;
}

void ProductionSiteDescr::add_competing_productionsite(const std::string& productionsite) {
	competing_productionsites_.insert(productionsite);
}
void ProductionSiteDescr::add_supports_productionsite(const std::string& productionsite) {
	supported_productionsites_.insert(productionsite);
}
void ProductionSiteDescr::add_supported_by_productionsite(const std::string& productionsite) {
	supported_by_productionsites_.insert(productionsite);
}
bool ProductionSiteDescr::competes_with_productionsite(const std::string& productionsite) const {
	return competing_productionsites_.count(productionsite) == 1;
}
bool ProductionSiteDescr::supports_productionsite(const std::string& productionsite) const {
	return supported_productionsites_.count(productionsite) == 1;
}
bool ProductionSiteDescr::is_supported_by_productionsite(const std::string& productionsite) const {
	return supported_by_productionsites_.count(productionsite) == 1;
}

/**
 * Create a new building of this type
 */
Building& ProductionSiteDescr::create_object() const {
	return *new ProductionSite(*this);
}

std::set<DescriptionIndex>* ProductionSiteDescr::ware_demand_checks() const {
	return ware_demand_checks_.get();
}
std::set<DescriptionIndex>* ProductionSiteDescr::worker_demand_checks() const {
	return worker_demand_checks_.get();
}
void ProductionSiteDescr::clear_demand_checks() {
	ware_demand_checks_->clear();
	ware_demand_checks_.reset(nullptr);
	worker_demand_checks_->clear();
	worker_demand_checks_.reset(nullptr);
}

/*
==============================

IMPLEMENTATION

==============================
*/

ProductionSite::ProductionSite(const ProductionSiteDescr& ps_descr)
   : Building(ps_descr), working_positions_(ps_descr.nr_working_positions()) {
	format_statistics_string();

	if (descr().has_ship_fleet_check() || descr().has_ferry_fleet_check()) {
		field_terrain_changed_subscriber_ = Notifications::subscribe<NoteFieldTerrainChanged>(
		   [this](const NoteFieldTerrainChanged& note) {
			   if (note.action == NoteFieldTerrainChanged::Change::kTerrain &&
			       owner().egbase().map().calc_distance(note.fc, get_position()) <=
			          descr().workarea_info().rbegin()->first + 1) {
				   init_yard_interfaces(get_owner()->egbase());
			   }
		   });
	}
}

void ProductionSite::load_finish(EditorGameBase& egbase) {
	Building::load_finish(egbase);
	format_statistics_string();
}

void ProductionSite::postload(EditorGameBase& egbase) {
	Building::postload(egbase);

	// TODO(Nordfriese): This is only needed for v1.1 savegame compatibility
	if ((descr().has_ship_fleet_check() && ship_fleet_interfaces_.empty()) ||
	    (descr().has_ferry_fleet_check() && ferry_fleet_interfaces_.empty())) {
		init_yard_interfaces(egbase);
	}
}

/**
 * Display whether we're occupied.
 */
void ProductionSite::update_statistics_string(std::string* s) {
	uint32_t nr_xp_requests = 0;
	uint32_t nr_requests = 0;
	uint32_t nr_coming = 0;

	auto worker_descr_it = descr().working_positions().begin();
	uint32_t worker_descr_it_counter = worker_descr_it->second;

	for (uint32_t i = 0; i < descr().nr_working_positions(); ++i) {
		const Widelands::Request* request = working_positions_.at(i).worker_request;
		// Check whether a request is being fulfilled or not
		bool vacant = false;
		if (request != nullptr) {
			if (request->is_open()) {
				vacant = true;
			} else {
				++nr_coming;
			}
		} else if (working_positions_.at(i).worker == nullptr) {
			// We might have no request, but no worker either
			vacant = true;
		}

		if (vacant) {
			// Check if this worker can be built directly
			const WorkerDescr& d =
			   *owner().egbase().descriptions().get_worker_descr(worker_descr_it->first);
			if (d.is_buildable() || d.has_demand_check()) {
				++nr_requests;
			} else {
				++nr_xp_requests;
			}
		}

		--worker_descr_it_counter;
		if (worker_descr_it_counter == 0) {
			++worker_descr_it;
			if (worker_descr_it != descr().working_positions().end()) {
				worker_descr_it_counter = worker_descr_it->second;
			}
		}
	}

	if (nr_xp_requests > 0) {
		*s = StyleManager::color_tag(
		   (nr_xp_requests == 1 ?
             owner().tribe().get_productionsite_experienced_worker_missing_string() :
             owner().tribe().get_productionsite_experienced_workers_missing_string()),
		   g_style_manager->building_statistics_style().low_color());
		return;
	}

	if (nr_requests > 0) {
		*s = StyleManager::color_tag(
		   (nr_requests == 1 ? owner().tribe().get_productionsite_worker_missing_string() :
                             owner().tribe().get_productionsite_workers_missing_string()),
		   g_style_manager->building_statistics_style().low_color());
		return;
	}

	if (nr_coming > 0) {
		*s = StyleManager::color_tag(
		   (nr_coming == 1 ? owner().tribe().get_productionsite_worker_coming_string() :
                           owner().tribe().get_productionsite_workers_coming_string()),
		   g_style_manager->building_statistics_style().medium_color());
		return;
	}

	if (is_stopped_) {
		*s = StyleManager::color_tag(
		   _("(stopped)"), g_style_manager->building_statistics_style().neutral_color());
		return;
	}
	*s = statistics_string_on_changed_statistics_;
}

/**
 * Detect if the workers are experienced enough for an target building
 * Buildable workers are skipped, but upgraded ones (required be target site) are tested
 * @param idx Index of the enhancement
 */
bool ProductionSite::has_workers(DescriptionIndex targetSite, Game& game) {
	// bld holds the description of the building we want to have
	if (upcast(ProductionSiteDescr const, bld, owner().tribe().get_building_descr(targetSite))) {

		if (bld->nr_working_positions() != 0u) {

			// Iterating over workers positions in target building
			for (const auto& wp : bld->working_positions()) {

				// If worker for this position is buildable, just skip him
				if (owner().tribe().get_worker_descr(wp.first)->is_buildable()) {
					continue;
				}

				// This position needs promoted worker, so trying to find out if there is such worker
				// currently available in this site
				const DescriptionIndex needed_worker = wp.first;
				bool worker_available = false;
				for (unsigned int i = 0; i < descr().nr_working_positions(); ++i) {
					const Worker* cw = working_positions()->at(i).worker.get(game);
					if (cw != nullptr) {
						DescriptionIndex current_worker = cw->descr().worker_index();
						if (owner().tribe().get_worker_descr(current_worker)->can_act_as(needed_worker)) {
							worker_available = true;  // We found a worker for the position
							break;
						}
					}
				}
				if (!worker_available) {
					// We dont have needed workers in the site :(
					return false;
				}
			}

			// If we are here, all needs are satisfied
			return true;
		}
		throw wexception("Building, index: %d, needs no workers!\n", targetSite);
	}
	throw wexception("No such building, index: %d\n", targetSite);
}

InputQueue& ProductionSite::inputqueue(DescriptionIndex const wi,
                                       WareWorker const type,
                                       const Request* /* req */) {
	for (InputQueue* ip_queue : input_queues_) {
		if (ip_queue->get_index() == wi && ip_queue->get_type() == type) {
			return *ip_queue;
		}
	}
	if (!(owner().tribe().has_ware(wi) || owner().tribe().has_worker(wi))) {
		throw wexception("%s (%u) has no InputQueue for unknown %s %u", descr().name().c_str(),
		                 serial(), type == WareWorker::wwWARE ? "ware" : "worker", wi);
	}
	throw wexception("%s (%u) has no InputQueue for %s %u: %s", descr().name().c_str(), serial(),
	                 type == WareWorker::wwWARE ? "ware" : "worker", wi,
	                 type == WareWorker::wwWARE ?
                       owner().tribe().get_ware_descr(wi)->name().c_str() :
                       owner().tribe().get_worker_descr(wi)->name().c_str());
}

/**
 * Calculate statistic.
 */
void ProductionSite::format_statistics_string() {
	// TODO(sirver): this method does too much: it formats the actual statistics
	// as a string and persists them into a string for reuse when the class is
	// asked for the statistics string. However this string should only then be constructed.

	// format would treat uint8_t as char
	const unsigned int percent = std::min(get_actual_statistics() * 100 / 98, 100);
	const std::string perc_str = StyleManager::color_tag(
	   format(_("%i%%"), percent),
	   (percent < 33) ? g_style_manager->building_statistics_style().low_color() :
	   (percent < 66) ? g_style_manager->building_statistics_style().medium_color() :
                       g_style_manager->building_statistics_style().high_color());

	if (0 < percent && percent < 100) {
		RGBColor color = g_style_manager->building_statistics_style().high_color();
		std::string trend;
		if (last_stat_percent_ < actual_percent_) {
			trend_ = Trend::kRising;
			color = g_style_manager->building_statistics_style().high_color();
			trend = "+";
		} else if (last_stat_percent_ > actual_percent_) {
			trend_ = Trend::kFalling;
			color = g_style_manager->building_statistics_style().low_color();
			trend = "-";
		} else {
			trend_ = Trend::kUnchanged;
			color = g_style_manager->building_statistics_style().neutral_color();
			trend = "=";
		}

		// TODO(GunChleoc): We might need to reverse the order here for RTL languages
		statistics_string_on_changed_statistics_ =
		   format("%s\u2009%s", perc_str, StyleManager::color_tag(trend, color));
	} else {
		statistics_string_on_changed_statistics_ = perc_str;
	}
	last_stat_percent_ = actual_percent_;
}

/**
 * Initialize the production site.
 */
bool ProductionSite::init(EditorGameBase& egbase) {
	Building::init(egbase);

	const BillOfMaterials& input_wares = descr().input_wares();
	const BillOfMaterials& input_workers = descr().input_workers();
	input_queues_.resize(input_wares.size() + input_workers.size());

	size_t i = 0;
	for (const WareAmount& pair : input_wares) {
		input_queues_[i++] = new WaresQueue(*this, pair.first, pair.second);
	}
	for (const WareAmount& pair : input_workers) {
		input_queues_[i++] = new WorkersQueue(*this, pair.first, pair.second);
	}

	//  Request missing workers.
	auto wp = working_positions_.begin();
	for (const auto& temp_wp : descr().working_positions()) {
		DescriptionIndex const worker_index = temp_wp.first;
		for (uint32_t j = temp_wp.second; j != 0u; --j, ++wp) {
			if (Worker* const worker = wp->worker.get(egbase)) {
				worker->set_location(this);
			} else {
				wp->worker_request = &request_worker(worker_index);
			}
		}
	}

	init_yard_interfaces(egbase);

	if (upcast(Game, game, &egbase)) {
		try_start_working(*game);
	}
	return true;
}

/**
 * Change the economy for the wares queues.
 *
 * \note Workers are dealt with in the PlayerImmovable code.
 */
void ProductionSite::set_economy(Economy* const e, WareWorker type) {
	if (Economy* const old = get_economy(type)) {
		for (InputQueue* ip_queue : input_queues_) {
			if (ip_queue->get_type() == type) {
				ip_queue->remove_from_economy(*old);
			}
		}
	}

	Building::set_economy(e, type);
	for (uint32_t i = descr().nr_working_positions(); i != 0u;) {
		if (Request* const r = working_positions_.at(--i).worker_request) {
			if (r->get_type() == type) {
				r->set_economy(e);
			}
		}
	}

	if (e != nullptr) {
		for (InputQueue* ip_queue : input_queues_) {
			if (ip_queue->get_type() == type) {
				ip_queue->add_to_economy(*e);
			}
		}
	}
}

/**
 * Cleanup after a production site is removed
 */
void ProductionSite::cleanup(EditorGameBase& egbase) {
	for (uint32_t i = descr().nr_working_positions(); i != 0u;) {
		--i;
		delete working_positions_.at(i).worker_request;
		working_positions_.at(i).worker_request = nullptr;
		Worker* const w = working_positions_.at(i).worker.get(egbase);

		//  Ensure we do not re-request the worker when remove_worker is called.
		working_positions_.at(i).worker = nullptr;

		// Actually remove the worker
		if (w != nullptr) {
			w->set_location(nullptr);
		}
	}

	// Cleanup the wares queues
	for (InputQueue* iq : input_queues_) {
		iq->cleanup();
		delete iq;
	}
	input_queues_.clear();

	while (!ship_fleet_interfaces_.empty()) {
		ship_fleet_interfaces_.front()->remove(egbase);
	}
	while (!ferry_fleet_interfaces_.empty()) {
		ferry_fleet_interfaces_.front()->remove(egbase);
	}

	Building::cleanup(egbase);
}

/**
 * Create a new worker inside of us out of thin air
 *
 * returns true on success and false if there is no room for this worker
 */
bool ProductionSite::warp_worker(EditorGameBase& egbase, const WorkerDescr& wdes, int32_t slot) {
	auto current = working_positions_.begin();
	for (int i = descr().nr_working_positions(); i != 0; i--, ++current) {
		if (slot < 0) {
			if (current->worker.get(egbase) != nullptr) {
				continue;
			}
			assert(current->worker_request);
			if (current->worker_request->get_index() != wdes.worker_index()) {
				continue;
			}
		} else if (slot > 0) {
			--slot;
			continue;
		} else {
			assert(!current->worker.get(egbase));
			assert(current->worker_request);
			assert(wdes.can_act_as(current->worker_request->get_index()));
		}

		// Okay, space is free and worker is fitting. Let's create him
		Worker& worker = wdes.create(egbase, get_owner(), this, get_position());

		if (upcast(Game, game, &egbase)) {
			worker.start_task_idle(*game, 0, -1);
		}
		current->worker = &worker;
		delete current->worker_request;
		current->worker_request = nullptr;

		if (upcast(Game, game, &egbase)) {
			try_start_working(*game);
		}
		return true;
	}
	return false;
}

/**
 * Locate the worker slot at which the given worker is currently employed.
 * @return Index in descr().working_positions() (slot requirement) and working_positions_ (current
 *         slot), or (-1, -1) if the worker does not work here.
 */
std::pair<int32_t, int32_t> ProductionSite::find_worker(OPtr<Worker> w) {
	auto wp = working_positions_.begin();
	const BillOfMaterials& bom = descr().working_positions();
	int32_t wp_index = 0;
	int32_t bom_index = 0;
	for (auto temp_wp = bom.begin(); temp_wp != bom.end(); ++temp_wp, ++bom_index) {
		for (uint32_t j = temp_wp->second; j != 0u; --j, ++wp, ++wp_index) {
			if (wp->worker == w) {
				return std::pair<int32_t, int32_t>(bom_index, wp_index);
			}
		}
	}
	return std::pair<int32_t, int32_t>(-1, -1);
}

/**
 * Tries to replace a new worker request with a worker already present on the site
 * @param game: Game reference to retrieve description info
 * @param worker_index: The required worker type
 * @param wp: Pointer to the slot to be filled (contains the worker request)
 */
void ProductionSite::try_replace_worker(const Game* game,
                                        DescriptionIndex worker_index,
                                        WorkingPosition* wp) {
	// Search replacement worker in the same building
	for (auto& wp_repl : working_positions_) {
		if (!wp_repl.worker.is_set()) {
			continue;
		}
		std::pair<int32_t, int32_t> w_pair = find_worker(wp_repl.worker);
		const DescriptionIndex worker_index_repl = descr().working_positions().at(w_pair.first).first;

		// Only move workers into higher qualified jobs
		const WorkerDescr& wd = game->descriptions().workers().get(worker_index);
		bool needs_higher_qualification =
		   worker_index_repl != worker_index && wd.can_act_as(worker_index_repl);

		Worker* w_repl = wp_repl.worker.get(*game);
		assert(w_repl != nullptr);
		bool is_over_qualified = w_repl->descr().worker_index() != worker_index_repl;
		if (is_over_qualified && needs_higher_qualification &&
		    w_repl->descr().can_act_as(worker_index)) {
			// Move worker to evicted slot
			delete wp->worker_request;
			*wp = wp_repl;
			molog(owner().egbase().get_gametime(), "%s promoted\n", w_repl->descr().name().c_str());
			// Request the now missing worker instead and loop again
			wp_repl = WorkingPosition(&request_worker(worker_index_repl), nullptr);
			try_replace_worker(game, worker_index_repl, &wp_repl);
			return;
		}
	}
}

/**
 * Intercept remove_worker() calls to unassign our worker, if necessary.
 */
void ProductionSite::remove_worker(Worker& w) {
	molog(owner().egbase().get_gametime(), "%s leaving\n", w.descr().name().c_str());

	std::pair<int32_t, int32_t> wp_indexes = find_worker(OPtr<Worker>(&w));
	if (wp_indexes.first < 0) {
		Building::remove_worker(w);
		return;
	}

	DescriptionIndex const worker_index = descr().working_positions().at(wp_indexes.first).first;
	WorkingPosition* wp = &working_positions_.at(wp_indexes.second);
	// do not request the type of worker that is currently assigned – maybe a
	// trained worker was evicted to make place for a level 0 worker.
	// Therefore we again request the worker from the WorkingPosition of descr()
	if (main_worker_ == wp_indexes.second) {
		main_worker_ = -1;
	}
	*wp = WorkingPosition(&request_worker(worker_index), nullptr);
	Building::remove_worker(w);
	// If the main worker was evicted, perhaps another worker is
	// still there to perform basic tasks
	if (upcast(Game, game, &get_owner()->egbase())) {
		try_replace_worker(game, worker_index, wp);
		try_start_working(*game);
	}
}

/**
 * Issue the worker requests
 */
Request& ProductionSite::request_worker(DescriptionIndex const wareid) {
	return *new Request(*this, wareid, ProductionSite::request_worker_callback, wwWORKER);
}

/**
 * Called when our worker arrives.
 */
void ProductionSite::request_worker_callback(
   Game& game, Request& rq, DescriptionIndex /* widx */, Worker* const w, PlayerImmovable& target) {
	ProductionSite& psite = dynamic_cast<ProductionSite&>(target);

	assert(w);
	assert(w->get_location(game) == &psite);

	// If there is more than one working position, it's possible, that different level workers are
	// requested and therefor possible, that a higher qualified worker answers a request for a lower
	// leveled worker, although a worker with equal level (as the arrived worker has) is needed as
	// well.
	// Therefor, we first check whether the worker exactly fits the requested one. If yes, we place
	// the
	// worker and everything is fine, else we shuffle through the working positions, whether one of
	// them
	// needs a worker like the one just arrived. That way it is of course still possible, that the
	// worker is
	// placed on the slot that originally requested the arrived worker.
	bool worker_placed = false;
	DescriptionIndex idx = w->descr().worker_index();
	for (WorkingPosition& wp : psite.working_positions_) {
		if (wp.worker_request == &rq) {
			if (wp.worker_request->get_index() == idx) {
				// Place worker
				delete &rq;
				wp = WorkingPosition(nullptr, w);
				worker_placed = true;
			} else {
				// Set new request for this slot
				DescriptionIndex workerid = wp.worker_request->get_index();
				delete wp.worker_request;
				wp.worker_request = &psite.request_worker(workerid);
			}
			break;
		}
	}
	while (!worker_placed) {
		{
			uint8_t nwp = psite.descr().nr_working_positions();
			uint8_t pos = 0;
			auto wp = psite.working_positions_.begin();
			for (; pos < nwp; ++wp, ++pos) {
				// Find a fitting slot
				if ((wp->worker.get(game) == nullptr) && !worker_placed) {
					if (wp->worker_request->get_index() == idx) {
						delete wp->worker_request;
						*wp = WorkingPosition(nullptr, w);
						worker_placed = true;
						break;
					}
				}
			}
		}
		if (!worker_placed) {
			// Find the next smaller version of this worker
			DescriptionIndex nuwo = game.descriptions().nr_workers();
			DescriptionIndex current = 0;
			for (; current < nuwo; ++current) {
				WorkerDescr const* worker = game.descriptions().get_worker_descr(current);
				if (worker->becomes() == idx) {
					idx = current;
					break;
				}
			}
			if (current == nuwo) {
				throw wexception(
				   "Something went wrong! No fitting place for worker %s in %s at (%u, %u) found!",
				   w->descr().name().c_str(), psite.descr().name().c_str(), psite.get_position().x,
				   psite.get_position().y);
			}
		}
	}

	// It's always the first worker doing building work,
	// the others only idle. Still, we need to wake up the
	// primary worker if the worker that has just arrived is
	// the last one we need to start working.
	w->start_task_idle(game, 0, -1);
	psite.try_start_working(game);
	Notifications::publish(NoteBuilding(psite.serial(), NoteBuilding::Action::kWorkersChanged));
}

/**
 * Advance the program state if applicable.
 */
void ProductionSite::act(Game& game, uint32_t const data) {
	Building::act(game, data);

	if (program_timer_ && game.get_gametime() >= program_time_) {
		program_timer_ = false;

		if (!can_start_working()) {
			start_animation(game, descr().get_unoccupied_animation());
			while (!stack_.empty()) {
				program_end(game, ProgramResult::kFailed);
			}
		} else {
			assert(main_worker_ >= 0);
			if (stack_.empty()) {
				working_positions_.at(main_worker_).worker.get(game)->update_task_buildingwork(game);
				return;
			}

			State& state = top_state();
			if (state.program->size() <= state.ip) {
				return program_end(game, ProgramResult::kCompleted);
			}

			if (anim_ != descr().get_animation(default_anim_, this)) {
				// Restart idle animation, which is the default
				start_animation(game, descr().get_animation(default_anim_, this));
			}

			return program_act(game);
		}
	}
}

void ProductionSite::set_next_program_override(Game& game,
                                               const std::string& name,
                                               MapObject* extra_data) {
	program_start(game, name, true, extra_data);
}

bool ProductionSite::has_forced_state() const {
	return std::any_of(stack_.begin(), stack_.end(), [](const State& s) {
		return (s.flags & State::StateFlags::kStateFlagIgnoreStopped) != 0;
	});
}

void ProductionSite::find_and_start_next_program(Game& game) {
	program_start(game, MapObjectProgram::kMainProgram);
}

/**
 * Perform the current program action.
 *
 * \pre The program is running and in a valid state.
 * \post (Potentially indirect) scheduling for the next step has been done.
 */
void ProductionSite::program_act(Game& game) {
	const State& state = top_state();

	// 'Stop' of building is considered only when starting
	// new productions cycle. Otherwise it can lead to consumption
	// of input wares without producing anything
	if (is_stopped_ && state.ip == 0 &&
	    ((state.flags & State::StateFlags::kStateFlagIgnoreStopped) == 0u)) {
		program_end(game, ProgramResult::kFailed);
		program_timer_ = true;
		program_time_ = schedule_act(game, Duration(20000));
	} else {
		(*state.program)[state.ip].execute(game, *this);
	}
}

/**
 * Remember that we need to fetch an ware from the flag.
 */
bool ProductionSite::fetch_from_flag(Game& game) {
	++fetchfromflag_;

	if (main_worker_ >= 0) {
		assert(working_positions_.at(main_worker_).worker.get(game));
		working_positions_.at(main_worker_).worker.get(game)->update_task_buildingwork(game);
	}

	return true;
}

void ProductionSite::log_general_info(const EditorGameBase& egbase) const {
	Building::log_general_info(egbase);

	molog(egbase.get_gametime(), "is_stopped: %u\n", static_cast<int>(is_stopped_));
	molog(egbase.get_gametime(), "main_worker: %i\n", main_worker_);
}

void ProductionSite::set_stopped(bool const stopped) {
	is_stopped_ = stopped;
	get_economy(wwWARE)->rebalance_supply();
	get_economy(wwWORKER)->rebalance_supply();
	Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kChanged));
}

void ProductionSite::set_infinite_production(const bool i) {
	infinite_production_ = i;
	Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kChanged));
}

/**
 * \return True if this production site could theoretically start working (if
 * all workers are present)
 */
bool ProductionSite::can_start_working() const {
	for (uint32_t i = descr().nr_working_positions(); i != 0u;) {
		if (working_positions_.at(--i).worker_request != nullptr) {
			return false;
		}
	}
	return true;
}

void ProductionSite::try_start_working(Game& game) {
	const size_t nr_workers = descr().nr_working_positions();
	for (uint32_t i = 0; i < nr_workers; ++i) {
		if (main_worker_ == static_cast<int>(i) || main_worker_ < 0) {
			if (Worker* worker = working_positions_.at(i).worker.get(game)) {
				// We may start even if can_start_working() returns false, because basic actions
				// like unloading extra wares should take place anyway
				main_worker_ = i;
				worker->reset_tasks(game);
				worker->start_task_buildingwork(game);
				return;
			}
		}
	}
}

/**
 * There's currently nothing to do for the worker.
 *
 * \note We assume that the worker is inside the building when this is called.
 */
bool ProductionSite::get_building_work(Game& game, Worker& worker, bool const success) {
	assert(!descr().working_positions().empty());
	assert(main_worker_ >= 0);
	assert(&worker == working_positions_.at(main_worker_).worker.get(game));

	// If unsuccessful: Check if we need to abort current program
	if (!success) {
		State* state = get_state();
		if (state->ip < state->program->size()) {
			(*state->program)[state->ip].building_work_failed(game, *this, worker);
		}
	}

	// Default actions first
	if (WareInstance* const ware = worker.fetch_carried_ware(game)) {
		worker.start_task_dropoff(game, *ware);
		return true;
	}

	if (fetchfromflag_ != 0) {
		--fetchfromflag_;
		worker.start_task_fetchfromflag(game);
		return true;
	}

	if (!produced_wares_.empty()) {
		//  There is still a produced ware waiting for delivery. Carry it out
		//  before continuing with the program.
		WareAmount& ware_type_with_count = *produced_wares_.rbegin();
		{
			DescriptionIndex const ware_index = ware_type_with_count.first;
			const WareDescr& ware_ware_descr =
			   *owner().tribe().get_ware_descr(ware_type_with_count.first);
			{
				WareInstance& ware = *new WareInstance(ware_index, &ware_ware_descr);
				ware.init(game);
				worker.start_task_dropoff(game, ware);
			}
			get_owner()->ware_produced(ware_index);  //  for statistics
		}
		assert(ware_type_with_count.second);
		if (--ware_type_with_count.second == 0) {
			produced_wares_.pop_back();
		}
		return true;
	}

	if (!recruited_workers_.empty()) {
		//  There is still a recruited worker waiting to be released. Send it
		//  out.
		WareAmount& worker_type_with_count = *recruited_workers_.rbegin();
		{
			const WorkerDescr& worker_descr =
			   *owner().tribe().get_worker_descr(worker_type_with_count.first);
			{
				Worker& recruit = dynamic_cast<Worker&>(worker_descr.create_object());
				recruit.set_owner(worker.get_owner());
				recruit.set_position(game, worker.get_position());
				recruit.init(game);
				recruit.set_location(this);
				recruit.start_task_leavebuilding(game, true);
				worker.start_task_releaserecruit(game, recruit);
			}
		}
		assert(worker_type_with_count.second);
		if (--worker_type_with_count.second == 0) {
			recruited_workers_.pop_back();
		}
		return true;
	}

	// Drop all the wares that are too much out to the flag.
	// Input-workers are coming out by themselves
	for (InputQueue* queue : input_queues_) {
		if (queue->get_type() == wwWARE && queue->get_filled() > queue->get_max_fill()) {
			queue->set_filled(queue->get_filled() - 1);
			const WareDescr& wd = *owner().tribe().get_ware_descr(queue->get_index());
			WareInstance& ware = *new WareInstance(queue->get_index(), &wd);
			ware.init(game);
			worker.start_task_dropoff(game, ware);
			return true;
		}
	}

	// Check if all workers are there
	if (!can_start_working()) {
		// Try again a bit later
		worker.start_task_idle(game, 0, 3000);
		return true;
	}

	// Start program if we haven't already done so
	State* state = get_state();
	if (state == nullptr) {
		find_and_start_next_program(game);
	} else if (state->ip < state->program->size()) {
		const ProductionProgram::Action& action = (*state->program)[state->ip];
		return action.get_building_work(game, *this, worker);
	}

	return false;
}

/**
 * Advance the program to the next step.
 */
void ProductionSite::program_step(Game& game, const Duration& delay, ProgramResult const phase) {
	State& state = top_state();
	++state.ip;
	state.phase = phase;
	program_timer_ = true;
	program_time_ = schedule_act(game, delay);
}

/**
 * Push the given program onto the stack and schedule acting.
 */
void ProductionSite::program_start(Game& game,
                                   const std::string& program_name,
                                   bool force,
                                   MapObject* extra_data) {
	State state;

	state.program = descr().get_program(program_name);
	state.ip = 0;
	state.phase = ProgramResult::kNone;
	state.objvar = extra_data;

	if (force) {
		state.flags |= State::StateFlags::kStateFlagIgnoreStopped;
	}
	if (extra_data != nullptr) {
		state.flags |= State::StateFlags::kStateFlagHasExtraData;
	}

	stack_.push_back(state);

	program_timer_ = true;
	Duration tdelta(10);
	FailedSkippedPrograms::const_iterator i = failed_skipped_programs_.find(program_name);
	if (i != failed_skipped_programs_.end()) {
		const Time& gametime = game.get_gametime();
		const Time& earliest_allowed_start_time = i->second + Duration(10000);
		if (gametime + tdelta < earliest_allowed_start_time) {
			tdelta = earliest_allowed_start_time - gametime;
		}
	}
	program_time_ = schedule_act(game, tdelta);
}

/**
 * Ends the current program now and updates the productivity statistics.
 *
 * \pre Any program is running
 * \post No program is running, acting is scheduled
 */
void ProductionSite::program_end(Game& game, ProgramResult const result) {
	assert(!stack_.empty());

	const std::string& program_name = top_state().program->name();

	stack_.pop_back();
	if (!stack_.empty()) {
		top_state().phase = result;
	}

	const Duration current_duration = game.get_gametime() - last_program_end_time;
	assert(game.get_gametime() >= last_program_end_time);
	last_program_end_time = game.get_gametime();

	switch (result) {
	case ProgramResult::kFailed:
		failed_skipped_programs_[program_name] = game.get_gametime();
		update_actual_statistics(current_duration, false);
		format_statistics_string();
		break;
	case ProgramResult::kCompleted:
		failed_skipped_programs_.erase(program_name);
		train_workers(game);
		update_actual_statistics(current_duration, true);
		format_statistics_string();
		break;
	case ProgramResult::kSkipped:
		failed_skipped_programs_[program_name] = game.get_gametime();
		update_actual_statistics(current_duration, false);
		format_statistics_string();
		break;
	case ProgramResult::kNone:
		failed_skipped_programs_.erase(program_name);
		break;
	}

	program_timer_ = true;
	program_time_ = schedule_act(game, post_timer_);
}

void ProductionSite::train_workers(Game& game) {
	for (uint32_t i = descr().nr_working_positions(); i != 0u;) {
		working_positions_.at(--i).worker.get(game)->gain_experience(game);
	}
	Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kWorkersChanged));
}

void ProductionSite::notify_player(Game& game, uint8_t minutes, FailNotificationType type) {
	if (get_actual_statistics() == 0 ||
	    (get_actual_statistics() <= descr().out_of_resource_productivity_threshold() &&
	     trend_ == Trend::kFalling)) {

		if (type == FailNotificationType::kFull) {
			// The building has nothing to do
			assert(!descr().resource_not_needed_message().empty());
			set_production_result(descr().resource_not_needed_message());
		} else if (descr().out_of_resource_message().empty()) {
			// We have no message body to send
			set_production_result(_("Can’t find any more resources!"));
		} else {
			// Send full message
			set_production_result(descr().out_of_resource_heading());

			assert(!descr().out_of_resource_message().empty());
			send_message(game, Message::Type::kEconomy, descr().out_of_resource_title(),
			             descr().icon_filename(), descr().out_of_resource_heading(),
			             descr().out_of_resource_message(), true, Duration(minutes * 60000), 0);
		}

		// The following sends "out of resources" messages to be picked up by AI
		// used as information for dismantling and upgrading buildings
		if (descr().get_ismine()) {
			Notifications::publish(NoteProductionSiteOutOfResources(this, get_owner()));
		}
	}
}

void ProductionSite::unnotify_player() {
	set_production_result("");
}

void ProductionSite::init_yard_interfaces(EditorGameBase& egbase) {
	const Map& map = egbase.map();

	while (!ship_fleet_interfaces_.empty()) {
		ship_fleet_interfaces_.front()->remove(egbase);
	}
	while (!ferry_fleet_interfaces_.empty()) {
		ferry_fleet_interfaces_.front()->remove(egbase);
	}

	if (descr().has_ship_fleet_check()) {
		std::vector<Coords> result;
		// 10 is a custom value to make sure the "ocean" is at least 10 nodes big.
		constexpr int kMinOceanSize = 10;
		map.find_reachable_fields(
		   egbase,
		   Area<FCoords>(map.get_fcoords(get_position()), descr().workarea_info().rbegin()->first),
		   &result, CheckStepDefault(MOVECAPS_WALK), FindNodeShore(kMinOceanSize));

		for (const Coords& coords : result) {
			ship_fleet_interfaces_.push_back(ShipFleetYardInterface::create(egbase, *this, coords));
		}

		if (ship_fleet_interfaces_.empty()) {
			if (upcast(Game, game, &egbase)) {
				send_message(*game, Message::Type::kEconomy, pgettext("building", "No Shore"),
				             descr().icon_filename(), _("Ship Yard Without Shore"),
				             _("Your ship yard has not been built close enough to a shore. It will not "
				               "be able to build ships."));
			}
		}
	}

	if (descr().has_ferry_fleet_check()) {
		std::vector<Coords> result;
		map.find_reachable_fields(
		   egbase,
		   Area<FCoords>(map.get_fcoords(get_position()), descr().workarea_info().rbegin()->first),
		   &result, CheckStepDefault(MOVECAPS_WALK), FindNodeFerry(0));

		for (const Coords& coords : result) {
			ferry_fleet_interfaces_.push_back(FerryFleetYardInterface::create(egbase, *this, coords));
		}

		if (ferry_fleet_interfaces_.empty()) {
			if (upcast(Game, game, &egbase)) {
				send_message(*game, Message::Type::kEconomy, pgettext("building", "No Shore"),
				             descr().icon_filename(), _("Ferry Yard Without Shore"),
				             _("Your ferry yard has not been built close enough to a shore. It will "
				               "not be able to build ferries."));
			}
		}
	}
}

void ProductionSite::remove_fleet_interface(EditorGameBase& /*egbase*/,
                                            const ShipFleetYardInterface* interface) {
	auto it = std::find(ship_fleet_interfaces_.begin(), ship_fleet_interfaces_.end(), interface);
	if (it == ship_fleet_interfaces_.end()) {
		throw wexception("Attempt to remove unknown ship fleet interface %u",
		                 interface == nullptr ? 0 : interface->serial());
	}
	*it = ship_fleet_interfaces_.back();
	ship_fleet_interfaces_.pop_back();
}

void ProductionSite::remove_fleet_interface(EditorGameBase& /*egbase*/,
                                            const FerryFleetYardInterface* interface) {
	auto it = std::find(ferry_fleet_interfaces_.begin(), ferry_fleet_interfaces_.end(), interface);
	if (it == ferry_fleet_interfaces_.end()) {
		throw wexception("Attempt to remove unknown ferry fleet interface %u",
		                 interface == nullptr ? 0 : interface->serial());
	}
	*it = ferry_fleet_interfaces_.back();
	ferry_fleet_interfaces_.pop_back();
}

std::unique_ptr<const BuildingSettings> ProductionSite::create_building_settings() const {
	std::unique_ptr<ProductionsiteSettings> settings(
	   new ProductionsiteSettings(descr(), owner().tribe()));
	settings->stopped = is_stopped_;
	for (auto& pair : settings->ware_queues) {
		pair.second.priority = get_priority(wwWARE, pair.first);
		for (const auto& queue : input_queues_) {
			if (queue->get_type() == wwWARE && queue->get_index() == pair.first) {
				pair.second.desired_fill = std::min(pair.second.max_fill, queue->get_max_fill());
				if (pair.second.desired_fill == 0) {
					// Players may set slots to 0 before enhancing a building to retrieve precious wares
					// – we assume they want the slot to be fully filled in the upgraded building
					pair.second.desired_fill = pair.second.max_fill;
				}
				break;
			}
		}
	}
	for (auto& pair : settings->worker_queues) {
		pair.second.priority = get_priority(wwWORKER, pair.first);
		for (const auto& queue : input_queues_) {
			if (queue->get_type() == wwWORKER && queue->get_index() == pair.first) {
				pair.second.desired_fill = std::min(pair.second.max_fill, queue->get_max_fill());
				if (pair.second.desired_fill == 0) {
					pair.second.desired_fill = pair.second.max_fill;
				}
				break;
			}
		}
	}
	// Prior to the resolution of a defect report against ISO C++11, local variable 'settings' would
	// have been copied despite being returned by name, due to its not matching the function return
	// type. Call 'std::move' explicitly to avoid copying on older compilers.
	// On modern compilers a simple 'return settings;' would've been fine.
	return std::unique_ptr<const BuildingSettings>(std::move(settings));
}

/// Changes the default anim string to \li anim
void ProductionSite::set_default_anim(const std::string& anim) {
	if (default_anim_ == anim) {
		return;
	}

	if (!descr().is_animation_known(anim)) {
		return;
	}

	default_anim_ = anim;
}

constexpr Duration kStatsEntireDuration(5 * 60 * 1000);  // statistic evaluation base
constexpr Duration kStatsDurationCap(180 * 1000);        // This is highest allowed program duration

void ProductionSite::update_actual_statistics(Duration duration, const bool produced) {
	// just for case something went very wrong...
	if (duration > kStatsDurationCap) {
		duration = kStatsDurationCap;
	}
	const Duration past_duration = kStatsEntireDuration - duration;
	actual_percent_ = (actual_percent_ * past_duration.get() +
	                   static_cast<unsigned int>(produced) * duration.get() * 1000) /
	                  kStatsEntireDuration.get();
	assert(actual_percent_ <= 1000);  // be sure we do not go above 100%
}

}  // namespace Widelands
