/*
 * Copyright (C) 2004, 2006-2010, 2012, 2016 by the Widelands Development Team
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
#include <cmath>
#include <ctime>
#include <memory>
#include <queue>
#include <typeinfo>
#include <unordered_set>

#include "ai/ai_hints.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/time_string.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "economy/wares_queue.h"
#include "logic/findbob.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
#include "logic/playercommand.h"

// following is in miliseconds (widelands counts time in ms)
constexpr int kFieldInfoExpiration = 12 * 1000;
constexpr int kMineFieldInfoExpiration = 20 * 1000;
constexpr int kNewMineConstInterval = 19000;
constexpr int kBusyMineUpdateInterval = 2000;
// building of the same building can be started after 25s at earliest
constexpr int kBuildingMinInterval = 25 * 1000;
constexpr int kMinBFCheckInterval = 5 * 1000;
constexpr int kMinMFCheckInterval = 19 * 1000;
constexpr int kShipCheckInterval = 5 * 1000;
constexpr int kMarineDecisionInterval = 20 * 1000;
constexpr int kTrainingSitesCheckInterval = 15 * 1000;
constexpr int kManagementUpdateInterval = 10 * 60 * 1000;

// handfull of constants used for expeditions/colonization
constexpr int kColonyScanStartArea = 35;
constexpr int kColonyScanMinArea = 10;
constexpr int kExpeditionMaxDuration = 90 * 60 * 1000;
constexpr uint32_t kNoShip = std::numeric_limits<uint32_t>::max();
// constexpr uint32_t kNever = std::numeric_limits<uint32_t>::max();
constexpr uint32_t kNoExpedition = 0;

// following two are used for roads management, for creating shortcuts and dismantling dispensable
// roads
constexpr int32_t kSpotsTooLittle = 15;
constexpr int32_t kSpotsEnough = 25;

constexpr uint16_t kTargetQuantCap = 30;

// this is intended for map developers, by default should be off
constexpr bool kPrintStats = true;

constexpr int8_t kUncalculated = -1;
constexpr uint8_t kFalse = 0;
constexpr uint8_t kTrue = 1;

constexpr bool kAbsValue = true;

// duration of military campaign
constexpr int kCampaignDuration = 15 * 60 * 1000;

// for scheduler
constexpr int kMaxJobs = 4;

using namespace Widelands;

DefaultAI::NormalImpl DefaultAI::normal_impl;
DefaultAI::WeakImpl DefaultAI::weak_impl;
DefaultAI::VeryWeakImpl DefaultAI::very_weak_impl;

/// Constructor of DefaultAI
DefaultAI::DefaultAI(Game& ggame, PlayerNumber const pid, DefaultAI::Type const t)
   : ComputerPlayer(ggame, pid),
     type_(t),
     player_(nullptr),
     tribe_(nullptr),
     num_prod_constructionsites(0),
     num_ports(0),
     last_attack_time_(0),
     enemysites_check_delay_(60),
     wood_policy_(WoodPolicy::kAllowRangers),
     soldier_status_(SoldiersStatus::kEnough),
     next_ai_think_(0),
     next_mine_construction_due_(0),
     inhibit_road_building_(0),
     time_of_last_construction_(0),
     numof_warehouses_(0),
     avg_military_score_(0),
     new_buildings_stop_(false),
     needs_boost_economy(false),
     resource_necessity_territory_(100),
     resource_necessity_mines_(100),
     resource_necessity_water_(0),
     resource_necessity_water_needed_(false),
     military_last_dismantle_(0),
     military_last_build_(0),
     last_road_dismantled_(0),
     military_status_last_updated(0),
     seafaring_economy(false),
     expedition_ship_(kNoShip),
     spots_(0),
     ts_basic_count_(0),
     ts_basic_const_count_(0),
     ts_advanced_count_(0),
     ts_advanced_const_count_(0),
     ts_without_trainers_(0),
     highest_nonmil_prio_(0),
     // attack_count_(0),
     scheduler_delay_counter_(0) {

	// Subscribe to NoteFieldPossession.
	field_possession_subscriber_ =
	   Notifications::subscribe<NoteFieldPossession>([this](const NoteFieldPossession& note) {
		   if (note.player != player_) {
			   return;
		   }
		   if (note.ownership == NoteFieldPossession::Ownership::GAINED) {
			   unusable_fields.push_back(note.fc);
		   }
		});

	// Subscribe to NoteImmovables.
	immovable_subscriber_ =
	   Notifications::subscribe<NoteImmovable>([this](const NoteImmovable& note) {
		   if (player_ == nullptr) {
			   return;
		   }
		   if (note.pi->owner().player_number() != player_->player_number()) {
			   return;
		   }
		   if (note.ownership == NoteImmovable::Ownership::GAINED) {
			   gain_immovable(*note.pi);
		   } else {
			   lose_immovable(*note.pi);
		   }
		});

	// Subscribe to ProductionSiteOutOfResources.
	outofresource_subscriber_ = Notifications::subscribe<NoteProductionSiteOutOfResources>(
	   [this](const NoteProductionSiteOutOfResources& note) {
		   if (note.ps->owner().player_number() != player_->player_number()) {
			   return;
		   }

		   out_of_resources_site(*note.ps);

		});

	// Subscribe to TrainingSiteSoldierTrained.
	soldiertrained_subscriber_ = Notifications::subscribe<NoteTrainingSiteSoldierTrained>(
	   [this](const NoteTrainingSiteSoldierTrained& note) {
		   if (note.ts->owner().player_number() != player_->player_number()) {
			   return;
		   }

		   soldier_trained(*note.ts);

		});

	// Subscribe to ShipNotes.
	shipnotes_subscriber_ =
	   Notifications::subscribe<NoteShipMessage>([this](const NoteShipMessage& note) {

		   // in a short time between start and late_initialization the player
		   // can get notes that can not be processed.
		   // It seems that this causes no problem, at least no substantial
		   if (player_ == nullptr) {
			   return;
		   }
		   if (note.ship->get_owner()->player_number() != player_->player_number()) {
			   return;
		   }

		   switch (note.message) {

		   case NoteShipMessage::Message::kGained:
			   gain_ship(*note.ship, NewShip::kBuilt);
			   break;

		   case NoteShipMessage::Message::kLost:
			   for (std::list<ShipObserver>::iterator i = allships.begin(); i != allships.end(); ++i) {
				   if (i->ship == note.ship) {
					   allships.erase(i);
					   break;
				   }
			   }
			   break;

		   case NoteShipMessage::Message::kWaitingForCommand:
			   for (std::list<ShipObserver>::iterator i = allships.begin(); i != allships.end(); ++i) {
				   if (i->ship == note.ship) {
					   i->waiting_for_command_ = true;
					   break;
				   }
			   }
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

	if (tribe_ == nullptr) {
		late_initialization();
	}

	const uint32_t gametime = static_cast<uint32_t>(game().get_gametime());

	if (next_ai_think_ > gametime) {
		return;
	}

	// AI now thinks twice in a seccond, if the game engine allows this
	// if too busy, the period can be many seconds.
	next_ai_think_ = gametime + 500;
	SchedulerTaskId due_task = SchedulerTaskId::kUnset;

	sort_task_pool();

	const int32_t delay_time = gametime - taskPool.front().due_time;

	// Here we decide how many jobs will be run now (none - 5)
	// in case no job is due now, it can be zero
	uint32_t jobs_to_run_count = (delay_time < 0) ? 0 : 1;

	// Here we collect data for "too late ..." message
	if (delay_time > 5000) {
		scheduler_delay_counter_ += 1;
	} else {
		scheduler_delay_counter_ = 0;
	}

	if (jobs_to_run_count == 0) {
		// well we have nothing to do now
		return;
	}

	// And printing it now and resetting counter
	if (scheduler_delay_counter_ > 10) {
		log(" %d: AI: game speed too high, jobs are too late (now %2d seconds)\n", player_number(),
		    static_cast<int32_t>(delay_time / 1000));
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
	for (uint8_t i = 0; i < jobs_to_run_count; i += 1) {
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
	for (uint8_t i = 0; i < current_task_queue.size(); ++i) {

		due_task = current_task_queue[i].id;

		sched_stat_[static_cast<uint32_t>(due_task)] += 1;

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
			};
			set_taskpool_task_time(gametime + 1000, SchedulerTaskId::kRoadCheck);
			// testing 5 roads
			{
				const int32_t roads_to_check = (roads.size() + 1 < 5) ? roads.size() + 1 : 5;
				for (int j = 0; j < roads_to_check; j += 1) {
					// improve_roads function will test one road and rotate roads vector
					if (improve_roads(gametime)) {
						// if significant change takes place do not go on
						break;
					};
				}
			}
			break;
		case SchedulerTaskId::kUnbuildableFCheck:
			set_taskpool_task_time(gametime + 4000, SchedulerTaskId::kUnbuildableFCheck);
			update_all_not_buildable_fields();
			break;
		case SchedulerTaskId::kCheckEconomies:
			check_economies();
			set_taskpool_task_time(gametime + 8000, SchedulerTaskId::kCheckEconomies);
			break;
		case SchedulerTaskId::kProductionsitesStats:
			update_productionsite_stats();
			// Updating the stats every 10 seconds should be enough
			set_taskpool_task_time(gametime + 10000, SchedulerTaskId::kProductionsitesStats);
			break;
		case SchedulerTaskId::kConstructBuilding:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			if (gametime < 15000) {  // More frequent at the beginning of game
				set_taskpool_task_time(gametime + 2000, SchedulerTaskId::kConstructBuilding);
			} else {
				set_taskpool_task_time(gametime + 6000, SchedulerTaskId::kConstructBuilding);
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
				set_taskpool_task_time(gametime + 15000, SchedulerTaskId::kCheckProductionsites);
				// testing 5 productionsites (if there are 5 of them)
				int32_t ps_to_check = (productionsites.size() < 5) ? productionsites.size() : 5;
				for (int j = 0; j < ps_to_check; j += 1) {
					// one productionsite per one check_productionsites() call
					if (check_productionsites(gametime)) {
						// if significant change takes place do not go on
						break;
					};
				}
			}
			break;
		case SchedulerTaskId::kCheckShips:
			set_taskpool_task_time(gametime + 3 * kShipCheckInterval, SchedulerTaskId::kCheckShips);
			check_ships(gametime);
			break;
		case SchedulerTaskId::KMarineDecisions:
			set_taskpool_task_time(
			   gametime + kMarineDecisionInterval, SchedulerTaskId::KMarineDecisions);
			marine_main_decisions();
			break;
		case SchedulerTaskId::kCheckMines:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			set_taskpool_task_time(gametime + 15000, SchedulerTaskId::kCheckMines);
			// checking 3 mines if possible
			{
				int32_t mines_to_check = (mines_.size() < 5) ? mines_.size() : 5;
				for (int j = 0; j < mines_to_check; j += 1) {
					// every run of check_mines_() checks one mine
					if (check_mines_(gametime)) {
						// if significant change takes place do not go on
						break;
					};
				}
			}
			break;
		case SchedulerTaskId::kCheckMilitarysites:
			// just to be sure the value is reset
			// 4 seconds is really fine
			set_taskpool_task_time(gametime + 4 * 1000, SchedulerTaskId::kCheckMilitarysites);
			check_militarysites(gametime);
			break;
		case SchedulerTaskId::kCheckTrainingsites:
			set_taskpool_task_time(
			   gametime + kTrainingSitesCheckInterval, SchedulerTaskId::kCheckTrainingsites);
			check_trainingsites(gametime);
			break;
		case SchedulerTaskId::kCountMilitaryVacant:
			count_military_vacant_positions(gametime);
			set_taskpool_task_time(gametime + 15 * 1000, SchedulerTaskId::kCountMilitaryVacant);
			break;
		case SchedulerTaskId::kWareReview:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			set_taskpool_task_time(gametime + 15 * 60 * 1000, SchedulerTaskId::kWareReview);
			review_wares_targets(gametime);
			break;
		case SchedulerTaskId::kPrintStats:
			if (check_economies()) {  // economies must be consistent
				return;
			}
			set_taskpool_task_time(gametime + 10 * 60 * 1000, SchedulerTaskId::kPrintStats);
			print_stats(gametime);
			break;
		case SchedulerTaskId::kCheckEnemySites:
			check_enemy_sites(gametime);
			set_taskpool_task_time(gametime + 19 * 1000, SchedulerTaskId::kCheckEnemySites);
			break;
		case SchedulerTaskId::kManagementUpdate:
			management_data.review(
			   gametime, player_number(), mines_.size(),
			   player_statistics.get_player_power(player_number()),
			   player_statistics.get_player_power(player_number()) -
			      player_statistics.get_enemies_max_power(),
			   player_statistics.get_player_land(player_number()),
			   player_statistics.get_player_land(player_number()) -
			      player_statistics.get_enemies_max_land(),
			   (persistent_data->last_attacked_player != std::numeric_limits<int16_t>::max()));
			set_taskpool_task_time(
			   gametime + kManagementUpdateInterval, SchedulerTaskId::kManagementUpdate);
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
 * world), so this is done after complete loading of the map.
 */
