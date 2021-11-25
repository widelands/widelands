/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
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

#include "ai/defaultai.h"

#include <algorithm>
#include <cstdlib>
#include <memory>

#include "ai/ai_hints.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "economy/wares_queue.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/playercommand.h"

// following is in milliseconds (widelands counts time in ms)
constexpr Duration kFieldInfoExpiration(12 * 1000);
constexpr Duration kMineFieldInfoExpiration(20 * 1000);
constexpr Duration kNewMineConstInterval(19000);
constexpr Duration kBusyMineUpdateInterval(2000);
// building of the same building can be started after 25s at earliest
constexpr Duration kBuildingMinInterval(25 * 1000);
constexpr Duration kMinBFCheckInterval(5 * 1000);
constexpr Duration kMinMFCheckInterval(19 * 1000);
constexpr Duration kMarineDecisionInterval(20 * 1000);
constexpr Duration kRemainingBasicBuildingsResetTime(1 * 60 * 1000);

// following two are used for roads management, for creating shortcuts and dismantling dispensable
// roads
constexpr int32_t kSpotsEnough = 25;

constexpr uint16_t kTargetQuantCap = 30;

// this is intended for map developers & testers, should be off by default
constexpr bool kPrintStats = false;

// for scheduler
constexpr int kMaxJobs = 4;

// Count of mine types / ground resources
constexpr int kMineTypes = 4;

namespace AI {

// Fix undefined references
constexpr Duration DefaultAI::kManagementUpdateInterval;
constexpr Duration DefaultAI::kStatUpdateInterval;
constexpr Duration DefaultAI::kFlagWarehouseUpdInterval;
constexpr Duration DefaultAI::kExpeditionMinDuration;
constexpr Duration DefaultAI::kExpeditionMaxDuration;
constexpr Duration DefaultAI::kShipCheckInterval;
constexpr Duration DefaultAI::kCampaignDuration;
constexpr Duration DefaultAI::kTrainingSitesCheckInterval;

DefaultAI::NormalImpl DefaultAI::normal_impl;
DefaultAI::WeakImpl DefaultAI::weak_impl;
DefaultAI::VeryWeakImpl DefaultAI::very_weak_impl;

Time DefaultAI::last_seafaring_check_ = Time(0);
bool DefaultAI::map_allows_seafaring_ = false;

/// Constructor of DefaultAI
DefaultAI::DefaultAI(Widelands::Game& ggame, Widelands::PlayerNumber const pid, AiType const t)
   : ComputerPlayer(ggame, pid),
     type_(t),
     player_(nullptr),
     tribe_(nullptr),
     attackers_count_(0),
     // Delay initialization to allow scenario scripts
     // to load custom units/buildings at gametime 0
     next_ai_think_(1),
     scheduler_delay_counter_(0),
     numof_psites_in_constr(0),
     num_ports(0),
     numof_warehouses_(0),
     numof_warehouses_in_const_(0),
     military_last_dismantle_(0),
     military_last_build_(0),
     time_of_last_construction_(0),
     next_mine_construction_due_(0),
     fishers_count_(0),
     first_iron_mine_built(50 * 60 * 60 * 1000),
     ts_finished_count_(0),
     ts_in_const_count_(0),
     ts_without_trainers_(0),
     enemysites_check_delay_(30),
     spots_(0),
     resource_necessity_water_needed_(false),
     highest_nonmil_prio_(0),
     expedition_ship_(kNoShip) {

	// Subscribe to NoteFieldPossession.
	field_possession_subscriber_ = Notifications::subscribe<Widelands::NoteFieldPossession>(
	   [this](const Widelands::NoteFieldPossession& note) {
		   if (note.player != player_) {
			   return;
		   }
		   if (note.ownership == Widelands::NoteFieldPossession::Ownership::GAINED) {
			   unusable_fields.push_back(note.fc);
		   }
	   });

	// Subscribe to NoteImmovables.
	immovable_subscriber_ = Notifications::subscribe<Widelands::NoteImmovable>(
	   [this](const Widelands::NoteImmovable& note) {
		   if (player_ == nullptr) {
			   return;
		   }
		   if (note.pi->owner().player_number() != player_->player_number()) {
			   return;
		   }
		   if (note.ownership == Widelands::NoteImmovable::Ownership::GAINED) {
			   gain_immovable(*note.pi);
		   } else {
			   lose_immovable(*note.pi);
		   }
	   });

	// Subscribe to ProductionSiteOutOfResources.
	outofresource_subscriber_ =
	   Notifications::subscribe<Widelands::NoteProductionSiteOutOfResources>(
	      [this](const Widelands::NoteProductionSiteOutOfResources& note) {
		      if (note.ps->owner().player_number() != player_->player_number()) {
			      return;
		      }

		      out_of_resources_site(*note.ps);
	      });

	// Subscribe to TrainingSiteSoldierTrained.
	soldiertrained_subscriber_ = Notifications::subscribe<Widelands::NoteTrainingSiteSoldierTrained>(
	   [this](const Widelands::NoteTrainingSiteSoldierTrained& note) {
		   if (note.ts->owner().player_number() != player_->player_number()) {
			   return;
		   }

		   soldier_trained(*note.ts);
	   });

	// Subscribe to ShipNotes.
	shipnotes_subscriber_ =
	   Notifications::subscribe<Widelands::NoteShip>([this](const Widelands::NoteShip& note) {
		   // in a short time between start and late_initialization the player
		   // can get notes that can not be processed.
		   // It seems that this causes no problem, at least no substantial
		   if (player_ == nullptr) {
			   return;
		   }
		   if (note.ship->get_owner()->player_number() != player_->player_number()) {
			   return;
		   }

		   switch (note.action) {

		   case Widelands::NoteShip::Action::kGained:
			   gain_ship(*note.ship, NewShip::kBuilt);
			   break;

		   case Widelands::NoteShip::Action::kLost:
			   for (std::deque<ShipObserver>::iterator i = allships.begin(); i != allships.end();
			        ++i) {
				   if (i->ship == note.ship) {
					   allships.erase(i);
					   break;
				   }
			   }
			   break;

		   case Widelands::NoteShip::Action::kWaitingForCommand:
			   for (ShipObserver& observer : allships) {
				   if (observer.ship == note.ship) {
					   observer.waiting_for_command_ = true;
					   break;
				   }
			   }
			   break;
		   default:
			   // Do nothing
			   break;
		   }
	   });
}

DefaultAI::~DefaultAI() {
	while (!buildable_fields.empty()) {
		delete buildable_fields.back();
		buildable_fields.pop_back();
	}

	while (!mineable_fields.empty()) {
		delete mineable_fields.back();
		mineable_fields.pop_back();
	}

	while (!economies.empty()) {
		delete economies.back();
		economies.pop_back();
	}
}

/**
 * Main loop of computer player_ "defaultAI"
 *
 * General behaviour is defined here.
 */
void DefaultAI::think() {
	const Time& gametime = game().get_gametime();

	if (next_ai_think_ > gametime) {
		return;
	}

	if (tribe_ == nullptr) {
		late_initialization();
	}

	// AI now thinks twice in a seccond, if the game engine allows this
	// if too busy, the period can be many seconds.
	next_ai_think_ = gametime + Duration(500);

	if (player_->is_picking_custom_starting_position()) {
		// TODO(Nordfriese): In picking_custom_starting_position mode, try to find
		// a nice spot somewhere instead of just picking the default position.
		// This here will not work if another player has already chosen a spot
		// close to our default location…
		if (!player_->pick_custom_starting_position(game().map().get_starting_pos(player_number()))) {
			log_warn_time(gametime, "AI %u: default starting position already taken!\n",
			              static_cast<unsigned>(player_number()));
		}
		return;
	}

	SchedulerTaskId due_task = SchedulerTaskId::kUnset;

	sort_task_pool();

	const int32_t delay_time = gametime.get() - taskPool.front().due_time.get();

	// Here we decide how many jobs will be run now (none - 5)
	// in case no job is due now, it can be zero
	uint32_t jobs_to_run_count = (delay_time < 0) ? 0 : 1;

	// Here we collect data for "too late ..." message
	if (delay_time > 5000) {
		++scheduler_delay_counter_;
	} else {
		scheduler_delay_counter_ = 0;
	}

	if (jobs_to_run_count == 0) {
		// well we have nothing to do now
		return;
	}

	// And printing it now and resetting counter
	if (scheduler_delay_counter_ > 10) {
		log_warn_time(gametime, " %d: AI: game speed too high, jobs are too late (now %2d seconds)\n",
		              player_number(), static_cast<int32_t>(delay_time / 1000));
		scheduler_delay_counter_ = 0;
	}

	// 400 provides that second job is run if delay time is longer then 1.6 sec
	if (delay_time / 400 > 1) {
		jobs_to_run_count = sqrt(static_cast<uint32_t>(delay_time / 500));
	}

	jobs_to_run_count = (jobs_to_run_count > kMaxJobs) ? kMaxJobs : jobs_to_run_count;
	assert(jobs_to_run_count > 0 && jobs_to_run_count <= kMaxJobs);
	assert(jobs_to_run_count < taskPool.size());

	// Pool of tasks to be executed this run. In ideal situation it will consist of one task only.
	std::vector<SchedulerTask> current_task_queue;
	assert(current_task_queue.empty());
	// Here we push SchedulerTask members into the temporary queue, providing that a task is due now
	// and
	// the limit (jobs_to_run_count) is not exceeded
	for (uint8_t i = 0; i < jobs_to_run_count; ++i) {
		if (taskPool[i].due_time <= gametime) {
			current_task_queue.push_back(taskPool[i]);
			sort_task_pool();
		} else {
			break;
		}
	}

	assert(!current_task_queue.empty() && current_task_queue.size() <= jobs_to_run_count);

	// Ordering temporary queue so that higher priority (lower number) is on the beginning
	std::sort(current_task_queue.begin(), current_task_queue.end());

	// Performing tasks from temporary queue one by one
	for (const SchedulerTask& task : current_task_queue) {

		due_task = task.id;

		++sched_stat_[static_cast<uint32_t>(due_task)];

		// Now AI runs a job selected above to be performed in this turn
		// (only one but some of them needs to run check_economies() to
		// guarantee consistency)
		// job names are selfexplanatory
		switch (due_task) {
		case SchedulerTaskId::kBbuildableFieldsCheck:
			update_all_buildable_fields(gametime);
			set_taskpool_task_time(
			   gametime + kMinBFCheckInterval, SchedulerTaskId::kBbuildableFieldsCheck);
			break;
		case SchedulerTaskId::kMineableFieldsCheck:
			update_all_mineable_fields(gametime);
			set_taskpool_task_time(
			   gametime + kMinMFCheckInterval, SchedulerTaskId::kMineableFieldsCheck);
			break;
		case SchedulerTaskId::kRoadCheck:
			if (check_economies()) {  // is a must
				return;
			}
			set_taskpool_task_time(gametime + Duration(1000), SchedulerTaskId::kRoadCheck);
			// testing 5 roads
			{
				const int32_t roads_to_check = roads.size() / 30 + 1;
				for (int j = 0; j < roads_to_check; ++j) {
					// improve_roads function will test one road and rotate roads vector
					if (improve_roads(gametime)) {
						// if significant change takes place do not go on
						break;
					}
				}
			}
			break;
		case SchedulerTaskId::kUnbuildableFCheck:
			set_taskpool_task_time(gametime + Duration(4000), SchedulerTaskId::kUnbuildableFCheck);
			update_all_not_buildable_fields();
			break;
		case SchedulerTaskId::kCheckEconomies:
			check_economies();
			set_taskpool_task_time(gametime + Duration(8000), SchedulerTaskId::kCheckEconomies);
			break;
		case SchedulerTaskId::kProductionsitesStats:
			update_productionsite_stats();
			// Updating the stats every 10 seconds should be enough
			set_taskpool_task_time(gametime + Duration(10000), SchedulerTaskId::kProductionsitesStats);
			break;
		case SchedulerTaskId::kConstructBuilding:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			if (gametime < Time(15000)) {  // More frequent at the beginning of game
				set_taskpool_task_time(gametime + Duration(2000), SchedulerTaskId::kConstructBuilding);
			} else {
				set_taskpool_task_time(gametime + Duration(6000), SchedulerTaskId::kConstructBuilding);
			}
			if (construct_building(gametime)) {
				time_of_last_construction_ = gametime;
			}
			break;
		case SchedulerTaskId::kCheckProductionsites:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			{
				set_taskpool_task_time(
				   gametime + Duration(15000), SchedulerTaskId::kCheckProductionsites);
				// testing 5 productionsites (if there are 5 of them)
				int32_t ps_to_check = (productionsites.size() < 5) ? productionsites.size() : 5;
				for (int j = 0; j < ps_to_check; ++j) {
					// one productionsite per one check_productionsites() call
					if (check_productionsites(gametime)) {
						// if significant change takes place do not go on
						break;
					}
				}
			}
			break;
		case SchedulerTaskId::kCheckShips:
			// if function returns false, we can postpone next call
			{
				const uint8_t wait_multiplier = (check_ships(gametime)) ? 1 : 5;
				set_taskpool_task_time(
				   gametime + kShipCheckInterval * wait_multiplier, SchedulerTaskId::kCheckShips);
			}
			break;
		case SchedulerTaskId::KMarineDecisions:
			// if function returns false, we can postpone for next call
			{
				const uint8_t wait_multiplier = (marine_main_decisions(gametime)) ? 1 : 5;
				set_taskpool_task_time(gametime + kMarineDecisionInterval * wait_multiplier,
				                       SchedulerTaskId::KMarineDecisions);
			}
			break;
		case SchedulerTaskId::kCheckMines:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			set_taskpool_task_time(gametime + Duration(15000), SchedulerTaskId::kCheckMines);
			// checking 3 mines if possible
			{
				int32_t mines_to_check = (mines_.size() < 5) ? mines_.size() : 5;
				for (int j = 0; j < mines_to_check; ++j) {
					// every run of check_mines_() checks one mine
					if (check_mines_(gametime)) {
						// if significant change takes place do not go on
						break;
					}
				}
			}
			break;
		case SchedulerTaskId::kCheckMilitarysites:
			// just to be sure the value is reset
			if (check_militarysites(gametime)) {
				set_taskpool_task_time(
				   gametime + Duration(15 * 1000), SchedulerTaskId::kCheckMilitarysites);
			} else {
				set_taskpool_task_time(
				   gametime + Duration(4 * 1000), SchedulerTaskId::kCheckMilitarysites);
			}
			break;
		case SchedulerTaskId::kCheckTrainingsites:
			set_taskpool_task_time(
			   gametime + kTrainingSitesCheckInterval, SchedulerTaskId::kCheckTrainingsites);
			check_trainingsites(gametime);
			break;
		case SchedulerTaskId::kCountMilitaryVacant:
			count_military_vacant_positions();
			set_taskpool_task_time(
			   gametime + Duration(25 * 1000), SchedulerTaskId::kCountMilitaryVacant);
			break;
		case SchedulerTaskId::kWareReview:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			set_taskpool_task_time(gametime + Duration(15 * 60 * 1000), SchedulerTaskId::kWareReview);
			review_wares_targets(gametime);
			break;
		case SchedulerTaskId::kPrintStats:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			set_taskpool_task_time(gametime + Duration(10 * 60 * 1000), SchedulerTaskId::kPrintStats);
			print_stats(gametime);
			break;
		case SchedulerTaskId::kCheckEnemySites:
			check_enemy_sites(gametime);
			set_taskpool_task_time(gametime + Duration(19 * 1000), SchedulerTaskId::kCheckEnemySites);
			break;
		case SchedulerTaskId::kManagementUpdate:
			// This task is used for training the AI, so it should be usually disabled
			{  // statistics for spotted warehouses
				uint16_t conquered_wh = 0;
				for (auto coords : enemy_warehouses) {
					if (get_land_owner(game().map(), coords) == player_number()) {
						++conquered_wh;
					}
				}
				if (!basic_economy_established) {
					assert(!persistent_data->remaining_basic_buildings.empty());
					verb_log_info_time(
					   gametime,
					   "AI %2d: Basic economy not achieved, %" PRIuS " building(s) missing, f.e.: %s\n",
					   player_number(), persistent_data->remaining_basic_buildings.size(),
					   get_building_observer(persistent_data->remaining_basic_buildings.begin()->first)
					      .name);
				}
				if (!enemy_warehouses.empty()) {
					verb_log_info_time(gametime, "Conquered warehouses: %d / %" PRIuS "\n", conquered_wh,
					                   enemy_warehouses.size());
				}
				management_data.review(
				   gametime, player_number(), player_statistics.get_player_land(player_number()),
				   player_statistics.get_enemies_max_land(),
				   player_statistics.get_old60_player_land(player_number()), attackers_count_,
				   soldier_trained_log.count(gametime),
				   player_statistics.get_player_power(player_number()),
				   count_productionsites_without_buildings(), first_iron_mine_built);
				set_taskpool_task_time(
				   gametime + kManagementUpdateInterval, SchedulerTaskId::kManagementUpdate);
			}
			break;
		case SchedulerTaskId::kUpdateStats:
			update_player_stat(gametime);
			set_taskpool_task_time(gametime + kStatUpdateInterval, SchedulerTaskId::kUpdateStats);
			break;
		case SchedulerTaskId::kWarehouseFlagDist:
			check_flag_distances(gametime);
			set_taskpool_task_time(
			   gametime + kFlagWarehouseUpdInterval, SchedulerTaskId::kWarehouseFlagDist);
			break;
		case SchedulerTaskId::kUnset:
			NEVER_HERE();
		}
	}
}

/**
 * Cares for all variables not initialised during construction
 *
 * When DefaultAI is constructed, some information is not yet available (e.g.
 * descriptions), so this is done after complete loading of the map.
 */
void DefaultAI::late_initialization() {
	player_ = game().get_player(player_number());
	tribe_ = &player_->tribe();
	if (game().is_ai_training_mode()) {
		ai_training_mode_ = true;
		management_data.set_ai_training_mode();
	}
	const Time& gametime = game().get_gametime();

	verb_log_info_time(gametime, "ComputerPlayer(%d): initializing as type %u%s\n", player_number(),
	                   static_cast<unsigned int>(type_),
	                   (ai_training_mode_) ? ", in ai training mode" : "");
	if (player_->team_number() > 0) {
		verb_log_info_time(gametime, "    ... member of team %d\n", player_->team_number());
	}

	wares.resize(game().descriptions().nr_wares());
	for (Widelands::DescriptionIndex i = 0;
	     i < static_cast<Widelands::DescriptionIndex>(game().descriptions().nr_wares()); ++i) {
		wares.at(i).preciousness =
		   game().descriptions().get_ware_descr(i)->ai_hints().preciousness(tribe_->name());
	}

	const Widelands::DescriptionIndex& nr_buildings = game().descriptions().nr_buildings();

	// The data struct below is owned by Player object, the purpose is to have them saved therein
	persistent_data = player_->get_mutable_ai_persistent_state();
	management_data.persistent_data = player_->get_mutable_ai_persistent_state();
	const bool create_basic_buildings_list =
	   !persistent_data->initialized || (gametime.get() < kRemainingBasicBuildingsResetTime.get());

	if (!persistent_data->initialized) {
		// As all data are initialized without given values, they must be populated with reasonable
		// values first
		persistent_data->initialize();

		// AI's DNA population
		management_data.new_dna_for_persistent(player_number(), type_);
		management_data.copy_persistent_to_local();
		management_data.mutate(player_number());
		if (ai_training_mode_) {
			management_data.dump_data(player_number());
		}

		management_data.test_consistency(true);
		assert(management_data.get_military_number_at(42) ==
		       management_data.get_military_number_at(kMutationRatePosition));

	} else {
		// Doing some consistency checks
		check_range<Time>(persistent_data->expedition_start_time, gametime, "expedition_start_time");
		check_range<uint16_t>(persistent_data->ships_utilization, 0, 10000, "ships_utilization_");

		// for backward consistency
		if (persistent_data->ai_personality_mil_upper_limit <
		    persistent_data->target_military_score) {
			persistent_data->ai_personality_mil_upper_limit = persistent_data->target_military_score;
		}
		if (persistent_data->least_military_score > persistent_data->target_military_score) {
			persistent_data->least_military_score = persistent_data->target_military_score;
		}

		if (ai_training_mode_) {
			verb_log_dbg_time(
			   gametime, "AI %2d: reinitializing dna (kAITrainingMode set true)", player_number());
			management_data.new_dna_for_persistent(player_number(), type_);
			management_data.copy_persistent_to_local();
			management_data.mutate(player_number());
			management_data.dump_data(player_number());

		} else {
			management_data.copy_persistent_to_local();
		}

		management_data.test_consistency(true);

		verb_log_info_time(gametime, " AI %2d: %" PRIuS " basic buildings in savegame file. %s\n",
		                   player_number(), persistent_data->remaining_basic_buildings.size(),
		                   (create_basic_buildings_list) ?
                            "New list will be recreated though (kAITrainingMode is true)" :
                            "");
	}

	// Even if we have basic buildings from savefile, we ignore them and recreate them based
	// on lua conf files
	if (create_basic_buildings_list) {
		persistent_data->remaining_basic_buildings.clear();
	}

	buildings_immovable_attributes_.clear();

	// Temporarily remember the lumberjacks for speeding up the ranger detection
	std::set<const Widelands::ProductionSiteDescr*> lumberjacks;

	for (Widelands::DescriptionIndex building_index = 0; building_index < nr_buildings;
	     ++building_index) {
		const Widelands::BuildingDescr& bld = *tribe_->get_building_descr(building_index);
		if (!tribe_->has_building(building_index) &&
		    bld.type() != Widelands::MapObjectType::MILITARYSITE) {
			continue;
		}

		const std::string& building_name = bld.name();
		const BuildingHints& bh = bld.hints();
		buildings_.resize(buildings_.size() + 1);
		BuildingObserver& bo = buildings_.back();
		bo.name = building_name.c_str();
		bo.id = building_index;
		bo.desc = &bld;
		bo.type = BuildingObserver::Type::kBoring;
		bo.cnt_built = 0;
		bo.cnt_under_construction = 0;
		bo.cnt_target = 1;  // default for everything
		bo.cnt_limit_by_aimode = std::numeric_limits<int32_t>::max();
		bo.cnt_upgrade_pending = 0;
		bo.stocklevel_count = 0;
		bo.stocklevel_time = Time(0);
		bo.last_dismantle_time = Time(0);
		// this is set to negative number, otherwise the AI would wait 25 sec
		// after game start not building anything
		// TODO(Nordfriese): …but times can no longer be negative. Seems to be working fine though…
		bo.construction_decision_time = Time(0);  // -60 * 60 * 1000;
		bo.last_building_built = Time();
		bo.build_material_shortage = false;
		bo.current_stats = 0;
		bo.unoccupied_count = 0;
		bo.unconnected_count = 0;
		bo.new_building_overdue = 0;
		bo.primary_priority = 0;
		if (bld.is_buildable()) {
			bo.set_is(BuildingAttribute::kBuildable);
		}
		if (bld.needs_seafaring()) {
			bo.set_is(BuildingAttribute::kNeedsSeafaring);
		}
		if (create_basic_buildings_list &&
		    bh.basic_amount() > 0) {  // This is the very begining of the game
			assert(persistent_data->remaining_basic_buildings.count(bo.id) == 0);
			persistent_data->remaining_basic_buildings.emplace(
			   std::make_pair(bo.id, bh.basic_amount()));
		}
		bo.basic_amount = bh.basic_amount();
		if (bh.needs_water()) {
			verb_log_dbg_time(gametime, "AI %d detected coast building: %s", player_number(), bo.name);
			bo.set_is(BuildingAttribute::kNeedsCoast);
		}
		if (bh.is_space_consumer()) {
			bo.set_is(BuildingAttribute::kSpaceConsumer);
		}
		bo.expansion_type = bh.is_expansion_type();
		bo.fighting_type = bh.is_fighting_type();
		bo.mountain_conqueror = bh.is_mountain_conqueror();
		bo.prohibited_till = Time(bh.get_prohibited_till() * 1000);  // value in conf is in seconds
		bo.forced_after = Time(bh.get_forced_after() * 1000);        // value in conf is in seconds
		if (bld.get_isport()) {
			bo.set_is(BuildingAttribute::kPort);
		}
		bo.max_trainingsites_proportion = 100;
		bo.initial_preciousness = 0;
		bo.max_preciousness = 0;
		bo.max_needed_preciousness = 0;

		// Is total count of this building limited by AI mode?
		if (bh.get_ai_limit(type_) >= 0) {
			bo.cnt_limit_by_aimode = bh.get_ai_limit(type_);
		}

		// Read all interesting data from ware producing buildings
		if (bld.type() == Widelands::MapObjectType::PRODUCTIONSITE) {
			const Widelands::ProductionSiteDescr& prod =
			   dynamic_cast<const Widelands::ProductionSiteDescr&>(bld);
			bo.type = bld.get_ismine() ? BuildingObserver::Type::kMine :
                                      BuildingObserver::Type::kProductionsite;
			for (const auto& temp_input : prod.input_wares()) {
				bo.inputs.push_back(temp_input.first);
			}
			for (const Widelands::DescriptionIndex& temp_output : prod.output_ware_types()) {
				bo.ware_outputs.push_back(temp_output);
				if (tribe_->is_construction_material(temp_output) && !bo.inputs.empty()) {
					wares.at(temp_output).refined_build_material = true;
				}
			}

			// Read information about worker outputs
			if (!prod.output_worker_types().empty()) {
				for (const Widelands::DescriptionIndex& temp_output : prod.output_worker_types()) {
					if (temp_output == tribe_->soldier()) {
						bo.set_is(BuildingAttribute::kBarracks);
					}
					const WareWorkerHints* worker_hints =
					   tribe_->get_worker_descr(temp_output)->ai_hints();
					const int worker_preciousness = worker_hints->preciousness(tribe_->name());
					if (worker_preciousness != Widelands::kInvalidWare) {
						bo.initial_preciousness += worker_preciousness;
					}
				}
				if (!bo.is(BuildingAttribute::kBarracks) && bo.ware_outputs.empty()) {
					bo.set_is(BuildingAttribute::kRecruitment);
					verb_log_dbg_time(
					   gametime, "AI %d detected recruitment site: %s", player_number(), bo.name);
				}
			}

			for (const auto& temp_position : prod.working_positions()) {
				for (uint8_t i = 0; i < temp_position.second; i++) {
					bo.positions.push_back(temp_position.first);
				}
			}

			// If this is a producer, does it act also as supporter?
			if (!bo.ware_outputs.empty() && !prod.supported_productionsites().empty()) {
				verb_log_dbg_time(
				   gametime, "AI %d detected supporting producer: %s", player_number(), bo.name);
				for (const auto& supp : prod.supported_productionsites()) {
					verb_log_dbg_time(gametime, "  -> %s", supp.c_str());
				}
				bo.set_is(BuildingAttribute::kSupportingProducer);
			}

			bo.supported_producers.clear();
			for (const std::string& supported_building_name : prod.supported_productionsites()) {
				Widelands::DescriptionIndex supported_building_index =
				   tribe_->building_index(supported_building_name);
				bo.supported_producers.insert(std::make_pair(
				   supported_building_index, dynamic_cast<const Widelands::ProductionSiteDescr*>(
				                                tribe_->get_building_descr(supported_building_index))));
			}
			// TODO(hessenfarmer): we need to find a solution for amazons here as they do not mine
			// iron
			iron_resource_id = game().descriptions().resource_index("resource_iron");
			if (iron_resource_id == Widelands::INVALID_INDEX) {
				throw wexception(
				   "The AI needs the descriptions to define the resource 'resource_iron'");
			}

			if (bo.type == BuildingObserver::Type::kMine) {
				// get the resource needed by the mine
				const auto& collected_resources = prod.collected_resources();
				if (collected_resources.size() > 1) {
					log_warn("AI %d: The mine '%s' will mine multiple resources. The AI can't handle "
					         "this and will simply pick the first one from the list.",
					         player_number(), bo.name);
				}
				const auto& first_resource_it = collected_resources.begin();
				if (first_resource_it == collected_resources.end()) {
					log_warn(
					   "AI %d: The mine '%s' does not mine any resources!", player_number(), bo.name);
					bo.mines = Widelands::INVALID_INDEX;
				} else {
					bo.mines = game().descriptions().resource_index(first_resource_it->first);
				}

				// populating mines_per_type map
				if (mines_per_type.count(bo.mines) == 0) {
					mines_per_type[bo.mines] = MineTypesObserver();
				}
				// Identify iron mines based on mines value
				if (bo.mines == iron_resource_id) {
					mines_per_type[bo.mines].is_critical = true;
					mine_fields_stat.add_critical_ore(bo.mines);
				}
			}

			if (bh.is_shipyard()) {
				bo.set_is(BuildingAttribute::kShipyard);
			}
			if (bh.supports_seafaring()) {
				bo.set_is(BuildingAttribute::kSupportsSeafaring);
			}

			// now we find out if the upgrade of the building is a full substitution
			// (produces all wares as current one)
			const Widelands::DescriptionIndex enhancement = bld.enhancement();
			if (enhancement != Widelands::INVALID_INDEX &&
			    bo.type == BuildingObserver::Type::kProductionsite) {
				std::unordered_set<Widelands::DescriptionIndex> enh_outputs;
				const Widelands::ProductionSiteDescr& enh_prod =
				   dynamic_cast<const Widelands::ProductionSiteDescr&>(
				      *tribe_->get_building_descr(enhancement));

				// collecting wares that are produced in enhanced building
				for (const Widelands::DescriptionIndex& ware : enh_prod.output_ware_types()) {
					enh_outputs.insert(ware);
				}
				// now testing outputs of current building
				// and comparing
				bo.set_is(BuildingAttribute::kUpgradeSubstitutes);
				for (Widelands::DescriptionIndex ware : bo.ware_outputs) {
					if (enh_outputs.count(ware) == 0) {
						bo.unset_is(BuildingAttribute::kUpgradeSubstitutes);
						break;
					}
				}

				std::unordered_set<Widelands::DescriptionIndex> cur_outputs;
				// collecting wares that are produced in enhanced building
				for (const Widelands::DescriptionIndex& ware : bo.ware_outputs) {
					cur_outputs.insert(ware);
				}
				// Does upgraded building produce any different outputs?
				for (Widelands::DescriptionIndex ware : enh_outputs) {
					if (cur_outputs.count(ware) == 0) {
						bo.set_is(BuildingAttribute::kUpgradeExtends);
						break;
					}
				}
			}

			// now we identify producers of critical build materials
			for (Widelands::DescriptionIndex ware : bo.ware_outputs) {
				// building material except for trivial material
				if (wares.at(ware).refined_build_material) {
					bo.set_is(BuildingAttribute::kBuildingMatProducer);
					if (bo.type == BuildingObserver::Type::kMine) {
						mines_per_type[bo.mines].is_critical = true;
						mine_fields_stat.add_critical_ore(bo.mines);
					}
				}
			}

			for (const auto& temp_buildcosts : prod.buildcost()) {
				// building material except for trivial material
				if (wares.at(temp_buildcosts.first).refined_build_material) {
					bo.critical_building_material.push_back(temp_buildcosts.first);
				}
			}

			// Some important buildings are identified
			if (prod.input_wares().empty() && !prod.output_ware_types().empty() &&
			    prod.created_immovables().empty() && !prod.collected_immovables().empty()) {
				// TODO(hessenfarmer): hardcoded strings should be parsed from a definition file
				for (const auto& attribute : prod.collected_attributes()) {
					if (attribute.second == Widelands::MapObjectDescr::get_attribute_id("rocks")) {
						verb_log_dbg_time(
						   gametime, "AI %d detected quarry: %s", player_number(), bo.name);
						bo.set_is(BuildingAttribute::kNeedsRocks);
						buildings_immovable_attributes_[attribute.second].insert(
						   ImmovableAttribute(bo.name, BuildingAttribute::kNeedsRocks));
						break;
					} else if (attribute.second == Widelands::MapObjectDescr::get_attribute_id("tree") ||
					           attribute.second ==
					              Widelands::MapObjectDescr::get_attribute_id("normal_tree") ||
					           attribute.second ==
					              Widelands::MapObjectDescr::get_attribute_id("tree_balsa")) {
						verb_log_dbg_time(
						   gametime, "AI %d detected lumberjack: %s", player_number(), bo.name);
						bo.set_is(BuildingAttribute::kLumberjack);
						buildings_immovable_attributes_[attribute.second].insert(
						   ImmovableAttribute(bo.name, BuildingAttribute::kLumberjack));
						break;
					} else if (attribute.second ==
					           Widelands::MapObjectDescr::get_attribute_id("ripe_bush")) {
						verb_log_dbg_time(
						   gametime, "AI %d detected berry collector: %s", player_number(), bo.name);
						bo.set_is(BuildingAttribute::kNeedsBerry);
						buildings_immovable_attributes_[attribute.second].insert(
						   ImmovableAttribute(bo.name, BuildingAttribute::kNeedsBerry));
					}
				}
				if (bo.is(BuildingAttribute::kLumberjack)) {
					lumberjacks.insert(&prod);
				}
			}

			// here we identify hunters
			if (!prod.collected_bobs().empty()) {
				verb_log_dbg_time(gametime, "AI %d detected hunter: %s", player_number(), bo.name);
				bo.set_is(BuildingAttribute::kHunter);
			}

			// fishers
			if (bh.needs_water() && prod.collected_resources().count("resource_fish") == 1) {
				verb_log_dbg_time(gametime, "AI %d detected fisher: %s", player_number(), bo.name);
				bo.set_is(BuildingAttribute::kFisher);
			}

			// wells
			if (prod.input_wares().empty()) {
				for (Widelands::DescriptionIndex ware_index : prod.output_ware_types()) {
					if (tribe_->get_ware_descr(ware_index)->name() == "water" &&
					    prod.collected_resources().count("resource_water") == 1) {
						verb_log_dbg_time(gametime, "AI %d detected well: %s", player_number(), bo.name);
						bo.set_is(BuildingAttribute::kWell);
					}
				}
			}

			bo.requires_supporters = bh.requires_supporters();
			if (bo.requires_supporters) {
				verb_log_dbg_time(
				   gametime, "AI %d: %s strictly requires supporters\n", player_number(), bo.name);
			}
			continue;
		}

		// now for every military building, we fill critical_building_material vector
		// with critical construction wares
		if (bld.type() == Widelands::MapObjectType::MILITARYSITE) {
			bo.type = BuildingObserver::Type::kMilitarysite;
			const Widelands::MilitarySiteDescr& milit =
			   dynamic_cast<const Widelands::MilitarySiteDescr&>(bld);
			for (const auto& temp_buildcosts : milit.buildcost()) {
				// Below are non-critical wares (wares produced without inputs)
				if (wares.at(temp_buildcosts.first).refined_build_material) {
					bo.critical_building_material.push_back(temp_buildcosts.first);
				}
			}
			continue;
		}

		if (bld.type() == Widelands::MapObjectType::WAREHOUSE) {
			bo.type = BuildingObserver::Type::kWarehouse;
			continue;
		}

		if (bld.type() == Widelands::MapObjectType::TRAININGSITE) {
			bo.type = BuildingObserver::Type::kTrainingsite;
			bo.max_trainingsites_proportion = bh.trainingsites_max_percent();
			assert(bo.max_trainingsites_proportion <= 100);
			const Widelands::TrainingSiteDescr& train =
			   dynamic_cast<const Widelands::TrainingSiteDescr&>(bld);
			for (const auto& temp_input : train.input_wares()) {
				bo.inputs.push_back(temp_input.first);

				// collecting subsitutes
				// TODO(hessenfarmer): should be identified from trainingprograms not hardcoded
				if (tribe_->ware_index("meat") == temp_input.first ||
				    tribe_->ware_index("fish") == temp_input.first ||
				    tribe_->ware_index("smoked_meat") == temp_input.first ||
				    tribe_->ware_index("smoked_fish") == temp_input.first) {
					bo.substitute_inputs.insert(temp_input.first);
				}
			}
			// Creating vector with critical material, to be used to discourage
			// building of new sites if ware is lacking
			for (const auto& temp_buildcosts : train.buildcost()) {
				// building material except for trivial material
				if (wares.at(temp_buildcosts.first).refined_build_material) {
					bo.critical_building_material.push_back(temp_buildcosts.first);
				}
			}
			for (const auto& temp_buildcosts : train.enhancement_cost()) {
				// building material except for trivial material
				if (wares.at(temp_buildcosts.first).refined_build_material) {
					bo.critical_building_material.push_back(temp_buildcosts.first);
				}
			}
			continue;
		}

		if (bld.type() == Widelands::MapObjectType::CONSTRUCTIONSITE) {
			bo.type = BuildingObserver::Type::kConstructionsite;
			continue;
		}
	}

	// Forester/Ranger
	rangers_.clear();
	for (BuildingObserver& bo : buildings_) {
		if (bo.type != BuildingObserver::Type::kProductionsite) {
			continue;
		}
		const Widelands::ProductionSiteDescr* prodsite =
		   dynamic_cast<const Widelands::ProductionSiteDescr*>(
		      tribe_->get_building_descr(tribe_->building_index(bo.name)));
		for (const std::string& candidate : prodsite->supported_productionsites()) {
			for (const Widelands::ProductionSiteDescr* lumberjack : lumberjacks) {
				if (lumberjack->name() == candidate) {
					verb_log_dbg_time(gametime, "AI %d detected ranger: %s -> %s", player_number(),
					                  bo.name, lumberjack->name().c_str());
					bo.set_is(BuildingAttribute::kRanger);
					rangers_.push_back(bo);
					wood_policy_[bo.id] = WoodPolicy::kAllowRangers;
					for (const auto& attribute : prodsite->created_attributes()) {
						buildings_immovable_attributes_[attribute.second].insert(
						   ImmovableAttribute(bo.name, BuildingAttribute::kRanger));
					}
				}
			}
		}
	}
	lumberjacks.clear();

	// We must verify that some buildings has been identified
	// Also note that the AI assumes that some buildings are unique, if you want to
	// create e.g. two barracks or bakeries, the impact on the AI must be considered
	if (count_buildings_with_attribute(BuildingAttribute::kBarracks) != 1) {
		log_warn("The AI needs the tribe '%s' to define 1 type of barracks building. "
		         "This is the building that produces the tribe's 'soldier' worker.",
		         tribe_->name().c_str());
	}
	if (count_buildings_with_attribute(BuildingAttribute::kWell) > 1) {
		log_warn("The AI needs the tribe '%s' to define 1 type of well at the most. "
		         "This is the building that produces the ware 'water', has no inputs and mines "
		         "the 'resource_water'.",
		         tribe_->name().c_str());
	}
	if (count_buildings_with_attribute(BuildingAttribute::kHunter) > 1) {
		log_warn("The AI needs the tribe '%s' to define 1 type of hunter's building at the most. "
		         "Hunters are buildings that collect any bob from the map.",
		         tribe_->name().c_str());
	}

	if (count_buildings_with_attribute(BuildingAttribute::kFisher) != 1) {
		log_warn("The AI needs the tribe '%s' to define 1 type of fisher's building. "
		         "This is the building that collects the map resource 'resource_fish' and has "
		         "'needs_water' in its AI hints.",
		         tribe_->name().c_str());
	}

	// atlanteans they consider water as a resource
	// (together with mines, rocks and wood)
	// TODO(hessenfarmer): this should be moved from hardcoded to configurable / detectable
	if (tribe_->name() == "atlanteans" || tribe_->name() == "amazons") {
		resource_necessity_water_needed_ = true;
	}

	// Populating taskPool with all AI jobs and their starting times
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(0)),
	                                 SchedulerTaskId::kConstructBuilding, 6,
	                                 "construct a building"));
	taskPool.push_back(SchedulerTask(
	   std::max<Time>(gametime, Time(1000)), SchedulerTaskId::kRoadCheck, 2, "roads check"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(15 * 1000)),
	                                 SchedulerTaskId::kCheckProductionsites, 5,
	                                 "productionsites check"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(30 * 1000)),
	                                 SchedulerTaskId::kProductionsitesStats, 1,
	                                 "productionsites statistics"));
	taskPool.push_back(SchedulerTask(
	   std::max<Time>(gametime, Time(30 * 1000)), SchedulerTaskId::kCheckMines, 5, "check mines"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(0)),
	                                 SchedulerTaskId::kCheckMilitarysites, 5,
	                                 "check militarysites"));
	taskPool.push_back(SchedulerTask(
	   std::max<Time>(gametime, Time(30 * 1000)), SchedulerTaskId::kCheckShips, 5, "check ships"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(1000)),
	                                 SchedulerTaskId::kCheckEconomies, 1, "check economies"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(30 * 1000)),
	                                 SchedulerTaskId::KMarineDecisions, 5, "marine decisions"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(2 * 60 * 1000)),
	                                 SchedulerTaskId::kCheckTrainingsites, 5,
	                                 "check training sites"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(1000)),
	                                 SchedulerTaskId::kBbuildableFieldsCheck, 2,
	                                 "check buildable fields"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(1000)),
	                                 SchedulerTaskId::kMineableFieldsCheck, 2,
	                                 "check mineable fields"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(1000)),
	                                 SchedulerTaskId::kUnbuildableFCheck, 1,
	                                 "check unbuildable fields"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(15 * 60 * 1000)),
	                                 SchedulerTaskId::kWareReview, 9, "wares review"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(10 * 60 * 1000)),
	                                 SchedulerTaskId::kPrintStats, 9, "print statistics"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(60 * 1000)),
	                                 SchedulerTaskId::kCountMilitaryVacant, 2,
	                                 "count military vacant"));
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(10 * 60 * 1000)),
	                                 SchedulerTaskId::kCheckEnemySites, 6, "check enemy sites"));
	if (ai_training_mode_) {
		taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(10 * 1000)),
		                                 SchedulerTaskId::kManagementUpdate, 8, "reviewing"));
	}
	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(9 * 1000)),
	                                 SchedulerTaskId::kUpdateStats, 6, "update player stats"));
	taskPool.push_back(SchedulerTask(
	   std::max<Time>(gametime, Time(10 * 1000)), SchedulerTaskId::kUpdateStats, 15, "review"));

	taskPool.push_back(SchedulerTask(std::max<Time>(gametime, Time(10 * 1000)),
	                                 SchedulerTaskId::kWarehouseFlagDist, 5,
	                                 "Flag-Warehouse Update"));

	const Widelands::Map& map = game().map();

	// here we scan entire map for own ships
	std::set<Widelands::OPtr<Widelands::Ship>> found_ships;
	for (int16_t y = 0; y < map.get_height(); ++y) {
		for (int16_t x = 0; x < map.get_width(); ++x) {
			Widelands::FCoords f = map.get_fcoords(Widelands::Coords(x, y));
			// there are too many bobs on the map so we investigate
			// only bobs on water
			if (f.field->nodecaps() & Widelands::MOVECAPS_SWIM) {
				for (Widelands::Bob* bob = f.field->get_first_bob(); bob;
				     bob = bob->get_next_on_field()) {
					if (upcast(Widelands::Ship, ship, bob)) {
						if (ship->get_owner() == player_ && !found_ships.count(ship)) {
							found_ships.insert(ship);
							gain_ship(*ship, NewShip::kFoundOnLoad);
						}
					}
				}
			}
		}
	}

	// here we scan entire map for owned unused fields and own buildings
	std::set<Widelands::OPtr<Widelands::PlayerImmovable>> found_immovables;
	for (int16_t y = 0; y < map.get_height(); ++y) {
		for (int16_t x = 0; x < map.get_width(); ++x) {
			Widelands::FCoords f = map.get_fcoords(Widelands::Coords(x, y));

			if (f.field->get_owned_by() != player_number()) {
				continue;
			}

			unusable_fields.push_back(f);

			if (upcast(Widelands::PlayerImmovable, imm, f.field->get_immovable())) {
				//  Guard by a set - immovables might be on several nodes at once.
				if (&imm->owner() == player_ && !found_immovables.count(imm)) {
					found_immovables.insert(imm);
					gain_immovable(*imm, true);
				}
			}
		}
	}

	// blocking space consumers vicinity (when reloading a game)
	for (const ProductionSiteObserver& ps_obs : productionsites) {
		if (ps_obs.bo->is(BuildingAttribute::kSpaceConsumer) &&
		    !ps_obs.bo->is(BuildingAttribute::kRanger)) {
			Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
			   map,
			   Widelands::Area<Widelands::FCoords>(map.get_fcoords(ps_obs.site->get_position()), 4));
			do {
				blocked_fields.add(mr.location(), game().get_gametime() + Duration(20 * 60 * 1000));
			} while (mr.advance(map));
		}
	}

	// getting list of all fields nearby port space and a list of outer fields for shipyard priority
	// TODO(tiborb): it seems port spaces can change over time so ports_vicinity needs to be
	// refreshed from
	// time to time
	for (const Widelands::Coords& c : map.get_port_spaces()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   map, Widelands::Area<Widelands::FCoords>(map.get_fcoords(c), 3));
		do {
			const uint32_t hash = mr.location().hash();
			ports_vicinity.insert(hash);
		} while (mr.advance(map));
		Widelands::HollowArea<> ha(Widelands::Area<>(map.get_fcoords(c), 8), 3);
		Widelands::MapHollowRegion<> mhr(map, ha);
		do {
			const uint32_t hash = mhr.location().hash();
			ports_shipyard_region.insert(hash);
		} while (mhr.advance(map));
	}

	// printing identified basic buildings if we are in the basic economy mode
	basic_economy_established = persistent_data->remaining_basic_buildings.empty();
	if (!basic_economy_established) {
		verb_log_dbg_time(gametime,
		                  "%2d: Initializing in the basic economy mode, required buildings:\n",
		                  player_number());
		for (auto bb : persistent_data->remaining_basic_buildings) {
			verb_log_dbg_time(gametime, "   %3d / %-28s- target %d\n", bb.first,
			                  get_building_observer(bb.first).name, bb.second);
		}
	}

	update_player_stat(gametime);

	// Initialise the max duration of a single ship's expedition
	const uint32_t map_area = uint32_t(map.get_height()) * map.get_width();
	const uint32_t map_area_root = round(sqrt(map_area));
	int scope = 320 - 64;
	int off = map_area_root - 64;
	if (off < 0) {
		off = 0;
	}
	if (off > scope) {
		off = scope;
	}
	expedition_max_duration =
	   kExpeditionMinDuration +
	   Duration(static_cast<double>(off) * (kExpeditionMaxDuration - kExpeditionMinDuration).get() /
	            scope);
	verb_log_dbg_time(gametime,
	                  " %d: expedition max duration = %u (%u minutes), map area root: %u\n",
	                  player_number(), expedition_max_duration.get() / 1000,
	                  expedition_max_duration.get() / (60 * 1000), map_area_root);
	assert(expedition_max_duration >= kExpeditionMinDuration);
	assert(expedition_max_duration <= kExpeditionMaxDuration);

	// Sometimes there can be a ship in expedition, but expedition start time is not given
	// e.g. human player played this player before
	if (expedition_ship_ != kNoShip && persistent_data->expedition_start_time ==
	                                      Widelands::Player::AiPersistentState::kNoExpedition) {
		// Current gametime is better then 'Player::AiPersistentState::kNoExpedition'
		persistent_data->expedition_start_time = gametime;
	}

	productionsites_ratio_ = management_data.get_military_number_at(86) / 10 + 12;

	// Just to be initialized
	soldier_status_ = SoldiersStatus::kEnough;
	vacant_mil_positions_average_ = 0;
	spots_avail.resize(4);
	trees_nearby_treshold_ = 3 + std::abs(management_data.get_military_number_at(121)) / 2;
	last_road_dismantled_ = Time(0);
	dead_ends_check_ = true;
	last_attack_time_ = Time(0);
}

