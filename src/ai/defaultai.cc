/*
 * Copyright (C) 2004, 2006-2010, 2012 by the Widelands Development Team
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
#include <ctime>
#include <memory>
#include <queue>
#include <typeinfo>
#include <unordered_set>

#include "ai/ai_hints.h"
#include "base/log.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/road.h"
#include "economy/wares_queue.h"
#include "logic/constructionsite.h"
#include "logic/findbob.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/map.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/playercommand.h"
#include "logic/productionsite.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/world/world.h"
#include "profile/profile.h"

// following is in miliseconds (widelands counts time in ms)
// constexpr int kFieldUpdateInterval = 2000;
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
constexpr int kTrainingSitesCheckInterval = 45 * 1000;

// this is intended for map developers, by default should be off
constexpr bool kPrintStats = false;

// Some buildings have to be built close to borders and their
// priority might be decreased below 0, so this is to
// compensate
constexpr int32_t kDefaultPrioBoost = 12;

using namespace Widelands;

DefaultAI::AggressiveImpl DefaultAI::aggressiveImpl;
DefaultAI::NormalImpl DefaultAI::normalImpl;
DefaultAI::DefensiveImpl DefaultAI::defensiveImpl;

/// Constructor of DefaultAI
DefaultAI::DefaultAI(Game& ggame, PlayerNumber const pid, uint8_t const t)
   : ComputerPlayer(ggame, pid),
     type_(t),
     player_(nullptr),
     tribe_(nullptr),
     num_constructionsites_(0),
     num_milit_constructionsites(0),
     num_prod_constructionsites(0),
     num_ports(0),
     last_attacked_player_(std::numeric_limits<uint16_t>::max()),
     enemysites_check_delay_(60),
     wood_policy_(WoodPolicy::kStartRangers),
     next_ai_think_(0),
     next_mine_construction_due_(0),
     inhibit_road_building_(0),
     time_of_last_construction_(0),
     enemy_last_seen_(0),
     numof_warehouses_(0),
     new_buildings_stop_(false),
     resource_necessity_territory_(100),
     resource_necessity_mines_(100),
     resource_necessity_water_(0),
     resource_necessity_water_needed_(false),
     unstationed_milit_buildings_(0),
     military_last_dismantle_(0),
     military_last_build_(0),
     seafaring_economy(false),
     colony_scan_area_(35),
     spots_(0),
     vacant_mil_positions_(0),
     ts_basic_count_(0),
     ts_basic_const_count_(0),
     ts_advanced_count_(0),
     ts_advanced_const_count_(0),
     ts_without_trainers_(0),
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
			   	break;
		   	default:
				;
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
	ScheduleTasks DueTask = ScheduleTasks::kIdle;
	DueTask = get_oldest_task(gametime);
	schedStat[static_cast<uint32_t>(DueTask)] += 1;

	// now AI runs a job selected above to be performed in this turn
	// (only one but some of them needs to run check_economies() to
	// guarantee consistency)
	// job names are selfexplanatory
	switch (DueTask) {
		case ScheduleTasks::kBbuildableFieldsCheck :
			update_all_buildable_fields(gametime);
			taskDue[ScheduleTasks::kBbuildableFieldsCheck] = gametime + kMinBFCheckInterval;
			break;
		case ScheduleTasks::kMineableFieldsCheck :
			update_all_mineable_fields(gametime);
			taskDue[ScheduleTasks::kMineableFieldsCheck] = gametime + kMinMFCheckInterval;
			break;
		case ScheduleTasks::kRoadCheck :
			if (check_economies()) {  // is a must
				return;
			};
			taskDue[ScheduleTasks::kRoadCheck] = gametime + 1000;
			// testing 5 roads
			{
				const int32_t roads_to_check = (roads.size() + 1 < 5) ? roads.size() + 1 : 5;
				for (int i = 0; i < roads_to_check; i += 1) {
					// improve_roads function will test one road and rotate roads vector
					if (improve_roads(gametime)) {
						// if significant change takes place do not go on
						break;
					};
				}
			}
			break;
		case ScheduleTasks::kUnbuildableFCheck :
			taskDue[ScheduleTasks::kUnbuildableFCheck] = gametime + 4000;
			update_all_not_buildable_fields();
			break;
		case ScheduleTasks::kCheckEconomies :
			check_economies();
			taskDue[ScheduleTasks::kCheckEconomies] = gametime + 8000;
			break;
		case ScheduleTasks::kProductionsitesStats :
			update_productionsite_stats(gametime);
			break;
		case ScheduleTasks::kConstructBuilding :
			if (check_economies()) {  // economies must be consistent
				return;
			}
			taskDue[ScheduleTasks::kConstructBuilding] = gametime + 6000;
			if (construct_building(gametime)) {
				time_of_last_construction_ = gametime;
			}
			break;
		case ScheduleTasks::kCheckProductionsites :
			if (check_economies()) {  // economies must be consistent
				return;
			}
			{
				// testing 5 productionsites (if there are 5 of them)
				int32_t ps_to_check = (productionsites.size() < 5) ? productionsites.size() : 5;
				for (int i = 0; i < ps_to_check; i += 1) {
					// one productionsite per one check_productionsites() call
					if (check_productionsites(gametime)) {
						// if significant change takes place do not go on
						break;
					};
				}
			}
			taskDue[ScheduleTasks::kCheckProductionsites] = gametime + 15000;
			break;
		case ScheduleTasks::kCheckShips :
			check_ships(gametime);
			break;
		case ScheduleTasks::KMarineDecisions :
			marine_main_decisions(gametime);
			break;
		case ScheduleTasks::kCheckMines :
			if (check_economies()) {  // economies must be consistent
				return;
			}
			taskDue[ScheduleTasks::kCheckMines] = gametime + 15000;
			// checking 3 mines if possible
			{
				int32_t mines_to_check = (mines_.size() < 5) ? mines_.size() : 5;
				for (int i = 0; i < mines_to_check; i += 1) {
					// every run of check_mines_() checks one mine
					if (check_mines_(gametime)) {
						// if significant change takes place do not go on
						break;
					};
				}
			}
			break;
		case ScheduleTasks::kCheckMilitarysites :
			check_militarysites(gametime);
			break;
		case ScheduleTasks::kCheckTrainingsites :
			check_trainingsites(gametime);
			break;
		case ScheduleTasks::kCountMilitaryVacant :
			count_military_vacant_positions();
			taskDue[ScheduleTasks::kCountMilitaryVacant] = gametime + 60 * 1000;
			break;
		case ScheduleTasks::kWareReview :
			if (check_economies()) {  // economies must be consistent
				return;
			}
			taskDue[ScheduleTasks::kWareReview] = gametime + 15 * 60 * 1000;
			review_wares_targets(gametime);
			break;
		case ScheduleTasks::kPrintStats :
			if (check_economies()) {  // economies must be consistent
				return;
			}
			print_stats(gametime);
			break;
		case ScheduleTasks::kCheckEnemySites :
			check_enemy_sites(gametime);
			taskDue[ScheduleTasks::kCheckEnemySites] = gametime + 19 * 1000;
			break;
		case ScheduleTasks::kIdle :
			break;
		default:
			;
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
	log("ComputerPlayer(%d): initializing (%u)\n", player_number(), type_);
	WareIndex const nr_wares = tribe_->get_nrwares();
	wares.resize(nr_wares);

	for (WareIndex i = 0; i < nr_wares; ++i) {
		wares.at(i).producers_ = 0;
		wares.at(i).consumers_ = 0;
		wares.at(i).preciousness_ = tribe_->get_ware_descr(i)->preciousness();
	}

	// collect information about the different buildings our tribe can construct
	BuildingIndex const nr_buildings = tribe_->get_nrbuildings();
	const World& world = game().world();

	for (BuildingIndex i = 0; i < nr_buildings; ++i) {
		const BuildingDescr& bld = *tribe_->get_building_descr(i);
		const std::string& building_name = bld.name();
		const BuildingHints& bh = bld.hints();
		buildings_.resize(buildings_.size() + 1);
		BuildingObserver& bo = buildings_.back();
		bo.name = building_name.c_str();
		bo.id = i;
		bo.desc = &bld;
		bo.type = BuildingObserver::BORING;
		bo.cnt_built_ = 0;
		bo.cnt_under_construction_ = 0;
		bo.cnt_target_ = 1;  // default for everything
		bo.stocklevel_ = 0;
		bo.stocklevel_time = 0;
		bo.last_dismantle_time_ = 0;
		// this is set to negative number, otherwise the AI would wait 25 sec
		// after game start not building anything
		bo.construction_decision_time_ = -60 * 60 * 1000;
		bo.build_material_shortage_ = false;
		bo.production_hint_ = -1;
		bo.current_stats_ = 0;
		bo.unoccupied_ = false;
		bo.unconnected_ = 0;
		bo.is_buildable_ = bld.is_buildable();
		bo.need_trees_ = bh.is_logproducer();
		bo.need_stones_ = bh.is_stoneproducer();
		bo.need_water_ = bh.get_needs_water();
		bo.mines_water_ = bh.mines_water();
		bo.recruitment_ = bh.for_recruitment();
		bo.space_consumer_ = bh.is_space_consumer();
		bo.expansion_type_ = bh.is_expansion_type();
		bo.fighting_type_ = bh.is_fighting_type();
		bo.mountain_conqueror_ = bh.is_mountain_conqueror();
		bo.prohibited_till_ = bh.get_prohibited_till() * 1000;  // value in conf is in seconds
		bo.forced_after_ = bh.get_forced_after() * 1000;        // value in conf is in seconds
		bo.is_port_ = bld.get_isport();
		bo.trainingsite_type_ = TrainingSiteType::kNoTS;
		bo.upgrade_substitutes_ = false;
		bo.output_needed_ = ExtendedBool::kUnset;
		bo.max_preciousness = 0;
		bo.max_needed_preciousness_ = 0;

		if (bh.renews_map_resource()) {
			bo.production_hint_ = tribe_->safe_ware_index(bh.get_renews_map_resource());
		}

		// I just presume cut wood is named "log" in the game
		if (tribe_->safe_ware_index("log") == bo.production_hint_) {
			bo.plants_trees_ = true;
		} else {
			bo.plants_trees_ = false;
		}

		// Read all interesting data from ware producing buildings
		if (bld.type() == MapObjectType::PRODUCTIONSITE) {
			const ProductionSiteDescr& prod = dynamic_cast<const ProductionSiteDescr&>(bld);
			bo.type = bld.get_ismine() ? BuildingObserver::MINE : BuildingObserver::PRODUCTIONSITE;
			for (const WareAmount& temp_input : prod.inputs()) {
				bo.inputs_.push_back(temp_input.first);
			}
			for (const WareIndex& temp_output : prod.output_ware_types()) {
				bo.outputs_.push_back(temp_output);
			}

			if (bo.type == BuildingObserver::MINE) {
				// get the resource needed by the mine
				if (bh.get_mines()) {
					bo.mines_ = world.get_resource(bh.get_mines());
				}

				bo.mines_percent_ = bh.get_mines_percent();

				// populating mines_per_type map
				if (mines_per_type.count(bo.mines_) == 0) {
					mines_per_type[bo.mines_] = MineTypesObserver();
				}
			}

			// here we identify hunters
			if (bo.outputs_.size() == 1 && tribe_->safe_ware_index("meat") == bo.outputs_.at(0)) {
				bo.is_hunter_ = true;
			} else {
				bo.is_hunter_ = false;
			}

			// and fishers
			if (bo.outputs_.size() == 1 && tribe_->safe_ware_index("fish") == bo.outputs_.at(0)) {
				bo.is_fisher_ = true;
			} else {
				bo.is_fisher_ = false;
			}

			if (building_name == "shipyard") {
				bo.is_shipyard_ = true;
			} else {
				bo.is_shipyard_ = false;
			}

			// now we find out if the upgrade of the building is a full substitution
			// (produces all wares as current one)
			const BuildingIndex enhancement = bld.enhancement();
			if (enhancement != INVALID_INDEX && bo.type == BuildingObserver::PRODUCTIONSITE) {
				std::unordered_set<WareIndex> enh_outputs;
				const ProductionSiteDescr& enh_prod
					=
					dynamic_cast<const ProductionSiteDescr&>(*tribe_->get_building_descr(enhancement));

				// collecting wares that are produced in enhanced building
				for (const WareIndex& ware : enh_prod.output_ware_types()) {
					enh_outputs.insert(ware);
				}
				// now testing outputs of current building
				// and comparing
				bo.upgrade_substitutes_ = true;
				for (WareIndex ware : bo.outputs_) {
					if (enh_outputs.count(ware) == 0) {
						bo.upgrade_substitutes_ = false;
						break;
					}
				}
			}

			// plus some manually picked buildings,
			// see preferred_upgrade list
			for (const char* pb : preferred_upgrade) {
				if (strcmp(bld.name().c_str(), pb) == 0) {
					bo.upgrade_substitutes_ = true;
				}
			}

			continue;
		}

		// now for every military building, we fill critical_built_mat_ vector
		// with critical construction wares
		// non critical are excluded (see below)
		if (bld.type() == MapObjectType::MILITARYSITE) {
			bo.type = BuildingObserver::MILITARYSITE;
			const MilitarySiteDescr& milit = dynamic_cast<const MilitarySiteDescr&>(bld);
			for (const std::pair<unsigned char, unsigned char>& temp_buildcosts : milit.buildcost()) {
				// bellow are non-critical wares
				if (tribe_->ware_index("log") == temp_buildcosts.first ||
				    tribe_->ware_index("blackwood") == temp_buildcosts.first ||
				    tribe_->ware_index("planks") == temp_buildcosts.first ||
				    tribe_->ware_index("wood") == temp_buildcosts.first ||
				    tribe_->ware_index("raw_stone") == temp_buildcosts.first ||
				    tribe_->ware_index("stone") == temp_buildcosts.first)
					continue;

				bo.critical_built_mat_.push_back(temp_buildcosts.first);
			}
			continue;
		}

		if (bld.type() == MapObjectType::WAREHOUSE) {
			bo.type = BuildingObserver::WAREHOUSE;
			continue;
		}

		if (bld.type() == MapObjectType::TRAININGSITE) {
			bo.type = BuildingObserver::TRAININGSITE;
			const TrainingSiteDescr& train = dynamic_cast<const TrainingSiteDescr&>(bld);
			for (const WareAmount& temp_input : train.inputs()) {
				bo.inputs_.push_back(temp_input.first);

				// collecting subsitutes
				if (tribe_->ware_index("meat") == temp_input.first ||
				    tribe_->ware_index("fish") == temp_input.first ||
				    tribe_->ware_index("smoked_meat") == temp_input.first ||
				    tribe_->ware_index("smoked_fish") == temp_input.first) {
					bo.substitute_inputs_.insert(temp_input.first);
				}
			}

			bo.trainingsite_type_ = bh.get_trainingsite_type();
			// it would behave badly if no type was set
			// make sure all TS have its type set properly in conf files
			assert(bo.trainingsite_type_ != TrainingSiteType::kNoTS);
			continue;
		}

		if (bld.type() == MapObjectType::CONSTRUCTIONSITE) {
			bo.type = BuildingObserver::CONSTRUCTIONSITE;
			continue;
		}
	}



	// atlanteans they consider water as a resource
	// (together with mines, stones and wood)
	if (tribe_->name() == "atlanteans") {
		resource_necessity_water_needed_ = true;
	}

	Map& map = game().map();

	// here we generate list of all ports and their vicinity from entire map
	for (const Coords& c : map.get_port_spaces()) {
		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(map.get_fcoords(c), 3));
		do {
			const int32_t hash = coords_hash(map.get_fcoords(*(mr.location().field)));
			if (port_reserved_coords.count(hash) == 0)
				port_reserved_coords.insert(hash);
		} while (mr.advance(map));

		// the same for NW neighbour of a field
		Coords c_nw;
		map.get_tln(c, &c_nw);
		MapRegion<Area<FCoords>> mr_nw(map, Area<FCoords>(map.get_fcoords(c_nw), 3));
		do {
			const int32_t hash = coords_hash(map.get_fcoords(*(mr_nw.location().field)));
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
					gain_immovable(*imm);
				}
			}
		}
	}

	// blocking space consumers vicinity (when reloading a game)
	for (const ProductionSiteObserver& ps_obs : productionsites) {
		if (ps_obs.bo->space_consumer_ && !ps_obs.bo->plants_trees_) {
			MapRegion<Area<FCoords>> mr(
			   map, Area<FCoords>(map.get_fcoords(ps_obs.site->get_position()), 4));
			do {
				BlockedField blocked2(
				   map.get_fcoords(*(mr.location().field)), game().get_gametime() + 20 * 60 * 1000);
				blocked_fields.push_back(blocked2);
			} while (mr.advance(map));
		}
	}

	taskDue[ScheduleTasks::kConstructBuilding] = 0;
	taskDue[ScheduleTasks::kRoadCheck] = 1000;
	taskDue[ScheduleTasks::kCheckProductionsites] = 15 * 1000;
	taskDue[ScheduleTasks::kProductionsitesStats] = 30000;
	taskDue[ScheduleTasks::kCheckMines] = 30 * 1000;
	taskDue[ScheduleTasks::kCheckMilitarysites] = 0;
	taskDue[ScheduleTasks::kCheckShips] = 30 * 1000;
	taskDue[ScheduleTasks::kCheckEconomies] = 1000;
	taskDue[ScheduleTasks::KMarineDecisions] = 30 * 1000;
	taskDue[ScheduleTasks::kCheckTrainingsites] = 2 * 60 * 1000;
	taskDue[ScheduleTasks::kBbuildableFieldsCheck] = 1000;
	taskDue[ScheduleTasks::kMineableFieldsCheck] = 1000;
	taskDue[ScheduleTasks::kUnbuildableFCheck] = 1000;
	taskDue[ScheduleTasks::kWareReview] = 15 * 60 * 1000;
	taskDue[ScheduleTasks::kPrintStats] = 30 * 60 * 1000;
	taskDue[ScheduleTasks::kCountMilitaryVacant] = 10 * 60 * 1000;
	taskDue[ScheduleTasks::kCheckEnemySites] = 10 * 60 * 1000;
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
	       (buildable_fields.front()->field_info_expiration_ - kFieldInfoExpiration + 1000) <=
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
		bf.field_info_expiration_ = gametime + kFieldInfoExpiration;
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
	       (mineable_fields.front()->field_info_expiration_ - kMineFieldInfoExpiration + 1000) <=
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
		mf->field_info_expiration_ = gametime + kMineFieldInfoExpiration;
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
	uint32_t maxchecks = unusable_fields.size();

	if (maxchecks > 50) {
		maxchecks = 50;
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
void DefaultAI::update_buildable_field(BuildableField& field, uint16_t range, bool military) {
	// look if there is any unowned land nearby
	Map& map = game().map();
	const int32_t gametime = game().get_gametime();
	FindNodeUnowned find_unowned(player_, game());
	FindNodeUnownedMineable find_unowned_mines_pots(player_, game());
	PlayerNumber const pn = player_->player_number();
	const World& world = game().world();
	field.unowned_land_nearby_ =
	   map.find_fields(Area<FCoords>(field.coords, range), nullptr, find_unowned);
	FindNodeAllyOwned find_ally(player_, game(), player_number());
	const int32_t AllyOwnedFields =
	   map.find_fields(Area<FCoords>(field.coords, 3), nullptr, find_ally);

	field.near_border_ = false;
	if (AllyOwnedFields > 0) {
		field.near_border_ = true;
	} else if (field.unowned_land_nearby_ > 0) {
		if (map.find_fields(Area<FCoords>(field.coords, 4), nullptr, find_unowned) > 0) {
			field.near_border_ = true;
		}
	}

	// to save some CPU
	if ((mines_.size() > 8 && gametime % 3 > 0) || field.unowned_land_nearby_ == 0) {
		field.unowned_mines_pots_nearby_ = 0;
	} else {
		uint32_t close_mines =
		   map.find_fields(Area<FCoords>(field.coords, 4), nullptr, find_unowned_mines_pots);
		uint32_t distant_mines =
		   map.find_fields(Area<FCoords>(field.coords, (range + 6 < 14) ? 14 : range + 6),
		                   nullptr,
		                   find_unowned_mines_pots);
		distant_mines = distant_mines - close_mines;
		field.unowned_mines_pots_nearby_ = 4 * close_mines + distant_mines / 2;
		if (distant_mines > 0) {
			field.unowned_mines_pots_nearby_ += 15;
		}
	}

	// identifying portspace fields
	if (!field.is_portspace_) {  // if we know it, no need to do it once more
		if (player_->get_buildcaps(field.coords) & BUILDCAPS_PORT) {
			field.is_portspace_ = true;
		}
	}

	// testing for near portspaces
	if (field.portspace_nearby_ == Widelands::ExtendedBool::kUnset) {
		field.portspace_nearby_ = ExtendedBool::kFalse;
		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(field.coords, 2));
		do {
			if (port_reserved_coords.count(coords_hash(mr.location())) > 0) {
				field.portspace_nearby_ = ExtendedBool::kTrue;
				break;
			}
		} while (mr.advance(map));
	}

	// testing if a port is nearby, such field will get a priority boost
	uint16_t nearest_distance = std::numeric_limits<uint16_t>::max();
	for (const WarehouseSiteObserver& wh_obs : warehousesites) {
		const uint16_t actual_distance = map.calc_distance(field.coords, wh_obs.site->get_position());
		nearest_distance = std::min(nearest_distance, actual_distance);
	}
	if (nearest_distance < 15) {
		field.port_nearby_ = true;
	} else {
		field.port_nearby_ = false;
	}

	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	// Search in a radius of range
	map.find_immovables(Area<FCoords>(field.coords, range), &immovables);

	// Is this a general update or just for military consideration
	// (second is used in check_militarysites)
	if (!military) {
		int32_t const tree_attr = MapObjectDescr::get_attribute_id("tree");
		field.preferred_ = false;
		field.enemy_nearby_ = false;
		field.area_military_capacity_ = 0;
		field.military_loneliness_ = 1000;  // instead of floats(v-
		field.area_military_presence_ = 0;
		field.military_stationed_ = 0;
		field.trees_nearby_ = 0;
		field.space_consumers_nearby_ = 0;
		field.producers_nearby_.clear();
		field.producers_nearby_.resize(wares.size());
		field.consumers_nearby_.clear();
		field.consumers_nearby_.resize(wares.size());
		std::vector<Coords> water_list;
		std::vector<Coords> resource_list;
		std::vector<Bob*> critters_list;

		if (field.water_nearby_ == -1) {  //-1 means "value has never been calculated"

			FindNodeWater find_water(game().world());
			map.find_fields(Area<FCoords>(field.coords, 5), &water_list, find_water);
			field.water_nearby_ = water_list.size();

			if (resource_necessity_water_needed_) {  // for atlanteans
				map.find_fields(Area<FCoords>(field.coords, 14), &water_list, find_water);
				field.distant_water_ = water_list.size() - field.water_nearby_;
			}
		}

		// counting fields with fish
		if (field.water_nearby_ > 0 && (field.fish_nearby_ == -1 || gametime % 10 == 0)) {
			map.find_fields(Area<FCoords>(field.coords, 6),
			                &resource_list,
			                FindNodeResource(world.get_resource("fish")));
			field.fish_nearby_ = resource_list.size();
		}

		// counting fields with critters (game)
		// not doing this always, this does not change fast
		if (gametime % 10 == 0) {
			map.find_bobs(Area<FCoords>(field.coords, 6), &critters_list, FindBobCritter());
			field.critters_nearby_ = critters_list.size();
		}

		FCoords fse;
		map.get_neighbour(field.coords, WALK_SE, &fse);

		if (BaseImmovable const* const imm = fse.field->get_immovable()) {
			if (dynamic_cast<Flag const*>(imm) ||
			    (dynamic_cast<Road const*>(imm) && (fse.field->nodecaps() & BUILDCAPS_FLAG))) {
				field.preferred_ = true;
			}
		}

		for (uint32_t i = 0; i < immovables.size(); ++i) {
			const BaseImmovable& base_immovable = *immovables.at(i).object;

			if (upcast(PlayerImmovable const, player_immovable, &base_immovable)) {

				// TODO(unknown): Only continue; if this is an opposing site
				// allied sites should be counted for military influence
				if (player_immovable->owner().player_number() != pn) {
					if (player_->is_hostile(player_immovable->owner())) {
						field.enemy_nearby_ = true;
					}

					continue;
				}
			}

			if (upcast(Building const, building, &base_immovable)) {
				if (upcast(ConstructionSite const, constructionsite, building)) {
					const BuildingDescr& target_descr = constructionsite->building();

					if (dynamic_cast<ProductionSiteDescr const*>(&target_descr)) {
						consider_productionsite_influence(
						   field,
						   immovables.at(i).coords,
						   get_building_observer(constructionsite->descr().name().c_str()));
					}
				}

				if (dynamic_cast<const ProductionSite*>(building)) {
					consider_productionsite_influence(
					   field,
					   immovables.at(i).coords,
					   get_building_observer(building->descr().name().c_str()));
				}
			}

			if (immovables.at(i).object->has_attribute(tree_attr)) {
				++field.trees_nearby_;
			}
		}

		// stones are not renewable, we will count them only if previous state is nonzero
		if (field.stones_nearby_ > 0 && gametime % 3 == 0) {

			field.stones_nearby_ =
				map.find_immovables(Area<FCoords>(map.get_fcoords(field.coords), 6),
					nullptr,
		            FindImmovableAttribute(MapObjectDescr::get_attribute_id("granite")));

			// adding 10 if stones found
			field.stones_nearby_ = (field.stones_nearby_ > 0) ? field.stones_nearby_ + 10:0;
		}

		// ground water is not renewable and its amount can only fall, we will count them only if
		// previous state is nonzero
		if (field.ground_water_ > 0) {
			field.ground_water_ = field.coords.field->get_resources_amount();
		}
	}

	// the following is done always (regardless of military or not)

	// we get immovables with higher radius
	immovables.clear();
	map.find_immovables(Area<FCoords>(field.coords, (range < 11) ? 11 : range), &immovables);
	field.military_stationed_ = 0;
	field.military_unstationed_ = 0;
	field.military_in_constr_nearby_ = 0;
	field.area_military_capacity_ = 0;
	field.military_loneliness_ = 1000;
	field.area_military_presence_ = 0;

	for (uint32_t i = 0; i < immovables.size(); ++i) {

		const BaseImmovable& base_immovable = *immovables.at(i).object;

		// testing if it is enemy-owned field
		// TODO(unknown): count such fields...
		if (upcast(PlayerImmovable const, player_immovable, &base_immovable)) {

			// TODO(unknown): Only continue; if this is an opposing site
			// allied sites should be counted for military influence
			if (player_immovable->owner().player_number() != pn) {
				if (player_->is_hostile(player_immovable->owner())) {
					field.enemy_nearby_ = true;
				}

				continue;
			}
		}

		// if we are here, immovable is ours
		if (upcast(Building const, building, &base_immovable)) {
			if (upcast(ConstructionSite const, constructionsite, building)) {
				const BuildingDescr& target_descr = constructionsite->building();

				if (upcast(MilitarySiteDescr const, target_ms_d, &target_descr)) {
					const int32_t dist = map.calc_distance(field.coords, immovables.at(i).coords);
					const int32_t radius = target_ms_d->get_conquers() + 4;

					if (radius > dist) {
						field.area_military_capacity_ +=
						   target_ms_d->get_max_number_of_soldiers() / 2 + 1;
						if (field.coords != immovables.at(i).coords) {
							field.military_loneliness_ *= static_cast<double_t>(dist) / radius;
						}
						field.military_in_constr_nearby_ += 1;
					}
				}
			}

			if (upcast(MilitarySite const, militarysite, building)) {
				const int32_t dist = map.calc_distance(field.coords, immovables.at(i).coords);
				const int32_t radius = militarysite->descr().get_conquers() + 4;

				if (radius > dist) {

					field.area_military_capacity_ += militarysite->max_soldier_capacity();
					field.area_military_presence_ += militarysite->stationed_soldiers().size();

					if (militarysite->stationed_soldiers().empty()) {
						field.military_unstationed_ += 1;
					} else {
						field.military_stationed_ += 1;
					}

					if (field.coords != immovables.at(i).coords) {
						field.military_loneliness_ *= static_cast<double_t>(dist) / radius;
					}
				}
			}
		}
	}
}

/// Updates one mineable field
void DefaultAI::update_mineable_field(MineableField& field) {
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	Map& map = game().map();
	map.find_immovables(Area<FCoords>(field.coords, 5), &immovables);
	field.preferred_ = false;
	field.mines_nearby_ = 0;
	FCoords fse;
	map.get_brn(field.coords, &fse);

	if (BaseImmovable const* const imm = fse.field->get_immovable()) {
		if (dynamic_cast<Flag const*>(imm) ||
		    (dynamic_cast<Road const*>(imm) && (fse.field->nodecaps() & BUILDCAPS_FLAG))) {
			field.preferred_ = true;
		}
	}

	for (const ImmovableFound& temp_immovable : immovables) {
		if (upcast(Building const, bld, temp_immovable.object)) {
			if (player_number() != bld->owner().player_number()) {
				continue;
			}
			if (bld->descr().get_ismine()) {
				if (get_building_observer(bld->descr().name().c_str()).mines_ ==
				    field.coords.field->get_resources()) {
					++field.mines_nearby_;
				}
			} else if (upcast(ConstructionSite const, cs, bld)) {
				if (cs->building().get_ismine()) {
					if (get_building_observer(cs->building().name().c_str()).mines_ ==
					    field.coords.field->get_resources()) {
						++field.mines_nearby_;
					}
				}
			}
		}
	}

	// 0 is default, and thus indicates that counting must be done
	if (field.same_mine_fields_nearby_ == 0) {
		FindNodeMineable find_mines_spots_nearby(game(), field.coords.field->get_resources());
		field.same_mine_fields_nearby_ =
		   map.find_fields(Area<FCoords>(field.coords, 4), nullptr, find_mines_spots_nearby);
	}
}

/// Updates the production and MINE sites statistics needed for construction decision.
void DefaultAI::update_productionsite_stats(uint32_t const gametime) {
	// Updating the stats every 10 seconds should be enough
	taskDue[ScheduleTasks::kProductionsitesStats] = gametime + 10000;
	uint16_t fishers_count = 0;  // used for atlanteans only

	// Reset statistics for all buildings
	for (uint32_t i = 0; i < buildings_.size(); ++i) {
		buildings_.at(i).current_stats_ = 0;
		buildings_.at(i).unoccupied_ = false;
		buildings_.at(i).unconnected_ = 0;
	}

	// Check all available productionsites
	for (uint32_t i = 0; i < productionsites.size(); ++i) {
		assert(productionsites.front().bo->cnt_built_ > 0);
		// is connected
		const bool connected_to_wh =
		   !productionsites.front().site->get_economy()->warehouses().empty();

		// unconnected buildings are excluded from statistics review
		if (connected_to_wh) {
			// Add statistics value
			productionsites.front().bo->current_stats_ +=
			   productionsites.front().site->get_crude_statistics();

			// counting fishers
			if (productionsites.front().bo->is_fisher_) {
				fishers_count += 1;
			}

			// Check whether this building is completely occupied
			productionsites.front().bo->unoccupied_ |=
			   !productionsites.front().site->can_start_working();
		} else {
			productionsites.front().bo->unconnected_ += 1;
		}

		// Now reorder the buildings
		productionsites.push_back(productionsites.front());
		productionsites.pop_front();
	}

	if (resource_necessity_water_needed_) {
		if (fishers_count == 0) {
			resource_necessity_water_ = 100;
		} else if (fishers_count == 1) {
			resource_necessity_water_ = 50;
		} else {
			resource_necessity_water_ = 10;
		}
	}

	// for mines_ also
	// Check all available mines
	for (uint32_t i = 0; i < mines_.size(); ++i) {
		assert(mines_.front().bo->cnt_built_ > 0);

		const bool connected_to_wh =
		   !mines_.front().site->get_economy()->warehouses().empty();

		// unconnected mines are excluded from statistics review
		if (connected_to_wh) {
			// Add statistics value
			mines_.front().bo->current_stats_ += mines_.front().site->get_statistics_percent();
			// Check whether this building is completely occupied
			mines_.front().bo->unoccupied_ |= !mines_.front().site->can_start_working();
		} else {
			mines_.front().bo->unconnected_ += 1;
		}

		// Now reorder the buildings
		mines_.push_back(mines_.front());
		mines_.pop_front();
	}

	// Scale statistics down
	for (uint32_t i = 0; i < buildings_.size(); ++i) {
		if ((buildings_.at(i).cnt_built_ - buildings_.at(i).unconnected_) > 0) {
			buildings_.at(i).current_stats_ /=
			   (buildings_.at(i).cnt_built_ - buildings_.at(i).unconnected_);
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
	// Just used for easy checking whether a mine or something else was built.
	bool mine = false;
	bool field_blocked = false;
	uint32_t consumers_nearby_count = 0;
	std::vector<int32_t> spots_avail;
	spots_avail.resize(4);
	Map& map = game().map();

	for (int32_t i = 0; i < 4; ++i)
		spots_avail.at(i) = 0;

	for (std::list<BuildableField*>::iterator i = buildable_fields.begin();
	     i != buildable_fields.end();
	     ++i)
		++spots_avail.at((*i)->coords.field->nodecaps() & BUILDCAPS_SIZEMASK);

	spots_ = spots_avail.at(BUILDCAPS_SMALL);
	spots_ += spots_avail.at(BUILDCAPS_MEDIUM);
	spots_ += spots_avail.at(BUILDCAPS_BIG);

	// here we possible stop building of new buildings
	new_buildings_stop_ = false;
	MilitaryStrategy expansion_mode = MilitaryStrategy::kResourcesOrDefense;

	// helper variable - we need some proportion of free spots vs productionsites
	// the proportion depends on size of economy
	// this proportion defines how dense the buildings will be
	// it is degressive (allows high density on the beginning)
	int32_t needed_spots = 0;
	if (productionsites.size() < 50) {
		needed_spots = productionsites.size();
	} else if (productionsites.size() < 100) {
		needed_spots =   50 + (productionsites.size() -  50) *  5;
	} else if (productionsites.size() < 200) {
		needed_spots =  300 + (productionsites.size() - 100) * 10;
	} else {
		needed_spots = 1300 + (productionsites.size() - 200) * 20;
	}

	// there are many reasons why to stop building production buildings
	// (note there are numerous exceptions)
	// 1. to not have too many constructionsites
	if (num_prod_constructionsites > productionsites.size() / 7 + 2) {
		new_buildings_stop_ = true;
	}
	// 2. to not exhaust all free spots
	if (spots_ < needed_spots) {
		new_buildings_stop_ = true;
	}
	// 3. too keep some proportions production sites vs military sites
	if ((num_prod_constructionsites + productionsites.size()) >
	    (num_milit_constructionsites + militarysites.size()) * 5) {
		new_buildings_stop_ = true;
	}
	// 4. if we do not have 3 mines at least
	if (mines_.size() < 3) {
		new_buildings_stop_ = true;
	}
	// BUT if enemy is nearby, we cancel above stop
	if (new_buildings_stop_ && enemy_last_seen_ + 10 * 60 * 1000 > gametime) {
		new_buildings_stop_ = false;
	}

	// we must calculate wood policy
	const WareIndex wood_index = tribe_->safe_ware_index("log");
	// stocked wood is to be in some propotion to productionsites and
	// constructionsites (this proportion is bit artifical, or we can say
	// it is proportion to the size of economy). Plus some positive 'margin'
	const int32_t stocked_wood_margin = get_warehoused_stock(wood_index) -
		productionsites.size() * 2 -
		num_prod_constructionsites;
	if (stocked_wood_margin > 80) {
		wood_policy_ = WoodPolicy::kDismantleRangers;
	} else if  (stocked_wood_margin > 25) {
		wood_policy_ = WoodPolicy::kStopRangers;
	} else if  (stocked_wood_margin > 10) {
		wood_policy_ = WoodPolicy::kStartRangers;
	} else {
		wood_policy_ = WoodPolicy::kBuildRangers;
	}

	// we must consider need for mines
	// set necessity for mines
	// we use 'virtual mines', because also mine spots can be changed
	// to mines when AI decides so
	const int32_t virtual_mines =
	   mines_.size() + mineable_fields.size() / 15 - productionsites.size() / 25;
	resource_necessity_mines_ = 100 * (15 - virtual_mines) / 15;
	resource_necessity_mines_ = (resource_necessity_mines_ > 100) ? 100 : resource_necessity_mines_;
	resource_necessity_mines_ = (resource_necessity_mines_ < 20) ? 10 : resource_necessity_mines_;

	// here we calculate how badly we need to expand, result is number (0-100)
	// like a percent
	if (spots_ == 0) {
		resource_necessity_territory_ = 100;
	} else {
		resource_necessity_territory_ = 100 * 5 * (productionsites.size() + 5) / spots_;
		resource_necessity_territory_ =
		   (resource_necessity_territory_ > 100) ? 100 : resource_necessity_territory_;
		resource_necessity_territory_ =
		   (resource_necessity_territory_ < 10) ? 10 : resource_necessity_territory_;
		// alse we need at lest 4 big spots
		if (spots_avail.at(BUILDCAPS_BIG) < 2) {
			resource_necessity_territory_ = 100;
		}
		if (spots_avail.at(BUILDCAPS_MEDIUM) < 4) {
			resource_necessity_territory_ = 100;
		}
	}

	// this controls a speed and willingness to expand the teritorry
	const int32_t vacant_plus_in_construction_minus_prod =
	   vacant_mil_positions_ + 2 * num_milit_constructionsites - productionsites.size() / 7;
	if (vacant_plus_in_construction_minus_prod > 20) {
		expansion_mode = MilitaryStrategy::kNoNewMilitary;
	} else if (vacant_plus_in_construction_minus_prod > 13) {
		expansion_mode = MilitaryStrategy::kDefenseOnly;
	} else if (vacant_plus_in_construction_minus_prod > 6) {
		expansion_mode = MilitaryStrategy::kResourcesOrDefense;
	} else {
		// this is intended for initial phase of game when the player still has enough soldiers
		// but we still want to force it to follow resources instead for plain expansion
		if (virtual_mines <= 2 && (unstationed_milit_buildings_ + num_milit_constructionsites) > 2) {
			expansion_mode = MilitaryStrategy::kResourcesOrDefense;
		} else if (unstationed_milit_buildings_ + num_milit_constructionsites >= 1) {
			expansion_mode = MilitaryStrategy::kExpansion;
		} else {
			expansion_mode = MilitaryStrategy::kPushExpansion;
		}
	}

	BuildingObserver* best_building = nullptr;
	int32_t proposed_priority = 0;
	Coords proposed_coords;

	// Remove outdated fields from blocker list
	for (std::list<BlockedField>::iterator i = blocked_fields.begin(); i != blocked_fields.end();)
		if (i->blocked_until_ < gametime) {
			i = blocked_fields.erase(i);
		} else {
			++i;
		}

	// testing big military buildings, whether critical construction
	// material is available (at least in amount of
	// 2/3 of default target amount)
	for (uint32_t j = 0; j < buildings_.size(); ++j) {

		BuildingObserver& bo = buildings_.at(j);
		if (!bo.buildable(*player_)) {
			continue;
		}

		// not doing this for non-military buildins
		if (!(bo.type == BuildingObserver::MILITARYSITE))
			continue;

		// and neither for small military buildings
		if (bo.desc->get_size() == BaseImmovable::SMALL)
			continue;

		bo.build_material_shortage_ = false;

		// checking we have enough critical material on stock
		for (uint32_t m = 0; m < bo.critical_built_mat_.size(); ++m) {
			WareIndex wt(static_cast<size_t>(bo.critical_built_mat_.at(m)));
			// shortage = less then 3 items in warehouses
			if (get_warehoused_stock(wt) < 3) {
				bo.build_material_shortage_ = true;
			}
		}
	}

	//Resetting output_needed_ in building observer
	for (uint32_t j = 0; j < buildings_.size(); ++j) {
		BuildingObserver& bo = buildings_.at(j);
		if (bo.type == BuildingObserver::PRODUCTIONSITE || bo.type == BuildingObserver::MINE){
			bo.output_needed_ = ExtendedBool::kUnset;
		}
	}

	// first scan all buildable fields for regular buildings
	for (std::list<BuildableField*>::iterator i = buildable_fields.begin();
	     i != buildable_fields.end();
	     ++i) {
		BuildableField* const bf = *i;

		if (bf->field_info_expiration_ < gametime) {
			continue;
		}

		// Continue if field is blocked at the moment
		field_blocked = false;

		for (std::list<BlockedField>::iterator j = blocked_fields.begin(); j != blocked_fields.end();
		     ++j) {
			if (j->coords == bf->coords) {
				field_blocked = true;
			}
		}

		// continue;
		if (field_blocked) {
			continue;
		}

		assert(player_);
		int32_t const maxsize = player_->get_buildcaps(bf->coords) & BUILDCAPS_SIZEMASK;

		// For every field test all buildings
		for (uint32_t j = 0; j < buildings_.size(); ++j) {
			BuildingObserver& bo = buildings_.at(j);

			if (!bo.buildable(*player_)) {
				continue;
			}

			if (bo.prohibited_till_ > gametime) {
				continue;
			}

			// if current field is not big enough
			if (bo.desc->get_size() > maxsize) {
				continue;
			}

			// testing for reserved ports
			if (!bo.is_port_) {
				if (port_reserved_coords.count(coords_hash(bf->coords)) > 0) {
					continue;
				}
			}

			if (time(nullptr) % 3 == 0 && bo.total_count() > 0) {
				continue;
			}  // add randomnes and ease AI

			if (bo.type == BuildingObserver::MINE) {
				continue;
			}

			// here we do an exemption for lumberjacks, mainly in early stages of game
			// sometimes the first one is not built and AI waits too long for second attempt
			if (gametime - bo.construction_decision_time_ < kBuildingMinInterval && !bo.need_trees_) {
				continue;
			}

			if (bo.unoccupied_) {
				continue;
			}

			if (!(bo.type == BuildingObserver::MILITARYSITE) && bo.cnt_under_construction_ >= 2) {
				continue;
			}

			int32_t prio = 0;  // score of a bulding on a field

			if (bo.type == BuildingObserver::PRODUCTIONSITE) {

				// exclude spots on border
				if (bf->near_border_ && !bo.need_trees_ && !bo.need_stones_ && !bo.is_fisher_) {
					continue;
				}

				// this just indicates that values must be recalculated - if this building
				// is to be considered later on in this function
				if (bo.output_needed_ == ExtendedBool::kUnset) {
					check_building_necessity(bo);
				}

				// this can be only a well (as by now)
				if (bo.mines_water_) {
					if (bf->ground_water_ < 2) {
						continue;
					}

					if (bo.cnt_under_construction_ + bo.unoccupied_ > 0) {
						continue;
					}

					prio = 0;
					// one well is forced
					if (bo.total_count() == 0) {
						prio = 200;
					}  // boost for first/only well
					else if (new_buildings_stop_) {
						continue;
					}

					bo.cnt_target_ = 1 + productionsites.size() / 50;

					if (bo.stocklevel_time < game().get_gametime() - 30 * 1000) {
						bo.stocklevel_ = get_stocklevel(bo);
						bo.stocklevel_time = game().get_gametime();
					}
					if (bo.stocklevel_ > 50 + productionsites.size() * 5) {
						continue;
					}
					prio += bf->ground_water_ - 2;
					prio = recalc_with_border_range(*bf, prio);

				} else if (bo.need_trees_) {  // LUMBERJACS

					bo.cnt_target_ =
					   3 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 15;

					if (bo.total_count() == 0) {
						prio = 500 + bf->trees_nearby_;
					}

					else if (bo.total_count() == 1) {
						prio = 400 + bf->trees_nearby_;
					}

					else if (bf->trees_nearby_ < 2) {
						continue;
					}

					else {

						if (bo.total_count() < bo.cnt_target_) {
							prio = 75;
						} else {
							prio = 0;
						}

						if (bf->producers_nearby_.at(bo.outputs_.at(0)) > 1) {
							continue;
						}

						prio += 2 * bf->trees_nearby_ - 10 -
						        bf->producers_nearby_.at(bo.outputs_.at(0)) * 5 -
						        new_buildings_stop_ * 15;

						if (bf->near_border_) {
							prio = prio / 2;
						}
					}

				} else if (bo.need_stones_) {

					// quaries are generally to be built everywhere where stones are
					// no matter the need for stones, as stones are considered an obstacle
					// to expansion
					if (bo.cnt_under_construction_ > 0) {
						continue;
					}
					prio = bf->stones_nearby_;

					// value is initialized with 1 but minimal value that can be
					// calculated is 11
					if (prio <= 1) {
						continue;
					}

					if (bo.total_count() - bo.unconnected_ == 0) {
						prio += 150;
					}

					if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
						bo.stocklevel_ = get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
						bo.stocklevel_time = game().get_gametime();
					}

					if (bo.stocklevel_ == 0) {
						prio *= 2;
					}

					// to prevent to many quaries on one spot
					prio = prio - 50 * bf->producers_nearby_.at(bo.outputs_.at(0));

					if (bf->near_border_) {
						prio = prio / 2;
					}

				} else if (bo.is_hunter_) {
					if (bf->critters_nearby_ < 5) {
						continue;
					}

					if (gametime > 5 * 60 * 1000 &&
					(bo.total_count() - bo.unconnected_  == 0)) {
						prio += 10;
					} else if (new_buildings_stop_) {
						continue;
					}

					prio +=
					   (bf->critters_nearby_ * 3) - 8 - 5 * bf->producers_nearby_.at(bo.outputs_.at(0));

				} else if (bo.is_fisher_) {  // fisher

					// ~are fishes needed?
					if (bo.max_needed_preciousness_ == 0) {
						continue;
					}

					if (bo.cnt_under_construction_ + bo.unoccupied_ > 0) {
						continue;
					}

					if (bf->water_nearby_ < 2) {
						continue;
					}

					// we use preciousness to allow atlanteans to build the fishers huts
					// atlanteans have preciousness 4, other tribes 3
					if (bo.max_needed_preciousness_ < 4 && new_buildings_stop_) {
						continue;
					}

					if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
						bo.stocklevel_ = get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
						bo.stocklevel_time = game().get_gametime();
					}

					if (bo.stocklevel_ > 50) {
						continue;
					}

					if (bf->producers_nearby_.at(bo.outputs_.at(0)) >= 1) {
						continue;
					}

					prio = bf->fish_nearby_ - new_buildings_stop_ * 15 * bo.total_count();

				} else if (bo.production_hint_ >= 0) {
					// first setting targets (needed also for dismantling)
					if (bo.plants_trees_) {
						bo.cnt_target_ =
						   2 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 15;
					} else {
						bo.cnt_target_ =
						   1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 20;
					}

					if ((bo.cnt_under_construction_ + bo.unoccupied_) > 1) {
						continue;
					}

					if (bo.plants_trees_) {  // RANGERS

						// if there are too many trees nearby
						if (bf->trees_nearby_ > 25 && bo.total_count() >= 1) {
							continue;
						}

						// for small starting spots - to prevent crowding by rangers and trees
						if (spots_ < (10 * bo.total_count()) && bo.total_count() > 0) {
							continue;
						}

						if (bo.total_count() == 0) {
							prio = 200;
						}
						if (bo.total_count() > 2 * bo.cnt_target_) {
							continue;
						}
						// we can go above target if there is shortage of logs on stock
						else if (bo.total_count() >= bo.cnt_target_) {
							if (wood_policy_ != WoodPolicy::kBuildRangers) {
								continue;
							}
						}

						// considering near trees and producers
						prio += (30 - bf->trees_nearby_) * 2 +
						        bf->producers_nearby_.at(bo.production_hint_) * 5 -
						        new_buildings_stop_ * 15;

					} else {  // FISH BREEDERS and GAME KEEPERS
						if (new_buildings_stop_ && (bo.total_count() - bo.unconnected_) > 0) {
							continue;
						}

						// especially for fish breeders
						if (bo.need_water_ && bf->water_nearby_ < 2) {
							continue;
						}
						if (bo.need_water_) {
							prio += bf->water_nearby_ / 5;
						}

						if ((bo.total_count() - bo.unconnected_) > bo.cnt_target_) {
							continue;
						}

						if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
							bo.stocklevel_ =
							   get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
							bo.stocklevel_time = game().get_gametime();
						}
						if (bo.stocklevel_ > 50) {
							continue;
						}

						if (bo.total_count() == 0 && gametime > 45 * 1000) {
							prio += 100 + bf->producers_nearby_.at(bo.production_hint_) * 10;
						} else if (bf->producers_nearby_.at(bo.production_hint_) == 0) {
							continue;
						} else {
							prio += bf->producers_nearby_.at(bo.production_hint_) * 10;
						}

						if (bf->enemy_nearby_) {
							prio -= 10;
						}
					}

				} else if (bo.recruitment_ && !new_buildings_stop_) {
					// this will depend on number of mines_ and productionsites
					if (static_cast<int32_t>((productionsites.size() + mines_.size()) / 30) >
					       bo.total_count() &&
					    (bo.cnt_under_construction_ + bo.unoccupied_) == 0 &&
					    // but only if current buildings are utilized enough
					    (bo.total_count() == 0 || bo.current_stats_ > 60)) {
							prio = 4 + kDefaultPrioBoost;
						}
				} else {  // finally normal productionsites
					if (bo.production_hint_ >= 0) {
						continue;
					}

					// generally we allow 1 building in construction, but if
					// preciousness of missing ware is >=10 and it is farm-like building
					// we allow 2 in construction
					if (bo.max_needed_preciousness_ >= 10
						&& bo.inputs_.empty()
						&& gametime > 30 * 60 * 1000) {
						if ((bo.cnt_under_construction_ + bo.unoccupied_) > 1) {
							continue;
						}
					} else {
						if ((bo.cnt_under_construction_ + bo.unoccupied_) > 0) {
							continue;
						}
					}

					if (bo.forced_after_ < gametime && (bo.total_count() - bo.unconnected_) == 0) {
						prio += 150;
					} else if ((bo.cnt_built_ - bo.unconnected_) == 1 &&
					           game().get_gametime() > 40 * 60 * 1000 &&
					           bo.desc->enhancement() != INVALID_INDEX && !mines_.empty()) {
						prio += 10;
					} else if (bo.is_shipyard_) {
						if (!seafaring_economy) {
							continue;
						}
					} else if (bo.output_needed_ == ExtendedBool::kFalse) {
						continue;
					} else if ((bo.cnt_built_ - bo.unconnected_) == 0 &&
					           game().get_gametime() > 40 * 60 * 1000) {
						prio += kDefaultPrioBoost;
					} else if ((bo.cnt_built_ - bo.unconnected_) > 1 && bo.current_stats_ > 97) {
						prio -= kDefaultPrioBoost * (new_buildings_stop_);
					} else if (new_buildings_stop_) {
						continue;
					}
					// we check separatelly buildings with no inputs and some inputs
					if (bo.inputs_.empty()) {

						prio += bo.max_needed_preciousness_ + kDefaultPrioBoost;

						if (bo.space_consumer_) {  // need to consider trees nearby
							prio += 20 - (bf->trees_nearby_ / 3);
						}

						// we attempt to cluster space consumers together
						if (bo.space_consumer_) {  // need to consider trees nearby
							prio += bf->space_consumers_nearby_ * 10;
							prio += (bf->space_consumers_nearby_ > 2) ? 2 : bf->space_consumers_nearby_ * 2;
						}

						if (bo.space_consumer_ && !bf->water_nearby_) {  // not close to water
							prio += 1;
						}

						if (bo.space_consumer_ &&
						    !bf->unowned_mines_pots_nearby_) {  // not close to mountains
							prio += 1;
						}

						if (!bo.space_consumer_) {
							prio -= bf->producers_nearby_.at(bo.outputs_.at(0)) * 20;
						}  // leave some free space between them
					}

					else if (bo.is_shipyard_) {
						// for now AI builds only one shipyard
						if (bf->water_nearby_ > 3 && (bo.total_count() - bo.unconnected_) == 0 &&
						    seafaring_economy) {
							prio += kDefaultPrioBoost + productionsites.size() * 5 + bf->water_nearby_;
						}

					} else if (!bo.inputs_.empty()) {
						if ((bo.total_count() - bo.unconnected_ == 0)) {
							prio += bo.max_needed_preciousness_ + kDefaultPrioBoost;
						}
						if ((bo.cnt_built_ - bo.unconnected_) > 0
							&&
							is_productionsite_needed(bo.outputs_.size(),
													bo.current_stats_,
													PerfEvaluation::kForConstruction)) {
							prio += bo.max_needed_preciousness_ + kDefaultPrioBoost - 3 +
							        (bo.current_stats_ - 55) / 8;
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

					//+1 if any consumers_ are nearby
					consumers_nearby_count = 0;

					for (size_t k = 0; k < bo.outputs_.size(); ++k)
						consumers_nearby_count += bf->consumers_nearby_.at(bo.outputs_.at(k));

					if (consumers_nearby_count > 0) {
						prio += 1;
					}
				}

				// consider borders (for medium + big buildings and ones with input)
				// =>decreasing the score
				// but only if we have enough free spots to built on
				// otherwise it will slow down the expansion - small buildings would be preferred
				if (spots_avail.at(BUILDCAPS_MEDIUM) > 40
					&&
					spots_avail.at(BUILDCAPS_BIG) > 20
					&&
					(bo.desc->get_size() == 2 ||
					 bo.desc->get_size() == 3 ||
					 !bo.inputs_.empty())) {
						prio = recalc_with_border_range(*bf, prio);
					}

			}  // production sites done
			else if (bo.type == BuildingObserver::MILITARYSITE) {

				if (!bf->unowned_land_nearby_) {
					continue;
				}

				if (military_last_build_ > gametime - 10 * 1000) {
					continue;
				}

				prio = 0;

				// calculating some sub-scores, some of them are prohibitive
				// decreasing score if some critical materials are missing
				// (relevant for medium and big buildings)
				int32_t prio_for_mat_shortage = 0;
				prio_for_mat_shortage -= bo.cnt_under_construction_ * bo.build_material_shortage_;
				if (bf->enemy_nearby_) {
					prio_for_mat_shortage *= 50;
				} else {
					prio_for_mat_shortage *= 1000;  // = prohibitive
				}

				// decreasing score if other militarysites constuctions
				// are nearby
				int32_t prio_for_in_constr = 0;
				prio_for_in_constr -= 700 * (((static_cast<int32_t>(num_milit_constructionsites) - 2) < 0) ?
									  0 :
									  (num_milit_constructionsites - 2)) /
						(militarysites.size() + 2);
				if (!bf->enemy_nearby_) {
					prio_for_in_constr *= 3;
				}

				// similarly if unmanned militarysites are nearby
				int32_t prio_for_unmanned_nearby = 0;
				prio_for_unmanned_nearby -= bf->military_in_constr_nearby_ + bf->military_unstationed_;
				if (bf->enemy_nearby_) {
					prio_for_unmanned_nearby *= 20;
				} else {
					prio_for_unmanned_nearby *= 1000;
				}

				// not continuing if score too low
				if (prio_for_in_constr + prio_for_mat_shortage + prio_for_unmanned_nearby <= -1000) {
					continue;
				}

				// is the building suitable for the situation?
				if (expansion_mode == MilitaryStrategy::kNoNewMilitary) {
					continue;
				}

				if (expansion_mode == MilitaryStrategy::kDefenseOnly && !bf->enemy_nearby_) {
					continue;
				}

				if (expansion_mode == MilitaryStrategy::kResourcesOrDefense &&
				    !(bf->unowned_mines_pots_nearby_ || bf->stones_nearby_ || bf->water_nearby_ ||
				      (bf->distant_water_ && resource_necessity_water_needed_) || bf->enemy_nearby_)) {
					continue;
				}

				if (bf->enemy_nearby_ && bo.fighting_type_) {
					;
				}  // it is ok, go on
				else if (bf->unowned_mines_pots_nearby_ > 2 &&
				         (bo.mountain_conqueror_ || bo.expansion_type_)) {
					;
				}  // it is ok, go on
				else if (bo.expansion_type_) {
					if (bo.desc->get_size() == 2 && gametime % 2 >= 1) {
						continue;
					}
					if (bo.desc->get_size() == 3 && gametime % 3 >= 1) {
						continue;
					};
				} else {
					continue;
				}  // the building is not suitable for situation


				// calculating other sub scores
				// for resources (mines, water, stones)
				int32_t prio_for_resources = 0;
				prio_for_resources += 2 * (bf->unowned_mines_pots_nearby_ * resource_necessity_mines_) / 100 +
						bf->stones_nearby_ + (bf->water_nearby_ * resource_necessity_water_) / 100;
				// special bonus due to remote water for atlanteans
				if (resource_necessity_water_needed_) {
					prio_for_resources += (bf->distant_water_ * resource_necessity_water_) / 100 / 3;
				}
				// reducing score if too little unowned land
				if (bf->unowned_land_nearby_ < 10) {
					prio_for_resources = prio_for_resources * bf->unowned_land_nearby_ / 10;
				}
				if (bf->enemy_nearby_) {  // not important when fighting enemies
					prio_for_resources /= 5;
				}

				// for unowned land nearby
				int32_t prio_for_unowned_land = 0;
				if (expansion_mode == MilitaryStrategy::kExpansion ||
				    expansion_mode == MilitaryStrategy::kPushExpansion) {
					prio_for_unowned_land +=
					   (bf->unowned_land_nearby_ * resource_necessity_territory_) / 100;
				}

				// for distance to nearest military sites
				int32_t prio_for_loneliness = bf->military_loneliness_;
				if (!bf->enemy_nearby_) {
					prio_for_loneliness /= 10;
				}

				// additional score for bigger buildings
				int32_t prio_for_size = bo.desc->get_size() - 1;
				if (bf->enemy_nearby_) {
					prio_for_size *= 30;
				} else {
					prio_for_size *= 15;
				}

				// additional score if enemy is nearby
				int32_t prio_for_enemy = 0;
				if (bf->enemy_nearby_) {
					prio_for_enemy += 50;
				}

				// a boost to prevent an expansion halt
				int32_t local_boost = 0;
				if (expansion_mode == MilitaryStrategy::kPushExpansion) {
					local_boost = 200;
				}

				// summing
				prio += prio_for_mat_shortage + prio_for_in_constr + prio_for_unmanned_nearby +
				        prio_for_resources + prio_for_unowned_land + prio_for_loneliness +
				        prio_for_size + local_boost + prio_for_enemy;

				// special bonus if a portspace is close
				if (bf->portspace_nearby_ == ExtendedBool::kTrue) {
					if (num_ports == 0) {
						prio += 25;
					} else {
						prio += 5;
					}
				}

				// penalty if we build lesser than possible building
				if (bo.desc->get_size() < maxsize) {
					prio = prio - 5;
				}

				// just generally prevent too many buildings
				prio -= 40;

			} else if (bo.type == BuildingObserver::WAREHOUSE) {

				// exclude spots on border
				if (bf->near_border_ && !bo.is_port_) {
					continue;
				}

				if (!bf->is_portspace_ && bo.is_port_) {
					continue;
				}

				if (bo.cnt_under_construction_ > 0) {
					continue;
				}

				bool warehouse_needed = false;

				//  Build one warehouse for ~every 35 productionsites and mines_.
				//  Militarysites are slightly important as well, to have a bigger
				//  chance for a warehouses (containing waiting soldiers or wares
				//  needed for soldier training) near the frontier.
				prio = static_cast<int32_t>(productionsites.size() + mines_.size()) + 20
				-
				35 * static_cast<int32_t>(numof_warehouses_);
				if (prio > 0) {
					warehouse_needed = true;
				} else {
					prio = 0;
				}

				// But we still can built a port if it is first one
				if (bo.is_port_ && bo.total_count() == 0 && productionsites.size() > 5 &&
				    !bf->enemy_nearby_ && bf->is_portspace_ && seafaring_economy) {
					prio += kDefaultPrioBoost + productionsites.size();
					warehouse_needed = true;
				}

				if (!warehouse_needed) {
					continue;
				}

				// we prefer ports to a normal warehouse
				if (bo.is_port_) {
					prio += 15;
				}

				// it is good to have more then 1 warehouse
				if (numof_warehouses_ == 1) {
					prio += 10;
				}

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
				prio += nearest_distance - 10;

				// dont be close to enemies
				if (bf->enemy_nearby_){
					if (bo.is_port_) {
						prio -= 10;
					} else {
						prio -= 40;
					}
				}

				// being too close to a border is not good either
				if (bf->unowned_land_nearby_ && !bo.is_port_ && prio > 0) {
					prio /= 2;
					prio -= 10;
				}

			} else if (bo.type == BuildingObserver::TRAININGSITE) {

				// exclude spots on border
				if (bf->near_border_) {
					continue;
				}

				// it is a bit difficult to get a new trainer.....
				if (ts_without_trainers_) {
					continue;
				}

				// target is only one for both types
				if ((ts_basic_const_count_ + ts_advanced_const_count_) > 0) {
					continue;
				}

				// we build one basic training site for 50 militarysites
				if (bo.trainingsite_type_ == TrainingSiteType::kBasic &&
				    militarysites.size() / 50 < static_cast<int32_t>(ts_basic_count_)) {
					continue;
				}
				// we build one advanced training site for 75 militarysites
				if (bo.trainingsite_type_ == TrainingSiteType::kAdvanced &&
				    militarysites.size() / 75 < static_cast<int32_t>(ts_advanced_count_)) {
					continue;
				}

				// for type1 we need 15 productionsties
				if (bo.trainingsite_type_ == TrainingSiteType::kBasic && productionsites.size() < 15) {
					continue;
				}

				// for type2 we need 4 mines
				if (bo.trainingsite_type_ == TrainingSiteType::kAdvanced && virtual_mines < 4) {
					continue;
				}

				prio = 4 + kDefaultPrioBoost;

				// take care about borders and enemies
				if (bf->enemy_nearby_) {
					prio /= 2;
				}

				if (bf->unowned_land_nearby_) {
					prio /= 2;
				}
			}

			// think of space consuming buildings nearby like farms or vineyards
			if (bo.type != BuildingObserver::MILITARYSITE) {
				prio -= bf->space_consumers_nearby_ * 10;
			}

			// Stop here, if priority is 0 or less.
			if (prio <= 0) {
				continue;
			}

			// testing also vicinity
			if (!bo.is_port_) {
				if (port_reserved_coords.count(coords_hash(bf->coords)) > 0) {
					continue;
				}
			}

			// Prefer road side fields
			prio += bf->preferred_ ? 5 : 0;
			// don't waste good land for small huts
			prio -= (maxsize - bo.desc->get_size()) * 20;

			// prefer vicinity of ports (with exemption of warehouses)
			if (bf->port_nearby_ && bo.type == BuildingObserver::MILITARYSITE) {
				prio *= 2;
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

			for (uint32_t i = 0; i < buildings_.size() && productionsites.size() > 8; ++i) {
				BuildingObserver& bo = buildings_.at(i);

				if (!bo.buildable(*player_) || bo.type != BuildingObserver::MINE) {
					continue;
				}

				if (bo.prohibited_till_ > gametime) {
					continue;
				}

				if (gametime - bo.construction_decision_time_ < kBuildingMinInterval) {
					continue;
				}

				// Don't build another building of this type, if there is already
				// one that is unoccupied_ at the moment
				// or under construction
				if ((bo.cnt_under_construction_ + bo.unoccupied_) > 0) {
					continue;
				}

				if (bo.output_needed_ == ExtendedBool::kUnset) {
					check_building_necessity(bo);
				}

				if (bo.output_needed_ == ExtendedBool::kFalse
					&&
				 	(bo.total_count() - bo.unconnected_) > 0) {
						continue;
				}

				// if current one(s) are performing badly
				if ((bo.total_count() - bo.unconnected_) >= 1 && bo.current_stats_ < 50) {
					continue;
				}

				// this is penalty if there are existing mines too close
				// it is treated as multiplicator for count of near mines
				uint32_t nearness_penalty = 0;
				if ((mines_per_type[bo.mines_].in_construction + mines_per_type[bo.mines_].finished) ==
				    0) {
					nearness_penalty = 0;
				} else {
					nearness_penalty = 40;
				}

				// bonus score to prefer if too few mines
				uint32_t bonus_score = 0;
				if ((mines_per_type[bo.mines_].in_construction + mines_per_type[bo.mines_].finished) ==
				    0) {
					bonus_score = 15;
				} else if ((mines_per_type[bo.mines_].in_construction +
				            mines_per_type[bo.mines_].finished) == 1) {
					bonus_score = 5;
				}

				// iterating over fields
				for (std::list<MineableField*>::iterator j = mineable_fields.begin();
				     j != mineable_fields.end();
				     ++j) {

					MineableField* const mf = *j;

					if (mf->field_info_expiration_ <= gametime) {
						continue;
					}

					if (mf->coords.field->get_resources() != bo.mines_) {
						continue;
					}

					int32_t prio = 0;
					MapRegion<Area<FCoords>> mr(map, Area<FCoords>(mf->coords, 2));
					do {
						if (bo.mines_ == mr.location().field->get_resources()) {
							prio += mr.location().field->get_resources_amount();
						}
					} while (mr.advance(map));

					prio /= 10;

					// Only build mines_ on locations where some material can be mined
					if (prio < 1) {
						continue;
					}

					// applying nearnes penalty
					prio -= mf->mines_nearby_ * nearness_penalty;

					// applying bonus score
					prio += bonus_score;

					// applying max needed
					prio += bo.max_needed_preciousness_ * 3;

					// prefer mines in the middle of mine fields of the
					// same type, so we add a small bonus here
					// depending on count of same mines nearby,
					// though this does not reflects how many resources
					// are (left) in nearby mines
					prio += mf->same_mine_fields_nearby_;

					// Continue if field is blocked at the moment
					bool blocked = false;

					for (std::list<BlockedField>::iterator k = blocked_fields.begin();
					     k != blocked_fields.end();
					     ++k)
						if ((*j)->coords == k->coords) {
							blocked = true;
							break;
						}

					if (blocked) {

						continue;
					}

					// Prefer road side fields
					prio += mf->preferred_ ? 1 : 0;

					if (prio > proposed_priority) {
						best_building = &bo;
						proposed_priority = prio;
						proposed_coords = mf->coords;
						mine = true;
					}
				}  // end of evaluation of field
			}

		}  // section if mine size >0
	}     // end of mines_ section

	// if there is no winner:
	if (best_building == nullptr) {
		return false;
	}

	// send the command to construct a new building
	game().send_player_build(player_number(), proposed_coords, best_building->id);
	BlockedField blocked(
	   game().map().get_fcoords(proposed_coords), game().get_gametime() + 120000);  // two minutes
	blocked_fields.push_back(blocked);

	// we block also nearby fields
	// if farms and so on, for quite a long time
	// if military sites only for short time for AI can update information on near buildable fields
	if ((best_building->space_consumer_ && !best_building->plants_trees_) ||
	    best_building->type == BuildingObserver::MILITARYSITE) {
		uint32_t block_time = 0;
		uint32_t block_area = 0;
		if (best_building->space_consumer_) {
			block_time = 45 * 60 * 1000;
			block_area = 3;
		} else {  // militray buildings for a very short time
			block_time = 25 * 1000;
			block_area = 6;
		}

		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(map.get_fcoords(proposed_coords), block_area));
		do {
			BlockedField blocked2(
			   map.get_fcoords(*(mr.location().field)), game().get_gametime() + block_time);
			blocked_fields.push_back(blocked2);
		} while (mr.advance(map));
	}

	if (!(best_building->type == BuildingObserver::MILITARYSITE)) {
		best_building->construction_decision_time_ = gametime;
	} else {  // very ugly hack here
		military_last_build_ = gametime;
		best_building->construction_decision_time_ = gametime - kBuildingMinInterval / 2;
	}

	// set the type of update that is needed
	if (mine) {
		next_mine_construction_due_ = gametime + kBusyMineUpdateInterval;
	}

	return true;
}

// improves current road system
bool DefaultAI::improve_roads(uint32_t gametime) {

	// first force a split on roads that are longer than 3 parts
	// with exemption when there is too few building spots
	if (spots_ > 20 && !roads.empty()) {
		const Path& path = roads.front()->get_path();

		if (path.get_nsteps() > 3) {
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
		}

		roads.push_back(roads.front());
		roads.pop_front();

		// occasionaly we test if the road can be dismounted
		if (gametime % 5 == 0) {
			const Road& road = *roads.front();
			if (dispensable_road_test(*const_cast<Road*>(&road))) {
				game().send_player_bulldoze(*const_cast<Road*>(&road));
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
	if (Building* b = flag.get_building()) {
		BuildingObserver& bo = get_building_observer(b->descr().name().c_str());
		if (bo.type == BuildingObserver::WAREHOUSE) {
			is_warehouse = true;
		}
	}

	// if this is end flag (or sole building) or just randomly
	if (flag.nr_of_roads() <= 1 || gametime % 10 == 0) {
		create_shortcut_road(flag, 11, 20, gametime);
		inhibit_road_building_ = gametime + 800;
		// a warehouse with 3 or less roads
	} else if (is_warehouse && flag.nr_of_roads() <= 3) {
		create_shortcut_road(flag, 9, -1, gametime);
		inhibit_road_building_ = gametime + 400;
		// and when a flag is full with wares
	} else if (flag.current_wares() > 5) {
		create_shortcut_road(flag, 9, -2, gametime);
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

	if (roadstartflag.current_wares() > 0 || roadendflag.current_wares() > 0) {
		return false;
	}

	std::priority_queue<NearFlag> queue;
	// only used to collect flags reachable walking over roads
	std::vector<NearFlag> reachableflags;
	queue.push(NearFlag(roadstartflag, 0, 0));
	uint8_t pathcounts = 0;
	uint8_t checkradius = 8;
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

// is productionsite needed
// used for building new buildings or dismantle of old, intended for ones
// that have inputs
bool DefaultAI::is_productionsite_needed(int32_t outputs,
										int32_t performance,
										PerfEvaluation purpose) {
	int32_t expected_performance = 0;
	if (outputs > 0) {
		expected_performance = 10 + 70 / outputs;
	} else {
		expected_performance = 80;
	}
	if (purpose == PerfEvaluation::kForDismantle) {
		expected_performance /= 2;
	}
	if (performance > expected_performance) {
		return true;
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
	    eco->dismantle_grace_time_ != std::numeric_limits<int32_t>::max()) {
		eco->dismantle_grace_time_ = std::numeric_limits<int32_t>::max();
	}

	// first we deal with situations when this is economy with no warehouses
	// and this is a flag belonging to a building/constructionsite
	// such economy must get dismantle grace time (if not set yet)
	// end sometimes extended checkradius
	if (flag.get_economy()->warehouses().empty() && flag.get_building()) {

		// occupied military buildings get special treatment
		//(extended grace time + longer radius)
		bool occupied_military_ = false;
		Building* b = flag.get_building();
		if (upcast(MilitarySite, militb, b)) {
			if (militb->stationed_soldiers().size() > 0) {
				occupied_military_ = true;
			}
		}

		// if we are within grace time, it is OK, just go on
		if (eco->dismantle_grace_time_ > gametime &&
		    eco->dismantle_grace_time_ != std::numeric_limits<int32_t>::max()) {
			;

			// if grace time is not set, this is probably first time without a warehouse and we must
			// set it
		} else if (eco->dismantle_grace_time_ == std::numeric_limits<int32_t>::max()) {

			// constructionsites
			if (upcast(ConstructionSite const, constructionsite, flag.get_building())) {
				BuildingObserver& bo =
				   get_building_observer(constructionsite->building().name().c_str());
				// first very special case - a port (in the phase of constructionsite)
				// this might be a new colonization port
				if (bo.is_port_) {
					eco->dismantle_grace_time_ = gametime + 60 * 60 * 1000;  // one hour should be enough
				} else {  // other constructionsites, usually new (standalone) constructionsites
					eco->dismantle_grace_time_ =
					   gametime + 30 * 1000 +            // very shot time is enough
					   (eco->flags.size() * 30 * 1000);  // + 30 seconds for every flag in economy
				}

				// buildings
			} else {

				if (occupied_military_) {
					eco->dismantle_grace_time_ =
					   (gametime + 90 * 60 * 1000) + (eco->flags.size() * 20 * 1000);

				} else {  // for other normal buildings
					eco->dismantle_grace_time_ =
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

	// 1. first we collect all reachange points
	std::vector<NearFlag> nearflags;
	std::unordered_set<uint32_t> lookuptable;

	FindNodeWithFlagOrRoad functor;
	CheckStepRoadAI check(player_, MOVECAPS_WALK, true);
	std::vector<Coords> reachable;

	// vector reachable now contains all suitable fields
	const uint32_t reachable_fields_count = map.find_reachable_fields(
	   Area<FCoords>(map.get_fcoords(flag.get_position()), checkradius), &reachable, check, functor);

	if (reachable_fields_count == 0) {
		return false;
	}

	for (const Coords& reachable_coords : reachable) {

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

			// now make sure that this field has not been processed yet
			const uint32_t hash = coords_hash(reachable_coords);
			if (lookuptable.count(hash) == 0) {
				lookuptable.insert(hash);

				// adding flag into NearFlags if road is possible
				std::unique_ptr<Path> path2(new Path());

				if (map.findpath(flag.get_position(), reachable_coords, 0, *path2, check) >= 0) {

					// path is possible, but for now we presume connection
					//'walking on existing roads' is not possible
					// so we assign 'virtual distance'
					int32_t virtual_distance = 0;
					// the same economy, but connection not spotted above via "walking on roads"
					// algorithm
					if (player_immovable->get_economy() == flag.get_economy()) {
						virtual_distance = 50;
					} else  // and now different economies
					{
						virtual_distance = 100;
					}

					// distance as the crow flies
					const int32_t dist = map.calc_distance(flag.get_position(), reachable_coords);

					nearflags.push_back(
					   NearFlag(*dynamic_cast<const Flag*>(map[reachable_coords].get_immovable()),
					            virtual_distance,
					            dist));
				}
			}
		}
	}

	// now we walk over roads and if field is reachable by roads, we change distance asigned before
	std::priority_queue<NearFlag> queue;
	std::vector<NearFlag> nearflags_tmp;  // only used to collect flags reachable walk over roads
	queue.push(NearFlag(flag, 0, 0));

	// algorithm to walk on roads
	while (!queue.empty()) {
		std::vector<NearFlag>::iterator f =
		   find(nearflags_tmp.begin(), nearflags_tmp.end(), queue.top().flag);

		if (f != nearflags_tmp.end()) {
			queue.pop();
			continue;
		}

		nearflags_tmp.push_back(queue.top());
		queue.pop();
		NearFlag& nf = nearflags_tmp.back();

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

			if (dist > checkradius) {  //  out of range of interest
				continue;
			}

			queue.push(NearFlag(*endflag, nf.cost_ + road->get_path().get_nsteps(), dist));
		}
	}

	// iterating over nearflags_tmp, each item in nearflags_tmp should be contained also in nearflags
	// so for each corresponding field in nearflags we update "cost" (distance on existing roads)
	// to actual value
	for (std::vector<NearFlag>::iterator nf_walk_it = nearflags_tmp.begin();
	     nf_walk_it != nearflags_tmp.end();
	     ++nf_walk_it) {
		uint32_t const hash_walk = coords_hash(nf_walk_it->flag->get_position());
		if (lookuptable.count(hash_walk) > 0) {
			// iterating over nearflags
			for (std::vector<NearFlag>::iterator nf_it = nearflags.begin(); nf_it != nearflags.end();
			     ++nf_it) {
				uint32_t const hash = coords_hash(nf_it->flag->get_position());
				if (hash == hash_walk) {
					// decreasing "cost" (of walking via roads)
					if (nf_it->cost_ > nf_walk_it->cost_) {
						nf_it->cost_ = nf_walk_it->cost_;
					}
				}
			}
		}
	}

	// ordering nearflags
	std::sort(nearflags.begin(), nearflags.end(), CompareShortening());

	// this is just a random number, will be used later
	int32_t random_gametime = game().get_gametime();

	// the problem here is that send_player_build_road() does not return success/failed
	// if it did, we would just test the first nearflag, then go on with further flags until
	// a road is built or nearflags are exhausted
	// but now we must just randomly pick one of nearflags
	// probabililty of picking decreases with position in nearflags
	for (uint32_t i = 0; i < nearflags.size() && i < 10; ++i) {
		NearFlag& nf = nearflags.at(i);

		// terminating looping if reduction is too low (nearflags are sorted by reduction)
		if ((nf.cost_ - nf.distance_) < min_reduction) {
			return false;
		}

		// testing the nearflag
		// usually we allow connecting only if both flags are closer then 'checkradius-2'
		// with exeption the flag belongs to a small economy (typically a new building not connected
		// yet)
		if ((nf.cost_ - nf.distance_) >= min_reduction && nf.distance_ >= 2 &&
		    nf.distance_ < checkradius - 2) {

			// sometimes the shortest road is not the buildable, even if map.findpath claims so
			// best so we add some randomness
			random_gametime /= 3;
			if (random_gametime % 3 > 1) {
				continue;
			}

			Path& path = *new Path();

			// value of pathcost is not important, it just indicates, that the path can be built
			const int32_t pathcost =
			   map.findpath(flag.get_position(), nf.flag->get_position(), 0, path, check);

			if (pathcost >= 0) {
				if (static_cast<int32_t>(nf.cost_ - path.get_nsteps()) > min_reduction) {
					game().send_player_build_road(player_number(), path);
					return true;
				}
			}
			delete &path;
		}
	}

	// if all possible roads skipped
	if (last_attempt_) {
		Building* bld = flag.get_building();
		// first we block the field for 15 minutes, probably it is not good place to build a
		// building on
		BlockedField blocked(
		   game().map().get_fcoords(bld->get_position()), game().get_gametime() + 15 * 60 * 1000);
		blocked_fields.push_back(blocked);
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
	     obs_iter != economies.end();
	     ++obs_iter) {
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
		site.unoccupied_till_ = game().get_gametime();
	}

	// is it connected to wh at all?
	const bool connected_to_wh = !site.site->get_economy()->warehouses().empty();

	// do not dismantle or upgrade the same type of building too soon - to give some time to update
	// statistics
	if (site.bo->last_dismantle_time_ > game().get_gametime() - 30 * 1000) {
		return false;
	}

	// Get max radius of recursive workarea
	WorkareaInfo::size_type radius = 0;
	const WorkareaInfo& workarea_info = site.bo->desc->m_workarea_info;
	for (const std::pair<uint32_t, std::set<std::string>>& temp_info : workarea_info) {
		if (radius < temp_info.first) {
			radius = temp_info.first;
		}
	}

	Map& map = game().map();

	// first we try to upgrade
	// Upgrading policy
	// a) if there are two buildings and none enhanced and there are workers
	// available, one is to be enhanced
	// b) if there are two buildings
	// statistics percents are decisive
	// c) yet there are buildings that might be upgraded, even when
	// there is no second buiding of the kind (flag upgrade_substitutes_)

	const BuildingIndex enhancement = site.site->descr().enhancement();
	if (connected_to_wh && enhancement != INVALID_INDEX &&
	    ((site.bo->cnt_built_ - site.bo->unoccupied_) > 1 ||
	    site.bo->upgrade_substitutes_)) {

		BuildingIndex enbld = INVALID_INDEX;  // to get rid of this

		// Only enhance buildings that are allowed (scenario mode)
		// do not do decisions too fast
		if (player_->is_building_type_allowed(enhancement)) {

			const BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
			BuildingObserver& en_bo = get_building_observer(bld.name().c_str());
			BuildingObserver* bestbld = nullptr;

			if (gametime - en_bo.construction_decision_time_ >= kBuildingMinInterval &&
			    (en_bo.cnt_under_construction_ + en_bo.unoccupied_) == 0) {

				// don't upgrade without workers
				if (site.site->has_workers(enhancement, game())) {

					// forcing first upgrade
					if (en_bo.cnt_built_ == 0) {
						enbld = enhancement;
						bestbld = &en_bo;
					}

					// if the decision was not made yet, consider normal upgrade
					if (enbld == INVALID_INDEX) {
						// compare the performance %
						if (static_cast<int32_t>(en_bo.current_stats_) -
						       static_cast<int32_t>(site.bo->current_stats_) >
						    20) {

							enbld = enhancement;
							bestbld = &en_bo;
						}

						if ((static_cast<int32_t>(en_bo.current_stats_) > 85 &&
						     en_bo.total_count() * 2 < site.bo->total_count()) ||
						    (static_cast<int32_t>(en_bo.current_stats_) > 50 &&
						     en_bo.total_count() * 4 < site.bo->total_count())) {

							enbld = enhancement;
							bestbld = &en_bo;
						}
					}
				}
			}

			// Enhance if enhanced building is useful
			// additional: we dont want to lose the old building
			if (enbld != INVALID_INDEX) {
				game().send_player_enhance_building(*site.site, enbld);
				bestbld->construction_decision_time_ = gametime;

				return true;
			}
		}
	}

	// Lumberjack / Woodcutter handling
	if (site.bo->need_trees_) {

		// Do not destruct the last few lumberjacks
		if (site.bo->cnt_built_ <= site.bo->cnt_target_) {
			return false;
		}

		if (site.site->get_statistics_percent() > 20) {
			return false;
		}

		const uint32_t remaining_trees =
		   map.find_immovables(Area<FCoords>(map.get_fcoords(site.site->get_position()), radius),
		                       nullptr,
		                       FindImmovableAttribute(MapObjectDescr::get_attribute_id("tree")));

		// do not dismantle if there are some trees remaining
		if (remaining_trees > 5) {
			return false;
		}

		if (site.bo->stocklevel_time < game().get_gametime() - 10 * 1000) {
			site.bo->stocklevel_ = get_stocklevel(*site.bo);
			site.bo->stocklevel_time = game().get_gametime();
		}

		// if we need wood badly
		if (remaining_trees > 0 && site.bo->stocklevel_ <= 50) {
			return false;
		}

		// so finally we dismantle the lumberjac
		site.bo->last_dismantle_time_ = game().get_gametime();
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}

		return true;
	}

	// Wells handling
	if (site.bo->mines_water_) {
		if (site.unoccupied_till_ + 6 * 60 * 1000 < gametime &&
		    site.site->get_statistics_percent() == 0) {
			site.bo->last_dismantle_time_ = gametime;
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			if (connected_to_wh) {
				game().send_player_dismantle(*site.site);
			} else {
				game().send_player_bulldoze(*site.site);
			}

			return true;
		}

		// do not consider dismantling if we are under target
		if (site.bo->last_dismantle_time_ + 90 * 1000 > game().get_gametime()) {
			return false;
		}

		// now we test the stocklevel and dismantle the well if we have enough water
		// but first we make sure we do not dismantle a well too soon
		// after dismantling previous one
		if (site.bo->stocklevel_time < game().get_gametime() - 5 * 1000) {
			site.bo->stocklevel_ = get_stocklevel(*site.bo);
			site.bo->stocklevel_time = game().get_gametime();
		}
		if (site.bo->stocklevel_ > 250 + productionsites.size() * 5) {  // dismantle
			site.bo->last_dismantle_time_ = game().get_gametime();
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
	if (site.bo->need_stones_) {

		if (map.find_immovables(
		       Area<FCoords>(map.get_fcoords(site.site->get_position()), 6),
		       nullptr,

		       FindImmovableAttribute(MapObjectDescr::get_attribute_id("granite"))) == 0) {
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

		if (site.unoccupied_till_ + 6 * 60 * 1000 < gametime &&
		    site.site->get_statistics_percent() == 0) {
			// it is possible that there are stones but quary is not able to mine them
			site.bo->last_dismantle_time_ = game().get_gametime();
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
	if (site.bo->inputs_.empty()  // does not consume anything
	    &&
	    site.bo->production_hint_ == -1  // not a renewing building (forester...)
	    &&
	    site.unoccupied_till_ + 10 * 60 * 1000 < gametime  // > 10 minutes old
	    &&
	    site.site->can_start_working()  // building is occupied
	    &&
	    site.bo->space_consumer_ && !site.bo->plants_trees_) {

		// if we have more buildings then target
		if ((site.bo->cnt_built_ - site.bo->unconnected_) > site.bo->cnt_target_) {
			if (site.bo->stocklevel_time < game().get_gametime() - 5 * 1000) {
				site.bo->stocklevel_ = get_stocklevel(*site.bo);
				site.bo->stocklevel_time = game().get_gametime();
			}

			if (site.site->get_statistics_percent() < 30 &&
			    site.bo->stocklevel_ > 100) {  // production stats == 0%
				site.bo->last_dismantle_time_ = game().get_gametime();
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
		if (site.site->get_statistics_percent() <= 10 && site.bo->cnt_built_ > 1) {

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

	// buildings with inputs_, checking if we can a dismantle some due to low performance
	if (!site.bo->inputs_.empty() &&
		(site.bo->cnt_built_ - site.bo->unoccupied_) >= 3 &&
	    site.site->can_start_working() &&
	    !is_productionsite_needed(site.bo->outputs_.size(),
	    							site.site->get_statistics_percent(),
	    							PerfEvaluation::kForDismantle) &&
	    site.bo->current_stats_ < 30 &&              // overall statistics
	    (game().get_gametime() - site.unoccupied_till_) > 10 * 60 * 1000) {

		site.bo->last_dismantle_time_ = game().get_gametime();
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

	if (site.bo->inputs_.empty() && site.bo->production_hint_ < 0 &&
	    site.site->can_start_working() && !site.bo->space_consumer_ &&
	    site.site->get_statistics_percent() < 10 &&
	    ((game().get_gametime() - site.built_time_) > 10 * 60 * 1000)) {

		site.bo->last_dismantle_time_ = game().get_gametime();
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
	if (site.bo->production_hint_ >= 0) {

		// dismantling the rangers hut, but only if we have them above a target
		if (wood_policy_ == WoodPolicy::kDismantleRangers && site.bo->cnt_built_ > site.bo->cnt_target_) {

			site.bo->last_dismantle_time_ = game().get_gametime();
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
		if ((wood_policy_ == WoodPolicy::kStopRangers
			||
			wood_policy_ == WoodPolicy::kDismantleRangers)
			&&
			!site.site->is_stopped()) {

			game().send_player_start_stop_building(*site.site);
			return false;
		}

		const uint32_t trees_in_vicinity =
		   map.find_immovables(Area<FCoords>(map.get_fcoords(site.site->get_position()), 5),
		                       nullptr,
		                       FindImmovableAttribute(MapObjectDescr::get_attribute_id("tree")));

		// stop ranger if enough trees around regardless of policy
		if (trees_in_vicinity > 25) {
			if (!site.site->is_stopped()) {
				game().send_player_start_stop_building(*site.site);
			}
		// if not enough trees nearby, we can start them if required
		} else if ((wood_policy_ == WoodPolicy::kStartRangers ||
		 	wood_policy_ == WoodPolicy::kBuildRangers)
		 	&&
		 	site.site->is_stopped()) {
				game().send_player_start_stop_building(*site.site);
		}
	}

	return false;
}

// This function scans current situation with shipyards, ports, ships, ongoing expeditions
// and makes two decisions:
// - build a ship
// - start preparation for expedition
bool DefaultAI::marine_main_decisions(uint32_t const gametime) {
	if (gametime < taskDue[ScheduleTasks::KMarineDecisions]) {
		return false;
	}

	if (!seafaring_economy) {
		taskDue[ScheduleTasks::KMarineDecisions] = std::numeric_limits<uint32_t>::max();
		return false;
	}

	taskDue[ScheduleTasks::KMarineDecisions] = gametime + kMarineDecisionInterval;

	// getting some base statistics
	player_ = game().get_player(player_number());
	uint16_t ports_count = 0;
	uint16_t shipyards_count = 0;
	uint16_t working_shipyards_count = 0;
	uint16_t expeditions_in_prep = 0;
	uint16_t expeditions_in_progress = 0;
	bool idle_shipyard_stocked = false;

	// goes over all warehouses (these includes ports)
	for (const WarehouseSiteObserver& wh_obs : warehousesites) {
		if (wh_obs.bo->is_port_) {
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
		if (ps_obs.bo->is_shipyard_) {
			shipyards_count += 1;
			if (!ps_obs.site->is_stopped()) {
				working_shipyards_count += 1;
			}
			// counting stocks
			uint8_t stocked_wares = 0;
			std::vector<WaresQueue*> const warequeues = ps_obs.site->warequeues();
			size_t const nr_warequeues = warequeues.size();
			for (size_t i = 0; i < nr_warequeues; ++i) {
				stocked_wares += warequeues[i]->get_filled();
			}
			if (stocked_wares == 16 && ps_obs.site->is_stopped()) {
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

	enum class FleetStatus : uint8_t {kNeedShip = 0, kEnoughShips = 1, kDoNothing = 2};

	// now we must compare ports vs ships to decide if new ship is needed or new expedition can start
	FleetStatus enough_ships = FleetStatus::kDoNothing;
	if (static_cast<float>(allships.size()) >= ports_count) {
		enough_ships = FleetStatus::kEnoughShips;
	} else if (static_cast<float>(allships.size()) < ports_count) {
		enough_ships = FleetStatus::kNeedShip;
	}

	// building a ship? if yes, find a shipyard and order it to build a ship
	if (shipyards_count > 0 && enough_ships == FleetStatus::kNeedShip && idle_shipyard_stocked &&
	    ports_count > 0) {

		for (const ProductionSiteObserver& ps_obs : productionsites) {
			if (ps_obs.bo->is_shipyard_ && ps_obs.site->can_start_working() &&
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
	if (ports_count > 0 && enough_ships == FleetStatus::kEnoughShips && expeditions_in_prep == 0 &&
	    expeditions_in_progress == 0) {
		// we need to find a port
		for (const WarehouseSiteObserver& wh_obs : warehousesites) {

			if (wh_obs.bo->is_port_) {
				game().send_player_start_or_cancel_expedition(*wh_obs.site);
				return true;
			}
		}
	}
	return true;
}

// This identifies ships that are waiting for command
bool DefaultAI::check_ships(uint32_t const gametime) {
	if (gametime < taskDue[ScheduleTasks::kCheckShips]) {
		return false;
	}

	if (!seafaring_economy) {
		taskDue[ScheduleTasks::kCheckShips] = std::numeric_limits<int32_t>::max();
		return false;
	}

	bool action_taken = false;

	if (!allships.empty()) {
		// iterating over ships and executing what is needed
		for (std::list<ShipObserver>::iterator i = allships.begin(); i != allships.end(); ++i) {

			// only two states need an attention
			if ((i->ship->get_ship_state() == Widelands::Ship::EXP_WAITING ||
			     i->ship->get_ship_state() == Widelands::Ship::EXP_FOUNDPORTSPACE) &&
			    !i->waiting_for_command_) {
				if (gametime - i->last_command_time > 180 * 1000) {
					i->waiting_for_command_ = true;
					log("  %1d: last command for ship at %3dx%3d was %3d seconds ago, something wrong "
					    "here?...\n",
					    player_number(),
					    i->ship->get_position().x,
					    i->ship->get_position().y,
					    (gametime - i->last_command_time) / 1000);
				}
			}
			// if ships is waiting for command
			if (i->waiting_for_command_) {
				expedition_management(*i);
				action_taken = true;
			}
		}
	}

	// processing marineTaskQueue_
	while (!marineTaskQueue_.empty()) {
		if (marineTaskQueue_.back() == kStopShipyard) {
			// iterate over all production sites searching for shipyard
			for (std::list<ProductionSiteObserver>::iterator site = productionsites.begin();
			     site != productionsites.end();
			     ++site) {
				if (site->bo->is_shipyard_) {
					if (!site->site->is_stopped()) {
						game().send_player_start_stop_building(*site->site);
					}
				}
			}
		}

		if (marineTaskQueue_.back() == kReprioritize) {
			for (std::list<ProductionSiteObserver>::iterator site = productionsites.begin();
			     site != productionsites.end();
			     ++site) {
				if (site->bo->is_shipyard_) {
					for (uint32_t k = 0; k < site->bo->inputs_.size(); ++k) {
						game().send_player_set_ware_priority(
						   *site->site, wwWARE, site->bo->inputs_.at(k), HIGH_PRIORITY);
					}
				}
			}
		}

		marineTaskQueue_.pop_back();
	}

	if (action_taken) {
		taskDue[ScheduleTasks::kCheckShips] = gametime + kShipCheckInterval;
	} else {
		taskDue[ScheduleTasks::kCheckShips] = gametime + 3 * kShipCheckInterval;
	}

	return true;
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
	if (site.built_time_ + 6 * 60 * 1000 < gametime && !site.site->can_start_working()) {
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		return true;
	}

	// doing nothing when failed count is too low
	if (site.no_resources_count < 4) {
		return false;
	}

	// dismantling when the failed count is too high
	if (site.no_resources_count > 12) {
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		if (connected_to_wh) {
			game().send_player_dismantle(*site.site);
		} else {
			game().send_player_bulldoze(*site.site);
		}
		site.bo->construction_decision_time_ = gametime;
		return true;
	}

	// is output needed (compare stocked materials vs target values)
	check_building_necessity(*site.bo);

	// if we have enough of mined materials on stock - do not upgrade (yet)
	if (site.bo->output_needed_ == ExtendedBool::kFalse) {
		return false;
	}

	// Check whether building is enhanceable. If yes consider an upgrade.
	const BuildingIndex enhancement = site.site->descr().enhancement();

	// if no enhancement is possible
	if (enhancement == INVALID_INDEX) {
		// will be destroyed when no_resource_count will overflow
		return false;
	}

	if (!connected_to_wh) {
		// no enhancement possible
		return false;
	}

	bool changed = false;
	if (player_->is_building_type_allowed(enhancement)) {
		// first exclude possibility there are enhancements in construction or unoccupied_
		const BuildingDescr& bld = *tribe_->get_building_descr(enhancement);
		BuildingObserver& en_bo = get_building_observer(bld.name().c_str());

		// if it is too soon for enhancement and making sure there are no unoccupied mines
		if (gametime - en_bo.construction_decision_time_ >= kBuildingMinInterval &&
		    en_bo.unoccupied_ + en_bo.cnt_under_construction_ == 0) {

			// now verify that there are enough workers
			if (site.site->has_workers(enhancement, game())) {  // enhancing
				game().send_player_enhance_building(*site.site, enhancement);
				en_bo.construction_decision_time_ = gametime;
				changed = true;
			}
		}
	}

	return changed;
}

// this count ware as hints
uint32_t DefaultAI::get_stocklevel_by_hint(size_t hintoutput) {
	uint32_t count = 0;
	WareIndex wt(hintoutput);
	for (EconomyObserver* observer : economies) {
		// Don't check if the economy has no warehouse.
		if (observer->economy.warehouses().empty()) {
			continue;
		}

		count += observer->economy.stock_ware(wt);
	}

	return count;
}

// goes over all outputs of a building and compare stocked material with
// target values. The result is yes/no and some scores
void DefaultAI::check_building_necessity(BuildingObserver& bo) {
	// iterate over outputs of building, counts warehoused stock
	// and deciding if enough
	bo.max_preciousness = 0;
	bo.max_needed_preciousness_ = 0;

	for (uint32_t m = 0; m < bo.outputs_.size(); ++m) {
		WareIndex wt(static_cast<size_t>(bo.outputs_.at(m)));

		uint16_t target = tribe_->get_ware_descr(wt)->default_target_quantity() / 3;
		// at least  1
		target = std::max<uint16_t>(target, 1);

		uint16_t preciousness = wares.at(bo.outputs_.at(m)).preciousness_;

		if (get_warehoused_stock(wt) < target) {
			if (bo.max_needed_preciousness_ < preciousness) {
				bo.max_needed_preciousness_ = preciousness;
			}
		}

		if (bo.max_preciousness < preciousness) {
			bo.max_preciousness = preciousness;
		}
	}

	// here we decide if the building is needed
	if (bo.max_needed_preciousness_ > 0) {
		bo.output_needed_ = ExtendedBool::kTrue;
	} else {
		bo.output_needed_ = ExtendedBool::kFalse;
	}
}

// counts produced output on stock
// if multiple outputs, it returns lowest value
uint32_t DefaultAI::get_stocklevel(BuildingObserver& bo) {
	uint32_t count = std::numeric_limits<uint32_t>::max();

	if (!bo.outputs_.empty()) {
		for (EconomyObserver* observer : economies) {
			// Don't check if the economy has no warehouse.
			if (observer->economy.warehouses().empty()) {
				continue;
			}

			for (uint32_t m = 0; m < bo.outputs_.size(); ++m) {
				WareIndex wt(static_cast<size_t>(bo.outputs_.at(m)));
				if (count > observer->economy.stock_ware(wt)) {
					count = observer->economy.stock_ware(wt);
				}
			}
		}
	}

	return count;
}

// counts produced output on stock
// if multiple outputs, it returns lowest value
uint32_t DefaultAI::get_stocklevel(WareIndex wt) {
	uint32_t count = 0;

	for (EconomyObserver* observer : economies) {
		// Don't check if the economy has no warehouse.
		if (observer->economy.warehouses().empty()) {
			continue;
		}
		count += observer->economy.stock_ware(wt);
	}

	return count;
}

// counts produced output in warehouses (only)
// perhaps it will be able to replace get_stocklevel
uint32_t DefaultAI::get_warehoused_stock(WareIndex wt) {
	uint32_t count = 0;

	for (std::list<WarehouseSiteObserver>::iterator i = warehousesites.begin();
	     i != warehousesites.end();
	     ++i) {
		count += i->site->get_wares().stock(wt);
	}

	return count;
}

// this just counts free positions in military and training sites
void DefaultAI::count_military_vacant_positions() {
	// counting vacant positions
	vacant_mil_positions_ = 0;
	for (TrainingSiteObserver tso : trainingsites) {
		vacant_mil_positions_ += 10 * (tso.site->soldier_capacity() - tso.site->stationed_soldiers().size());
	}
	for (MilitarySiteObserver mso : militarysites) {
		vacant_mil_positions_ += mso.site->soldier_capacity() - mso.site->stationed_soldiers().size();
	}
}

// this function only check with trainingsites
// manipulates input queues and soldier capacity
bool DefaultAI::check_trainingsites(uint32_t gametime) {
	if (taskDue[ScheduleTasks::kCheckTrainingsites] > gametime) {
		return false;
	}
	if (!trainingsites.empty()) {
		taskDue[ScheduleTasks::kCheckTrainingsites] = gametime + kTrainingSitesCheckInterval;
	} else {
		taskDue[ScheduleTasks::kCheckTrainingsites] = gametime + 2 * kTrainingSitesCheckInterval;
		return false;
	}

	trainingsites.push_back(trainingsites.front());
	trainingsites.pop_front();

	TrainingSite* ts = trainingsites.front().site;
	TrainingSiteObserver& tso = trainingsites.front();

	const BuildingIndex enhancement = ts->descr().enhancement();

	if (enhancement != INVALID_INDEX && ts_without_trainers_ == 0 && mines_.size() > 3 &&
	    (ts_basic_const_count_ + ts_advanced_const_count_) == 0 && ts_advanced_count_ > 0) {

		if (player_->is_building_type_allowed(enhancement)) {
			game().send_player_enhance_building(*tso.site, enhancement);
		}
	}

	// changing capacity to 0 - this will happen only once.....
	if (tso.site->soldier_capacity() > 1) {
		game().send_player_change_soldier_capacity(*ts, - tso.site->soldier_capacity());
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
			continue;}

		// now modifying max_fill of armors and weapons
		for (std::string pattern : armors_and_weapons) {

			if (tribe_->get_ware_descr(warequeues1[i]->get_ware())->name().find(pattern) != std::string::npos) {
				if (warequeues1[i]->get_max_fill() > 1) {
					game().send_player_set_ware_max_fill(*ts, warequeues1[i]->get_ware(), 1);
					continue;
				}
			}
		}
	}

	// changing priority if basic
	if (tso.bo->trainingsite_type_ == TrainingSiteType::kBasic) {
		for (uint32_t k = 0; k < tso.bo->inputs_.size(); ++k) {
			game().send_player_set_ware_priority(
			   *ts, wwWARE, tso.bo->inputs_.at(k), HIGH_PRIORITY);
		}
	}

	// if soldier capacity is set to 0, we need to find out if the site is
	// supplied enough to incrase the capacity to 1
	if (tso.site->soldier_capacity() == 0) {

		// First subsitute wares
		int32_t filled = 0;
		bool supplied_enough = true;
		std::vector<WaresQueue*> const warequeues2 = tso.site->warequeues();
		nr_warequeues = warequeues2.size();
		for (size_t i = 0; i < nr_warequeues; ++i) {
			if (tso.bo->substitute_inputs_.count(warequeues2[i]->get_ware()) > 0) {
				filled += warequeues2[i]->get_filled();
			}
		}
		if (filled < 5) {
			supplied_enough = false;
		}

		// checking non subsitutes
		for (size_t i = 0; i < nr_warequeues; ++i) {
			if (tso.bo->substitute_inputs_.count(warequeues2[i]->get_ware()) == 0) {
				const uint32_t required_amount
				 =
				 (warequeues2[i]->get_max_fill() < 5) ? warequeues2[i]->get_max_fill() : 5;
				if (warequeues2[i]->get_filled() < required_amount) {
					supplied_enough = false;
				}
			}
		}

		if (supplied_enough) {
			game().send_player_change_soldier_capacity(*ts, 1);
		}
	}

	ts_without_trainers_ = 0;  // zeroing
	for (std::list<TrainingSiteObserver>::iterator site = trainingsites.begin();
	     site != trainingsites.end();
	     ++site) {

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
	if (taskDue[ScheduleTasks::kCheckMilitarysites] > gametime) {
		return false;
	}

	// just to be sure the value is reset
	taskDue[ScheduleTasks::kCheckMilitarysites] = gametime + 4 * 1000;  // 4 seconds is really fine
	// even if there are no finished & attended military sites, probably there are ones just in
	// construction
	unstationed_milit_buildings_ = 0;

	for (std::list<MilitarySiteObserver>::iterator it = militarysites.begin();
	     it != militarysites.end();
	     ++it) {
		if (it->site->stationed_soldiers().size() == 0) {
			unstationed_milit_buildings_ += 1;
		}
	}

	// Only useable, if defaultAI owns at least one militarysite
	if (militarysites.empty()) {
		return false;
	}

	// Check next militarysite
	bool changed = false;
	Map& map = game().map();
	MilitarySite* ms = militarysites.front().site;
	MilitarySiteObserver& mso = militarysites.front();
	uint32_t const vision = ms->descr().vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	// look if there are any enemies building
	FindNodeEnemiesBuilding find_enemy(player_, game());

	// first we make sure there is no enemy at all
	if (map.find_fields(Area<FCoords>(f, vision + 4), nullptr, find_enemy) == 0) {

		mso.enemies_nearby_ = false;

		// If no enemy in sight - decrease the number of stationed soldiers
		// as long as it is > 1 - BUT take care that there is a warehouse in the
		// same economy where the thrown out soldiers can go to.

		if (ms->economy().warehouses().size()) {
			uint32_t const j = ms->soldier_capacity();

			if (MilitarySite::kPrefersRookies != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersRookies);
			} else if (j > 1) {
				game().send_player_change_soldier_capacity(*ms, (j > 2) ? -2 : -1);
			}
			// if the building is in inner land and other militarysites still
			// hold the miliary influence of the field, consider to destruct the
			// building to free some building space.
			else {
				// treat this field like a buildable and write military info to it.
				BuildableField bf(f);
				update_buildable_field(bf, vision, true);
				const int32_t size_penalty = ms->get_size() - 1;
				FindNodeAllyOwned find_ally(player_, game(), player_number());
				const int32_t allyOwnedFields =
				   map.find_fields(Area<FCoords>(f, vision), nullptr, find_ally);

				int16_t score = 0;
				score += (bf.area_military_capacity_ > 6);
				score += (bf.area_military_capacity_ > 22);
				score += (bf.area_military_presence_ > 4);
				score += (bf.military_loneliness_ < 180);
				score += (bf.military_stationed_ > 2);
				score -= size_penalty;
				score += ((bf.unowned_land_nearby_ + allyOwnedFields) < 10);

				if (score >= 4) {
					if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
						flags_to_be_removed.push_back(ms->base_flag().get_position());
						game().send_player_dismantle(*ms);
						military_last_dismantle_ = game().get_gametime();
					} else {
						game().send_player_bulldoze(*ms);
						military_last_dismantle_ = game().get_gametime();
					}
				}
			}
		}
	} else {

		int32_t unused1 = 0;
		uint16_t unused2 = 0;

		mso.enemies_nearby_ = false;

		// yes enemy is nearby, but still we must distinguish whether
		// he is accessible (over the land)
		if (other_player_accessible(
		       vision + 4, &unused1, &unused2, ms->get_position(), WalkSearch::kEnemy)) {

			uint32_t const total_capacity = ms->max_soldier_capacity();
			uint32_t const target_capacity = ms->soldier_capacity();

			game().send_player_change_soldier_capacity(*ms, total_capacity - target_capacity);
			changed = true;

			// and also set preference to Heroes
			if (MilitarySite::kPrefersHeroes != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersHeroes);
				changed = true;
			}

			mso.enemies_nearby_ = true;
			enemy_last_seen_ = gametime;
		} else {  // otherwise decrease soldiers
			uint32_t const j = ms->soldier_capacity();

			if (MilitarySite::kPrefersRookies != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersRookies);
			} else if (j > 1) {
				game().send_player_change_soldier_capacity(*ms, (j > 2) ? -2 : -1);
			}
		}
	}

	// reorder:;
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();
	taskDue[ScheduleTasks::kCheckMilitarysites] = gametime + 5 * 1000;  // 10 seconds is really fine
	return changed;
}

/**
 * This function takes care about the unowned and opposing territory and
 * recalculates the priority for non-military buildings
 * The goal is to minimize losses when teritory is lost
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

	// in unowned teritory, decreasing to 2/3
	if (bf.unowned_land_nearby_ > 15) {
		prio *= 2;
		prio /= 3;
	}

	// to preserve positive score
	if (prio == 0) {
		prio = 1;
	}

	// Further decrease the score if enemy nearby
	if (bf.enemy_nearby_) {
		prio /= 2;
	}

	return prio;
}

/**
 * calculates how much a productionsite of type \arg bo is needed inside it's
 * economy. \arg prio is initial value for this calculation
 *
 * \returns the calculated priority
 */