void DefaultAI::late_initialization() {
	player_ = game().get_player(player_number());
	tribe_ = &player_->tribe();
	const uint32_t gametime = game().get_gametime();

	log("ComputerPlayer(%d): initializing as type %u\n", player_number(),
	    static_cast<unsigned int>(type_));
	if (player_->team_number() > 0) {
		log("    ... member of team %d\n", player_->team_number());
	}

	wares.resize(game().tribes().nrwares());
	for (DescriptionIndex i = 0; i < static_cast<DescriptionIndex>(game().tribes().nrwares()); ++i) {
		wares.at(i).producers = 0;
		wares.at(i).consumers = 0;
		wares.at(i).preciousness = game().tribes().get_ware_descr(i)->preciousness(tribe_->name());
	}

	const DescriptionIndex& nr_buildings = game().tribes().nrbuildings();

	// Collect information about the different buildings that our tribe can have
	for (DescriptionIndex building_index = 0; building_index < nr_buildings; ++building_index) {
		const BuildingDescr& bld = *tribe_->get_building_descr(building_index);
		if (!tribe_->has_building(building_index) && bld.type() != MapObjectType::MILITARYSITE) {
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
		bo.stocklevel_time = 0;
		bo.last_dismantle_time = 0;
		// this is set to negative number, otherwise the AI would wait 25 sec
		// after game start not building anything
		bo.construction_decision_time = -60 * 60 * 1000;
		bo.last_building_built = kNever;
		bo.build_material_shortage = false;
		bo.production_hint = kUncalculated;
		bo.current_stats = 0;
		bo.unoccupied_count = 0;
		bo.unconnected_count = 0;
		bo.new_building_overdue = 0;
		bo.primary_priority = 0;
		bo.is_buildable = bld.is_buildable();
		bo.need_trees = bh.is_logproducer();
		bo.need_rocks = bh.is_graniteproducer();
		bo.need_water = bh.get_needs_water();
		bo.mines_water = bh.mines_water();
		bo.recruitment = bh.for_recruitment();
		bo.space_consumer = bh.is_space_consumer();
		bo.expansion_type = bh.is_expansion_type();
		bo.fighting_type = bh.is_fighting_type();
		bo.mountain_conqueror = bh.is_mountain_conqueror();
		bo.prohibited_till = bh.get_prohibited_till() * 1000;  // value in conf is in seconds
		bo.forced_after = bh.get_forced_after() * 1000;        // value in conf is in seconds
		bo.is_port = bld.get_isport();
		bo.trainingsite_type = TrainingSiteType::kNoTS;
		bo.upgrade_substitutes = false;
		bo.upgrade_extends = false;
		bo.produces_building_material = false;
		bo.max_preciousness = 0;
		bo.max_needed_preciousness = 0;

		if (bh.renews_map_resource()) {
			bo.production_hint = tribe_->safe_ware_index(bh.get_renews_map_resource());
		}

		// I just presume cut wood is named "log" in the game
		if (tribe_->safe_ware_index("log") == bo.production_hint) {
			bo.plants_trees = true;
		} else {
			bo.plants_trees = false;
		}

		// Is total count of this building limited by AI mode?
		if (type_ == DefaultAI::Type::kVeryWeak && bh.get_very_weak_ai_limit() >= 0) {
			bo.cnt_limit_by_aimode = bh.get_very_weak_ai_limit();
			log(" %d: AI 'very weak' mode: applying limit %d building(s) for %s\n", player_number(),
			    bo.cnt_limit_by_aimode, bo.name);
		}
		if (type_ == DefaultAI::Type::kWeak && bh.get_weak_ai_limit() >= 0) {
			bo.cnt_limit_by_aimode = bh.get_weak_ai_limit();
			log(" %d: AI 'weak' mode: applying limit %d building(s) for %s\n", player_number(),
			    bo.cnt_limit_by_aimode, bo.name);
		}

		// Read all interesting data from ware producing buildings
		if (bld.type() == MapObjectType::PRODUCTIONSITE) {
			const ProductionSiteDescr& prod = dynamic_cast<const ProductionSiteDescr&>(bld);
			bo.type = bld.get_ismine() ? BuildingObserver::Type::kMine :
			                             BuildingObserver::Type::kProductionsite;
			for (const auto& temp_input : prod.inputs()) {
				bo.inputs.push_back(temp_input.first);
			}
			for (const DescriptionIndex& temp_output : prod.output_ware_types()) {
				bo.outputs.push_back(temp_output);
			}

			if (bo.type == BuildingObserver::Type::kMine) {
				// get the resource needed by the mine
				if (bh.get_mines()) {
					bo.mines = game().world().get_resource(bh.get_mines());
				}

				bo.mines_percent = bh.get_mines_percent();

				// populating mines_per_type map
				if (mines_per_type.count(bo.mines) == 0) {
					mines_per_type[bo.mines] = MineTypesObserver();
				}
			}

			// here we identify hunters
			if (bo.outputs.size() == 1 && tribe_->safe_ware_index("meat") == bo.outputs.at(0)) {
				bo.is_hunter = true;
			} else {
				bo.is_hunter = false;
			}

			// and fishers
			if (bo.outputs.size() == 1 && tribe_->safe_ware_index("fish") == bo.outputs.at(0)) {
				bo.is_fisher = true;
			} else {
				bo.is_fisher = false;
			}

			bo.is_shipyard = bh.is_shipyard();

			// now we find out if the upgrade of the building is a full substitution
			// (produces all wares as current one)
			const DescriptionIndex enhancement = bld.enhancement();
			if (enhancement != INVALID_INDEX && bo.type == BuildingObserver::Type::kProductionsite) {
				std::unordered_set<DescriptionIndex> enh_outputs;
				const ProductionSiteDescr& enh_prod =
				   dynamic_cast<const ProductionSiteDescr&>(*tribe_->get_building_descr(enhancement));

				// collecting wares that are produced in enhanced building
				for (const DescriptionIndex& ware : enh_prod.output_ware_types()) {
					enh_outputs.insert(ware);
				}
				// now testing outputs of current building
				// and comparing
				bo.upgrade_substitutes = true;
				for (DescriptionIndex ware : bo.outputs) {
					if (enh_outputs.count(ware) == 0) {
						bo.upgrade_substitutes = false;
						break;
					}
				}

				std::unordered_set<DescriptionIndex> cur_outputs;
				// collecting wares that are produced in enhanced building
				for (const DescriptionIndex& ware : bo.outputs) {
					cur_outputs.insert(ware);
				}
				bo.upgrade_extends = false;
				for (DescriptionIndex ware : enh_outputs) {
					if (cur_outputs.count(ware) == 0) {
						bo.upgrade_extends = true;
						break;
					}
				}
			}

			// now we identify producers of critical build materials
			// hardwood now
			for (DescriptionIndex ware : bo.outputs) {
				// iterating over wares subsitutes
				if (tribe_->ware_index("wood") == ware || tribe_->ware_index("blackwood") == ware ||
				    tribe_->ware_index("marble") == ware || tribe_->ware_index("planks") == ware) {
					bo.produces_building_material = true;
				}
			}
			continue;
		}

		// now for every military building, we fill critical_building_material vector
		// with critical construction wares
		// non critical are excluded (see below)
		if (bld.type() == MapObjectType::MILITARYSITE) {
			bo.type = BuildingObserver::Type::kMilitarysite;
			const MilitarySiteDescr& milit = dynamic_cast<const MilitarySiteDescr&>(bld);
			for (const auto& temp_buildcosts : milit.buildcost()) {
				// bellow are non-critical wares (well, various types of wood)
				if (tribe_->ware_index("log") == temp_buildcosts.first ||
				    tribe_->ware_index("blackwood") == temp_buildcosts.first ||
				    tribe_->ware_index("planks") == temp_buildcosts.first)
					continue;

				bo.critical_building_material.push_back(temp_buildcosts.first);
			}
			continue;
		}

		if (bld.type() == MapObjectType::WAREHOUSE) {
			bo.type = BuildingObserver::Type::kWarehouse;
			continue;
		}

		if (bld.type() == MapObjectType::TRAININGSITE) {
			bo.type = BuildingObserver::Type::kTrainingsite;
			const TrainingSiteDescr& train = dynamic_cast<const TrainingSiteDescr&>(bld);
			for (const auto& temp_input : train.inputs()) {
				bo.inputs.push_back(temp_input.first);

				// collecting subsitutes
				if (tribe_->ware_index("meat") == temp_input.first ||
				    tribe_->ware_index("fish") == temp_input.first ||
				    tribe_->ware_index("smoked_meat") == temp_input.first ||
				    tribe_->ware_index("smoked_fish") == temp_input.first) {
					bo.substitute_inputs.insert(temp_input.first);
				}

				for (const auto& temp_buildcosts : train.buildcost()) {
					// critical wares for trainingsites
					if (tribe_->ware_index("spidercloth") == temp_buildcosts.first ||
					    tribe_->ware_index("gold") == temp_buildcosts.first ||
					    tribe_->ware_index("grout") == temp_buildcosts.first) {
						bo.critical_building_material.push_back(temp_buildcosts.first);
					}
				}
			}
			bo.trainingsite_type = bh.get_trainingsite_type();
			// it would behave badly if no type was set
			// make sure all TS have its type set properly in conf files
			assert(bo.trainingsite_type != TrainingSiteType::kNoTS);
			continue;
		}

		if (bld.type() == MapObjectType::CONSTRUCTIONSITE) {
			bo.type = BuildingObserver::Type::kConstructionsite;
			continue;
		}
	}

	// atlanteans they consider water as a resource
	// (together with mines, rocks and wood)
	if (tribe_->name() == "atlanteans") {
		resource_necessity_water_needed_ = true;
	}

	// Populating taskPool with all AI jobs and their starting times
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 0),
	                                 SchedulerTaskId::kConstructBuilding, 6,
	                                 "construct a building"));
	taskPool.push_back(SchedulerTask(
	   std::max<uint32_t>(gametime, 1 * 1000), SchedulerTaskId::kRoadCheck, 2, "roads check"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 15 * 1000),
	                                 SchedulerTaskId::kCheckProductionsites, 5,
	                                 "productionsites check"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 30 * 1000),
	                                 SchedulerTaskId::kProductionsitesStats, 1,
	                                 "productionsites statistics"));
	taskPool.push_back(SchedulerTask(
	   std::max<uint32_t>(gametime, 30 * 1000), SchedulerTaskId::kCheckMines, 5, "check mines"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 0 * 1000),
	                                 SchedulerTaskId::kCheckMilitarysites, 5,
	                                 "check militarysites"));
	taskPool.push_back(SchedulerTask(
	   std::max<uint32_t>(gametime, 30 * 1000), SchedulerTaskId::kCheckShips, 5, "check ships"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 1 * 1000),
	                                 SchedulerTaskId::kCheckEconomies, 1, "check economies"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 30 * 1000),
	                                 SchedulerTaskId::KMarineDecisions, 5, "marine decisions"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 2 * 60 * 1000),
	                                 SchedulerTaskId::kCheckTrainingsites, 5,
	                                 "check training sites"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 1 * 1000),
	                                 SchedulerTaskId::kBbuildableFieldsCheck, 2,
	                                 "check buildable fields"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 1 * 1000),
	                                 SchedulerTaskId::kMineableFieldsCheck, 2,
	                                 "check mineable fields"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 1 * 1000),
	                                 SchedulerTaskId::kUnbuildableFCheck, 1,
	                                 "check unbuildable fields"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 15 * 60 * 1000),
	                                 SchedulerTaskId::kWareReview, 9, "wares review"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 10 * 60 * 1000),
	                                 SchedulerTaskId::kPrintStats, 9, "print statistics"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 1 * 60 * 1000),
	                                 SchedulerTaskId::kCountMilitaryVacant, 2,
	                                 "count military vacant"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 10 * 60 * 1000),
	                                 SchedulerTaskId::kCheckEnemySites, 6, "check enemy sites"));
	taskPool.push_back(SchedulerTask(std::max<uint32_t>(gametime, 10 * 1000),
	                                 SchedulerTaskId::kManagementUpdate, 7, "Management update"));

	Map& map = game().map();

	// here we generate list of all ports and their vicinity from entire map
	for (const Coords& c : map.get_port_spaces()) {
		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(map.get_fcoords(c), 3));
		do {
			const int32_t hash = map.get_fcoords(*(mr.location().field)).hash();
			if (port_reserved_coords.count(hash) == 0)
				port_reserved_coords.insert(hash);
		} while (mr.advance(map));

		// the same for NW neighbour of a field
		Coords c_nw;
		map.get_tln(c, &c_nw);
		MapRegion<Area<FCoords>> mr_nw(map, Area<FCoords>(map.get_fcoords(c_nw), 3));
		do {
			const int32_t hash = map.get_fcoords(*(mr_nw.location().field)).hash();
			if (port_reserved_coords.count(hash) == 0)
				port_reserved_coords.insert(hash);
		} while (mr_nw.advance(map));
	}

	if (!port_reserved_coords.empty()) {
		seafaring_economy = true;
	}

	// here we scan entire map for own ships
	std::set<OPtr<Ship>> found_ships;
	for (int16_t y = 0; y < map.get_height(); ++y) {
		for (int16_t x = 0; x < map.get_width(); ++x) {
			FCoords f = map.get_fcoords(Coords(x, y));
			// there are too many bobs on the map so we investigate
			// only bobs on water
			if (f.field->nodecaps() & MOVECAPS_SWIM) {
				for (Bob* bob = f.field->get_first_bob(); bob; bob = bob->get_next_on_field()) {
					if (upcast(Ship, ship, bob)) {
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
	std::set<OPtr<PlayerImmovable>> found_immovables;
	for (int16_t y = 0; y < map.get_height(); ++y) {
		for (int16_t x = 0; x < map.get_width(); ++x) {
			FCoords f = map.get_fcoords(Coords(x, y));

			if (f.field->get_owned_by() != player_number()) {
				continue;
			}

			unusable_fields.push_back(f);

			if (upcast(PlayerImmovable, imm, f.field->get_immovable())) {
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
		if (ps_obs.bo->space_consumer && !ps_obs.bo->plants_trees) {
			MapRegion<Area<FCoords>> mr(
			   map, Area<FCoords>(map.get_fcoords(ps_obs.site->get_position()), 4));
			do {
				blocked_fields.add(mr.location(), game().get_gametime() + 20 * 60 * 1000);
			} while (mr.advance(map));
		}
	}

	// The data struct below is owned by Player object, the purpose is to have them saved therein
	persistent_data = player_->get_mutable_ai_persistent_state();
	management_data.pd = player_->get_mutable_ai_persistent_state();

	if (persistent_data->initialized == kFalse) {
		// As all data are initialized without given values, they must be populated with reasonable
		// values first
		persistent_data->colony_scan_area = kColonyScanStartArea;
		persistent_data->trees_around_cutters = 0;
		persistent_data->initialized = kTrue;
		persistent_data->last_attacked_player = std::numeric_limits<int16_t>::max();
		persistent_data->expedition_start_time = kNoExpedition;
		persistent_data->ships_utilization = 200;
		persistent_data->no_more_expeditions = kFalse;
		persistent_data->target_military_score = 0;
		persistent_data->least_military_score = 100;
		persistent_data->ai_personality_attack_margin = std::max(std::rand() % 20 - 5, 0);
		persistent_data->ai_productionsites_ratio = std::rand() % 5 + 7;
		persistent_data->ai_personality_wood_difference = std::rand() % 40 - 20;
		persistent_data->ai_personality_early_militarysites = std::rand() % 20 + 20;
		persistent_data->last_soldier_trained = kNever;
		persistent_data->ai_personality_mil_upper_limit = 0;  // std::rand() % 125 + 325; NOCOM

		// all zeroes
		assert(persistent_data->neuron_weights.size() == 0);
		assert(persistent_data->neuron_functs.size() == 0);
		assert(persistent_data->magic_numbers_size == 0);
		assert(persistent_data->neuron_pool_size == 0);
		assert(persistent_data->magic_numbers.size() == 0);

		// AI's DNA population
		management_data.initialize(player_number());

		printf(" magic numbers: %5d %5d %5lu\n", magic_numbers_size,
		       persistent_data->magic_numbers_size, persistent_data->magic_numbers.size());
		printf("  neurons: %5d %5d %5lu %5lu\n", neuron_pool_size, persistent_data->neuron_pool_size,
		       persistent_data->neuron_functs.size(), persistent_data->neuron_weights.size());

		management_data.test_consistency();

		if (player_number() == 2) {
			management_data.mutate(gametime);
		}
		printf(" magic numbers: %5d %5d %5lu\n", magic_numbers_size,
		       persistent_data->magic_numbers_size, persistent_data->magic_numbers.size());
		printf("  neurons: %5d %5d %5lu %5lu\n", neuron_pool_size, persistent_data->neuron_pool_size,
		       persistent_data->neuron_functs.size(), persistent_data->neuron_weights.size());

		management_data.test_consistency();

	} else if (persistent_data->initialized == kTrue) {
		// Doing some consistency checks
		check_range<uint32_t>(
		   persistent_data->expedition_start_time, gametime, "expedition_start_time");
		check_range<uint16_t>(persistent_data->ships_utilization, 0, 10000, "ships_utilization_");
		check_range<int32_t>(
		   persistent_data->ai_personality_attack_margin, 15, "ai_personality_attack_margin");
		check_range<uint32_t>(
		   persistent_data->ai_productionsites_ratio, 5, 15, "ai_productionsites_ratio");
		check_range<int32_t>(persistent_data->ai_personality_wood_difference, -20, 19,
		                     "ai_personality_wood_difference");
		check_range<uint32_t>(persistent_data->ai_personality_early_militarysites, 20, 40,
		                      "ai_personality_early_militarysites");

		printf("MN: %lu  %d %d\n", persistent_data->magic_numbers.size(),
		       persistent_data->magic_numbers_size, magic_numbers_size);
		printf("NP: %lu  %lu %d %d %lu\n", persistent_data->neuron_functs.size(),
		       persistent_data->neuron_weights.size(), persistent_data->neuron_pool_size,
		       neuron_pool_size, management_data.neuron_pool.size());

		assert(persistent_data->magic_numbers_size == magic_numbers_size);
		assert(persistent_data->neuron_pool_size == neuron_pool_size);
		assert(persistent_data->magic_numbers.size() == magic_numbers_size);
		assert(persistent_data->neuron_weights.size() == neuron_pool_size);
		assert(persistent_data->neuron_functs.size() == neuron_pool_size);
		assert(persistent_data->f_neurons.size() == f_neuron_pool_size);

		for (uint32_t i = 0; i < persistent_data->magic_numbers_size; i = i + 1) {
			management_data.set_military_number_at(i, persistent_data->magic_numbers[i]);
		}
		for (uint32_t i = 0; i < persistent_data->neuron_pool_size; i = i + 1) {
			management_data.neuron_pool.push_back(Neuron(persistent_data->neuron_weights[i],
			                                             persistent_data->neuron_functs[i],
			                                             management_data.new_neuron_id()));
		}

		for (uint32_t i = 0; i < persistent_data->f_neuron_pool_size; i = i + 1) {
			management_data.f_neuron_pool.push_back(FNeuron(persistent_data->f_neurons[i]));
		}

		management_data.test_consistency();
		management_data.dump_data();

	} else {
		throw wexception("Corrupted AI data");
	}

	update_player_stat();
	management_data.set_mutation_multiplicator(1);
	switch (type_) {
	case DefaultAI::Type::kNormal:
		management_data.set_mutation_multiplicator(30);
		break;
	case DefaultAI::Type::kWeak:
		management_data.set_mutation_multiplicator(5);
		break;
	case DefaultAI::Type::kVeryWeak:
		break;
	}
}

/**
 * Checks ALL available buildable fields.
 *
 * this shouldn't be used often, as it might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_buildable_fields(const uint32_t gametime) {
	uint16_t i = 0;

	// we test 40 fields that were update more than 1 seconds ago
	while (!buildable_fields.empty() &&
	       (buildable_fields.front()->field_info_expiration - kFieldInfoExpiration + 1000) <=
	          gametime &&
	       i < 40) {
		BuildableField& bf = *buildable_fields.front();

		//  check whether we lost ownership of the node
		if (bf.coords.field->get_owned_by() != player_number()) {
			delete &bf;
			buildable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player_->get_buildcaps(bf.coords) & BUILDCAPS_SIZEMASK) == 0) {
			unusable_fields.push_back(bf.coords);
			delete &bf;
			buildable_fields.pop_front();
			continue;
		}

		update_buildable_field(bf);
		bf.field_info_expiration = gametime + kFieldInfoExpiration;
		buildable_fields.push_back(&bf);
		buildable_fields.pop_front();

		i += 1;
	}
}

/**
 * Checks ALL available mineable fields.
 *
 * this shouldn't be used often, as it might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_mineable_fields(const uint32_t gametime) {

	uint16_t i = 0;  // counter, used to track # of checked fields

	// we test 30 fields that were updated more than 1 seconds ago
	// to avoid re-test of the same field twice
	while (!mineable_fields.empty() &&
	       (mineable_fields.front()->field_info_expiration - kMineFieldInfoExpiration + 1000) <=
	          gametime &&
	       i < 30) {
		MineableField* mf = mineable_fields.front();

		//  check whether we lost ownership of the node
		if (mf->coords.field->get_owned_by() != player_number()) {
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player_->get_buildcaps(mf->coords) & BUILDCAPS_MINE) == 0) {
			unusable_fields.push_back(mf->coords);
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		update_mineable_field(*mf);
		mf->field_info_expiration = gametime + kMineFieldInfoExpiration;
		mineable_fields.push_back(mf);
		mineable_fields.pop_front();

		i += 1;
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
		if (player_->get_buildcaps(unusable_fields.front()) & BUILDCAPS_SIZEMASK) {
			buildable_fields.push_back(new BuildableField(unusable_fields.front()));
			unusable_fields.pop_front();
			update_buildable_field(*buildable_fields.back());
			continue;
		}

		if (player_->get_buildcaps(unusable_fields.front()) & BUILDCAPS_MINE) {
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
	Map& map = game().map();
	const uint32_t gametime = game().get_gametime();
	FindNodeUnownedWalkable find_unowned_walkable(player_, game());
	FindNodeUnownedBuildable find_unowned_buildable(player_, game());
	FindNodeUnownedMineable find_unowned_mines_pots(player_, game());
	FindNodeAllyOwned find_ally(player_, game(), player_number());
	PlayerNumber const pn = player_->player_number();
	const World& world = game().world();

	const uint16_t production_area = 6;
	const uint16_t buildable_spots_check_area = 10;
	const uint16_t enemy_check_area = 16;
	const uint16_t distant_resources_area = 16;

	const bool verbose = false;

	field.unowned_land_nearby = map.find_fields(
	   Area<FCoords>(field.coords, enemy_check_area), nullptr, find_unowned_walkable);

	if (field.unowned_land_nearby > 0) {
		field.unowned_buildable_spots_nearby = map.find_fields(
		   Area<FCoords>(field.coords, buildable_spots_check_area), nullptr, find_unowned_buildable);
	} else {
		field.unowned_buildable_spots_nearby = 0;
	}

	// Is this near border  // get rid of allyownedfields
	if (map.find_fields(Area<FCoords>(field.coords, 3), nullptr, find_ally) ||
	    map.find_fields(Area<FCoords>(field.coords, 3), nullptr, find_unowned_walkable)) {
		field.near_border = true;
	} else {
		field.near_border = true;
	}

	// are we going to count resources now?
	bool resource_count_now = false;
	// Testing in first 10 seconds or if last testing was more then 60 sec ago
	if (field.last_resources_check_time < 10000 ||
	    field.last_resources_check_time - gametime > 60 * 1000) {
		resource_count_now = true;
		field.last_resources_check_time = gametime;
	}

	// testing mines
	if (resource_count_now) {
		uint32_t close_mines = map.find_fields(
		   Area<FCoords>(field.coords, production_area), nullptr, find_unowned_mines_pots);
		uint32_t distant_mines =
		   map.find_fields(Area<FCoords>(field.coords, distant_resources_area), nullptr,

		                   find_unowned_mines_pots);
		distant_mines = distant_mines - close_mines;
		field.unowned_mines_spots_nearby = 4 * close_mines + distant_mines / 2;
		if (distant_mines > 0) {
			field.unowned_mines_spots_nearby += 15;
		}
	}

	// identifying portspace fields
	if (field.is_portspace ==
	    Widelands::ExtendedBool::kUnset) {  // if we know it, no need to do it once more
		if (player_->get_buildcaps(field.coords) & BUILDCAPS_PORT) {
			field.is_portspace = ExtendedBool::kTrue;
		} else {
			field.is_portspace = ExtendedBool::kFalse;
		}
	}

	// testing for near portspaces
	if (field.portspace_nearby == Widelands::ExtendedBool::kUnset) {
		field.portspace_nearby = ExtendedBool::kFalse;
		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(field.coords, 4));
		do {
			if (port_reserved_coords.count(mr.location().hash()) > 0) {
				field.portspace_nearby = ExtendedBool::kTrue;
				break;
			}
		} while (mr.advance(map));
	}

	// testing if a port is nearby, such field will get a priority boost
	if (resource_count_now) {  // misusing a bit
		uint16_t nearest_distance = std::numeric_limits<uint16_t>::max();
		for (const WarehouseSiteObserver& wh_obs : warehousesites) {
			if (wh_obs.bo->is_port) {
				const uint16_t actual_distance =
				   map.calc_distance(field.coords, wh_obs.site->get_position());
				nearest_distance = std::min(nearest_distance, actual_distance);
			}
		}
		if (nearest_distance < 15) {
			field.port_nearby = true;
		} else {
			field.port_nearby = false;
		}
	}

	// testing fields in radius 1 to find biggest buildcaps.
	// This is to calculate capacity that will be lost if something is
	// built here
	field.max_buildcap_nearby = 0;
	MapRegion<Area<FCoords>> mr(map, Area<FCoords>(field.coords, 1));
	do {
		if ((player_->get_buildcaps(mr.location()) & BUILDCAPS_SIZEMASK) >
		    field.max_buildcap_nearby) {
			field.max_buildcap_nearby = player_->get_buildcaps(mr.location()) & BUILDCAPS_SIZEMASK;
		}
	} while (mr.advance(map));

	assert((player_->get_buildcaps(field.coords) & BUILDCAPS_SIZEMASK) <= field.max_buildcap_nearby);

	// Testing surface water (once only)
	if (field.water_nearby == kUncalculated) {
		assert(field.open_water_nearby == kUncalculated);

		FindNodeWater find_water(game().world());
		field.water_nearby =
		   map.find_fields(Area<FCoords>(field.coords, production_area), nullptr, find_water);

		if (field.water_nearby > 0) {
			FindNodeOpenWater find_open_water(game().world());
			field.open_water_nearby =
			   map.find_fields(Area<FCoords>(field.coords, production_area), nullptr, find_open_water);
		}

		if (resource_necessity_water_needed_) {  // for atlanteans
			field.distant_water = map.find_fields(Area<FCoords>(field.coords, distant_resources_area),
			                                      nullptr, find_water) -
			                      field.water_nearby;
			assert(field.open_water_nearby <= field.water_nearby);
		}
	}

	// counting fields with fish
	if (field.water_nearby > 0 && (field.fish_nearby == kUncalculated || resource_count_now)) {
		field.fish_nearby = map.find_fields(Area<FCoords>(field.coords, production_area), nullptr,
		                                    FindNodeResource(world.get_resource("fish")));
		;
	}

	// counting fields with critters (game)
	// not doing this always, this does not change fast
	if (resource_count_now) {

		field.critters_nearby =
		   map.find_bobs(Area<FCoords>(field.coords, production_area), nullptr, FindBobCritter());
		;
	}

	FCoords fse;
	map.get_neighbour(field.coords, WALK_SE, &fse);
	field.preferred = false;
	if (BaseImmovable const* const imm = fse.field->get_immovable()) {
		if (dynamic_cast<Flag const*>(imm) ||
		    (dynamic_cast<Road const*>(imm) && (fse.field->nodecaps() & BUILDCAPS_FLAG))) {
			field.preferred = true;
		}
	}

	// Rocks are not renewable, we will count them only if previous state is nonzero
	if (field.rocks_nearby > 0 && resource_count_now) {

		field.rocks_nearby =
		   map.find_immovables(Area<FCoords>(map.get_fcoords(field.coords), production_area), nullptr,
		                       FindImmovableAttribute(MapObjectDescr::get_attribute_id("rocks")));

		// adding 10 if rocks found
		field.rocks_nearby = (field.rocks_nearby > 0) ? field.rocks_nearby + 10 : 0;
	}

	// ground water is not renewable and its amount can only fall, we will count them only if
	// previous state is nonzero
	if (field.ground_water > 0 && resource_count_now) {
		field.ground_water = field.coords.field->get_resources_amount();
	}

	// Counting trees nearby
	if (resource_count_now) {
		int32_t const tree_attr = MapObjectDescr::get_attribute_id("tree");
		field.trees_nearby =
		   map.find_immovables(Area<FCoords>(map.get_fcoords(field.coords), production_area), nullptr,
		                       FindImmovableAttribute(tree_attr));
	}

	// resetting some values
	field.enemy_nearby = false;
	field.area_military_capacity = 0;
	field.own_military_presence = 0;
	field.ally_military_presence = 0;
	field.enemy_military_presence = 0;
	field.unconnected_nearby = false;

	field.space_consumers_nearby = 0;
	field.rangers_nearby = 0;
	field.producers_nearby.clear();
	field.producers_nearby.resize(wares.size());
	field.consumers_nearby.clear();
	field.consumers_nearby.resize(wares.size());
	field.supporters_nearby.clear();
	field.supporters_nearby.resize(wares.size());

	// collect information about productionsites nearby
	std::vector<ImmovableFound> immovables;
	// Search in a radius of range
	map.find_immovables(Area<FCoords>(field.coords, production_area + 2), &immovables);

	for (uint32_t i = 0; i < immovables.size(); ++i) {
		const BaseImmovable& base_immovable = *immovables.at(i).object;

		if (upcast(PlayerImmovable const, player_immovable, &base_immovable)) {

			// TODO(unknown): Only continue; if this is an opposing site
			// allied sites should be counted for military influence
			if (player_immovable->owner().player_number() != pn) {
				continue;
			}
			// here we identify the buiding (including expected building if constructionsite)
			// and calculate some statistics about nearby buildings
			if (upcast(ProductionSite const, productionsite, player_immovable)) {
				BuildingObserver& bo = get_building_observer(productionsite->descr().name().c_str());
				consider_productionsite_influence(field, immovables.at(i).coords, bo);
			}
			if (upcast(ConstructionSite const, constructionsite, player_immovable)) {
				const BuildingDescr& target_descr = constructionsite->building();
				BuildingObserver& bo = get_building_observer(target_descr.name().c_str());
				consider_productionsite_influence(field, immovables.at(i).coords, bo);
			}
		}
	}

	field.military_stationed = 0;
	field.military_unstationed = 0;
	field.military_in_constr_nearby = 0;
	field.military_loneliness = 1000;
	field.unconnected_nearby = false;

	// Now testing military aspects
	immovables.clear();
	map.find_immovables(Area<FCoords>(field.coords, enemy_check_area), &immovables);

	// We are interested in unconnected immovables, but we must be also close to connected ones
	bool any_connected_imm = false;
	bool any_unconnected_imm = false;

	for (uint32_t i = 0; i < immovables.size(); ++i) {

		const BaseImmovable& base_immovable = *immovables.at(i).object;

		// testing if immovable is owned by someone else and collecting some statistics
		if (upcast(Building const, building, &base_immovable)) {

			const PlayerNumber bpn = building->owner().player_number();
			if (bpn == pn) {
				;
			} else if (!player_statistics.players_in_same_team(bpn, pn)) {  // it is enemy
				assert(player_statistics.get_is_enemy(bpn));
				field.enemy_nearby = true;
				if (upcast(MilitarySite const, militarysite, building)) {
					field.enemy_military_presence += militarysite->stationed_soldiers().size();
				}
				continue;
			} else {  // it is ally
				assert(!player_statistics.get_is_enemy(bpn));
				if (upcast(MilitarySite const, militarysite, building)) {
					field.ally_military_presence += militarysite->stationed_soldiers().size();
				}
				continue;
			}
		}

		// if we are here, immovable is ours
		if (upcast(Building const, building, &base_immovable)) {

			assert(building->owner().player_number() == pn);

			// connected to warehouse
			bool connected = !building->get_economy()->warehouses().empty();
			if (connected) {
				any_connected_imm = true;
			}

			if (upcast(ConstructionSite const, constructionsite, building)) {
				const BuildingDescr& target_descr = constructionsite->building();

				if (upcast(MilitarySiteDescr const, target_ms_d, &target_descr)) {
					const int32_t dist = map.calc_distance(field.coords, immovables.at(i).coords);
					const int32_t radius = target_ms_d->get_conquers() + 4;

					if (radius > dist) {
						field.area_military_capacity += target_ms_d->get_max_number_of_soldiers() / 2 + 1;
						if (field.coords != immovables.at(i).coords) {
							field.military_loneliness *= static_cast<double_t>(dist) / radius;
						}
						field.military_in_constr_nearby += 1;
					}
				}
			} else if (!connected) {
				// we dont care about unconnected constructionsites
				any_unconnected_imm = true;
			}

			if (upcast(MilitarySite const, militarysite, building)) {
				const int32_t dist = map.calc_distance(field.coords, immovables.at(i).coords);
				const int32_t radius = militarysite->descr().get_conquers() + 4;

				if (radius > dist) {

					field.area_military_capacity += militarysite->max_soldier_capacity();
					field.own_military_presence += militarysite->stationed_soldiers().size();

					if (militarysite->stationed_soldiers().empty()) {
						field.military_unstationed += 1;
					} else {
						field.military_stationed += 1;
					}

					if (field.coords != immovables.at(i).coords) {
						field.military_loneliness *= static_cast<double_t>(dist) / radius;
					}
				}
			}
		}
	}
	if (any_unconnected_imm && any_connected_imm && field.military_in_constr_nearby == 0) {
		field.unconnected_nearby = true;
	}

	// if ((field.enemy_military_presence + field.ally_military_presence)  > 15 )printf ("bf %3dx%3d:
	// presence own: %3d, enemy: %3d, ally %3d\n",
	// field.coords.x,
	// field.coords.y,
	// field.own_military_presence,
	// field.enemy_military_presence,
	// field.ally_military_presence);

	// if there is a militarysite on field, we try to walk to enemy
	field.enemy_accessible_ = false;
	field.local_soldier_capacity = 0;
	field.is_militarysite = false;
	if (upcast(MilitarySite, ms, field.coords.field->get_immovable())) {
		if (field.enemy_nearby) {
			uint32_t unused1 = 0;
			uint16_t unused2 = 0;
			field.enemy_accessible_ = other_player_accessible(
			   enemy_check_area + 3, &unused1, &unused2, field.coords, WalkSearch::kEnemy);
		}
		field.local_soldier_capacity = ms->max_soldier_capacity();
		if (verbose)
			printf(" %3dx%3d - militarysite here\n", field.coords.x, field.coords.y);
		field.is_militarysite = true;
	}

	// Calculating field score
	field.military_score_ = 0;
	field.inland = false;

	if (!(field.enemy_nearby || field.near_border)) {
		field.inland = true;
	}

	// score here is a compound of various input values
	// usually resources in vicinity, but when enemy is nearby
	// additional bonus is added

	if (!field.is_militarysite) {
		field.military_score_ += 100;
		field.military_score_ -=
		   field.military_in_constr_nearby * std::abs(management_data.get_military_number_at(51)) / 2;
		if (field.enemy_nearby) {
			// speciic for enemy_nearby
			// field.military_score_  += 50;
			field.military_score_ += management_data.get_military_number_at(0);
			field.military_score_ += std::abs(management_data.get_military_number_at(39));
			field.military_score_ -= management_data.neuron_pool[0].get_result_safe(
			   field.military_loneliness / 50, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[4].get_result_safe(
			   (field.area_military_capacity + 4) / 5, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[8].get_result_safe(
			   (field.military_in_constr_nearby + field.military_unstationed) * 3, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[18].get_result_safe(
			   static_cast<uint8_t>(soldier_status_) * 3, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[53].get_result_safe(
			   field.ally_military_presence, kAbsValue);
			field.military_score_ += management_data.neuron_pool[54].get_result_safe(
			   field.enemy_military_presence * 2, kAbsValue);
			field.military_score_ += management_data.neuron_pool[61].get_result_safe(
			   field.enemy_military_presence / 3, kAbsValue);

			if (management_data.f_neuron_pool[12].get_result(
			       mines_.size() < 4, field.unowned_mines_spots_nearby < 5,
			       persistent_data->last_soldier_trained < gametime &&
			          persistent_data->last_soldier_trained + 20 * 60 * 1000 < gametime)) {
				field.military_score_ -= std::abs(management_data.get_military_number_at(58));
			}

		} else if (field.near_border) {
			// near border, but not enemy nearby
			field.military_score_ += management_data.get_military_number_at(1);
			field.military_score_ -= management_data.neuron_pool[1].get_result_safe(
			   field.military_loneliness / 40, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[5].get_result_safe(
			   (field.area_military_capacity + 4) / 5, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[9].get_result_safe(
			   (field.military_in_constr_nearby + field.military_unstationed) * 3, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[36].get_result_safe(
			   static_cast<uint8_t>(soldier_status_) * 3, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[58].get_result_safe(
			   field.ally_military_presence, kAbsValue);
		} else if (field.inland) {
			// inland
			field.military_score_ += management_data.get_military_number_at(2);
			field.military_score_ -= management_data.neuron_pool[2].get_result_safe(
			   field.military_loneliness / 40, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[6].get_result_safe(
			   (field.area_military_capacity + 4) / 5, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[10].get_result_safe(
			   (field.military_in_constr_nearby + field.military_unstationed) * 10, kAbsValue);
			field.military_score_ -= management_data.neuron_pool[37].get_result_safe(
			   static_cast<uint8_t>(soldier_status_) * 3, kAbsValue);
		} else {
			NEVER_HERE();
		}
		field.military_score_ -=
		   management_data.neuron_pool[13].get_result_safe(field.military_stationed, kAbsValue);
		field.military_score_ -=
		   management_data.neuron_pool[15].get_result_safe(field.military_stationed / 2, kAbsValue);
	} else {  // militarysite
		// field.military_score_ -= 100;
		field.military_score_ += management_data.get_military_number_at(3);
		field.military_score_ += std::abs(management_data.get_military_number_at(59));
		field.military_score_ -=
		   field.military_in_constr_nearby * std::abs(management_data.get_military_number_at(52)) / 5;
		if (field.enemy_accessible_) {
			field.military_score_ += std::abs(management_data.get_military_number_at(11) / 2);
		} else if (field.near_border) {
			field.military_score_ += std::abs(management_data.get_military_number_at(12));
		} else {
			field.military_score_ += management_data.get_military_number_at(9);
		}
		field.military_score_ -=
		   management_data.neuron_pool[3].get_result_safe(field.military_loneliness / 40, kAbsValue);
		field.military_score_ -= management_data.neuron_pool[7].get_result_safe(
		                            (field.area_military_capacity + 4) / 5, kAbsValue) /
		                         2;
		field.military_score_ -=
		   management_data.neuron_pool[11].get_result_safe(
		      (field.military_in_constr_nearby + field.military_unstationed) * 3, kAbsValue) /
		   2;
		field.military_score_ -=
		   management_data.neuron_pool[20].get_result_safe(field.military_stationed, kAbsValue);
		field.military_score_ -=
		   management_data.neuron_pool[17].get_result_safe(field.military_stationed / 2, kAbsValue);
		field.military_score_ -= management_data.neuron_pool[38].get_result_safe(
		   static_cast<uint8_t>(soldier_status_) * 3, kAbsValue);
		field.military_score_ -=
		   management_data.neuron_pool[28].get_result_safe(
		      field.area_military_capacity - field.local_soldier_capacity, kAbsValue) /
		   2;
		field.military_score_ -=
		   management_data.neuron_pool[55].get_result_safe(field.ally_military_presence, kAbsValue);
		field.military_score_ += management_data.neuron_pool[56].get_result_safe(
		   field.enemy_military_presence * 2, kAbsValue);
		field.military_score_ += management_data.neuron_pool[59].get_result_safe(
		   field.enemy_military_presence / 3, kAbsValue);
	}

	// generally
	field.military_score_ +=
	   field.unconnected_nearby * std::abs(management_data.get_military_number_at(4));
	const int32_t land_score = management_data.neuron_pool[21].get_result_safe(
	                              (field.unowned_land_nearby + 8) / 9, kAbsValue) *
	                           resource_necessity_territory_ / 100;
	assert(land_score >= 0 && land_score <= 100);
	// printf ("Land score: %3d\n", land_score);
	field.military_score_ += land_score;

	field.military_score_ +=
	   field.unowned_land_nearby * std::abs(management_data.get_military_number_at(50)) / 50;

	field.military_score_ += field.unowned_buildable_spots_nearby *
	                         std::abs(management_data.get_military_number_at(55)) / 20;
	field.military_score_ += management_data.neuron_pool[62].get_result_safe(
	                            field.unowned_buildable_spots_nearby, kAbsValue) /
	                         3;

	const int32_t mines_score = management_data.neuron_pool[22].get_result_safe(
	                               (field.unowned_mines_spots_nearby + 9) / 10, kAbsValue) *
	                            resource_necessity_mines_ / 100;
	assert(mines_score >= 0 && mines_score <= 100);
	field.military_score_ -= management_data.neuron_pool[12].get_result_safe(
	   field.own_military_sites_nearby_(), kAbsValue);
	field.military_score_ -= management_data.neuron_pool[14].get_result_safe(
	   field.own_military_sites_nearby_() / 4, kAbsValue);

	field.military_score_ += mines_score;
	field.military_score_ +=
	   field.preferred * std::abs(management_data.get_military_number_at(8) / 10);
	field.military_score_ +=
	   field.unconnected_nearby * management_data.get_military_number_at(10) / 10;

	if (!field.inland) {
		field.military_score_ +=
		   management_data.neuron_pool[24].get_result_safe((field.rocks_nearby + 2) / 3, kAbsValue);
		field.military_score_ +=
		   management_data.neuron_pool[25].get_result_safe((field.water_nearby + 2) / 2, kAbsValue);
		field.military_score_ +=
		   resource_necessity_water_needed_ *
		   management_data.neuron_pool[26].get_result_safe((field.distant_water + 4) / 5, kAbsValue);
		field.military_score_ +=
		   management_data.neuron_pool[27].get_result_safe(field.trees_nearby / 2, kAbsValue);
	}

	if (field.portspace_nearby == ExtendedBool::kTrue) {
		if (num_ports == 0) {
			field.military_score_ += std::abs(management_data.get_military_number_at(5) / 3);
		} else {
			field.military_score_ += 25;
		}
	}

	// sometimes expansion is stalled and this is to help boost it
	if (msites_in_constr() == 0 && soldier_status_ == SoldiersStatus::kFull) {
		field.military_score_ += std::abs(management_data.get_military_number_at(6));

		if (field.enemy_nearby) {
			field.military_score_ += std::abs(management_data.get_military_number_at(7));
		}
	}

	if (field.is_militarysite && verbose)
		printf("actual score: %4d b\n", field.military_score_);

	avg_military_score_ = (avg_military_score_ * 99 + field.military_score_) / 100;
	// printf (" %d: %3dx%3d: military_score: %6d, avg: %6d\n",
	// player_number(),
	// field.coords.x,
	// field.coords.y,
	// field.military_score_,
	// avg_military_score_);
}

/// Updates one mineable field
void DefaultAI::update_mineable_field(MineableField& field) {
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	Map& map = game().map();
	map.find_immovables(Area<FCoords>(field.coords, 5), &immovables);
	field.preferred = false;
	field.mines_nearby = 0;
	FCoords fse;
	map.get_brn(field.coords, &fse);

	if (BaseImmovable const* const imm = fse.field->get_immovable()) {
		if (dynamic_cast<Flag const*>(imm) ||
		    (dynamic_cast<Road const*>(imm) && (fse.field->nodecaps() & BUILDCAPS_FLAG))) {
			field.preferred = true;
		}
	}

	for (const ImmovableFound& temp_immovable : immovables) {
		if (upcast(Building const, bld, temp_immovable.object)) {
			if (player_number() != bld->owner().player_number()) {
				continue;
			}
			if (bld->descr().get_ismine()) {
				if (get_building_observer(bld->descr().name().c_str()).mines ==
				    field.coords.field->get_resources()) {
					++field.mines_nearby;
				}
			} else if (upcast(ConstructionSite const, cs, bld)) {
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
		   map.find_fields(Area<FCoords>(field.coords, 4), nullptr, find_mines_spots_nearby);
	}
}

/// Updates the production and MINE sites statistics needed for construction decision.
void DefaultAI::update_productionsite_stats() {
	uint16_t fishers_count = 0;  // used for atlanteans only

	// Reset statistics for all buildings
	for (uint32_t i = 0; i < buildings_.size(); ++i) {
		buildings_.at(i).current_stats = 0;
		buildings_.at(i).unoccupied_count = 0;
		buildings_.at(i).unconnected_count = 0;
	}

	// Check all available productionsites
	for (uint32_t i = 0; i < productionsites.size(); ++i) {
		assert(productionsites.front().bo->cnt_built > 0);
		// is connected
		const bool connected_to_wh =
		   !productionsites.front().site->get_economy()->warehouses().empty();

		// unconnected buildings are excluded from statistics review
		if (connected_to_wh) {
			// Add statistics value
			productionsites.front().bo->current_stats +=
			   productionsites.front().site->get_crude_statistics();

			// counting fishers
			if (productionsites.front().bo->is_fisher) {
				fishers_count += 1;
			}

			// Check whether this building is completely occupied
			if (!productionsites.front().site->can_start_working()) {
				productionsites.front().bo->unoccupied_count += 1;
			}
		} else {
			productionsites.front().bo->unconnected_count += 1;
		}

		// Now reorder the buildings
		productionsites.push_back(productionsites.front());
		productionsites.pop_front();
	}

	if (resource_necessity_water_needed_) {
		resource_necessity_water_ =
		   management_data.neuron_pool[35].get_result_safe(fishers_count * 4, kAbsValue);
		assert(resource_necessity_water_ >= 0 && resource_necessity_water_ <= 100);
	}

	// for mines_ also
	// Check all available mines
	for (uint32_t i = 0; i < mines_.size(); ++i) {
		assert(mines_.front().bo->cnt_built > 0);

		const bool connected_to_wh = !mines_.front().site->get_economy()->warehouses().empty();

		// unconnected mines are excluded from statistics review
		if (connected_to_wh) {
			// Add statistics value
			mines_.front().bo->current_stats += mines_.front().site->get_statistics_percent();
			// Check whether this building is completely occupied
			if (!mines_.front().site->can_start_working()) {
				mines_.front().bo->unoccupied_count += 1;
			}
		} else {
			mines_.front().bo->unconnected_count += 1;
		}

		// Now reorder the buildings
		mines_.push_back(mines_.front());
		mines_.pop_front();
	}

	// Scale statistics down
	for (uint32_t i = 0; i < buildings_.size(); ++i) {
		if ((buildings_.at(i).cnt_built - buildings_.at(i).unconnected_count) > 0) {
			buildings_.at(i).current_stats /=
			   (buildings_.at(i).cnt_built - buildings_.at(i).unconnected_count);
		}
	}
}

// * Constructs the most needed building
//   algorithm goes over all available spots and all allowed buildings,
//   scores every combination and one with highest and positive score
//   is built.
// * Buildings are split into categories
// * The logic is complex but approximately:
// - buildings producing building material are preferred
// - buildings identified as basic are preferred
// - first bulding of a type is preferred
// - buildings identified as 'direct food supplier' are built after 15 min.
//   from game start
// - if a building is upgradeable, second building is also preferred
//   (there should be no upgrade when there are not two buildings of the same type)
// - algorithm is trying to take into account actual utlization of buildings
//   (the one shown in GUI/game is not reliable, it calculates own statistics)
// * military buildings have own strategy, split into two situations:
// - there is no enemy
// - there is an enemy
//   Currently more military buildings are built than needed
//   and "optimization" (dismantling not needed buildings) is done afterwards
bool DefaultAI::construct_building(uint32_t gametime) {
	if (buildable_fields.empty()) {
		return false;
	}

	// Just used for easy checking whether a mine or something else was built.
	bool mine = false;
	uint32_t consumers_nearby_count = 0;
	std::vector<int32_t> spots_avail;
	spots_avail.resize(4);
	Map& map = game().map();

	for (int32_t i = 0; i < 4; ++i)
		spots_avail.at(i) = 0;

	for (std::list<BuildableField*>::iterator i = buildable_fields.begin();
	     i != buildable_fields.end(); ++i)
		++spots_avail.at((*i)->coords.field->nodecaps() & BUILDCAPS_SIZEMASK);

	spots_ = spots_avail.at(BUILDCAPS_SMALL);
	spots_ += spots_avail.at(BUILDCAPS_MEDIUM);
	spots_ += spots_avail.at(BUILDCAPS_BIG);

	// here we possible stop building of new buildings
	new_buildings_stop_ = false;

	// helper variable - we need some proportion of free spots vs productionsites
	// the proportion depends on size of economy
	// this proportion defines how dense the buildings will be
	// it is degressive (allows high density on the beginning)
	int32_t needed_spots = 0;
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

	// This is a replacement for simple count of mines
	const int32_t virtual_mines = mines_.size() + mineable_fields.size() / 25;

	// *_military_scores are used as minimal score for a new military building
	// to be built. As AI does not traverse all building fields at once, these thresholds
	// are gradually going down until it finds a field&building that are above threshold
	// and this combination is used...
	// least_military_score is hardlimit, floating very slowly
	// target_military_score is always set according to latest best building (using the same
	// score) and quickly falling down until it reaches the least_military_score
	// this one (=target_military_score) is actually used to decide if building&field is allowed
	// candidate
	// least_military_score is allowed to get bellow 100 only if there is no military site in
	// construction
	// right now in order to (try to) avoid expansion lockup

	// Bools below are helpers to improve readability of code

	const PlayerNumber pn = player_number();

	const bool too_many_ms_constructionsites =
	   (pow(msites_in_constr(), 2) > militarysites.size() + 2);

	// For new building stop
	const bool nbs_1 = management_data.f_neuron_pool[13].get_result(
	   highest_nonmil_prio_ > 18 + management_data.get_military_number_at(29) / 10, has_enough_space,
	   virtual_mines >= 5,
	   player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn),
	   (num_prod_constructionsites + mines_in_constr()) >
	      (productionsites.size() + mines_built()) / persistent_data->ai_productionsites_ratio + 2);

	const bool nbs_2 = management_data.f_neuron_pool[14].get_result(
	   gametime > 45 * 60 * 1000, mines_.size() < 2, needs_boost_economy,
	   too_many_ms_constructionsites, player_statistics.any_enemy_seen_lately(gametime));

	const bool nbs_3 = management_data.f_neuron_pool[15].get_result(
	   virtual_mines >= 5, too_many_ms_constructionsites, has_enough_space,
	   player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn),
	   virtual_mines >= 5);

	needs_boost_economy = management_data.f_neuron_pool[16].get_result(nbs_1, nbs_2, nbs_3);

	// For new military score limit
	const bool ms_1 = management_data.f_neuron_pool[17].get_result(
	   highest_nonmil_prio_ > 18 + management_data.get_military_number_at(65) / 10, has_enough_space,
	   virtual_mines >= 5,
	   player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn),
	   (num_prod_constructionsites + mines_in_constr()) >
	      (productionsites.size() + mines_built()) / persistent_data->ai_productionsites_ratio + 2);

	const bool ms_2 = management_data.f_neuron_pool[18].get_result(
	   gametime > 45 * 60 * 1000, mines_.size() < 2, needs_boost_economy,
	   too_many_ms_constructionsites, player_statistics.any_enemy_seen_lately(gametime));

	const bool ms_3 = management_data.f_neuron_pool[19].get_result(
	   virtual_mines >= 5, too_many_ms_constructionsites, has_enough_space,
	   player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn),
	   virtual_mines >= 5);

	const bool increase_score_limit = management_data.f_neuron_pool[20].get_result(ms_1, ms_2, ms_3);

	// resetting highest_nonmil_prio_ so it can be recalculated anew
	highest_nonmil_prio_ = 0;

	const int32_t kBottomLimit =
	   std::abs(management_data.get_military_number_at(67));  // is this variable needed now?
	// modifying least_military_score, down if more military sites are needed and vice versa

	if (increase_score_limit) {
		if (persistent_data->least_military_score <
		    persistent_data
		       ->ai_personality_mil_upper_limit) {  // No sense in letting it grow too high
			persistent_data->least_military_score += 20;
			persistent_data->least_military_score = std::min(
			   persistent_data->ai_personality_mil_upper_limit, persistent_data->least_military_score);
		}
	} else {
		// least_military_score is decreased, but depending on the size of territory
		switch (static_cast<uint32_t>(log10(buildable_fields.size()))) {
		case 0:
			persistent_data->least_military_score -= 10;
			break;
		case 1:
			persistent_data->least_military_score -= 8;
			break;
		case 2:
			persistent_data->least_military_score -= 5;
			break;
		case 3:
			persistent_data->least_military_score -= 3;
			break;
		default:
			persistent_data->least_military_score -= 2;
		}
		// do not get bellow kBottomLimit if there is at least one ms in construction

		if ((msites_in_constr() > 0 || soldier_status_ != SoldiersStatus::kFull) &&
		    persistent_data->least_military_score < kBottomLimit) {
			persistent_data->least_military_score = kBottomLimit;
		}
		if (persistent_data->least_military_score < 0) {
			persistent_data->least_military_score = 0;
		}
	}

	// This is effective score, falling down very quickly
	if (persistent_data->target_military_score >
	    persistent_data->ai_personality_mil_upper_limit + 150) {
		persistent_data->target_military_score = 8 * persistent_data->target_military_score / 10;
	} else {
		persistent_data->target_military_score = 9 * persistent_data->target_military_score / 10;
	}
	if (persistent_data->target_military_score < persistent_data->least_military_score) {
		persistent_data->target_military_score = persistent_data->least_military_score;
	}

	// Under some circumstances we prohibit new productionsites
	new_buildings_stop_ = management_data.f_neuron_pool[8].get_result(
	   (num_prod_constructionsites + mines_in_constr()) >
	      (productionsites.size() + mines_built()) / persistent_data->ai_productionsites_ratio + 2,
	   has_enough_space, (num_prod_constructionsites + productionsites.size()) >
	                        (msites_in_constr() + militarysites.size()) *
	                           (5 + std::abs(management_data.get_military_number_at(28)) / 40),
	   mines_.size() < 2);

	// we must calculate wood policy
	const DescriptionIndex wood_index = tribe_->safe_ware_index("log");
	// stocked wood is to be in some propotion to productionsites and
	// constructionsites (this proportion is bit artifical, or we can say
	// it is proportion to the size of economy). Plus some positive 'margin'
	const int32_t stocked_wood_margin = calculate_stocklevel(wood_index) -
	                                    productionsites.size() * 2 - num_prod_constructionsites +
	                                    persistent_data->ai_personality_wood_difference;
	if (gametime < 15 * 60 * 1000) {
		wood_policy_ = WoodPolicy::kAllowRangers;
	} else if (stocked_wood_margin > 80) {
		wood_policy_ = WoodPolicy::kDismantleRangers;
	} else if (stocked_wood_margin > 25) {
		wood_policy_ = WoodPolicy::kStopRangers;
	} else {
		wood_policy_ = WoodPolicy::kAllowRangers;
	}

	// we must consider need for mines
	// set necessity for mines
	// we use 'virtual mines', because also mine spots can be changed
	// to mines when AI decides so
	resource_necessity_mines_ =
	   100 - management_data.neuron_pool[34].get_result_safe(virtual_mines * 2, kAbsValue);
	assert(resource_necessity_mines_ >= 0 && resource_necessity_mines_ <= 100);

	// here we calculate how badly we need to expand, result is number (25-100)
	// like a percent
	resource_necessity_territory_ =
	   25 +
	   (100 -
	    management_data.neuron_pool[31].get_result_safe(productionsites.size() / 2, kAbsValue) +
	    100 - management_data.neuron_pool[32].get_result_safe(BUILDCAPS_BIG, kAbsValue) + 100 -
	    management_data.neuron_pool[33].get_result_safe(spots_ / 20, kAbsValue)) /
	      4;
	assert(resource_necessity_territory_ >= 25 && resource_necessity_territory_ <= 100);
	// printf ("resource_necessity_territory_: %3d\n", resource_necessity_territory_ );

	BuildingObserver* best_building = nullptr;
	int32_t proposed_priority = 0;
	Coords proposed_coords;

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
		      bo.type == BuildingObserver::Type::kTrainingsite))
			continue;

		// and neither for small military buildings
		if (bo.type == BuildingObserver::Type::kMilitarysite &&
		    bo.desc->get_size() == BaseImmovable::SMALL)
			continue;

		bo.build_material_shortage = false;

		// checking we have enough critical material on stock
		for (uint32_t m = 0; m < bo.critical_building_material.size(); ++m) {
			DescriptionIndex wt(static_cast<size_t>(bo.critical_building_material.at(m)));
			uint32_t treshold = 3;
			// generally trainingsites are more important
			if (bo.type == BuildingObserver::Type::kTrainingsite) {
				treshold = 2;
			}

			if (calculate_stocklevel(wt) < treshold) {
				bo.build_material_shortage = true;
				break;
			}
		}
	}

	// Calculating actual needness
	for (uint32_t j = 0; j < buildings_.size(); ++j) {
		BuildingObserver& bo = buildings_.at(j);

		if (!bo.buildable(*player_)) {
			bo.new_building = BuildingNecessity::kNotNeeded;
		} else if (bo.type == BuildingObserver::Type::kProductionsite ||
		           bo.type == BuildingObserver::Type::kMine) {

			bo.new_building = check_building_necessity(bo, PerfEvaluation::kForConstruction, gametime);

			if (bo.is_shipyard) {
				assert(bo.new_building == BuildingNecessity::kAllowed ||
				       bo.new_building == BuildingNecessity::kForbidden);
			}

			if (bo.new_building == BuildingNecessity::kAllowed) {
				bo.new_building_overdue = 0;
			}

			// Now verifying that all 'buildable' buildings has positive max_needed_preciousness
			// if they have outputs, all other must have zero max_needed_preciousness
			if ((bo.new_building == BuildingNecessity::kNeeded ||
			     bo.new_building == BuildingNecessity::kForced ||
			     bo.new_building == BuildingNecessity::kAllowed ||
			     bo.new_building == BuildingNecessity::kNeededPending) &&
			    !bo.outputs.empty()) {
				if (bo.max_needed_preciousness <= 0) {
					throw wexception("AI: Max presciousness must not be <= 0 for building: %s",
					                 bo.desc->name().c_str());
				}
			} else if (bo.new_building == BuildingNecessity::kForbidden) {
				bo.max_needed_preciousness = 0;
			} else {
				// For other situations we make sure max_needed_preciousness is zero
				assert(bo.max_needed_preciousness == 0);
			}

			// Positive max_needed_preciousness says a building type is needed
			// here we increase or reset the counter
			// The counter is added to score when considering new building
			if (bo.max_needed_preciousness > 0) {
				bo.new_building_overdue += 1;
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
					bo.primary_priority = bo.max_needed_preciousness;
				} else {
					bo.primary_priority = bo.max_needed_preciousness +
					                      bo.max_needed_preciousness * bo.new_building_overdue / 100 +
					                      bo.new_building_overdue / 20;
				}
			} else {
				bo.primary_priority = 0;
			}

		} else if (bo.type == BuildingObserver::Type::kMilitarysite) {
			bo.new_building = check_building_necessity(bo, gametime);
			// if (bo.new_building != BuildingNecessity::kAllowed && msites_in_constr() == 0 &&
			// (gametime - military_last_build_) > 60 * 1000){
			// printf (" %d: msite (size: %d) not allowed, in constr: 0, last one %5d s ago, s.status:
			// %d\n", //NOCOM
			// player_number(),
			// bo.desc->get_size(),
			//(gametime - military_last_build_) / 1000,
			// soldier_status_);
			//}
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
	}

	// first scan all buildable fields for regular buildings
	for (std::list<BuildableField*>::iterator i = buildable_fields.begin();
	     i != buildable_fields.end(); ++i) {
		BuildableField* const bf = *i;

		if (bf->field_info_expiration < gametime) {
			continue;
		}

		// Continue if field is blocked at the moment
		if (blocked_fields.is_blocked(bf->coords)) {
			continue;
		}

		assert(player_);
		int32_t const maxsize = player_->get_buildcaps(bf->coords) & BUILDCAPS_SIZEMASK;

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

			// testing for reserved ports
			if (!bo.is_port) {
				if (port_reserved_coords.count(bf->coords.hash()) > 0) {
					continue;
				}
			}

			if (time(nullptr) % 3 == 0 && bo.total_count() > 0) {
				continue;
			}  // add randomnes and ease AI

			if (bo.type == BuildingObserver::Type::kMine) {
				continue;
			}

			// here we do an exemption for lumberjacks, mainly in early stages of game
			// sometimes the first one is not built and AI waits too long for second attempt
			if (gametime - bo.construction_decision_time < kBuildingMinInterval && !bo.need_trees) {
				continue;
			}

			if (!(bo.type == BuildingObserver::Type::kMilitarysite) &&
			    bo.cnt_under_construction >= 2) {
				continue;
			}

			int32_t prio = 0;  // score of a bulding on a field

			if (bo.type == BuildingObserver::Type::kProductionsite) {

				prio = management_data.neuron_pool[44].get_result_safe(bf->military_score_ / 20) / 5;
				assert(prio >= -20 && prio <= 20);

				// this can be only a well (as by now)
				if (bo.mines_water) {

					if (bo.new_building == BuildingNecessity::kForced) {
						assert(bo.total_count() - bo.unconnected_count == 0);
					}

					if (bf->ground_water < 2) {
						continue;
					}

					prio = bo.primary_priority;

					// keep wells more distant
					if (bf->producers_nearby.at(bo.outputs.at(0)) > 2) {
						continue;
					}

					// one well is forced
					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 200;
					}

					prio += bf->ground_water - 2;

				} else if (bo.need_trees) {  // LUMBERJACS

					prio = bo.primary_priority;

					prio += -20 + 200 / (bo.total_count() + 1);

					if (bo.new_building == BuildingNecessity::kForced) {
						prio *= 2;
					} else if (management_data.f_neuron_pool[32].get_result(
					              bf->trees_nearby > 25, bf->producers_nearby.at(bo.outputs.at(0)) >=
					                                        bf->supporters_nearby.at(bo.outputs.at(0)),
					              bo.total_count() >= 2, new_buildings_stop_, true)) {
						continue;
					}

					prio -= management_data.neuron_pool[57].get_result_safe(
					           get_stocklevel(bo, gametime) / 2, kAbsValue) /
					        5;

					// consider cutters and rangers nearby
					prio -= bf->producers_nearby.at(bo.outputs.at(0)) *
					        std::abs(management_data.get_military_number_at(25) / 3);
					prio += bf->supporters_nearby.at(bo.outputs.at(0)) * 5;

					prio += std::abs(management_data.get_military_number_at(26) / 20) * bf->trees_nearby;

				} else if (bo.need_rocks) {

					// Quarries are generally to be built everywhere where rocks are
					// no matter the need for granite, as rocks are considered an obstacle
					// to expansion
					prio = 2 * bf->rocks_nearby;

					if (bf->rocks_nearby > 0 && bf->near_border) {
						prio += management_data.get_military_number_at(27) / 5;
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

					// to prevent to many quaries on one spot
					prio = prio - 50 * bf->producers_nearby.at(bo.outputs.at(0));

				} else if (bo.is_hunter) {

					if (bf->critters_nearby < 5) {
						continue;
					}

					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 20;
					}

					// Overdue priority here
					prio += bo.primary_priority;

					prio += bf->supporters_nearby.at(bo.outputs.at(0)) * 5;

					prio +=
					   (bf->critters_nearby * 3) - 8 - 5 * bf->producers_nearby.at(bo.outputs.at(0));

				} else if (bo.is_fisher) {  // fisher

					if (bf->water_nearby < 2 || bf->fish_nearby < 2) {
						continue;
					}

					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 20;
					}

					// Overdue priority here
					prio += bo.primary_priority;

					prio -= bf->producers_nearby.at(bo.outputs.at(0)) * 20;
					prio += bf->supporters_nearby.at(bo.outputs.at(0)) * 10;

					prio += -5 + bf->fish_nearby;

				} else if (bo.production_hint >= 0) {
					if (bo.plants_trees) {
						assert(bo.cnt_target > 0);
					} else {
						bo.cnt_target =
						   1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 50;
					}

					// They have no own primary priority
					assert(bo.primary_priority == 0);

					if (bo.plants_trees) {  // RANGERS

						assert(bo.new_building == BuildingNecessity::kNeeded);

						// if there are too many trees nearby
						// if (bf->trees_nearby > 25 && bo.total_count() >= 1) {
						// continue;
						//}

						if (management_data.f_neuron_pool[3].get_result(
						       bf->trees_nearby > 25, bf->near_border,
						       persistent_data->trees_around_cutters >= 50, new_buildings_stop_,
						       bo.cnt_target > bo.total_count())) {
							continue;
						}

						prio = 0;

						prio += std::abs(management_data.f_neuron_pool[2].get_result(
						                    spots_<(4 * bo.total_count()), bo.total_count() <= 1,
						                           get_stocklevel(bo, gametime) < 2, new_buildings_stop_,
						                           persistent_data->trees_around_cutters> 40) *
						                 std::abs(management_data.get_military_number_at(36)) / 2);

						prio +=
						   std::abs(management_data.f_neuron_pool[1].get_result(
						               gametime < 60 * 60 * 1000, get_stocklevel(bo, gametime) < 2,
						               persistent_data->trees_around_cutters + 5 < bf->trees_nearby * 10,
						               bf->near_border, persistent_data->trees_around_cutters) *
						            management_data.get_military_number_at(37) / 2);

						if (bo.total_count() == 0) {
							prio += 200;
						} else {
							prio +=
							   std::abs(management_data.get_military_number_at(66) * 2) / bo.total_count();
						}

						prio -= bf->water_nearby / 5;

						prio += management_data.neuron_pool[67].get_result_safe(
						           persistent_data->trees_around_cutters / 3, kAbsValue) /
						        2;

						prio -=
						   management_data.neuron_pool[49].get_result_safe(bf->trees_nearby, kAbsValue) /
						   5;

						prio += bf->producers_nearby.at(bo.production_hint) * 5 -
						        new_buildings_stop_ * 15 - bf->space_consumers_nearby * 5 -
						        bf->rocks_nearby / 3 + bf->supporters_nearby.at(bo.production_hint) * 3;
						// printf (" %d priority for ranger here %d, overdue: %d\n", player_number(),
						// prio, bo.new_building_overdue);

					} else {  // FISH BREEDERS and GAME KEEPERS

						// especially for fish breeders
						if (bo.need_water && (bf->water_nearby < 6 || bf->fish_nearby < 6)) {
							continue;
						}
						if (bo.need_water) {
							prio += (-6 + bf->water_nearby) / 3;
							prio += (-6 + bf->fish_nearby) / 3;
						}

						if ((bo.total_count() - bo.unconnected_count) > bo.cnt_target) {
							continue;
						}

						if (get_stocklevel(bo, gametime) > 50) {
							continue;
						}

						if (bo.total_count() == 0) {
							prio += 100;
						} else if (!bo.need_water) {
							prio += 10 / bo.total_count();
						}

						prio += bf->producers_nearby.at(bo.production_hint) * 10;
						prio -= bf->supporters_nearby.at(bo.production_hint) * 20;

						if (bf->enemy_nearby) {
							prio -= 5;
						}
					}

				} else if (bo.recruitment && !new_buildings_stop_) {
					// this will depend on number of mines_ and productionsites
					if (static_cast<int32_t>((productionsites.size() + mines_.size()) / 30) >
					       bo.total_count() &&
					    (bo.cnt_under_construction + bo.unoccupied_count) == 0 &&
					    // but only if current buildings are utilized enough
					    (bo.total_count() == 0 || bo.current_stats > 60)) {
						prio = 10;
					}
				} else {  // finally normal productionsites
					assert(bo.production_hint < 0);

					if (bo.new_building == BuildingNecessity::kForced) {
						prio += 150;
						assert(!bo.is_shipyard);
					} else if (bo.is_shipyard) {
						assert(bo.new_building == BuildingNecessity::kAllowed);
						if (!seafaring_economy) {
							continue;
						}
					} else {
						assert(bo.new_building == BuildingNecessity::kNeeded);
					}

					// Overdue priority here
					prio += bo.primary_priority;

					// we check separatelly buildings with no inputs and some inputs
					if (bo.inputs.empty()) {

						assert(!bo.is_shipyard);

						if (bo.space_consumer) {
							// we dont like trees nearby
							prio += 1 - bf->trees_nearby / 15;
							// we attempt to cluster space consumers together
							prio += bf->space_consumers_nearby * 2;
							// and be far from rangers
							prio += 1 - bf->rangers_nearby * 3;
						} else {
							// leave some free space between them
							prio -= bf->producers_nearby.at(bo.outputs.at(0)) * 5;
						}

						if (bo.space_consumer && !bf->water_nearby) {  // not close to water
							prio += 1;
						}

						if (bo.space_consumer &&
						    !bf->unowned_mines_spots_nearby) {  // not close to mountains
							prio += 1;
						}
					}

					else if (bo.is_shipyard) {
						// for now AI builds only one shipyard
						assert(bo.total_count() == 0);
						if (bf->open_water_nearby > 3 && seafaring_economy) {
							printf(" Considering shipyard %d  %d  %d\n", bf->open_water_nearby,
							       bo.total_count(), bo.unconnected_count);
							prio += productionsites.size() * 5 + bf->open_water_nearby * 2;
						} else {
							continue;
						}
					}

					if (prio <= 0) {
						continue;
					}

					// bonus for big buildings if shortage of big fields
					if (spots_avail.at(BUILDCAPS_BIG) <= 5 && bo.desc->get_size() == 3) {
						prio += 10;
					}

					if (spots_avail.at(BUILDCAPS_MEDIUM) <= 5 && bo.desc->get_size() == 2) {
						prio += 5;
					}

					// +1 if any consumers_ are nearby
					consumers_nearby_count = 0;

					for (size_t k = 0; k < bo.outputs.size(); ++k)
						consumers_nearby_count += bf->consumers_nearby.at(bo.outputs.at(k));

					if (consumers_nearby_count > 0) {
						prio += 1;
					}
				}

				// Consider border with exemption of some huts
				if (!(bo.need_trees || bo.need_water || bo.is_fisher)) {
					prio = recalc_with_border_range(*bf, prio);
				} else if (bf->near_border && (bo.need_trees || bo.need_water)) {
					prio /= 2;
				}

			}  // production sites done
			else if (bo.type == BuildingObserver::Type::kMilitarysite) {

				assert(prio == 0);
				prio = bo.primary_priority;

				// This is another restriction of military building - but general
				if (bf->enemy_nearby && bo.fighting_type) {
					;
				}  // it is ok, go on
				else if (bf->unowned_mines_spots_nearby > 2 &&
				         (bo.mountain_conqueror || bo.expansion_type)) {
					;
				}  // it is ok, go on
				else if (bo.expansion_type) {
					if (bo.desc->get_size() == 2 && gametime % 2 >= 1) {
						continue;
					}
					if (bo.desc->get_size() == 3 && gametime % 4 >= 1) {
						continue;
					};
				} else {
					continue;
				}  // the building is not suitable for situation

				if (bf->enemy_nearby) {
					prio -= bo.build_material_shortage * 50;
				} else if (bf->near_border) {
					prio -= bo.build_material_shortage * 150;
				} else {
					prio -= bo.build_material_shortage * 500;  // prohibitive
				}

				// additional score for bigger buildings
				const int32_t prio_for_size = bo.desc->get_size() - 1;
				if (prio_for_size == 0) {
					;  // no bonus for score
				} else if (management_data.f_neuron_pool[10].get_result(
				              prio_for_size == 1, bf->enemy_nearby,
				              spots_<kSpotsTooLittle, bf->unowned_land_nearby> 5,
				              bf->unowned_buildable_spots_nearby > 0)) {
					prio +=
					   management_data.neuron_pool[29].get_result_safe(prio_for_size * 10, kAbsValue) /
					   3;
					prio += management_data.neuron_pool[30].get_result_safe(
					           bf->unowned_land_nearby / 5, kAbsValue) *
					        prio_for_size / 3;
					prio += management_data.get_military_number_at(54) *
					        std::min<uint16_t>(bf->unowned_buildable_spots_nearby, 5) / 20;
				} else {
					prio +=
					   management_data.neuron_pool[16].get_result_safe(prio_for_size * 10, kAbsValue) /
					   3;
				}

				prio += bf->military_score_;

				// if place+building is not good enough
				if (prio <= persistent_data->target_military_score) {
					continue;
				}
				if (prio > persistent_data->ai_personality_mil_upper_limit) {
					persistent_data->ai_personality_mil_upper_limit =
					   prio + std::abs(management_data.get_military_number_at(68)) * 3;
					printf(" %d increasing ai_personality_mil_upper_limit to %3d\n", pn,
					       persistent_data->ai_personality_mil_upper_limit);
				}
			} else if (bo.type == BuildingObserver::Type::kWarehouse) {

				// exclude spots on border
				if (bf->near_border && !bo.is_port) {
					continue;
				}

				assert(bf->is_portspace != ExtendedBool::kUnset);
				if (bf->is_portspace != ExtendedBool::kTrue && bo.is_port) {
					continue;
				}
				prio = bo.primary_priority;

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
				prio +=
				   management_data.neuron_pool[47].get_result_safe(nearest_distance / 2, kAbsValue);

				// dont be close to enemies
				if (bf->enemy_nearby) {
					prio -= 40;
				}

				// being too close to a border is not good either
				if (bf->unowned_land_nearby && !bo.is_port && prio > 0) {
					prio /= 2;
					prio -= 10;
				}

			} else if (bo.type == BuildingObserver::Type::kTrainingsite) {

				// Even if a site is forced it has kNeeded necessity now
				assert(bo.primary_priority > 0 && bo.new_building == BuildingNecessity::kNeeded);

				prio = bo.primary_priority;

				// for spots close to a border
				if (bf->near_border) {
					prio -= 5;
				}

				// be should rather have some mines
				if (virtual_mines < 6) {
					prio -= (6 - virtual_mines) * 7;
				}

				// take care about borders and enemies
				if (bf->enemy_nearby) {
					prio -= 20;
				}

				if (bf->unowned_land_nearby) {
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

			// testing also vicinity
			if (!bo.is_port) {
				if (port_reserved_coords.count(bf->coords.hash()) > 0) {
					continue;
				}
			}

			// Prefer road side fields
			prio += bf->preferred ? 5 : 0;

			// don't waste good land for small huts
			const bool space_stress =
			   (spots_avail.at(BUILDCAPS_MEDIUM) < 5 || spots_avail.at(BUILDCAPS_BIG) < 5);

			if (space_stress && bo.type == BuildingObserver::Type::kMilitarysite) {
				prio -= (bf->max_buildcap_nearby - bo.desc->get_size()) * 3;
			} else if (space_stress) {
				prio -= (bf->max_buildcap_nearby - bo.desc->get_size()) * 10;
			} else {
				prio -= (bf->max_buildcap_nearby - bo.desc->get_size()) * 3;
			}

			// prefer vicinity of ports (with exemption of warehouses)
			if (bf->port_nearby && bo.type == BuildingObserver::Type::kMilitarysite) {
				prio *= 2;
			}

			if (bo.type != BuildingObserver::Type::kMilitarysite && highest_nonmil_prio_ < prio) {
				highest_nonmil_prio_ = prio;
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
				if (productionsites.size() <= 8)
					break;

				if (bo.type != BuildingObserver::Type::kMine) {
					continue;
				}

				if (gametime - bo.construction_decision_time < kBuildingMinInterval) {
					continue;
				}

				assert(bo.new_building != BuildingNecessity::kAllowed);

				// skip if a mine is not required
				if (!(bo.new_building == BuildingNecessity::kNeeded ||
				      bo.new_building == BuildingNecessity::kForced)) {
					continue;
				}

				// this is penalty if there are existing mines too close
				// it is treated as multiplier for count of near mines
				uint32_t nearness_penalty = 0;
				if ((mines_per_type[bo.mines].in_construction + mines_per_type[bo.mines].finished) ==
				    0) {
					nearness_penalty = 0;
				} else {
					nearness_penalty = 40;
				}

				// bonus score to prefer if too few mines
				uint32_t bonus_score = 0;
				if ((mines_per_type[bo.mines].in_construction + mines_per_type[bo.mines].finished) ==
				    0) {
					bonus_score = 2 * bo.primary_priority;
				}

				// iterating over fields
				for (std::list<MineableField*>::iterator j = mineable_fields.begin();
				     j != mineable_fields.end(); ++j) {

					MineableField* const mf = *j;

					if (mf->field_info_expiration <= gametime) {
						continue;
					}

					if (mf->coords.field->get_resources() != bo.mines) {
						continue;
					}

					int32_t prio = 0;
					MapRegion<Area<FCoords>> mr(map, Area<FCoords>(mf->coords, 2));
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

					// applying nearnes penalty
					prio -= mf->mines_nearby * nearness_penalty;

					// applying bonus score
					prio += bonus_score;

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
		persistent_data->target_military_score = proposed_priority;
	}

	// send the command to construct a new building
	game().send_player_build(player_number(), proposed_coords, best_building->id);
	blocked_fields.add(proposed_coords, game().get_gametime() + 2 * 60 * 1000);

	// resetting new_building_overdue
	best_building->new_building_overdue = 0;

	// we block also nearby fields
	// if farms and so on, for quite a long time
	// if military sites only for short time for AI can update information on near buildable fields
	if ((best_building->space_consumer && !best_building->plants_trees) ||
	    best_building->type == BuildingObserver::Type::kMilitarysite) {
		uint32_t block_time = 0;
		uint32_t block_area = 0;
		if (best_building->space_consumer) {
			if (spots_ > kSpotsEnough) {
				block_time = 45 * 60 * 1000;
			} else {
				block_time = 10 * 60 * 1000;
			}
			block_area = 3;
		} else {  // militray buildings for a very short time
			block_time = 25 * 1000;
			block_area = 6;
		}

		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(map.get_fcoords(proposed_coords), block_area));
		do {
			blocked_fields.add(mr.location(), game().get_gametime() + block_time);
		} while (mr.advance(map));
	}

	if (!(best_building->type == BuildingObserver::Type::kMilitarysite)) {
		best_building->construction_decision_time = gametime;
	} else {  // very ugly hack here
		military_last_build_ = gametime;
		best_building->construction_decision_time = gametime - kBuildingMinInterval / 2;
	}

	// set the type of update that is needed
	if (mine) {
		next_mine_construction_due_ = gametime + kBusyMineUpdateInterval;
	}

	return true;
}

// improves current road system
bool DefaultAI::improve_roads(uint32_t gametime) {

	if (!roads.empty()) {
		const Path& path = roads.front()->get_path();

		// first force a split on roads that are longer than 3 parts
		if (path.get_nsteps() > 3 && spots_ > kSpotsEnough) {
			const Map& map = game().map();
			CoordPath cp(map, path);
			// try to split after two steps
			CoordPath::StepVector::size_type i = cp.get_nsteps() - 1, j = 1;

			for (; i >= j; --i, ++j) {
				{
					const Coords c = cp.get_coords().at(i);

					if (map[c].nodecaps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag(player_number(), c);
						return true;
					}
				}
				{
					const Coords c = cp.get_coords().at(j);

					if (map[c].nodecaps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag(player_number(), c);
						return true;
					}
				}
			}

			// Unable to set a flag - perhaps the road was build stupid
			game().send_player_bulldoze(*const_cast<Road*>(roads.front()));
			return true;
		}

		roads.push_back(roads.front());
		roads.pop_front();

		// Occasionaly (not more then once in 15 seconds) we test if the road can be dismantled
		// if there is shortage of spots we do it always
		if (last_road_dismantled_ + 15 * 1000 < gametime &&
		    (gametime % 5 == 0 || spots_ < kSpotsTooLittle)) {
			const Road& road = *roads.front();
			if (dispensable_road_test(*const_cast<Road*>(&road))) {
				game().send_player_bulldoze(*const_cast<Road*>(&road));
				last_road_dismantled_ = gametime;
				return true;
			}
		}
	}

	if (inhibit_road_building_ >= gametime) {
		return true;
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

	const Flag& flag = *eco->flags.front();

	// now we test if it is dead end flag, if yes, destroying it
	if (flag.is_dead_end() && flag.current_wares() == 0) {
		game().send_player_bulldoze(*const_cast<Flag*>(&flag));
		eco->flags.pop_front();
		return true;
	}

	bool is_warehouse = false;
	bool has_building = false;
	if (Building* b = flag.get_building()) {
		has_building = true;
		BuildingObserver& bo = get_building_observer(b->descr().name().c_str());
		if (bo.type == BuildingObserver::Type::kWarehouse) {
			is_warehouse = true;
		}
	}

	// is connected to a warehouse?
	const bool needs_warehouse = flag.get_economy()->warehouses().empty();

	// Various tests to invoke building of a shortcut (new road)
	if (flag.nr_of_roads() == 0 || needs_warehouse) {
		create_shortcut_road(flag, 17, 22, gametime);
		inhibit_road_building_ = gametime + 800;
	} else if (!has_building && flag.nr_of_roads() == 1) {
		// This is end of road without any building, we do not initiate interconnection thus
		return false;
	} else if (flag.nr_of_roads() == 1 || gametime % 10 == 0) {
		if (spots_ > kSpotsEnough) {
			// This is the normal situation
			create_shortcut_road(flag, 15, 22, gametime);
			inhibit_road_building_ = gametime + 800;
		} else if (spots_ > kSpotsTooLittle) {
			// We are short of spots so shortening must be significant
			create_shortcut_road(flag, 15, 35, gametime);
			inhibit_road_building_ = gametime + 800;
		} else {
			// We are very short of spots so shortening must be even bigger
			create_shortcut_road(flag, 15, 50, gametime);
			inhibit_road_building_ = gametime + 800;
		}
		// a warehouse with 3 or less roads
	} else if (is_warehouse && flag.nr_of_roads() <= 3) {
		create_shortcut_road(flag, 9, -5, gametime);
		inhibit_road_building_ = gametime + 400;
		// and when a flag is full with wares
	} else if (spots_ > kSpotsEnough && flag.current_wares() > 5) {
		create_shortcut_road(flag, 9, -5, gametime);
		inhibit_road_building_ = gametime + 400;
	} else {
		return false;
	}

	return true;
}

// the function takes a road (road is smallest section of roads with two flags on the ends)
// and tries to find alternative route from one flag to another.
// if route exists, it is not too long, and current road is not intensively used
// the road can be dismantled
bool DefaultAI::dispensable_road_test(Widelands::Road& road) {

	Flag& roadstartflag = road.get_flag(Road::FlagStart);
	Flag& roadendflag = road.get_flag(Road::FlagEnd);

	// We do not dismantle (even consider it) if the road is busy (some wares on flags), unless there
	// is shortage of build spots
	if (spots_ > kSpotsTooLittle &&
	    roadstartflag.current_wares() + roadendflag.current_wares() > 0) {
		return false;
	}

	std::priority_queue<NearFlag> queue;
	// only used to collect flags reachable walking over roads
	std::vector<NearFlag> reachableflags;
	queue.push(NearFlag(roadstartflag, 0, 0));
	uint8_t pathcounts = 0;
	uint8_t checkradius = 0;
	if (spots_ > kSpotsEnough) {
		checkradius = 7;
	} else if (spots_ > kSpotsTooLittle) {
		checkradius = 11;
	} else {
		checkradius = 15;
	}
	Map& map = game().map();

	// algorithm to walk on roads
	while (!queue.empty()) {

		// testing if we stand on the roadendflag
		// if is is for first time, just go on,
		// if second time, the goal is met, function returns true
		if (roadendflag.get_position().x == queue.top().flag->get_position().x &&
		    roadendflag.get_position().y == queue.top().flag->get_position().y) {
			pathcounts += 1;
			if (pathcounts > 1) {
				// OK, this is a second route how to get to roadendflag
				return true;
			}
			queue.pop();
			continue;
		}

		std::vector<NearFlag>::iterator f =
		   find(reachableflags.begin(), reachableflags.end(), queue.top().flag);

		if (f != reachableflags.end()) {
			queue.pop();
			continue;
		}

		reachableflags.push_back(queue.top());
		queue.pop();
		NearFlag& nf = reachableflags.back();

		for (uint8_t i = 1; i <= 6; ++i) {
			Road* const near_road = nf.flag->get_road(i);

			if (!near_road) {
				continue;
			}

			Flag* endflag = &near_road->get_flag(Road::FlagStart);

			if (endflag == nf.flag) {
				endflag = &near_road->get_flag(Road::FlagEnd);
			}

			int32_t dist = map.calc_distance(roadstartflag.get_position(), endflag->get_position());

			if (dist > checkradius) {  //  out of range of interest
				continue;
			}

			queue.push(NearFlag(*endflag, 0, dist));
		}
	}
	return false;
}

// trying to connect the flag to another one, be it from own economy
// or other economy
bool DefaultAI::create_shortcut_road(const Flag& flag,
                                     uint16_t checkradius,
                                     int16_t min_reduction,
                                     int32_t gametime) {

	// Increasing the failed_connection_tries counter
	// At the same time it indicates a time an economy is without a warehouse
	EconomyObserver* eco = get_economy_observer(flag.economy());
	// if we passed grace time this will be last attempt and if it fails
	// building is destroyes
	bool last_attempt_ = false;

	// this should not happen, but if the economy has a warehouse and a dismantle
	// grace time set, we must 'zero' the dismantle grace time
	if (!flag.get_economy()->warehouses().empty() &&
	    eco->dismantle_grace_time != std::numeric_limits<int32_t>::max()) {
		eco->dismantle_grace_time = std::numeric_limits<int32_t>::max();
	}

	// first we deal with situations when this is economy with no warehouses
	// and this is a flag belonging to a building/constructionsite
	// such economy must get dismantle grace time (if not set yet)
	// end sometimes extended checkradius
	if (flag.get_economy()->warehouses().empty() && flag.get_building()) {

		// occupied military buildings get special treatment
		// (extended grace time + longer radius)
		bool occupied_military_ = false;
		Building* b = flag.get_building();
		if (upcast(MilitarySite, militb, b)) {
			if (militb->stationed_soldiers().size() > 0) {
				occupied_military_ = true;
			}
		}

		// if we are within grace time, it is OK, just go on
		if (eco->dismantle_grace_time > gametime &&
		    eco->dismantle_grace_time != std::numeric_limits<int32_t>::max()) {
			;

			// if grace time is not set, this is probably first time without a warehouse and we must
			// set it
		} else if (eco->dismantle_grace_time == std::numeric_limits<int32_t>::max()) {

			// constructionsites
			if (upcast(ConstructionSite const, constructionsite, flag.get_building())) {
				BuildingObserver& bo =
				   get_building_observer(constructionsite->building().name().c_str());
				// first very special case - a port (in the phase of constructionsite)
				// this might be a new colonization port
				if (bo.is_port) {
					eco->dismantle_grace_time = gametime + 60 * 60 * 1000;  // one hour should be enough
				} else {  // other constructionsites, usually new (standalone) constructionsites
					eco->dismantle_grace_time =
					   gametime + 30 * 1000 +            // very shot time is enough
					   (eco->flags.size() * 30 * 1000);  // + 30 seconds for every flag in economy
				}

				// buildings
			} else {

				if (occupied_military_) {
					eco->dismantle_grace_time =
					   (gametime + 90 * 60 * 1000) + (eco->flags.size() * 20 * 1000);

				} else {  // for other normal buildings
					eco->dismantle_grace_time =
					   gametime + (45 * 60 * 1000) + (eco->flags.size() * 20 * 1000);
				}
			}

			// we have passed grace_time - it is time to dismantle
		} else {
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

	Map& map = game().map();

	// initializing new object of FlagsForRoads, we will push there all candidate flags
	Widelands::FlagsForRoads RoadCandidates(min_reduction);

	FindNodeWithFlagOrRoad functor;
	CheckStepRoadAI check(player_, MOVECAPS_WALK, true);

	// get all flags within radius
	std::vector<Coords> reachable;
	map.find_reachable_fields(
	   Area<FCoords>(map.get_fcoords(flag.get_position()), checkradius), &reachable, check, functor);

	for (const Coords& reachable_coords : reachable) {

		// ignore starting flag, of course
		if (reachable_coords == flag.get_position()) {
			continue;
		}

		// first make sure there is an immovable (should be, but still)
		if (upcast(PlayerImmovable const, player_immovable, map[reachable_coords].get_immovable())) {

			// if it is the road, make a flag there
			if (dynamic_cast<const Road*>(map[reachable_coords].get_immovable())) {
				game().send_player_build_flag(player_number(), reachable_coords);
			}

			// do not go on if it is not a flag
			if (!dynamic_cast<const Flag*>(map[reachable_coords].get_immovable())) {
				continue;
			}

			// testing if a flag/road's economy has a warehouse, if not we are not
			// interested to connect to it
			if (player_immovable->economy().warehouses().size() == 0) {
				continue;
			}

			// This is a candidate, sending all necessary info to RoadCandidates
			const bool different_economy = (player_immovable->get_economy() != flag.get_economy());
			const int32_t air_distance = map.calc_distance(flag.get_position(), reachable_coords);
			RoadCandidates.add_flag(reachable_coords, air_distance, different_economy);
		}
	}

	// now we walk over roads and if field is reachable by roads, we change the distance assigned
	// above
	std::priority_queue<NearFlag> queue;
	std::vector<NearFlag> nearflags;  // only used to collect flags reachable walk over roads
	queue.push(NearFlag(flag, 0, 0));

	// algorithm to walk on roads
	while (!queue.empty()) {
		std::vector<NearFlag>::iterator f =
		   find(nearflags.begin(), nearflags.end(), queue.top().flag);

		if (f != nearflags.end()) {
			queue.pop();
			continue;
		}

		nearflags.push_back(queue.top());
		queue.pop();
		NearFlag& nf = nearflags.back();

		for (uint8_t i = 1; i <= 6; ++i) {
			Road* const road = nf.flag->get_road(i);

			if (!road) {
				continue;
			}

			Flag* endflag = &road->get_flag(Road::FlagStart);

			if (endflag == nf.flag) {
				endflag = &road->get_flag(Road::FlagEnd);
			}

			int32_t dist = map.calc_distance(flag.get_position(), endflag->get_position());

			if (dist > checkradius + 5) {  //  Testing bigger vicinity then checkradius....
				continue;
			}

			queue.push(NearFlag(*endflag, nf.cost + road->get_path().get_nsteps(), dist));
		}
	}

	// Sending calculated walking costs from nearflags to RoadCandidates to update info on
	// Candidate flags/roads
	for (auto& nf_walk : nearflags) {
		if (map.calc_distance(flag.get_position(), nf_walk.flag->get_position()) <= checkradius) {
			// nearflags contains also flags beyond the radius, so we skip these
			RoadCandidates.set_road_distance(
			   nf_walk.flag->get_position(), static_cast<int32_t>(nf_walk.cost));
		}
	}

	// We do not calculate roads to all nearby flags, ideally we investigate 4 roads, but the number
	// can be higher if a road cannot be built to considered flag. The logic is: 2 points for
	// possible
	// road, 1 for impossible, and count < 10 so in worst scenario we will calculate 10 impossible
	// roads without finding any possible
	uint32_t count = 0;
	uint32_t current = 0;  // hash of flag that we are going to calculate in the iteration
	while (count < 10 && RoadCandidates.get_best_uncalculated(&current)) {
		const Widelands::Coords coords = Coords::unhash(current);

		Path path;

		// value of pathcost is not important, it just indicates, that the path can be built
		const int32_t pathcost = map.findpath(flag.get_position(), coords, 0, path, check);
		if (pathcost >= 0) {
			RoadCandidates.road_possible(coords, path.get_nsteps());
			count += 2;
		} else {
			RoadCandidates.road_impossible(coords);
			count += 1;
		}
	}

	// Well and finally building the winning road
	uint32_t winner_hash = 0;
	if (RoadCandidates.get_winner(&winner_hash, (gametime % 4 > 0) ? 1 : 2)) {
		const Widelands::Coords target_coords = Coords::unhash(winner_hash);
		Path& path = *new Path();
		const int32_t pathcost = map.findpath(flag.get_position(), target_coords, 0, path, check);
		assert(pathcost >= 0);
		game().send_player_build_road(player_number(), path);
		return true;
	}

	// if all possible roads skipped
	if (last_attempt_) {
		Building* bld = flag.get_building();
		// first we block the field and vicinity for 15 minutes, probably it is not a good place to
		// build on
		MapRegion<Area<FCoords>> mr(
		   game().map(), Area<FCoords>(map.get_fcoords(bld->get_position()), 2));
		do {
			blocked_fields.add(mr.location(), game().get_gametime() + 15 * 60 * 1000);
		} while (mr.advance(map));
		eco->flags.remove(&flag);
		game().send_player_bulldoze(*const_cast<Flag*>(&flag));
		return true;
	}

	return false;
}

/**
 * Checks if anything in one of the economies changed and takes care for these
 * changes.
 *
 * \returns true, if something was changed.
 */
bool DefaultAI::check_economies() {
	while (!new_flags.empty()) {
		const Flag& flag = *new_flags.front();
		new_flags.pop_front();
		get_economy_observer(flag.economy())->flags.push_back(&flag);
	}

	for (std::list<EconomyObserver*>::iterator obs_iter = economies.begin();
	     obs_iter != economies.end(); ++obs_iter) {
		// check if any flag has changed its economy
		std::list<Flag const*>& fl = (*obs_iter)->flags;

		for (std::list<Flag const*>::iterator j = fl.begin(); j != fl.end();) {
			if (&(*obs_iter)->economy != &(*j)->economy()) {
				get_economy_observer((*j)->economy())->flags.push_back(*j);
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
bool DefaultAI::check_productionsites(uint32_t gametime) {
	if (productionsites.empty()) {
		return false;
	}

	// Reorder and set new values; - better now because there are multiple returns in the function
	productionsites.push_back(productionsites.front());
	productionsites.pop_front();

	// Get link to productionsite that should be checked
	ProductionSiteObserver& site = productionsites.front();

	// first we werify if site is working yet (can be unoccupied since the start)
	if (!site.site->can_start_working()) {
		site.unoccupied_till = game().get_gametime();
	}

	// is it connected to wh at all?
	const bool connected_to_wh = !site.site->get_economy()->warehouses().empty();

	// do not dismantle or upgrade the same type of building too soon - to give some time to update
	// statistics
	if (site.bo->last_dismantle_time > game().get_gametime() - 30 * 1000) {
		return false;
	}

	// Get max radius of recursive workarea
	WorkareaInfo::size_type radius = 0;
	const WorkareaInfo& workarea_info = site.bo->desc->workarea_info_;
	for (const auto& temp_info : workarea_info) {
		if (radius < temp_info.first) {
			radius = temp_info.first;
		}
	}

	Map& map = game().map();

	// The code here is bit complicated
	// a) Either this site is pending for upgrade, if ready, order the upgrade
	// b) other site of type is pending for upgrade
	// c) if none of above, we can consider upgrade of this one

	const DescriptionIndex enhancement = site.site->descr().enhancement();

	bool considering_upgrade = enhancement != INVALID_INDEX;

	// First we check for rare case when input wares are set to 0 but AI is not aware that
	// the site is pending for upgrade - one possible cause is this is a freshly loaded game
	if (!site.upgrade_pending) {
		bool resetting_wares = false;
		for (auto& queue : site.site->warequeues()) {
			if (queue->get_max_fill() == 0) {
				resetting_wares = true;
				game().send_player_set_ware_max_fill(
				   *site.site, queue->get_ware(), queue->get_max_size());
			}
		}
		if (resetting_wares) {
			log(" %d: AI: input queues were reset to max for %s (game just loaded?)\n",
			    player_number(), site.bo->name);
			return true;
		}
	}

	if (site.upgrade_pending) {
		// The site is in process of emptying its input queues
		// Counting remaining wares in the site now
		int32_t left_wares = 0;
		for (auto& queue : site.site->warequeues()) {
			left_wares += queue->get_filled();
		}
		// Do nothing when some wares are left, but do not wait more then 4 minutes
		if (site.bo->construction_decision_time + 4 * 60 * 1000 > gametime && left_wares > 0) {
			return false;
		}
		assert(site.bo->cnt_upgrade_pending == 1);
		assert(enhancement != INVALID_INDEX);
		game().send_player_enhance_building(*site.site, enhancement);
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
		if (site.bo->upgrade_extends) {
			if (gametime < site.built_time + 10 * 60 * 1000) {
				considering_upgrade = false;
			}
		} else {
			if (gametime < 45 * 60 * 1000 || gametime < site.built_time + 20 * 60 * 1000) {
				considering_upgrade = false;
			}
		}
	}

	// No upgrade without proper workers
	if (considering_upgrade && !site.site->has_workers(enhancement, game())) {
		considering_upgrade = false;
	}

	if (considering_upgrade) {

		const BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
		BuildingObserver& en_bo = get_building_observer(bld.name().c_str());
		bool doing_upgrade = false;

		// 10 minutes is a time to productions statics to settle
		if ((en_bo.last_building_built == kNever ||
		     gametime - en_bo.last_building_built >= 10 * 60 * 1000) &&
		    (en_bo.cnt_under_construction + en_bo.unoccupied_count) == 0) {

			// forcing first upgrade
			if (en_bo.total_count() == 0) {
				doing_upgrade = true;
			}

			if (en_bo.total_count() == 1) {
				if (en_bo.current_stats > 55) {
					doing_upgrade = true;
				}
			}

			if (en_bo.total_count() > 1) {
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

			// reducing input queues
			for (auto& queue : site.site->warequeues()) {
				game().send_player_set_ware_max_fill(*site.site, queue->get_ware(), 0);
			}
			site.bo->construction_decision_time = gametime;
			en_bo.construction_decision_time = gametime;
			site.upgrade_pending = true;
			site.bo->cnt_upgrade_pending += 1;
			return true;
		}
	}

	// Lumberjack / Woodcutter handling
	if (site.bo->need_trees) {

		// do not dismantle immediatelly
		if ((game().get_gametime() - site.built_time) < 4 * 60 * 1000) {
			return false;
		}

		const uint32_t remaining_trees = map.find_immovables(
		   Area<FCoords>(map.get_fcoords(site.site->get_position()), radius), nullptr,
		   FindImmovableAttribute(MapObjectDescr::get_attribute_id("tree")));

		// generally, trees_around_cutters = remaining_trees + 9 *
		// persistent_data->trees_around_cutters
		// but keep in mind that trees_around_cutters is multiplied by 10
		persistent_data->trees_around_cutters =
		   (remaining_trees * 10 + 9 * persistent_data->trees_around_cutters) / 10;

		// Do not destruct the last few lumberjacks
		if (site.bo->cnt_built <= site.bo->cnt_target) {
			return false;
		}

		if (site.site->get_statistics_percent() > 20) {
			return false;
		}

		// do not dismantle if there are some trees remaining
		if (remaining_trees > 5) {
			return false;
		}

		// if we need wood badly
		if (remaining_trees > 0 && get_stocklevel(*site.bo, gametime) <= 50) {
			return false;
		}

		// so finally we dismantle the lumberjac
		site.bo->last_dismantle_time = game().get_gametime();
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}

		return true;
	}

	// Wells handling
	if (site.bo->mines_water) {
		if (site.unoccupied_till + 6 * 60 * 1000 < gametime &&
		    site.site->get_statistics_percent() == 0) {
			site.bo->last_dismantle_time = gametime;
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site);
			} else {
				game().send_player_bulldoze(*site.site);
			}

			return true;
		}

		// do not consider dismantling if we are under target
		if (site.bo->last_dismantle_time + 90 * 1000 > game().get_gametime()) {
			return false;
		}

		// now we test the stocklevel and dismantle the well if we have enough water
		// but first we make sure we do not dismantle a well too soon
		// after dismantling previous one
		if (get_stocklevel(*site.bo, gametime) > 250 + productionsites.size() * 5) {  // dismantle
			site.bo->last_dismantle_time = game().get_gametime();
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		return false;
	}

	// Quarry handling
	if (site.bo->need_rocks) {

		if (map.find_immovables(Area<FCoords>(map.get_fcoords(site.site->get_position()), 6), nullptr,

		                        FindImmovableAttribute(MapObjectDescr::get_attribute_id("rocks"))) ==
		    0) {
			// destruct the building and it's flag (via flag destruction)
			// the destruction of the flag avoids that defaultAI will have too many
			// unused roads - if needed the road will be rebuild directly.
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		if (site.unoccupied_till + 6 * 60 * 1000 < gametime &&
		    site.site->get_statistics_percent() == 0) {
			// it is possible that there are rocks but quarry is not able to mine them
			site.bo->last_dismantle_time = game().get_gametime();
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site);
			} else {
				game().send_player_bulldoze(*site.site);
			}

			return true;
		}

		return false;
	}

	// All other SPACE_CONSUMERS without input and above target_count
	if (site.bo->inputs.empty()                              // does not consume anything
	    && site.bo->production_hint == -1                    // not a renewing building (forester...)
	    && site.unoccupied_till + 10 * 60 * 1000 < gametime  // > 10 minutes old
	    && site.site->can_start_working()                    // building is occupied
	    && site.bo->space_consumer && !site.bo->plants_trees) {

		// if we have more buildings then target
		if ((site.bo->cnt_built - site.bo->unconnected_count) > site.bo->cnt_target) {
			// if (site.bo->stocklevel_time < game().get_gametime() - 5 * 1000) {
			// site.bo->stocklevel = calculate_stocklevel(*site.bo);
			// site.bo->stocklevel_time = game().get_gametime();
			//}

			if (site.site->get_statistics_percent() < 30 && get_stocklevel(*site.bo, gametime) > 100) {
				site.bo->last_dismantle_time = game().get_gametime();
				flags_to_be_removed.push_back(site.site->base_flag().get_position());
				if (connected_to_wh) {
					game().send_player_dismantle(*site.site);
				} else {
					game().send_player_bulldoze(*site.site);
				}
				return true;
			}
		}

		// a building can be dismanteld if it performs too bad, if it is not the last one
		if (site.site->get_statistics_percent() <= 10 && site.bo->cnt_built > 1) {

			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		return false;
	}

	// buildings with inputs, checking if we can a dismantle some due to low performance
	if (!site.bo->inputs.empty() && (site.bo->cnt_built - site.bo->unoccupied_count) >= 3 &&
	    site.site->can_start_working() &&
	    check_building_necessity(*site.bo, PerfEvaluation::kForDismantle, gametime) ==
	       BuildingNecessity::kNotNeeded &&
	    gametime - site.bo->last_dismantle_time > 5 * 60 * 1000 &&

	    site.bo->current_stats > site.site->get_statistics_percent() &&  // underperformer
	    (game().get_gametime() - site.unoccupied_till) > 10 * 60 * 1000) {

		site.bo->last_dismantle_time = game().get_gametime();

		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		return true;
	}

	// remaining buildings without inputs and not supporting ones (fishers only left probably and
	// hunters)
	if (site.bo->inputs.empty() && site.bo->production_hint < 0 && site.site->can_start_working() &&
	    !site.bo->space_consumer && site.site->get_statistics_percent() < 10 &&
	    ((game().get_gametime() - site.built_time) > 10 * 60 * 1000)) {

		site.bo->last_dismantle_time = game().get_gametime();
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		return true;
	}

	// supporting productionsites (rangers)
	// stop/start them based on stock avaiable
	if (site.bo->production_hint >= 0) {

		if (!site.bo->plants_trees) {
			// other supporting sites, like fish breeders, gamekeepers are not dismantled at all
			return false;
		}

		// dismantling the rangers hut, but only if we have them above a target
		if (wood_policy_ == WoodPolicy::kDismantleRangers &&
		    site.bo->cnt_built > site.bo->cnt_target) {

			site.bo->last_dismantle_time = game().get_gametime();
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site);
			} else {
				game().send_player_bulldoze(*site.site);
			}
			return true;
		}

		// stopping a ranger (sometimes the policy can be kDismantleRangers,
		// but we still preserve some rangers for sure)
		if ((wood_policy_ == WoodPolicy::kStopRangers ||
		     wood_policy_ == WoodPolicy::kDismantleRangers) &&
		    !site.site->is_stopped()) {

			game().send_player_start_stop_building(*site.site);
			return false;
		}

		const uint32_t trees_in_vicinity =
		   map.find_immovables(Area<FCoords>(map.get_fcoords(site.site->get_position()), 5), nullptr,
		                       FindImmovableAttribute(MapObjectDescr::get_attribute_id("tree")));

		// stop ranger if enough trees around regardless of policy
		if (trees_in_vicinity > 25) {
			if (!site.site->is_stopped()) {
				game().send_player_start_stop_building(*site.site);
			}
			// if not enough trees nearby, we can start them if required
		} else if ((wood_policy_ == WoodPolicy::kAllowRangers) && site.site->is_stopped()) {
			game().send_player_start_stop_building(*site.site);
		}
	}

	return false;
}

// This function scans current situation with shipyards, ports, ships, ongoing expeditions
// and makes two decisions:
// - build a ship
// - start preparation for expedition
bool DefaultAI::marine_main_decisions() {

	if (!seafaring_economy) {
		set_taskpool_task_time(kNever, SchedulerTaskId::KMarineDecisions);
		return false;
	}

	// getting some base statistics
	player_ = game().get_player(player_number());
	uint16_t ports_count = 0;
	uint16_t shipyards_count = 0;
	uint16_t expeditions_in_prep = 0;
	uint16_t expeditions_in_progress = 0;
	bool idle_shipyard_stocked = false;

	// goes over all warehouses (these includes ports)
	for (const WarehouseSiteObserver& wh_obs : warehousesites) {
		if (wh_obs.bo->is_port) {
			ports_count += 1;
			if (Widelands::PortDock* pd = wh_obs.site->get_portdock()) {
				if (pd->expedition_started()) {
					expeditions_in_prep += 1;
				}
			}
		}
	}

	// goes over productionsites and gets status of shipyards
	for (const ProductionSiteObserver& ps_obs : productionsites) {
		if (ps_obs.bo->is_shipyard) {
			shipyards_count += 1;

			// counting stocks
			uint8_t stocked_wares = 0;
			std::vector<WaresQueue*> const warequeues = ps_obs.site->warequeues();
			size_t const nr_warequeues = warequeues.size();
			for (size_t i = 0; i < nr_warequeues; ++i) {
				stocked_wares += warequeues[i]->get_filled();
			}
			if (stocked_wares == 16 && ps_obs.site->is_stopped() && ps_obs.site->can_start_working()) {
				idle_shipyard_stocked = true;
			}
		}
	}

	// and now over ships
	for (std::list<ShipObserver>::iterator sp_iter = allships.begin(); sp_iter != allships.end();
	     ++sp_iter) {
		if (sp_iter->ship->state_is_expedition()) {
			expeditions_in_progress += 1;
		}
	}

	assert(allships.size() >= expeditions_in_progress);

	enum class FleetStatus : uint8_t { kNeedShip = 0, kEnoughShips = 1, kDoNothing = 2 };

	// now we must compare ports vs ships to decide if new ship is needed or new expedition can start
	FleetStatus enough_ships = FleetStatus::kDoNothing;
	if (shipyards_count == 0 || !idle_shipyard_stocked || ports_count == 0) {
		enough_ships = FleetStatus::kDoNothing;
	} else if (allships.size() - expeditions_in_progress == 0) {
		// We always need at least one ship in transport mode
		enough_ships = FleetStatus::kNeedShip;
	} else if (persistent_data->ships_utilization > 5000) {
		// If ships utilization is too high
		enough_ships = FleetStatus::kNeedShip;
	} else {
		enough_ships = FleetStatus::kEnoughShips;
	}

	// building a ship? if yes, find a shipyard and order it to build a ship
	if (enough_ships == FleetStatus::kNeedShip) {

		for (const ProductionSiteObserver& ps_obs : productionsites) {
			if (ps_obs.bo->is_shipyard && ps_obs.site->can_start_working() &&
			    ps_obs.site->is_stopped()) {
				// make sure it is fully stocked
				// counting stocks
				uint8_t stocked_wares = 0;
				std::vector<WaresQueue*> const warequeues = ps_obs.site->warequeues();
				size_t const nr_warequeues = warequeues.size();
				for (size_t i = 0; i < nr_warequeues; ++i) {
					stocked_wares += warequeues[i]->get_filled();
				}
				if (stocked_wares < 16) {
					continue;
				}

				game().send_player_start_stop_building(*ps_obs.site);
				return true;
			}
		}
	}

	// starting an expedition? if yes, find a port and order it to start an expedition
	if (idle_shipyard_stocked && persistent_data->no_more_expeditions == kFalse && ports_count > 0 &&
	    enough_ships == FleetStatus::kEnoughShips && expeditions_in_prep == 0 &&
	    expeditions_in_progress == 0) {
		// we need to find a port
		for (const WarehouseSiteObserver& wh_obs : warehousesites) {
			if (wh_obs.bo->is_port) {
				game().send_player_start_or_cancel_expedition(*wh_obs.site);
				return true;
			}
		}
	}
	return true;
}

// This identifies ships that are waiting for command
bool DefaultAI::check_ships(uint32_t const gametime) {

	if (!seafaring_economy) {
		set_taskpool_task_time(std::numeric_limits<int32_t>::max(), SchedulerTaskId::kCheckShips);
		return false;
	}

	bool action_taken = false;

	if (!allships.empty()) {
		// iterating over ships and doing what is needed
		for (std::list<ShipObserver>::iterator i = allships.begin(); i != allships.end(); ++i) {

			const Widelands::Ship::ShipStates ship_state = i->ship->get_ship_state();

			// Here we manage duration of expedition and related variables
			if (ship_state == Widelands::Ship::ShipStates::kExpeditionWaiting ||
			    ship_state == Widelands::Ship::ShipStates::kExpeditionScouting ||
			    ship_state == Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {

				// the function below will take care of variables like
				// - expedition_ship_
				// - expedition_start_time
				// - expected_colony_scan
				// - no_more_expeditions_
				check_ship_in_expedition(*i, gametime);

				// We are not in expedition mode (or perhaps building a colonisation port)
				// so resetting start time
			} else if (expedition_ship_ == i->ship->serial()) {
				// Obviously expedition just ended
				persistent_data->expedition_start_time = kNoExpedition;
				expedition_ship_ = kNoShip;
			}

			// only two states need an attention
			if ((i->ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionWaiting ||
			     i->ship->get_ship_state() ==
			        Widelands::Ship::ShipStates::kExpeditionPortspaceFound) &&
			    !i->waiting_for_command_) {
				if (gametime - i->last_command_time > 180 * 1000) {
					i->waiting_for_command_ = true;
					log("  %1d: last command for ship at %3dx%3d was %3d seconds ago, something wrong "
					    "here?...\n",
					    player_number(), i->ship->get_position().x, i->ship->get_position().y,
					    (gametime - i->last_command_time) / 1000);
				}
			}
			// if ships is waiting for command
			if (i->waiting_for_command_) {
				expedition_management(*i);
				action_taken = true;
			}

			// Checking utilization
			if (i->ship->get_ship_state() == Widelands::Ship::ShipStates::kTransport) {
				// Good utilization is 10 pieces of ware onboard, to track utilization we use range
				// 0-10000
				// to avoid float or rounding errors if integers in range 0-100
				const int16_t tmp_util =
				   (i->ship->get_nritems() > 10) ? 10000 : i->ship->get_nritems() * 1000;
				// This number is kind of average
				persistent_data->ships_utilization =
				   persistent_data->ships_utilization * 19 / 20 + tmp_util / 20;

				// Arithmetics check
				assert(persistent_data->ships_utilization >= 0 &&
				       persistent_data->ships_utilization <= 10000);
			}
		}
	}

	// processing marine_task_queue
	while (!marine_task_queue.empty()) {
		if (marine_task_queue.back() == kStopShipyard) {
			// iterate over all production sites searching for shipyard
			for (std::list<ProductionSiteObserver>::iterator site = productionsites.begin();
			     site != productionsites.end(); ++site) {
				if (site->bo->is_shipyard) {
					if (!site->site->is_stopped()) {
						game().send_player_start_stop_building(*site->site);
					}
				}
			}
		}

		if (marine_task_queue.back() == kReprioritize) {
			for (std::list<ProductionSiteObserver>::iterator site = productionsites.begin();
			     site != productionsites.end(); ++site) {
				if (site->bo->is_shipyard) {
					for (uint32_t k = 0; k < site->bo->inputs.size(); ++k) {
						game().send_player_set_ware_priority(
						   *site->site, wwWARE, site->bo->inputs.at(k), HIGH_PRIORITY);
					}
				}
			}
		}

		marine_task_queue.pop_back();
	}

	if (action_taken) {
		set_taskpool_task_time(gametime + kShipCheckInterval, SchedulerTaskId::kCheckShips);
	}
	return true;
}

/**
 * This is part of check_ships() function separated due to readibility purpuses
 */
void DefaultAI::check_ship_in_expedition(ShipObserver& so, uint32_t const gametime) {
	// consistency check
	assert(expedition_ship_ == so.ship->serial() || expedition_ship_ == kNoShip);

	// This is obviously new expedition
	if (expedition_ship_ == kNoShip) {
		assert(persistent_data->expedition_start_time == kNoExpedition);
		persistent_data->expedition_start_time = gametime;
		expedition_ship_ = so.ship->serial();

		// Already known expedition, all we do now, is decreasing persistent_data->colony_scan_area
		// based on lapsed time
	} else if (gametime - persistent_data->expedition_start_time < kExpeditionMaxDuration) {
		assert(persistent_data->expedition_start_time > kNoExpedition);
		// remaining_time is a percent so in range 0-100
		const uint32_t remaining_time = 100 - ((gametime - persistent_data->expedition_start_time) /
		                                       (kExpeditionMaxDuration / 100));
		assert(remaining_time <= 100);

		// We calculate expected value and actual value (persistent_data->colony_scan_area
		// is changed only when needed)
		const uint32_t expected_colony_scan =
		   kColonyScanMinArea + (kColonyScanStartArea - kColonyScanMinArea) * remaining_time / 100;
		assert(expected_colony_scan >= kColonyScanMinArea &&
		       expected_colony_scan <= kColonyScanStartArea);

		// So changing it if needed
		if (expected_colony_scan < persistent_data->colony_scan_area) {
			persistent_data->colony_scan_area = expected_colony_scan;
		}

		// Expedition overdue. Setting no_more_expeditions = true
		// But we do not cancel it, the code for cancellation does not work properly now
		// TODO(unknown): - expedition code for cancellation needs to be fixed and afterwareds
		// AI can be changed to cancel overdue expedition
	} else if (gametime - persistent_data->expedition_start_time >= kExpeditionMaxDuration) {
		assert(persistent_data->expedition_start_time > 0);
		persistent_data->colony_scan_area = kColonyScanMinArea;
		persistent_data->no_more_expeditions = kTrue;
	}
}

/**
 * checks the first mine in list, takes care if it runs out of
 * resources and finally reenqueues it at the end of the list.
 *
 * \returns true, if something was changed.
 */
bool DefaultAI::check_mines_(uint32_t const gametime) {
	if (mines_.empty()) {
		return false;
	}

	// Reorder and set new values; - due to returns within the function
	mines_.push_back(mines_.front());
	mines_.pop_front();

	// Get link to productionsite that should be checked
	ProductionSiteObserver& site = mines_.front();

	const bool connected_to_wh = !site.site->get_economy()->warehouses().empty();

	// first get rid of mines that are missing workers for some time (6 minutes),
	// released worker (if any) can be usefull elsewhere !
	if (site.built_time + 6 * 60 * 1000 < gametime && !site.site->can_start_working()) {
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		return true;
	}

	// to avoid problems with uint underflow, we discourage considerations below
	if (gametime < 10 * 60 * 1000) {
		return false;
	}

	// if mine is working, doing nothing
	if (site.no_resources_since > gametime - 5 * 60 * 1000) {
		return false;
	}

	// Check whether building is enhanceable. If yes consider an upgrade.
	const DescriptionIndex enhancement = site.site->descr().enhancement();
	bool has_upgrade = false;
	if (enhancement != INVALID_INDEX) {
		if (player_->is_building_type_allowed(enhancement)) {
			has_upgrade = true;
		}
	}

	// every type of mine has minimal number of mines that are to be preserved
	// (we will not dismantle even if there are no mineable resources left for this level of mine
	// and output is not needed)
	bool forcing_upgrade = false;
	const uint16_t minimal_mines_count = (site.bo->produces_building_material) ? 2 : 1;
	if (has_upgrade && mines_per_type[site.bo->mines].total_count() <= minimal_mines_count) {
		forcing_upgrade = true;
	}

	// dismantling a mine
	if (!has_upgrade) {  // if no upgrade, now
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		site.bo->construction_decision_time = gametime;
		return true;
		// if having an upgrade, after half hour
	} else if (site.no_resources_since < gametime - 30 * 60 * 1000 && !forcing_upgrade) {
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		site.bo->construction_decision_time = gametime;
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
	const BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
	BuildingObserver& en_bo = get_building_observer(bld.name().c_str());

	// Make sure we do not exceed limit given by AI mode
	if (en_bo.aimode_limit_status() == AiModeBuildings::kAnotherAllowed) {

		// if it is too soon for enhancement
		if (gametime - en_bo.construction_decision_time >= kBuildingMinInterval) {
			// now verify that there are enough workers
			if (site.site->has_workers(enhancement, game())) {  // enhancing
				game().send_player_enhance_building(*site.site, enhancement);
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

BuildingNecessity DefaultAI::check_warehouse_necessity(BuildingObserver& bo,
                                                       const uint32_t gametime) {
	bo.primary_priority = 0;

	if (bo.cnt_under_construction > 0 ||
	    bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}

	bool warehouse_needed = false;

	//  Build one warehouse for ~every 35 productionsites and mines_.
	//  Militarysites are slightly important as well, to have a bigger
	//  chance for a warehouses (containing waiting soldiers or wares
	//  needed for soldier training) near the frontier.
	// bo.primary_priority = static_cast<int32_t>(productionsites.size() + mines_.size()) + 20 -
	// static_cast<int32_t>(numof_warehouses_) * (35 + management_data.get_military_number_at(15) /
	// 5);
	int32_t needed_count = static_cast<int32_t>(productionsites.size() + mines_.size()) /
	                          (40 + management_data.get_military_number_at(21) / 10) +
	                       1;
	assert(needed_count >= 0 &&
	       needed_count <= (static_cast<uint16_t>(productionsites.size() + mines_.size()) / 10) + 2);

	if (needed_count > numof_warehouses_) {
		warehouse_needed = true;
		bo.primary_priority = (needed_count - numof_warehouses_) *
		                      (20 + management_data.get_military_number_at(22) / 10);
	} else {
		bo.primary_priority = 0;
	}

	if ((productionsites.size() + mines_.size()) > 20 && numof_warehouses_ <= 1) {
		bo.primary_priority += 15 + management_data.get_military_number_at(23) / 5;
	}

	if (numof_warehouses_ <= 1 && player_statistics.any_enemy_seen_lately(gametime)) {
		bo.primary_priority += 15 + management_data.get_military_number_at(24) / 10;
	}

	bo.primary_priority +=
	   management_data.neuron_pool[45].get_result_safe(7 - numof_warehouses_ * 3, kAbsValue) / 5;
	bo.primary_priority +=
	   management_data.neuron_pool[46].get_result_safe(gametime / 1000 / 60 / 5, kAbsValue) / 5;

	// But we still can built a port if it is first one
	if (bo.is_port && bo.total_count() == 0 && productionsites.size() > 5 && seafaring_economy) {
		bo.primary_priority += std::abs(management_data.get_military_number_at(16)) / 10;
		warehouse_needed = true;
	}

	if (!warehouse_needed) {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
		;
	}

	// we prefer ports to a normal warehouse
	if (bo.is_port) {
		bo.primary_priority += std::abs(management_data.get_military_number_at(17)) / 10;
	}

	// considering overdue time
	bo.primary_priority += -20 + bo.new_building_overdue;

	if (bo.primary_priority > 0) {
		bo.new_building_overdue += 1;
		return BuildingNecessity::kAllowed;
	} else {
		bo.new_building_overdue = 0;
		return BuildingNecessity::kForbidden;
	}
}

// this receives an building observer and have to decide if new/one of
// current buildings of this type is needed
// This is core of construct_building() function
// This is run once when construct_building() is run, or when considering
// dismantle
BuildingNecessity DefaultAI::check_building_necessity(BuildingObserver& bo,
                                                      const PerfEvaluation purpose,
                                                      const uint32_t gametime) {

	// Very first we finds if AI is allowed to build such building due to its mode
	if (purpose == PerfEvaluation::kForConstruction &&
	    bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
		return BuildingNecessity::kForbidden;
	}

	// First we deal with training sites, they are separate category
	if (bo.type == BuildingObserver::Type::kTrainingsite) {

		bo.primary_priority = 0;
		if (bo.aimode_limit_status() != AiModeBuildings::kAnotherAllowed) {
			return BuildingNecessity::kNotNeeded;
		} else if (ts_without_trainers_ || (ts_basic_const_count_ + ts_advanced_const_count_) > 0) {
			return BuildingNecessity::kNotNeeded;
		} else if (bo.prohibited_till > gametime) {
			return BuildingNecessity::kNotNeeded;
		} else if (bo.build_material_shortage) {
			return BuildingNecessity::kNotNeeded;
		}

		// It seems we might need it after all
		bo.primary_priority = -30;

		if (bo.forced_after < gametime && bo.total_count() == 0) {
			bo.primary_priority += 50;
		}

		// If we are close to enemy (was seen in last 15 minutes)
		if (player_statistics.any_enemy_seen_lately(gametime)) {
			bo.primary_priority += std::abs(management_data.get_military_number_at(57) / 3);
		}

		// We build one trainig site per X military sites
		// with some variations, of course
		bo.primary_priority += static_cast<int32_t>(militarysites.size() + productionsites.size()) -
		                       50 * (ts_basic_count_ + ts_advanced_count_);

		// We prefer basic trainingsites and sites with lower count
		if (bo.trainingsite_type == TrainingSiteType::kBasic) {
			bo.primary_priority += 1;
		}
		bo.primary_priority -= 2 * bo.total_count();

		// Special bonus for very first site of type
		if (bo.total_count() == 0) {
			bo.primary_priority += std::abs(management_data.get_military_number_at(56) / 3);
		}

		if (bo.primary_priority > 0) {
			return BuildingNecessity::kNeeded;
		} else {
			return BuildingNecessity::kNotNeeded;
		}
	}

	// Let deal with productionsites now
	// First we iterate over outputs of building, count warehoused stock
	// and deciding if we have enough on stock (in warehouses)
	bo.max_preciousness = 0;
	bo.max_needed_preciousness = 0;

	for (uint32_t m = 0; m < bo.outputs.size(); ++m) {
		DescriptionIndex wt(static_cast<size_t>(bo.outputs.at(m)));

		uint16_t target = tribe_->get_ware_descr(wt)->default_target_quantity(tribe_->name());
		if (target == Widelands::kInvalidWare) {
			target = kTargetQuantCap;
		}
		target /= 3;

		// at least  1
		target = std::max<uint16_t>(target, 1);

		uint16_t preciousness = wares.at(bo.outputs.at(m)).preciousness;
		if (preciousness < 1) {  // it seems there are wares with 0 preciousness
			preciousness = 1;     // (no entry in conf files?). But we need positive value here
		}

		if (calculate_stocklevel(wt) < target) {
			if (bo.max_needed_preciousness < preciousness) {
				bo.max_needed_preciousness = preciousness;
			}
		}

		if (bo.max_preciousness < preciousness) {
			bo.max_preciousness = preciousness;
		}
	}

	if (!bo.outputs.empty()) {
		assert(bo.max_preciousness > 0);
	}

	if (bo.is_shipyard) {
		assert(bo.max_preciousness == 0);
	}

	// This flag is to be used when buildig is forced. AI will not build another building when
	// a substitution exists. F.e. mines or pairs like tavern-inn
	// To skip unnecessary calculation, we calculate this only if we have 0 count of the buildings
	bool has_substitution_building = false;
	if (bo.total_count() == 0 && bo.upgrade_substitutes &&
	    bo.type == BuildingObserver::Type::kProductionsite) {
		const DescriptionIndex enhancement = bo.desc->enhancement();
		BuildingObserver& en_bo =
		   get_building_observer(tribe_->get_building_descr(enhancement)->name().c_str());
		if (en_bo.total_count() > 0) {
			has_substitution_building = true;
		}
	}
	if (bo.total_count() == 0 && bo.type == BuildingObserver::Type::kMine) {
		if (mines_per_type[bo.mines].in_construction + mines_per_type[bo.mines].finished > 0) {
			has_substitution_building = true;
		}
	}

	// Some buildings are upgraded to ones that does not produce current output, so we need to have
	// two of current buildings to have at least one left after one of them is upgraded
	// Logic is: after 30th minute we need second building if there is no enhanced building yet,
	// and after 90th minute we want second building unconditionally
	bool needs_second_for_upgrade = false;
	if (gametime > 30 * 60 * 1000 && bo.cnt_built == 1 && bo.cnt_under_construction == 0 &&
	    bo.upgrade_extends && !bo.upgrade_substitutes &&
	    bo.type == BuildingObserver::Type::kProductionsite) {
		const DescriptionIndex enhancement = bo.desc->enhancement();
		BuildingObserver& en_bo =
		   get_building_observer(tribe_->get_building_descr(enhancement)->name().c_str());
		if ((gametime > 30 * 60 * 1000 && en_bo.total_count() == 0) || gametime > 90 * 60 * 1000) {
			// We fake this
			bo.max_needed_preciousness = bo.max_preciousness;
			needs_second_for_upgrade = true;
		}
	}

	// This function is going to say if a building is needed. But there is a 'new_buildings_stop_'
	// flag that should be obeyed, but sometimes can be ignored.
	// So we can have two types of needed: kNeeded and KNeededPending
	// below we define which one will be returned if building is 'needed'
	BuildingNecessity needed_type = BuildingNecessity::kNeeded;
	if (new_buildings_stop_) {
		needed_type = BuildingNecessity::kNeededPending;
		if (gametime < 15 * 60 * 1000) {
			;                                     // no exemption here within first 15 minutes
		} else if (gametime < 25 * 60 * 1000) {  // exemption after 15 minutes - 1 building allowed

			if (bo.type == BuildingObserver::Type::kMine) {
				if (mines_per_type[bo.mines].in_construction + mines_per_type[bo.mines].finished == 0) {
					needed_type = BuildingNecessity::kNeeded;
				}
			}
			// if (bo.type == BuildingObserver::Type::kProductionsite) {
			// if (bo.produces_building_material || bo.max_needed_preciousness >= 10) {
			// if (bo.total_count() == 0) {
			// needed_type = BuildingNecessity::kNeeded;
			//}
			//}
			//}
		} else {  // exemption after 25 minutes - 2 buildings allowed
			if (bo.type == BuildingObserver::Type::kMine) {
				if (mines_per_type[bo.mines].in_construction + mines_per_type[bo.mines].finished <= 1) {
					needed_type = BuildingNecessity::kNeeded;
				}
			}
			// if (bo.type == BuildingObserver::Type::kProductionsite) {
			// if (bo.produces_building_material || bo.max_needed_preciousness >= 10) {
			// if (bo.total_count() <= 1) {
			// needed_type = BuildingNecessity::kNeeded;
			//}
			//}
			//}
		}
	}

	// And finally the 'core' of this function
	// First deal with construction of new sites
	if (purpose == PerfEvaluation::kForConstruction) {
		if (bo.forced_after < gametime && bo.total_count() == 0 && !has_substitution_building) {
			bo.max_needed_preciousness = bo.max_preciousness;
			return BuildingNecessity::kForced;
		} else if (bo.prohibited_till > gametime) {
			return BuildingNecessity::kForbidden;
		} else if (bo.is_hunter || bo.is_fisher || bo.mines_water) {

			bo.cnt_target = 1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 40;

			if (bo.max_needed_preciousness == 0) {
				return BuildingNecessity::kNotNeeded;
			} else if (bo.cnt_under_construction + bo.unoccupied_count > 0) {
				return BuildingNecessity::kForbidden;
			} else if (bo.total_count() > 0 && new_buildings_stop_) {
				return BuildingNecessity::kForbidden;
			} else if (bo.total_count() >= bo.cnt_target &&
			           bo.last_building_built + 15 * 60 * 100 > gametime) {
				return BuildingNecessity::kForbidden;
			} else {
				return BuildingNecessity::kNeeded;
			}
		} else if (bo.need_trees) {
			if (bo.total_count() > 1 && (bo.cnt_under_construction + bo.unoccupied_count > 0)) {
				return BuildingNecessity::kForbidden;
			}
			bo.cnt_target = 3 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 20;

			// adjusting/decreasing based on cnt_limit_by_aimode
			bo.cnt_target = limit_cnt_target(bo.cnt_target, bo.cnt_limit_by_aimode);

			// for case the wood is not needed yet, to avoid inconsistency later on
			bo.max_needed_preciousness = bo.max_preciousness;

			if (bo.total_count() < bo.cnt_target) {
				return BuildingNecessity::kNeeded;
			} else {
				return BuildingNecessity::kAllowed;
			}
		} else if (bo.plants_trees) {
			int32_t value = management_data.neuron_pool[39].get_result_safe(
			   mines_.size() + productionsites.size() / 2, kAbsValue);
			value -= management_data.neuron_pool[40].get_result_safe(
			   persistent_data->trees_around_cutters / 10, kAbsValue);
			value -= management_data.neuron_pool[41].get_result_safe(
			   get_stocklevel(bo, gametime) / 5, kAbsValue);
			value -= management_data.neuron_pool[42].get_result_safe(bo.total_count(), kAbsValue) * 2;
			value += management_data.neuron_pool[43].get_result_safe(spots_ / 3);
			value += management_data.get_military_number_at(13);
			value /= 8;
			value = std::max(value, -3);

			bo.cnt_target = 5 + value;
			assert(bo.cnt_target > 1 && bo.cnt_target < 60);

			// adjusting/decreasing based on cnt_limit_by_aimode
			bo.cnt_target = limit_cnt_target(bo.cnt_target, bo.cnt_limit_by_aimode);

			// printf (" %d:  rangers target: %d, rangers %s\n",
			// player_number(),
			// bo.cnt_target,
			//(wood_policy_ == WoodPolicy::kAllowRangers)?"allowed":"not allowed");

			if (wood_policy_ != WoodPolicy::kAllowRangers) {
				return BuildingNecessity::kForbidden;
			}

			if (bo.total_count() > 1 && (bo.cnt_under_construction + bo.unoccupied_count > 0)) {
				return BuildingNecessity::kForbidden;
			} else if (bo.total_count() > bo.cnt_target) {
				return BuildingNecessity::kForbidden;
			}
			return BuildingNecessity::kNeeded;
		} else if (bo.need_rocks && bo.cnt_under_construction + bo.unoccupied_count == 0) {
			bo.max_needed_preciousness = bo.max_preciousness;  // even when rocks are not needed
			return BuildingNecessity::kAllowed;
		} else if (bo.production_hint >= 0 && bo.cnt_under_construction + bo.unoccupied_count == 0) {
			return BuildingNecessity::kAllowed;
			//} else if (bo.cnt_under_construction + bo.unoccupied_count > 0 &&
			// bo.max_needed_preciousness < 10) {
			// return BuildingNecessity::kForbidden;
		} else if (bo.cnt_under_construction + bo.unoccupied_count > 0 && gametime < 30 * 60 * 1000) {
			return BuildingNecessity::kForbidden;
		} else if (bo.cnt_under_construction + bo.unoccupied_count > 1) {
			return BuildingNecessity::kForbidden;  // for preciousness>=10 and after 30 min
		} else if (bo.type == BuildingObserver::Type::kMine) {
			if ((mines_per_type[bo.mines].in_construction + mines_per_type[bo.mines].finished) == 0) {
				// unless a mine is prohibited, we want to have at least one of the kind
				bo.max_needed_preciousness = bo.max_preciousness;
				return BuildingNecessity::kNeeded;
			} else if (((mines_per_type[bo.mines].in_construction +
			             mines_per_type[bo.mines].finished) == 1) &&
			           bo.produces_building_material) {
				bo.max_needed_preciousness = bo.max_preciousness;
				return BuildingNecessity::kNeeded;
			}
			if (bo.max_needed_preciousness == 0) {
				return BuildingNecessity::kNotNeeded;
			}
			if (bo.current_stats < 40) {
				return BuildingNecessity::kForbidden;
			}
			assert(bo.last_building_built != kNever);
			if (gametime < bo.last_building_built + 3 * 60 * 1000) {
				return BuildingNecessity::kForbidden;
			}
			return needed_type;
		}
		if (bo.max_needed_preciousness > 0) {

			// if (bo.cnt_under_construction + bo.unoccupied_count > 0) {
			// assert(bo.cnt_under_construction + bo.unoccupied_count == 1);
			// assert(bo.max_needed_preciousness >= 10 || bo.produces_building_material);
			// assert(gametime >= 25 * 60 * 1000);
			//}

			// Inputs
			// bo.cnt_under_construction + bo.unoccupied_count > 0
			// bo.max_needed_preciousness >= 10
			// bo.produces_building_material
			// gametime >= 25 * 60 * 1000
			// bo.total_count() == 0
			// bo.total_count() <= 1
			// bo.current_stats > 10
			// bo.outputs.empty()
			//! bo.outputs.empty() && bo.current_stats > 10 + 70 / bo.outputs.size()
			// bo.last_building_built != kNever && gametime < bo.last_building_built + 10 * 60 * 1000
			// needs_second_for_upgrade

			// NOCOM
			const bool nb_1 = management_data.f_neuron_pool[21].get_result(
			   bo.total_count() == 0, gametime >= 25 * 60 * 1000, bo.max_needed_preciousness >= 10,
			   !bo.outputs.empty() && bo.current_stats > 10 + 70 / bo.outputs.size(),
			   needs_second_for_upgrade);
			const bool nb_2 = management_data.f_neuron_pool[24].get_result(
			   bo.cnt_under_construction + bo.unoccupied_count > 0, bo.max_needed_preciousness >= 10,
			   bo.total_count() <= 1, bo.current_stats > 10,
			   bo.last_building_built != kNever && gametime < bo.last_building_built + 10 * 60 * 1000);

			const bool nb_3 = management_data.f_neuron_pool[22].get_result(
			   bo.cnt_under_construction + bo.unoccupied_count > 0, bo.max_needed_preciousness >= 10,
			   bo.produces_building_material, bo.current_stats > 10, bo.inputs.empty());

			const bool nb_4 = management_data.f_neuron_pool[25].get_result(
			   bo.cnt_under_construction + bo.unoccupied_count > 0, needs_second_for_upgrade,
			   wood_policy_ == WoodPolicy::kAllowRangers, gametime >= 15 * 60 * 1000,
			   bo.inputs.empty());

			const bool nb_5 = management_data.f_neuron_pool[23].get_result(nb_1, nb_2, nb_3, nb_4);

			if (nb_5) {
				// is either needed
				// if (bo.produces_building_material && bo.total_count() <= 1 && bo.current_stats > 10)
				// {
				return BuildingNecessity::kNeeded;
				//} else {
				// return needed_type;
				//}
			} else {
				return BuildingNecessity::kForbidden;
			}

			// if (bo.cnt_under_construction + bo.unoccupied_count > 0) {
			// assert(bo.cnt_under_construction + bo.unoccupied_count == 1);
			// assert(bo.max_needed_preciousness >= 10 || bo.produces_building_material);
			// assert(gametime >= 25 * 60 * 1000);
			//}

			//// First 'if' is special support for hardwood producers (to have 2 of them)
			// if (bo.produces_building_material && bo.total_count() <= 1 && bo.current_stats > 10) {
			// return BuildingNecessity::kNeeded;
			//} else if (bo.inputs.empty()) {
			// return needed_type;
			//} else if (bo.total_count() == 0) {
			// return needed_type;
			//} else if (!bo.outputs.empty() && bo.current_stats > 10 + 70 / bo.outputs.size()) {
			// assert(bo.last_building_built != kNever);
			// if (gametime < bo.last_building_built + 10 * 60 * 1000) {
			//// Previous building built less then 10 minutes ago
			//// Wait a bit, perhaps average utilization will drop down in the meantime
			// return BuildingNecessity::kNeededPending;
			//} else {
			// return needed_type;
			//}
			//} else if (needs_second_for_upgrade) {
			// return needed_type;
			//} else {
			// return BuildingNecessity::kForbidden;
			//}
		} else if (bo.is_shipyard) {
			if (bo.total_count() > 0) {
				return BuildingNecessity::kForbidden;
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
		} else if (!bo.outputs.empty() && bo.current_stats > (10 + 70 / bo.outputs.size()) / 2) {
			return BuildingNecessity::kNeeded;
		} else {
			return BuildingNecessity::kNotNeeded;
		}
	}
	NEVER_HERE();
}

// Now we can prohibit some militarysites, based on size, the goal is not to
// exhaust AI resources on the beginning of the game
// We count bigger buildings, medium ones get 1 points, big ones 2 points
// and we force some proportion to the number of military sites
// sidenote: function can return kNotNeeded, but it means 'not allowed'
BuildingNecessity DefaultAI::check_building_necessity(BuildingObserver& bo,
                                                      const uint32_t gametime) {

	assert(militarysites.size() == msites_built());
	// logically size of militarysite must in between 1 and 3 (including)
	const uint8_t size = bo.desc->get_size();
	assert(size >= BaseImmovable::SMALL && size <= BaseImmovable::BIG);

	if (military_last_build_ > gametime - 15 * 1000) {
		return BuildingNecessity::kForbidden;
	}

	// if (msites_in_constr() == 0 && (military_last_build_ +
	// std::abs(management_data.get_military_number_at(53)) * 6 * 1000 < gametime)) {
	// return BuildingNecessity::kAllowed;
	//}

	bo.primary_priority = 0;

	// if (soldier_status_ == SoldiersStatus::kFull ||
	// soldier_status_ == SoldiersStatus::kEnough) {
	// bo.primary_priority = (gametime - military_last_build_) / 1000 / 10; // 1 point for 10 seconds
	//}

	uint32_t const big_buildings_score =
	   msites_per_size[2].in_construction + msites_per_size[2].finished +
	   msites_per_size[3].in_construction * 2 + msites_per_size[3].finished * 2;

	const uint32_t msites_total = msites_built() + msites_in_constr();

	if (size == BaseImmovable::SMALL) {  // this function is intended for medium and bigger sites
		if (management_data.f_neuron_pool[11].get_result(
		       msites_in_constr() == 0, (military_last_build_ + 5 * 60 * 1000 > gametime),
		       big_buildings_score >
		          msites_total * 10 / (10 + management_data.get_military_number_at(14) / 20),
		       soldier_status_ == SoldiersStatus::kShortage ||
		          soldier_status_ == SoldiersStatus::kBadShortage)) {
			return BuildingNecessity::kNotNeeded;
		} else {
			return BuildingNecessity::kAllowed;
		}
	}

	// it would be convenient to have more inputs for FNeuron
	if (management_data.f_neuron_pool[9].get_result(
	       msites_in_constr() == 0, player_statistics.any_enemy_seen_lately(gametime),
	       spots_<kSpotsTooLittle, big_buildings_score> msites_total * 10 /
	          (10 + management_data.get_military_number_at(64) / 20),
	       soldier_status_ == SoldiersStatus::kShortage ||
	          soldier_status_ == SoldiersStatus::kBadShortage)) {
		return BuildingNecessity::kNotNeeded;
	} else {
		return BuildingNecessity::kAllowed;
	}
}

// counts produced output on stock
// if multiple outputs, it returns lowest value
uint32_t DefaultAI::calculate_stocklevel(BuildingObserver& bo) {
	uint32_t count = std::numeric_limits<uint32_t>::max();

	for (uint32_t m = 0; m < bo.outputs.size(); ++m) {
		DescriptionIndex wt(static_cast<size_t>(bo.outputs.at(m)));
		const uint32_t stock = calculate_stocklevel(wt);
		if (count > stock) {
			count = stock;
		}
	}

	return count;
}

// counts produced output on stock
// if multiple outputs, it returns lowest value
uint32_t DefaultAI::calculate_stocklevel(Widelands::DescriptionIndex wt) {
	uint32_t count = 0;

	for (std::list<WarehouseSiteObserver>::iterator i = warehousesites.begin();
	     i != warehousesites.end(); ++i) {
		count += i->site->get_wares().stock(wt);
	}

	return count;
}

// This is wrapper function to prevent too frequent recalculation of stocklevel
// and distinquish if we count stocks for production hint or for outputs of a productionsite
uint32_t DefaultAI::get_stocklevel(BuildingObserver& bo, const uint32_t gametime) {
	if (bo.stocklevel_time < gametime - 5 * 1000) {
		if (bo.production_hint > 0) {
			bo.stocklevel_count = calculate_stocklevel(static_cast<size_t>(bo.production_hint));
		} else if (!bo.outputs.empty()) {
			bo.stocklevel_count = calculate_stocklevel(bo);
		} else {
			bo.stocklevel_count = 0;
		}
		bo.stocklevel_time = gametime;
	}
	return bo.stocklevel_count;
}

// this just counts free positions in military and training sites
void DefaultAI::count_military_vacant_positions(const uint32_t gametime) {
	// counting vacant positions
	int32_t vacant_mil_positions_ = 0;
	for (TrainingSiteObserver tso : trainingsites) {
		vacant_mil_positions_ +=
		   5 * std::min<int32_t>(
		          (tso.site->soldier_capacity() - tso.site->stationed_soldiers().size()), 2);
	}
	for (MilitarySiteObserver mso : militarysites) {
		vacant_mil_positions_ += mso.site->soldier_capacity() - mso.site->stationed_soldiers().size();
	}

	SoldiersStatus soldier_status_tmp;

	if (vacant_mil_positions_ <= 1) {
		soldier_status_tmp = SoldiersStatus::kFull;
	} else if (vacant_mil_positions_ * 4 <= static_cast<int32_t>(militarysites.size())) {
		soldier_status_tmp = SoldiersStatus::kEnough;
	} else if (vacant_mil_positions_ > static_cast<int32_t>(militarysites.size())) {
		soldier_status_tmp = SoldiersStatus::kBadShortage;
	} else {
		soldier_status_tmp = SoldiersStatus::kShortage;
	}

	// Never increase soldier status too soon
	if (soldier_status_tmp >= soldier_status_) {
		soldier_status_ = soldier_status_tmp;
		military_status_last_updated = gametime;
	} else if (soldier_status_tmp < soldier_status_ &&
	           military_status_last_updated +
	                 std::abs(management_data.get_military_number_at(60)) * 60 * 1000 / 10 <
	              gametime) {
		printf("%d / %d: finaly increasing soldier status %d -> %d\n", player_number(),
		       gametime / 1000, soldier_status_, soldier_status_tmp);
		soldier_status_ = soldier_status_tmp;
	}
}

// this function only check with trainingsites
// manipulates input queues and soldier capacity
bool DefaultAI::check_trainingsites(uint32_t gametime) {

	if (trainingsites.empty()) {
		set_taskpool_task_time(
		   gametime + 2 * kTrainingSitesCheckInterval, SchedulerTaskId::kCheckTrainingsites);
		return false;
	}

	trainingsites.push_back(trainingsites.front());
	trainingsites.pop_front();

	TrainingSite* ts = trainingsites.front().site;
	TrainingSiteObserver& tso = trainingsites.front();

	const DescriptionIndex enhancement = ts->descr().enhancement();

	if (enhancement != INVALID_INDEX && ts_without_trainers_ == 0 && mines_.size() > 3 &&
	    (ts_basic_const_count_ + ts_advanced_const_count_) == 0 && ts_advanced_count_ > 0) {

		if (player_->is_building_type_allowed(enhancement)) {
			game().send_player_enhance_building(*tso.site, enhancement);
		}
	}

	// changing capacity to 0 - this will happen only once.....
	if (tso.site->soldier_capacity() > 1) {
		game().send_player_change_soldier_capacity(*ts, -tso.site->soldier_capacity());
		return true;
	}

	// reducing ware queues
	// - for armours and weapons to 1
	// - for others to 6
	std::vector<WaresQueue*> const warequeues1 = tso.site->warequeues();
	size_t nr_warequeues = warequeues1.size();
	for (size_t i = 0; i < nr_warequeues; ++i) {

		// if it was decreased yet
		if (warequeues1[i]->get_max_fill() <= 1) {
			continue;
		}

		// now modifying max_fill of armors and weapons
		for (std::string pattern : armors_and_weapons) {

			if (tribe_->get_ware_descr(warequeues1[i]->get_ware())->name().find(pattern) !=
			    std::string::npos) {
				if (warequeues1[i]->get_max_fill() > 1) {
					game().send_player_set_ware_max_fill(*ts, warequeues1[i]->get_ware(), 1);
					continue;
				}
			}
		}
	}

	// changing priority if basic
	if (tso.bo->trainingsite_type == TrainingSiteType::kBasic) {
		for (uint32_t k = 0; k < tso.bo->inputs.size(); ++k) {
			game().send_player_set_ware_priority(*ts, wwWARE, tso.bo->inputs.at(k), HIGH_PRIORITY);
		}
	}

	// are we willing to train another soldier
	// bool want_train = true;
	const PlayerNumber pn = player_number();

	// if soldier capacity is set to 0, we need to find out if the site is
	// supplied enough to incrase the capacity to 1
	if (tso.site->soldier_capacity() == 0) {

		// First subsitute wares
		int32_t filled = 0;
		// We call a soldier to a trainingsite only if it is stocked. Shortage is deficit of wares
		// Generally we accept shortage 1, but if training is stalled (no trained soldier in last 1
		// minutes)
		// we can accept also shortage up to 3
		int32_t shortage = 0;
		std::vector<WaresQueue*> const warequeues2 = tso.site->warequeues();
		nr_warequeues = warequeues2.size();
		for (size_t i = 0; i < nr_warequeues; ++i) {
			if (tso.bo->substitute_inputs.count(warequeues2[i]->get_ware()) > 0) {
				filled += warequeues2[i]->get_filled();
			}
		}
		if (filled < 5) {
			shortage += 5 - filled;
		}

		// checking non subsitutes
		for (size_t i = 0; i < nr_warequeues; ++i) {
			if (tso.bo->substitute_inputs.count(warequeues2[i]->get_ware()) == 0) {
				const uint32_t required_amount =
				   (warequeues2[i]->get_max_fill() < 5) ? warequeues2[i]->get_max_fill() : 5;
				if (warequeues2[i]->get_filled() < required_amount) {
					shortage += required_amount - warequeues2[i]->get_filled();
				}
			}
		}

		if (shortage <= 3) {  // training only if supplied

			const bool i1 = player_statistics.get_player_power(pn) -
			                   player_statistics.get_old_player_power(pn) -
			                   (player_statistics.get_visible_enemies_power(gametime) -
			                    player_statistics.get_old_visible_enemies_power(gametime)) >
			                0;
			const bool i2 =
			   static_cast<int16_t>(player_statistics.get_visible_enemies_power(gametime) -
			                        player_statistics.get_old_visible_enemies_power(gametime)) >
			   management_data.get_military_number_at(34) / 10;
			const bool i3 = player_statistics.get_player_power(pn) * 2 >
			                player_statistics.get_visible_enemies_power(gametime);
			const bool i4 = player_statistics.any_enemy_seen_lately(gametime);
			const bool i5 =
			   soldier_status_ == SoldiersStatus::kEnough || soldier_status_ == SoldiersStatus::kFull;

			const bool i6 = mines_.size() < 3;
			const bool i7 = shortage <= 1;
			const bool i8 = persistent_data->last_soldier_trained < gametime;
			const bool i9 = persistent_data->last_soldier_trained == kNever;

			const bool ts_1 = management_data.f_neuron_pool[26].get_result(i1, i3, i6, i8, i9);
			const bool ts_2 = management_data.f_neuron_pool[27].get_result(i2, i4, i5, i7, i9);
			const bool ts_3 = management_data.f_neuron_pool[28].get_result(i1, i3, i4, i5, i7);

			const bool wanna_train = management_data.f_neuron_pool[29].get_result(ts_1, ts_2, ts_3);

			if (wanna_train) {
				game().send_player_change_soldier_capacity(*ts, 1);
			}
		}

		//// Has any soldier been trained up to now?
		// if (gametime > 10 * 60 * 1000 && persistent_data->last_soldier_trained < gametime) {
		// if (persistent_data->last_soldier_trained + 10 * 60 * 1000 < gametime) {
		// if (shortage <= 3) {
		// game().send_player_change_soldier_capacity(*ts, 1);
		//}
		//} else {
		// if (shortage <= 1) {
		// game().send_player_change_soldier_capacity(*ts, 1);
		//}
		//}
		//// or this can be very first soldier to be trained
		//} else if (shortage <= 3) {
		// game().send_player_change_soldier_capacity(*ts, 1);
		//}
	}

	ts_without_trainers_ = 0;  // zeroing
	for (std::list<TrainingSiteObserver>::iterator site = trainingsites.begin();
	     site != trainingsites.end(); ++site) {

		if (!site->site->can_start_working()) {
			ts_without_trainers_ += 1;
		}
	}
	return true;
}

/**
 * Updates the first military building in list and reenques it at the end of
 * the list afterwards. If a militarysite is in secure area but holds more than
 * one soldier, the number of stationed soldiers is decreased. If the building
 * is near a border, the number of stationed soldiers is maximized
 *
 * \returns true if something was changed
 */
bool DefaultAI::check_militarysites(uint32_t gametime) {

	// Only useable, if defaultAI owns at least one militarysite
	if (militarysites.empty()) {
		return false;
	}

	const bool verbose = false;

	// Check next militarysite
	bool changed = false;
	Map& map = game().map();
	MilitarySite* ms = militarysites.front().site;
	// MilitarySiteObserver& mso = militarysites.front();
	// uint32_t const vision = ms->descr().vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	// look if there are any enemies building
	FindNodeEnemiesBuilding find_enemy(player_, game());

	int16_t usefullness_score = 0;
	BuildableField bf(f);
	if (military_last_dismantle_ + 2 * 60 * 1000 < gametime) {  // 2 minutes between every dismantle
		update_buildable_field(bf);

		usefullness_score += bf.military_score_;

		if (verbose && (bf.enemy_nearby)) {
			printf(" %d MSite: at %3dx%3d, enemy: %s / accessible: %s, usefulness score: %4d\n",
			       player_number(), f.x, f.y, (bf.enemy_nearby) ? "Y" : "N",
			       (bf.enemy_accessible_) ? "Y" : "N", usefullness_score);
		}
	}

	if (usefullness_score < 0 && (gametime - militarysites.front().built_time) > 2 * 60 * 1000) {
		changed = true;
		if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
			flags_to_be_removed.push_back(ms->base_flag().get_position());
			game().send_player_dismantle(*ms);
			military_last_dismantle_ = game().get_gametime();
		} else {
			game().send_player_bulldoze(*ms);
			military_last_dismantle_ = game().get_gametime();
		}
	} else if (!bf.enemy_accessible_) {
		uint32_t const j = ms->soldier_capacity();

		if (MilitarySite::kPrefersRookies != ms->get_soldier_preference()) {
			game().send_player_militarysite_set_soldier_preference(*ms, MilitarySite::kPrefersRookies);
		} else if (j > 1) {
			game().send_player_change_soldier_capacity(*ms, (j > 2) ? -2 : -1);
		}
	} else {
		// set preferHeroes and modify number of soldiers
		Quantity const total_capacity = ms->max_soldier_capacity();
		Quantity const target_capacity = ms->soldier_capacity();

		if (total_capacity != target_capacity && (soldier_status_ == SoldiersStatus::kEnough ||
		                                          soldier_status_ == SoldiersStatus::kFull)) {
			game().send_player_change_soldier_capacity(*ms, total_capacity - target_capacity);
			changed = true;
		}

		if (soldier_status_ == SoldiersStatus::kShortage && target_capacity == 1 &&
		    total_capacity > 1) {
			game().send_player_change_soldier_capacity(*ms, +1);
			changed = true;
		}

		if (soldier_status_ == SoldiersStatus::kShortage &&
		    target_capacity > total_capacity / 2 + 1) {
			game().send_player_change_soldier_capacity(*ms, -1);
			changed = true;
		}

		if (soldier_status_ == SoldiersStatus::kBadShortage && target_capacity > 1) {
			game().send_player_change_soldier_capacity(*ms, -1);
			changed = true;
		}

		// and also set preference to Heroes
		if (MilitarySite::kPrefersHeroes != ms->get_soldier_preference()) {
			game().send_player_militarysite_set_soldier_preference(*ms, MilitarySite::kPrefersHeroes);
			changed = true;
		}
	}

	// reorder:;
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();
	return changed;
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
			// prio += 3 * (spots_ - kSpotsEnough);
		}
	}

	return prio;
}
// for buildable field, it considers effect of building of type bo on position coords
void DefaultAI::consider_productionsite_influence(BuildableField& field,
                                                  Coords coords,
                                                  const BuildingObserver& bo) {
	if (bo.space_consumer && !bo.plants_trees &&
	    game().map().calc_distance(coords, field.coords) < 8) {
		++field.space_consumers_nearby;
	}

	for (size_t i = 0; i < bo.inputs.size(); ++i) {
		++field.consumers_nearby.at(bo.inputs.at(i));
	}

	for (size_t i = 0; i < bo.outputs.size(); ++i) {
		++field.producers_nearby.at(bo.outputs.at(i));
	}

	if (bo.production_hint >= 0) {
		++field.supporters_nearby.at(bo.production_hint);
	}

	if (bo.plants_trees) {
		++field.rangers_nearby;
	}
}

/// \returns the economy observer containing \arg economy
EconomyObserver* DefaultAI::get_economy_observer(Economy& economy) {
	for (std::list<EconomyObserver*>::iterator i = economies.begin(); i != economies.end(); ++i)
		if (&(*i)->economy == &economy)
			return *i;

	economies.push_front(new EconomyObserver(economy));
	return economies.front();
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

// this is called whenever we gain ownership of a PlayerImmovable
void DefaultAI::gain_immovable(PlayerImmovable& pi, const bool found_on_load) {
	if (upcast(Building, building, &pi)) {
		gain_building(*building, found_on_load);
	} else if (upcast(Flag const, flag, &pi)) {
		new_flags.push_back(flag);
	} else if (upcast(Road const, road, &pi)) {
		roads.push_front(road);
	}
}

// this is called whenever we gain ownership of a Ship
void DefaultAI::gain_ship(Ship& ship, NewShip type) {

	allships.push_back(ShipObserver());
	allships.back().ship = &ship;
	if (game().get_gametime() % 2 == 0) {
		allships.back().island_circ_direction = IslandExploreDirection::kClockwise;
	} else {
		allships.back().island_circ_direction = IslandExploreDirection::kCounterClockwise;
	}

	if (type == NewShip::kBuilt) {
		marine_task_queue.push_back(kStopShipyard);
	} else {
		seafaring_economy = true;
		if (ship.state_is_expedition()) {
			if (expedition_ship_ == kNoShip) {
				// OK, this ship is in expedition
				expedition_ship_ = ship.serial();
			} else {
				// What? Another ship in expedition? AI is not able to manage two expedition ships...
				log(" %d: AI will not control ship %s, as there is already another one in expedition\n",
				    player_number(), ship.get_shipname().c_str());
			}
		}
	}
}

// this is called whenever we lose ownership of a PlayerImmovable
void DefaultAI::lose_immovable(const PlayerImmovable& pi) {
	if (upcast(Building const, building, &pi)) {
		lose_building(*building);
	} else if (upcast(Flag const, flag, &pi)) {
		for (EconomyObserver* eco_obs : economies) {
			for (std::list<Flag const*>::iterator flag_iter = eco_obs->flags.begin();
			     flag_iter != eco_obs->flags.end(); ++flag_iter) {
				if (*flag_iter == flag) {
					eco_obs->flags.erase(flag_iter);
					return;
				}
			}
		}
		for (std::list<Flag const*>::iterator flag_iter = new_flags.begin();
		     flag_iter != new_flags.end(); ++flag_iter) {
			if (*flag_iter == flag) {
				new_flags.erase(flag_iter);
				return;
			}
		}
	} else if (upcast(Road const, road, &pi)) {
		roads.remove(road);
	}
}

// this is called when a mine reports "out of resources"
void DefaultAI::out_of_resources_site(const ProductionSite& site) {

	const uint32_t gametime = game().get_gametime();

	// we must identify which mine matches the productionsite a note reffers to
	for (std::list<ProductionSiteObserver>::iterator i = mines_.begin(); i != mines_.end(); ++i)
		if (i->site == &site) {
			if (i->no_resources_since > gametime) {
				i->no_resources_since = gametime;
			}
			break;
		}
}

// This is called when soldier left the trainingsite
// the purpose is to set soldier capacity to 0
// (AI will then wait till training site is stocked)
void DefaultAI::soldier_trained(const TrainingSite& site) {

	persistent_data->last_soldier_trained = game().get_gametime();

	// printf (" %d: soldier just trained (%d)\n", player_number(),
	// persistent_data->last_soldier_trained / 1000); NOCOM

	for (TrainingSiteObserver& trainingsite_obs : trainingsites) {
		if (trainingsite_obs.site == &site) {
			if (trainingsite_obs.site->soldier_capacity() > 0) {
				game().send_player_change_soldier_capacity(
				   *trainingsite_obs.site, -trainingsite_obs.site->soldier_capacity());
			}
			return;
		}
	}

	log(" %d: Computer player error - trainingsite not found\n", player_number());
}

// walk and search for territory controlled by other player
// usually scanning radius is enough but sometimes we must walk to
// verify that an enemy territory is really accessible by land
bool DefaultAI::other_player_accessible(const uint32_t max_distance,
                                        uint32_t* tested_fields,
                                        uint16_t* mineable_fields_count,
                                        const Coords& starting_spot,
                                        const WalkSearch& type) {
	Map& map = game().map();
	std::list<uint32_t> queue;
	std::unordered_set<uint32_t> done;
	queue.push_front(starting_spot.hash());
	PlayerNumber const pn = player_->player_number();

	while (!queue.empty()) {
		// if already processed
		if (done.count(queue.front()) > 0) {
			queue.pop_front();
			continue;
		}

		done.insert(queue.front());

		Coords tmp_coords = Coords::unhash(queue.front());

		// if beyond range
		if (map.calc_distance(starting_spot, tmp_coords) > max_distance) {
			continue;
		}

		Field* f = map.get_fcoords(tmp_coords).field;

		// not interested if not walkable (starting spot is an exemption.
		if (tmp_coords != starting_spot && !(f->nodecaps() & MOVECAPS_WALK)) {
			continue;
		}

		// sometimes we search for any owned territory (f.e. when considering
		// a port location), but when testing (starting from) own military building
		// we must ignore own territory, of course
		// NEW HERE
		const PlayerNumber field_owner = f->get_owned_by();
		if (field_owner > 0) {

			// if field is owned by anybody
			if (type == WalkSearch::kAnyPlayer) {
				*tested_fields = done.size();
				return true;
			}

			// if anybody but not me
			if (type == WalkSearch::kOtherPlayers && field_owner != pn) {
				*tested_fields = done.size();
				return true;
			}

			// if owned by enemy
			if (type == WalkSearch::kEnemy && field_owner != pn) {
				// NEW HERE
				// if not in the same taem => it is enemy
				if (!player_statistics.players_in_same_team(pn, field_owner)) {
					*tested_fields = done.size();
					return true;
				}
			}
		}

		// increase mines counter
		// (used when testing possible port location)
		if (f->nodecaps() & BUILDCAPS_MINE) {
			mineable_fields_count += 1;
		};

		// add neighbours to a queue (duplicates are no problem)
		// to relieve AI/CPU we skip every second field in each direction
		// obstacles are usually wider then one field
		for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			Coords neigh_coords1;
			map.get_neighbour(tmp_coords, dir, &neigh_coords1);
			Coords neigh_coords2;
			map.get_neighbour(neigh_coords1, dir, &neigh_coords2);
			queue.push_front(neigh_coords2.hash());
		}
	}
	*tested_fields = done.size();
	return false;  // no players found
}

// this scores spot for potential colony
uint8_t DefaultAI::spot_scoring(Widelands::Coords candidate_spot) {

	uint8_t score = 0;
	uint16_t mineable_fields_count = 0;
	uint32_t tested_fields = 0;

	// On the beginning we search for completely deserted area,
	// but later we will accept also area adjacent to own teritorry
	WalkSearch search_type = WalkSearch::kAnyPlayer;
	if (persistent_data->colony_scan_area < 25) {
		search_type = WalkSearch::kEnemy;
	}

	const bool other_player =
	   other_player_accessible(persistent_data->colony_scan_area, &tested_fields,
	                           &mineable_fields_count, candidate_spot, search_type);

	// if we run into other player
	// (maybe we should check for enemies, rather?)
	if (other_player) {
		return 0;
	}

	Map& map = game().map();
	// If the available area (island) is too small...
	// colony_scan_area is a radius (distance) and has no direct relevance to the size of area,
	// but it seems a good measurement
	if (tested_fields < persistent_data->colony_scan_area) {
		return 0;
	}

	// if we are here we put score
	score = 1;
	if (mineable_fields_count > 0) {
		score += 1;
	}

	// here we check for surface rocks + trees
	std::vector<ImmovableFound> immovables;
	// Search in a radius of range
	map.find_immovables(Area<FCoords>(map.get_fcoords(candidate_spot), 10), &immovables);

	int32_t const rocks_attr = MapObjectDescr::get_attribute_id("rocks");
	uint16_t rocks = 0;
	int32_t const tree_attr = MapObjectDescr::get_attribute_id("tree");
	uint16_t trees = 0;

	for (uint32_t j = 0; j < immovables.size(); ++j) {
		if (immovables.at(j).object->has_attribute(rocks_attr)) {
			++rocks;
		}
		if (immovables.at(j).object->has_attribute(tree_attr)) {
			++trees;
		}
	}
	if (rocks > 1) {
		score += 1;
	}
	if (trees > 1) {
		score += 1;
	}

	return score;
}

// this is called whenever ship received a notification that requires
// navigation decisions (these notifiation are processes not in 'real time')
void DefaultAI::expedition_management(ShipObserver& so) {

	Map& map = game().map();
	const int32_t gametime = game().get_gametime();

	// second we put current spot into visited_spots
	bool first_time_here = false;
	if (so.visited_spots.count(so.ship->get_position().hash()) == 0) {
		first_time_here = true;
		so.visited_spots.insert(so.ship->get_position().hash());
	}

	// If we have portspace following options are avaiable:
	// 1. Build a port there
	if (!so.ship->exp_port_spaces().empty()) {  // making sure we have possible portspaces

		// we score the place
		const uint8_t spot_score = spot_scoring(so.ship->exp_port_spaces().front());

		if ((gametime / 10) % 8 < spot_score) {  // we build a port here
			game().send_player_ship_construct_port(*so.ship, so.ship->exp_port_spaces().front());
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;
			// blocking the area for some time to save AI from idle attempts to built there
			// buildings
			// TODO(TiborB): how long it takes to build a port?
			// I used 5 minutes
			MapRegion<Area<FCoords>> mr(
			   game().map(), Area<FCoords>(map.get_fcoords(so.ship->exp_port_spaces().front()), 8));
			do {
				blocked_fields.add(mr.location(), game().get_gametime() + 5 * 60 * 1000);
			} while (mr.advance(map));

			return;
		}
	}

	// 2. Go on with expedition

	if (first_time_here) {
		game().send_player_ship_explore_island(*so.ship, so.island_circ_direction);
		so.last_command_time = gametime;
		so.waiting_for_command_ = false;

		// we was here but to add randomnes we might continue with expedition
	} else if (gametime % 2 == 0) {
		game().send_player_ship_explore_island(*so.ship, so.island_circ_direction);
		so.last_command_time = gametime;
		so.waiting_for_command_ = false;
	} else {
		// get swimmable directions
		std::vector<Direction> possible_directions;
		for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {

			// testing distance of 8 fields
			// this would say there is an 'open sea' there
			Widelands::FCoords tmp_fcoords = map.get_fcoords(so.ship->get_position());
			for (int8_t i = 0; i < 8; ++i) {
				tmp_fcoords = map.get_neighbour(tmp_fcoords, dir);
				if (tmp_fcoords.field->nodecaps() & MOVECAPS_SWIM) {
					if (i == 7) {
						possible_directions.push_back(dir);
						break;  // not needed but.....
					}
				} else {
					break;
				}
			}
		}

		// we test if there is open sea
		if (possible_directions.empty()) {
			// 2.A No there is no open sea
			game().send_player_ship_explore_island(*so.ship, so.island_circ_direction);
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;
			;
		} else {
			// 2.B Yes, pick one of avaliable directions
			const Direction final_direction =
			   possible_directions.at(gametime % possible_directions.size());
			game().send_player_ship_scouting_direction(
			   *so.ship, static_cast<WalkingDir>(final_direction));
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;
		}
	}
	return;
}

// this is called whenever we gain a new building
void DefaultAI::gain_building(Building& b, const bool found_on_load) {

	BuildingObserver& bo = get_building_observer(b.descr().name().c_str());

	if (bo.type == BuildingObserver::Type::kConstructionsite) {
		BuildingObserver& target_bo =
		   get_building_observer(dynamic_cast<const ConstructionSite&>(b).building().name().c_str());
		++target_bo.cnt_under_construction;
		if (target_bo.type == BuildingObserver::Type::kProductionsite) {
			++num_prod_constructionsites;
		}
		if (target_bo.type == BuildingObserver::Type::kMilitarysite) {
			msites_per_size[target_bo.desc->get_size()].in_construction += 1;
		}
		if (target_bo.type == BuildingObserver::Type::kMine) {
			mines_per_type[target_bo.mines].in_construction += 1;
		}
		if (target_bo.type == BuildingObserver::Type::kTrainingsite) {
			if (target_bo.trainingsite_type == TrainingSiteType::kBasic) {
				ts_basic_const_count_ += 1;
			}
			if (target_bo.trainingsite_type == TrainingSiteType::kAdvanced) {
				ts_advanced_const_count_ += 1;
			}
		}

		set_taskpool_task_time(game().get_gametime(), SchedulerTaskId::kRoadCheck);

	} else {
		++bo.cnt_built;
		const uint32_t gametime = game().get_gametime();
		bo.last_building_built = gametime;

		if (bo.type == BuildingObserver::Type::kProductionsite) {
			productionsites.push_back(ProductionSiteObserver());
			productionsites.back().site = &dynamic_cast<ProductionSite&>(b);
			productionsites.back().bo = &bo;
			productionsites.back().bo->new_building_overdue = 0;
			if (found_on_load && gametime > 5 * 60 * 1000) {
				productionsites.back().built_time = gametime - 5 * 60 * 1000;
			} else {
				productionsites.back().built_time = gametime;
			}
			productionsites.back().unoccupied_till = gametime;
			productionsites.back().stats_zero = 0;
			productionsites.back().no_resources_since = kNever;
			productionsites.back().upgrade_pending = false;
			productionsites.back().bo->unoccupied_count += 1;
			if (bo.is_shipyard) {
				marine_task_queue.push_back(kStopShipyard);
				marine_task_queue.push_back(kReprioritize);
			}

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				++wares.at(bo.outputs.at(i)).producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				++wares.at(bo.inputs.at(i)).consumers;
		} else if (bo.type == BuildingObserver::Type::kMine) {
			mines_.push_back(ProductionSiteObserver());
			mines_.back().site = &dynamic_cast<ProductionSite&>(b);
			mines_.back().bo = &bo;
			mines_.back().built_time = gametime;
			mines_.back().no_resources_since = kNever;
			mines_.back().bo->unoccupied_count += 1;

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				++wares.at(bo.outputs.at(i)).producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				++wares.at(bo.inputs.at(i)).consumers;

			mines_per_type[bo.mines].finished += 1;

		} else if (bo.type == BuildingObserver::Type::kMilitarysite) {
			militarysites.push_back(MilitarySiteObserver());
			militarysites.back().site = &dynamic_cast<MilitarySite&>(b);
			militarysites.back().bo = &bo;
			militarysites.back().checks = bo.desc->get_size();
			if (found_on_load && gametime > 5 * 60 * 1000) {
				militarysites.back().built_time = gametime - 5 * 60 * 1000;
			} else {
				militarysites.back().built_time = gametime;
			}
			militarysites.back().enemies_nearby = true;
			msites_per_size[bo.desc->get_size()].finished += 1;

		} else if (bo.type == BuildingObserver::Type::kTrainingsite) {
			ts_without_trainers_ += 1;
			trainingsites.push_back(TrainingSiteObserver());
			trainingsites.back().site = &dynamic_cast<TrainingSite&>(b);
			trainingsites.back().bo = &bo;
			if (bo.trainingsite_type == TrainingSiteType::kBasic) {
				ts_basic_count_ += 1;
			}
			if (bo.trainingsite_type == TrainingSiteType::kAdvanced) {
				ts_advanced_count_ += 1;
			}

		} else if (bo.type == BuildingObserver::Type::kWarehouse) {
			++numof_warehouses_;
			warehousesites.push_back(WarehouseSiteObserver());
			warehousesites.back().site = &dynamic_cast<Warehouse&>(b);
			warehousesites.back().bo = &bo;
			if (bo.is_port) {
				++num_ports;
				seafaring_economy = true;
				// unblock nearby fields, might be used for other buildings...
				Map& map = game().map();
				MapRegion<Area<FCoords>> mr(
				   map, Area<FCoords>(map.get_fcoords(warehousesites.back().site->get_position()), 3));
				do {
					const int32_t hash = map.get_fcoords(*(mr.location().field)).hash();
					if (port_reserved_coords.count(hash) > 0)
						port_reserved_coords.erase(hash);
				} while (mr.advance(map));
			}
		}
	}
}

// this is called whenever we lose a building
void DefaultAI::lose_building(const Building& b) {

	BuildingObserver& bo = get_building_observer(b.descr().name().c_str());

	if (bo.type == BuildingObserver::Type::kConstructionsite) {
		BuildingObserver& target_bo =
		   get_building_observer(dynamic_cast<const ConstructionSite&>(b).building().name().c_str());
		--target_bo.cnt_under_construction;
		if (target_bo.type == BuildingObserver::Type::kProductionsite) {
			--num_prod_constructionsites;
		}
		if (target_bo.type == BuildingObserver::Type::kMilitarysite) {
			msites_per_size[target_bo.desc->get_size()].in_construction -= 1;
		}
		if (target_bo.type == BuildingObserver::Type::kMine) {
			mines_per_type[target_bo.mines].in_construction -= 1;
		}
		if (target_bo.type == BuildingObserver::Type::kTrainingsite) {
			if (target_bo.trainingsite_type == TrainingSiteType::kBasic) {
				ts_basic_const_count_ -= 1;
				assert(ts_basic_const_count_ >= 0);
			}
			if (target_bo.trainingsite_type == TrainingSiteType::kAdvanced) {
				ts_advanced_const_count_ -= 1;
				assert(ts_advanced_const_count_ >= 0);
			}
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

			for (std::list<ProductionSiteObserver>::iterator i = productionsites.begin();
			     i != productionsites.end(); ++i)
				if (i->site == &b) {
					if (i->upgrade_pending) {
						bo.cnt_upgrade_pending -= 1;
					}
					assert(bo.cnt_upgrade_pending == 0 || bo.cnt_upgrade_pending == 1);
					productionsites.erase(i);
					break;
				}

			for (uint32_t i = 0; i < bo.outputs.size(); ++i) {
				--wares.at(bo.outputs.at(i)).producers;
			}

			for (uint32_t i = 0; i < bo.inputs.size(); ++i) {
				--wares.at(bo.inputs.at(i)).consumers;
			}

		} else if (bo.type == BuildingObserver::Type::kMine) {
			for (std::list<ProductionSiteObserver>::iterator i = mines_.begin(); i != mines_.end();
			     ++i) {
				if (i->site == &b) {
					mines_.erase(i);
					break;
				}
			}

			for (uint32_t i = 0; i < bo.outputs.size(); ++i) {
				--wares.at(bo.outputs.at(i)).producers;
			}

			for (uint32_t i = 0; i < bo.inputs.size(); ++i) {
				--wares.at(bo.inputs.at(i)).consumers;
			}

			mines_per_type[bo.mines].finished -= 1;

		} else if (bo.type == BuildingObserver::Type::kMilitarysite) {
			msites_per_size[bo.desc->get_size()].finished -= 1;

			for (std::list<MilitarySiteObserver>::iterator i = militarysites.begin();
			     i != militarysites.end(); ++i) {
				if (i->site == &b) {
					militarysites.erase(i);
					break;
				}
			}

		} else if (bo.type == BuildingObserver::Type::kTrainingsite) {

			for (std::list<TrainingSiteObserver>::iterator i = trainingsites.begin();
			     i != trainingsites.end(); ++i) {
				if (i->site == &b) {
					trainingsites.erase(i);
					if (bo.trainingsite_type == TrainingSiteType::kBasic) {
						ts_basic_count_ -= 1;
						assert(ts_basic_count_ >= 0);
					}
					if (bo.trainingsite_type == TrainingSiteType::kAdvanced) {
						ts_advanced_count_ -= 1;
						assert(ts_advanced_count_ >= 0);
					}
					break;
				}
			}

		} else if (bo.type == BuildingObserver::Type::kWarehouse) {
			assert(numof_warehouses_ > 0);
			--numof_warehouses_;
			if (bo.is_port) {
				--num_ports;
			}

			for (std::list<WarehouseSiteObserver>::iterator i = warehousesites.begin();
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
// NOTE: This is not needed anymore and it seems it is not missed neither
bool DefaultAI::check_supply(const BuildingObserver& bo) {
	size_t supplied = 0;
	for (const Widelands::DescriptionIndex& temp_inputs : bo.inputs) {
		for (const BuildingObserver& temp_building : buildings_) {
			if (temp_building.cnt_built &&
			    std::find(temp_building.outputs.begin(), temp_building.outputs.end(), temp_inputs) !=
			       temp_building.outputs.end() &&
			    check_supply(temp_building)) {
				++supplied;
				break;
			}
		}
	}

	return supplied == bo.inputs.size();
}

// This calculates strength of vector of soldiers, f.e. soldiers in a building or
// ones ready to attack
int32_t DefaultAI::calculate_strength(const std::vector<Widelands::Soldier*>& soldiers) {

	if (soldiers.empty()) {
		return 0;
	}

	Tribes tribe = Tribes::kNone;

	if (soldiers.at(0)->get_owner()->tribe().name() == "atlanteans") {
		tribe = Tribes::kAtlanteans;
	} else if (soldiers.at(0)->get_owner()->tribe().name() == "barbarians") {
		tribe = Tribes::kBarbarians;
	} else if (soldiers.at(0)->get_owner()->tribe().name() == "empire") {
		tribe = Tribes::kEmpire;
	} else {
		throw wexception("AI warning: Unable to calculate strenght for player of tribe %s",
		                 soldiers.at(0)->get_owner()->tribe().name().c_str());
	}

	float health = 0;
	float attack = 0;
	float defense = 0;
	float evade = 0;
	float final = 0;

	for (Soldier* soldier : soldiers) {
		switch (tribe) {
		case (Tribes::kAtlanteans):
			health = 135 + 40 * soldier->get_health_level();
			attack = 14 + 8 * soldier->get_attack_level();
			defense = static_cast<float>(94 - 8 * soldier->get_defense_level()) / 100;
			evade = static_cast<float>(70 - 17 * soldier->get_evade_level()) / 100;
			break;
		case (Tribes::kBarbarians):
			health += 130 + 28 * soldier->get_health_level();
			attack += 14 + 7 * soldier->get_attack_level();
			defense += static_cast<float>(97 - 8 * soldier->get_defense_level()) / 100;
			evade += static_cast<float>(75 - 15 * soldier->get_evade_level()) / 100;
			break;
		case (Tribes::kEmpire):
			health += 130 + 21 * soldier->get_health_level();
			attack += 14 + 8 * soldier->get_attack_level();
			defense += static_cast<float>(95 - 8 * soldier->get_defense_level()) / 100;
			evade += static_cast<float>(70 - 16 * soldier->get_evade_level()) / 100;
			break;
		case (Tribes::kNone):
			NEVER_HERE();
		}

		final += (attack * health) / (defense * evade);
	}

	// 2500 is aproximate strength of one unpromoted soldier
	return static_cast<int32_t>(final / 2500);
}

// TODO (tiborb) - should be called from scheduler, once in 60s is enough
void DefaultAI::update_player_stat() {
	Map& map = game().map();
	Widelands::PlayerNumber const pn = player_number();
	PlayerNumber const nr_players = map.get_nrplayers();
	uint32_t plr_in_game = 0;
	iterate_players_existing_novar(p, nr_players, game())++ plr_in_game;

	// receiving games statistics and parsing it (reading latest entry)
	const Game::GeneralStatsVector& genstats = game().get_general_statistics();

	// Collecting statistics and saving them in player_statistics object
	const Player* me = game().get_player(pn);
	for (Widelands::PlayerNumber j = 1; j <= plr_in_game; ++j) {
		const Player* this_player = game().get_player(j);
		if (this_player) {
			try {
				const uint32_t vsize = genstats.at(j - 1).miltary_strength.size();

				uint32_t cur_strength = 0;
				uint32_t cur_land = 0;
				uint32_t old_strength = 0;
				uint32_t old60_strength = 0;
				uint32_t old_land = 0;
				uint32_t cass = 0;
				if (vsize > 0) {
					cur_strength = genstats.at(j - 1).miltary_strength.back();
					cur_land = genstats.at(j - 1).land_size.back();
					cass = genstats.at(j - 1).nr_casualties.back();

					if (vsize > 11) {
						old_strength = genstats.at(j - 1).miltary_strength[vsize - 10];
						old_land = genstats.at(j - 1).land_size[vsize - 10];
					} else {
						old_strength = genstats.at(j - 1).miltary_strength[0];
						old_land = genstats.at(j - 1).land_size[0];
					}
					if (vsize > 61) {
						old60_strength = genstats.at(j - 1).miltary_strength[vsize - 60];
					} else {
						old60_strength = genstats.at(j - 1).miltary_strength[0];
					}
				}

				player_statistics.add(pn, j, me->team_number(), this_player->team_number(),
				                      cur_strength, old_strength, old60_strength, cass, cur_land,
				                      old_land);
			} catch (const std::out_of_range&) {
				log("ComputerPlayer(%d): genstats entry missing - size :%d\n",
				    static_cast<unsigned int>(player_number()),
				    static_cast<unsigned int>(genstats.size()));
			}
		}
	}

	player_statistics.recalculate_team_power();
}

bool DefaultAI::check_enemy_sites(uint32_t const gametime) {

	Map& map = game().map();

	// define which players are attackable
	std::vector<Attackable> player_attackable;
	PlayerNumber const nr_players = map.get_nrplayers();
	player_attackable.resize(nr_players);
	uint32_t plr_in_game = 0;
	Widelands::PlayerNumber const pn = player_number();

	iterate_players_existing_novar(p, nr_players, game())++ plr_in_game;

	update_player_stat();

	// printf (" %d: strength: %3d -> %3d ->%3d\n",
	// pn, player_statistics.get_old60_player_power(pn), player_statistics.get_old_player_power(pn),
	// player_statistics.get_player_power(pn));

	// printf ("%d : visible enemies (%d) power: %3d -> %3d, any seen at %u\n",
	// pn,
	// player_statistics.enemies_seen_lately_count(gametime),
	// player_statistics.get_old_visible_enemies_power(gametime),
	// player_statistics.get_visible_enemies_power(gametime),
	// player_statistics.enemy_last_seen());

	// defining treshold ratio of own_strength/enemy's strength
	uint32_t treshold_ratio = 100;
	if (type_ == DefaultAI::Type::kNormal) {
		treshold_ratio = 80;
	}
	if (type_ == DefaultAI::Type::kVeryWeak) {
		treshold_ratio = 120;
	}

	// let's say a 'campaign' is a series of attacks,
	// if there is more then 3 minutes without attack after last
	// attack, then a campaign is over.
	// To start new campaign (=attack again), our strenth must exceed
	// target values (calculated above) by some treshold =
	// ai_personality_attack_margin
	// Once a new campaign started we will fight until
	// we get below above treshold or there will be 3
	// minutes gap since last attack
	// note - AI is not aware of duration of attacks
	// everywhere we consider time when an attack is ordered.
	if (last_attack_time_ < gametime - kCampaignDuration) {
		treshold_ratio += persistent_data->ai_personality_attack_margin;
	}

	const uint32_t my_power = player_statistics.get_modified_player_power(pn);

	// now we test all players to identify 'attackable' ones
	for (Widelands::PlayerNumber j = 1; j <= plr_in_game; ++j) {
		// if we are the same team, or it is just me
		if (player_statistics.players_in_same_team(pn, j) || pn == j) {
			player_attackable[j - 1] = Attackable::kNotAttackable;
			continue;
		}

		// now we compare strength
		// strength of the other player (considering his team)
		uint32_t players_power = player_statistics.get_modified_player_power(j);

		if (players_power == 0) {
			player_attackable.at(j - 1) = Attackable::kAttackable;
		} else if (my_power * 100 / players_power > treshold_ratio * 8) {
			player_attackable.at(j - 1) = Attackable::kAttackableVeryWeak;
		} else if (my_power * 100 / players_power > treshold_ratio * 4) {
			player_attackable.at(j - 1) = Attackable::kAttackableAndWeak;
		} else if (my_power * 100 / players_power > treshold_ratio) {
			player_attackable.at(j - 1) = Attackable::kAttackable;
		} else {
			player_attackable.at(j - 1) = Attackable::kNotAttackable;
		}
	}

	// first we scan vicitnity of couple of militarysites to get new enemy sites
	// Militarysites rotate (see check_militarysites())
	int32_t i = 0;
	for (MilitarySiteObserver mso : militarysites) {
		i += 1;
		if (i % 4 == 0)
			continue;
		if (i > 20)
			continue;

		MilitarySite* ms = mso.site;
		uint32_t const vision = ms->descr().vision_range();
		FCoords f = map.get_fcoords(ms->get_position());

		// get list of immovable around this our military site
		std::vector<ImmovableFound> immovables;
		map.find_immovables(Area<FCoords>(f, (vision + 3 < 13) ? 13 : vision + 3), &immovables,
		                    FindImmovableAttackable());

		for (uint32_t j = 0; j < immovables.size(); ++j) {
			if (upcast(MilitarySite const, bld, immovables.at(j).object)) {
				const PlayerNumber opn = bld->owner().player_number();
				if (player_statistics.get_is_enemy(opn)) {
					assert(opn != pn);
					player_statistics.set_last_time_seen(gametime, opn);
					if (enemy_sites.count(bld->get_position().hash()) == 0) {
						enemy_sites[bld->get_position().hash()] = EnemySiteObserver();
					}
				}
			}
			if (upcast(Warehouse const, wh, immovables.at(j).object)) {
				const PlayerNumber opn = wh->owner().player_number();
				if (player_statistics.get_is_enemy(opn)) {
					assert(opn != pn);
					player_statistics.set_last_time_seen(gametime, opn);
					if (enemy_sites.count(wh->get_position().hash()) == 0) {
						enemy_sites[wh->get_position().hash()] = EnemySiteObserver();
					}
				}
			}
		}
	}

	// now we update some of them
	uint32_t best_target = std::numeric_limits<uint32_t>::max();
	uint8_t best_score = 0;
	uint32_t count = 0;
	// sites that were either conquered or destroyed
	std::vector<uint32_t> disappeared_sites;

	// Willingness to attack depend on how long ago the last soldier has been trained. This is used
	// as
	// indicator how busy our trainingsites are.
	// Moreover the stronger AI the more sensitive to it it is (a score of attack willingness is more
	// decreased if promotion of soldiers is stalled)
	int8_t training_score = 0;
	if (persistent_data->last_soldier_trained > gametime) {
		// No soldier was ever trained ...
		switch (type_) {
		case DefaultAI::Type::kNormal:
			training_score = -8;
			break;
		case DefaultAI::Type::kWeak:
			training_score = -4;
			break;
		case DefaultAI::Type::kVeryWeak:
			training_score = -2;
		}
	} else {
		training_score +=
		   management_data.neuron_pool[19].get_result_safe(
		      (gametime - persistent_data->last_soldier_trained) / 60 / 1000, kAbsValue) /
		   10;
	}

	// Also we should have at least some training sites to be more willing to attack
	// Of course, very weak AI can have only one trainingsite so will be always penalized by this
	training_score += management_data.neuron_pool[23].get_result_safe(
	                     (ts_basic_count_ + ts_advanced_count_ - ts_without_trainers_) * 4) /
	                  10;

	// some black box magic related to growth
	training_score +=
	   management_data.f_neuron_pool[6].get_result(
	      player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn),
	      player_statistics.get_visible_enemies_power(pn) >
	         player_statistics.get_old_visible_enemies_power(pn),
	      player_statistics.get_player_power(pn) - player_statistics.get_old_player_power(pn) > 0) *
	   std::abs(management_data.get_military_number_at(63)) / 10;

	const bool strong_enough = player_statistics.strong_enough(pn);

	for (std::map<uint32_t, EnemySiteObserver>::iterator site = enemy_sites.begin();
	     site != enemy_sites.end(); ++site) {

		// we test max 12 sites and prefer ones tested more then 1 min ago
		if (((site->second.last_tested + (enemysites_check_delay_ * 1000)) > gametime && count > 4) ||
		    count > 12) {
			continue;
		}
		count += 1;

		site->second.last_tested = gametime;
		uint8_t defenders_strength = 0;
		bool is_warehouse = false;
		bool is_attackable = false;
		// we cannot attack unvisible site and there is no other way to find out
		const bool is_visible =
		   (1 < player_->vision(Map::get_index(Coords::unhash(site->first), map.get_width())));
		uint16_t owner_number = 100;

		// testing if we can attack the building - result is a flag
		// if we dont get a flag, we remove the building from observers list
		FCoords f = map.get_fcoords(Coords::unhash(site->first));
		uint32_t site_to_be_removed = std::numeric_limits<uint32_t>::max();
		Flag* flag = nullptr;

		if (upcast(MilitarySite, bld, f.field->get_immovable())) {
			if (player_->is_hostile(bld->owner())) {
				std::vector<Soldier*> defenders;
				defenders = bld->present_soldiers();
				defenders_strength = calculate_strength(defenders);

				flag = &bld->base_flag();
				if (is_visible && bld->can_attack()) {
					is_attackable = true;
				}
				owner_number = bld->owner().player_number();
			}
		}
		if (upcast(Warehouse, Wh, f.field->get_immovable())) {
			if (player_->is_hostile(Wh->owner())) {

				std::vector<Soldier*> defenders;
				defenders = Wh->present_soldiers();
				defenders_strength = calculate_strength(defenders);

				flag = &Wh->base_flag();
				is_warehouse = true;
				if (is_visible && Wh->can_attack()) {
					is_attackable = true;
				}
				owner_number = Wh->owner().player_number();
			}
		}

		// if flag is defined it is a good taget
		if (flag) {
			// updating some info
			// updating info on mines nearby if needed
			if (site->second.mines_nearby == ExtendedBool::kUnset) {
				FindNodeMineable find_mines_spots_nearby(game(), f.field->get_resources());
				const int32_t minescount =
				   map.find_fields(Area<FCoords>(f, 6), nullptr, find_mines_spots_nearby);
				if (minescount > 0) {
					site->second.mines_nearby = ExtendedBool::kTrue;
				} else {
					site->second.mines_nearby = ExtendedBool::kFalse;
				}
			}

			site->second.is_warehouse = is_warehouse;

			// getting rid of default
			if (site->second.last_time_attackable == kNever) {
				site->second.last_time_attackable = gametime;
			}

			// can we attack:
			if (is_attackable) {
				std::vector<Soldier*> attackers;
				player_->find_attack_soldiers(*flag, &attackers);
				if (attackers.empty()) {
					site->second.attack_soldiers_strength = 0;
				} else {
					int32_t strength = calculate_strength(attackers);

					site->second.attack_soldiers_strength = strength;
					assert(!attackers.empty());
					site->second.attack_soldiers_competency = strength * 10 / attackers.size();
					// printf ("soldiers :%d, competency: %d\n", attackers.size(),
					// site->second.attack_soldiers_competency);
				}
			} else {
				site->second.attack_soldiers_strength = 0;
			}

			site->second.defenders_strength = defenders_strength;

			if (site->second.attack_soldiers_strength > 0 &&
			    (player_attackable[owner_number - 1] == Attackable::kAttackable ||
			     player_attackable[owner_number - 1] == Attackable::kAttackableAndWeak ||
			     player_attackable[owner_number - 1] == Attackable::kAttackableVeryWeak)) {
				site->second.score =
				   site->second.attack_soldiers_strength - site->second.defenders_strength / 2;

				if (is_warehouse) {
					site->second.score += 2;
				} else {
					site->second.score -= 2;
				}

				// considering heroes/rokie status of soldiers
				site->second.score += management_data.neuron_pool[63].get_result_safe(
				                         site->second.attack_soldiers_competency - 10, kAbsValue) /
				                      10;
				site->second.score -= std::abs(management_data.get_military_number_at(62)) / 10;
				site->second.score +=
				   management_data.f_neuron_pool[31].get_result(
				      site->second.attack_soldiers_competency > 15,
				      site->second.attack_soldiers_competency > 25,
				      site->second.defenders_strength > site->second.attack_soldiers_strength,
				      2 * site->second.defenders_strength > 3 * site->second.attack_soldiers_strength,
				      3 * site->second.defenders_strength > 2 * site->second.attack_soldiers_strength) /
				   20;

				site->second.score -= static_cast<uint8_t>(soldier_status_);

				if (site->second.mines_nearby == ExtendedBool::kTrue) {
					site->second.score += std::abs(management_data.get_military_number_at(18)) / 15;
				}
				// we dont want to attack multiple players at the same time too eagerly
				if (owner_number != persistent_data->last_attacked_player) {
					site->second.score -= 3 + std::abs(management_data.get_military_number_at(38)) / 40;
				}

				// Let consider mines nearby and generally
				site->second.score +=
				   management_data.f_neuron_pool[5].get_result(
				      mines_.size() <= 2, mines_.size() > 2 && mines_.size() < 6, mines_.size() >= 6,
				      site->second.mines_nearby == ExtendedBool::kTrue) *
				   std::abs(management_data.get_military_number_at(19)) / 20;

				// Applying (decreasing score) if trainingsites are not working
				site->second.score += training_score;

				// We have an advantage over stongest opponent
				if (strong_enough) {
					site->second.score += 3;
				}

				// Enemy is too weak, be more aggressive attacking him
				if (player_attackable[owner_number - 1] == Attackable::kAttackableAndWeak) {
					site->second.score += 4;
				}
				if (player_attackable[owner_number - 1] == Attackable::kAttackableVeryWeak) {
					site->second.score += 8;
				}

				// consider change in power
				site->second.score +=
				   management_data.f_neuron_pool[4].get_result(
				      player_statistics.get_player_power(pn) >  // 1
				         player_statistics.get_old_player_power(pn),
				      player_statistics.get_player_power(owner_number) >  // 2
				         player_statistics.get_old_player_power(owner_number),
				      player_statistics.get_player_power(pn) -  // 3
				            player_statistics.get_old_player_power(pn) >
				         0,
				      static_cast<int32_t>(player_statistics.get_player_power(pn) -  // 4
				                           player_statistics.get_old_player_power(pn)) >
				         static_cast<int32_t>(player_statistics.get_player_power(owner_number) -
				                              player_statistics.get_old_player_power(owner_number)),
				      5 * player_statistics.get_player_power(pn) >  // 5
				         4 * player_statistics.get_old_player_power(pn)) *
				   std::abs(management_data.get_military_number_at(35)) / 10;
				site->second.score +=
				   management_data.f_neuron_pool[30].get_result(
				      player_statistics.get_player_power(pn) >  // 1
				         player_statistics.get_old60_player_power(pn),
				      3 * player_statistics.get_player_power(pn) >  // 2
				         2 * player_statistics.get_old60_player_power(pn),
				      2 * player_statistics.get_player_power(pn) >  // 3
				         3 * player_statistics.get_old60_player_power(pn),
				      player_statistics.get_player_power(pn) >  // 4
				         player_statistics.get_old_player_power(pn),
				      static_cast<int32_t>(player_statistics.get_player_power(pn) -  // 5
				                           player_statistics.get_old60_player_power(pn)) >
				         static_cast<int32_t>(player_statistics.get_player_power(owner_number) -
				                              player_statistics.get_old60_player_power(owner_number))) *
				   std::abs(management_data.get_military_number_at(61)) / 10;

				// treating no attack score
				if (site->second.no_attack_counter < 0) {
					// we cannot attack yet
					site->second.score = 0;
					// but increase the counter by 1
					site->second.no_attack_counter += 1;
				}
			} else {
				site->second.score = 0;
			}  // or the score will remain 0

			if (site->second.score > 0) {
				if (site->second.score > best_score) {
					best_score = site->second.score;
					best_target = site->first;
				}
			}

			if (site->second.attack_soldiers_strength > 0) {
				site->second.last_time_attackable = gametime;
			}
			if (site->second.last_time_attackable + 20 * 60 * 1000 < gametime) {
				site_to_be_removed = site->first;
			}
		} else {  // we dont have a flag, let remove the site from out observer list
			site_to_be_removed = site->first;
		}

		if (site_to_be_removed < std::numeric_limits<uint32_t>::max()) {
			disappeared_sites.push_back(site_to_be_removed);
		}
	}

	while (!disappeared_sites.empty()) {
		enemy_sites.erase(disappeared_sites.back());
		disappeared_sites.pop_back();
	}

	// modifying enemysites_check_delay_,this depends on the count
	// of enemysites in observer
	if (count >= 13 && enemysites_check_delay_ < 180) {
		enemysites_check_delay_ += 3;
	}
	if (count < 10 && enemysites_check_delay_ > 45) {
		enemysites_check_delay_ -= 2;
	}

	// if coordinates hash is not set
	if (best_target == std::numeric_limits<uint32_t>::max()) {
		return false;
	}

	assert(enemy_sites.count(best_target) > 0);

	// attacking
	FCoords f = map.get_fcoords(Coords::unhash(best_target));
	// setting no attack counter here
	// this gauranties that it will not be attacked in next 3
	// turns
	enemy_sites[best_target].no_attack_counter =
	   -3 + management_data.get_military_number_at(20) / 40;
	assert(enemy_sites[best_target].no_attack_counter > -7 &&
	       enemy_sites[best_target].no_attack_counter < 0);

	Flag* flag = nullptr;  // flag of a building to be attacked
	if (upcast(MilitarySite, bld, f.field->get_immovable())) {
		flag = &bld->base_flag();
	} else if (upcast(Warehouse, Wh, f.field->get_immovable())) {
		flag = &Wh->base_flag();
	} else {
		return false;  // this should not happen
	}

	// how many attack soldiers we can send?
	uint32_t attackers = player_->find_attack_soldiers(*flag);

	// Of course not all of them:
	// reduce by 0-3 for attackers below 10
	// but for soldiers in range 10-40 reduce by much more.
	// Soldiers above 40 are ignored for calculation

	// Number of soldiers in the range 10-40, random portion of
	// them will be used
	uint32_t above_ten = (attackers > 10) ? attackers - 10 : 0;
	above_ten = (above_ten > 30) ? 30 : above_ten;

	attackers = attackers - (gametime % 3) - ((above_ten > 0) ? gametime % above_ten : 0);
	attackers += management_data.neuron_pool[48].get_result_safe(training_score / 2, kAbsValue) / 10;

	if (attackers <= 0) {
		return false;
	}

	game().send_player_enemyflagaction(*flag, player_number(), attackers);

	last_attack_time_ = gametime;
	persistent_data->last_attacked_player = flag->owner().player_number();

	return true;
}

// This runs once in 15 minutes, and adjust wares targets based on number of
// productionsites and ports
void DefaultAI::review_wares_targets(uint32_t const gametime) {

	player_ = game().get_player(player_number());
	tribe_ = &player_->tribe();

	// to avoid floats real multiplier is multiplier/10
	const uint16_t multiplier = std::max<uint16_t>((productionsites.size() + num_ports * 5) / 5, 10);

	for (EconomyObserver* observer : economies) {
		DescriptionIndex nritems = player_->egbase().tribes().nrwares();
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

			uint16_t new_target = std::max<uint16_t>(default_target * multiplier / 10, 2);
			assert(new_target > 1);

			game().send_player_command(*new Widelands::CmdSetWareTargetQuantity(
			   gametime, player_number(), player_->get_economy_number(&observer->economy), id,
			   new_target));
		}
	}
}

// Sets due_time based on job ID
void DefaultAI::set_taskpool_task_time(const uint32_t gametime,
                                       const Widelands::SchedulerTaskId task) {

	for (auto& item : taskPool) {
		if (item.id == task) {
			item.due_time = gametime;
			return;
		}
	}
	NEVER_HERE();
}

// Retrieves due time of the task based on its ID
uint32_t DefaultAI::get_taskpool_task_time(const Widelands::SchedulerTaskId task) {
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
// TODO(tiborb ?): - it would be nice to have this activated by a command line switch
void DefaultAI::print_stats(uint32_t const gametime) {

	if (!kPrintStats) {
		set_taskpool_task_time(std::numeric_limits<int32_t>::max(), SchedulerTaskId::kPrintStats);
		return;
	}

	PlayerNumber const pn = player_number();

	const DescriptionIndex& nr_buildings = game().tribes().nrbuildings();
	std::set<DescriptionIndex> materials;

	// Collect information about the different buildings that our tribe can have
	for (DescriptionIndex building_index = 0; building_index < nr_buildings; ++building_index) {
		const BuildingDescr& bld = *tribe_->get_building_descr(building_index);
		if (!tribe_->has_building(building_index)) {
			continue;
		}
		if (bld.type() == MapObjectType::PRODUCTIONSITE) {
			const ProductionSiteDescr& prod = dynamic_cast<const ProductionSiteDescr&>(bld);
			for (const auto& temp_input : prod.inputs()) {
				if (materials.count(temp_input.first) == 0) {
					materials.insert(temp_input.first);
				}
			}
			for (const auto& temp_cost : prod.buildcost()) {
				if (materials.count(temp_cost.first) == 0) {
					materials.insert(temp_cost.first);
				}
			}
		}

		if (bld.type() == MapObjectType::TRAININGSITE) {
			const ProductionSiteDescr& train = dynamic_cast<const TrainingSiteDescr&>(bld);
			for (const auto& temp_cost : train.buildcost()) {
				if (materials.count(temp_cost.first) == 0) {
					materials.insert(temp_cost.first);
				}
			}
		}
	}

	std::string summary = "";
	for (const auto material : materials) {
		uint32_t stock = calculate_stocklevel(material);
		// printf (" testing %s  %d\n", game().tribes().get_ware_descr(material)
		// ->descname().c_str(), calculate_stocklevel(material));
		if (stock == 0) {
			summary = summary + game().tribes().get_ware_descr(material)->descname() + ", ";
		}
	}

	if (false)
		printf(" %1d: %s Buildings count: Pr:%3u, Ml:%3u, Mi:%2u, Wh:%2u, Po:%u.\n", pn,
		       gamestring_with_leading_zeros(gametime), static_cast<uint32_t>(productionsites.size()),
		       static_cast<uint32_t>(militarysites.size()), static_cast<uint32_t>(mines_.size()),
		       static_cast<uint32_t>(warehousesites.size() - num_ports), num_ports);
	// printf(" Missing wares (zero stock): %s\n", summary.c_str());

	if (false)
		printf(" %1s %-30s   %5s(perf)  %6s %6s %6s %8s %5s %5s %5s %5s\n", "T", "Buildings", "work.",
		       "const.", "unocc.", "uncon.", "needed", "prec.", "pprio", "stock", "targ.");
	for (uint32_t j = 0; j < buildings_.size(); ++j) {
		BuildingObserver& bo = buildings_.at(j);
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

			if (false)
				printf(" %1s %-30s %5d(%3d%%)  %6d %6d %6d %8s %5d %5d %5d %5d\n", btype.c_str(),
				       bo.name, bo.total_count() - bo.cnt_under_construction - bo.unoccupied_count -
				                   bo.unconnected_count,
				       bo.current_stats, bo.cnt_under_construction, bo.unoccupied_count,
				       bo.unconnected_count, needeness.c_str(), bo.max_needed_preciousness,
				       bo.primary_priority, get_stocklevel(bo, gametime), bo.cnt_target);
		}
	}

	std::string why = "; Why: ";

	if ((num_prod_constructionsites + mines_in_constr()) >
	    (productionsites.size() + mines_built()) / persistent_data->ai_productionsites_ratio + 2) {
		why += " too many constr.";
	}
	// 3. too keep some proportions production sites vs military sites
	if ((num_prod_constructionsites + productionsites.size()) >
	    (msites_in_constr() + militarysites.size()) * 5) {
		why += ", too many productionsites";
	}
	// 4. if we do not have 2 mines at least
	if (mines_.size() < 2) {
		why += ", less then 2 mines";
	}

	if (false)
		printf("Prodsites in constr: %2d, mines in constr: %2d %s %s\n", num_prod_constructionsites,
		       mines_in_constr(), (new_buildings_stop_) ? "NEW BUILDING STOP" : "", why.c_str());

	if (false)
		printf("Least military score: %5d/%3d, msites in constr: %3d,"
		       "soldier st: %2d, strength: %3d\n",
		       persistent_data->least_military_score, persistent_data->ai_personality_mil_upper_limit,
		       msites_in_constr(), static_cast<uint8_t>(soldier_status_),
		       player_statistics.get_modified_player_power(player_number()));
	std::string wpolicy = "";
	switch (wood_policy_) {
	case WoodPolicy::kDismantleRangers:
		wpolicy = "Dismantle rangers";
		break;
	case WoodPolicy::kAllowRangers:
		wpolicy = "Allow rangers";
		break;
	case WoodPolicy::kStopRangers:
		wpolicy = "Stop rangers";
		break;
	default:
		wpolicy = "unknown";
	}
	if (false)
		printf("Trees around cutters: %4d/10, woodcutters policy: %s\n",
		       persistent_data->trees_around_cutters, wpolicy.c_str());
}

template <typename T>
void DefaultAI::check_range(T value, T bottom_range, T upper_range, const char* value_name) {
	if (value < bottom_range || value > upper_range) {
		log(" %d: unexpected value for %s: %d\n", player_number(), value_name, value);
	}
}

template <typename T> void DefaultAI::check_range(T value, T upper_range, const char* value_name) {
	if (value > upper_range) {
		log(" %d: unexpected value for %s: %d\n", player_number(), value_name, value);
	}
}

int32_t DefaultAI::limit_cnt_target(const int32_t current_cnt_target, const int32_t ai_limit) {

	if (ai_limit >= std::numeric_limits<int32_t>::max() - 1) {
		// = ai limit is not set
		return current_cnt_target;
	}

	int32_t new_target = current_cnt_target;

	if (current_cnt_target > (ai_limit + 1) / 2) {
		new_target = (ai_limit + 1) / 2;
	}
	assert(new_target * 2 >= ai_limit);
	assert(new_target > 0);
	assert(new_target <= ai_limit);

	return new_target;
}