/**
 * Checks PART of available buildable fields.
 *
 * this checks about 40-50 buildable fields. In big games, the player can have thousands
 * of them, so we rotate the buildable_fields container and check 35 fields, and in addition
 * we look for medium & big fields and near border fields if needed.
 */
void DefaultAI::update_all_buildable_fields(const Time& gametime) {

	// Every call we try to check first 35 buildable fields
	constexpr uint16_t kMinimalFieldsCheck = 35;

	uint16_t i = 0;

	// To be sure we have some info about enemies we might see
	update_player_stat(gametime);

	// Generally we check fields as they are in the container, but we need also given
	// number of "special" fields. So if given number of fields are not found within
	// "regular" check, we must go on and look also on other fields...
	uint8_t non_small_needed = 4;
	uint8_t near_border_needed = 10;

	// we test 35 fields that were update more than 1 seconds ago
	while (!buildable_fields.empty() &&
	       i < std::min<uint16_t>(kMinimalFieldsCheck, buildable_fields.size())) {
		BuildableField& bf = *buildable_fields.front();

		if ((buildable_fields.front()->field_info_expiration - kFieldInfoExpiration +
		     Duration(1000)) <= gametime) {

			//  check whether we lost ownership of the node
			if (bf.coords.field->get_owned_by() != player_number()) {
				delete &bf;
				buildable_fields.pop_front();
				continue;
			}

			//  check whether we can still construct regular buildings on the node
			if ((player_->get_buildcaps(bf.coords) & Widelands::BUILDCAPS_SIZEMASK) == 0) {
				unusable_fields.push_back(bf.coords);
				delete &bf;
				buildable_fields.pop_front();
				continue;
			}

			update_buildable_field(bf);
			if (non_small_needed > 0) {
				int32_t const maxsize =
				   player_->get_buildcaps(bf.coords) & Widelands::BUILDCAPS_SIZEMASK;
				if (maxsize > 1) {
					--non_small_needed;
				}
			}
			if (near_border_needed > 0) {
				if (bf.near_border) {
					--near_border_needed;
				}
			}
		}
		bf.field_info_expiration = gametime + kFieldInfoExpiration;
		buildable_fields.push_back(&bf);
		buildable_fields.pop_front();

		++i;
	}

	// If needed we iterate once more and look for 'special' fields
	// starting in the middle of buildable_fields to skip fields tested lately
	// But not doing this if the count of buildable fields is too low
	// (no need to bother)
	if (buildable_fields.size() < kMinimalFieldsCheck * 3) {
		return;
	}

	for (uint32_t j = buildable_fields.size() / 2; j < buildable_fields.size(); j++) {
		// If we dont need to iterate (anymore) ...
		if (non_small_needed + near_border_needed == 0) {
			break;
		}

		// Skip if the field is not ours or was updated lately
		if (buildable_fields[j]->coords.field->get_owned_by() != player_number()) {
			continue;
		}
		// We are not interested in fields where info has expired less than 20s ago
		if (buildable_fields[j]->field_info_expiration + Duration(20 * 1000) > gametime) {
			continue;
		}

		// Continue if field is blocked at the moment
		if (blocked_fields.is_blocked(buildable_fields[j]->coords)) {
			continue;
		}

		// Few constants to keep the code cleaner
		const int32_t field_maxsize =
		   player_->get_buildcaps(buildable_fields[j]->coords) & Widelands::BUILDCAPS_SIZEMASK;
		const bool field_near_border = buildable_fields[j]->near_border;

		// Let decide if we need to update and for what reason
		const bool update_due_size = non_small_needed && field_maxsize > 1;
		const bool update_due_border = near_border_needed && field_near_border;

		if (!(update_due_size || update_due_border)) {
			continue;
		}

		// decreasing the counters
		if (update_due_size) {
			assert(non_small_needed > 0);
			--non_small_needed;
		}
		if (update_due_border) {
			assert(near_border_needed > 0);
			--near_border_needed;
		}

		// and finnaly update the buildable field
		update_buildable_field(*buildable_fields[j]);
		buildable_fields[j]->field_info_expiration = gametime + kFieldInfoExpiration;
	}
}