int32_t DefaultAI::calculate_need_for_ps(BuildingObserver& bo, int32_t prio) {
	// some randomness to avoid that defaultAI is building always
	// the same (always == another game but same map with
	// defaultAI on same coords)
	prio += time(nullptr) % 3 - 1;

	// check if current economy can supply enough material for
	// production.
	for (uint32_t k = 0; k < bo.inputs_.size(); ++k) {
		prio += 2 * wares.at(bo.inputs_.at(k)).producers_;
		prio -= wares.at(bo.inputs_.at(k)).consumers_;
	}

	if (bo.inputs_.empty()) {
		prio += 4;
	}

	int32_t output_prio = 0;

	for (uint32_t k = 0; k < bo.outputs_.size(); ++k) {
		WareObserver& wo = wares.at(bo.outputs_.at(k));

		if (wo.consumers_ > 0) {
			output_prio += wo.preciousness_;
			output_prio += wo.consumers_ * 2;
			output_prio -= wo.producers_ * 2;

			if (bo.total_count() == 0) {
				output_prio += 10;  // add a big bonus
			}
		}
	}

	if (bo.outputs_.size() > 1) {
		output_prio =
		   static_cast<int32_t>(ceil(output_prio / sqrt(static_cast<double>(bo.outputs_.size()))));
	}

	prio += 2 * output_prio;

	// If building consumes some wares, multiply with current statistics of all
	// other buildings of this type to avoid constructing buildings where already
	// some are running on low resources.
	// Else at least add a part of the stats t the calculation.
	if (!bo.inputs_.empty()) {
		prio *= bo.current_stats_;
		prio /= 100;
	} else {
		prio = ((prio * bo.current_stats_) / 100) + (prio / 2);
	}

	return prio;
}