/**
 * Checks ALL available mineable fields.
 *
 * this shouldn't be used often, as it might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_mineable_fields(const Time& gametime) {

	uint16_t i = 0;  // counter, used to track # of checked fields

	// we test 30 fields that were updated more than 1 seconds ago
	// to avoid re-test of the same field twice
	while (!mineable_fields.empty() &&
	       (mineable_fields.front()->field_info_expiration - kMineFieldInfoExpiration +
	        Duration(1000)) <= gametime &&
	       i < 30) {
		MineableField* mf = mineable_fields.front();

		//  check whether we lost ownership of the node
		if (mf->coords.field->get_owned_by() != player_number()) {
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player_->get_buildcaps(mf->coords) & Widelands::BUILDCAPS_MINE) == 0) {
			unusable_fields.push_back(mf->coords);
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		update_mineable_field(*mf);
		mf->field_info_expiration = gametime + kMineFieldInfoExpiration;
		mineable_fields.push_back(mf);
		mineable_fields.pop_front();

		++i;
	}
	// Updating overall statistics, first we flush the data and then iterate over all mine fields
	// ignoring fields that are blocked usually because they are not accessible
	mine_fields_stat.zero();
	for (const auto& mineable_field : mineable_fields) {
		if (mineable_field->coords.field->get_resources_amount() > 0 &&
		    !blocked_fields.is_blocked(mineable_field->coords)) {
			mine_fields_stat.add(mineable_field->coords.field->get_resources());
		}
	}

	// Following asserts presume that there are 1-3 critical mines
	if (mine_fields_stat.count_types() == kMineTypes) {
		assert(mine_fields_stat.has_critical_ore_fields());
	}
	if (mine_fields_stat.count_types() == 0) {
		assert(!mine_fields_stat.has_critical_ore_fields());
	}
}

/**
 * Checks up to 50 fields that weren't buildable the last time.
 *
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_not_buildable_fields() {
	int32_t const pn = player_number();

	// We are checking at least 5 unusable fields (or less if there are not 5 of them)
	// at once, but not more then 200...
	// The idea is to check each field at least once a minute, of course with big maps
	// it will take longer
	uint32_t maxchecks = unusable_fields.size();
	if (maxchecks > 5) {
		maxchecks = std::min<uint32_t>(5 + (unusable_fields.size() - 5) / 15, 200);
	}

	for (uint32_t i = 0; i < maxchecks; ++i) {
		//  check whether we lost ownership of the node
		if (unusable_fields.front().field->get_owned_by() != pn) {
			unusable_fields.pop_front();
			continue;
		}

		// check whether building capabilities have improved
		if (player_->get_buildcaps(unusable_fields.front()) & Widelands::BUILDCAPS_SIZEMASK) {
			buildable_fields.push_back(new BuildableField(unusable_fields.front()));
			unusable_fields.pop_front();
			update_buildable_field(*buildable_fields.back());
			continue;
		}

		if (player_->get_buildcaps(unusable_fields.front()) & Widelands::BUILDCAPS_MINE) {
			mineable_fields.push_back(new MineableField(unusable_fields.front()));
			unusable_fields.pop_front();
			update_mineable_field(*mineable_fields.back());
			continue;
		}

		unusable_fields.push_back(unusable_fields.front());
		unusable_fields.pop_front();
	}
}

/// Updates one buildable field
void DefaultAI::update_buildable_field(BuildableField& field) {
	// look if there is any unowned land nearby
	const Widelands::Map& map = game().map();
	const Time& gametime = game().get_gametime();
	FindNodeUnownedWalkable find_unowned_walkable(player_, game());
	FindEnemyNodeWalkable find_enemy_owned_walkable(player_, game());
	FindNodeUnownedBuildable find_unowned_buildable(player_, game());
	FindNodeUnownedMineable find_unowned_mines_pots(player_, game());
	FindNodeUnownedMineable find_unowned_iron_mines(player_, game(), iron_resource_id);
	FindNodeAllyOwned find_ally(player_, game(), player_number());
	Widelands::PlayerNumber const pn = player_->player_number();
	const Widelands::Descriptions& descriptions = game().descriptions();

	constexpr uint16_t kProductionArea = 6;
	constexpr uint16_t kBuildableSpotsCheckArea = 10;
	constexpr uint16_t kEnemyCheckArea = 16;
	const uint16_t ms_enemy_check_area =
	   kEnemyCheckArea + std::abs(management_data.get_military_number_at(75)) / 10;
	constexpr uint16_t kDistantResourcesArea = 20;

	uint16_t actual_enemy_check_area = kEnemyCheckArea;
	field.is_militarysite = false;
	if (field.coords.field->get_immovable()) {
		if (field.coords.field->get_immovable()->descr().type() ==
		    Widelands::MapObjectType::MILITARYSITE) {
			field.is_militarysite = true;
			actual_enemy_check_area = ms_enemy_check_area;
		}
	}

	field.unowned_land_nearby = map.find_fields(
	   game(), Widelands::Area<Widelands::FCoords>(field.coords, actual_enemy_check_area), nullptr,
	   find_unowned_walkable);

	field.enemy_owned_land_nearby = map.find_fields(
	   game(), Widelands::Area<Widelands::FCoords>(field.coords, actual_enemy_check_area), nullptr,
	   find_enemy_owned_walkable);

	field.nearest_buildable_spot_nearby = std::numeric_limits<uint16_t>::max();
	field.unowned_buildable_spots_nearby = 0;
	field.unowned_portspace_vicinity_nearby = 0;
	if (field.unowned_land_nearby > 0 ||
	    (field.enemy_owned_land_nearby > 0 &&
	     field.enemy_military_presence <
	        std::abs(management_data.get_military_number_at(174)) / 10)) {
		std::vector<Widelands::Coords> found_buildable_fields;

		// first looking for unowned buildable spots
		field.unowned_buildable_spots_nearby = map.find_fields(
		   game(), Widelands::Area<Widelands::FCoords>(field.coords, kBuildableSpotsCheckArea),
		   &found_buildable_fields, find_unowned_buildable);
		field.unowned_buildable_spots_nearby += map.find_fields(
		   game(), Widelands::Area<Widelands::FCoords>(field.coords, kBuildableSpotsCheckArea),
		   &found_buildable_fields, find_enemy_owned_walkable);
		// Now iterate over fields to collect statistics
		for (auto& coords : found_buildable_fields) {
			// We are not interested in blocked fields
			if (blocked_fields.is_blocked(coords)) {
				continue;
			}
			// And now looking for nearest field
			const uint32_t cur_distance = map.calc_distance(coords, field.coords);
			if (cur_distance < field.nearest_buildable_spot_nearby) {
				field.nearest_buildable_spot_nearby = cur_distance;
			}
		}

		// now looking for unowned_portspace_vicinity_nearby
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   map, Widelands::Area<Widelands::FCoords>(field.coords, kBuildableSpotsCheckArea));
		do {

			if (mr.location().field->get_owned_by() == 0 &&
			    ports_vicinity.count(mr.location().hash()) > 0) {
				++field.unowned_portspace_vicinity_nearby;
			}
		} while (mr.advance(map));
	}

	// Is this near the border? Get rid of fields owned by ally
	field.near_border =
	   (map.find_fields(
	       game(), Widelands::Area<Widelands::FCoords>(field.coords, 3), nullptr, find_ally) ||
	    map.find_fields(game(), Widelands::Area<Widelands::FCoords>(field.coords, 3), nullptr,
	                    find_unowned_walkable) > 0);

	// are we going to count resources now?
	static bool resource_count_now = false;
	resource_count_now = false;
	// Testing in first 10 seconds or if last testing was more then 60 sec ago
	if (field.last_resources_check_time < Time(10000) ||
	    gametime > field.last_resources_check_time + Duration(60 * 1000)) {
		resource_count_now = true;
		field.last_resources_check_time = gametime;
	}

	// testing mines
	if (resource_count_now) {
		uint32_t close_mines =
		   map.find_fields(game(), Widelands::Area<Widelands::FCoords>(field.coords, kProductionArea),
		                   nullptr, find_unowned_mines_pots);
		uint32_t distant_mines = map.find_fields(
		   game(), Widelands::Area<Widelands::FCoords>(field.coords, kDistantResourcesArea), nullptr,
		   find_unowned_mines_pots);
		distant_mines = distant_mines - close_mines;
		field.unowned_mines_spots_nearby = 4 * close_mines + distant_mines / 2;
		if (distant_mines > 0) {
			field.unowned_mines_spots_nearby += 15;
		}
		if (field.unowned_mines_spots_nearby > 0 &&
		    // for performance considerations we count iron nodes only if we have less than 2 iron
		    // mines now...
		    mines_per_type[iron_resource_id].total_count() <= 1) {
			// counting iron mines, if we have less than two iron mines
			field.unowned_iron_mines_nearby = map.find_fields(
			   game(), Widelands::Area<Widelands::FCoords>(field.coords, kDistantResourcesArea),
			   nullptr, find_unowned_iron_mines);
		} else {
			field.unowned_iron_mines_nearby = 0;
		}
	}

	// identifying portspace fields
	if (player_->get_buildcaps(field.coords) & Widelands::BUILDCAPS_PORT) {
		field.is_portspace = ExtendedBool::kTrue;
	} else {
		field.is_portspace = ExtendedBool::kFalse;
	}

	// testing for near portspaces
	if (ports_vicinity.count(field.coords.hash()) > 0) {
		field.portspace_nearby = ExtendedBool::kTrue;
	} else {
		field.portspace_nearby = ExtendedBool::kFalse;
	}

	// testing if shipyard should be buildable
	if (ports_shipyard_region.count(field.coords.hash()) > 0) {
		field.shipyard_preferred = ExtendedBool::kTrue;
	} else {
		field.shipyard_preferred = ExtendedBool::kFalse;
	}

	// testing if a port is nearby, such field will get a priority boost
	if (resource_count_now) {  // misusing a bit
		uint16_t nearest_distance = std::numeric_limits<uint16_t>::max();
		for (const WarehouseSiteObserver& wh_obs : warehousesites) {
			if (wh_obs.bo->is(BuildingAttribute::kPort)) {
				const uint16_t actual_distance =
				   map.calc_distance(field.coords, wh_obs.site->get_position());
				nearest_distance = std::min(nearest_distance, actual_distance);
			}
		}
		field.port_nearby = (nearest_distance < 15);
	}

	// testing fields in radius 1 to find biggest buildcaps.
	// This is to calculate capacity that will be lost if something is
	// built here
	field.max_buildcap_nearby = 0;
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   map, Widelands::Area<Widelands::FCoords>(field.coords, 1));
	do {
		if ((player_->get_buildcaps(mr.location()) & Widelands::BUILDCAPS_SIZEMASK) >
		    field.max_buildcap_nearby) {
			field.max_buildcap_nearby =
			   player_->get_buildcaps(mr.location()) & Widelands::BUILDCAPS_SIZEMASK;
		}
	} while (mr.advance(map));

	assert((player_->get_buildcaps(field.coords) & Widelands::BUILDCAPS_SIZEMASK) <=
	       field.max_buildcap_nearby);

	// Testing surface water (once only)
	// TODO(GunChleoc): We can change the terrain by scripting, so we should work with notifications
	// here.
	// Let's leave this as it is for now for performance reasons - terrain change of water is
	// currently only
	// used in the Atlantean scenario.
	if (field.water_nearby == kUncalculated) {
		assert(field.open_water_nearby == kUncalculated);

		FindNodeWater find_water(game().descriptions());
		field.water_nearby =
		   map.find_fields(game(), Widelands::Area<Widelands::FCoords>(field.coords, kProductionArea),
		                   nullptr, find_water);

		if (field.water_nearby > 0) {
			FindNodeOpenWater find_open_water(game().descriptions());
			field.open_water_nearby = map.find_fields(
			   game(), Widelands::Area<Widelands::FCoords>(field.coords, kProductionArea), nullptr,
			   find_open_water);
		}

		if (resource_necessity_water_needed_) {  // for atlanteans
			field.distant_water =
			   map.find_fields(
			      game(), Widelands::Area<Widelands::FCoords>(field.coords, kDistantResourcesArea),
			      nullptr, find_water) -
			   field.water_nearby;
			assert(field.open_water_nearby <= field.water_nearby);
		}
	}

	Widelands::FCoords fse;
	map.get_neighbour(field.coords, Widelands::WALK_SE, &fse);
	field.preferred = false;
	if (Widelands::BaseImmovable const* const imm = fse.field->get_immovable()) {
		if ((imm->descr().type() == Widelands::MapObjectType::FLAG ||
		     imm->descr().type() == Widelands::MapObjectType::ROAD) &&
		    (fse.field->nodecaps() & Widelands::BUILDCAPS_FLAG)) {
			field.preferred = true;
		}
	}

	// counting fields with fish, doing it roughly every 10-th minute is enough
	if (field.water_nearby > 0 &&
	    (field.fish_nearby == kUncalculated || (resource_count_now && gametime.get() % 10 == 0))) {
		Widelands::CheckStepWalkOn fisher_cstep(Widelands::MOVECAPS_WALK, true);
		static std::vector<Widelands::Coords> fish_fields_list;  // pity this contains duplicates
		fish_fields_list.clear();
		map.find_reachable_fields(
		   game(), Widelands::Area<Widelands::FCoords>(field.coords, kProductionArea),
		   &fish_fields_list, fisher_cstep,
		   Widelands::FindNodeResource(descriptions.resource_index("resource_fish")));

		// This is "list" of unique fields in fish_fields_list we got above
		static std::set<Widelands::Coords> counted_fields;
		counted_fields.clear();
		field.fish_nearby = 0;
		for (auto fish_coords : fish_fields_list) {
			if (counted_fields.insert(fish_coords).second) {
				field.fish_nearby += map.get_fcoords(fish_coords).field->get_resources_amount();
			}
		}
	}

	// Counting resources that do not change fast
	if (resource_count_now) {
		// Counting fields with critters (game)
		field.critters_nearby =
		   map.find_bobs(game(), Widelands::Area<Widelands::FCoords>(field.coords, kProductionArea),
		                 nullptr, Widelands::FindBobCritter());

		// Counting trees, rocks, berry bushes nearby
		for (const auto& attribute_category : buildings_immovable_attributes_) {
			for (const ImmovableAttribute& attribute_info : attribute_category.second) {
				// Rocks are not renewable, we will count them only if they were never counted or
				// previous state is nonzero. This is done to save some work for AI (CPU utilization).
				if (attribute_info.building_attribute == BuildingAttribute::kNeedsRocks) {
					auto needs_rocks_it =
					   field.immovables_by_attribute_nearby.find(BuildingAttribute::kNeedsRocks);
					if (needs_rocks_it != field.immovables_by_attribute_nearby.end() &&
					    needs_rocks_it->second == 0) {
						continue;
					}
				}
				uint8_t amount = map.find_immovables(
				   game(),
				   Widelands::Area<Widelands::FCoords>(map.get_fcoords(field.coords), kProductionArea),
				   nullptr, Widelands::FindImmovableAttribute(attribute_category.first));

				// adding 2 if rocks found
				if (attribute_info.building_attribute == BuildingAttribute::kNeedsRocks && amount) {
					amount += 2;
				}

				field.immovables_by_attribute_nearby[attribute_info.building_attribute] = amount;
				field.immovables_by_name_nearby[attribute_info.building_name] = amount;
			}
		}

		// ground water is not renewable and its amount can only fall, we will count them only if
		// previous state is nonzero
		if (field.ground_water > 0) {
			field.ground_water = field.coords.field->get_resources_amount();
		}
	}

	// resetting some values
	field.enemy_nearby =
	   field.enemy_owned_land_nearby > std::abs(management_data.get_military_number_at(41) / 4);
	if (field.enemy_owned_land_nearby == 0) {
		assert(!field.enemy_nearby);
	}

	// resetting a bunch of values for the field
	field.ally_military_presence = 0;
	field.area_military_capacity = 0;
	field.consumers_nearby.clear();
	field.consumers_nearby.resize(wares.size());
	field.enemy_military_presence = 0;
	field.enemy_military_sites = 0;
	field.enemy_wh_nearby = false;
	field.military_in_constr_nearby = 0;
	field.military_loneliness = 1000;
	field.military_stationed = 0;
	field.military_unstationed = 0;
	field.own_military_presence = 0;
	field.own_non_military_nearby = 0;
	field.producers_nearby.clear();
	field.supported_producers_nearby.clear();
	field.buildings_nearby.clear();
	field.producers_nearby.resize(wares.size());
	field.rangers_nearby = 0;
	field.space_consumers_nearby = 0;
	field.supporters_nearby.clear();
	field.unconnected_nearby = false;

	// collect information about productionsites nearby
	static std::vector<Widelands::ImmovableFound> immovables;
	immovables.reserve(50);
	immovables.clear();
	// Search in a radius of range
	map.find_immovables(
	   game(), Widelands::Area<Widelands::FCoords>(field.coords, kProductionArea + 2), &immovables);

	// function seems to return duplicates, so we will use serial numbers to filter them out
	static std::set<uint32_t> unique_serials;
	unique_serials.clear();

	for (const Widelands::ImmovableFound& imm_found : immovables) {
		const Widelands::BaseImmovable& base_immovable = *imm_found.object;
		if (!unique_serials.insert(base_immovable.serial()).second) {
			continue;  // serial was not inserted in the set, so this is a duplicate
		}

		if (upcast(Widelands::PlayerImmovable const, player_immovable, &base_immovable)) {

			// TODO(unknown): Only continue if this is an opposing site
			// allied sites should be counted for military influence
			if (player_immovable->owner().player_number() != pn) {
				continue;
			}
			// here we identify the buiding (including expected building if constructionsite)
			// and calculate some statistics about nearby buildings
			if (player_immovable->descr().type() == Widelands::MapObjectType::PRODUCTIONSITE) {
				BuildingObserver& bo = get_building_observer(player_immovable->descr().name().c_str());
				consider_productionsite_influence(field, imm_found.coords, bo);
			} else if (upcast(Widelands::ConstructionSite const, constructionsite, player_immovable)) {
				const Widelands::BuildingDescr& target_descr = constructionsite->building();
				if (target_descr.type() == Widelands::MapObjectType::PRODUCTIONSITE) {
					BuildingObserver& bo = get_building_observer(target_descr.name().c_str());
					consider_productionsite_influence(field, imm_found.coords, bo);
				}
			}
		}
	}

	// Now testing military aspects
	immovables.clear();
	map.find_immovables(game(),
	                    Widelands::Area<Widelands::FCoords>(field.coords, actual_enemy_check_area),
	                    &immovables);

	// We are interested in unconnected immovables, but we must be also close to connected ones
	static bool any_connected_imm = false;
	any_connected_imm = false;
	static bool any_unconnected_imm = false;
	any_unconnected_imm = false;
	unique_serials.clear();

	for (const Widelands::ImmovableFound& imm_found : immovables) {
		const Widelands::BaseImmovable& base_immovable = *imm_found.object;

		if (!unique_serials.insert(base_immovable.serial()).second) {
			continue;  // serial was not inserted in the set, so this is duplicate
		}

		// testing if immovable is owned by someone else and collecting some statistics
		if (upcast(Widelands::Building const, building, &base_immovable)) {

			const Widelands::PlayerNumber bpn = building->owner().player_number();
			if (player_statistics.get_is_enemy(bpn)) {  // owned by enemy
				assert(!player_statistics.players_in_same_team(bpn, pn));
				field.enemy_nearby = true;
				if (upcast(Widelands::MilitarySite const, militarysite, building)) {
					field.enemy_military_presence +=
					   militarysite->soldier_control()->stationed_soldiers().size();
					++field.enemy_military_sites;
				}
				if (upcast(Widelands::ConstructionSite const, constructionsite, building)) {
					const Widelands::BuildingDescr& target_descr = constructionsite->building();
					if (target_descr.type() == Widelands::MapObjectType::MILITARYSITE) {
						++field.enemy_military_sites;
					}
				}

				// Warehouses are counted here too as they can host soldiers as well
				if (upcast(Widelands::Warehouse const, warehouse, building)) {
					field.enemy_military_presence +=
					   warehouse->soldier_control()->stationed_soldiers().size();
					++field.enemy_military_sites;
					field.enemy_wh_nearby = true;
					enemy_warehouses.insert(building->get_position().hash());
				}
				continue;
			} else if (bpn != pn) {  // it is an ally
				assert(!player_statistics.get_is_enemy(bpn));
				if (upcast(Widelands::MilitarySite const, militarysite, building)) {
					field.ally_military_presence +=
					   militarysite->soldier_control()->stationed_soldiers().size();
				}
				continue;
			}

			// if we are here, the immovable is ours
			assert(building->owner().player_number() == pn);

			// connected to a warehouse
			// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
			// ferries
			bool connected = !building->get_economy(Widelands::wwWORKER)->warehouses().empty();
			if (connected) {
				any_connected_imm = true;
			}

			if (upcast(Widelands::ConstructionSite const, constructionsite, building)) {
				const Widelands::BuildingDescr& target_descr = constructionsite->building();

				if (upcast(Widelands::MilitarySiteDescr const, target_ms_d, &target_descr)) {
					const int32_t dist = map.calc_distance(field.coords, imm_found.coords);
					const int32_t radius = target_ms_d->get_conquers() + 4;

					if (radius > dist) {
						field.area_military_capacity += target_ms_d->get_max_number_of_soldiers() / 2 + 1;
						if (field.coords != imm_found.coords) {
							field.military_loneliness *= static_cast<double_t>(dist) / radius;
						}
						++field.military_in_constr_nearby;
					}
				}
			} else if (!connected) {
				// we don't care about unconnected constructionsites
				any_unconnected_imm = true;
			}

			if (upcast(Widelands::MilitarySite const, militarysite, building)) {
				const int32_t dist = map.calc_distance(field.coords, imm_found.coords);
				const int32_t radius = militarysite->descr().get_conquers() + 4;

				if (radius > dist) {
					field.area_military_capacity +=
					   militarysite->soldier_control()->max_soldier_capacity();
					field.own_military_presence +=
					   militarysite->soldier_control()->stationed_soldiers().size();

					if (militarysite->soldier_control()->stationed_soldiers().empty()) {
						++field.military_unstationed;
					} else {
						++field.military_stationed;
					}

					if (field.coords != imm_found.coords) {
						field.military_loneliness *= static_cast<double_t>(dist) / radius;
					}
				}
			} else {
				++field.own_non_military_nearby;
			}
		}
	}

	assert(field.military_loneliness <= 1000);

	if (any_unconnected_imm && any_connected_imm && field.military_in_constr_nearby == 0) {
		field.unconnected_nearby = true;
	}

	// if there is a militarysite on the field, we try to walk to enemy
	field.enemy_accessible_ = false;
	field.local_soldier_capacity = 0;
	if (field.is_militarysite) {
		if (upcast(Widelands::MilitarySite, ms, field.coords.field->get_immovable())) {
			if (field.enemy_nearby) {
				uint32_t unused1 = 0;
				uint16_t unused2 = 0;
				field.enemy_accessible_ = other_player_accessible(
				   actual_enemy_check_area + 3, &unused1, &unused2, field.coords, WalkSearch::kEnemy);
			}
			field.local_soldier_capacity = ms->soldier_control()->max_soldier_capacity();
			field.is_militarysite = true;
		} else {
			NEVER_HERE();
		}
	}

	// Calculating field score
	field.military_score_ = 0;
	field.inland = false;

	if (!(field.enemy_nearby || field.near_border)) {
		field.inland = true;
	}

	const uint8_t score_parts_size = 69;
	int32_t score_parts[score_parts_size] = {0};
	if (field.enemy_owned_land_nearby) {
		score_parts[0] = 3 * management_data.neuron_pool[73].get_result_safe(
		                        field.enemy_owned_land_nearby / 5, kAbsValue);
		score_parts[1] = 3 * management_data.neuron_pool[76].get_result_safe(
		                        field.enemy_owned_land_nearby, kAbsValue);
		score_parts[2] = 3 * management_data.neuron_pool[54].get_result_safe(
		                        field.enemy_military_presence * 2, kAbsValue);
		score_parts[3] = 3 * management_data.neuron_pool[61].get_result_safe(
		                        field.enemy_military_presence / 3, kAbsValue);
		score_parts[4] =
		   (!field.enemy_accessible_) ? (-100 + management_data.get_military_number_at(55)) : 0;
		score_parts[5] = 2 * management_data.neuron_pool[50].get_result_safe(
		                        field.enemy_owned_land_nearby, kAbsValue);

		score_parts[6] =
		   field.enemy_military_sites * std::abs(management_data.get_military_number_at(67) / 2);
		score_parts[7] = 2 * management_data.neuron_pool[34].get_result_safe(
		                        field.enemy_military_sites * 2, kAbsValue);
		score_parts[8] = management_data.neuron_pool[56].get_result_safe(
		   field.enemy_military_presence * 2, kAbsValue);

		score_parts[9] = management_data.neuron_pool[65].get_result_safe(
		   (field.unowned_land_nearby + field.enemy_owned_land_nearby) / 2, kAbsValue);
		score_parts[10] = (field.enemy_accessible_) ? management_data.get_military_number_at(80) : 0;

		score_parts[11] =
		   -3 * management_data.neuron_pool[8].get_result_safe(
		           (field.military_in_constr_nearby + field.military_unstationed) * 3, kAbsValue);
		score_parts[12] =
		   -3 * management_data.neuron_pool[74].get_result_safe(
		           (field.military_in_constr_nearby + field.military_unstationed) * 5, kAbsValue);
		score_parts[13] = ((field.military_in_constr_nearby + field.military_unstationed) > 0) ?
                           -std::abs(management_data.get_military_number_at(32)) :
                           0;
		score_parts[14] = -1 * (field.military_in_constr_nearby + field.military_unstationed) *
		                  std::abs(management_data.get_military_number_at(12));

		score_parts[15] =
		   -2 * management_data.neuron_pool[75].get_result_safe(field.own_military_presence);
		score_parts[16] = -5 * std::min<int16_t>(field.area_military_capacity, 20);
		score_parts[17] = 3 * management_data.get_military_number_at(28);
		score_parts[18] =
		   (field.enemy_nearby) ? 3 * std::abs(management_data.get_military_number_at(68)) : 0;
		score_parts[19] =
		   (field.enemy_wh_nearby) ? 3 * std::abs(management_data.get_military_number_at(132)) : 0;
		score_parts[64] = (field.enemy_wh_nearby) ?
                           std::abs(management_data.get_military_number_at(135)) :
                           -std::abs(management_data.get_military_number_at(135));

	} else {  // for expansion or inner land

		score_parts[20] = management_data.neuron_pool[22].get_result_safe(
		   (field.unowned_mines_spots_nearby + 2) / 3, kAbsValue);
		score_parts[21] = (field.unowned_mines_spots_nearby > 0) ?
                           std::abs(management_data.get_military_number_at(58)) :
                           0;
		if (expansion_type.get_expansion_type() == ExpansionMode::kResources) {
			score_parts[23] = 2 * management_data.neuron_pool[78].get_result_safe(
			                         (field.unowned_mines_spots_nearby + 2) / 3, kAbsValue);
		}

		score_parts[24] =
		   (field.unowned_land_nearby) ?
            management_data.neuron_pool[25].get_result_safe(field.water_nearby / 2, kAbsValue) :
            0;
		score_parts[25] =
		   (field.unowned_land_nearby) ?
            management_data.neuron_pool[27].get_result_safe(
		         std::max(field.immovables_by_attribute_nearby[BuildingAttribute::kLumberjack],
		                  field.immovables_by_attribute_nearby[BuildingAttribute::kRanger]) /
		            2,
		         kAbsValue) :
            0;

		if (resource_necessity_water_needed_) {
			score_parts[26] =
			   (field.unowned_land_nearby) ?
               management_data.neuron_pool[15].get_result_safe(field.water_nearby, kAbsValue) :
               0;
			score_parts[27] =
			   resource_necessity_water_needed_ *
			   management_data.neuron_pool[17].get_result_safe(field.distant_water, kAbsValue) / 100;
		}
		score_parts[28] =
		   (field.unowned_land_nearby) ?
            management_data.neuron_pool[33].get_result_safe(field.water_nearby, kAbsValue) :
            0;
		score_parts[29] =
		   management_data.neuron_pool[10].get_result_safe(field.military_loneliness / 50, kAbsValue);

		score_parts[30] =
		   -10 * management_data.neuron_pool[8].get_result_safe(
		            3 * (field.military_in_constr_nearby + field.military_unstationed), kAbsValue);
		score_parts[31] =
		   -10 * management_data.neuron_pool[31].get_result_safe(
		            3 * (field.military_in_constr_nearby + field.military_unstationed), kAbsValue);
		score_parts[32] = -4 * field.military_in_constr_nearby *
		                  std::abs(management_data.get_military_number_at(82));
		score_parts[33] = (field.military_in_constr_nearby > 0) ?
                           -5 * management_data.get_military_number_at(85) :
                           0;

		score_parts[34] = -1 * management_data.neuron_pool[4].get_result_safe(
		                          (field.area_military_capacity + 4) / 5, kAbsValue);
		score_parts[35] = 3 * management_data.get_military_number_at(133);

		if (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) {
			score_parts[36] = -100 - 4 * std::abs(management_data.get_military_number_at(139));
		} else if (expansion_type.get_expansion_type() == ExpansionMode::kResources ||
		           expansion_type.get_expansion_type() == ExpansionMode::kSpace) {
			score_parts[37] =
			   +100 + 4 * std::abs(management_data.get_military_number_at(139));  // The same as above
		}
		if (msites_in_constr() > 0 && field.max_buildcap_nearby == Widelands::BUILDCAPS_BIG &&
		    spots_avail.at(Widelands::BUILDCAPS_BIG) <= 2) {
			score_parts[65] = -10 * std::abs(management_data.get_military_number_at(54));
		}
	}

	// common inputs
	if (field.unowned_iron_mines_nearby > 0 && mines_per_type[iron_resource_id].total_count() == 0) {
		score_parts[40] = field.unowned_iron_mines_nearby *
		                  std::abs(management_data.get_military_number_at(92)) / 50;
	}
	if (field.unowned_iron_mines_nearby && mines_per_type[iron_resource_id].total_count() <= 1) {
		score_parts[41] = 3 * std::abs(management_data.get_military_number_at(93));
	}

	score_parts[42] = (field.unowned_land_nearby) ? management_data.neuron_pool[18].get_result_safe(
	                                                   field.own_non_military_nearby, kAbsValue) :
                                                   0;

	score_parts[43] = 2 * management_data.neuron_pool[11].get_result_safe(
	                         field.unowned_buildable_spots_nearby, kAbsValue);
	score_parts[44] =
	   management_data.neuron_pool[12].get_result_safe(field.unowned_mines_spots_nearby, kAbsValue);
	score_parts[45] =
	   (field.unowned_land_nearby) ?
         field.military_loneliness * std::abs(management_data.get_military_number_at(53)) / 800 :
         0;

	score_parts[46] =
	   -1 * management_data.neuron_pool[55].get_result_safe(field.ally_military_presence, kAbsValue);
	score_parts[47] = -1 * management_data.neuron_pool[53].get_result_safe(
	                          2 * field.ally_military_presence, kAbsValue);
	score_parts[48] = -2 * management_data.neuron_pool[4].get_result_safe(
	                          (field.area_military_capacity + 4) / 5, kAbsValue);
	score_parts[49] = ((field.military_in_constr_nearby + field.military_unstationed) > 0) ?
                        -std::abs(management_data.get_military_number_at(81)) :
                        0;
	score_parts[55] = (field.military_loneliness < 10) ?
                        2 * std::abs(management_data.get_military_number_at(141)) :
                        0;
	score_parts[56] =
	   (any_unconnected_imm) ? 2 * std::abs(management_data.get_military_number_at(23)) : 0;
	score_parts[57] = 1 * management_data.neuron_pool[18].get_result_safe(
	                         2 * field.unowned_portspace_vicinity_nearby, kAbsValue);
	score_parts[58] = 3 * management_data.neuron_pool[19].get_result_safe(
	                         5 * field.unowned_portspace_vicinity_nearby, kAbsValue);
	score_parts[59] = (field.unowned_portspace_vicinity_nearby) ?
                        10 * std::abs(management_data.get_military_number_at(31)) :
                        0;
	score_parts[60] = 3 * management_data.neuron_pool[21].get_result_safe(
	                         20 - field.nearest_buildable_spot_nearby, kAbsValue);
	score_parts[61] = (field.nearest_buildable_spot_nearby < 8) ?
                        std::abs(management_data.get_military_number_at(153) * 2) :
                        0;
	score_parts[62] = (field.nearest_buildable_spot_nearby > 20) ?
                        -std::abs(management_data.get_military_number_at(154) * 2) :
                        0;
	score_parts[63] = (field.nearest_buildable_spot_nearby < 4) ?
                        std::abs(management_data.get_military_number_at(155) * 2) :
                        0;
	// 64 and 65 are used above
	score_parts[66] =
	   (field.unowned_mines_spots_nearby > 0 && !mine_fields_stat.has_critical_ore_fields()) ?
         std::abs(management_data.get_military_number_at(157)) :
         0;
	score_parts[67] = (field.unowned_mines_spots_nearby > 0 && mine_fields_stat.count_types() <= 4) ?
                        std::abs(management_data.get_military_number_at(158)) :
                        0;
	score_parts[68] =
	   (field.unowned_mines_spots_nearby == 0 && mine_fields_stat.count_types() <= 4) ?
         -std::abs(management_data.get_military_number_at(159)) :
         0;

	for (int32_t part : score_parts) {
		field.military_score_ += part;
	}

	if (ai_training_mode_) {
		if (field.military_score_ < -5000 || field.military_score_ > 2000) {
			verb_log_dbg_time(
			   gametime, "Warning field.military_score_ %5d, compounds: ", field.military_score_);
			for (int32_t part : score_parts) {
				verb_log_dbg_time(gametime, "%d, ", part);
			}
		}
	}

	// is new site allowed at all here?
	field.defense_msite_allowed = false;
	static int16_t multiplicator = 10;
	multiplicator = 10;
	if (soldier_status_ == SoldiersStatus::kBadShortage) {
		multiplicator = 4;
	} else if (soldier_status_ == SoldiersStatus::kShortage) {
		multiplicator = 7;
	}
	if (field.area_military_capacity < field.enemy_military_presence * multiplicator / 10) {
		field.defense_msite_allowed = true;
	}
}

/// Updates one mineable field
void DefaultAI::update_mineable_field(MineableField& field) {
	// collect information about resources in the area
	std::vector<Widelands::ImmovableFound> immovables;
	const Widelands::Map& map = game().map();
	map.find_immovables(game(), Widelands::Area<Widelands::FCoords>(field.coords, 5), &immovables);
	field.preferred = false;
	field.mines_nearby = 0;
	Widelands::FCoords fse;
	map.get_brn(field.coords, &fse);

	if (Widelands::BaseImmovable const* const imm = fse.field->get_immovable()) {
		if ((imm->descr().type() == Widelands::MapObjectType::FLAG ||
		     imm->descr().type() == Widelands::MapObjectType::ROAD) &&
		    (fse.field->nodecaps() & Widelands::BUILDCAPS_FLAG)) {
			field.preferred = true;
		}
	}

	for (const Widelands::ImmovableFound& temp_immovable : immovables) {
		if (upcast(Widelands::Building const, bld, temp_immovable.object)) {
			if (player_number() != bld->owner().player_number()) {
				continue;
			}
			if (bld->descr().get_ismine()) {
				if (get_building_observer(bld->descr().name().c_str()).mines ==
				    field.coords.field->get_resources()) {
					++field.mines_nearby;
				}
			} else if (upcast(Widelands::ConstructionSite const, cs, bld)) {
				if (cs->building().get_ismine()) {
					if (get_building_observer(cs->building().name().c_str()).mines ==
					    field.coords.field->get_resources()) {
						++field.mines_nearby;
					}
				}
			}
		}
	}

	// 0 is default, and thus indicates that counting must be done
	if (field.same_mine_fields_nearby == 0) {
		FindNodeMineable find_mines_spots_nearby(game(), field.coords.field->get_resources());
		field.same_mine_fields_nearby =
		   map.find_fields(game(), Widelands::Area<Widelands::FCoords>(field.coords, 4), nullptr,
		                   find_mines_spots_nearby);
	}
}

/// Updates the production and MINE sites statistics needed for construction decision.
void DefaultAI::update_productionsite_stats() {

	// Reset statistics for all buildings
	for (BuildingObserver& bo : buildings_) {
		bo.current_stats = 0;
		bo.unoccupied_count = 0;
		bo.unconnected_count = 0;
	}

	// Check all available productionsites
	for (uint32_t i = 0; i < productionsites.size(); ++i) {
		assert(productionsites.front().bo->cnt_built > 0);
		// is connected
		// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
		// ferries
		const bool connected_to_wh =
		   !productionsites.front().site->get_economy(Widelands::wwWORKER)->warehouses().empty();

		// unconnected buildings are excluded from statistics review
		if (connected_to_wh) {
			// Add statistics value
			productionsites.front().bo->current_stats +=
			   productionsites.front().site->get_statistics_percent();

			// Check whether this building is completely occupied
			if (!productionsites.front().site->can_start_working()) {
				++productionsites.front().bo->unoccupied_count;
			}
		} else {
			++productionsites.front().bo->unconnected_count;
		}

		// Now reorder the buildings
		productionsites.push_back(productionsites.front());
		productionsites.pop_front();
	}

	// for mines_ also
	// Check all available mines
	for (uint32_t i = 0; i < mines_.size(); ++i) {
		assert(mines_.front().bo->cnt_built > 0);

		const bool connected_to_wh =
		   !mines_.front().site->get_economy(Widelands::wwWORKER)->warehouses().empty();

		// unconnected mines are excluded from statistics review
		if (connected_to_wh) {
			// Add statistics value
			mines_.front().bo->current_stats += mines_.front().site->get_statistics_percent();
			// Check whether this building is completely occupied
			if (!mines_.front().site->can_start_working()) {
				++mines_.front().bo->unoccupied_count;
			}
		} else {
			++mines_.front().bo->unconnected_count;
		}

		// Now reorder the buildings
		mines_.push_back(mines_.front());
		mines_.pop_front();
	}

	// Scale statistics down
	for (BuildingObserver& bo : buildings_) {
		if (bo.cnt_built - bo.unconnected_count > 0) {
			bo.current_stats /= bo.cnt_built - bo.unconnected_count;
		}
	}
}

unsigned DefaultAI::find_immovables_nearby(
   const std::set<std::pair<Widelands::MapObjectType, Widelands::MapObjectDescr::AttributeIndex>>&
      attribute_infos,
   const Widelands::FCoords& position,
   const WorkareaInfo& workarea_info) const {
	unsigned result = 0;

	// Get max radius of recursive workarea
	WorkareaInfo::size_type radius = 0;
	for (const auto& temp_info : workarea_info) {
		if (radius < temp_info.first) {
			radius = temp_info.first;
		}
	}

	for (const auto& attribute_info : attribute_infos) {
		if (attribute_info.first != Widelands::MapObjectType::IMMOVABLE) {
			continue;
		}
		result += game().map().find_immovables(
		   game(), Widelands::Area<Widelands::FCoords>(position, radius), nullptr,
		   Widelands::FindImmovableAttribute(attribute_info.second));
	}
	return result;
}

// * Constructs the most needed building
//   algorithm goes over all available spots and all allowed buildings,
//   scores every combination and one with highest and positive score
//   is built.
// * Buildings are split into categories
// * The logic is complex but approximately:
// - some buildings belong to "basic economy" - these are preferred
// - some small huts are exempt from basic economy logic
// - first bulding of a type is preferred
// - algorithm is trying to take into account actual utlization of buildings
//   (the one shown in GUI/game is not reliable, it calculates own statistics)
// * military buildings use genetic algorithm logic to score fields
//   Currently more military buildings are built than needed
//   so there are always some vacant positions
bool DefaultAI::construct_building(const Time& gametime) {
	if (buildable_fields.empty()) {
		return false;
	}

	// Just used for easy checking whether a mine or something else was built.
	static bool mine = false;
	mine = false;
	static uint32_t consumers_nearby_count = 0;
	consumers_nearby_count = 0;

	const Widelands::Map& map = game().map();

	if (gametime > last_seafaring_check_ + Duration(20 * 1000)) {
		map_allows_seafaring_ = map.allows_seafaring();
		last_seafaring_check_ = gametime;
	}

	for (int32_t i = 0; i < 4; ++i) {
		spots_avail.at(i) = 0;
	}

	// We calculate owned buildable spots, of course ignoring ones that are blocked
	// for now
	for (const BuildableField* bf : buildable_fields) {
		if (blocked_fields.is_blocked(bf->coords)) {
			continue;
		}
		++spots_avail.at(bf->coords.field->nodecaps() & Widelands::BUILDCAPS_SIZEMASK);
	}

	spots_ = spots_avail.at(Widelands::BUILDCAPS_SMALL);
	spots_ += spots_avail.at(Widelands::BUILDCAPS_MEDIUM);
	spots_ += spots_avail.at(Widelands::BUILDCAPS_BIG);

	// helper variable - we need some proportion of free spots vs productionsites
	// the proportion depends on size of economy
	// this proportion defines how dense the buildings will be
	// it is degressive (allows high density on the beginning)
	static int32_t needed_spots = 0;
	if (productionsites.size() < 50) {
		needed_spots = productionsites.size();
	} else if (productionsites.size() < 100) {
		needed_spots = 50 + (productionsites.size() - 50) * 5;
	} else if (productionsites.size() < 200) {
		needed_spots = 300 + (productionsites.size() - 100) * 10;
	} else {
		needed_spots = 1300 + (productionsites.size() - 200) * 20;
	}
	const bool has_enough_space = (spots_ > needed_spots);

	// Do we have basic economy established? Informing that we just left the basic economy mode.
	if (!basic_economy_established && persistent_data->remaining_basic_buildings.empty()) {
		verb_log_info_time(gametime, "AI %2d: Player has achieved the basic economy at %s\n",
		                   player_number(), gamestring_with_leading_zeros(gametime.get()));
		basic_economy_established = true;
		assert(persistent_data->remaining_basic_buildings.empty());
	}

	if (!basic_economy_established && player_statistics.any_enemy_seen_lately(gametime) &&
	    management_data.f_neuron_pool[17].get_position(0)) {
		verb_log_info_time(gametime,
		                   "AI %2d: Player has not all buildings for basic economy yet (%" PRIuS
		                   " missing), but enemy is "
		                   "nearby, so quitting the mode at %s\n",
		                   player_number(), persistent_data->remaining_basic_buildings.size(),
		                   gamestring_with_leading_zeros(gametime.get()));
		basic_economy_established = true;
		// Zeroing following to preserve consistency
		persistent_data->remaining_basic_buildings.clear();
	}

	// *_military_scores are used as minimal score for a new military building
	// to be built. As AI does not traverse all building fields at once, these thresholds
	// are gradually going down until it finds a field&building that are above threshold
	// and this combination is used...
	// least_military_score is hardlimit, floating very slowly
	// target_military_score is always set according to latest best building (using the same
	// score) and quickly falling down until it reaches the least_military_score
	// this one (=target_military_score) is actually used to decide if building&field is allowed
	// candidate

	const Widelands::PlayerNumber pn = player_number();

	// Genetic algorithm is used here
	static bool inputs[2 * kFNeuronBitSize] = {false};
	// Resetting values as the variable is static
	std::fill(std::begin(inputs), std::end(inputs), false);
	inputs[0] = (pow(msites_in_constr(), 2) > militarysites.size() + 2);
	inputs[1] = !(pow(msites_in_constr(), 2) > militarysites.size() + 2);
	inputs[2] =
	   (highest_nonmil_prio_ > 18 + std::abs(management_data.get_military_number_at(29) / 10));
	inputs[3] =
	   !(highest_nonmil_prio_ > 18 + std::abs(management_data.get_military_number_at(29) / 10));
	inputs[4] = (highest_nonmil_prio_ > 18 + std::abs(management_data.get_military_number_at(48)));
	inputs[5] = !(highest_nonmil_prio_ > 18 + std::abs(management_data.get_military_number_at(49)));
	inputs[6] = ((numof_psites_in_constr + mines_in_constr()) >
	             (productionsites.size() + mines_built()) / productionsites_ratio_);
	inputs[7] = !((numof_psites_in_constr + mines_in_constr()) >
	              (productionsites.size() + mines_built()) / productionsites_ratio_);

	inputs[8] = (has_enough_space);
	inputs[9] = !(has_enough_space);
	inputs[10] = (has_enough_space);
	inputs[11] = !(has_enough_space);

	inputs[12] = (gametime > Time(45 * 60 * 1000));
	inputs[13] = !(gametime > Time(45 * 60 * 1000));

	inputs[14] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy);
	inputs[15] = !(expansion_type.get_expansion_type() == ExpansionMode::kEconomy);
	inputs[16] = (expansion_type.get_expansion_type() == ExpansionMode::kSpace);
	inputs[17] = !(expansion_type.get_expansion_type() == ExpansionMode::kSpace);

	inputs[18] = (player_statistics.any_enemy_seen_lately(gametime));
	inputs[19] = !(player_statistics.any_enemy_seen_lately(gametime));
	inputs[20] = (player_statistics.get_player_power(pn) >
	              player_statistics.get_old60_player_power(pn) +
	                 std::abs(management_data.get_military_number_at(130)) / 10);
	inputs[21] = !(player_statistics.get_player_power(pn) >
	               player_statistics.get_old60_player_power(pn) +
	                  std::abs(management_data.get_military_number_at(131)) / 10);
	inputs[22] =
	   (player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn));
	inputs[23] =
	   !(player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn));
	inputs[24] = (highest_nonmil_prio_ > 18 + management_data.get_military_number_at(65) / 10);
	inputs[25] = !(highest_nonmil_prio_ > 18 + management_data.get_military_number_at(65) / 10);
	inputs[26] = (player_statistics.get_modified_player_power(pn) >
	              player_statistics.get_visible_enemies_power(gametime));
	inputs[27] = (player_statistics.get_modified_player_power(pn) <=
	              player_statistics.get_visible_enemies_power(gametime));
	inputs[28] =
	   (player_statistics.get_player_power(pn) > player_statistics.get_enemies_average_power());
	inputs[29] =
	   !(player_statistics.get_player_power(pn) > player_statistics.get_enemies_average_power());
	inputs[30] =
	   (player_statistics.get_player_power(pn) > player_statistics.get_enemies_max_power());
	inputs[31] =
	   !(player_statistics.get_player_power(pn) > player_statistics.get_enemies_max_power());

	inputs[32] = (persistent_data->least_military_score <
	              persistent_data->ai_personality_mil_upper_limit *
	                 std::abs(management_data.get_military_number_at(69)) / 100);
	inputs[33] = !(persistent_data->least_military_score <
	               persistent_data->ai_personality_mil_upper_limit *
	                  std::abs(management_data.get_military_number_at(69)) / 100);
	inputs[34] = player_statistics.strong_enough(pn);
	inputs[35] = !player_statistics.strong_enough(pn);

	inputs[36] = (player_statistics.get_player_land(pn) < 500);
	inputs[37] = (player_statistics.get_player_land(pn) < 700);
	inputs[38] = (player_statistics.get_player_land(pn) < 900);
	inputs[39] = (player_statistics.get_player_land(pn) < 1100);
	inputs[40] = (player_statistics.get_player_land(pn) > 500);
	inputs[41] = (player_statistics.get_player_land(pn) > 700);
	inputs[42] = (player_statistics.get_player_land(pn) > 900);
	inputs[43] = (player_statistics.get_player_land(pn) > 1100);
	inputs[44] = (player_statistics.get_player_power(pn) >
	              player_statistics.get_old60_player_power(pn) +
	                 std::abs(management_data.get_military_number_at(130)) / 10);
	inputs[45] = !(player_statistics.get_player_power(pn) >
	               player_statistics.get_old60_player_power(pn) +
	                  std::abs(management_data.get_military_number_at(131)) / 10);
	inputs[46] =
	   (player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn));
	inputs[47] =
	   !(player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn));
	inputs[48] = (bakeries_count_ == 0);
	inputs[49] = (bakeries_count_ <= 1);
	inputs[50] = (bakeries_count_ <= 1);
	inputs[51] = (numof_psites_in_constr > 8);
	inputs[52] = (numof_psites_in_constr < 8);
	inputs[53] = (mine_fields_stat.has_critical_ore_fields());
	inputs[54] = (!mine_fields_stat.has_critical_ore_fields());
	inputs[55] = (mine_fields_stat.count_types() == kMineTypes);
	inputs[56] = (mine_fields_stat.count_types() != kMineTypes);
	inputs[57] = (mine_fields_stat.has_critical_ore_fields());
	inputs[58] = (!mine_fields_stat.has_critical_ore_fields());

	static int16_t needs_boost_economy_score = management_data.get_military_number_at(61) / 5;
	needs_boost_economy_score = management_data.get_military_number_at(61) / 5;
	static int16_t increase_score_limit_score = 0;
	increase_score_limit_score = 0;

	for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
		if (management_data.f_neuron_pool[51].get_position(i)) {
			needs_boost_economy_score += (inputs[i]) ? 1 : -1;
		}
		if (management_data.f_neuron_pool[52].get_position(i)) {
			increase_score_limit_score += (inputs[i]) ? 1 : -1;
		}
		if (management_data.f_neuron_pool[21].get_position(i)) {
			needs_boost_economy_score += (inputs[kFNeuronBitSize + i]) ? 1 : -1;
		}
		if (management_data.f_neuron_pool[22].get_position(i)) {
			increase_score_limit_score += (inputs[kFNeuronBitSize + i]) ? 1 : -1;
		}
	}

	// Finding expansion policy
	// Do we need basic resources? Do we have basic mines?
	const bool needs_fishers = resource_necessity_water_needed_ && fishers_count_ < 1;

	if (!mine_fields_stat.has_critical_ore_fields() ||
	    mines_per_type[iron_resource_id].total_count() < 1 || needs_fishers) {
		expansion_type.set_expantion_type(ExpansionMode::kResources);
	} else {
		// now we must decide if we go after spots or economy boost
		if (needs_boost_economy_score >= 3) {
			expansion_type.set_expantion_type(ExpansionMode::kEconomy);
		} else if (needs_boost_economy_score >= -2) {
			expansion_type.set_expantion_type(ExpansionMode::kBoth);
		} else {
			expansion_type.set_expantion_type(ExpansionMode::kSpace);
		}
	}

	const bool increase_least_score_limit =
	   (increase_score_limit_score > management_data.get_military_number_at(45) / 5);

	static uint16_t concurent_ms_in_constr_no_enemy = 1;
	concurent_ms_in_constr_no_enemy = 1;
	static uint16_t concurent_ms_in_constr_enemy_nearby = 2;
	concurent_ms_in_constr_enemy_nearby = 2;

	// resetting highest_nonmil_prio_ so it can be recalculated anew
	highest_nonmil_prio_ = 0;

	if (increase_least_score_limit) {
		if (persistent_data->least_military_score <
		    persistent_data
		       ->ai_personality_mil_upper_limit) {  // No sense in letting it grow too high
			persistent_data->least_military_score += 20;
			if (persistent_data->least_military_score > persistent_data->target_military_score) {
				persistent_data->target_military_score = persistent_data->least_military_score;
			}
			if (persistent_data->target_military_score >
			    persistent_data->ai_personality_mil_upper_limit) {
				persistent_data->ai_personality_mil_upper_limit =
				   persistent_data->target_military_score;
			}
		}
	} else {

		uint16_t divider = 1;  // this is to slow down decrementing the least military score
		switch (expansion_type.get_expansion_type()) {
		case ExpansionMode::kEconomy:
			divider = 3;
			break;
		case ExpansionMode::kBoth:
			divider = 2;
			break;
		default:
			divider = 1;
		}

		// least_military_score is decreased, but depending on the size of territory
		switch (static_cast<uint32_t>(log10(buildable_fields.size()))) {
		case 0:
			persistent_data->least_military_score -= 10 / divider;
			break;
		case 1:
			persistent_data->least_military_score -= 8 / divider;
			break;
		case 2:
			persistent_data->least_military_score -= 5 / divider;
			break;
		case 3:
			persistent_data->least_military_score -= 3 / divider;
			break;
		default:
			persistent_data->least_military_score -= 2 / divider;
		}
		if (persistent_data->least_military_score < 0) {
			persistent_data->least_military_score = 0;
		}
	}

	assert(persistent_data->least_military_score <= persistent_data->target_military_score);
	assert(persistent_data->target_military_score <=
	       persistent_data->ai_personality_mil_upper_limit);
	persistent_data->target_military_score = 9 * persistent_data->target_military_score / 10;
	if (persistent_data->target_military_score < persistent_data->least_military_score) {
		persistent_data->target_military_score = persistent_data->least_military_score;
	}
	assert(persistent_data->target_military_score >= persistent_data->least_military_score);

	// we must calculate wood policy for each type of rangers
	for (BuildingObserver& bo : rangers_) {
		// stocked level of supported wares (only lowest counts) is to be in some proportion to
		// productionsites and constructionsites (this proportion is bit artifical, or we can say
		// it is proportion to the size of economy). Plus some positive 'margin'.
		const int32_t stocked_wood_margin = get_stocklevel(bo, gametime) -
		                                    productionsites.size() * 2 - numof_psites_in_constr +
		                                    management_data.get_military_number_at(87) / 5;
		if (gametime < Time(15 * 60 * 1000)) {
			wood_policy_[bo.id] = WoodPolicy::kAllowRangers;
		} else if (stocked_wood_margin > 80) {
			wood_policy_[bo.id] = WoodPolicy::kDismantleRangers;
		} else if (stocked_wood_margin > 25) {
			wood_policy_[bo.id] = WoodPolicy::kStopRangers;
		} else {
			wood_policy_[bo.id] = WoodPolicy::kAllowRangers;
		}
		verb_log_dbg_time(gametime,
		                  "Name: %-30s id:%d stock: %d actual policy: %hhu policies(allow, stop, "
		                  "dismantle): %hhu, %hhu, %hhu \n",
		                  bo.name, bo.id, stocked_wood_margin,
		                  static_cast<uint8_t>(wood_policy_.at(bo.id)),
		                  static_cast<uint8_t>(WoodPolicy::kAllowRangers),
		                  static_cast<uint8_t>(WoodPolicy::kStopRangers),
		                  static_cast<uint8_t>(WoodPolicy::kDismantleRangers));
	}

	BuildingObserver* best_building = nullptr;
	int32_t proposed_priority = 0;
	Widelands::Coords proposed_coords;

	// Remove outdated fields from blocker list
	blocked_fields.remove_expired(gametime);

	// testing big military buildings, whether critical construction
	// material is available (at least in amount of
	// 2/3 of default target amount)
	for (BuildingObserver& bo : buildings_) {
		if (!bo.buildable(*player_)) {
			continue;
		}

		// not doing this for non-military buildins
		if (!(bo.type == BuildingObserver::Type::kMilitarysite ||
		      bo.type == BuildingObserver::Type::kTrainingsite ||
		      bo.type == BuildingObserver::Type::kProductionsite)) {
			continue;
		}

		// and neither for small military buildings
		if (bo.type == BuildingObserver::Type::kMilitarysite &&
		    bo.desc->get_size() == Widelands::BaseImmovable::SMALL) {
			continue;
		}

		bo.build_material_shortage = false;

		// checking we have enough critical material on stock
		for (uint32_t m = 0; m < bo.critical_building_material.size(); ++m) {
			Widelands::DescriptionIndex wt(static_cast<size_t>(bo.critical_building_material.at(m)));
			uint32_t treshold = 7;
			// generally trainingsites are more important
			if (bo.type == BuildingObserver::Type::kTrainingsite) {
				treshold = 4;
			}

			if (bo.type == BuildingObserver::Type::kProductionsite) {
				treshold = 2;
			}

			if (calculate_stocklevel(wt) <= treshold) {
				bo.build_material_shortage = true;
				break;
			}
		}
	}

	// Calculating actual neededness
	for (BuildingObserver& bo : buildings_) {
		// we check if a previously not buildable Building of the basic economy is buildable again
		// If so and we don't have basic economy achieved we add readd it to basic buildings list
		// This should only happen in scenarios via scripting
		if (!basic_economy_established && bo.basic_amount > static_cast<uint32_t>(bo.total_count()) &&
		    bo.buildable(*player_)) {
			persistent_data->remaining_basic_buildings.emplace(std::make_pair(bo.id, bo.basic_amount));
		}
		if (!bo.buildable(*player_)) {
			bo.new_building = BuildingNecessity::kNotNeeded;
			// This line removes buildings from basic economy if they are not allowed for the player
			// this should only happen by scripting.
			if (bo.basic_amount) {
				persistent_data->remaining_basic_buildings.erase(bo.id);
			}
		} else if (bo.type == BuildingObserver::Type::kProductionsite ||
		           bo.type == BuildingObserver::Type::kMine) {

			bo.new_building = check_building_necessity(bo, PerfEvaluation::kForConstruction, gametime);

			if (bo.is(BuildingAttribute::kShipyard)) {
				assert(bo.new_building == BuildingNecessity::kAllowed ||
				       bo.new_building == BuildingNecessity::kNeeded ||
				       bo.new_building == BuildingNecessity::kForbidden);
			}

			if (bo.new_building == BuildingNecessity::kAllowed) {
				bo.new_building_overdue = 0;
			}

			// Now verifying that all 'buildable' buildings has positive max_needed_preciousness
			// if they have outputs, all other must have zero max_needed_preciousness

			if (bo.new_building == BuildingNecessity::kForbidden) {
				bo.max_needed_preciousness = 0;
			} else if ((bo.new_building == BuildingNecessity::kNeeded ||
			            bo.new_building == BuildingNecessity::kForced ||
			            bo.new_building == BuildingNecessity::kAllowed ||
			            bo.new_building == BuildingNecessity::kNeededPending) &&
			           (!bo.ware_outputs.empty() ||
			            bo.initial_preciousness >
			               0)) {  // bo.initial_preciousness signals that we have a worker output
				bo.max_needed_preciousness =
				   std::max(bo.max_needed_preciousness, bo.initial_preciousness);
				bo.max_preciousness = std::max(bo.max_preciousness, bo.initial_preciousness);

				if (bo.max_needed_preciousness <= 0) {
					throw wexception("AI: Max preciousness must not be <= 0 for building: %s",
					                 bo.desc->name().c_str());
				}
			} else {
				// For other situations we make sure max_needed_preciousness is zero
				assert(bo.max_needed_preciousness == 0);
			}

			// Positive max_needed_preciousness says a building type is needed
			// here we increase or reset the counter
			// The counter is added to score when considering new building
			if (bo.max_needed_preciousness > 0) {
				++bo.new_building_overdue;
			} else {
				bo.new_building_overdue = 0;
			}

			// Here we consider a time how long a building needed
			// We calculate primary_priority used later in construct_building(),
			// it is basically max_needed_preciousness_ plus some 'bonus' for due time
			// Following scenarios are possible:
			// a) building is needed or forced: primary_priority grows with time
			// b) building is allowed: primary_priority = max_needed_preciousness (no time
			// consideration)
			// c) all other cases: primary_priority = 0;
			if (bo.max_needed_preciousness > 0) {
				if (bo.new_building == BuildingNecessity::kAllowed) {
					bo.primary_priority += bo.max_needed_preciousness;
				} else {
					bo.primary_priority += bo.primary_priority * bo.new_building_overdue *
					                       std::abs(management_data.get_military_number_at(120)) / 25;
					bo.primary_priority += bo.max_needed_preciousness +
					                       bo.max_needed_preciousness * bo.new_building_overdue *
					                          std::abs(management_data.get_military_number_at(70)) /
					                          100 +
					                       bo.new_building_overdue *
					                          std::abs(management_data.get_military_number_at(71)) / 10;
					if (bo.new_building == BuildingNecessity::kForced) {
						bo.primary_priority += bo.new_building_overdue *
						                       std::abs(management_data.get_military_number_at(119)) / 25;
					}
				}
			} else {
				bo.primary_priority = 0;
			}

		} else if (bo.type == BuildingObserver::Type::kMilitarysite) {
			bo.new_building = check_building_necessity(bo, gametime);
		} else if (bo.type == BuildingObserver::Type::kTrainingsite) {
			bo.new_building = check_building_necessity(bo, PerfEvaluation::kForConstruction, gametime);
		} else if (bo.type == BuildingObserver::Type::kWarehouse) {
			bo.new_building = check_warehouse_necessity(bo, gametime);
		} else if (bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
			bo.new_building = BuildingNecessity::kNotNeeded;
		} else {
			bo.new_building = BuildingNecessity::kAllowed;
			bo.primary_priority = 0;
		}

		const bool log_needed = (bo.new_building == BuildingNecessity::kAllowed ||
		                         bo.new_building == BuildingNecessity::kForced ||
		                         bo.new_building == BuildingNecessity::kNeeded);
		if (ai_training_mode_ && bo.type == BuildingObserver::Type::kProductionsite &&
		    (gametime.get() % 20 == 0 || log_needed)) {
			log_dbg_time(
			   gametime,
			   "AI %2d: %-35s(%2d now) %-11s: max prec: %2d/%2d, primary priority: %4d, overdue: "
			   "%3d\n",
			   player_number(), bo.name, bo.total_count(), (log_needed) ? "needed" : "not needed",
			   bo.max_needed_preciousness, bo.max_preciousness, bo.primary_priority,
			   bo.new_building_overdue);
		}
	}

	// first scan all buildable fields for regular buildings
	for (BuildableField* const bf : buildable_fields) {
		if (bf->field_info_expiration < gametime) {
			continue;
		}

		// Continue if field is blocked at the moment
		if (blocked_fields.is_blocked(bf->coords)) {
			continue;
		}

		assert(player_);
		int32_t const maxsize = player_->get_buildcaps(bf->coords) & Widelands::BUILDCAPS_SIZEMASK;

		// For every field test all buildings
		for (BuildingObserver& bo : buildings_) {
			if (!bo.buildable(*player_)) {
				continue;
			}

			if (bo.new_building == BuildingNecessity::kNotNeeded ||
			    bo.new_building == BuildingNecessity::kNeededPending ||
			    bo.new_building == BuildingNecessity::kForbidden) {
				continue;
			}

			assert(bo.new_building == BuildingNecessity::kForced ||
			       bo.new_building == BuildingNecessity::kNeeded ||
			       bo.new_building == BuildingNecessity::kAllowed);

			assert(bo.aimode_limit_status() == AiModeBuildings::kAnotherAllowed);

			// if current field is not big enough
			if (bo.desc->get_size() > maxsize) {
				continue;
			}

			if (std::rand() % 3 == 0 && bo.total_count() > 0) {  // NOLINT
				continue;
			}  // add randomnes and ease AI

			if (bo.type == BuildingObserver::Type::kMine) {
				continue;
			}

			// here we do an exemption for lumberjacks, mainly in early stages of game
			// sometimes the first one is not built and AI waits too long for second attempt
			if (gametime - bo.construction_decision_time < kBuildingMinInterval &&
			    !bo.is(BuildingAttribute::kLumberjack)) {
				continue;
			}

			if (!(bo.type == BuildingObserver::Type::kMilitarysite) &&
			    bo.cnt_under_construction >= 2) {
				continue;
			}

			int32_t prio = 0;  // score of a bulding on a field

			// testing for reserved ports
			if (!bo.is(BuildingAttribute::kPort)) {
				if (bf->portspace_nearby == ExtendedBool::kTrue) {
					if (num_ports == 0) {
						continue;
					}
					// If we have at least on port, we can perhaps build here something
					// but decreasing the score to discourage it
					prio -= 5 * std::abs(management_data.get_military_number_at(52));
				}
			}

			if (bo.type == BuildingObserver::Type::kProductionsite) {

				prio += management_data.neuron_pool[44].get_result_safe(bf->military_score_ / 20) / 5;

				// Some productionsites strictly require supporting sites nearby
				uint8_t number_of_supporters_nearby = 0;
				if (bo.requires_supporters) {
					if (bf->supporters_nearby.count(bo.name) == 0) {
						prio -= 100 + std::abs(management_data.get_military_number_at(18)) * 3;
					} else {
						number_of_supporters_nearby += bf->supporters_nearby.at(bo.desc->name());
					}
				}

				// Priorities will be adjusted according to nearby buildings needing support
				uint8_t number_of_supported_producers_nearby = 0;
				auto it = bf->supported_producers_nearby.find(bo.id);
				if (it != bf->supported_producers_nearby.end()) {
					number_of_supported_producers_nearby += it->second;
				}

				// Priorities will be adjusted according to nearby competitors
				uint8_t number_of_same_nearby = 0;
				auto same_it = bf->buildings_nearby.find(bo.id);
				if (same_it != bf->buildings_nearby.end()) {
					number_of_same_nearby += same_it->second;
				}

				// this can be only a well (as by now)
				if (bo.is(BuildingAttribute::kWell)) {

					if (bo.new_building == BuildingNecessity::kForced) {
						assert(bo.total_count() - bo.unconnected_count == 0);
					}

					if (bf->ground_water < 2) {
						continue;
					}

					prio += bo.primary_priority;

					// keep wells more distant
					if (number_of_same_nearby > 2) {
						continue;
					}

					// one well is forced
					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 200;
					}

					prio += -10 +
					        std::abs(management_data.get_military_number_at(59) / 50) * bf->ground_water;

				} else if (bo.is(BuildingAttribute::kLumberjack)) {

					prio += bo.primary_priority;

					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 5 * std::abs(management_data.get_military_number_at(17));
					}

					if (bf->immovables_by_name_nearby[bo.name] < trees_nearby_treshold_ &&
					    bo.new_building == BuildingNecessity::kAllowed) {
						continue;
					}

					prio += std::abs(management_data.get_military_number_at(26)) *
					        (bf->immovables_by_name_nearby[bo.name] - trees_nearby_treshold_) / 10;

					// consider cutters and rangers nearby
					prio += 2 * number_of_supporters_nearby *
					        std::abs(management_data.get_military_number_at(25));
					prio -=
					   number_of_same_nearby * std::abs(management_data.get_military_number_at(36)) * 3;

				} else if (bo.is(BuildingAttribute::kNeedsRocks)) {

					// Quarries are generally to be built everywhere where rocks are
					// no matter the need for granite, as rocks are considered an obstacle
					// to expansion
					const uint8_t rocks_nearby = bf->immovables_by_name_nearby[bo.name];
					if (rocks_nearby < 1) {
						continue;
					}
					prio += 2 * rocks_nearby;

					if (rocks_nearby > 0 && bf->near_border) {
						prio += management_data.get_military_number_at(27) / 2;
					}

					// value is initialized with 1 but minimal value that can be
					// calculated is 11
					if (prio <= 1) {
						continue;
					}

					if (bo.total_count() - bo.unconnected_count == 0) {
						prio += 150;
					}

					if (get_stocklevel(bo, gametime) == 0) {
						prio *= 2;
					}

					// to prevent too many quarries on one spot
					prio = prio - 50 * number_of_same_nearby;

				} else if (bo.is(BuildingAttribute::kHunter)) {

					if (bf->critters_nearby < 5) {
						continue;
					}

					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 20;
					}

					// Overdue priority here
					prio += bo.primary_priority;

					prio += number_of_supporters_nearby * 5;

					prio += (bf->critters_nearby * 3) - 8 - 5 * number_of_same_nearby;

				} else if (bo.is(BuildingAttribute::kFisher)) {  // fisher

					if (bf->fish_nearby <= 15) {
						continue;
					}

					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 200;
					}

					// Overdue priority here
					prio += bo.primary_priority;
					prio -= number_of_same_nearby * 20;
					prio += number_of_supporters_nearby * 20;

					prio += -5 + bf->fish_nearby *
					                (1 + std::abs(management_data.get_military_number_at(63) / 15));
					if (resource_necessity_water_needed_) {
						prio *= 3;
					}

				} else if (!bo.supported_producers.empty()) {
					if (bo.is(BuildingAttribute::kRanger)) {
						assert(bo.cnt_target > 0);
					}

					prio += bo.primary_priority;

					if (bo.is(BuildingAttribute::kRanger)) {

						assert(bo.new_building == BuildingNecessity::kNeeded ||
						       bo.new_building == BuildingNecessity::kForced);

						if (bo.total_count() == 0) {
							prio += 200;
						} else {
							prio += std::abs(management_data.get_military_number_at(66)) *
							        (bo.cnt_target - bo.total_count());
						}

						prio -= bf->water_nearby / 5;

						prio += management_data.neuron_pool[67].get_result_safe(
						           number_of_supported_producers_nearby * 5, kAbsValue) /
						        2;

						prio += management_data.neuron_pool[49].get_result_safe(
						           bf->immovables_by_name_nearby[bo.name], kAbsValue) /
						        5;

						prio += number_of_supported_producers_nearby * 5 -
						        (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) * 15 -
						        bf->space_consumers_nearby *
						           std::abs(management_data.get_military_number_at(102)) / 5 -
						        bf->immovables_by_attribute_nearby[BuildingAttribute::kNeedsRocks] / 3;

						prio += number_of_same_nearby * 3;
						// don't block port building spots with trees
						if (bf->unowned_portspace_vicinity_nearby > 0) {
							prio -= 500;
						}
						// frisian claypit and frisian farm
					} else if (bo.is(BuildingAttribute::kSupportingProducer)) {
						// we dont like trees nearby
						prio += 1 - std::max(
						               bf->immovables_by_attribute_nearby[BuildingAttribute::kLumberjack],
						               bf->immovables_by_attribute_nearby[BuildingAttribute::kRanger]) /
						               3;
						// and be far from rangers
						prio += 1 - bf->rangers_nearby *
						               std::abs(management_data.get_military_number_at(102)) / 5;

						// This is for a special case this is also supporter, it considers
						// producers nearby
						prio += management_data.neuron_pool[51].get_result_safe(
						           number_of_supported_producers_nearby * 5, kAbsValue) /
						        2;

						// now we find out if the supporter is needed depending on output stocklevel
						// and supported stocklevel
						const uint32_t combined_stocklevel = (get_stocklevel(bo, gametime));

						if (combined_stocklevel > 50 &&
						    persistent_data->remaining_basic_buildings.count(bo.id) == 0) {
							continue;
						}

						if (combined_stocklevel < 40) {
							prio += 5 * management_data.neuron_pool[23].get_result_safe(
							               (40 - combined_stocklevel) / 2, kAbsValue);
						}

						// taking into account the vicinity
						prio += number_of_supported_producers_nearby * 10;
						prio -= number_of_same_nearby * 15;

						if (bf->enemy_nearby) {  // not close to the enemy
							prio -= 20;
						}

						// don't block port building spots with immovables
						if (bo.is(BuildingAttribute::kSpaceConsumer) &&
						    bf->unowned_portspace_vicinity_nearby > 0) {
							prio -= 500;
						}

						if (bo.is(BuildingAttribute::kSpaceConsumer) &&
						    bf->water_nearby) {  // not close to water
							prio -= std::abs(management_data.get_military_number_at(103)) / 5;
						}

						if (bo.is(BuildingAttribute::kSpaceConsumer) &&
						    bf->unowned_mines_spots_nearby) {  // not close to mountains
							prio -= std::abs(management_data.get_military_number_at(104)) / 5;
						}
						// frisian berry farm
					} else if (bo.is(BuildingAttribute::kSpaceConsumer)) {
						// we dont like trees nearby
						prio += 1 - std::max(
						               bf->immovables_by_attribute_nearby[BuildingAttribute::kLumberjack],
						               bf->immovables_by_attribute_nearby[BuildingAttribute::kRanger]) /
						               4;
						// and be far from rangers
						prio += 1 - bf->rangers_nearby *
						               std::abs(management_data.get_military_number_at(102)) / 5;

						// now we find out if the supporter is needed depending on stocklevel
						const uint32_t current_stocklevel = (get_stocklevel(bo, gametime));

						if (current_stocklevel > 50 &&
						    persistent_data->remaining_basic_buildings.count(bo.id) == 0) {
							continue;
						}

						if (current_stocklevel < 40) {
							prio += 5 * management_data.neuron_pool[23].get_result_safe(
							               (40 - current_stocklevel) / 2, kAbsValue);
						}
						// taking into account the vicinity
						prio += number_of_supported_producers_nearby * 10;
						prio -= number_of_same_nearby * 8;

						if (bf->enemy_nearby) {  // not close to the enemy
							prio -= 20;
						}

						// don't block port building spots with immovables
						if (bf->unowned_portspace_vicinity_nearby > 0) {
							prio -= 500;
						}

						if (bf->water_nearby) {  // not close to water
							prio -= std::abs(management_data.get_military_number_at(103)) / 5;
						}

						if (bf->unowned_mines_spots_nearby) {  // not close to mountains
							prio -= std::abs(management_data.get_military_number_at(104)) / 5;
						}

					} else {  // FISH BREEDERS and GAME KEEPERS

						// especially for fish breeders
						if (bo.is(BuildingAttribute::kNeedsCoast) &&
						    (bf->water_nearby < 6 || bf->fish_nearby < 6)) {
							continue;
						}
						if (bo.is(BuildingAttribute::kNeedsCoast)) {
							prio += (-6 + bf->water_nearby) / 3;
							prio += (-6 + bf->fish_nearby) / 3;
						}

						const uint32_t current_stocklevel = (get_stocklevel(bo, gametime));

						if (current_stocklevel > 50 &&
						    persistent_data->remaining_basic_buildings.count(bo.id) == 0) {
							continue;
						}

						if (current_stocklevel < 40) {
							prio += 5 * management_data.neuron_pool[23].get_result_safe(
							               (40 - current_stocklevel) / 2, kAbsValue);
						}

						prio += number_of_supported_producers_nearby * 10;
						prio -= number_of_same_nearby * 20;

						if (bf->enemy_nearby) {
							prio -= 20;
						}

						if (bf->unowned_portspace_vicinity_nearby > 0) {
							prio -= 500;
						}
					}

				} else if (bo.is(BuildingAttribute::kRecruitment)) {
					prio += bo.primary_priority;
					prio -= bf->unowned_land_nearby * 2;
					prio -= (bf->enemy_nearby) * 100;
					prio -= (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) * 100;
				} else {  // finally normal productionsites
					assert(bo.supported_producers.empty());

					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 150;
						assert(!bo.is(BuildingAttribute::kShipyard));
					} else if (bo.is(BuildingAttribute::kShipyard)) {
						if (!map_allows_seafaring_) {
							continue;
						}
					} else {
						assert(bo.new_building == BuildingNecessity::kNeeded);
					}

					// considering neededness depending on stocklevel
					const uint32_t current_stocklevel = (get_stocklevel(bo, gametime));
					if (current_stocklevel > 50 &&
					    persistent_data->remaining_basic_buildings.count(bo.id) == 0) {
						continue;
					}
					if (current_stocklevel < 40 && !bo.is(BuildingAttribute::kShipyard)) {
						prio += 5 * management_data.neuron_pool[23].get_result_safe(
						               (40 - current_stocklevel) / 2, kAbsValue);
					}
					// This considers supporters nearby
					prio += management_data.neuron_pool[52].get_result_safe(
					           number_of_supporters_nearby * 5, kAbsValue) /
					        2;

					// Overdue priority here
					prio += bo.primary_priority;

					// we check separatelly buildings with no inputs and some inputs
					if (bo.inputs.empty()) {

						assert(!bo.is(BuildingAttribute::kShipyard));

						if (bo.is(BuildingAttribute::kSpaceConsumer)) {  // e.g. farms
							// we dont like trees nearby
							prio +=
							   1 -
							   std::max(bf->immovables_by_attribute_nearby[BuildingAttribute::kLumberjack],
							            bf->immovables_by_attribute_nearby[BuildingAttribute::kRanger]) /
							      4;
							// we attempt to cluster space consumers together
							prio += number_of_same_nearby * 2;
							// and be far from rangers
							prio += 1 - bf->rangers_nearby *
							               std::abs(management_data.get_military_number_at(102)) / 5;
						} else {
							// leave some free space between them
							prio -= number_of_same_nearby *
							        std::abs(management_data.get_military_number_at(108)) / 5;
						}

						if (bo.is(BuildingAttribute::kSpaceConsumer) &&
						    bf->water_nearby) {  // not close to water
							prio -= std::abs(management_data.get_military_number_at(103)) / 5;
						}

						if (bo.is(BuildingAttribute::kSpaceConsumer) &&
						    bf->unowned_mines_spots_nearby) {  // not close to mountains
							prio -= std::abs(management_data.get_military_number_at(104)) / 5;
						}
						if (bo.is(BuildingAttribute::kSpaceConsumer) &&
						    bf->unowned_portspace_vicinity_nearby > 0) {  // do not block Ports
							prio -= 500;
						}
						if (bo.is(BuildingAttribute::kNeedsBerry)) {
							prio += std::abs(management_data.get_military_number_at(13)) *
							        bf->immovables_by_name_nearby[bo.name] / 12;
						}
						// buildings that need coast and are not considered above e.g. amazons
						// water_gatherers
						if (bo.is(BuildingAttribute::kNeedsCoast) && (bf->water_nearby < 3)) {
							continue;
						}
						if (bo.is(BuildingAttribute::kNeedsCoast)) {
							prio += (-3 + bf->water_nearby);
						}
					} else if (bo.is(BuildingAttribute::kShipyard)) {
						// for now AI builds only one shipyard
						assert(bo.total_count() == 0);
						if (bf->open_water_nearby > 3 && map_allows_seafaring_ &&
						    bf->shipyard_preferred == ExtendedBool::kTrue) {
							prio += productionsites.size() * 5 +
							        bf->open_water_nearby *
							           std::abs(management_data.get_military_number_at(109)) / 10;
						} else {
							continue;
						}
					}

					if (prio <= 0) {
						continue;
					}

					// bonus for big buildings if shortage of big fields
					if (spots_avail.at(Widelands::BUILDCAPS_BIG) <= 5 && bo.desc->get_size() == 3) {
						prio += std::abs(management_data.get_military_number_at(105)) / 5;
					}

					if (spots_avail.at(Widelands::BUILDCAPS_MEDIUM) <= 5 && bo.desc->get_size() == 2) {
						prio += std::abs(management_data.get_military_number_at(106)) / 5;
					}

					// +1 if any consumers_ are nearby
					consumers_nearby_count = 0;

					for (const Widelands::DescriptionIndex& output : bo.ware_outputs) {
						consumers_nearby_count += bf->consumers_nearby.at(output);
					}

					if (consumers_nearby_count > 0) {
						prio += std::abs(management_data.get_military_number_at(107)) / 3;
					}
				}

				// Consider border with exemption of some huts
				if (!(bo.is(BuildingAttribute::kLumberjack) || bo.is(BuildingAttribute::kNeedsCoast) ||
				      bo.is(BuildingAttribute::kFisher))) {
					prio += recalc_with_border_range(*bf, prio);
				} else if (bf->near_border && (bo.is(BuildingAttribute::kLumberjack) ||
				                               bo.is(BuildingAttribute::kNeedsCoast))) {
					prio /= 2;
				}

			}  // production sites done
			else if (bo.type == BuildingObserver::Type::kMilitarysite) {

				prio += bo.primary_priority;

				// Two possibilities why to construct militarysite here
				if (!bf->defense_msite_allowed &&
				    (bf->nearest_buildable_spot_nearby < bo.desc->get_conquers() ||
				     bf->unowned_portspace_vicinity_nearby > 0) &&
				    (bf->military_in_constr_nearby + bf->military_unstationed) <
				       concurent_ms_in_constr_no_enemy) {
					// it will conquer new buildable spots for buildings or mines
				} else if (bf->defense_msite_allowed &&
				           (bf->military_in_constr_nearby + bf->military_unstationed) <
				              concurent_ms_in_constr_enemy_nearby) {
					// we need it to increase capacity on the field
					if (bo.fighting_type) {
						prio += 5;
					}
				} else {
					continue;
				}
				if (bf->unowned_mines_spots_nearby > 2 && bo.mountain_conqueror) {
					prio += 5;
				}
				prio += std::abs(management_data.get_military_number_at(35)) / 5 *
				        (static_cast<int16_t>(bo.desc->get_conquers()) -
				         static_cast<int16_t>(bf->nearest_buildable_spot_nearby));

				prio += bf->military_score_;

				// if place+building is not good enough
				if (prio <= persistent_data->target_military_score) {
					continue;
				}
				if (prio > persistent_data->ai_personality_mil_upper_limit) {
					persistent_data->ai_personality_mil_upper_limit = prio;
				}
			} else if (bo.type == BuildingObserver::Type::kWarehouse) {

				// exclude spots on border
				if (bf->near_border && !bo.is(BuildingAttribute::kPort)) {
					continue;
				}
				assert(bf->is_portspace != ExtendedBool::kUnset);
				if (bf->is_portspace != ExtendedBool::kTrue && bo.is(BuildingAttribute::kPort)) {
					continue;
				}
				prio += bo.primary_priority;

				// iterating over current warehouses and testing a distance
				// getting distance to nearest warehouse and adding it to a score
				uint16_t nearest_distance = std::numeric_limits<uint16_t>::max();
				for (const WarehouseSiteObserver& wh_obs : warehousesites) {
					const uint16_t actual_distance =
					   map.calc_distance(bf->coords, wh_obs.site->get_position());
					nearest_distance = std::min(nearest_distance, actual_distance);
				}
				// but limit to 30
				const uint16_t max_distance_considered = 30;
				nearest_distance = std::min(nearest_distance, max_distance_considered);
				if (nearest_distance < 13) {
					continue;
				}
				prio +=
				   management_data.neuron_pool[47].get_result_safe(nearest_distance / 2, kAbsValue) / 2;

				prio += bf->own_non_military_nearby * 3;

				// dont be close to enemies
				if (bf->enemy_nearby) {
					prio -= 40;
				}

				// being too close to a border is not good either
				if ((bf->unowned_land_nearby || bf->enemy_owned_land_nearby > 10) &&
				    !bo.is(BuildingAttribute::kPort) && prio > 0) {
					prio /= 2;
					prio -= 10;
				}

			} else if (bo.type == BuildingObserver::Type::kTrainingsite) {

				// Even if a site is forced it has kNeeded necessity now
				assert(bo.primary_priority > 0 && bo.new_building == BuildingNecessity::kNeeded);

				prio += bo.primary_priority;

				// for spots close to a border
				if (bf->near_border) {
					prio -= 5;
				}

				// take care about borders and enemies
				if (bf->enemy_nearby) {
					prio -= 20;
				}

				if (bf->unowned_land_nearby || bf->enemy_owned_land_nearby) {
					prio -= 15;
				}
			}

			// think of space consuming buildings nearby like farms or vineyards
			if (bo.type != BuildingObserver::Type::kMilitarysite) {
				prio -= bf->space_consumers_nearby * 10;
			}

			// Stop here, if priority is 0 or less.
			if (prio <= 0) {
				continue;
			}

			// Prefer road side fields
			prio += bf->preferred ? 5 : 0;

			// don't waste good land for small huts
			const bool space_stress = (spots_avail.at(Widelands::BUILDCAPS_MEDIUM) < 5 ||
			                           spots_avail.at(Widelands::BUILDCAPS_BIG) < 5);

			if (space_stress && bo.type == BuildingObserver::Type::kMilitarysite) {
				prio -= (bf->max_buildcap_nearby - bo.desc->get_size()) * 10;
			} else if (space_stress) {
				prio -= (bf->max_buildcap_nearby - bo.desc->get_size()) * 30;
			} else {
				prio -= (bf->max_buildcap_nearby - bo.desc->get_size()) * 5;
			}

			// prefer vicinity of ports (with exemption of warehouses)
			if (bf->port_nearby && bo.type == BuildingObserver::Type::kMilitarysite) {
				prio *= 2;
			}

			if (bo.type != BuildingObserver::Type::kMilitarysite && highest_nonmil_prio_ < prio) {
				highest_nonmil_prio_ = prio;
			}

			if (bo.type == BuildingObserver::Type::kMilitarysite) {
				if (prio <= persistent_data->target_military_score) {
					continue;
				}
			}

			if (prio > proposed_priority) {
				best_building = &bo;
				proposed_priority = prio;
				proposed_coords = bf->coords;
			}
		}  // ending loop over buildings
	}     // ending loop over fields

	// then try all mines_ - as soon as basic economy is build up.
	if (gametime > next_mine_construction_due_) {

		// not done here
		// update_all_mineable_fields(gametime);
		next_mine_construction_due_ = gametime + kNewMineConstInterval;

		if (!mineable_fields.empty()) {

			for (BuildingObserver& bo : buildings_) {
				if (productionsites.size() <= 8) {
					break;
				}

				if (bo.type != BuildingObserver::Type::kMine) {
					continue;
				}

				assert(bo.new_building != BuildingNecessity::kAllowed);

				// skip if a mine is not required
				if (!(bo.new_building == BuildingNecessity::kNeeded ||
				      bo.new_building == BuildingNecessity::kForced)) {
					continue;
				}

				// iterating over fields
				for (MineableField* const mf : mineable_fields) {
					if (mf->field_info_expiration <= gametime) {
						continue;
					}

					if (mf->coords.field->get_resources() != bo.mines) {
						continue;
					}

					// Continue if field is blocked at the moment
					if (blocked_fields.is_blocked(mf->coords)) {
						continue;
					}

					int32_t prio = 0;
					Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
					   map, Widelands::Area<Widelands::FCoords>(mf->coords, 2));
					do {
						if (bo.mines == mr.location().field->get_resources()) {
							prio += mr.location().field->get_resources_amount();
						}
					} while (mr.advance(map));

					prio /= 10;

					// Only build mines_ on locations where some material can be mined
					if (prio < 1) {
						continue;
					}

					// applying nearnest penalty
					prio -= mf->mines_nearby * std::abs(management_data.get_military_number_at(126));

					// applying max needed
					prio += bo.primary_priority;

					// prefer mines in the middle of mine fields of the
					// same type, so we add a small bonus here
					// depending on count of same mines nearby,
					// though this does not reflects how many resources
					// are (left) in nearby mines
					prio += mf->same_mine_fields_nearby;

					// Continue if field is blocked at the moment
					if (blocked_fields.is_blocked(mf->coords)) {
						continue;
					}

					// Prefer road side fields
					prio += mf->preferred ? 1 : 0;

					prio += bo.primary_priority;

					if (prio > proposed_priority) {
						best_building = &bo;
						proposed_priority = prio;
						proposed_coords = mf->coords;
						mine = true;
					}

					if (prio > highest_nonmil_prio_) {
						highest_nonmil_prio_ = prio;
					}
				}  // end of evaluation of field
			}

		}  // section if mine size >0
	}     // end of mines_ section

	// if there is no winner:
	if (best_building == nullptr) {
		return false;
	}

	if (best_building->type == BuildingObserver::Type::kMilitarysite) {
		assert(proposed_priority >= persistent_data->least_military_score);
		persistent_data->target_military_score = proposed_priority;
		if (persistent_data->target_military_score >
		    persistent_data->ai_personality_mil_upper_limit) {
			persistent_data->ai_personality_mil_upper_limit = persistent_data->target_military_score;
		}
		assert(proposed_priority >= persistent_data->least_military_score);
	}

	// send the command to construct a new building
	game().send_player_build(player_number(), proposed_coords, best_building->id);
	blocked_fields.add(proposed_coords, game().get_gametime() + Duration(2 * 60 * 1000));

	// resetting new_building_overdue
	best_building->new_building_overdue = 0;

	// we block also nearby fields
	// if farms and so on, for quite a long time
	// if military sites only for short time for AI can update information on near buildable fields
	if ((best_building->is(BuildingAttribute::kSpaceConsumer) &&
	     !best_building->is(BuildingAttribute::kRanger)) ||
	    best_building->type == BuildingObserver::Type::kMilitarysite) {
		Time block_time(0);
		uint32_t block_area = 0;
		if (best_building->is(BuildingAttribute::kSpaceConsumer)) {
			if (spots_ > kSpotsEnough) {
				block_time = Time(45 * 60 * 1000);
			} else {
				block_time = Time(15 * 60 * 1000);
			}
			block_area = 5;
		} else {  // militray buildings for a very short time
			block_time = Time(25 * 1000);
			block_area = 6;
		}

		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   map, Widelands::Area<Widelands::FCoords>(map.get_fcoords(proposed_coords), block_area));
		do {
			blocked_fields.add(mr.location(), game().get_gametime() + Duration(block_time.get()));
		} while (mr.advance(map));
	}

	if (best_building->is(BuildingAttribute::kRecruitment)) {
		verb_log_info_time(gametime, "AI %2d: Building a recruitment site: %s\n", player_number(),
		                   best_building->name);
	}

	if (best_building->is(BuildingAttribute::kShipyard)) {
		verb_log_info_time(gametime, "AI %2d: Building a shipyard site: %s Prio:%d \n",
		                   player_number(), best_building->name, proposed_priority);
	}

	if (!(best_building->type == BuildingObserver::Type::kMilitarysite)) {
		best_building->construction_decision_time = gametime;
	} else {
		military_last_build_ = gametime;
		best_building->construction_decision_time = gametime - Duration(kBuildingMinInterval / 2);
	}

	// set the type of update that is needed
	if (mine) {
		next_mine_construction_due_ = gametime + kBusyMineUpdateInterval;
	}

	return true;
}

// Re-calculating warehouse to flag distances
void DefaultAI::check_flag_distances(const Time& gametime) {
	for (WarehouseSiteObserver& wh_obs : warehousesites) {
		uint16_t checked_flags = 0;
		const uint32_t this_wh_hash = wh_obs.site->get_position().hash();
		uint32_t highest_distance_set = 0;

		std::queue<Widelands::Flag*>
		   remaining_flags;  // only used to collect flags reachable walk over roads
		remaining_flags.push(&wh_obs.site->base_flag());
		flag_warehouse_distance.set_distance(
		   wh_obs.site->base_flag().get_position().hash(), 0, gametime, this_wh_hash);
		uint32_t tmp_wh;
		assert(flag_warehouse_distance.get_distance(
		          wh_obs.site->base_flag().get_position().hash(), gametime, &tmp_wh) == 0);

		// Algorithm to walk on roads
		// All nodes are marked as to_be_checked == true first and once the node is checked it is
		// changed to false. Under some conditions, the same node can be checked twice, the
		// to_be_checked can be set back to true. Because less hoops (fewer flag-to-flag roads) does
		// not always mean shortest road.
		while (!remaining_flags.empty()) {
			++checked_flags;
			// looking for a node with shortest existing road distance from starting flag and one that
			// has to be checked Now going over roads leading from this flag
			const uint16_t current_flag_distance = flag_warehouse_distance.get_distance(
			   remaining_flags.front()->get_position().hash(), gametime, &tmp_wh);
			for (uint8_t i = Widelands::WalkingDir::FIRST_DIRECTION;
			     i <= Widelands::WalkingDir::LAST_DIRECTION; ++i) {
				Widelands::Road* const road = remaining_flags.front()->get_road(i);

				if (!road) {
					continue;
				}

				Widelands::Flag* endflag = &road->get_flag(Widelands::Road::FlagStart);

				if (endflag == remaining_flags.front()) {
					endflag = &road->get_flag(Widelands::Road::FlagEnd);
				}
				const uint16_t steps_count = road->get_path().get_nsteps();

				// Calculated distance can be used or ignored if f.e. longer than via other route
				bool const updated = flag_warehouse_distance.set_distance(
				   endflag->get_position().hash(), current_flag_distance + steps_count, gametime,
				   this_wh_hash);

				if (highest_distance_set < current_flag_distance + steps_count) {
					highest_distance_set = current_flag_distance + steps_count;
				}

				if (updated) {
					remaining_flags.push(endflag);
				}
			}
			remaining_flags.pop();
		}
	}

	// Now let do some lazy pruning - remove the flags that were not updated for long
	flag_warehouse_distance.remove_old_flag(gametime);
}

// Here we pick about 25 roads and investigate them. If it is a dead end we dismantle it
bool DefaultAI::dismantle_dead_ends() {
	bool road_dismantled = false;
	const uint16_t stepping = roads.size() / 25 + 1;

	for (uint16_t i = 0; i < roads.size(); i += stepping) {
		const Widelands::Flag& roadstartflag = roads[i]->get_flag(Widelands::RoadBase::FlagStart);
		const Widelands::Flag& roadendflag = roads[i]->get_flag(Widelands::RoadBase::FlagEnd);

		if (!roadstartflag.get_building() && roadstartflag.is_dead_end()) {
			game().send_player_bulldoze(*const_cast<Widelands::Flag*>(&roadstartflag));
			road_dismantled = true;
		}

		if (!roadendflag.get_building() && roadendflag.is_dead_end()) {
			game().send_player_bulldoze(*const_cast<Widelands::Flag*>(&roadendflag));
			road_dismantled = true;
		}
	}
	return road_dismantled;
}

// improves current road system
bool DefaultAI::improve_roads(const Time& gametime) {

	// First try to dismantle some dead ends on road
	if (dead_ends_check_ || gametime.get() % 50 == 0) {
		if (dismantle_dead_ends()) {
			return true;
		}
		dead_ends_check_ = false;
		return false;
	}

	if (!roads.empty()) {
		const Widelands::Path& path = roads.front()->get_path();

		// first force a split on roads that are longer than 3 parts
		if (path.get_nsteps() > 3 && spots_ > kSpotsEnough) {
			const Widelands::Map& map = game().map();
			Widelands::CoordPath cp(map, path);
			// try to split after two steps
			Widelands::CoordPath::StepVector::size_type i = cp.get_nsteps() - 1, j = 1;

			for (; i >= j; --i, ++j) {
				{
					const Widelands::Coords c = cp.get_coords().at(i);

					if (map[c].nodecaps() & Widelands::BUILDCAPS_FLAG) {
						game().send_player_build_flag(player_number(), c);
						return true;
					}
				}
				{
					const Widelands::Coords c = cp.get_coords().at(j);

					if (map[c].nodecaps() & Widelands::BUILDCAPS_FLAG) {
						game().send_player_build_flag(player_number(), c);
						return true;
					}
				}
			}

			// Unable to set a flag - perhaps the road was build stupid
			game().send_player_bulldoze(*const_cast<Widelands::Road*>(roads.front()));
			dead_ends_check_ = true;
			return true;
		}

		roads.push_back(roads.front());
		roads.pop_front();

		// Occasionaly (not more then once in 15 seconds) we test if the road can be dismantled
		// if there is shortage of spots we do it always
		if (last_road_dismantled_ + Duration(15 * 1000) < gametime &&
		    (gametime.get() % 40 == 0 || spots_ <= kSpotsEnough)) {
			const Widelands::Road& road = *roads.front();
			if (dispensable_road_test(*const_cast<Widelands::Road*>(&road))) {
				game().send_player_bulldoze(*const_cast<Widelands::Road*>(&road));
				last_road_dismantled_ = gametime;
				dead_ends_check_ = true;
				return true;
			}
		}
	}
	// now we rotate economies and flags to get one flag to go on with
	if (economies.empty()) {
		check_economies();
		return false;
	}

	if (economies.size() >= 2) {  // rotating economies
		economies.push_back(economies.front());
		economies.pop_front();
	}

	EconomyObserver* eco = economies.front();
	if (eco->flags.empty()) {
		check_economies();
		return false;
	}
	if (eco->flags.size() > 1) {
		eco->flags.push_back(eco->flags.front());
		eco->flags.pop_front();
	}

	const Widelands::Flag& flag = *eco->flags.front();

	// now we test if it is dead end flag, if yes, destroying it
	if (flag.is_dead_end() && flag.current_wares() == 0) {
		game().send_player_bulldoze(*const_cast<Widelands::Flag*>(&flag));
		eco->flags.pop_front();
		return true;
	}

	bool is_warehouse = false;
	if (Widelands::Building* b = flag.get_building()) {
		BuildingObserver& bo = get_building_observer(b->descr().name().c_str());
		if (bo.type == BuildingObserver::Type::kWarehouse) {
			is_warehouse = true;
		}
	}
	const uint32_t flag_coords_hash = flag.get_position().hash();

	if (flag_warehouse_distance.is_road_prohibited(flag_coords_hash, gametime)) {
		return false;
	}
	// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
	// ferries
	const bool needs_warehouse = flag.get_economy(Widelands::wwWORKER)->warehouses().empty();

	uint32_t tmp_wh;

	// when deciding if we attempt to build a road from here we use probability
	uint16_t probability_score = 0;
	if (flag.nr_of_roads() == 1) {
		probability_score += 20;
	}
	if (is_warehouse && flag.nr_of_roads() <= 3) {
		probability_score += 20;
	}
	probability_score += flag.current_wares() * 5;
	if (needs_warehouse) {
		probability_score += 500;
	}
	if (std::rand() % 10 == 0) {  // NOLINT
		probability_score +=
		   flag_warehouse_distance.get_distance(flag_coords_hash, gametime, &tmp_wh);
	}

	if (std::rand() % 200 < probability_score) {  // NOLINT
		create_shortcut_road(flag, 14, gametime);
		return true;
	}

	return false;
}

// This function takes a road (road is smallest section of roads with two flags on the ends)
// look for longer section of road that starts and ends with building/road crossing
// and tries to find alternative route from one end flag to another.
// If route exists, it is not too long, and current road is not intensively used
// the road can be dismantled
bool DefaultAI::dispensable_road_test(const Widelands::Road& road) {

	Widelands::Flag& roadstartflag = road.get_flag(Widelands::RoadBase::FlagStart);
	Widelands::Flag& roadendflag = road.get_flag(Widelands::RoadBase::FlagEnd);

	// Calculating full road (from crossing/building to another crossing/building),
	// this means we calculate vector of all flags of the "full road"
	std::vector<Widelands::Flag*> full_road;
	full_road.push_back(&roadstartflag);
	full_road.push_back(&roadendflag);

	uint16_t road_length = road.get_path().get_nsteps();

	for (int j = 0; j < 2; ++j) {
		bool new_road_found = true;
		while (new_road_found && full_road.back()->nr_of_roads() <= 2 &&
		       full_road.back()->get_building() == nullptr) {
			new_road_found = false;
			for (uint8_t i = Widelands::WalkingDir::FIRST_DIRECTION;
			     i <= Widelands::WalkingDir::LAST_DIRECTION; ++i) {
				Widelands::Road* const near_road = full_road.back()->get_road(i);

				if (!near_road) {
					continue;
				}

				Widelands::Flag* other_end;
				if (near_road->get_flag(Widelands::RoadBase::FlagStart).get_position().hash() ==
				    full_road.back()->get_position().hash()) {
					other_end = &near_road->get_flag(Widelands::RoadBase::FlagEnd);
				} else {
					other_end = &near_road->get_flag(Widelands::RoadBase::FlagStart);
				}

				// Have we already the end of road in our full_road?
				if (std::find(full_road.begin(), full_road.end(), other_end) == full_road.end()) {
					full_road.push_back(other_end);
					road_length += near_road->get_path().get_nsteps();
					new_road_found = true;
					break;
				}
			}
		}
		// we walked to one end, now let revert the content of full_road and repeat in opposite
		// direction
		std::reverse(full_road.begin(), full_road.end());
	}

	// To make decision how intensively the road is used, we count wares on it, but we distinguish
	// situation when entire road has only 2 flags or is longer
	uint16_t wares_on_road = 0;
	assert(full_road.size() > 1);
	if (full_road.size() == 2) {
		wares_on_road = roadstartflag.current_wares() + roadendflag.current_wares();
	} else {
		// We count wares only on inner flags
		for (uint16_t k = 1; k < full_road.size() - 1; ++k) {
			wares_on_road += full_road[k]->current_wares();
		}
	}

	// If it by chance starts or ends next to a warehouse...
	if (Widelands::Building* b = full_road.front()->get_building()) {
		BuildingObserver& bo = get_building_observer(b->descr().name().c_str());
		if (bo.type == BuildingObserver::Type::kWarehouse) {
			return false;
		}
	}
	if (Widelands::Building* b = full_road.back()->get_building()) {
		BuildingObserver& bo = get_building_observer(b->descr().name().c_str());
		if (bo.type == BuildingObserver::Type::kWarehouse) {
			return false;
		}
	}

	if (spots_ > kSpotsEnough && wares_on_road > 5) {
		return false;
	} else if (wares_on_road > 8) {
		return false;
	}

	std::priority_queue<NearFlag> queue;
	// only used to collect flags reachable walking over roads
	std::vector<NearFlag> reachableflags;

	queue.push(NearFlag(full_road.front(), 0));
	uint16_t alternative_path = std::numeric_limits<uint16_t>::max();
	const uint8_t checkradius = 3 * game().map().calc_distance(full_road.front()->get_position(),
	                                                           full_road.back()->get_position());

	// algorithm to walk on roads
	while (!queue.empty()) {

		// Testing if we stand on the roadendflag... if yes, the alternative path is found, no reason
		// to go on
		if (full_road.back()->get_position().x == queue.top().flag->get_position().x &&
		    full_road.back()->get_position().y == queue.top().flag->get_position().y) {
			alternative_path = queue.top().current_road_distance;
			break;
		}

		// If we were here, do not evaluate the flag again
		std::vector<NearFlag>::iterator f =
		   find(reachableflags.begin(), reachableflags.end(), queue.top().flag);
		if (f != reachableflags.end()) {
			queue.pop();
			continue;
		}

		reachableflags.push_back(queue.top());
		queue.pop();
		NearFlag& nf = reachableflags.back();

		// Now go over roads going from this flag
		for (uint8_t i = Widelands::WalkingDir::FIRST_DIRECTION;
		     i <= Widelands::WalkingDir::LAST_DIRECTION; ++i) {
			Widelands::Road* const near_road = nf.flag->get_road(i);

			if (!near_road) {
				continue;
			}

			// alternate road cannot lead via road to be dismantled
			if (near_road->serial() == road.serial()) {
				continue;
			}

			Widelands::Flag* endflag = &near_road->get_flag(Widelands::RoadBase::FlagStart);

			if (endflag == nf.flag) {
				endflag = &near_road->get_flag(Widelands::RoadBase::FlagEnd);
			}

			// When walking on nearby roads, we do not go too far from start and end of road
			const int32_t dist1 =
			   game().map().calc_distance(full_road.front()->get_position(), endflag->get_position());
			const int32_t dist2 =
			   game().map().calc_distance(full_road.back()->get_position(), endflag->get_position());
			if ((dist1 + dist2) > checkradius) {
				continue;
			}

			const uint32_t new_length = nf.current_road_distance + near_road->get_path().get_nsteps();
			queue.push(NearFlag(endflag, new_length));
		}
	}

	return alternative_path + wares_on_road <= road_length + 12;
}

// Trying to connect the flag to another one, be it from own economy
// or other economy
// The procedure is:
// - Collect all flags within checkradius into RoadCandidates, but first we dont even know if a road
// can be built to them
// - Walking over road network to collect info on flags that are accessible over road network
// - Then merge info from NearFlags to RoadCandidates and consider roads to few best candidates from
// RoadCandidates. We use score named "reduction" that is basically diff between connection over
// existing roads minus possible road from starting flag to candidate flag. Of course there are two
// special cases:
// - the candidate flag does not belong to the same economy, so no road connection exists
// - they are from same economy, but are connected beyond range of checkradius, so actual length of
// connection is not known
bool DefaultAI::create_shortcut_road(const Widelands::Flag& flag,
                                     uint16_t checkradius,
                                     const Time& gametime) {

	// Increasing the failed_connection_tries counter
	// At the same time it indicates a time an economy is without a warehouse
	// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
	// ferries
	EconomyObserver* eco = get_economy_observer(flag.economy(Widelands::wwWORKER));
	// if we passed grace time this will be last attempt and if it fails
	// building is destroyed
	bool last_attempt_ = false;

	// this should not happen, but if the economy has a warehouse and a dismantle
	// grace time set, we must 'zero' the dismantle grace time
	if (!flag.get_economy(Widelands::wwWORKER)->warehouses().empty() &&
	    eco->dismantle_grace_time.is_valid()) {
		eco->dismantle_grace_time = Time();
	}

	// first we deal with situations when this is economy with no warehouses
	// and this is a flag belonging to a building/constructionsite
	// such economy must get dismantle grace time (if not set yet)
	// end sometimes extended checkradius
	if (flag.get_economy(Widelands::wwWORKER)->warehouses().empty() && flag.get_building()) {

		// occupied military buildings get special treatment
		// (extended grace time + longer radius)
		bool occupied_military_ = false;
		Widelands::Building* b = flag.get_building();
		if (upcast(Widelands::MilitarySite, militb, b)) {
			occupied_military_ = !militb->soldier_control()->stationed_soldiers().empty();
		}

		// check if we are within grace time, if not or gracetime unset we need to do something
		// if we are within gracetime we do nothing (this loop is skipped)

		// if grace time is not set, this is probably first time without a warehouse and we must set
		// it
		if (eco->dismantle_grace_time.is_invalid()) {

			// constructionsites
			if (upcast(Widelands::ConstructionSite const, constructionsite, flag.get_building())) {
				BuildingObserver& bo =
				   get_building_observer(constructionsite->building().name().c_str());
				// first very special case - a port (in the phase of constructionsite)
				// this might be a new colonization port
				if (bo.is(BuildingAttribute::kPort)) {
					eco->dismantle_grace_time =
					   gametime + Duration(60 * 60 * 1000);  // one hour should be enough
				} else {  // other constructionsites, usually new (standalone) constructionsites
					eco->dismantle_grace_time =
					   gametime + Duration(30 * 1000 +  // very short time is enough
					                       (eco->flags.size() * 30 *
					                        1000));  // + 30 seconds for every flag in economy
				}

				// buildings
			} else {

				if (occupied_military_) {
					eco->dismantle_grace_time =
					   gametime + Duration((90 * 60 * 1000) + (eco->flags.size() * 20 * 1000));

				} else {  // for other normal buildings
					eco->dismantle_grace_time =
					   gametime + Duration((45 * 60 * 1000) + (eco->flags.size() * 20 * 1000));
				}
			}

			// we have passed grace_time - it is time to dismantle
		} else if (eco->dismantle_grace_time <= gametime) {
			last_attempt_ = true;
			// we increase a check radius in last attempt
			checkradius += 2;
		}

		// and bonus for occupied military buildings:
		if (occupied_military_) {
			checkradius += 4;
		}

		// and generally increase radius for unconnected buildings
		checkradius += 2;
	}

	// Now own roadfinding stuff
	const Widelands::Map& map = game().map();

	// Initializing new object of FlagsForRoads, we will push there all candidate flags
	// First we dont even know if a road can be built there (from current flag)
	// Adding also distance of this flag to nearest wh
	uint32_t tmp_wh;  // This information is not used, but we need it
	const uint32_t current_flag_dist_to_wh =
	   flag_warehouse_distance.get_distance(flag.get_position().hash(), gametime, &tmp_wh);

	FlagCandidates flag_candidates(current_flag_dist_to_wh);

	FindNodeWithFlagOrRoad functor;
	CheckStepRoadAI check(player_, Widelands::MOVECAPS_WALK, true);

	// get all flags within radius
	std::vector<Widelands::Coords> reachable;
	map.find_reachable_fields(
	   game(),
	   Widelands::Area<Widelands::FCoords>(map.get_fcoords(flag.get_position()), checkradius),
	   &reachable, check, functor);

	for (const Widelands::Coords& reachable_coords : reachable) {

		// ignore starting flag, of course
		if (reachable_coords == flag.get_position()) {
			continue;
		}

		const uint32_t reachable_coords_hash = reachable_coords.hash();

		// first make sure there is an immovable (should be, but still)
		Widelands::BaseImmovable* this_immovable = map[reachable_coords].get_immovable();
		if (upcast(Widelands::PlayerImmovable const, player_immovable, this_immovable)) {

			// if it is the road, make a flag there
			if (this_immovable->descr().type() == Widelands::MapObjectType::ROAD) {
				game().send_player_build_flag(player_number(), reachable_coords);
			}

			// do not go on if it is not a flag
			if (this_immovable->descr().type() != Widelands::MapObjectType::FLAG) {
				continue;
			}

			// testing if a flag/road's economy has a warehouse, if not we are not
			// interested to connect to it
			// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
			// ferries
			if (player_immovable->economy(Widelands::wwWORKER).warehouses().empty()) {
				continue;
			}

			// This is a candidate, sending all necessary info to RoadCandidates
			const bool is_different_economy = (player_immovable->get_economy(Widelands::wwWORKER) !=
			                                   flag.get_economy(Widelands::wwWORKER));
			const uint16_t air_distance = map.calc_distance(flag.get_position(), reachable_coords);

			if (!flag_candidates.has_candidate(reachable_coords_hash) &&
			    !flag_warehouse_distance.is_road_prohibited(reachable_coords_hash, gametime)) {
				flag_candidates.add_flag(
				   reachable_coords_hash, is_different_economy,
				   flag_warehouse_distance.get_distance(reachable_coords_hash, gametime, &tmp_wh),
				   air_distance);
			}
		}
	}

	// now we walk over roads and if field is reachable by roads, we change the distance assigned
	// above
	std::map<uint32_t, NearFlag> nearflags;  // only used to collect flags reachable walk over roads
	nearflags[flag.get_position().hash()] = NearFlag(&flag, 0);

	collect_nearflags(nearflags, flag, checkradius);

	// Sending calculated walking costs from nearflags to RoadCandidates to update info on
	// Candidate flags/roads
	for (auto& nf_walk : nearflags) {
		const uint32_t nf_hash = nf_walk.second.flag->get_position().hash();
		// NearFlag contains also flags beyond check radius, these are not relevant for us
		if (flag_candidates.has_candidate(nf_hash)) {
			flag_candidates.set_cur_road_distance(nf_hash, nf_walk.second.current_road_distance);
		}
	}

	// Here we must consider how much are buildable fields lacking
	// the number will be transformed to a weight passed to findpath function
	int32_t fields_necessity = 0;
	if (spots_ < kSpotsTooLittle) {
		fields_necessity += 10;
	}
	if (map_allows_seafaring_ && num_ports == 0) {
		fields_necessity += 10;
	}
	if (num_ports < 4) {
		fields_necessity += 5;
	}
	if (spots_ < kSpotsEnough) {
		fields_necessity += 5;
	}

	fields_necessity *= std::abs(management_data.get_military_number_at(64)) * 5;

	// Now we calculate roads from here to few best looking RoadCandidates....
	flag_candidates.sort_by_air_distance();
	uint32_t possible_roads_count = 0;
	for (const auto& flag_candidate : flag_candidates.flags()) {
		if (possible_roads_count > 10) {
			break;
		}
		const Widelands::Coords coords = Widelands::Coords::unhash(flag_candidate.coords_hash);
		Widelands::Path path;

		// value of pathcost is not important, it just indicates, that the path can be built
		// We send this information to RoadCandidates, with length of possible road if applicable
		const int32_t pathcost =
		   map.findpath(flag.get_position(), coords, 0, path, check, 0, fields_necessity);
		if (pathcost >= 0) {
			flag_candidates.set_road_possible(flag_candidate.coords_hash, path.get_nsteps());
			++possible_roads_count;
		}
	}

	// re-sorting again now by default by a score
	flag_candidates.sort();

	// Well and finally building the winning road (if any)
	const int32_t winner_min_score = (spots_ < kSpotsTooLittle) ? 50 : 25;

	FlagCandidates::Candidate* winner = flag_candidates.get_winner(winner_min_score);
	if (winner) {
		const Widelands::Coords target_coords = Widelands::Coords::unhash(winner->coords_hash);

		// This is to prohibit the flag for some time but with exemption of warehouse
		if (flag_warehouse_distance.get_distance(winner->coords_hash, gametime, &tmp_wh) > 0) {
			flag_warehouse_distance.set_road_built(winner->coords_hash, gametime);
		}
		// and we straight away set distance of future flag
		flag_warehouse_distance.set_distance(
		   flag.get_position().hash(), winner->start_flag_dist_to_wh + winner->possible_road_distance,
		   gametime, 0);  // faking the warehouse

		Widelands::Path& path = *new Widelands::Path();
#ifndef NDEBUG
		const int32_t pathcost =
		   map.findpath(flag.get_position(), target_coords, 0, path, check, 0, fields_necessity);
		assert(pathcost >= 0);
#else
		map.findpath(flag.get_position(), target_coords, 0, path, check, 0, fields_necessity);
#endif
		game().send_player_build_road(player_number(), path);
		return true;
	}
	// We can't build a road so let's block the vicinity as an indication this area is not
	// connectible
	// Usually we block for 2 minutes, but if it is a last attempt we block for 10 minutes
	// Note: we block the vicinity only if this economy (usually a sole flag with a building) is not
	// connected to a warehouse
	// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
	// ferries
	if (flag.get_economy(Widelands::wwWORKER)->warehouses().empty()) {

		// blocking only if latest block was less then 60 seconds ago or it is last attempt
		if (eco->fields_block_last_time + Duration((60 * 1000)) < gametime || last_attempt_) {
			eco->fields_block_last_time = gametime;

			const Duration block_time((last_attempt_ ? 10 : 2) * 60 * 1000);

			FindNodeAcceptAll buildable_functor;
			CheckStepOwnTerritory check_own(player_, Widelands::MOVECAPS_WALK, true);

			// get all flags within radius
			std::vector<Widelands::Coords> reachable_to_block;
			map.find_reachable_fields(
			   game(),
			   Widelands::Area<Widelands::FCoords>(map.get_fcoords(flag.get_position()), checkradius),
			   &reachable_to_block, check_own, buildable_functor);

			for (auto coords : reachable_to_block) {
				blocked_fields.add(coords, game().get_gametime() + block_time);
			}
		}

		// If it last attempt we also destroy the flag (with a building if any attached)
		if (last_attempt_) {
			remove_from_dqueue<Widelands::Flag>(eco->flags, &flag);
			game().send_player_bulldoze(*const_cast<Widelands::Flag*>(&flag));
			dead_ends_check_ = true;
			return true;
		}
	}
	return false;
}

void DefaultAI::collect_nearflags(std::map<uint32_t, NearFlag>& nearflags,
                                  const Widelands::Flag& flag,
                                  const uint16_t checkradius) {
	// Algorithm to walk on roads
	// All nodes are marked as to_be_checked == true first and once the node is checked it is changed
	// to false. Under some conditions, the same node can be checked twice, the to_be_checked can
	// be set back to true. Because less hoops (fewer flag-to-flag roads) does not always mean
	// shortest road.

	const Widelands::Map& map = game().map();

	for (;;) {
		// looking for a node with shortest existing road distance from starting flag and one that has
		// to be checked
		uint32_t start_field = kNoField;
		uint32_t nearest_distance = 10000;
		for (auto item : nearflags) {
			if (item.second.current_road_distance < nearest_distance && item.second.to_be_checked) {
				nearest_distance = item.second.current_road_distance;
				start_field = item.first;
			}
		}
		// OK, we failed to find a NearFlag where to_be_checked == true, so quitting the loop now
		if (start_field == kNoField) {
			break;
		}

		nearflags[start_field].to_be_checked = false;

		// Now going over roads leading from this flag
		for (uint8_t i = Widelands::WalkingDir::FIRST_DIRECTION;
		     i <= Widelands::WalkingDir::LAST_DIRECTION; ++i) {
			Widelands::Road* const road = nearflags[start_field].flag->get_road(i);

			if (!road) {
				continue;
			}

			Widelands::Flag* endflag = &road->get_flag(Widelands::Road::FlagStart);

			if (endflag == nearflags[start_field].flag) {
				endflag = &road->get_flag(Widelands::Road::FlagEnd);
			}

			const uint32_t endflag_hash = endflag->get_position().hash();

			const int32_t dist = map.calc_distance(flag.get_position(), endflag->get_position());

			if (dist > checkradius + 2) {  //  Testing bigger vicinity then checkradius....
				continue;
			}

			// There is few scenarios for this neighbour flag
			if (nearflags.count(endflag_hash) == 0) {
				// This is brand new flag
				// calculating diff how much closer we will get to the flag
				nearflags[endflag_hash] =
				   NearFlag(endflag, nearflags[start_field].current_road_distance +
				                        road->get_path().get_nsteps());
			} else {
				// We know about this flag already
				if (nearflags[endflag_hash].current_road_distance >
				    nearflags[start_field].current_road_distance + road->get_path().get_nsteps()) {
					// ..but this current connection is shorter than one found before
					nearflags[endflag_hash].current_road_distance =
					   nearflags[start_field].current_road_distance + road->get_path().get_nsteps();
					// So let re-check neighbours once more
					nearflags[endflag_hash].to_be_checked = true;
				}
			}
		}
	}
}

/**
 * Checks if anything in one of the economies changed and takes care for these
 * changes.
 *
 * \returns true, if something was changed.
 */
bool DefaultAI::check_economies() {
	while (!new_flags.empty()) {
		const Widelands::Flag& flag = *new_flags.front();
		new_flags.pop_front();
		// TODO(Nordfriese): Someone must urgently update the code since the big economy splitting for
		// the ferries
		get_economy_observer(flag.economy(Widelands::wwWORKER))->flags.push_back(&flag);
	}

	for (std::deque<EconomyObserver*>::iterator obs_iter = economies.begin();
	     obs_iter != economies.end(); ++obs_iter) {
		// check if any flag has changed its economy
		std::deque<Widelands::Flag const*>& fl = (*obs_iter)->flags;

		for (std::deque<Widelands::Flag const*>::iterator j = fl.begin(); j != fl.end();) {
			if (&(*obs_iter)->economy != &(*j)->economy(Widelands::wwWORKER)) {
				// the flag belongs to other economy so we must assign it there
				get_economy_observer((*j)->economy(Widelands::wwWORKER))->flags.push_back(*j);
				// and erase from this economy's observer
				j = fl.erase(j);
			} else {
				++j;
			}
		}

		// if there are no more flags in this economy,
		// we no longer need it's observer
		if ((*obs_iter)->flags.empty()) {
			delete *obs_iter;
			economies.erase(obs_iter);
			return true;
		}
	}
	return false;
}