void DefaultAI::consider_productionsite_influence(BuildableField& field,
                                                  Coords coords,
                                                  const BuildingObserver& bo) {
	if (bo.space_consumer_ && !bo.plants_trees_ &&
	    game().map().calc_distance(coords, field.coords) < 8) {
		++field.space_consumers_nearby_;
	}

	for (size_t i = 0; i < bo.inputs_.size(); ++i) {
		++field.consumers_nearby_.at(bo.inputs_.at(i));
	}

	for (size_t i = 0; i < bo.outputs_.size(); ++i) {
		++field.producers_nearby_.at(bo.outputs_.at(i));
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

	for (uint32_t i = 0; i < buildings_.size(); ++i) {
		if (!strcmp(buildings_.at(i).name, name)) {
			return buildings_.at(i);
		}
	}

	throw wexception("Help: I (player %d / tribe %s) do not know what to do with a %s",
	                 player_number(),
	                 tribe_->name().c_str(),
	                 name);
}

// this is called whenever we gain ownership of a PlayerImmovable
void DefaultAI::gain_immovable(PlayerImmovable& pi) {
	if (upcast(Building, building, &pi)) {
		gain_building(*building);
	} else if (upcast(Flag const, flag, &pi)) {
		new_flags.push_back(flag);
	} else if (upcast(Road const, road, &pi)) {
		roads.push_front(road);
	}
}

// this is called whenever we gain ownership of a Ship
void DefaultAI::gain_ship(Ship& ship, NewShip type) {

	if (type == NewShip::kBuilt) {
		marineTaskQueue_.push_back(kStopShipyard);
	} else {
		seafaring_economy = true;
	}

	allships.push_back(ShipObserver());
	allships.back().ship = &ship;
	if (game().get_gametime() % 2 == 0) {
		allships.back().island_circ_direction = IslandExploreDirection::kClockwise;
	} else {
		allships.back().island_circ_direction = IslandExploreDirection::kCounterClockwise;
	}
}

// this is called whenever we lose ownership of a PlayerImmovable
void DefaultAI::lose_immovable(const PlayerImmovable& pi) {
	if (upcast(Building const, building, &pi)) {
		lose_building(*building);
	} else if (upcast(Flag const, flag, &pi)) {
		for (EconomyObserver* eco_obs : economies) {
			for (std::list<Flag const*>::iterator flag_iter = eco_obs->flags.begin();
			     flag_iter != eco_obs->flags.end();
			     ++flag_iter) {
				if (*flag_iter == flag) {
					eco_obs->flags.erase(flag_iter);
					return;
				}
			}
		}
		for (std::list<Flag const*>::iterator flag_iter = new_flags.begin();
		     flag_iter != new_flags.end();
		     ++flag_iter) {
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

	// we must identify which mine matches the productionsite a note reffers to
	for (std::list<ProductionSiteObserver>::iterator i = mines_.begin(); i != mines_.end(); ++i)
		if (i->site == &site) {
			i->no_resources_count += 1;
			break;
		}
}

// This is called when soldier left the trainingsite
// the purpose is to set soldier capacity to 0
// (AI will then wait till training site is stocked)
void DefaultAI::soldier_trained(const TrainingSite& site) {

	for (TrainingSiteObserver & trainingsite_obs : trainingsites) {
		if (trainingsite_obs.site == &site) {
			if (trainingsite_obs.site->soldier_capacity() > 0) {
				game().send_player_change_soldier_capacity(*trainingsite_obs.site,
				- trainingsite_obs.site->soldier_capacity());
			}
			return;
		}
	}

	log (" %d: Computer player error - trainingsite not found\n",
	player_number());
}

// walk and search for teritorry controlled by other player
// usually scanning radius is enough but sometimes we must walk to
// verify that an enemy teritory is really accessible by land
bool DefaultAI::other_player_accessible(const uint32_t max_distance,
                                        int32_t* tested_fields,
                                        uint16_t* mineable_fields_count,
                                        const Widelands::Coords starting_spot,
                                        const WalkSearch type) {
	Map& map = game().map();
	std::list<uint32_t> queue;
	std::unordered_set<uint32_t> done;
	queue.push_front(coords_hash(starting_spot));
	PlayerNumber const pn = player_->player_number();

	while (!queue.empty()) {
		// if already processed
		if (done.count(queue.front()) > 0) {
			queue.pop_front();
			continue;
		}

		done.insert(queue.front());

		Coords tmp_coords = coords_unhash(queue.front());

		// if beyond range
		if (map.calc_distance(starting_spot, tmp_coords) > max_distance) {
			continue;
		}

		Field* f = map.get_fcoords(tmp_coords).field;

		// not interested if not walkable (starting spot is an exemption.
		if (tmp_coords != starting_spot && !(f->nodecaps() & MOVECAPS_WALK)) {
			continue;
		}

		// sometimes we search for any owned teritory (f.e. when considering
		// a port location), but when testing (starting from) own military building
		// we must ignore own teritory, of course
		if (f->get_owned_by() > 0) {

			// if field is owned by anybody
			if (type == WalkSearch::kAnyPlayer) {
				*tested_fields = done.size();
				return true;
			}

			// if anybody but not me
			if (type == WalkSearch::kOtherPlayers && f->get_owned_by() != pn) {
				*tested_fields = done.size();
				return true;
			}

			// if owned by enemy
			if  (type == WalkSearch::kEnemy && f->get_owned_by() != pn) {
				// for case I am not member of a team
				if (player_->team_number() == 0) {
					*tested_fields = done.size();
					return true;
				// if I am in team, testing if the same team
				} else if (player_->team_number() > 0
				&&
				player_->team_number()
				!=
				game().get_player(f->get_owned_by())->team_number()) {
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
			queue.push_front(coords_hash(neigh_coords2));
		}
	}
	*tested_fields = done.size();

	return false;  // no players found
}

// this scores spot for potential colony
uint8_t DefaultAI::spot_scoring(Widelands::Coords candidate_spot) {

	uint8_t score = 0;
	uint16_t mineable_fields_count = 0;
	int32_t tested_fields = 0;
	const bool other_player = other_player_accessible(colony_scan_area_,
	                                                  &tested_fields,
	                                                  &mineable_fields_count,
	                                                  candidate_spot,
	                                                  WalkSearch::kAnyPlayer);

	// if we run into other player
	// (maybe we should check for enemies, rather?)
	if (other_player) {
		return 0;
	}

	Map& map = game().map();
	// if the island is too small
	if (tested_fields < 50) {
		return 0;
	}

	// if we are here we put score
	score = 1;
	if (mineable_fields_count > 0) {
		score += 1;
	}

	// here we check for surface stones + trees
	std::vector<ImmovableFound> immovables;
	// Search in a radius of range
	map.find_immovables(Area<FCoords>(map.get_fcoords(candidate_spot), 10), &immovables);

	int32_t const stone_attr = MapObjectDescr::get_attribute_id("granite");
	uint16_t stones = 0;
	int32_t const tree_attr = MapObjectDescr::get_attribute_id("tree");
	uint16_t trees = 0;

	for (uint32_t j = 0; j < immovables.size(); ++j) {
		if (immovables.at(j).object->has_attribute(stone_attr)) {
			++stones;
		}
		if (immovables.at(j).object->has_attribute(tree_attr)) {
			++trees;
		}
	}
	if (stones > 1) {
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

	// first we put current spot into visited_spots_
	bool first_time_here = false;
	if (so.visited_spots_.count(coords_hash(so.ship->get_position())) == 0) {
		first_time_here = true;
		so.visited_spots_.insert(coords_hash(so.ship->get_position()));
	}

	// If we have portspace following options are avaiable:
	// 1. Build a port there
	if (so.ship->exp_port_spaces()->size() > 0) {  // making sure we have possible portspaces

		// we score the place
		const uint8_t spot_score = spot_scoring(so.ship->exp_port_spaces()->front());

		if ((gametime / 10) % 8 < spot_score) {  // we build a port here
			// const Coords last_portspace = so.ship->exp_port_spaces()->front();
			// remote_ports_coords.insert(coords_hash(last_portspace));
			game().send_player_ship_construct_port(*so.ship, so.ship->exp_port_spaces()->front());
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;
			// blocking the area for some time to save AI from idle attempts to built there
			// buildings
			// TODO(TiborB): how long it takes to build a port?
			// I used 5 minutes
			MapRegion<Area<FCoords>> mr(
			   game().map(), Area<FCoords>(map.get_fcoords(so.ship->exp_port_spaces()->front()), 8));
			do {
				BlockedField blocked2(
				   map.get_fcoords(*(mr.location().field)), gametime + 5 * 60 * 1000);
				blocked_fields.push_back(blocked2);
			} while (mr.advance(map));

			return;
		}

		// decreasing colony_scan_area_
		if (colony_scan_area_ > 15 && gametime % 10 == 0) {
			colony_scan_area_ -= 1;
		}
	}

	// if we are here, port was not ordered above
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
		// get swimable directions
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
			game().send_player_ship_scouting_direction(*so.ship, static_cast<WalkingDir>(final_direction));
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;
		}
	}
	return;
}

// this is called whenever we gain a new building
void DefaultAI::gain_building(Building& b) {

	BuildingObserver& bo = get_building_observer(b.descr().name().c_str());

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver& target_bo =
		   get_building_observer(dynamic_cast<const ConstructionSite&>(b).building().name().c_str());
		++target_bo.cnt_under_construction_;
		++num_constructionsites_;
		if (target_bo.type == BuildingObserver::PRODUCTIONSITE) {
			++num_prod_constructionsites;
		}
		if (target_bo.type == BuildingObserver::MILITARYSITE) {
			++num_milit_constructionsites;
		}
		if (target_bo.type == BuildingObserver::MINE) {
			mines_per_type[target_bo.mines_].in_construction += 1;
		}
		if (target_bo.type == BuildingObserver::TRAININGSITE) {
			if (target_bo.trainingsite_type_ == TrainingSiteType::kBasic) {
				ts_basic_const_count_ += 1;
			}
			if (target_bo.trainingsite_type_ == TrainingSiteType::kAdvanced) {
				ts_advanced_const_count_ += 1;
			}
		}

		// Let defaultAI try to directly connect the constructionsite
		taskDue[ScheduleTasks::kRoadCheck] = game().get_gametime();
	} else {
		++bo.cnt_built_;

		if (bo.type == BuildingObserver::PRODUCTIONSITE) {
			productionsites.push_back(ProductionSiteObserver());
			productionsites.back().site = &dynamic_cast<ProductionSite&>(b);
			productionsites.back().bo = &bo;
			productionsites.back().built_time_ = game().get_gametime();
			productionsites.back().unoccupied_till_ = game().get_gametime();
			productionsites.back().stats_zero_ = 0;
			productionsites.back().no_resources_count = 0;
			if (bo.is_shipyard_) {
				marineTaskQueue_.push_back(kStopShipyard);
				marineTaskQueue_.push_back(kReprioritize);
			}

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				++wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				++wares.at(bo.inputs_.at(i)).consumers_;
		} else if (bo.type == BuildingObserver::MINE) {
			mines_.push_back(ProductionSiteObserver());
			mines_.back().site = &dynamic_cast<ProductionSite&>(b);
			mines_.back().bo = &bo;
			mines_.back().built_time_ = game().get_gametime();

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				++wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				++wares.at(bo.inputs_.at(i)).consumers_;

			mines_per_type[bo.mines_].finished += 1;

		} else if (bo.type == BuildingObserver::MILITARYSITE) {
			militarysites.push_back(MilitarySiteObserver());
			militarysites.back().site = &dynamic_cast<MilitarySite&>(b);
			militarysites.back().bo = &bo;
			militarysites.back().checks = bo.desc->get_size();
			militarysites.back().enemies_nearby_ = true;

		} else if (bo.type == BuildingObserver::TRAININGSITE) {
			ts_without_trainers_ += 1;
			trainingsites.push_back(TrainingSiteObserver());
			trainingsites.back().site = &dynamic_cast<TrainingSite&>(b);
			trainingsites.back().bo = &bo;
			if (bo.trainingsite_type_ == TrainingSiteType::kBasic) {
				ts_basic_count_ += 1;
			}
			if (bo.trainingsite_type_ == TrainingSiteType::kAdvanced) {
				ts_advanced_count_ += 1;
			}

		} else if (bo.type == BuildingObserver::WAREHOUSE) {
			++numof_warehouses_;
			warehousesites.push_back(WarehouseSiteObserver());
			warehousesites.back().site = &dynamic_cast<Warehouse&>(b);
			warehousesites.back().bo = &bo;
			if (bo.is_port_) {
				++num_ports;
				seafaring_economy = true;
				// unblock nearby fields, might be used for other buildings...
				Map& map = game().map();
				MapRegion<Area<FCoords>> mr(
				   map, Area<FCoords>(map.get_fcoords(warehousesites.back().site->get_position()), 4));
				do {
					const int32_t hash = coords_hash(map.get_fcoords(*(mr.location().field)));
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

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver& target_bo =
		   get_building_observer(dynamic_cast<const ConstructionSite&>(b).building().name().c_str());
		--target_bo.cnt_under_construction_;
		--num_constructionsites_;
		if (target_bo.type == BuildingObserver::PRODUCTIONSITE) {
			--num_prod_constructionsites;
		}
		if (target_bo.type == BuildingObserver::MILITARYSITE) {
			--num_milit_constructionsites;
		}
		if (target_bo.type == BuildingObserver::MINE) {
			mines_per_type[target_bo.mines_].in_construction -= 1;
		}
		if (target_bo.type == BuildingObserver::TRAININGSITE) {
			if (target_bo.trainingsite_type_ == TrainingSiteType::kBasic) {
				ts_basic_const_count_ -= 1;
			}
			if (target_bo.trainingsite_type_ == TrainingSiteType::kAdvanced) {
				ts_advanced_const_count_ -= 1;
			}
		}

	} else {
		--bo.cnt_built_;

		if (bo.type == BuildingObserver::PRODUCTIONSITE) {

			for (std::list<ProductionSiteObserver>::iterator i = productionsites.begin();
			     i != productionsites.end();
			     ++i)
				if (i->site == &b) {
					productionsites.erase(i);
					break;
				}

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i) {
				--wares.at(bo.outputs_.at(i)).producers_;
			}

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i) {
				--wares.at(bo.inputs_.at(i)).consumers_;
			}
		} else if (bo.type == BuildingObserver::MINE) {
			for (std::list<ProductionSiteObserver>::iterator i = mines_.begin(); i != mines_.end();
			     ++i) {
				if (i->site == &b) {
					mines_.erase(i);
					break;
				}
			}

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i) {
				--wares.at(bo.outputs_.at(i)).producers_;
			}

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i) {
				--wares.at(bo.inputs_.at(i)).consumers_;
			}

			mines_per_type[bo.mines_].finished -= 1;

		} else if (bo.type == BuildingObserver::MILITARYSITE) {

			for (std::list<MilitarySiteObserver>::iterator i = militarysites.begin();
			     i != militarysites.end();
			     ++i) {
				if (i->site == &b) {
					militarysites.erase(i);
					break;
				}
			}
		} else if (bo.type == BuildingObserver::TRAININGSITE) {

			for (std::list<TrainingSiteObserver>::iterator i = trainingsites.begin();
			     i != trainingsites.end();
			     ++i) {
				if (i->site == &b) {
					trainingsites.erase(i);
					if (bo.trainingsite_type_ == TrainingSiteType::kBasic) {
						ts_basic_count_ -= 1;
					}
					if (bo.trainingsite_type_ == TrainingSiteType::kAdvanced) {
						ts_advanced_count_ -= 1;
					}
					break;
				}
			}
		} else if (bo.type == BuildingObserver::WAREHOUSE) {
			assert(numof_warehouses_ > 0);
			--numof_warehouses_;
			if (bo.is_port_) {
				--num_ports;
			}

			for (std::list<WarehouseSiteObserver>::iterator i = warehousesites.begin();
			     i != warehousesites.end();
			     ++i) {
				if (i->site == &b) {
					warehousesites.erase(i);
					break;
				}
			}
		}
	}
}

// Checks that supply line exists for given building.
// Recursively verify that all inputs_ have a producer.
// TODO(unknown): this function leads to periodic freezes of ~1 second on big games on my system.
// TODO(unknown): It needs profiling and optimization.
// NOTE: This is not needed anymore and it seems it is not missed neither
bool DefaultAI::check_supply(const BuildingObserver& bo) {
	size_t supplied = 0;
	for (const int16_t& temp_inputs : bo.inputs_) {
		for (const BuildingObserver& temp_building : buildings_) {
			if (temp_building.cnt_built_ &&
			    std::find(temp_building.outputs_.begin(), temp_building.outputs_.end(), temp_inputs) !=
			       temp_building.outputs_.end() &&
			    check_supply(temp_building)) {
				++supplied;
				break;
			}
		}
	}

	return supplied == bo.inputs_.size();
}

// This calculates strength of vector of soldiers, f.e. soldiers in a building or
// ones ready to attack
int32_t DefaultAI::calculate_strength(const std::vector<Widelands::Soldier*> soldiers) {

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

	float hp = 0;
	float al = 0;
	float dl = 0;
	float el = 0;
	float final = 0;

	for (Soldier * soldier : soldiers) {
		switch (tribe) {
			case (Tribes::kAtlanteans):
				hp = 135 + 40 * soldier->get_hp_level();
				al =  14 +  8 * soldier->get_attack_level();
				dl = static_cast<float>(94 -  8 * soldier->get_defense_level()) / 100;
				el = static_cast<float>(70 - 17 * soldier->get_evade_level()) / 100;
				break;
			case (Tribes::kBarbarians):
				hp += 130 + 28 * soldier->get_hp_level();
				al +=  14 +  7 * soldier->get_attack_level();
				dl += static_cast<float>(97 -  8 * soldier->get_defense_level()) / 100;
				el += static_cast<float>(75 - 15 * soldier->get_evade_level()) / 100;
				break;
			case (Tribes::kEmpire):
				hp += 130 + 21 * soldier->get_hp_level();
				al +=  14 +  8 * soldier->get_attack_level();
				dl += static_cast<float>(95 -  8 * soldier->get_defense_level()) / 100;
				el += static_cast<float>(70 - 16 * soldier->get_evade_level()) / 100;
				break;
			default:
				assert (false);
		}

		final += (al * hp) / (dl * el);
	}

	// 2500 is aproximate strength of one unpromoted soldier
	return static_cast<int32_t>(final / 2500);
}

bool DefaultAI::check_enemy_sites(uint32_t const gametime) {

	Map& map = game().map();

	// define which players are attackable
	std::vector<bool> player_attackable;
	PlayerNumber const nr_players = map.get_nrplayers();
	player_attackable.resize(nr_players);
	uint32_t plr_in_game = 0;
	uint16_t const pn = player_number();

	iterate_players_existing_novar(p, nr_players, game())++ plr_in_game;

	// receiving games statistics and parsing it (reading latest entry)
	const Game::GeneralStatsVector& genstats = game().get_general_statistics();

	// summing team power, creating team_power std::map of team_number:strength
	std::map<TeamNumber, uint32_t> team_power;
	for (uint8_t j = 1; j <= plr_in_game; ++j) {
		TeamNumber const tm = game().get_player(j)->team_number();
		if (tm == 0) {
			continue;
		}
		// for case this is new team
		if (team_power.count(tm) == 0) {
			// adding this team (number) to vector
			team_power[tm] = 0;
		}
		try {
			team_power[tm] += genstats.at(j - 1).miltary_strength.back();
		} catch (const std::out_of_range&) {
			log("ComputerPlayer(%d): genstats entry missing - size :%d\n",
			    player_number(),
			    static_cast<unsigned int>(genstats.size()));
		}
	}

	// defining treshold ratio of own_strenght/enemy's_strength
	uint32_t treshold_ratio = 100;
	if (type_ == AGGRESSIVE) {
		treshold_ratio = 80;
	}
	if (type_ == DEFENSIVE) {
		treshold_ratio = 120;
	}

	uint32_t my_power = 0;
	try {
		my_power = genstats.at(pn - 1).miltary_strength.back();
	} catch (const std::out_of_range&) {
		log("ComputerPlayer(%d): genstats entry missing - size :%d\n",
		    player_number(),
		    static_cast<unsigned int>(genstats.size()));
	}
	// adding power of team (minus my power) divided by 2
	// (if I am a part of a team of course)
	if (game().get_player(pn)->team_number() > 0) {
		my_power += (team_power[game().get_player(pn)->team_number()] - my_power) / 2;
	}

	// now we test all players to identify 'attackable' ones
	for (uint8_t j = 1; j <= plr_in_game; ++j) {
		// if it's me
		if (pn == j) {
			player_attackable[j - 1] = false;
			continue;
		}
		// if we are the same team
		if (game().get_player(pn)->team_number() > 0 &&
		game().get_player(pn)->team_number() == game().get_player(j)->team_number()) {
			player_attackable[j - 1] = false;
			continue;
		}

		// now we compare strength
		try {
			// strength of the other player
			uint32_t players_power = 0;
			if (!genstats.at(j - 1).miltary_strength.empty()) {
				players_power += genstats.at(j - 1).miltary_strength.back();
			}
			// +power of team (if member of a team)
			if (game().get_player(j)->team_number() > 0) {
				players_power += (team_power[game().get_player(j)->team_number()] - players_power) / 2;
			}

			if (players_power == 0) {
				player_attackable.at(j - 1) = true;
			} else if (my_power * 100 / players_power > treshold_ratio) {
				player_attackable.at(j - 1) = true;
			} else {
				player_attackable.at(j - 1) = false;
			}
		} catch (const std::out_of_range&) {
			log("ComputerPlayer(%d): genstats entry missing - size :%d\n",
			    player_number(),
			    static_cast<unsigned int>(genstats.size()));
			player_attackable.at(j - 1) = false;
		}
	}

	// first we scan vicitnity of couple of militarysites to get new enemy sites
	// militarysites rotate
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
		map.find_immovables(Area<FCoords>(f, (vision + 3 < 13) ? 13 : vision + 3),
		                    &immovables,
		                    FindImmovableAttackable());

		for (uint32_t j = 0; j < immovables.size(); ++j) {
			if (upcast(MilitarySite const, bld, immovables.at(j).object)) {
				if (player_->is_hostile(bld->owner())) {
					if (enemy_sites.count(coords_hash(bld->get_position())) == 0) {
						enemy_sites[coords_hash(bld->get_position())] = EnemySiteObserver();
					}
				}
			}
			if (upcast(Warehouse const, wh, immovables.at(j).object)) {
				if (player_->is_hostile(wh->owner())) {
					if (enemy_sites.count(coords_hash(wh->get_position())) == 0) {
						enemy_sites[coords_hash(wh->get_position())] = EnemySiteObserver();
					}
				}
			}
		}
	}

	// now we update some of them
	uint32_t best_target = std::numeric_limits<uint32_t>::max();
	uint8_t best_score = 0;
	uint32_t count = 0;

	for (std::map<uint32_t, EnemySiteObserver>::iterator site = enemy_sites.begin();
	     site != enemy_sites.end();
	     ++site) {

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
		const bool is_visible = (1 < player_->vision
			 	(Map::get_index(coords_unhash(site->first), map.get_width())));
		uint16_t owner_number = 100;

		// testing if we can attack the building - result is a flag
		// if we dont get a flag, we remove the building from observers list
		FCoords f = map.get_fcoords(coords_unhash(site->first));
		uint32_t site_to_be_removed = std::numeric_limits<uint32_t>::max();
		Flag* flag = nullptr;

		if (upcast(MilitarySite, bld, f.field->get_immovable())) {
			if (player_->is_hostile(bld->owner())) {
				std::vector<Soldier *> defenders;
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

				std::vector<Soldier *> defenders;
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

			site->second.warehouse_ = is_warehouse;

			// getting rid of default
			if (site->second.last_time_attackable == std::numeric_limits<uint32_t>::max()) {
				site->second.last_time_attackable = gametime;
			}

			// can we attack:
			if (is_attackable) {
				std::vector<Soldier *> attackers;
				player_->find_attack_soldiers(*flag, &attackers);
				int32_t strength = calculate_strength(attackers);

				site->second.attack_soldiers_strength = strength;
			} else {
				site->second.attack_soldiers_strength = 0;
			}

			site->second.defenders_strength = defenders_strength;

			if (site->second.attack_soldiers_strength > 0
				&&
				player_attackable[owner_number - 1]) {
				site->second.score = site->second.attack_soldiers_strength - site->second.defenders_strength / 2;

				if (is_warehouse) {
					site->second.score += 2;
				} else {
					site->second.score -= 2;
				}

				// here is some differentiation based on "character" of a player
				if (type_ == NORMAL) {
					site->second.score -= 3;
					site->second.score -= vacant_mil_positions_ / 8;
				} else if (type_ == DEFENSIVE) {
					site->second.score -= 6;
					site->second.score -= vacant_mil_positions_ / 4;
				} else {  //=AGRESSIVE
					site->second.score -= vacant_mil_positions_ / 16;
				}
				if (site->second.mines_nearby == ExtendedBool::kFalse) {
					site->second.score -= 1;
				} else {
					site->second.score += 1;
				}
				// we dont want to attack multiple players at the same time too eagerly
				if (owner_number != last_attacked_player_) {
					site->second.score -= 3;
				}
				// if we dont have mines yet
				if (mines_.size() <= 2) {
					site->second.score -= 8;
				}
				// also we should have at least some training sites
				if ((ts_basic_count_ + ts_advanced_count_) == 0) {
					site->second.score -= 4;
				}
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
			enemy_sites.erase(site_to_be_removed);
			continue;
		}
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

	// attacking
	FCoords f = map.get_fcoords(coords_unhash(best_target));
	// setting no attack counter here
	// this gauranties that it will not be attacked in next 3
	// turns
	enemy_sites[best_target].no_attack_counter = -3;

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
	uint32_t above_ten = (attackers > 10)? attackers - 10 : 0;
	above_ten = (above_ten > 30) ? 30 : above_ten;

	attackers = attackers - (gametime % 3) - ((above_ten > 0) ? gametime % above_ten : 0);

	if (attackers <= 0) {
		return false;
	}

	game().send_player_enemyflagaction(*flag, player_number(), attackers);
	last_attacked_player_ = flag->owner().player_number();

	return true;
}

// This runs once in 15 minutes, and adjust wares targets based on number of
// productionsites and ports
void DefaultAI::review_wares_targets(uint32_t const gametime) {

	player_ = game().get_player(player_number());
	tribe_ = &player_->tribe();

	// to avoid floats real multiplicator is multiplicator/10
	uint16_t multiplicator = 10;
	if ((productionsites.size() + num_ports * 5) > 50) {
		multiplicator = (productionsites.size() + num_ports * 5) / 5;
	}

	for (EconomyObserver* observer : economies) {
		WareIndex nritems = observer->economy.owner().tribe().get_nrwares();
		for (Widelands::WareIndex id = 0; id < nritems; ++id) {
			const uint16_t default_target = tribe_->get_ware_descr(id)->default_target_quantity();

			game().send_player_command(*new Widelands::CmdSetWareTargetQuantity(
			                              gametime,
			                              player_number(),
			                              player_->get_economy_number(&observer->economy),
			                              id,
			                              default_target * multiplicator / 10));
		}
	}
}

// run over dueTasks map and returns task with lower duetime
DefaultAI::ScheduleTasks DefaultAI::get_oldest_task(uint32_t const gametime) {

	uint32_t oldestTaskTime = gametime;            // we are looking for jobs due before now
	ScheduleTasks DueTask = ScheduleTasks::kIdle;  // default
	taskDue[ScheduleTasks::kIdle] = gametime;

	for (std::pair<ScheduleTasks, uint32_t> task : taskDue) {
		if (task.second < oldestTaskTime) {
			oldestTaskTime = task.second;
			DueTask = task.first;
		}
	}
	if ((gametime - oldestTaskTime) > 5000) {
		scheduler_delay_counter_ += 1;
	} else {
		scheduler_delay_counter_ = 0;
	}

	if (scheduler_delay_counter_ > 10) {
		log(" %d: AI: game speed too high, jobs are too late (now %2d seconds)\n",
		player_number(),
		static_cast<int32_t>((gametime - oldestTaskTime) / 1000));
		scheduler_delay_counter_ = 0;
	}

	return DueTask;
}

// This prints some basic statistics during a game to the command line -
// missing materials and counts of different types of buildings.
// The main purpose of this is when a game creator needs to finetune a map
// and needs to know what resourcess are missing for which player and so on.
// By default it is off (see kPrintStats)
// TODO(tiborb ?): - it would be nice to have this activated by a command line switch
void DefaultAI::print_stats(uint32_t const gametime) {

	if (!kPrintStats) {
		taskDue[ScheduleTasks::kPrintStats] = std::numeric_limits<int32_t>::max();
		return;
	}
	taskDue[ScheduleTasks::kPrintStats] = gametime + 30 * 60 * 1000;

	PlayerNumber const pn = player_number();

	// we test following materials
	const std::vector<std::string> materials = {"coal",
	                                            "log",
	                                            "ironore",
	                                            "iron",
	                                            "marble",
	                                            "plank",
	                                            "water",
	                                            "goldore",
	                                            "granite",
	                                            "fish",
	                                            "diamond",
	                                            "stone",
	                                            "corn",
	                                            "wheat",
	                                            "grape",
	                                            "quartz",
	                                            "bread",
	                                            "meat"};
	std::string summary = "";
	for (uint32_t j = 0; j < materials.size(); ++j) {
		WareIndex const index = tribe_->ware_index(materials.at(j));
		if (index == INVALID_INDEX) {
			continue;
		}
		if (get_warehoused_stock(index) > 0) {
			continue;
		}
		summary = summary + materials.at(j) + ", ";
	}

	log(" %1d: Buildings: Pr:%3u, Ml:%3u, Mi:%2u, Wh:%2u, Po:%u. Missing: %s\n",
	    pn,
	    static_cast<uint32_t>(productionsites.size()),
	    static_cast<uint32_t>(militarysites.size()),
	    static_cast<uint32_t>(mines_.size()),
	    static_cast<uint32_t>(warehousesites.size() - num_ports),
	    num_ports,
	    summary.c_str());
}