/**
 * checks the first productionsite in list, takes care if it runs out of
 * resources and finally reenqueues it at the end of the list.
 *
 * \returns true, if something was changed.
 */
bool DefaultAI::check_productionsites(const Time& gametime) {
	if (productionsites.empty()) {
		return false;
	}

	// Reorder and set new values; - better now because there are multiple returns in the function
	productionsites.push_back(productionsites.front());
	productionsites.pop_front();

	// Get link to productionsite that should be checked
	ProductionSiteObserver& site = productionsites.front();

	// Inform if we are above ai type limit.
	if (site.bo->total_count() > site.bo->cnt_limit_by_aimode) {
		verb_log_warn_time(gametime, "AI check_productionsites: Too many %s: %d, ai limit: %d\n",
		                   site.bo->name, site.bo->total_count(), site.bo->cnt_limit_by_aimode);
	}

	// first we werify if site is working yet (can be unoccupied since the start)
	if (!site.site->can_start_working()) {
		site.unoccupied_till = game().get_gametime();
	}

	// is it connected to wh at all?
	// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
	// ferries
	const bool connected_to_wh = !site.site->get_economy(Widelands::wwWORKER)->warehouses().empty();

	// do not dismantle or upgrade the same type of building too soon - to give some time to update
	// statistics
	if (game().get_gametime() <
	    site.bo->last_dismantle_time +
	       Duration((std::abs(management_data.get_military_number_at(164)) / 25 + 1) * 60 * 1000)) {
		return false;
	}

	const Widelands::Map& map = game().map();

	// First we check if we must release an experienced worker
	// iterate over all working positions of the actual productionsite
	for (uint8_t i = 0; i < site.site->descr().nr_working_positions(); i++) {
		// get the pointer to the worker assigned to the actual position
		const Widelands::Worker* cw = site.site->working_positions()->at(i).worker.get(game());
		if (cw) {  // a worker is assigned to the position
			// get the descritpion index of the worker assigned on this position
			Widelands::DescriptionIndex current_worker = cw->descr().worker_index();
			// if description indexes of assigned worker and normal worker differ
			// (this means an experienced worker is assigned to the position)
			// and we have none of the experienced workers on stock
			if (current_worker != site.bo->positions.at(i) &&
			    calculate_stocklevel(current_worker, WareWorker::kWorker) < 1) {
				// kick out the worker
				game().send_player_evict_worker(
				   *site.site->working_positions()->at(i).worker.get(game()));
				return true;
			}
		}
	}

	// The code here is bit complicated
	// a) Either this site is pending for upgrade, if ready, order the upgrade
	// b) other site of type is pending for upgrade
	// c) if none of above, we can consider upgrade of this one

	const Widelands::DescriptionIndex enhancement = site.site->descr().enhancement();

	bool considering_upgrade = enhancement != Widelands::INVALID_INDEX;

	if (considering_upgrade && !basic_economy_established &&
	    management_data.f_neuron_pool[17].get_position(2)) {
		const Widelands::BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
		BuildingObserver& en_bo = get_building_observer(bld.name().c_str());
		if (en_bo.basic_amount < 1) {
			considering_upgrade = false;
		}
	}

	// First we check for rare case when input wares are set to 0 but AI is not aware that
	// the site is pending for upgrade - one possible cause is this is a freshly loaded game
	if (!site.upgrade_pending) {
		bool resetting_wares = false;
		for (auto& queue : site.site->inputqueues()) {
			if (queue->get_max_fill() == 0) {
				resetting_wares = true;
				game().send_player_set_input_max_fill(
				   *site.site, queue->get_index(), queue->get_type(), queue->get_max_size());
			}
		}
		if (resetting_wares) {
			verb_log_info_time(gametime,
			                   " %d: AI: input queues were reset to max for %s (game just loaded?)\n",
			                   player_number(), site.bo->name);
			return true;
		}
	}

	if (site.upgrade_pending) {
		// The site is in process of emptying its input queues
		// Do nothing when some wares are left, but do not wait more then 4 minutes
		if (site.bo->construction_decision_time + Duration(4 * 60 * 1000) > gametime &&
		    !set_inputs_to_zero(site)) {
			return false;
		}
		assert(site.bo->cnt_upgrade_pending == 1);
		assert(enhancement != Widelands::INVALID_INDEX);
		game().send_player_enhance_building(*site.site, enhancement, true);
		return true;
	} else if (site.bo->cnt_upgrade_pending > 0) {
		// some other site of this type is in pending for upgrade
		assert(site.bo->cnt_upgrade_pending == 1);
		return false;
	}
	assert(site.bo->cnt_upgrade_pending == 0);

	// Of course type of productionsite must be allowed
	if (considering_upgrade && !player_->is_building_type_allowed(enhancement)) {
		considering_upgrade = false;
	}

	// Site must be connected to warehouse
	if (considering_upgrade && !connected_to_wh) {
		considering_upgrade = false;
	}

	// If upgrade produces new outputs, we upgrade unless the site is younger
	// then 10 minutes. Otherwise the site must be older then 20 minutes and
	// gametime > 45 minutes.
	if (considering_upgrade) {
		const Widelands::BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
		BuildingObserver& en_bo = get_building_observer(bld.name().c_str());
		Widelands::FCoords f = map.get_fcoords(site.site->get_position());
		BuildableField bf(f);
		update_buildable_field(bf);

		if (site.bo->is(BuildingAttribute::kUpgradeExtends)) {
			if (gametime < site.built_time + Duration(10 * 60 * 1000)) {
				considering_upgrade = false;
			}
		} else {
			if (gametime < Time(45 * 60 * 1000) ||
			    gametime < site.built_time + Duration(20 * 60 * 1000)) {
				considering_upgrade = false;
			}
		}
		// if supporters are required only upgrade if there are any nearby
		if (en_bo.requires_supporters && bf.supporters_nearby.count(en_bo.name) < 1) {
			considering_upgrade = false;
		}
		// if upgraded building is part of basic economy we allow earlier upgrade
		if (en_bo.cnt_built < static_cast<int32_t>(en_bo.basic_amount)) {
			considering_upgrade = true;
		}
	}

	// No upgrade without proper workers
	if (considering_upgrade && !site.site->has_workers(enhancement, game())) {
		const Widelands::BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
		BuildingObserver& en_bo = get_building_observer(bld.name().c_str());
		if (get_stocklevel(en_bo, gametime, WareWorker::kWorker) < 1) {
			considering_upgrade = false;
		}
	}

	if (considering_upgrade) {

		const Widelands::BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
		BuildingObserver& en_bo = get_building_observer(bld.name().c_str());
		bool doing_upgrade = false;

		// 10 minutes is a time to productions statics to settle
		if ((en_bo.last_building_built.is_invalid() ||
		     gametime - en_bo.last_building_built >= Duration(10 * 60 * 1000)) &&
		    (en_bo.cnt_under_construction + en_bo.unoccupied_count) == 0) {

			// forcing first upgrade
			if (en_bo.total_count() == 0 &&
			    (site.bo->cnt_built > 1 || site.bo->is(BuildingAttribute::kUpgradeSubstitutes))) {
				doing_upgrade = true;
			}

			if (en_bo.total_count() == 1 &&
			    (site.bo->cnt_built > 1 || site.bo->is(BuildingAttribute::kUpgradeSubstitutes))) {
				if (en_bo.current_stats > 55) {
					doing_upgrade = true;
				}
			}

			if (en_bo.total_count() > 1 &&
			    (site.bo->cnt_built > 2 || site.bo->is(BuildingAttribute::kUpgradeSubstitutes))) {
				if (en_bo.current_stats > 75) {
					doing_upgrade = true;
				}
			}

			// Don't forget about limitation of number of buildings
			if (en_bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
				doing_upgrade = false;
			}
		}

		// Here we just restrict input wares to 0 and set flag 'upgrade_pending' to true
		if (doing_upgrade) {
			set_inputs_to_zero(site);
			site.bo->construction_decision_time = gametime;
			en_bo.construction_decision_time = gametime;
			site.upgrade_pending = true;
			++site.bo->cnt_upgrade_pending;
			return true;
		}
	}

	// Barracks
	if (site.bo->is(BuildingAttribute::kBarracks)) {
		// If we somehow have more than one barracks we will dismantle current one
		if (site.bo->total_count() > 1) {
			verb_log_info_time(
			   gametime,
			   "AI %2d: We have %d barracks, that is not supported by AI and if caused by AI it is an "
			   "error; dismantling the barracks at %3dx%3d\n",
			   player_number(), site.bo->total_count(), site.site->get_position().x,
			   site.site->get_position().y);
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		assert(site.bo->total_count() == 1);
		for (auto& queue : site.site->inputqueues()) {
			if (queue->get_max_fill() > 4) {
				game().send_player_set_input_max_fill(
				   *site.site, queue->get_index(), queue->get_type(), 4);
			}
		}

		// AI takes multiple inputs into account and makes decision if barracks to be stopped/started
		int16_t tmp_score = 0;
		int16_t inputs[kFNeuronBitSize] = {0};
		const Widelands::PlayerNumber pn = player_number();
		tmp_score += (soldier_status_ == SoldiersStatus::kBadShortage) * 8;
		tmp_score += (soldier_status_ == SoldiersStatus::kShortage) * 4;
		tmp_score += (soldier_status_ == SoldiersStatus::kEnough) * 2;
		tmp_score += (soldier_status_ == SoldiersStatus::kFull) * 1;
		inputs[2] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) * -1;
		inputs[3] = (expansion_type.get_expansion_type() == ExpansionMode::kSpace) * 1;
		inputs[4] = -1;
		inputs[5] = -2;
		inputs[6] = -3;
		inputs[14] =
		   (player_statistics.get_player_power(pn) < player_statistics.get_old_player_power(pn)) * 1;
		inputs[15] =
		   (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn)) *
		   1;
		inputs[16] =
		   (player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn)) * 1;
		inputs[17] =
		   (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn)) *
		   1;
		inputs[18] = (expansion_type.get_expansion_type() == ExpansionMode::kSpace) * -1;
		inputs[19] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) * 1;
		inputs[20] = 1;
		inputs[21] = 2;
		inputs[22] = 3;
		inputs[23] = (ts_without_trainers_ > 0) ? -1 : 0;
		inputs[24] = (ts_without_trainers_ > 0) ? -2 : 0;
		inputs[25] = (ts_without_trainers_ > 0) ? -3 : 0;
		for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
			if (management_data.f_neuron_pool[24].get_position(i)) {
				tmp_score += inputs[i];
			}
		}

		// starting the site
		if (site.site->is_stopped() && tmp_score >= 0) {
			game().send_player_start_stop_building(*site.site);
			for (auto& queue : site.site->inputqueues()) {
				game().send_player_set_input_max_fill(
				   *site.site, queue->get_index(), queue->get_type(), 4);
			}
		}
		// stopping the site
		if (!site.site->is_stopped() && tmp_score < 0) {
			game().send_player_start_stop_building(*site.site);
			for (auto& queue : site.site->inputqueues()) {
				game().send_player_set_input_max_fill(
				   *site.site, queue->get_index(), queue->get_type(), 2);
			}
		}
	}

	// Lumberjack / Woodcutter handling
	if (site.bo->is(BuildingAttribute::kLumberjack)) {

		// do not dismantle immediately
		if ((game().get_gametime() - site.built_time) < Duration(6 * 60 * 1000)) {
			return false;
		}

		// Do not destruct the last few lumberjacks
		if (site.bo->cnt_built <= site.bo->cnt_target) {
			return false;
		}

		if (site.site->get_statistics_percent() >
		    std::abs(management_data.get_military_number_at(117)) / 2) {
			return false;
		}

		unsigned trees_nearby = find_immovables_nearby(site.site->descr().collected_attributes(),
		                                               map.get_fcoords(site.site->get_position()),
		                                               site.bo->desc->workarea_info_);

		if (trees_nearby > trees_nearby_treshold_ / 3) {
			return false;
		}

		// so finally we dismantle the lumberjack
		site.bo->last_dismantle_time = game().get_gametime();
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site, true);
		} else {
			game().send_player_bulldoze(*site.site);
		}

		return true;
	}

	// Wells handling
	if (site.bo->is(BuildingAttribute::kWell)) {
		// Never get below target count of wells
		if (site.bo->total_count() <= site.bo->cnt_target) {
			return false;
		}

		if (site.unoccupied_till + Duration(6 * 60 * 1000) < gametime &&
		    site.site->get_statistics_percent() == 0) {
			site.bo->last_dismantle_time = gametime;
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}

			return true;
		}

		// do not consider dismantling if we are under target
		if (site.bo->last_dismantle_time + Duration(90 * 1000) > game().get_gametime()) {
			return false;
		}

		// now we test the stocklevel and dismantle the well if we have enough water
		// but first we make sure we do not dismantle a well too soon
		// after dismantling previous one
		if (get_stocklevel(*site.bo, gametime) > 250 + productionsites.size() * 5) {  // dismantle
			site.bo->last_dismantle_time = game().get_gametime();
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		return false;
	}

	// Quarry handling
	if (site.bo->is(BuildingAttribute::kNeedsRocks)) {

		unsigned rocks_nearby = find_immovables_nearby(site.site->descr().collected_attributes(),
		                                               map.get_fcoords(site.site->get_position()),
		                                               site.bo->desc->workarea_info_);

		if (rocks_nearby == 0) {
			// destruct the building and it's flag (via flag destruction)
			// the destruction of the flag avoids that defaultAI will have too many
			// unused roads - if needed the road will be rebuild directly.
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		if (site.unoccupied_till + Duration(6 * 60 * 1000) < gametime &&
		    site.site->get_statistics_percent() == 0) {
			// it is possible that there are rocks but quarry is not able to mine them
			site.bo->last_dismantle_time = game().get_gametime();
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}

			return true;
		}

		return false;
	}

	// All other SPACE_CONSUMERS without input and above target_count
	if (site.bo->inputs.empty()                  // does not consume anything
	    && site.bo->supported_producers.empty()  // not a renewing building (forester...)
	    && site.bo->is(BuildingAttribute::kSpaceConsumer) &&
	    !site.bo->is(BuildingAttribute::kRanger)) {

		// if we have more buildings then target
		if ((site.bo->cnt_built - site.bo->unconnected_count) > site.bo->cnt_target &&
		    site.unoccupied_till +
		          Duration((std::abs(management_data.get_military_number_at(166)) / 5 + 1) * 60 *
		                   1000) <
		       gametime &&
		    site.site->can_start_working()) {

			if (site.site->get_statistics_percent() < 30 && get_stocklevel(*site.bo, gametime) > 100) {
				site.bo->last_dismantle_time = game().get_gametime();
				if (connected_to_wh) {
					game().send_player_dismantle(*site.site, true);
				} else {
					game().send_player_bulldoze(*site.site);
				}
				return true;
			}
		}

		// a building can be dismanteld if it performs too bad, if it is not the last one
		if (site.site->get_statistics_percent() <= 10 && site.bo->cnt_built > 1 &&
		    site.unoccupied_till +
		          Duration((std::abs(management_data.get_military_number_at(167)) / 5 + 1) * 60 *
		                   1000) <
		       gametime &&
		    site.site->can_start_working() &&
		    get_stocklevel(*site.bo, gametime) >
		       static_cast<unsigned int>(
		          (std::abs(management_data.get_military_number_at(168)) / 5))) {

			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		// Blocking the vicinity if too low performance and ware is still needed
		if (site.site->get_statistics_percent() <= 50 || get_stocklevel(*site.bo, gametime) < 5) {
			Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
			   map, Widelands::Area<Widelands::FCoords>(
			           map.get_fcoords(site.site->base_flag().get_position()), 5));
			do {
				blocked_fields.add(mr.location(), gametime + Duration(5 * 60 * 100));
			} while (mr.advance(map));
		}

		return false;
	}

	// buildings with inputs, checking if we can a dismantle some due to low performance
	if (!site.bo->inputs.empty() && (site.bo->cnt_built - site.bo->unoccupied_count) >= 3 &&
	    site.site->can_start_working() &&
	    check_building_necessity(*site.bo, PerfEvaluation::kForDismantle, gametime) ==
	       BuildingNecessity::kNotNeeded &&
	    gametime - site.bo->last_dismantle_time >
	       Duration(static_cast<uint32_t>(
	          (std::abs(management_data.get_military_number_at(169)) / 5 + 1) * 60 * 1000)) &&

	    site.bo->current_stats > site.site->get_statistics_percent() &&  // underperformer
	    (game().get_gametime() - site.unoccupied_till) > Duration(10 * 60 * 1000)) {

		site.bo->last_dismantle_time = game().get_gametime();

		if (connected_to_wh) {
			game().send_player_dismantle(*site.site, true);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		return true;
	}

	// remaining buildings without inputs and not supporting ones (fishers only left probably and
	// hunters)
	if (site.bo->inputs.empty() && site.bo->supported_producers.empty() &&
	    site.site->can_start_working() && !site.bo->is(BuildingAttribute::kSpaceConsumer) &&
	    site.site->get_statistics_percent() < 5 &&
	    ((game().get_gametime() - site.built_time) > Duration(10 * 60 * 1000))) {

		site.bo->last_dismantle_time = game().get_gametime();
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site, true);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		return true;
	}

	// supporting productionsites (rangers)
	// stop/start them based on stock avaiable
	if (!site.bo->supported_producers.empty()) {

		if (!site.bo->is(BuildingAttribute::kRanger)) {
			// other supporting sites, like fish breeders, gamekeepers are not dismantled at all
			return false;
		}

		// dismantling the rangers hut, but only if we have them above a target
		if (wood_policy_.at(site.bo->id) == WoodPolicy::kDismantleRangers &&
		    site.bo->cnt_built > site.bo->cnt_target) {

			site.bo->last_dismantle_time = game().get_gametime();
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		// stopping a ranger (sometimes the policy can be kDismantleRangers,
		// but we still preserve some rangers for sure)
		if ((wood_policy_.at(site.bo->id) == WoodPolicy::kStopRangers ||
		     wood_policy_.at(site.bo->id) == WoodPolicy::kDismantleRangers) &&
		    !site.site->is_stopped()) {

			game().send_player_start_stop_building(*site.site);
			return false;
		}

		uint8_t trees_nearby = 0;
		for (const auto& supported_building : site.bo->supported_producers) {
			trees_nearby += find_immovables_nearby(supported_building.second->collected_attributes(),
			                                       map.get_fcoords(site.site->get_position()),
			                                       site.bo->desc->workarea_info_);
		}

		// stop ranger if enough trees around regardless of policy
		if (trees_nearby > 25) {
			if (!site.site->is_stopped()) {
				game().send_player_start_stop_building(*site.site);
			}
			// if not enough trees nearby, we can start them if required
		} else if ((wood_policy_.at(site.bo->id) == WoodPolicy::kAllowRangers) &&
		           site.site->is_stopped()) {
			game().send_player_start_stop_building(*site.site);
		}
	}

	return false;
}

/**
 * checks the first mine in list, takes care if it runs out of
 * resources and finally reenqueues it at the end of the list.
 *
 * \returns true, if something was changed.
 */
bool DefaultAI::check_mines_(const Time& gametime) {
	if (mines_.empty()) {
		return false;
	}

	// Reorder and set new values; - due to returns within the function
	mines_.push_back(mines_.front());
	mines_.pop_front();

	// Get link to productionsite that should be checked
	ProductionSiteObserver& site = mines_.front();

	// TODO(Nordfriese): Someone should update the code since the big economy splitting for the
	// ferries
	const bool connected_to_wh = !site.site->get_economy(Widelands::wwWORKER)->warehouses().empty();

	// First we dismantle mines that are marked as such, generally we wait till all wares all gone
	if (site.dismantle_pending_since.is_valid()) {
		assert(site.dismantle_pending_since <= gametime);
		if (set_inputs_to_zero(site) ||
		    site.dismantle_pending_since + Duration(5 * 60 * 1000) < gametime) {
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site, true);
			} else {
				game().send_player_bulldoze(*site.site);
			}

			return true;
		} else if (site.dismantle_pending_since + Duration(3 * 60 * 1000) < gametime) {
			stop_site(site);
			return false;
		} else {
			return false;
		}
	} else if (site.site->can_start_working()) {
		set_inputs_to_max(site);
	} else {
		set_inputs_to_zero(site);
	}

	// First we check if we must release an experienced worker
	for (uint8_t i = 0; i < site.site->descr().nr_working_positions(); i++) {
		const Widelands::Worker* cw = site.site->working_positions()->at(i).worker.get(game());
		if (cw) {
			Widelands::DescriptionIndex current_worker = cw->descr().worker_index();
			if (current_worker != site.bo->positions.at(i) &&
			    calculate_stocklevel(current_worker, WareWorker::kWorker) < 1) {
				game().send_player_evict_worker(
				   *site.site->working_positions()->at(i).worker.get(game()));
				return true;
			}
		}
	}

	// Single _critical is a critical mine if it is the only one of its type, so it needs special
	// treatment
	bool single_critical = false;
	if ((site.bo->is(BuildingAttribute::kBuildingMatProducer) ||
	     site.bo->mines == iron_resource_id) &&
	    mines_per_type[site.bo->mines].finished == 1) {
		single_critical = true;
	}

	// first get rid of mines that have been  missing workers for some time (10 minutes),
	// released worker (if any) can be useful elsewhere !
	if (!single_critical && site.built_time + Duration(10 * 60 * 1000) < gametime &&
	    !site.site->can_start_working() && mines_per_type[site.bo->mines].total_count() > 2) {
		initiate_dismantling(site, gametime);
		return false;
	}

	// to avoid problems with uint underflow, we discourage considerations below
	if (gametime < Time(10 * 60 * 1000)) {
		return false;
	}

	// After 15 minutes in existence we check whether a miner is needed for a critical unoccupied
	// mine elsewhere
	if (site.built_time + Duration(15 * 60 * 1000) < gametime) {
		if (!mines_per_type[site.bo->mines].is_critical && critical_mine_unoccupied(gametime)) {
			for (uint8_t i = 0; i < site.site->descr().nr_working_positions(); i++) {
				const Widelands::Worker* cw = site.site->working_positions()->at(i).worker.get(game());
				if (cw) {
					game().send_player_evict_worker(
					   *site.site->working_positions()->at(i).worker.get(game()));
				}
			}
			return true;
		}
	}

	// if mine is working, doing nothing
	if (site.no_resources_since.is_invalid() ||
	    gametime < site.no_resources_since + Duration(5 * 60 * 1000)) {
		return false;
	}

	// Out of resources, first check whether a mine is not needed for critical mine
	if (!mines_per_type[site.bo->mines].is_critical && critical_mine_unoccupied(gametime)) {
		initiate_dismantling(site, gametime);
		return true;
	}

	// Check whether building is enhanceable. If yes consider an upgrade.
	const Widelands::DescriptionIndex enhancement = site.site->descr().enhancement();
	bool has_upgrade = false;
	if (enhancement != Widelands::INVALID_INDEX) {
		if (player_->is_building_type_allowed(enhancement)) {
			has_upgrade = true;
		}
	}

	// every type of mine has minimal number of mines that are to be preserved
	// (we will not dismantle even if there are no mineable resources left for this level of mine
	// and output is not needed)
	bool forcing_upgrade = false;
	const uint16_t minimal_mines_count =
	   (site.bo->is(BuildingAttribute::kBuildingMatProducer)) ? 2 : 1;
	if (has_upgrade && mines_per_type[site.bo->mines].total_count() <= minimal_mines_count) {
		forcing_upgrade = true;
	}

	// dismantling a mine
	if (!has_upgrade) {  // if no upgrade, now
		initiate_dismantling(site, gametime);
		return true;
		// if having an upgrade, after half hour
	} else if (site.no_resources_since + Duration(30 * 60 * 1000) < gametime && !forcing_upgrade) {
		initiate_dismantling(site, gametime);
		return true;
	}

	// if we are here, a mine is upgradeable

	// if we don't need the output, and we have other buildings of the same type, the function
	// returns
	// and building will be dismantled later.
	check_building_necessity(*site.bo, PerfEvaluation::kForDismantle, gametime);
	if (site.bo->max_needed_preciousness == 0 && !forcing_upgrade) {
		return false;
	}

	// again similarly, no upgrading if not connected, other parts of AI will dismantle it,
	// or connect to a warehouse
	if (!connected_to_wh) {
		return false;
	}

	// don't upgrade now if other mines of the same type are right now in construction
	if (mines_per_type[site.bo->mines].in_construction > 0) {
		return false;
	}

	bool changed = false;

	// first exclude possibility there are enhancements in construction or unoccupied_count
	const Widelands::BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
	BuildingObserver& en_bo = get_building_observer(bld.name().c_str());

	// Make sure we do not exceed limit given by AI mode
	if (en_bo.aimode_limit_status() == AiModeBuildings::kAnotherAllowed) {

		// if it is too soon for enhancement
		if (gametime - en_bo.construction_decision_time >= kBuildingMinInterval) {
			// now verify that there are enough workers
			if (site.site->has_workers(enhancement, game())) {  // enhancing
				game().send_player_enhance_building(*site.site, enhancement, true);
				if (site.bo->max_needed_preciousness == 0) {
					assert(mines_per_type[site.bo->mines].total_count() <= minimal_mines_count);
				}
				if (mines_per_type[site.bo->mines].total_count() > minimal_mines_count) {
					assert(site.bo->max_needed_preciousness > 0);
				}
				en_bo.construction_decision_time = gametime;
				changed = true;
			}
		}
	}

	return changed;
}

BuildingNecessity DefaultAI::check_warehouse_necessity(BuildingObserver& bo, const Time& gametime) {
	bo.primary_priority = 0;

	// First there are situation when we cannot built the warehouse/port
	// a) This is port and map is not seafaring one
	if (bo.is(BuildingAttribute::kPort) && !map_allows_seafaring_) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}

	// b) the site is prohibited
	if (bo.prohibited_till > gametime) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}

	// If this is a port and is the first bo be built
	const bool first_port_allowed = (bo.is(BuildingAttribute::kPort) && bo.total_count() == 0);

	// c) there are warehouses in construction (first port is exemption)
	if (numof_warehouses_in_const_ > 0 && !first_port_allowed) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}

	// d) there is ai limit for this bulding
	if (bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}

	// e) basic economy not established, but first port is an exemption
	if (!basic_economy_established && !first_port_allowed) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}

	// Number of needed warehouses decides if new one is needed and also
	// converts to the score
	int32_t needed_count = 0;
	if (first_port_allowed) {
		needed_count += numof_warehouses_ + numof_warehouses_in_const_ + 1;
	} else {
		needed_count += static_cast<int32_t>(productionsites.size() + mines_.size()) /
		                   (40 + management_data.get_military_number_at(21) / 10) +
		                1;
	}

	assert(needed_count >= 0 &&
	       needed_count <= (static_cast<uint16_t>(productionsites.size() + mines_.size()) / 10) + 2);

	if (player_statistics.any_enemy_seen_lately(gametime) +
	       (productionsites.size() + mines_.size()) >
	    10) {
		++needed_count;
	}

	// Port should always have higher score than a warehouse
	if (bo.is(BuildingAttribute::kPort)) {
		++needed_count;
	}

	// suppres a warehouse if not enough spots
	if (spots_ < kSpotsEnough && !bo.is(BuildingAttribute::kPort)) {
		--needed_count;
	}

	if (needed_count <= numof_warehouses_in_const_ + numof_warehouses_) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}

	// So now we know the warehouse here is needed.
	bo.primary_priority = 1 + (needed_count - numof_warehouses_) *
	                             std::abs(management_data.get_military_number_at(22) * 20);
	++bo.new_building_overdue;
	bo.primary_priority +=
	   bo.new_building_overdue * std::abs(management_data.get_military_number_at(16));
	if (bo.is(BuildingAttribute::kPort) && spots_ < kSpotsTooLittle) {
		bo.primary_priority += std::abs(management_data.get_military_number_at(152)) * 10;
	}
	return BuildingNecessity::kAllowed;
}

// this receives an building observer and have to decide if new/one of
// current buildings of this type is needed
// This is core of construct_building() function
// This is run once when construct_building() is run, or when considering
// dismantle
BuildingNecessity DefaultAI::check_building_necessity(BuildingObserver& bo,
                                                      const PerfEvaluation purpose,
                                                      const Time& gametime) {
	bo.primary_priority = 0;

	static BasicEconomyBuildingStatus site_needed_for_economy = BasicEconomyBuildingStatus::kNone;
	site_needed_for_economy = BasicEconomyBuildingStatus::kNone;
	if (gametime > Time(2 * 60 * 1000) && gametime < Time(120 * 60 * 1000) &&
	    !basic_economy_established) {
		if (persistent_data->remaining_basic_buildings.count(bo.id) &&
		    bo.cnt_under_construction == 0) {
			assert(persistent_data->remaining_basic_buildings.count(bo.id) > 0);
			if (spots_ < kSpotsTooLittle && bo.type != BuildingObserver::Type::kMine) {
				site_needed_for_economy = BasicEconomyBuildingStatus::kNeutral;
			} else {
				site_needed_for_economy = BasicEconomyBuildingStatus::kEncouraged;
			}

		} else {
			site_needed_for_economy = BasicEconomyBuildingStatus::kDiscouraged;
		}
	}

	// Very first we finds if AI is allowed to build such building due to its mode
	if (purpose == PerfEvaluation::kForConstruction &&
	    bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
		return BuildingNecessity::kForbidden;
	}

	// Perhaps buildings are not allowed because the map is no seafaring
	if (purpose == PerfEvaluation::kForConstruction && !map_allows_seafaring_ &&
	    bo.is(BuildingAttribute::kNeedsSeafaring)) {
		return BuildingNecessity::kForbidden;
	}

	// First we deal with training sites, they are separate category
	if (bo.type == BuildingObserver::Type::kTrainingsite) {

		if (!basic_economy_established && management_data.f_neuron_pool[17].get_position(1)) {
			return BuildingNecessity::kNotNeeded;
		} else if (bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
			return BuildingNecessity::kNotNeeded;
		} else if (ts_without_trainers_ > 0 || bo.cnt_under_construction > 0 ||
		           ts_in_const_count_ > 1) {
			return BuildingNecessity::kNotNeeded;
		} else if (bo.prohibited_till > gametime) {
			return BuildingNecessity::kNotNeeded;
		} else if (ts_without_trainers_ > 1) {
			return BuildingNecessity::kNotNeeded;
		} else if (bo.total_count() > 0) {
			if (soldier_trained_log.count(gametime, bo.id) / bo.total_count() < 5) {
				return BuildingNecessity::kNotNeeded;
			}
		}

		// It seems we might need it after all
		bo.primary_priority = -30;
		if (bo.build_material_shortage) {
			bo.primary_priority -= std::abs(management_data.get_military_number_at(72));
		}

		if (bo.forced_after > gametime && bo.total_count() == 0) {
			bo.primary_priority += 50 + std::abs(management_data.get_military_number_at(112) / 5);
		}

		// If we are close to enemy (was seen in last 15 minutes)
		if (player_statistics.any_enemy_seen_lately(gametime)) {
			bo.primary_priority += std::abs(management_data.get_military_number_at(57) / 2);
		}

		// Do we own some minefields for each critical mine
		if (!mine_fields_stat.has_critical_ore_fields()) {
			bo.primary_priority -= std::abs(management_data.get_military_number_at(156));
		}

		// We build one trainig site per X military sites
		// with some variations, of course
		int32_t target = 1 + static_cast<int32_t>(militarysites.size() + productionsites.size()) /
		                        (std::abs(management_data.get_military_number_at(113) / 2) + 1);
		assert(target > 0 && target < 500);

		uint16_t current_proportion = 0;
		if (ts_finished_count_ + ts_in_const_count_ > 0) {
			current_proportion = bo.total_count() * 100 / (ts_finished_count_ + ts_in_const_count_);
		}

		bo.primary_priority += (target - ts_finished_count_ - ts_in_const_count_) *
		                       std::abs(management_data.get_military_number_at(114) * 2);
		bo.primary_priority += (static_cast<int32_t>(militarysites.size() + productionsites.size()) -
		                        target * std::abs(management_data.get_military_number_at(78) / 4)) *
		                       3;

		// Special bonus for very first site of type
		if (bo.total_count() == 0) {
			bo.primary_priority += std::abs(management_data.get_military_number_at(56)) +
			                       bo.max_trainingsites_proportion - current_proportion;
		} else if (bo.max_trainingsites_proportion < current_proportion) {
			bo.primary_priority -= std::abs(management_data.get_military_number_at(128) * 3);
		}

		if (bo.primary_priority > 0) {
			return BuildingNecessity::kNeeded;
		} else {
			return BuildingNecessity::kNotNeeded;
		}
	}

	if (bo.is(BuildingAttribute::kRecruitment)) {
		if (bo.total_count() > 1) {
			return BuildingNecessity::kForbidden;
		}
		if (critical_mine_unoccupied(gametime)) {
			return BuildingNecessity::kForbidden;
		}
		if (!basic_economy_established) {
			return BuildingNecessity::kForbidden;
		}
		const uint16_t min_roads_count =
		   40 + std::abs(management_data.get_military_number_at(33)) / 2;
		if (static_cast<int>(roads.size()) < min_roads_count * (1 + bo.total_count())) {
			return BuildingNecessity::kForbidden;
		}
		bo.primary_priority += (roads.size() - min_roads_count * (1 + bo.total_count())) *
		                       (2 + std::abs(management_data.get_military_number_at(143)) / 5);
		return BuildingNecessity::kNeeded;
	}

	// Let deal with productionsites now
	// First we iterate over outputs of building, count warehoused stock
	// and deciding if we have enough on stock (in warehouses)

	// Calulate preciousness
	bo.max_preciousness = bo.initial_preciousness;
	bo.max_needed_preciousness = bo.initial_preciousness;
	for (uint32_t m = 0; m < bo.ware_outputs.size(); ++m) {
		Widelands::DescriptionIndex wt(static_cast<size_t>(bo.ware_outputs.at(m)));

		uint16_t target = tribe_->get_ware_descr(wt)->default_target_quantity(tribe_->name());
		if (target == Widelands::kInvalidWare) {
			target = kTargetQuantCap;
		}
		target /= 3;

		// at least  1
		target = std::max<uint16_t>(target, 1);

		// it seems there are wares with 0 preciousness (no entry in init files?), but we need
		// positive value here.
		// TODO(GunChleoc): Since we require in TribeDescr::load_wares that this is set for all wares
		// used
		// by a tribe, something seems to be wrong here. It should always be > 0.
		const uint16_t preciousness =
		   std::max<uint16_t>(wares.at(bo.ware_outputs.at(m)).preciousness, 1);

		if (calculate_stocklevel(wt) < target ||
		    site_needed_for_economy == BasicEconomyBuildingStatus::kEncouraged) {
			if (bo.max_needed_preciousness < preciousness) {
				bo.max_needed_preciousness = preciousness;
			}
			if (site_needed_for_economy == BasicEconomyBuildingStatus::kEncouraged) {
				bo.max_needed_preciousness +=
				   std::abs(management_data.get_military_number_at(144)) / 10;
			}
		}

		if (bo.max_preciousness < preciousness) {
			bo.max_preciousness = preciousness;
		}
	}

	// Do we have enough input materials on stock?
	bool inputs_on_stock = true;
	if (bo.type == BuildingObserver::Type::kProductionsite ||
	    bo.type == BuildingObserver::Type::kMine) {
		for (auto input : bo.inputs) {
			if (calculate_stocklevel(input) < 2) {
				inputs_on_stock = false;
				break;
			}
		}
	}

	// Do we have enough workers available in warehouses?
	bool workers_on_stock = true;
	if (bo.type == BuildingObserver::Type::kProductionsite ||
	    bo.type == BuildingObserver::Type::kMine) {
		for (auto worker : bo.positions) {
			if (calculate_stocklevel(worker, WareWorker::kWorker) < 1) {
				workers_on_stock = false;
				break;
			}
		}
	}

	// Do we have suppliers productionsites?
	const bool suppliers_exist = check_supply(bo);

	if (!bo.ware_outputs.empty()) {
		assert(bo.max_preciousness > 0);
	}

	if (bo.is(BuildingAttribute::kShipyard)) {
		assert(bo.max_preciousness == 0);
	}

	// This flag is to be used when buildig is forced. AI will not build another building when
	// a substitution exists. F.e. mines or pairs like tavern-inn
	// To skip unnecessary calculation, we calculate this only if we have 0 count of the buildings
	bool has_substitution_building = false;
	if (bo.total_count() == 0 && bo.is(BuildingAttribute::kUpgradeSubstitutes) &&
	    bo.type == BuildingObserver::Type::kProductionsite) {
		const Widelands::DescriptionIndex enhancement = bo.desc->enhancement();
		BuildingObserver& en_bo =
		   get_building_observer(tribe_->get_building_descr(enhancement)->name().c_str());
		if (en_bo.total_count() > 0) {
			has_substitution_building = true;
		}
	}
	if (bo.total_count() == 0 && bo.type == BuildingObserver::Type::kMine) {
		if (mines_per_type[bo.mines].total_count() > 0) {
			has_substitution_building = true;
		}
	}

	// Some buildings are upgraded to ones that does not produce current output, so we need to have
	// two of current buildings to have at least one left after one of them is upgraded
	// Logic is: after 30th minute we need second building if there is no enhanced building yet,
	// and after 90th minute we want second building unconditionally
	bool needs_second_for_upgrade = false;
	if (gametime > Time(30 * 60 * 1000) && bo.cnt_built == 1 && bo.cnt_under_construction == 0 &&
	    bo.is(BuildingAttribute::kUpgradeExtends) &&
	    !bo.is(BuildingAttribute::kUpgradeSubstitutes) &&
	    bo.type == BuildingObserver::Type::kProductionsite) {
		const Widelands::DescriptionIndex enhancement = bo.desc->enhancement();
		BuildingObserver& en_bo =
		   get_building_observer(tribe_->get_building_descr(enhancement)->name().c_str());
		if ((gametime > Time(30 * 60 * 1000) && en_bo.total_count() == 0) ||
		    gametime > Time(90 * 60 * 1000)) {
			// We fake this
			bo.max_needed_preciousness = bo.max_preciousness * 2;
			needs_second_for_upgrade = true;
		}
	}

	// And finally the 'core' of this function
	// First deal with construction of new sites
	if (purpose == PerfEvaluation::kForConstruction) {
		// Inform if we are above ai type limit.
		if (bo.total_count() > bo.cnt_limit_by_aimode) {
			verb_log_warn_time(gametime,
			                   "AI check_building_necessity: Too many %s: %d, ai limit: %d\n", bo.name,
			                   bo.total_count(), bo.cnt_limit_by_aimode);
		}

		if (bo.forced_after < gametime && bo.total_count() == 0 && !has_substitution_building) {
			bo.max_needed_preciousness = bo.max_preciousness;
			return BuildingNecessity::kForced;
		} else if (bo.prohibited_till > gametime) {
			return BuildingNecessity::kForbidden;
		} else if (bo.is(BuildingAttribute::kHunter) || bo.is(BuildingAttribute::kFisher) ||
		           bo.is(BuildingAttribute::kWell)) {

			bo.cnt_target = 1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 25;

			if (bo.cnt_under_construction + bo.unoccupied_count > 0) {
				return BuildingNecessity::kForbidden;
			}

			static int16_t inputs[kFNeuronBitSize] = {0};
			// Resetting values as the variable is static
			std::fill(std::begin(inputs), std::end(inputs), 0);
			inputs[0] = (bo.max_needed_preciousness == 0) ? -1 : 0;
			inputs[1] = (bo.max_needed_preciousness > 0) ? 2 : 0;
			inputs[2] = (bo.max_needed_preciousness == 0) ? -3 : 0;
			inputs[3] = (bo.total_count() > 0) ? -1 : 0;
			inputs[4] = (bo.total_count() > 1) ? -1 : 0;
			inputs[5] = (bo.total_count() > 0) ? -1 : 0;
			inputs[6] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? +1 : 0;
			inputs[7] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? +2 : 0;
			inputs[8] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? +3 : 0;
			inputs[9] = (expansion_type.get_expansion_type() == ExpansionMode::kBoth) ? +1 : 0;
			inputs[10] = (expansion_type.get_expansion_type() == ExpansionMode::kBoth) ? +1 : 0;
			inputs[11] = (bo.last_building_built.is_valid() &&
			              bo.last_building_built + Duration(5 * 60 * 100) < gametime) ?
                         +1 :
                         0;
			inputs[12] = (bo.last_building_built.is_valid() &&
			              bo.last_building_built + Duration(10 * 60 * 100) < gametime) ?
                         +1 :
                         0;
			inputs[13] = (bo.last_building_built.is_valid() &&
			              bo.last_building_built + Duration(20 * 60 * 100) < gametime) ?
                         +1 :
                         0;
			inputs[14] = (bo.total_count() >= bo.cnt_target) ? -1 : 0;
			inputs[15] = (bo.total_count() >= bo.cnt_target) ? -2 : 0;
			inputs[16] = (bo.total_count() < bo.cnt_target) ? -1 : 0;
			inputs[17] = (bo.total_count() < bo.cnt_target) ? -2 : 0;
			inputs[18] = +1;
			inputs[19] = +2;
			inputs[20] = -1;
			inputs[21] = -2;
			inputs[22] = -3;
			inputs[23] = -4;
			inputs[24] = -5;
			inputs[25] = (basic_economy_established) ? 1 : -1;
			inputs[26] = (basic_economy_established) ? 1 : -1;
			inputs[27] = (bo.total_count() > 0 && spots_ < kSpotsEnough) ? -2 : 0;
			inputs[28] = (bo.total_count() > 0 && spots_ < kSpotsTooLittle) ? -2 : 0;
			inputs[29] = (spots_ < kSpotsEnough) ? -1 : 0;
			inputs[30] = (spots_ < kSpotsTooLittle) ? -1 : 0;
			int16_t tmp_score = 0;
			for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
				if (management_data.f_neuron_pool[53].get_position(i)) {
					tmp_score += inputs[i];
				}
			}
			if (site_needed_for_economy == BasicEconomyBuildingStatus::kEncouraged) {
				tmp_score += 4;
			}
			if (site_needed_for_economy == BasicEconomyBuildingStatus::kDiscouraged) {
				tmp_score -= 2;
			}

			if (tmp_score < 0) {
				return BuildingNecessity::kForbidden;
			} else {
				if (bo.max_needed_preciousness <= 0) {
					bo.max_needed_preciousness = 1;
				}
				bo.primary_priority =
				   1 + tmp_score * std::abs(management_data.get_military_number_at(137) / 2);
				return BuildingNecessity::kNeeded;
			}
		} else if (bo.is(BuildingAttribute::kLumberjack)) {
			if (bo.total_count() > 1 && (bo.cnt_under_construction + bo.unoccupied_count > 0)) {
				return BuildingNecessity::kForbidden;
			}
			bo.cnt_target = 2;
			// adjusting/decreasing based on cnt_limit_by_aimode
			bo.cnt_target = std::min(bo.cnt_target, bo.cnt_limit_by_aimode);

			// for case the wood is not needed yet, to avoid inconsistency later on
			bo.max_needed_preciousness = bo.max_preciousness;

			bo.primary_priority = 0;

			if (bo.total_count() < bo.cnt_target) {
				bo.primary_priority += 10 * std::abs(management_data.get_military_number_at(34));
			}
			if (get_stocklevel(bo, gametime) < 10) {
				bo.primary_priority += std::abs(management_data.get_military_number_at(118));
			}
			if (bo.total_count() < bo.cnt_target) {
				return BuildingNecessity::kNeeded;
			} else {
				return BuildingNecessity::kAllowed;
			}
		} else if (bo.is(BuildingAttribute::kRanger)) {

			// making sure we have one completed supported lumberjack
			uint16_t supported_lumberjack_built = 0;
			for (const auto& supported_building : bo.supported_producers) {
				supported_lumberjack_built += get_building_observer(supported_building.first).cnt_built;
			}
			if (supported_lumberjack_built < 1) {
				return BuildingNecessity::kForbidden;
			}

			// genetic algorithm to decide whether new rangers are needed
			static int16_t tmp_target = 2;
			tmp_target = 2;
			static int16_t inputs[2 * kFNeuronBitSize] = {0};
			// Resetting values as the variable is static
			std::fill(std::begin(inputs), std::end(inputs), 0);

			inputs[0] = (persistent_data->trees_around_cutters < 10) * 2;
			inputs[1] = (persistent_data->trees_around_cutters < 20) * 2;
			inputs[2] = (persistent_data->trees_around_cutters < 30) * 2;
			inputs[3] = (persistent_data->trees_around_cutters < 40) * 2;
			inputs[4] = (persistent_data->trees_around_cutters < 50) * 1;
			inputs[5] = (persistent_data->trees_around_cutters < 60) * 1;
			inputs[6] = (persistent_data->trees_around_cutters < 10) * 1;
			inputs[7] = (persistent_data->trees_around_cutters < 20) * 1;
			inputs[8] = (persistent_data->trees_around_cutters < 100) * 1;
			inputs[9] = (persistent_data->trees_around_cutters < 200) * 1;
			inputs[10] = (persistent_data->trees_around_cutters < 300) * 1;
			inputs[11] = (persistent_data->trees_around_cutters < 400) * 1;
			inputs[12] = (wood_policy_.at(bo.id) == WoodPolicy::kAllowRangers) ? 1 : 0;
			inputs[13] = (wood_policy_.at(bo.id) == WoodPolicy::kAllowRangers) ? 1 : 0;
			inputs[14] = (get_stocklevel(bo, gametime) < 10) * 1;
			inputs[15] = (get_stocklevel(bo, gametime) < 10) * 1;
			inputs[16] = (get_stocklevel(bo, gametime) < 2) * 1;
			if (gametime > Time(15 * 60 * 1000)) {
				inputs[17] = (get_stocklevel(bo, gametime) > 30) * -1;
				inputs[18] = (get_stocklevel(bo, gametime) > 20) * -1;
				inputs[19] = (get_stocklevel(bo, gametime) > 10) * -1;
			} else {
				inputs[20] = 1;
				inputs[21] = 1;
			}
			inputs[22] = (basic_economy_established) ? -1 : 1;
			inputs[23] = (msites_in_constr() > 0) ? 1 : -2;
			inputs[24] = (msites_in_constr() > 1) ? 1 : -2;
			inputs[25] = (wood_policy_.at(bo.id) != WoodPolicy::kAllowRangers) ? 1 : 0;
			if (gametime > Time(90 * 60 * 1000)) {
				inputs[26] = (wood_policy_.at(bo.id) == WoodPolicy::kAllowRangers) ? 1 : 0;
				inputs[27] = (persistent_data->trees_around_cutters < 20) * 1;
			}
			if (gametime > Time(45 * 60 * 1000)) {
				inputs[28] = (wood_policy_.at(bo.id) == WoodPolicy::kAllowRangers) ? 1 : 0;
				inputs[29] = (persistent_data->trees_around_cutters < 20) * 1;
				inputs[30] = (get_stocklevel(bo, gametime) > 30) * -1;
			}
			inputs[31] = (persistent_data->trees_around_cutters < 100) * 2;
			inputs[32] = (persistent_data->trees_around_cutters < 200) * 2;
			inputs[33] = (mines_per_type[iron_resource_id].total_count() <= 1) * -1;
			inputs[34] = (mines_per_type[iron_resource_id].total_count() <= 1) * -1;
			inputs[35] = (mines_per_type[iron_resource_id].total_count() == 0) * -1;
			inputs[36] = (mines_per_type[iron_resource_id].total_count() == 0) * -1;
			inputs[37] = -1;
			inputs[38] = -1;
			inputs[39] = -1;
			if (productionsites.size() / 3 > static_cast<uint32_t>(bo.total_count()) &&
			    get_stocklevel(bo, gametime) < 20) {
				inputs[40] = (persistent_data->trees_around_cutters < 40) * 1;
				inputs[41] = (persistent_data->trees_around_cutters < 60) * 1;
				inputs[42] = (persistent_data->trees_around_cutters < 80) * 1;
			}
			if (productionsites.size() / 4 > static_cast<uint32_t>(bo.total_count()) &&
			    get_stocklevel(bo, gametime) < 20) {
				inputs[43] = (persistent_data->trees_around_cutters < 40) * 2;
				inputs[44] = (persistent_data->trees_around_cutters < 60) * 2;
				inputs[45] = (persistent_data->trees_around_cutters < 80) * 2;
			}

			if (productionsites.size() / 2 > static_cast<uint32_t>(bo.total_count()) &&
			    get_stocklevel(bo, gametime) < 10) {
				inputs[46] = (persistent_data->trees_around_cutters < 20) * 1;
				inputs[47] = (persistent_data->trees_around_cutters < 40) * 1;
				inputs[48] = (persistent_data->trees_around_cutters < 60) * 1;
				inputs[49] = (persistent_data->trees_around_cutters < 80) * 1;
			}
			inputs[50] = (bo.last_building_built.is_valid() &&
			              bo.last_building_built + Duration(1 * 60 * 100) > gametime) *
			             -2;
			inputs[51] = (bo.last_building_built.is_valid() &&
			              bo.last_building_built + Duration(2 * 60 * 100) > gametime) *
			             -2;
			inputs[52] = (bo.last_building_built.is_valid() &&
			              bo.last_building_built + Duration(4 * 60 * 100) > gametime) *
			             -2;
			inputs[53] = (bo.last_building_built.is_valid() &&
			              bo.last_building_built + Duration(6 * 60 * 100) > gametime) *
			             -2;
			inputs[54] = (Time(5 * 60 * 1000) > gametime) * -2;
			inputs[55] = (Time(6 * 60 * 1000) > gametime) * -2;
			inputs[56] = (Time(8 * 60 * 1000) > gametime) * -2;
			inputs[57] = (Time(10 * 60 * 1000) > gametime) * -2;
			inputs[58] = (spots_ < kSpotsEnough) ? -2 : 0;
			inputs[59] = (spots_ < kSpotsTooLittle) ? -2 : 0;
			inputs[60] = (spots_ < kSpotsTooLittle) ? -2 : 0;
			inputs[61] = (spots_ < kSpotsTooLittle) ? -2 : 0;
			inputs[62] = (basic_economy_established) ? 0 : -2;
			inputs[63] = (spots_ < kSpotsTooLittle) ? 0 : -2;

			for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
				if (management_data.f_neuron_pool[14].get_position(i)) {
					assert(inputs[i] >= -2 && inputs[i] <= 2);
					tmp_target += inputs[i];
				}
				if (management_data.f_neuron_pool[15].get_position(i)) {
					tmp_target += inputs[kFNeuronBitSize + i];
					assert(inputs[kFNeuronBitSize + i] >= -2 && inputs[kFNeuronBitSize + i] <= 2);
				}
			}

			if (site_needed_for_economy == BasicEconomyBuildingStatus::kDiscouraged) {
				tmp_target -= std::abs(management_data.get_military_number_at(145) / 10);
			}

			tmp_target = std::max<int16_t>(tmp_target, 2);

			bo.cnt_target = tmp_target;

			// adjusting/decreasing based on cnt_limit_by_aimode
			bo.cnt_target = std::min(bo.cnt_target, bo.cnt_limit_by_aimode);

			assert(bo.cnt_target > 1 && bo.cnt_target < 1000);

			// allow them always if basic economy not established and building is a basic one
			if (bo.total_count() < static_cast<int32_t>(bo.basic_amount)) {
				return BuildingNecessity::kNeeded;
			}

			if (wood_policy_.at(bo.id) != WoodPolicy::kAllowRangers) {
				return BuildingNecessity::kForbidden;
			}

			if (bo.total_count() > bo.cnt_target) {
				return BuildingNecessity::kForbidden;
			}

			const bool parallel_construction = (bo.total_count() + 2 < bo.cnt_target);
			if (parallel_construction && (bo.cnt_under_construction + bo.unoccupied_count <= 1)) {
				return BuildingNecessity::kNeeded;
			} else if (bo.cnt_under_construction + bo.unoccupied_count == 0) {
				return BuildingNecessity::kNeeded;
			}
			return BuildingNecessity::kForbidden;
		} else if (bo.is(BuildingAttribute::kNeedsRocks) &&
		           bo.cnt_under_construction + bo.unoccupied_count == 0) {
			bo.max_needed_preciousness = bo.max_preciousness;  // even when rocks are not needed
			return BuildingNecessity::kAllowed;
		} else if (!bo.supported_producers.empty() &&
		           !bo.is(BuildingAttribute::kSupportingProducer)) {
			// Pure supporting sites only

			if (bo.cnt_under_construction + bo.unoccupied_count - bo.unconnected_count > 0) {
				return BuildingNecessity::kForbidden;
			}

			// Rangers have been processed above
			assert(!bo.is(BuildingAttribute::kRanger));

			bo.primary_priority = 0;

			if (!basic_economy_established) {
				bo.cnt_target = bo.basic_amount;
			} else {
				bo.cnt_target = 1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 30;
			}

			if (bo.total_count() > bo.cnt_target + 1) {
				return BuildingNecessity::kForbidden;
			}

			// We allow another helper site if:
			// a) we are under target
			// b) if there is shortage of supported ware
			if (bo.total_count() < bo.cnt_target ||
			    (get_stocklevel(bo, gametime) == 0 &&
			     (bo.last_building_built.is_invalid() ||
			      bo.last_building_built + Duration(10 * 60 * 100) < gametime))) {

				if (persistent_data->remaining_basic_buildings.count(bo.id)) {
					bo.primary_priority += std::abs(management_data.get_military_number_at(60) * 10);
				}

				if (bo.total_count() < bo.cnt_target) {
					if (basic_economy_established) {
						bo.primary_priority += std::abs(management_data.get_military_number_at(51) * 6);
					} else if (persistent_data->remaining_basic_buildings.count(bo.id)) {
						bo.primary_priority += std::abs(management_data.get_military_number_at(146) * 6);
					} else {
						bo.primary_priority +=
						   -200 + std::abs(management_data.get_military_number_at(147) * 8);
					}
				}

				return BuildingNecessity::kAllowed;
			}
			return BuildingNecessity::kForbidden;

		} else if (bo.is(BuildingAttribute::kBarracks)) {
			if (site_needed_for_economy == BasicEconomyBuildingStatus::kDiscouraged) {
				return BuildingNecessity::kForbidden;
			}
			if (gametime > Time(30 * 60 * 1000) && bo.total_count() == 0) {

				int16_t tmp_score = 1;
				tmp_score += mines_per_type[iron_resource_id].total_count();
				tmp_score += (soldier_status_ == SoldiersStatus::kBadShortage) * 2;
				tmp_score += (soldier_status_ == SoldiersStatus::kShortage) * 2;
				tmp_score += (gametime.get() / 60 / 1000 - 20) / 4;
				bo.max_needed_preciousness =
				   1 + tmp_score * std::abs(management_data.get_military_number_at(134)) / 15;
				bo.max_preciousness = bo.max_needed_preciousness;
				return BuildingNecessity::kNeeded;
			} else {
				bo.max_needed_preciousness = 0;
				bo.max_preciousness = 0;
				return BuildingNecessity::kForbidden;
			}
		} else if (bo.type == BuildingObserver::Type::kMine) {
			bo.primary_priority = bo.max_needed_preciousness;
			if (mines_per_type[bo.mines].total_count() == 0 &&
			    site_needed_for_economy != BasicEconomyBuildingStatus::kDiscouraged) {
				// unless a mine is prohibited, we want to have at least one of the kind
				bo.max_needed_preciousness = bo.max_preciousness;
				return BuildingNecessity::kNeeded;
			} else if (mines_per_type[bo.mines].finished == mines_per_type[bo.mines].total_count() &&
			           bo.current_stats >
			              static_cast<uint32_t>(
			                 85 + std::abs(management_data.get_military_number_at(129)) / 10) &&
			           site_needed_for_economy != BasicEconomyBuildingStatus::kDiscouraged) {
				bo.max_needed_preciousness = bo.max_preciousness;
				return BuildingNecessity::kNeeded;
			}
			if (bo.max_needed_preciousness == 0) {
				return BuildingNecessity::kNotNeeded;
			}
			if (gametime - bo.construction_decision_time < kBuildingMinInterval) {
				return BuildingNecessity::kForbidden;
			}
			if (mines_per_type[bo.mines].in_construction > 0) {
				return BuildingNecessity::kForbidden;
			}
			if (mines_per_type[bo.mines].finished >= 1 && bo.current_stats < 50) {
				return BuildingNecessity::kForbidden;
			}

			if (bo.last_building_built.is_valid() &&
			    gametime < bo.last_building_built + Duration(3 * 60 * 1000)) {
				return BuildingNecessity::kForbidden;
			}

			static int16_t inputs[kFNeuronBitSize] = {0};
			// Resetting values as the variable is static
			std::fill(std::begin(inputs), std::end(inputs), 0);
			inputs[0] = (gametime < Time(15 * 60 * 1000)) ? -2 : 0;
			inputs[1] = (gametime < Time(30 * 60 * 1000)) ? -2 : 0;
			inputs[2] = (gametime < Time(45 * 60 * 1000)) ? -2 : 0;
			inputs[3] = (mines_per_type[bo.mines].total_count() == 1) ? 3 : 0;
			inputs[4] = (mines_per_type[bo.mines].total_count() == 1) ? 2 : 0;
			inputs[5] = (bo.mines == iron_resource_id) ? 2 : 1;
			inputs[6] = (bo.current_stats - 50) / 10;
			inputs[7] = (gametime < Time(15 * 60 * 1000)) ? -1 : 0;
			inputs[8] = (gametime < Time(30 * 60 * 1000)) ? -1 : 0;
			inputs[9] = (gametime < Time(45 * 60 * 1000)) ? -1 : 0;
			inputs[10] = (mines_per_type[bo.mines].total_count() == 1) ? 2 : 0;
			inputs[11] = (mines_per_type[bo.mines].total_count() == 1) ? 1 : 0;
			inputs[12] = (bo.mines == iron_resource_id) ? 2 : 0;
			inputs[13] = (bo.current_stats - 50) / 10;
			inputs[14] = (bo.current_stats - 50) / 10;
			inputs[15] = management_data.get_military_number_at(123) / 10;
			inputs[16] = 0;
			inputs[17] = (inputs_on_stock) ? 0 : -2;
			inputs[18] = (suppliers_exist) ? 0 : -3;
			inputs[19] = (inputs_on_stock) ? 0 : -4;
			inputs[20] = (mines_per_type[bo.mines].total_count() == 1) ? 3 : 0;
			inputs[21] = (mines_per_type[bo.mines].total_count() == 1) ? 2 : 0;
			inputs[22] = (bo.current_stats - 50) / 10;
			inputs[23] = (bo.current_stats - 50) / 20;
			inputs[24] = (suppliers_exist) ? 0 : -5;
			inputs[25] = (suppliers_exist) ? 0 : -2;
			inputs[26] = (workers_on_stock) ? 0 : -5;
			inputs[27] = (workers_on_stock) ? 0 : -2;
			inputs[28] = (bo.is(BuildingAttribute::kBuildingMatProducer)) ? 1 : 0;
			inputs[29] = (bo.is(BuildingAttribute::kBuildingMatProducer)) ? 3 : 0;
			inputs[30] = (mines_per_type[bo.mines].is_critical) ? 1 : -1;

			int16_t tmp_score = management_data.get_military_number_at(83) / 5;

			// Building productionsites above limit in Basic economy mode is strongly discouraged, but
			// still possible
			const int16_t basic_economy_score =
			   25 + std::abs(management_data.get_military_number_at(122) * 2);

			if (site_needed_for_economy == BasicEconomyBuildingStatus::kEncouraged) {
				tmp_score += basic_economy_score;
			}

			if (site_needed_for_economy == BasicEconomyBuildingStatus::kDiscouraged) {
				tmp_score -= basic_economy_score;
			}

			for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
				if (management_data.f_neuron_pool[36].get_position(i)) {
					tmp_score += inputs[i];
				}
			}
			if (tmp_score < 0) {
				return BuildingNecessity::kNeededPending;
			} else {
				bo.primary_priority +=
				   tmp_score * std::abs(management_data.get_military_number_at(127) / 5);
				return BuildingNecessity::kNeeded;
			}

		} else if (bo.max_needed_preciousness > 0) {

			// help variable to determine wood availability in the economy
			const int32_t stocked_wood_level = calculate_stocklevel(tribe_->safe_ware_index("log")) -
			                                   productionsites.size() * 2 - numof_psites_in_constr +
			                                   management_data.get_military_number_at(87) / 5;
			static int16_t inputs[4 * kFNeuronBitSize] = {0};
			// Resetting values as the variable is static
			std::fill(std::begin(inputs), std::end(inputs), 0);
			inputs[0] = (bo.total_count() <= 1) ?
                        std::abs(management_data.get_military_number_at(110)) / 10 :
                        0;
			inputs[1] = bo.total_count() * -3 / 2;
			inputs[2] =
			   (bo.total_count() == 0) ? std::abs(management_data.get_military_number_at(0)) / 10 : 0;
			inputs[3] = (gametime >= Time(25 * 60 * 1000) && bo.inputs.empty()) ?
                        management_data.get_military_number_at(1) / 10 :
                        0;
			inputs[4] = (bo.max_needed_preciousness >= 10) ?
                        std::abs(management_data.get_military_number_at(2)) / 10 :
                        0;
			inputs[5] =
			   (!bo.ware_outputs.empty() && bo.current_stats > 10 + 70 / bo.ware_outputs.size()) ?
               management_data.get_military_number_at(3) / 10 :
               0;
			inputs[6] = (needs_second_for_upgrade) ?
                        std::abs(management_data.get_military_number_at(4)) / 5 :
                        0;
			inputs[7] = (bo.cnt_under_construction + bo.unoccupied_count) * -1 *
			            std::abs(management_data.get_military_number_at(9)) / 5;
			inputs[8] =
			   (!bo.ware_outputs.empty() && bo.current_stats > 25 + 70 / bo.ware_outputs.size()) ?
               management_data.get_military_number_at(7) / 8 :
               0;
			inputs[9] = (bo.is(BuildingAttribute::kBuildingMatProducer)) ?
                        std::abs(management_data.get_military_number_at(10)) / 10 :
                        0;
			inputs[10] =
			   (bo.build_material_shortage) ? -management_data.get_military_number_at(39) / 10 : 0;
			inputs[11] =
			   stocked_wood_level > 25 ? std::abs(management_data.get_military_number_at(15)) / 10 : 0;
			inputs[12] = (gametime >= Time(15 * 60 * 1000)) ?
                         std::abs(management_data.get_military_number_at(94)) / 10 :
                         0;
			inputs[13] = management_data.get_military_number_at(8) / 10;
			inputs[14] = (persistent_data->trees_around_cutters < 20) ?
                         -1 * std::abs(management_data.get_military_number_at(95)) / 10 :
                         0;
			inputs[15] = (persistent_data->trees_around_cutters > 100) ?
                         std::abs(management_data.get_military_number_at(96)) / 10 :
                         0;
			inputs[16] = (player_statistics.any_enemy_seen_lately(gametime)) ?
                         management_data.get_military_number_at(97) / 10 :
                         0;
			inputs[17] =
			   (spots_ > kSpotsEnough) ? std::abs(management_data.get_military_number_at(74)) / 8 : 0;
			inputs[18] = management_data.get_military_number_at(98) / 10;
			inputs[19] = (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) ?
                         -1 * std::abs(management_data.get_military_number_at(40)) / 10 :
                         0;
			inputs[20] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ?
                         std::abs(management_data.get_military_number_at(50)) / 10 :
                         0;
			inputs[21] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy ||
			              expansion_type.get_expansion_type() == ExpansionMode::kBoth) ?
                         3 :
                         0;
			inputs[22] =
			   (bo.total_count() == 0 && bo.is(BuildingAttribute::kBuildingMatProducer)) ? 3 : 0;
			if (bo.cnt_built > 0 && !bo.ware_outputs.empty()) {
				inputs[22] += bo.current_stats / 10;
			}
			inputs[23] = (!player_statistics.strong_enough(player_number())) ? 5 : 0;
			inputs[24] = (bo.inputs.empty()) ? 6 : 0;
			inputs[25] =
			   (bo.total_count() == 0 && bo.is(BuildingAttribute::kBuildingMatProducer)) ? 4 : 0;
			inputs[26] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? 2 : 0;
			inputs[27] = stocked_wood_level > 25 ? 4 : 0;
			inputs[28] = (bo.max_needed_preciousness >= 10) ? 4 : 0;
			inputs[29] = (bo.inputs.empty() && bo.max_needed_preciousness >= 10) ? 3 : 0;
			inputs[30] = bo.max_needed_preciousness / 2;
			inputs[31] = ((bo.cnt_under_construction + bo.unoccupied_count) > 0) ? -5 : 0;
			inputs[32] = bo.max_needed_preciousness / 2;
			inputs[33] = -(bo.cnt_under_construction + bo.unoccupied_count) * 4;
			if (bo.cnt_built > 0 && !bo.ware_outputs.empty() && !bo.inputs.empty()) {
				inputs[34] +=
				   bo.current_stats / (std::abs(management_data.get_military_number_at(192)) + 1) * 10;
			}
			inputs[35] = (!bo.ware_outputs.empty() && !bo.inputs.empty() &&
			              bo.current_stats > 10 + 70 / bo.ware_outputs.size()) ?
                         2 :
                         0;
			inputs[36] = (!bo.ware_outputs.empty() && !bo.inputs.empty() &&
			              bo.cnt_under_construction + bo.unoccupied_count == 0) ?
                         bo.current_stats / 12 :
                         0;
			if (bo.cnt_built > 0 && !bo.inputs.empty() && !bo.ware_outputs.empty() &&
			    bo.current_stats < 20) {
				inputs[37] = -5;
			}
			inputs[38] = (bo.cnt_under_construction + bo.unoccupied_count > 0) ? -10 : 0;
			if (bo.cnt_built > 0 && !bo.ware_outputs.empty() && bo.current_stats < 15) {
				inputs[39] = -10;
			}
			inputs[40] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? 3 : 0;
			inputs[41] = (bo.build_material_shortage) ? -3 : 0;
			inputs[42] = (!player_statistics.strong_enough(player_number())) ? 2 : 0;
			inputs[43] = (bo.inputs.empty()) ? 3 : 0;
			inputs[44] = (bo.inputs.empty() && bo.max_needed_preciousness >= 10) ? 3 : 0;
			inputs[45] = bo.max_needed_preciousness / 2;
			inputs[46] =
			   (!bo.ware_outputs.empty() && bo.current_stats > 10 + 70 / bo.ware_outputs.size()) ? 4 :
                                                                                                0;
			inputs[47] = (!bo.ware_outputs.empty() && bo.current_stats > 85) ? 4 : 0;
			inputs[48] = (bo.max_needed_preciousness >= 10 &&
			              (bo.cnt_under_construction + bo.unoccupied_count) == 1) ?
                         5 :
                         0;
			inputs[49] = (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) ? -4 : 1;
			inputs[50] = (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) ? -1 : 1;
			inputs[51] = (gametime < Time(20 * 60 * 1000)) ? -4 : 0;
			inputs[52] = (bo.total_count() == 0) ? 4 : 0;
			inputs[53] = (bo.total_count() == 0) ? 2 : 0;
			inputs[54] = (spots_ < kSpotsEnough) ? -5 : 0;
			inputs[55] = (bo.max_needed_preciousness >= 10 &&
			              (bo.cnt_under_construction + bo.unoccupied_count) == 1) ?
                         3 :
                         0;
			inputs[56] = (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) ? -8 : 1;
			inputs[57] = (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) ? -6 : 1;
			inputs[58] = (bo.total_count() == 0 && inputs_on_stock) ? 4 : 0;
			inputs[59] = (bo.inputs.empty()) ? 5 : bo.current_stats / 10 - 5;
			inputs[60] = (spots_ < kSpotsTooLittle) ? -10 : 0;
			inputs[61] = (player_statistics.any_enemy_seen_lately(gametime)) ? 2 : 0;
			inputs[62] = (player_statistics.any_enemy_seen_lately(gametime) &&
			              bo.cnt_under_construction + bo.unoccupied_count == 0) ?
                         6 :
                         0;
			inputs[63] = (!bo.ware_outputs.empty() && !bo.inputs.empty()) ? bo.current_stats / 10 : 0;
			inputs[64] = (gametime > Time(20 * 60 * 1000) && bo.total_count() == 0) ? 3 : 0;
			inputs[65] = (gametime > Time(45 * 60 * 1000) && bo.total_count() == 0) ? 3 : 0;
			inputs[66] = (gametime > Time(60 * 60 * 1000) && bo.total_count() <= 1) ? 3 : 0;
			inputs[67] = (gametime > Time(50 * 60 * 1000) && bo.total_count() <= 1) ? 2 : 0;
			inputs[68] =
			   (bo.inputs.empty() && gametime > Time(50 * 60 * 1000) && bo.total_count() <= 1) ? 2 : 0;
			inputs[69] =
			   (!bo.inputs.empty() && gametime > Time(50 * 60 * 1000) && bo.total_count() <= 1) ? 2 :
                                                                                               0;
			inputs[70] =
			   (bo.inputs.empty() && gametime > Time(25 * 60 * 1000) && bo.total_count() <= 1) ? 2 : 0;
			inputs[71] =
			   (!bo.inputs.empty() && gametime > Time(25 * 60 * 1000) && bo.total_count() <= 1) ? 2 :
                                                                                               0;
			if (bo.last_building_built.is_valid()) {
				inputs[72] = (gametime < bo.last_building_built + Duration(3 * 60 * 1000)) ? -4 : 0;
				inputs[73] = (gametime < bo.last_building_built + Duration(5 * 60 * 1000)) ? -2 : 0;
				inputs[74] = (gametime < bo.last_building_built + Duration(2 * 60 * 1000)) ? -5 : 0;
				inputs[75] = (gametime < bo.last_building_built + Duration(10 * 60 * 1000)) ? -2 : 0;
				inputs[76] = (gametime < bo.last_building_built + Duration(20 * 60 * 1000)) ? -2 : 0;
			}
			inputs[77] = (gametime > Time(35 * 60 * 1000) && bo.total_count() == 0) ? 3 : 0;
			inputs[78] = (gametime > Time(60 * 60 * 1000) && bo.total_count() == 0) ? 3 : 0;
			inputs[79] = (expansion_type.get_expansion_type() == ExpansionMode::kResources ||
			              expansion_type.get_expansion_type() == ExpansionMode::kSpace) *
			             management_data.get_military_number_at(37) / 10;
			inputs[80] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) *
			             management_data.get_military_number_at(38) / 10;
			inputs[81] = (expansion_type.get_expansion_type() == ExpansionMode::kSpace) *
			             management_data.get_military_number_at(46) / 10;
			inputs[82] = (inputs_on_stock) ? 0 : -2;
			inputs[83] = (suppliers_exist) ? 0 : -2;
			inputs[84] = (inputs_on_stock) ? 0 : -4;
			inputs[85] = (suppliers_exist) ? 0 : -4;
			inputs[86] = (inputs_on_stock) ? 0 : -8;
			inputs[87] = (suppliers_exist) ? 0 : -8;
			inputs[88] = (workers_on_stock) ? 0 : -2;
			inputs[89] = (workers_on_stock) ? 0 : -6;
			inputs[90] = (bo.is(BuildingAttribute::kBuildingMatProducer)) ?
                         std::abs(management_data.get_military_number_at(10)) / 10 :
                         0;
			inputs[91] = (bo.build_material_shortage) ? -2 : 0;
			inputs[92] = (numof_psites_in_constr < 4) ? 3 : 0;
			inputs[93] = (numof_psites_in_constr < 8) ? 3 : 0;
			inputs[94] = (bo.inputs.empty()) ? 5 : 0;
			inputs[95] = (bo.inputs.empty()) ? 3 : 0;
			inputs[96] = stocked_wood_level < 25 ? -2 : 0;
			inputs[97] = stocked_wood_level < 25 ? -8 : 0;
			inputs[98] = stocked_wood_level < 25 ? -4 : 0;
			inputs[99] = stocked_wood_level < 25 ? -1 : 0;
			inputs[100] = (bo.total_count() == 0) ? 3 : 0;
			inputs[101] = (bo.total_count() == 0) ? 6 : 0;
			if (bo.is(BuildingAttribute::kSupportingProducer)) {
				if (bo.total_count() == 0) {
					inputs[102] = 1;
					inputs[103] = 2;
					inputs[104] = -2;
				}
				inputs[105] = -3;
				inputs[106] = -2;
			}
			inputs[107] =
			   std::abs(management_data.get_military_number_at(194)) - get_stocklevel(bo, gametime);
			inputs[108] =
			   std::abs(management_data.get_military_number_at(191)) - get_stocklevel(bo, gametime);
			inputs[109] =
			   (!bo.inputs.empty() && gametime > Time(50 * 60 * 1000) && bo.total_count() <= 1) ?
               std::abs(management_data.get_military_number_at(163)) / 10 :
               0;
			inputs[110] = (bo.ware_outputs.size() == 1) ?
                          (tribe_->get_ware_descr(bo.ware_outputs.at(0))
			                     ->default_target_quantity(tribe_->name()) -
			                  get_stocklevel(bo, gametime)) *
			                    std::abs(management_data.get_military_number_at(165)) / 20 :
                          0;
			inputs[111] = bo.current_stats / (bo.ware_outputs.size() + 1);
			// boost for buildings supporting seafaring
			if (bo.is(BuildingAttribute::kSupportsSeafaring) && map_allows_seafaring_) {
				inputs[112] = std::abs(management_data.get_military_number_at(170)) / 10;
				inputs[113] = 4;
				if (bo.total_count() == 0) {
					inputs[114] = std::abs(management_data.get_military_number_at(172)) / 10;
					inputs[115] = 4;
				}
			}
			inputs[116] = -(bo.unoccupied_count * bo.unoccupied_count);
			inputs[117] = -(2 * bo.unoccupied_count);

			int16_t tmp_score = 0;
			for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
				if (management_data.f_neuron_pool[8].get_position(i)) {
					const int16_t partial_input = inputs[i];
					tmp_score += partial_input;
				}
				if (management_data.f_neuron_pool[11].get_position(i)) {
					const int16_t partial_input = inputs[i + kFNeuronBitSize];
					tmp_score += partial_input;
				}
				if (management_data.f_neuron_pool[59].get_position(i)) {
					const int16_t partial_input = inputs[i + 2 * kFNeuronBitSize];
					tmp_score += partial_input;
				}
				if (management_data.f_neuron_pool[12].get_position(i)) {
					const int16_t partial_input = inputs[i + 3 * kFNeuronBitSize];
					tmp_score += partial_input;
				}
			}

			const int32_t base_economy_bonus =
			   30 + std::abs(management_data.get_military_number_at(142));
			if (site_needed_for_economy == BasicEconomyBuildingStatus::kDiscouraged) {
				tmp_score -= base_economy_bonus;
			} else if (site_needed_for_economy == BasicEconomyBuildingStatus::kEncouraged) {
				tmp_score += base_economy_bonus;
			}

			const int16_t bottom_limit = management_data.get_military_number_at(73) / 2 +
			                             management_data.get_military_number_at(47) / 10;
			const int16_t upper_limit =
			   bottom_limit + std::abs(management_data.get_military_number_at(44) / 3);

			if (tmp_score > upper_limit) {
				// Productionsite is needed
				bo.primary_priority += (tmp_score - bottom_limit) / 2;
				return BuildingNecessity::kNeeded;
			} else if (tmp_score > bottom_limit) {
				// Site is needed, but not right now
				return BuildingNecessity::kNeededPending;
			} else {
				// Not allowed
				return BuildingNecessity::kForbidden;
			}

		} else if (bo.is(BuildingAttribute::kShipyard)) {
			if (bo.total_count() > 0 ||
			    (!basic_economy_established &&
			     site_needed_for_economy == BasicEconomyBuildingStatus::kDiscouraged) ||
			    !map_allows_seafaring_) {
				return BuildingNecessity::kForbidden;
			}
			bo.primary_priority = 0;
			if (num_ports > 0) {
				bo.primary_priority += std::abs(management_data.get_military_number_at(150) * 3);
			}
			if (spots_ < kSpotsTooLittle) {
				bo.primary_priority += std::abs(management_data.get_military_number_at(151) * 3);
			}
			if (bo.primary_priority > 0) {
				return BuildingNecessity::kNeeded;
			}
			return BuildingNecessity::kAllowed;
		} else if (bo.max_needed_preciousness == 0) {
			return BuildingNecessity::kNotNeeded;
		} else {
			return BuildingNecessity::kForbidden;
		}
	} else if (purpose == PerfEvaluation::kForDismantle) {  // now for dismantling
		// never dismantle last building (a care should be taken elsewhere)
		assert(bo.total_count() > 0);
		if (bo.total_count() == 1) {
			return BuildingNecessity::kNeeded;
		} else if (bo.max_preciousness >= 10 && bo.total_count() == 2) {
			return BuildingNecessity::kNeeded;
		} else if (!bo.ware_outputs.empty() &&
		           bo.current_stats > (10 + 60 / bo.ware_outputs.size()) / 2) {
			return BuildingNecessity::kNeeded;
		} else if (bo.inputs.size() == 1 &&
		           calculate_stocklevel(static_cast<size_t>(bo.inputs.at(0))) >
		              static_cast<unsigned int>(
		                 std::abs(management_data.get_military_number_at(171)))) {
			return BuildingNecessity::kNeeded;
		} else {
			return BuildingNecessity::kNotNeeded;
		}
	}
	NEVER_HERE();
}

// counts produced output on stock
uint32_t DefaultAI::calculate_stocklevel(Widelands::DescriptionIndex wt,
                                         const WareWorker what) const {
	uint32_t count = 0;

	for (const WarehouseSiteObserver& obs : warehousesites) {
		if (what == WareWorker::kWare) {
			count += obs.site->get_wares().stock(wt);
		} else {
			count += obs.site->get_workers().stock(wt);
		}
	}
	return count;
}

// This is a wrapper function to prevent too frequent recalculation of stocklevel
// and distinguish if we count stocks for production hint, for outputs or for workers of a
// productionsite
// if multiple outputs, it returns lowest value
uint32_t
DefaultAI::get_stocklevel(BuildingObserver& bo, const Time& gametime, const WareWorker what) const {
	if (bo.stocklevel_time + Duration(5 * 1000) < gametime) {
		if (what == WareWorker::kWare &&
		    (!bo.supported_producers.empty() || !bo.ware_outputs.empty())) {
			// looking for smallest value
			bo.stocklevel_count = std::numeric_limits<uint32_t>::max();
			for (const auto& supported_building : bo.supported_producers) {
				for (Widelands::DescriptionIndex output_ware :
				     supported_building.second->output_ware_types()) {
					const uint32_t res = calculate_stocklevel(static_cast<size_t>(output_ware), what);
					if (res < bo.stocklevel_count) {
						bo.stocklevel_count = res;
					}
				}
			}
			for (auto ph : bo.ware_outputs) {
				const uint32_t res = calculate_stocklevel(static_cast<size_t>(ph), what);
				if (res < bo.stocklevel_count) {
					bo.stocklevel_count = res;
				}
			}
		} else if (what == WareWorker::kWorker) {
			bo.stocklevel_count = std::numeric_limits<uint32_t>::max();
			for (auto ph : bo.positions) {
				const uint32_t res = calculate_stocklevel(static_cast<size_t>(ph), what);
				if (res < bo.stocklevel_count) {
					bo.stocklevel_count = res;
				}
			}
		} else {
			bo.stocklevel_count = 0;
		}
		assert(bo.stocklevel_count < std::numeric_limits<uint32_t>::max());
		bo.stocklevel_time = gametime;
	}
	return bo.stocklevel_count;
}

/**
 * This function takes care about the unowned and opposing territory and
 * recalculates the priority for non-military buildings
 * The goal is to minimize losses when territory is lost
 *
 * \arg bf   = BuildableField to be checked
 * \arg prio = priority until now.
 *
 * \returns the recalculated priority
 */
int32_t DefaultAI::recalc_with_border_range(const BuildableField& bf, int32_t prio) {

	// no change when priority is not positive number
	if (prio <= 0) {
		return prio;
	}

	if (bf.enemy_nearby || bf.near_border) {
		prio /= 2;
	}

	// if unowned territory nearby
	prio -= bf.unowned_land_nearby / 4;
	prio -= bf.enemy_owned_land_nearby / 3;

	// further decrease the score if enemy nearby
	if (bf.enemy_nearby) {
		prio -= 10;
	}

	// and if close (up to 2 fields away) from border
	if (bf.near_border) {
		prio -= 10;
		if (spots_ > 0 && spots_ < kSpotsEnough) {
			prio -= std::abs(management_data.neuron_pool[60].get_result_safe(
			           kSpotsEnough / spots_, kAbsValue)) /
			        4;
		}
	}

	return prio;
}
// for buildable field, it considers effect of building of type bo on position coords
void DefaultAI::consider_productionsite_influence(BuildableField& field,
                                                  Widelands::Coords coords,
                                                  const BuildingObserver& bo) {
	if (bo.is(BuildingAttribute::kSpaceConsumer) && !bo.is(BuildingAttribute::kRanger) &&
	    game().map().calc_distance(coords, field.coords) < 8) {
		++field.space_consumers_nearby;
	}

	for (const Widelands::DescriptionIndex& i : bo.inputs) {
		++field.consumers_nearby.at(i);
	}

	for (const Widelands::DescriptionIndex& i : bo.ware_outputs) {
		++field.producers_nearby.at(i);
	}

	const Widelands::ProductionSiteDescr* productionsite =
	   dynamic_cast<const Widelands::ProductionSiteDescr*>(bo.desc);
	for (const auto& supported : productionsite->supported_productionsites()) {
		if (field.supporters_nearby.count(supported) != 1) {
			field.supporters_nearby[supported] = 0;
		}
		++field.supporters_nearby[supported];
	}
	for (const auto& supporter : productionsite->supported_by_productionsites()) {
		Widelands::DescriptionIndex supporter_building_index = tribe_->building_index(supporter);
		++field.supported_producers_nearby[supporter_building_index];
	}

	if (bo.is(BuildingAttribute::kRanger)) {
		++field.rangers_nearby;
	}

	++field.buildings_nearby[bo.id];
}

/// \returns the economy observer containing \arg economy
EconomyObserver* DefaultAI::get_economy_observer(Widelands::Economy& economy) {
	for (EconomyObserver* eco : economies) {
		if (&eco->economy == &economy) {
			return eco;
		}
	}

	economies.push_front(new EconomyObserver(economy));
	return economies.front();
}

// counts buildings with the BuildingAttribute
// Type of buildings, not individual buildings are meant
uint8_t DefaultAI::count_buildings_with_attribute(BuildingAttribute attribute) {
	uint8_t count = 0;
	if (tribe_ == nullptr) {
		late_initialization();
	}

	for (BuildingObserver& bo : buildings_) {
		if (bo.is(attribute)) {
			++count;
		}
	}

	return count;
}

// Calculates ratio of the buildings that the player has in comparison to all buildings that
// are buildable by the player
// In range 0 - 1000, to avoid floats
uint32_t DefaultAI::count_productionsites_without_buildings() {
	uint32_t total = 0;
	uint32_t existing = 0;
	if (tribe_ == nullptr) {
		late_initialization();
	}

	for (BuildingObserver& bo : buildings_) {
		if (bo.type == BuildingObserver::Type::kProductionsite &&
		    bo.is(BuildingAttribute::kBuildable)) {
			++total;
			if (bo.cnt_built > 0) {
				existing += 1000;
			} else if (bo.cnt_under_construction > 0) {
				existing += 500;
			}
		}
	}

	return (total > 0) ? (existing / total) : 0;
}

// \returns the building observer
BuildingObserver& DefaultAI::get_building_observer(char const* const name) {
	if (tribe_ == nullptr) {
		late_initialization();
	}

	for (BuildingObserver& bo : buildings_) {
		if (!strcmp(bo.name, name)) {
			return bo;
		}
	}

	throw wexception("Help: I (player %d / tribe %s) do not know what to do with a %s",
	                 player_number(), tribe_->name().c_str(), name);
}

// checks if the building has a building observer (for debug purposes)
bool DefaultAI::has_building_observer(char const* const name) {
	if (tribe_ == nullptr) {
		late_initialization();
	}

	for (BuildingObserver& bo : buildings_) {
		if (!strcmp(bo.name, name)) {
			return true;
		}
	}

	return false;
}

// return observer for a first (only) building that has required attribute
BuildingObserver& DefaultAI::get_building_observer(BuildingAttribute attribute) {
	if (tribe_ == nullptr) {
		late_initialization();
	}

	for (BuildingObserver& bo : buildings_) {
		if (bo.is(attribute)) {
			return bo;
		}
	}

	throw wexception(
	   "Sorry, cannot find building with attribute %d", static_cast<int32_t>(attribute));
}

// return observer for a building with the id
BuildingObserver& DefaultAI::get_building_observer(const Widelands::DescriptionIndex di) {
	if (tribe_ == nullptr) {
		late_initialization();
	}

	for (BuildingObserver& bo : buildings_) {
		if (bo.id == di) {
			return bo;
		}
	}

	log_err_time(
	   game().get_gametime(), "Sorry, cannot find building with id %d", static_cast<int32_t>(di));
	// I noticed that exception test is being lost so will will print it into log as well
	throw wexception("Sorry, cannot find building with id %d", static_cast<int32_t>(di));
}

// this is called whenever we gain ownership of a PlayerImmovable
void DefaultAI::gain_immovable(Widelands::PlayerImmovable& pi, const bool found_on_load) {
	if (upcast(Widelands::Building, building, &pi)) {
		gain_building(*building, found_on_load);
	} else if (upcast(Widelands::Flag const, flag, &pi)) {
		new_flags.push_back(flag);
	} else if (upcast(Widelands::Road const, road, &pi)) {
		roads.push_front(road);
	}
}

// this is called whenever we lose ownership of a PlayerImmovable
void DefaultAI::lose_immovable(const Widelands::PlayerImmovable& pi) {
	if (upcast(Widelands::Building const, building, &pi)) {
		lose_building(*building);
	} else if (upcast(Widelands::Flag const, flag, &pi)) {
		// Flag to be removed can be:
		// 1. In one of our economies
		for (EconomyObserver* eco_obs : economies) {
			if (remove_from_dqueue<Widelands::Flag>(eco_obs->flags, flag)) {
				return;
			}
		}

		// 2. in new flags to be processed yet
		if (remove_from_dqueue<Widelands::Flag>(new_flags, flag)) {
			return;
		}

		// 3. Or in neither of them
	} else if (upcast(Widelands::Road const, road, &pi)) {
		remove_from_dqueue<Widelands::Road>(roads, road);
	}
}

// this is called when a mine reports "out of resources"
void DefaultAI::out_of_resources_site(const Widelands::ProductionSite& site) {

	const Time& gametime = game().get_gametime();

	// we must identify which mine matches the productionsite a note refers to
	for (ProductionSiteObserver& mine : mines_) {
		if (mine.site == &site) {
			if (mine.no_resources_since > gametime) {
				mine.no_resources_since = gametime;
			}
			break;
		}
	}
}

// walk and search for territory controlled by some player type
// usually scanning radius is enough but sometimes we must walk to
// verify that an enemy territory is really accessible by land
bool DefaultAI::other_player_accessible(const uint32_t max_distance,
                                        uint32_t* tested_fields,
                                        uint16_t* mineable_fields_count,
                                        const Widelands::Coords& starting_spot,
                                        const WalkSearch& type) {
	const Widelands::Map& map = game().map();
	std::list<uint32_t> queue;
	std::unordered_set<uint32_t> done;
	queue.push_front(starting_spot.hash());
	Widelands::PlayerNumber const pn = player_->player_number();

	while (!queue.empty()) {
		// if already processed
		if (done.count(queue.front()) > 0) {
			queue.pop_front();
			continue;
		}

		done.insert(queue.front());

		Widelands::Coords tmp_coords = Widelands::Coords::unhash(queue.front());

		// if beyond range
		if (map.calc_distance(starting_spot, tmp_coords) > max_distance) {
			continue;
		}

		Widelands::Field* f = map.get_fcoords(tmp_coords).field;

		// not interested if not walkable (starting spot is an exemption.
		if (tmp_coords != starting_spot && !(f->nodecaps() & Widelands::MOVECAPS_WALK)) {
			continue;
		}

		// sometimes we search for any owned territory (f.e. when considering
		// a port location), but when testing (starting from) own military building
		// we must ignore own territory, of course
		const Widelands::PlayerNumber field_owner = f->get_owned_by();
		if (field_owner > 0) {

			// if field is owned by anybody
			if (type == WalkSearch::kAnyPlayer) {
				*tested_fields = done.size();
				return true;
			}

			// if somebody but not me
			if (type == WalkSearch::kOtherPlayers && field_owner != pn) {
				*tested_fields = done.size();
				return true;
			}

			// if owned by enemy
			if (type == WalkSearch::kEnemy && field_owner != pn) {
				// if not in the same team => it is an enemy
				if (!player_statistics.players_in_same_team(pn, field_owner)) {
					*tested_fields = done.size();
					return true;
				}
			}
		}

		// increase mines counter
		// (used when testing possible port location)
		if (f->nodecaps() & Widelands::BUILDCAPS_MINE) {
			++mineable_fields_count;
		}

		// add neighbours to a queue (duplicates are no problem)
		// to relieve AI/CPU we skip every second field in each direction
		// obstacles are usually wider then one field
		for (Widelands::Direction dir = Widelands::FIRST_DIRECTION; dir <= Widelands::LAST_DIRECTION;
		     ++dir) {
			Widelands::Coords neigh_coords1;
			map.get_neighbour(tmp_coords, dir, &neigh_coords1);
			Widelands::Coords neigh_coords2;
			map.get_neighbour(neigh_coords1, dir, &neigh_coords2);
			queue.push_front(neigh_coords2.hash());
		}
	}
	*tested_fields = done.size();
	return false;  // no players found
}

// this is called whenever we gain a new building
void DefaultAI::gain_building(Widelands::Building& b, const bool found_on_load) {

	BuildingObserver& bo = get_building_observer(b.descr().name().c_str());

	if (bo.type == BuildingObserver::Type::kConstructionsite) {
		BuildingObserver& target_bo = get_building_observer(
		   dynamic_cast<const Widelands::ConstructionSite&>(b).building().name().c_str());
		++target_bo.cnt_under_construction;
		if (target_bo.type == BuildingObserver::Type::kProductionsite) {
			++numof_psites_in_constr;
		}
		if (target_bo.type == BuildingObserver::Type::kMilitarysite) {
			++msites_per_size[target_bo.desc->get_size()].in_construction;
		}
		if (target_bo.type == BuildingObserver::Type::kMine) {
			++mines_per_type[target_bo.mines].in_construction;
		}
		if (target_bo.type == BuildingObserver::Type::kWarehouse) {
			++numof_warehouses_in_const_;
		}
		if (target_bo.type == BuildingObserver::Type::kTrainingsite) {
			++ts_in_const_count_;
		}

		set_taskpool_task_time(game().get_gametime(), SchedulerTaskId::kRoadCheck);

	} else {
		++bo.cnt_built;
		const Time& gametime = game().get_gametime();
		bo.last_building_built = gametime;
		// erase building from remaining_basic_buildings, unless we are loading a saved game
		if (!found_on_load && persistent_data->remaining_basic_buildings.count(bo.id) > 0) {
			if (persistent_data->remaining_basic_buildings[bo.id] > 1) {
				--persistent_data->remaining_basic_buildings[bo.id];
			} else {
				persistent_data->remaining_basic_buildings.erase(bo.id);
			}
		}
		// Remaining basic buildings map contain either no entry for the building, or the number is
		// nonzero
		assert(persistent_data->remaining_basic_buildings.count(bo.id) == 0 ||
		       persistent_data->remaining_basic_buildings[bo.id] > 0);

		if (bo.type == BuildingObserver::Type::kProductionsite) {
			productionsites.push_back(ProductionSiteObserver());
			productionsites.back().site = &dynamic_cast<Widelands::ProductionSite&>(b);
			productionsites.back().bo = &bo;
			productionsites.back().bo->new_building_overdue = 0;
			if (found_on_load && gametime > Time(5 * 60 * 1000)) {
				productionsites.back().built_time = gametime - Duration(5 * 60 * 1000);
			} else {
				productionsites.back().built_time = gametime;
			}
			productionsites.back().unoccupied_till = gametime;
			++productionsites.back().bo->unoccupied_count;
			if (bo.is(BuildingAttribute::kShipyard)) {
				marine_task_queue.push_back(kStopShipyard);
				marine_task_queue.push_back(kReprioritize);
			}
			if (bo.is(BuildingAttribute::kFisher)) {
				++fishers_count_;
			}

		} else if (bo.type == BuildingObserver::Type::kMine) {
			mines_.push_back(ProductionSiteObserver());
			mines_.back().site = &dynamic_cast<Widelands::ProductionSite&>(b);
			mines_.back().bo = &bo;
			mines_.back().built_time = gametime;
			assert(mines_.back().no_resources_since.is_invalid());
			assert(!mines_.back().upgrade_pending);
			assert(mines_.back().dismantle_pending_since.is_invalid());
			++mines_.back().bo->unoccupied_count;

			++mines_per_type[bo.mines].finished;

			if (bo.is(BuildingAttribute::kBuildingMatProducer)) {
				++buil_material_mines_count;
			}

			set_inputs_to_zero(mines_.back());

			// Is this first mine?
			if (bo.mines == iron_resource_id && gametime < first_iron_mine_built) {
				first_iron_mine_built = gametime;
			}

		} else if (bo.type == BuildingObserver::Type::kMilitarysite) {
			militarysites.push_back(MilitarySiteObserver());
			militarysites.back().site = &dynamic_cast<Widelands::MilitarySite&>(b);
			militarysites.back().bo = &bo;
			militarysites.back().understaffed = 0;
			if (found_on_load && gametime > Time(5 * 60 * 1000)) {
				militarysites.back().built_time = gametime - Duration(5 * 60 * 1000);
			} else {
				militarysites.back().built_time = gametime;
			}
			militarysites.back().last_change = Time(0);
			++msites_per_size[bo.desc->get_size()].finished;

		} else if (bo.type == BuildingObserver::Type::kTrainingsite) {
			++ts_without_trainers_;
			++ts_finished_count_;
			trainingsites.push_back(TrainingSiteObserver());
			trainingsites.back().site = &dynamic_cast<Widelands::TrainingSite&>(b);
			trainingsites.back().bo = &bo;

		} else if (bo.type == BuildingObserver::Type::kWarehouse) {
			++numof_warehouses_;
			warehousesites.push_back(WarehouseSiteObserver());
			warehousesites.back().site = &dynamic_cast<Widelands::Warehouse&>(b);
			warehousesites.back().bo = &bo;
			if (bo.is(BuildingAttribute::kPort)) {
				++num_ports;
			}
		}
	}
}

// this is called whenever we lose a building
void DefaultAI::lose_building(const Widelands::Building& b) {

	BuildingObserver& bo = get_building_observer(b.descr().name().c_str());

	if (bo.type == BuildingObserver::Type::kConstructionsite) {
		BuildingObserver& target_bo = get_building_observer(
		   dynamic_cast<const Widelands::ConstructionSite&>(b).building().name().c_str());
		--target_bo.cnt_under_construction;
		if (target_bo.type == BuildingObserver::Type::kProductionsite) {
			--numof_psites_in_constr;
		}
		if (target_bo.type == BuildingObserver::Type::kMilitarysite) {
			--msites_per_size[target_bo.desc->get_size()].in_construction;
		}
		if (target_bo.type == BuildingObserver::Type::kMine) {
			--mines_per_type[target_bo.mines].in_construction;
		}
		if (target_bo.type == BuildingObserver::Type::kWarehouse) {
			--numof_warehouses_in_const_;
		}
		if (target_bo.type == BuildingObserver::Type::kTrainingsite) {
			assert(ts_in_const_count_ > 0);
			--ts_in_const_count_;
		}

	} else {
		--bo.cnt_built;

		// we are not able to reliably identify if lost building is counted in
		// unconnected or unoccupied count, but we must adjust the value to
		// avoid inconsistency
		const uint32_t cnt_built = bo.cnt_built;
		if (bo.unconnected_count > cnt_built) {
			bo.unconnected_count = cnt_built;
		}
		if (bo.unoccupied_count > cnt_built) {
			bo.unoccupied_count = cnt_built;
		}

		if (bo.type == BuildingObserver::Type::kProductionsite) {
			for (std::deque<ProductionSiteObserver>::iterator i = productionsites.begin();
			     i != productionsites.end(); ++i) {
				if (i->site == &b) {
					if (i->upgrade_pending) {
						--bo.cnt_upgrade_pending;
					}
					assert(bo.cnt_upgrade_pending == 0 || bo.cnt_upgrade_pending == 1);
					productionsites.erase(i);
					break;
				}
			}

			if (bo.is(BuildingAttribute::kFisher)) {
				assert(fishers_count_ > 0);
				--fishers_count_;
			}

		} else if (bo.type == BuildingObserver::Type::kMine) {
			for (std::deque<ProductionSiteObserver>::iterator i = mines_.begin(); i != mines_.end();
			     ++i) {
				if (i->site == &b) {
					mines_.erase(i);
					break;
				}
			}

			--mines_per_type[bo.mines].finished;

			if (bo.is(BuildingAttribute::kBuildingMatProducer)) {
				assert(buil_material_mines_count > 0);
				++buil_material_mines_count;
			}

		} else if (bo.type == BuildingObserver::Type::kMilitarysite) {
			--msites_per_size[bo.desc->get_size()].finished;

			for (std::deque<MilitarySiteObserver>::iterator i = militarysites.begin();
			     i != militarysites.end(); ++i) {
				if (i->site == &b) {
					militarysites.erase(i);
					break;
				}
			}
		} else if (bo.type == BuildingObserver::Type::kTrainingsite) {
			assert(ts_finished_count_ >= 1);
			--ts_finished_count_;

			for (std::deque<TrainingSiteObserver>::iterator i = trainingsites.begin();
			     i != trainingsites.end(); ++i) {
				if (i->site == &b) {
					trainingsites.erase(i);
					break;
				}
			}
		} else if (bo.type == BuildingObserver::Type::kWarehouse) {
			assert(numof_warehouses_ > 0);
			--numof_warehouses_;
			if (bo.is(BuildingAttribute::kPort)) {
				--num_ports;
			}

			for (std::deque<WarehouseSiteObserver>::iterator i = warehousesites.begin();
			     i != warehousesites.end(); ++i) {
				if (i->site == &b) {
					warehousesites.erase(i);
					break;
				}
			}
		}
	}
}

// Checks that supply line exists for given building.
// Recursively verify that all inputs have a producer.
// TODO(unknown): this function leads to periodic freezes of ~1 second on big games on my system.
// TODO(unknown): It needs profiling and optimization.
bool DefaultAI::check_supply(const BuildingObserver& bo) {
	size_t supplied = 0;
	for (const Widelands::DescriptionIndex& temp_inputs : bo.inputs) {
		for (const BuildingObserver& temp_building : buildings_) {
			if (temp_building.cnt_built &&
			    std::find(temp_building.ware_outputs.begin(), temp_building.ware_outputs.end(),
			              temp_inputs) != temp_building.ware_outputs.end() &&
			    check_supply(temp_building)) {
				++supplied;
				break;
			}
		}
	}

	return supplied == bo.inputs.size();
}

// TODO(tiborb): - should be called from scheduler, once in 60s is enough
void DefaultAI::update_player_stat(const Time& gametime) {
	if (player_statistics.get_update_time() > Time(0) &&
	    player_statistics.get_update_time() + Duration(15 * 1000) > gametime) {
		return;
	}
	player_statistics.set_update_time(gametime);
	Widelands::PlayerNumber const pn = player_number();
	Widelands::PlayerNumber const nr_players = game().map().get_nrplayers();

	// receiving games statistics and parsing it (reading latest entry)
	const Widelands::Game::GeneralStatsVector& genstats = game().get_general_statistics();

	// Collecting statistics and saving them in player_statistics object
	const Widelands::Player* me = game().get_player(pn);
	for (Widelands::PlayerNumber j = 1; j <= nr_players; ++j) {
		const Widelands::Player* this_player = game().get_player(j);
		if (this_player) {
			try {
				const uint32_t vsize = genstats.at(j - 1).miltary_strength.size();

				uint32_t cur_strength = 0;
				uint32_t cur_land = 0;
				uint32_t old_strength = 0;
				uint32_t old60_strength = 0;
				uint32_t old_land = 0;
				uint32_t old60_land = 0;
				uint32_t cass = 0;
				if (vsize > 0) {
					cur_strength = genstats.at(j - 1).miltary_strength.back();
					cur_land = genstats.at(j - 1).land_size.back();
					cass = genstats.at(j - 1).nr_casualties.back();

					if (vsize > 21) {
						old_strength = genstats.at(j - 1).miltary_strength[vsize - 20];
						old_land = genstats.at(j - 1).land_size[vsize - 20];
					} else {
						old_strength = genstats.at(j - 1).miltary_strength[0];
						old_land = genstats.at(j - 1).land_size[0];
					}
					if (vsize > 91) {
						old60_strength = genstats.at(j - 1).miltary_strength[vsize - 90];
						old60_land = genstats.at(j - 1).land_size[vsize - 90];
					} else {
						old60_strength = genstats.at(j - 1).miltary_strength[0];
						old60_land = genstats.at(j - 1).land_size[0];
					}
				}

				player_statistics.add(pn, j, me->team_number(), this_player->team_number(),
				                      cur_strength, old_strength, old60_strength, cass, cur_land,
				                      old_land, old60_land);
			} catch (const std::out_of_range&) {
				verb_log_warn_time(gametime, "ComputerPlayer(%d): genstats entry missing - size :%d\n",
				                   static_cast<unsigned int>(player_number()),
				                   static_cast<unsigned int>(genstats.size()));
			}
		} else {
			// Well, under some circumstances it is possible we have stat for this player and he does
			// not exist anymore
			player_statistics.remove_stat(j);
		}
	}

	player_statistics.recalculate_team_power();
}

// This runs once in 15 minutes, and adjust wares targets based on number of
// productionsites and ports
void DefaultAI::review_wares_targets(const Time& gametime) {

	player_ = game().get_player(player_number());
	tribe_ = &player_->tribe();

	// to avoid floats real multiplier is multiplier/10
	const uint16_t multiplier = std::max<uint16_t>((productionsites.size() + num_ports * 5) / 5, 10);

	for (EconomyObserver* observer : economies) {
		if (observer->economy.type() != Widelands::wwWARE) {
			// Don't set ware target quantities for worker economies
			continue;
		}

		Widelands::DescriptionIndex nritems = player_->egbase().descriptions().nr_wares();
		for (Widelands::DescriptionIndex id = 0; id < nritems; ++id) {

			// Just skip wares that are not used by a tribe
			if (!tribe_->has_ware(id)) {
				continue;
			}

			uint16_t default_target =
			   tribe_->get_ware_descr(id)->default_target_quantity(tribe_->name());

			// It seems that when default target for ware is not set, it returns
			// kInvalidWare (=254), this is confusing for AI so we change it to 10
			if (default_target == Widelands::kInvalidWare) {
				default_target = kTargetQuantCap;
			}

			const uint16_t new_target = std::max<uint16_t>(default_target * multiplier / 10, 3);
			assert(new_target > 1);

			game().send_player_command(new Widelands::CmdSetWareTargetQuantity(
			   gametime, player_number(), observer->economy.serial(), id, new_target));
		}
	}
}

// Sets due_time based on job ID
void DefaultAI::set_taskpool_task_time(const Time& gametime, const SchedulerTaskId task) {

	for (auto& item : taskPool) {
		if (item.id == task) {
			item.due_time = gametime;
			return;
		}
	}
	NEVER_HERE();
}

// Retrieves due time of the task based on its ID
const Time& DefaultAI::get_taskpool_task_time(const SchedulerTaskId task) {
	for (const auto& item : taskPool) {
		if (item.id == task) {
			return item.due_time;
		}
	}

	throw wexception("AI internal error: nonexistent task.");
}

// This performs one "iteration" of sorting based on due_time
// We by design do not need full sorting...
void DefaultAI::sort_task_pool() {
	assert(!taskPool.empty());
	for (int8_t i = taskPool.size() - 1; i > 0; --i) {
		if (taskPool[i - 1].due_time > taskPool[i].due_time) {
			std::iter_swap(taskPool.begin() + i - 1, taskPool.begin() + i);
		}
	}
}

// following two functions count mines of the same type (same output,
// all levels)
uint32_t DefaultAI::mines_in_constr() const {
	uint32_t count = 0;
	for (const auto& m : mines_per_type) {
		count += m.second.in_construction;
	}
	return count;
}

uint32_t DefaultAI::mines_built() const {
	uint32_t count = 0;
	for (const auto& m : mines_per_type) {
		count += m.second.finished;
	}
	return count;
}

// following two functions count militarysites of the same size
uint32_t DefaultAI::msites_in_constr() const {
	uint32_t count = 0;
	for (const auto& m : msites_per_size) {
		count += m.second.in_construction;
	}
	return count;
}
uint32_t DefaultAI::msites_built() const {
	uint32_t count = 0;
	for (const auto& m : msites_per_size) {
		count += m.second.finished;
	}
	return count;
}

// This prints some basic statistics during a game to the command line -
// missing materials and counts of different types of buildings.
// The main purpose of this is when a game creator needs to finetune a map
// and needs to know what resourcess are missing for which player and so on.
// By default it is off (see kPrintStats)
// TODO(tiborb): - it would be nice to have this activated by a command line switch
void DefaultAI::print_stats(const Time& gametime) {

	if (!kPrintStats) {
		set_taskpool_task_time(Time(), SchedulerTaskId::kPrintStats);
		return;
	}

	Widelands::PlayerNumber const pn = player_number();

	const Widelands::DescriptionIndex& nr_buildings = game().descriptions().nr_buildings();
	std::set<Widelands::DescriptionIndex> materials;

	// Collect information about the different buildings that our tribe can have
	for (Widelands::DescriptionIndex building_index = 0; building_index < nr_buildings;
	     ++building_index) {
		const Widelands::BuildingDescr& bld = *tribe_->get_building_descr(building_index);
		if (!tribe_->has_building(building_index)) {
			continue;
		}
		if (bld.type() == Widelands::MapObjectType::PRODUCTIONSITE) {
			const Widelands::ProductionSiteDescr& prod =
			   dynamic_cast<const Widelands::ProductionSiteDescr&>(bld);
			for (const auto& temp_input : prod.input_wares()) {
				materials.insert(temp_input.first);
			}
			for (const auto& temp_cost : prod.buildcost()) {
				materials.insert(temp_cost.first);
			}
		}

		if (bld.type() == Widelands::MapObjectType::TRAININGSITE) {
			const Widelands::ProductionSiteDescr& train =
			   dynamic_cast<const Widelands::TrainingSiteDescr&>(bld);
			for (const auto& temp_cost : train.buildcost()) {
				materials.insert(temp_cost.first);
			}
		}
	}

	verb_log_dbg_time(
	   gametime, " AI %1d: %s Buildings count: Pr:%3u, Ml:%3u, Mi:%2u, Wh:%2u, Po:%u.\n", pn,
	   gamestring_with_leading_zeros(gametime.get()), static_cast<uint32_t>(productionsites.size()),
	   static_cast<uint32_t>(militarysites.size()), static_cast<uint32_t>(mines_.size()),
	   static_cast<uint32_t>(warehousesites.size() - num_ports), num_ports);
	verb_log_dbg_time(gametime, " %1s %-30s   %5s(perf)  %6s %6s %6s %8s %5s %5s %5s %5s\n", "T",
	                  "Buildings", "work.", "const.", "unocc.", "uncon.", "needed", "prec.", "pprio",
	                  "stock", "targ.");
	for (BuildingObserver& bo : buildings_) {
		if ((bo.total_count() > 0 || bo.new_building == BuildingNecessity::kNeeded ||
		     bo.new_building == BuildingNecessity::kForced ||
		     bo.new_building == BuildingNecessity::kNeededPending ||
		     bo.new_building == BuildingNecessity::kAllowed) &&
		    bo.type != BuildingObserver::Type::kMilitarysite) {
			std::string needeness;
			if (bo.new_building == BuildingNecessity::kNeededPending) {
				needeness = "pend";
			} else if (bo.new_building == BuildingNecessity::kForced) {
				needeness = "forc";
			} else if (bo.new_building == BuildingNecessity::kAllowed) {
				needeness = "allw";
			} else if (bo.new_building == BuildingNecessity::kNotNeeded ||
			           bo.new_building == BuildingNecessity::kForbidden) {
				needeness = "no";
			} else {
				needeness = "yes";
			}
			std::string btype;
			switch (bo.type) {
			case BuildingObserver::Type::kWarehouse:
				btype = "W";
				break;
			case BuildingObserver::Type::kMine:
				btype = "M";
				break;
			case BuildingObserver::Type::kTrainingsite:
				btype = "T";
				break;
			case BuildingObserver::Type::kProductionsite:
				btype = "P";
				break;
			default:
				btype = "?";
			}

			verb_log_dbg_time(gametime, " %1s %-30s %5d(%3d%%)  %6d %6d %6d %8s %5d %5d %5d %5d\n",
			                  btype.c_str(), bo.name,
			                  bo.total_count() - bo.cnt_under_construction - bo.unoccupied_count -
			                     bo.unconnected_count,
			                  bo.current_stats, bo.cnt_under_construction, bo.unoccupied_count,
			                  bo.unconnected_count, needeness.c_str(), bo.max_needed_preciousness,
			                  bo.primary_priority, get_stocklevel(bo, gametime), bo.cnt_target);
		}
	}

	std::string why = "; Why: ";

	if ((numof_psites_in_constr + mines_in_constr()) >
	    (productionsites.size() + mines_built()) / persistent_data->ai_productionsites_ratio + 2) {
		why += " too many constr.";
	}
	// 3. too keep some proportions production sites vs military sites
	if ((numof_psites_in_constr + productionsites.size()) >
	    (msites_in_constr() + militarysites.size()) * 5) {
		why += ", too many productionsites";
	}
	// 4. if we do not have 2 mines at least
	if (mines_.size() < 2) {
		why += ", less then 2 mines";
	}

	verb_log_dbg_time(
	   gametime, "Prodsites in constr: %2d, mines in constr: %2d %s %s\n", numof_psites_in_constr,
	   mines_in_constr(),
	   (expansion_type.get_expansion_type() != ExpansionMode::kEconomy) ? "NEW BUILDING STOP" : "",
	   why.c_str());

	verb_log_dbg_time(gametime,
	                  "Least military score: %5d/%3d, msites in constr: %3d,"
	                  "soldier st: %2d, strength: %3d\n",
	                  persistent_data->least_military_score,
	                  persistent_data->ai_personality_mil_upper_limit, msites_in_constr(),
	                  static_cast<int8_t>(soldier_status_),
	                  player_statistics.get_modified_player_power(player_number()));
}

template <typename T>
void DefaultAI::check_range(T value, T bottom_range, T upper_range, const char* value_name) {
	if (value < bottom_range || value > upper_range) {
		verb_log_dbg_time(game().get_gametime(), " %d: unexpected value for %s: %d\n",
		                  player_number(), value_name, value);
	}
}

template <typename T> void DefaultAI::check_range(T value, T upper_range, const char* value_name) {
	if (value > upper_range) {
		verb_log_dbg_time(game().get_gametime(), " %d: unexpected value for %s: %d\n",
		                  player_number(), value_name, value.get());
	}
}

template <typename T>
bool DefaultAI::remove_from_dqueue(std::deque<T const*>& dq, T const* member) {
	for (auto it = dq.begin(); it != dq.end(); ++it) {
		if (*it == member) {
			it = dq.erase(it);
			return true;
		}
	}
	return false;
}

// Looking for situation where for a critical mine (iron, or marble) there is just one mine and it
// is
// unoccupied, probably we need to dismantle another one to release a miner
bool DefaultAI::critical_mine_unoccupied(const Time& gametime) {
	// resetting unoccupied
	for (auto& mine : mines_per_type) {
		mine.second.unoccupied = 0;
	}
	for (auto& mine : mines_) {
		if (!mines_per_type[mine.bo->mines].is_critical) {
			continue;
		}
		if (mine.built_time + Duration(3 * 60 * 1000) < gametime && !mine.site->can_start_working()) {
			++mines_per_type[mine.bo->mines].unoccupied;
		}
	}

	// Now check that that there is no working mine of the critical type
	for (auto& mine : mines_per_type) {
		if (mine.second.is_critical && mine.second.finished > 0 &&
		    mine.second.unoccupied == mine.second.finished) {
			return true;
		}
		assert(mine.second.unoccupied <= mines_.size());
		assert(mine.second.unoccupied <= mine.second.total_count());
	}
	return false;
}

// Sets all inputs to zero and return true if inputs are already empty
bool DefaultAI::set_inputs_to_zero(const ProductionSiteObserver& site) {
	uint16_t remaining_wares = 0;

	for (auto& queue : site.site->inputqueues()) {
		remaining_wares += queue->get_filled();
		if (queue->get_max_fill() > 0) {
			game().send_player_set_input_max_fill(
			   *site.site, queue->get_index(), queue->get_type(), 0);
		}
	}
	return remaining_wares == 0;
}

void DefaultAI::set_inputs_to_max(const ProductionSiteObserver& site) {
	for (auto& queue : site.site->inputqueues()) {
		if (queue->get_max_fill() < queue->get_max_size()) {
			game().send_player_set_input_max_fill(
			   *site.site, queue->get_index(), queue->get_type(), queue->get_max_size());
		}
	}
}
void DefaultAI::stop_site(const ProductionSiteObserver& site) {
	if (!site.site->is_stopped()) {
		game().send_player_start_stop_building(*site.site);
	}
}

void DefaultAI::initiate_dismantling(ProductionSiteObserver& site, const Time& gametime) {
	site.dismantle_pending_since = gametime;
	set_inputs_to_zero(site);
	site.bo->construction_decision_time = gametime;
}

Widelands::PlayerNumber DefaultAI::get_land_owner(const Widelands::Map& map,
                                                  const uint32_t coords) const {
	Widelands::FCoords f = map.get_fcoords(Widelands::Coords::unhash(coords));
	return f.field->get_owned_by();
}
}  // namespace AI
