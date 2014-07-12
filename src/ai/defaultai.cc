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
#include <queue>
#include <typeinfo>

#include "ai/ai_hints.h"
#include "base/log.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "logic/constructionsite.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/map.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/world/world.h"
#include "profile/profile.h"

constexpr int kFieldUpdateInterval = 1000;
constexpr int kIdleMineUpdateInterval = 22000;
constexpr int kBusyMineUpdateInterval = 2000;
// building of the same building can be started after 25s at earliest
constexpr int kBuildingMinInterval = 25 * 1000;
constexpr bool kMilitaryDebug = false;
constexpr bool kMilDismDebug = false;
constexpr bool kMilitScoreDebug = false;
constexpr bool kQuarryDismDebug = false;
constexpr bool kProductionDebug = false;
constexpr bool kHintDebug = false;
constexpr bool kWinnerDebug = false;
constexpr bool kNewBuildingDebug = false;
constexpr bool kStandbyDebug = false;
constexpr bool kMinesDebug = false;
constexpr bool kUpgradeDebug = false;
constexpr bool kStockDebug = false;
constexpr bool kMinesUpdateDebug = false;
constexpr bool kWoodDebug = false;
constexpr bool kSpaceDebug = false;
constexpr bool kStatDebug = false;
constexpr bool kIdleDismantle = false;
constexpr bool kWellDebug = false;
constexpr int kBaseInfrastructureTime = 20 * 60 * 1000;

using namespace Widelands;

DefaultAI::AggressiveImpl DefaultAI::aggressiveImpl;
DefaultAI::NormalImpl DefaultAI::normalImpl;
DefaultAI::DefensiveImpl DefaultAI::defensiveImpl;

/// Constructor of DefaultAI
DefaultAI::DefaultAI(Game& ggame, Player_Number const pid, uint8_t const t)
   : Computer_Player(ggame, pid),
     type(t),
     m_buildable_changed(true),
     m_mineable_changed(true),
     player(nullptr),
     tribe(nullptr),
     total_constructionsites(0),
     next_road_due_(2000),
     next_stats_update_due_(30000),
     next_construction_due_(1000),
     next_mine_construction_due_(0),
     next_productionsite_check_due_(0),
     next_mine_check_due_(0),
     next_militarysite_check_due_(0),
     next_attack_consideration_due_(300000),
     next_helpersites_check_due_(180000),
     inhibit_road_building_(0),
     time_of_last_construction_(0),
     numof_warehouses_(0),
     new_buildings_stop_(false),
     unstationed_milit_buildings_(0),
     military_under_constr_(0),
     military_last_dismantle_(0),
     military_last_build_(0) {
}

DefaultAI::~DefaultAI() {
	while (not buildable_fields.empty()) {
		delete buildable_fields.back();
		buildable_fields.pop_back();
	}

	while (not mineable_fields.empty()) {
		delete mineable_fields.back();
		mineable_fields.pop_back();
	}

	while (not economies.empty()) {
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

	if (tribe == nullptr)
		late_initialization();

	const int32_t gametime = game().get_gametime();

	if (m_buildable_changed) {
		// update statistics about buildable fields
		update_all_buildable_fields(gametime);
	}

	m_buildable_changed = false;

	// if there are more than one economy try to connect them with a road.
	if (next_road_due_ <= gametime) {
		next_road_due_ = gametime + 1000;

		if (construct_roads(gametime)) {
			m_buildable_changed = true;
			// m_mineable_changed = true;
			return;
		}
	} else
		// only go on, after defaultAI tried to connect all economies.
		return;

	// NOTE Because of the check above, the following parts of think() are used
	// NOTE only once every second at maximum. This increases performance and as
	// NOTE human player_s can not even react that fast, it should not be a
	// NOTE disadvantage for the defaultAI.
	// This must be checked every time as changes of bobs in AI area aren't
	// handled by the AI itself.
	update_all_not_buildable_fields();

	// IF defaultAI is AGGRESSIVE - we definitely should consider to attack as
	// often as possible.
	if (type == AGGRESSIVE)
		if (next_attack_consideration_due_ <= gametime)
			consider_attack(gametime);

	// check if anything in the economies changed.
	// This needs to be done before new buildings are placed, to ensure that no
	// empty economy is left.
	if (check_economies())
		return;

	// Before thinking about a new construction, update current stats, to have
	// a better view on current economy.
	if (next_stats_update_due_ <= gametime)
		update_productionsite_stats(gametime);

	// Now try to build something if possible
	if (next_construction_due_ <= gametime) {
		next_construction_due_ = gametime + 2000;

		if (construct_building(gametime)) {
			time_of_last_construction_ = gametime;
			return;
		}
	}

	// verify that our production sites are doing well
	if (check_productionsites(gametime))
		return;

	// Check the mines and consider upgrading or destroying one
	if (check_mines_(gametime))
		return;

	// consider whether a change of the soldier capacity of some militarysites
	// would make sense.
	if (check_militarysites(gametime))
		return;

	//  Finally consider military actions if defaultAI type is Aggressive or
	// Normal.
	if (!(type == DEFENSIVE))
		if (next_attack_consideration_due_ <= gametime)
			consider_attack(gametime);

	// improve existing roads!
	// This sounds important, but actually is not as important as the other
	// actions are. Reasons are the following:
	//    * The "donkey feature" made economies more stable, even with stupid
	//      roads.
	//    * If defaultAI builds too much roads, it will waste good building
	//      space.
	if (improve_roads(gametime)) {
		m_buildable_changed = true;
		m_mineable_changed = true;
		inhibit_road_building_ = gametime + 2500;
		return;
	}
}

/// called by Widelands game engine when an immovable changed
void DefaultAI::receive(const NoteImmovable& note) {
	if (note.lg == LOSE)
		lose_immovable(*note.pi);
	else
		gain_immovable(*note.pi);
}

/// called by Widelands game engine when a field changed
void DefaultAI::receive(const NoteFieldPossession& note) {
	if (note.lg == GAIN)
		unusable_fields.push_back(note.fc);
}

/**
 * Cares for all variables not initialised during construction
 *
 * When DefaultAI is constructed, some information is not yet available (e.g.
 * world), so this is done after complete loading of the map.
 */
void DefaultAI::late_initialization() {
	player = game().get_player(player_number());
	NoteReceiver<NoteImmovable>::connect(*player);
	NoteReceiver<NoteFieldPossession>::connect(*player);
	tribe = &player->tribe();
	log("ComputerPlayer(%d): initializing (%u)\n", player_number(), type);
	Ware_Index const nr_wares = tribe->get_nrwares();
	wares.resize(nr_wares);

	for (Ware_Index i = 0; i < nr_wares; ++i) {
		wares.at(i).producers_ = 0;
		wares.at(i).consumers_ = 0;
		wares.at(i).preciousness_ = tribe->get_ware_descr(i)->preciousness();
	}

	// collect information about the different buildings our tribe can construct
	Building_Index const nr_buildings = tribe->get_nrbuildings();
	const World& world = game().world();

	for (Building_Index i = 0; i < nr_buildings; ++i) {
		const Building_Descr& bld = *tribe->get_building_descr(i);
		const std::string& building_name = bld.name();
		const BuildingHints& bh = bld.hints();
		buildings.resize(buildings.size() + 1);
		BuildingObserver& bo = buildings.back();
		bo.name = building_name.c_str();
		bo.id = i;
		bo.desc = &bld;
		bo.type = BuildingObserver::BORING;
		bo.cnt_built_ = 0;
		bo.cnt_under_construction_ = 0;
		bo.cnt_target_ = 0;
		bo.stocklevel_ = 0;
		bo.stocklevel_time = 0;
		bo.last_dismantle_time_ = 0;
		// this is set to negative number, otherwise the AI would wait 25 sec
		// after game start not building anything
		bo.construction_decision_time_ = -60 * 60 * 1000;
		bo.production_hint_ = -1;
		bo.current_stats_ = 0;
		bo.unoccupied_ = false;
		bo.is_basic_ = false;
		bo.is_buildable_ = bld.is_buildable();
		bo.need_trees_ = bh.is_logproducer();
		bo.need_stones_ = bh.is_stoneproducer();
		bo.mines_marble_ = bh.is_marbleproducer();
		bo.need_water_ = bh.get_needs_water();
		bo.mines_water_ = bh.mines_water();
		bo.recruitment_ = bh.for_recruitment();
		bo.space_consumer_ = bh.is_space_consumer();

		if (char const* const s = bh.get_renews_map_resource()) {
			bo.production_hint_ = tribe->safe_ware_index(s);

			if (kHintDebug)
				log(" TDEBUG: %-20s get production hint: %d\n", bo.name, bo.production_hint_);
		}

		// I just presume cut wood is named "log" in the game
		if (tribe->safe_ware_index("log") == bo.production_hint_)
			bo.plants_trees_ = true;
		else
			bo.plants_trees_ = false;

		if (kHintDebug)
			log(" building %-15s: plants trees: %s\n", bo.name, bo.plants_trees_ ? "Y" : "N");

		// Read all interesting data from ware producing buildings
		if (typeid(bld) == typeid(ProductionSite_Descr)) {
			const ProductionSite_Descr& prod =
			   ref_cast<ProductionSite_Descr const, Building_Descr const>(bld);
			bo.type = bld.get_ismine() ? BuildingObserver::MINE : BuildingObserver::PRODUCTIONSITE;
			container_iterate_const(BillOfMaterials, prod.inputs(), j)
			bo.inputs_.push_back(j.current->first);
			container_iterate_const(ProductionSite_Descr::Output, prod.output_ware_types(), j)
			bo.outputs_.push_back(*j.current);

			if (bo.type == BuildingObserver::MINE) {
				// get the resource needed by the mine
				if (char const* const s = bh.get_mines())
					bo.mines_ = world.get_resource(s);

				bo.mines_percent_ = bh.get_mines_percent();
			}

			bo.is_basic_ = bh.is_basic();
			bo.prod_build_material_ = bh.prod_build_material();

			continue;
		}

		if (typeid(bld) == typeid(MilitarySite_Descr)) {
			bo.type = BuildingObserver::MILITARYSITE;
			continue;
		}

		if (typeid(bld) == typeid(Warehouse_Descr)) {
			bo.type = BuildingObserver::WAREHOUSE;
			continue;
		}

		if (typeid(bld) == typeid(TrainingSite_Descr)) {
			bo.type = BuildingObserver::TRAININGSITE;
			continue;
		}

		if (typeid(bld) == typeid(ConstructionSite_Descr)) {
			bo.type = BuildingObserver::CONSTRUCTIONSITE;
			continue;
		}
	}

	total_constructionsites = 0;
	next_construction_due_ = 0;
	next_road_due_ = 1000;
	next_productionsite_check_due_ = 0;
	inhibit_road_building_ = 0;
	// Add all fields that we own
	Map& map = game().map();
	std::set<OPtr<PlayerImmovable>> found_immovables;

	for (Y_Coordinate y = 0; y < map.get_height(); ++y) {
		for (X_Coordinate x = 0; x < map.get_width(); ++x) {
			FCoords f = map.get_fcoords(Coords(x, y));

			if (f.field->get_owned_by() != player_number())
				continue;

			unusable_fields.push_back(f);

			if (upcast(PlayerImmovable, imm, f.field->get_immovable()))

				//  Guard by a set - immovables might be on several nodes at once.
				if (&imm->owner() == player and not found_immovables.count(imm)) {
					found_immovables.insert(imm);
					gain_immovable(*imm);
				}
		}
	}
}

/**
 * Checks ALL available buildable fields.
 *
 * this shouldn't be used often, as it might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_buildable_fields(const int32_t gametime) {
	while (not buildable_fields.empty() and buildable_fields.front()->next_update_due_ <= gametime) {
		BuildableField& bf = *buildable_fields.front();

		//  check whether we lost ownership of the node
		if (bf.coords.field->get_owned_by() != player_number()) {
			delete &bf;
			buildable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player->get_buildcaps(bf.coords) & BUILDCAPS_SIZEMASK) == 0) {
			unusable_fields.push_back(bf.coords);
			delete &bf;
			buildable_fields.pop_front();
			continue;
		}

		update_buildable_field(bf);
		bf.next_update_due_ = gametime + kFieldUpdateInterval;
		buildable_fields.push_back(&bf);
		buildable_fields.pop_front();
	}
}

/**
 * Checks ALL available mineable fields.
 *
 * this shouldn't be used often, as it might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_mineable_fields(const int32_t gametime) {
	while (not mineable_fields.empty() and mineable_fields.front()->next_update_due_ <= gametime) {
		MineableField* mf = mineable_fields.front();

		//  check whether we lost ownership of the node
		if (mf->coords.field->get_owned_by() != player_number()) {
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player->get_buildcaps(mf->coords) & BUILDCAPS_MINE) == 0) {
			unusable_fields.push_back(mf->coords);
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		update_mineable_field(*mf);
		mf->next_update_due_ = gametime + kFieldUpdateInterval;  // in fact this has very small effect
		mineable_fields.push_back(mf);
		mineable_fields.pop_front();
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

	if (maxchecks > 50)
		maxchecks = 50;

	for (uint32_t i = 0; i < maxchecks; ++i) {
		//  check whether we lost ownership of the node
		if (unusable_fields.front().field->get_owned_by() != pn) {
			unusable_fields.pop_front();
			continue;
		}

		// check whether building capabilities have improved
		if (player->get_buildcaps(unusable_fields.front()) & BUILDCAPS_SIZEMASK) {
			buildable_fields.push_back(new BuildableField(unusable_fields.front()));
			unusable_fields.pop_front();
			update_buildable_field(*buildable_fields.back());
			continue;
		}

		if (player->get_buildcaps(unusable_fields.front()) & BUILDCAPS_MINE) {
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
	FindNodeUnowned find_unowned(player, game());
	FindNodeUnownedMineable find_unowned_mines_pots(player, game());
	Player_Number const pn = player->player_number();
	field.unowned_land_nearby_ =
	   map.find_fields(Area<FCoords>(field.coords, range), nullptr, find_unowned);

	if (field.unowned_land_nearby_ > 2)  // 2 is 'reasonably low' number here
		field.unowned_mines_pots_nearby_ = map.find_fields(
		   Area<FCoords>(field.coords, range + 2),
		   nullptr,
		   find_unowned_mines_pots);  //+2: a mine can mine raw materials from some range
	else
		field.unowned_mines_pots_nearby_ = 0;

	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	// Search in a radius of range
	map.find_immovables(Area<FCoords>(field.coords, range), &immovables);

	// Is this a general update or just for military consideration
	// (second is used in check_militarysites)
	if (!military) {
		int32_t const tree_attr = Map_Object_Descr::get_attribute_id("tree");
		field.reachable = false;
		field.preferred_ = false;
		field.enemy_nearby_ = false;
		field.military_capacity_ = 0;
		field.military_loneliness_ = 1000;  // instead of floats(v-
		field.military_presence_ = 0;
		field.military_stationed_ = 0;
		field.trees_nearby_ = 0;
		field.space_consumers_nearby_ = 0;
		field.producers_nearby_.clear();
		field.producers_nearby_.resize(wares.size());
		field.consumers_nearby_.clear();
		field.consumers_nearby_.resize(wares.size());
		std::vector<Coords> water_list;

		if (field.water_nearby_ == -1) {  //-1 means "value has never been calculated"
			FindNodeWater find_water(game().world());
			map.find_fields(Area<FCoords>(field.coords, 4), &water_list, find_water);
			field.water_nearby_ = water_list.size();
		}

		FCoords fse;
		map.get_neighbour(field.coords, WALK_SE, &fse);

		if (BaseImmovable const* const imm = fse.field->get_immovable())
			if (dynamic_cast<Flag const*>(imm)
			    or(dynamic_cast<Road const*>(imm) && (fse.field->nodecaps() & BUILDCAPS_FLAG)))
				field.preferred_ = true;

		for (uint32_t i = 0; i < immovables.size(); ++i) {
			const BaseImmovable& base_immovable = *immovables.at(i).object;

			if (dynamic_cast<const Flag*>(&base_immovable))
				field.reachable = true;

			if (upcast(PlayerImmovable const, player_immovable, &base_immovable))

				//// TODO  Only continue; if this is an opposing site
				//// TODO  allied sites should be counted for military influence
				if (player_immovable->owner().player_number() != pn) {
					if (player->is_hostile(player_immovable->owner()))
						field.enemy_nearby_ = true;

					continue;
				}

			if (upcast(Building const, building, &base_immovable)) {
				if (upcast(ConstructionSite const, constructionsite, building)) {
					const Building_Descr& target_descr = constructionsite->building();

					if (dynamic_cast<ProductionSite_Descr const*>(&target_descr))
						consider_productionsite_influence(
						   field,
						   immovables.at(i).coords,
						   get_building_observer(constructionsite->name().c_str()));
				}

				if (dynamic_cast<const ProductionSite*>(building))
					consider_productionsite_influence(
					   field, immovables.at(i).coords, get_building_observer(building->name().c_str()));
			}

			if (immovables.at(i).object->has_attribute(tree_attr))
				++field.trees_nearby_;
		}

		// stones are not renewable, we will count them only if previous state si nonzero
		if (field.stones_nearby_ > 0) {

			int32_t const stone_attr = Map_Object_Descr::get_attribute_id("stone");
			field.stones_nearby_ = 0;

			for (uint32_t j = 0; j < immovables.size(); ++j) {
				// const BaseImmovable & base_immovable = *immovables.at(i).object;
				if (immovables.at(j).object->has_attribute(stone_attr))
					++field.stones_nearby_;
			}
		}

		//// ground water is not renewable and its amount can only fall, we will count them only if
		/// previous state si nonzero
		if (field.ground_water_ > 0) {
			field.ground_water_ =
			   field.coords.field->get_resources_amount();  // field->get_resources();

			// log(" RESOURCE DEBUG: %2d",field.get_resources());
		}
	}

	// folowing is done allways (regardless of military or not)

	if (kMilitScoreDebug)
		log(" FIELD SCORE: central building: %3dx%3d\n", field.coords.x, field.coords.y);

	field.military_stationed_ = 0;

	for (uint32_t i = 0; i < immovables.size(); ++i) {
		const BaseImmovable& base_immovable = *immovables.at(i).object;

		// testing if it is enemy-owned field
		// TODO count such fields...
		if (upcast(PlayerImmovable const, player_immovable, &base_immovable))

			// TODO  Only continue; if this is an opposing site
			// TODO  allied sites should be counted for military influence
			if (player_immovable->owner().player_number() != pn) {
				if (player->is_hostile(player_immovable->owner()))
					field.enemy_nearby_ = true;

				continue;
			}

		if (upcast(Building const, building, &base_immovable)) {
			if (upcast(ConstructionSite const, constructionsite, building)) {
				const Building_Descr& target_descr = constructionsite->building();

				if (upcast(MilitarySite_Descr const, target_ms_d, &target_descr)) {
					const int32_t dist = map.calc_distance(field.coords, immovables.at(i).coords);
					const int32_t radius = target_ms_d->get_conquers() + 4;
					const int32_t v = radius - dist;

					if (v > 0) {
						if (kMilitScoreDebug)
							log(" FIELD SCORE:  testing near military construction site at %3dx%3d, "
							    "presumed capacity: %d, loneliness:%4f (%2d:%2d)\n",
							    immovables.at(i).coords.x,
							    immovables.at(i).coords.y,
							    2,
							    static_cast<double>(dist) / radius,
							    dist,
							    radius);

						field.military_capacity_ += 2;
						field.military_loneliness_ *= static_cast<double_t>(dist) / radius;
						field.military_in_constr_nearby_ += 1;
					}
				}
			}

			if (upcast(MilitarySite const, militarysite, building)) {
				const int32_t dist = map.calc_distance(field.coords, immovables.at(i).coords);
				const int32_t radius = militarysite->get_conquers() + 4;
				const int32_t v = radius - dist;

				if (v > 0 and dist > 0) {
					if (kMilitScoreDebug)
						log(" FIELD SCORE:  testing near military building at %3dx%3d, capacity: %d, "
						    "loneliness:%3f (%2d:%2d), stationed: %" PRIuS "\n",
						    immovables.at(i).coords.x,
						    immovables.at(i).coords.y,
						    militarysite->maxSoldierCapacity(),
						    static_cast<double>(dist) / radius,
						    dist,
						    radius,
						    militarysite->stationedSoldiers().size());

					field.military_capacity_ += militarysite->maxSoldierCapacity();
					field.military_presence_ += militarysite->stationedSoldiers().size();

					if (!militarysite->stationedSoldiers().empty()) {
						field.military_stationed_ += 1;
					}

					field.military_loneliness_ *= static_cast<double_t>(dist) / radius;
				}
			}
		}
	}

	if (kMilitScoreDebug)
		log(" FIELD SCORE:  results: capacity: %d, presence: %d, loneliness: %4d, stationed: %1d\n",
		    field.military_capacity_,
		    field.military_presence_,
		    field.military_loneliness_,
		    field.military_stationed_);

	//}
}

/// Updates one mineable field
void DefaultAI::update_mineable_field(MineableField& field) {
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	Map& map = game().map();
	map.find_immovables(Area<FCoords>(field.coords, 5), &immovables);
	field.reachable = false;
	field.preferred_ = false;
	field.mines_nearby_ = 1;
	FCoords fse;
	map.get_brn(field.coords, &fse);

	if (BaseImmovable const* const imm = fse.field->get_immovable())
		if (dynamic_cast<Flag const*>(imm)
		    or(dynamic_cast<Road const*>(imm) && (fse.field->nodecaps() & BUILDCAPS_FLAG)))
			field.preferred_ = true;

	container_iterate_const(std::vector<ImmovableFound>, immovables, i) {
		if (dynamic_cast<Flag const*>(i.current->object))
			field.reachable = true;
		else if (upcast(Building const, bld, i.current->object)) {
			if (bld->descr().get_ismine()) {
				++field.mines_nearby_;
			} else if (upcast(ConstructionSite const, cs, bld)) {
				if (cs->building().get_ismine())
					++field.mines_nearby_;
			}
		}
	}
}

/// Updates the production and MINE sites statistics needed for construction decision.
void DefaultAI::update_productionsite_stats(int32_t const gametime) {
	// Updating the stats every 10 seconds should be enough
	next_stats_update_due_ = gametime + 10000;

	// Reset statistics for all buildings
	for (uint32_t i = 0; i < buildings.size(); ++i) {
		if (buildings.at(i).cnt_built_ > 0)
			buildings.at(i).current_stats_ = 0;
		// If there are no buildings of that type set the current_stats_ to 100
		else
			buildings.at(i).current_stats_ = 0;  // there was 100, this confuses algorithm

		buildings.at(i).unoccupied_ = false;
	}

	// Check all available productionsites
	for (uint32_t i = 0; i < productionsites.size(); ++i) {
		ProductionSiteObserver& productionsite_observer = productionsites.front();
		// TODO(sirver): Popping here means that we HAVE to push another thing at
		// the end or our loop will not do the right think. This is terrible
		// design. It would be much better to push to a new vector and swap the
		// productionsites' content after the loop. I refuse to fix this though.
		productionsites.pop_front();

		assert(productionsite_observer.bo->cnt_built_ > 0);

		ProductionSite* productionsite = productionsite_observer.site.get(game());
		if (productionsite != nullptr) {
			// Add statistics value
			productionsite_observer.bo->current_stats_ += productionsite->get_crude_statistics();
			if (kStatDebug and abs(productionsite->get_crude_statistics() -
			                       productionsite->get_statistics_percent()) > 50)
				log(" STAT DEBUG: %15s (%3dx%3d): crude statistic: %3d vs official statistics: %3d\n",
				    productionsite->name().c_str(),
				    productionsite->get_position().x,
				    productionsite->get_position().y,
				    productionsite->get_crude_statistics(),
				    productionsite->get_statistics_percent());
			// Check whether this building is completely occupied
			productionsite_observer.bo->unoccupied_ |= !productionsite->can_start_working();
		}
		// Now reorder the buildings
		productionsites.push_back(productionsite_observer);
	}

	// for mines_ also
	// Check all available productionsites
	for (uint32_t i = 0; i < mines_.size(); ++i) {
		assert(mines_.front().bo->cnt_built_ > 0);
		ProductionSite* mine = mines_.front().site.get(game());
		if (mine != nullptr) {
			// Add statistics value
			mines_.front().bo->current_stats_ += mine->get_statistics_percent();
			// Check whether this building is completely occupied
			mines_.front().bo->unoccupied_ |= !mine->can_start_working();
		}
		// Now reorder the buildings
		mines_.push_back(mines_.front());
		mines_.pop_front();
	}

	// Scale statistics down
	for (uint32_t i = 0; i < buildings.size(); ++i) {
		if (buildings.at(i).cnt_built_ > 0)
			buildings.at(i).current_stats_ /= buildings.at(i).cnt_built_;
	}
}

// * Constructs the most needed building
//   algorithm goes over all available spots and all allowed buildings,
//   scores every combination and one with highest and positive score
//   is built.
// * Buildings are split into categories
// * The logic is complex but aproximatelly:
// - buildings producing building material are preffered
// - buildings identified as basic are preffered
// - first bulding of a type is preffered
// - if a bulding is upgradeable, second building is also preffered
//   (there should be no upgrade when there are not two buildings of the same type)
// - algorigthm is trying to take into account actual utlization of buildings
//   (the one shown in GUI/game is not reliable, it calculates own statistics)
// * military buildings have own strategy, split into two situations:
// - there is no enemy
// - there is an enemy
//   Currently more military buildings are built then needed
//   and "optimalization" (dismantling not needed buildings) is done afterwards
bool DefaultAI::construct_building(int32_t gametime) {  // (int32_t gametime)
	//  Just used for easy checking whether a mine or something else was built.
	bool mine = false;
	bool field_blocked = false;
	int32_t spots = 0;
	uint32_t consumers_nearby_count = 0;
	// this is to increase score so also building near borders can be built
	int32_t bulgarian_constant = 12;
	std::vector<int32_t> spots_avail;
	spots_avail.resize(4);

	for (int32_t i = 0; i < 4; ++i)
		spots_avail.at(i) = 0;

	for (std::list<BuildableField*>::iterator i = buildable_fields.begin();
	     i != buildable_fields.end();
	     ++i)
		++spots_avail.at((*i)->coords.field->nodecaps() & BUILDCAPS_SIZEMASK);

	// calculating expand factor
	int32_t expand_factor = 0;

	if (type != DEFENSIVE) {
		++expand_factor;

		// check space and set the need for expansion
		if (spots_avail.at(BUILDCAPS_BIG) < static_cast<uint16_t>(2 + (productionsites.size() / 50)))
			expand_factor += 2;

		if (spots_avail.at(BUILDCAPS_MEDIUM) + spots_avail.at(BUILDCAPS_BIG) <
		    static_cast<uint16_t>(4 + (productionsites.size() / 50)))
			expand_factor += type;

		spots = spots_avail.at(BUILDCAPS_SMALL);
		spots += spots_avail.at(BUILDCAPS_MEDIUM);
		spots += spots_avail.at(BUILDCAPS_BIG);

		if (type == AGGRESSIVE)
			spots -= militarysites.size() / 20;

		if (spots < 16)
			expand_factor *= 2;

		if ((type == AGGRESSIVE) && spots < 32)
			expand_factor *= 2;
	} else {
		// check space and set the need for expansion
		if (spots_avail.at(BUILDCAPS_BIG) < 7)
			++expand_factor;

		if (spots_avail.at(BUILDCAPS_MEDIUM) + spots_avail.at(BUILDCAPS_BIG) < 12)
			++expand_factor;

		if (spots_avail.at(BUILDCAPS_SMALL) + spots_avail.at(BUILDCAPS_MEDIUM) +
		       spots_avail.at(BUILDCAPS_BIG) <
		    16)
			expand_factor *= 3;
	}

	// checking amount of free spots, if needed setting new building stop flag
	new_buildings_stop_ = false;

	if (militarysites.size() * 2 + 20 < productionsites.size() or spots <
	    (7 + static_cast<int32_t>(productionsites.size()) / 5)) {
		new_buildings_stop_ = true;
	}

	if (kNewBuildingDebug)
		log(" TDEBUG new buildings stop: %s; milit: %3" PRIuS " vs prod: %3" PRIuS
		    " buildings, spots: %4d\n",
		    new_buildings_stop_ ? "Y" : "N",
		    militarysites.size(),
		    productionsites.size(),
		    spots);

	// sometimes there is to many military buildings in construction, so we must
	// prevent initialization of further buildings start
	bool new_military_buildings_stop = false;
	// in areas close to enemies, above limit is not effective, so we have second one
	// more benevolent limit
	bool near_enemy_b_buildings_stop = false;
	// in some situation expansion just halts because there are not good spots to expand
	// so this is a boost to increase a score and allow building a military building on a spot
	// that is normally not suitable for building
	int32_t military_boost = 1;

	int32_t treshold = (militarysites.size() + productionsites.size()) / 100 + 1;

	if (unstationed_milit_buildings_ + military_under_constr_ / 3 > treshold) {
		new_military_buildings_stop = true;

		if (kMilitaryDebug)
			log(" TDEBUG new military buildings stop ON, %d %d \n",
			    unstationed_milit_buildings_,
			    military_under_constr_);
	} else if (kMilitaryDebug)
		log(" TDEBUG new military buildings stop OFF, %d %d\n",
		    unstationed_milit_buildings_,
		    military_under_constr_);

	if (unstationed_milit_buildings_ + military_under_constr_ / 3 > 2 * treshold) {
		near_enemy_b_buildings_stop = true;

		if (kMilitaryDebug)
			log(" TDEBUG new military near-enemy buildings stop ON, %d %d \n",
			    unstationed_milit_buildings_,
			    military_under_constr_);
	}

	// here we deal with situation when for some time no new military building was built
	// in fact this is a last time when there were any military buildings in construction
	if ((unstationed_milit_buildings_ + military_under_constr_) > 0)
		military_last_build_ = game().get_gametime();

	if (military_last_build_ + 1 * 60 * 1000 < game().get_gametime()) {
		if (kMilitaryDebug)
			log(" TDEBUG: Boosting military building\n");

		military_boost = 200;
	}

	// Defensive AIs also attack sometimes (when they want to expand)
	if (type == DEFENSIVE && expand_factor > 1)
		if (next_attack_consideration_due_ <= game().get_gametime())
			consider_attack(game().get_gametime());

	// Building_Index proposed_building = INVALID_INDEX; // I need BuildingObserver not index
	BuildingObserver* best_building = nullptr;
	int32_t proposed_priority = 0;
	Coords proposed_coords;

	// Remove outdated fields from blocker list
	for (std::list<BlockedField>::iterator i = blocked_fields.begin(); i != blocked_fields.end();)
		if (i->blocked_until_ < game().get_gametime()) {
			i = blocked_fields.erase(i);
		} else
			++i;

	// these are 3 helping variables
	bool output_is_needed = false;
	int16_t max_preciousness = 0;         // preciousness_ of most precious output
	int16_t max_needed_preciousness = 0;  // preciousness_ of most precious NEEDED output

	// first scan all buildable fields for regular buildings
	for (std::list<BuildableField*>::iterator i = buildable_fields.begin();
	     i != buildable_fields.end();
	     ++i) {
		BuildableField* const bf = *i;

		if (!bf->reachable)
			continue;

		if (time(nullptr) % 5 == 0)
			continue;  // add randomnes and ease AI

		// Continue if field is blocked at the moment
		field_blocked = false;

		for (std::list<BlockedField>::iterator j = blocked_fields.begin(); j != blocked_fields.end();
		     ++j)
			if (j->coords == bf->coords)
				field_blocked = true;

		// continue;
		if (field_blocked)
			continue;

		assert(player);
		int32_t const maxsize = player->get_buildcaps(bf->coords) & BUILDCAPS_SIZEMASK;

		// For every field test all buildings
		for (uint32_t j = 0; j < buildings.size(); ++j) {
			BuildingObserver& bo = buildings.at(j);

			if (!bo.buildable(*player))
				continue;

			if (time(nullptr) % 3 == 0)
				continue;  // add randomnes and ease AI

			if (bo.type == BuildingObserver::MINE)
				continue;

			if (gametime - bo.construction_decision_time_ < kBuildingMinInterval)
				continue;

			if (bo.unoccupied_)
				continue;

			if (bo.type != BuildingObserver::MILITARYSITE and bo.cnt_under_construction_ >= 2)
				continue;

			// so we are going to seriously evaluate this building on this field,
			// first some base info
			// if at least on of outputs is needed
			output_is_needed = false;
			// max presiousness of outputs
			max_preciousness = 0;
			// max preciousness of most needed output
			max_needed_preciousness = 0;

			// Check if the produced wares are needed (if it is producing anything)
			if (!bo.outputs_.empty()) {
				container_iterate(std::list<EconomyObserver*>, economies, l) {
					// Don't check if the economy has no warehouse.
					if ((*l.current)->economy.warehouses().empty())
						continue;

					for (uint32_t m = 0; m < bo.outputs_.size(); ++m) {
						Ware_Index wt(static_cast<size_t>(bo.outputs_.at(m)));

						if ((*l.current)->economy.needs_ware(wt)) {
							output_is_needed = true;

							if (wares.at(bo.outputs_.at(m)).preciousness_ > max_needed_preciousness)
								max_needed_preciousness = wares.at(bo.outputs_.at(m)).preciousness_;

							max_preciousness = wares.at(bo.outputs_.at(m)).preciousness_;
						} else {
							if (wares.at(bo.outputs_.at(m)).preciousness_ > max_preciousness)
								max_preciousness = wares.at(bo.outputs_.at(m)).preciousness_;
						}
					}
				}
			}

			// if current field is not big enough
			if (bo.desc->get_size() > maxsize)
				continue;

			int32_t prio = 0;  // score of a bulding on a field

			if (bo.type == BuildingObserver::PRODUCTIONSITE) {

				// this can be only a well (as by now)
				if (bo.mines_water_) {
					if (bf->ground_water_ < 2)
						continue;

					if (bo.cnt_under_construction_ + bo.unoccupied_ > 0)
						continue;
					if ((bo.cnt_built_ + bo.unoccupied_) > 0 and gametime < kBaseInfrastructureTime)
						continue;
					if (new_buildings_stop_)
						continue;
					bo.cnt_target_ =
					   2 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 20;
					if ((bo.cnt_built_ + bo.cnt_under_construction_ + bo.unoccupied_) > bo.cnt_target_)
						continue;

					if (bo.stocklevel_time < game().get_gametime() - 30 * 1000) {
						bo.stocklevel_ = get_stocklevel(bo);
						bo.stocklevel_time = game().get_gametime();
					}
					if (bo.stocklevel_ > 50)
						continue;
					if (kWellDebug)
						log(" kWellDebug: %-15s has %2d resources, stocklevel: %3d\n",
						    bo.name,
						    bf->ground_water_,
						    bo.stocklevel_);
					prio = bf->ground_water_ - 2;
					prio = recalc_with_border_range(*bf, prio);

				} else if (bo.need_trees_) {  // LUMBERJACS

					if (kWoodDebug)
						log(" TDEBUG: %1d: buildings count:%2d-%2d-%2d, trees nearby: %3d, "
						    "build.stop: %s\n",
						    player_number(),
						    bo.cnt_built_,
						    bo.cnt_under_construction_,
						    bo.unoccupied_,
						    bf->trees_nearby_ * 2,
						    (new_buildings_stop_) ? "Y" : "N");

					if (bo.cnt_built_ + bo.cnt_under_construction_ + bo.unoccupied_ <= 2)
						prio = bulgarian_constant + 200 + bf->trees_nearby_;
					else if (bo.cnt_under_construction_ + bo.unoccupied_ <= 1) {
						prio =
						   bf->trees_nearby_ - 5 -
						   new_buildings_stop_ * 40;  //+ bf->producers_nearby_.at(bo.outputs_.at(0))*5;
					}

					if (kWoodDebug and prio > 0)
						log(" TDEBUG: %1d: suggesting woodcutter with prio: %2d, total: %2d\n",
						    player_number(),
						    prio,
						    bo.cnt_built_ + bo.cnt_under_construction_ + bo.unoccupied_);
				} else if (bo.need_stones_) {

					// quaries are generally to be built everywhere where stones are
					// no matter the need for stones, as stones are considered an obstacle
					// to expansion
					if (bo.cnt_under_construction_ > 0)
						continue;
					prio = bf->stones_nearby_;

					if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
						bo.stocklevel_ = get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
						bo.stocklevel_time = game().get_gametime();
					}

					if (bo.stocklevel_ < 20)
						prio = prio * 2;

					if (bo.total_count() == 0)
						prio = prio * 2;

					// to prevent to many quaries on one spot
					prio = prio - 30 * bf->producers_nearby_.at(bo.outputs_.at(0));

				} else if (bo.production_hint_ >= 0) {
					// first setting targets (needed also for dismantling)
					if (bo.plants_trees_)
						bo.cnt_target_ =
						   2 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 15;
					else
						bo.cnt_target_ =
						   1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 20;

					if ((bo.cnt_under_construction_ + bo.unoccupied_) > 0)
						continue;

					// production hint (f.e. associate forester with logs)

					if (bo.need_water_ and bf->water_nearby_ < 3)  // probably some of them needs water
						continue;

					if (bo.plants_trees_) {  // RANGERS
						if (bo.total_count() < bo.cnt_target_)
							prio = 70;
						else {  // even when we are above goal we need to consider level of stock
							if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
								bo.stocklevel_ =
								   get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
								bo.stocklevel_time = game().get_gametime();
							}

							if (bo.stocklevel_ < 5 and new_buildings_stop_)
								prio = bf->producers_nearby_.at(bo.production_hint_) * 5 - 5 -
								       bf->trees_nearby_ * 2;
							else if (bo.stocklevel_ < 50)
								prio = 50 + bf->producers_nearby_.at(bo.production_hint_) * 5 -
								       bf->trees_nearby_ * 2;
							else
								continue;  // we are above tresh
						}
					} else if (gametime > kBaseInfrastructureTime and not
					                         new_buildings_stop_) {  // gamekeepers or so
						if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
							bo.stocklevel_ =
							   get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
							bo.stocklevel_time = game().get_gametime();
						}

						if (bo.total_count() < bo.cnt_target_) {
							prio = bf->producers_nearby_.at(bo.production_hint_) * 10;
							prio = recalc_with_border_range(*bf, prio);

						} else if (bo.stocklevel_ < 50 and not new_buildings_stop_) {
							prio = bf->producers_nearby_.at(bo.production_hint_) * 5;
							prio = recalc_with_border_range(*bf, prio);  // only for not wood producers_
						} else
							continue;
					}

					if (kHintDebug or(kWoodDebug and bo.plants_trees_))
						log(" TDEBUG: %1d: suggesting new %16s at %3dx%3d, current count: %2d, goal: "
						    "%2d, stocklevel_:%3d, new build. stop: %s, prio: %d\n",
						    player_number(),
						    bo.name,
						    bf->coords.x,
						    bf->coords.y,
						    bo.total_count(),
						    bo.cnt_target_,
						    bo.stocklevel_,
						    new_buildings_stop_ ? "Y" : "N",
						    prio);

					if (prio <= 0)
						continue;
				} else if (bo.recruitment_ and gametime >
				           kBaseInfrastructureTime and not new_buildings_stop_) {
					// this will depend on number of mines_ and productionsites
					if (static_cast<int32_t>((productionsites.size() + mines_.size()) / 30) >
					       bo.total_count() and bo.cnt_under_construction_ ==
					    0)
						prio = 4 + bulgarian_constant;
				} else {  // finally normal productionsites
					if (bo.production_hint_ >= 0)
						continue;

					if ((bo.cnt_under_construction_ + bo.unoccupied_) > 0)
						continue;

					// first eliminate buildings needing water if there is short supplies
					if (bo.need_water_ and bf->water_nearby_ < 3)
						continue;

					if ((bo.is_basic_ or bo.prod_build_material_)and bo.total_count() == 0)
						prio = 150 + max_preciousness;
					else if (game().get_gametime() <
					         kBaseInfrastructureTime or
					            new_buildings_stop_)  // leave 15 minutes for basic infrastructure only
						continue;
					else if (((bo.is_basic_ or bo.prod_build_material_)and bo.total_count() <=
					          1)or(output_is_needed and bo.total_count() == 0))
						prio = 80 + max_preciousness;
					else if (bo.inputs_.size() == 0) {
						bo.cnt_target_ =
						   1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 8;

						if (bo.cnt_built_ >
						    bo.cnt_target_ and not
						       bo.space_consumer_)  // spaceconsumers_ can be built more then target
							continue;

						if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
							bo.stocklevel_ = get_stocklevel(bo);
							bo.stocklevel_time = game().get_gametime();
						}
						if (kSpaceDebug and bo.space_consumer_)
							log(" TDEBUG: %1d: considering %-15s, stock: %3d\n",
							    player_number(),
							    bo.name,
							    bo.stocklevel_);

						if (bo.stocklevel_ < 50) {
							prio = max_preciousness + bulgarian_constant;

							if (bo.space_consumer_)
								prio += 5;

							if (not bo.space_consumer_)
								prio -= bf->producers_nearby_.at(bo.outputs_.at(0)) *
								        20;  // leave some free space between them

							if (bo.cnt_built_ < 2)
								prio += 5;

							prio = recalc_with_border_range(*bf, prio);

							if (kProductionDebug or(kSpaceDebug and bo.space_consumer_))
								log(" TDEBUG: %1d: proposing %-15s , on stock: %3d(<50), stat: %3d, "
								    "count: %2d/T:%2d, setting priority: %2d, on %3d %3d\n",
								    player_number(),
								    bo.name,
								    bo.stocklevel_,
								    bo.current_stats_,
								    bo.total_count(),
								    bo.cnt_target_,
								    prio,
								    bf->coords.x,
								    bf->coords.y);
						}
					} else if (!bo.inputs_.empty()) {
						// to have two buildings from everything (intended for upgradeable buildings)
						// but I do not know how to identify such buildings
						if (bo.cnt_built_ == 1
						    and game().get_gametime() > 60 * 60 * 1000
							and bo.desc->enhancement() != INVALID_INDEX
						    and !mines_.empty()) {
							prio = max_preciousness + bulgarian_constant;
						}
						// if output is needed and there are no idle buildings
						else if (output_is_needed) {
							if (bo.cnt_built_ > 0 and bo.current_stats_ > 80) {
								prio = max_preciousness + bulgarian_constant + 30;

								if (kProductionDebug)
									log(" TDEBUG: %2d/%-15s with inputs_: stats: %3d>90, setting "
									    "priority: %2d, on %3d %3d\n",
									    bo.id,
									    bo.name,
									    bo.current_stats_,
									    prio,
									    bf->coords.x,
									    bf->coords.y);
							} else if (bo.cnt_built_ > 0 and bo.current_stats_ > 55) {
								prio = max_preciousness + bulgarian_constant;

								if (kProductionDebug)
									log(" TDEBUG: %2d/%-15s with inputs_: stats: %3d>60, setting "
									    "priority: %2d, on %3d %3d\n",
									    bo.id,
									    bo.name,
									    bo.current_stats_,
									    prio,
									    bf->coords.x,
									    bf->coords.y);
							}
						}
					}

					if (prio <= 0)
						continue;

					// then we consider borders and enemies nearby (if any)
					prio = recalc_with_border_range(*bf, prio);

					//+1 if any consumers_ are nearby
					consumers_nearby_count = 0;

					for (size_t k = 0; k < bo.outputs_.size(); ++k)
						consumers_nearby_count += bf->consumers_nearby_.at(bo.outputs_.at(k));

					if (consumers_nearby_count > 0)
						prio += 1;
				}
			}  // production sites done
			else if (bo.type == BuildingObserver::MILITARYSITE) {
				if (military_boost > 1 and kMilitaryDebug)
					log(" TDEBUG: boosting: unowned land %d \n", bf->unowned_land_nearby_);

				if (new_military_buildings_stop and not bf->enemy_nearby_)
					continue;

				if (near_enemy_b_buildings_stop and bf->enemy_nearby_)
					continue;

				if (bo.desc->get_size() ==
				    3 and game().get_gametime() <
				       15 * 60 * 1000)  // do not built fortresses in first half of hour of game
					continue;

				if (!bf->unowned_land_nearby_)
					continue;

				// not to build so many military buildings nearby
				if (!bf->enemy_nearby_ and bf->military_in_constr_nearby_ > 1)
					continue;

				// here is to consider unowned potential mines
				int32_t mines_spots_score = 0;
				mines_spots_score = bf->unowned_mines_pots_nearby_;

				if (mines_spots_score > 0) {
					mines_spots_score *= 4;
					mines_spots_score += 8;
				}

				prio = (bf->unowned_land_nearby_ - 4 + mines_spots_score + bf->stones_nearby_ / 2 +
				        bf->military_loneliness_ / 5 - 100 + military_boost);  // * (1 + type);

				if (kMilitaryDebug)
					log(" NEW MILITARY: %3dx%3d: unowned: %3d(%3d), mines_: %3d, lonel.: %4d (%4d), "
					    "stones: %2d, boost: %3d,result: %3d, enemy:% 2d, cur stat: %2d/%2d/%2d, "
					    "stops:%s %s\n",
					    bf->coords.x,
					    bf->coords.y,
					    bf->unowned_land_nearby_ - 4,
					    mines_spots_score,
					    bf->unowned_mines_pots_nearby_,
					    bf->military_loneliness_ / 5 - 100,
					    bf->military_loneliness_,
					    bf->stones_nearby_ / 2,
					    military_boost,
					    prio,
					    bf->enemy_nearby_,
					    bo.cnt_built_,
					    unstationed_milit_buildings_,
					    bo.cnt_under_construction_,
					    new_military_buildings_stop ? "Y" : "N",
					    near_enemy_b_buildings_stop ? "Y" : "N");

				if (bo.desc->get_size() < maxsize)
					prio = prio - 5;  // penalty

				if (bf->enemy_nearby_ and bf->military_capacity_ < 12) {
					if (kMilitaryDebug)
						log(" NEW MILITARY: Military capacity: %2d, boosting priority by 100d\n",
						    bf->military_capacity_);

					prio += 100;
				}

				if (kMilitaryDebug and prio > 0)
					log(" NEW MILITARY:  candidate's final priority: %d \n", prio);
			} else if (bo.type == BuildingObserver::WAREHOUSE) {
				//  Build one warehouse for ~every 35 productionsites and mines_.
				//  Militarysites are slightly important as well, to have a bigger
				//  chance for a warehouses (containing waiting soldiers or wares
				//  needed for soldier training) near the frontier.
				if ((static_cast<int32_t>(productionsites.size() + mines_.size())) / 35 >
				       static_cast<int32_t>(numof_warehouses_) and bo.cnt_under_construction_ ==
				    0)
					prio = 13;

				// take care about borders and enemies
				prio = recalc_with_border_range(*bf, prio);

				// TODO:
				// introduce check that there is no warehouse nearby to prevent to close placing

			} else if (bo.type == BuildingObserver::TRAININGSITE) {
				// build after 20 production sites and then after each 50 production site
				if (static_cast<int32_t>((productionsites.size() + 30) / 50) >
				       bo.total_count() and bo.cnt_under_construction_ ==
				    0)
					prio = 4;

				// take care about borders and enemies
				prio = recalc_with_border_range(*bf, prio);
			}

			// think of space consuming buildings nearby like farms or vineyards
			prio -= bf->space_consumers_nearby_ * 10;

			// Stop here, if priority is 0 or less.
			if (prio <= 0)
				continue;

			// Prefer road side fields
			prio += bf->preferred_ ? 1 : 0;
			// don't waste good land for small huts
			prio -= (maxsize - bo.desc->get_size()) * 5;

			if (prio > proposed_priority) {
				best_building = &bo;
				proposed_priority = prio;
				proposed_coords = bf->coords;
			}
		}  // ending loop over buildings
	}     // ending loop over fields

	// then try all mines_ - as soon as basic economy is build up.
	if (gametime > next_mine_construction_due_) {

		update_all_mineable_fields(gametime);
		next_mine_construction_due_ = gametime + kIdleMineUpdateInterval;

		if (!mineable_fields.empty()) {

			for (uint32_t i = 0; i < buildings.size() && productionsites.size() > 8; ++i) {
				BuildingObserver& bo = buildings.at(i);

				if (not bo.mines_marble_ and gametime <
				    kBaseInfrastructureTime)  // allow only stone mines_ in early stages of game
					continue;

				if (!bo.buildable(*player) || bo.type != BuildingObserver::MINE)
					continue;

				if (gametime - bo.construction_decision_time_ < kBuildingMinInterval)
					continue;

				// Don't build another building of this type, if there is already
				// one that is unoccupied_ at the moment
				// or under construction
				if ((bo.cnt_under_construction_ + bo.unoccupied_) > 0)
					continue;

				// calculating actual amount of mined raw materials
				if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
					bo.stocklevel_ = get_stocklevel(bo);
					bo.stocklevel_time = game().get_gametime();
				}

				if (kMinesDebug)
					log(" TDEBUG: %1d considering %12s/%1d: stat: %3d(<20), stocklevel_: %2d, count %2d "
					    "/ %2d / %2d\n",
					    player_number(),
					    bo.name,
					    bo.mines_,
					    bo.current_stats_,
					    bo.stocklevel_,
					    bo.total_count(),
					    bo.unoccupied_,
					    bo.cnt_under_construction_);

				// Only try to build mines_ that produce needed wares.
				if (((bo.cnt_built_ - bo.unoccupied_) > 0 and bo.current_stats_ < 20)or bo.stocklevel_ >
				    40 + static_cast<uint32_t>(bo.mines_marble_) * 30) {
					if (kMinesDebug)
						log("  No need to seek for this mine type: stat: %2d, stock :%2d/%2d\n",
						    bo.current_stats_,
						    bo.stocklevel_,
						    40 + bo.mines_marble_ * 30);
					continue;
				}

				// this is penalty if there are existing mines too close
				// it is treated as multiplicator for count of near mines
				uint32_t nearness_penalty = 0;
				if ((bo.cnt_built_ + bo.cnt_under_construction_) == 0)
					nearness_penalty = 0;
				else if (bo.mines_marble_)
					nearness_penalty = 7;
				else
					nearness_penalty = 10;

				// iterating over fields
				for (std::list<MineableField*>::iterator j = mineable_fields.begin();
				     j != mineable_fields.end();
				     ++j) {

					if ((*j)->coords.field->get_resources() != bo.mines_)
						continue;

					int32_t prio = (*j)->coords.field->get_resources_amount();

					// applying nearnes penalty
					prio = prio - (*j)->mines_nearby_ * nearness_penalty;

					if (kMinesDebug)
						log("  TDEBUG: priority of spot at %3d x %3d: %3d (%2d - %2d*%2d) , current "
						    "best: %2d \n",
						    (*j)->coords.x,
						    (*j)->coords.y,
						    prio,
						    (*j)->coords.field->get_resources_amount(),
						    (*j)->mines_nearby_,
						    nearness_penalty,
						    proposed_priority);

					// Only build mines_ on locations where some material can be mined
					if (prio < 2)
						continue;

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
						if (kMinesDebug)
							log("   spot blocked!\n");
						continue;
					}

					if (prio > proposed_priority) {
						// proposed_building = bo.id;
						best_building = &bo;
						proposed_priority = prio;
						proposed_coords = (*j)->coords;
						mine = true;

						if (kMinesDebug)
							log("   TDEBUG: using %-12s as a candidate\n", bo.name);
					}
				}  // end of evaluation of field
			}

		}  // section if mine size >0
	}     // end of mines_ section

	// if there is no winner:
	// if (proposed_building == INVALID_INDEX) {
	if (best_building == nullptr) {
		if (kWinnerDebug)
			log(" TDEBUG:  no building picked up\n");

		return false;
	}

	// send the command to construct a new building
	game().send_player_build(player_number(), proposed_coords, best_building->id);
	BlockedField blocked(
	   game().map().get_fcoords(proposed_coords), game().get_gametime() + 120000);  // two minutes
	blocked_fields.push_back(blocked);

	if (best_building->type != BuildingObserver::MILITARYSITE)
		best_building->construction_decision_time_ = gametime;
	else  // very ugly hack here
		best_building->construction_decision_time_ = gametime - kBuildingMinInterval / 2;

	if (kWinnerDebug)
		log(" TDEBUG:  winning priority %4d, building %2d, coords: %3d x %3d, M: %s\n",
		    proposed_priority,
		    best_building->id,
		    proposed_coords.x,
		    proposed_coords.y,
		    mine ? "Y" : "N");

	// set the type of update that is needed
	if (mine) {
		next_mine_construction_due_ = gametime + kBusyMineUpdateInterval;

		if (kMinesUpdateDebug)
			log(" TDEBUG expanding mine update by: %d,building %2d, coords: %3d x %3d  \n",
			    kBusyMineUpdateInterval,
			    best_building->id,
			    proposed_coords.x,
			    proposed_coords.y);

		if (kMinesUpdateDebug)
			log("  new next_mine_construction_due_=%10d\n", next_mine_construction_due_);

		// m_mineable_changed = true;
	} else
		// last_mine_constr_time=gametime;
		m_buildable_changed = true;

	return true;
}

/**
 * This function searches for places where a new road is needed to connect two
 * economies. It then sends the request to build the road.
 */
bool DefaultAI::construct_roads(int32_t gametime) {
	if (economies.size() < 2) {
		// only one economy, no need for new roads
		return false;
	}

	uint32_t economies_to_connect = 0;
	EconomyObserver* eo_to_connect = economies.front();  // dummy initialisation

	//  fetch first two economies that might be connectable
	for (std::list<EconomyObserver*>::iterator i = economies.begin();
	     economies_to_connect < 2 && i != economies.end();
	     ++i)

		//  Do not try to connect economies that already failed in last time.
		if ((*i)->next_connection_try <= gametime) {
			if (economies_to_connect == 1)
				eo_to_connect = *i;

			++economies_to_connect;
		}

	// No need to connect, if only one economy
	if (economies_to_connect < 2)
		return false;

	if (eo_to_connect->flags.empty())
		return check_economies();

	// Check if the flag is still there and if not care about that situation
	if (!eo_to_connect->flags.front()) {
		eo_to_connect->flags.pop_front();
		return check_economies();
	}

	// Try to connect - this should work fine as in nearly all cases we simply
	// connect a constructionsite
	bool done = connect_flag_to_another_economy(*eo_to_connect->flags.front());
	eo_to_connect->flags.push_back(eo_to_connect->flags.front());
	eo_to_connect->flags.pop_front();

	if (done) {
		eo_to_connect->failed_connection_tries = 0;
		return true;
	}

	// If the economy consists of just one constructionsite, and the defaultAI
	// failed more than 4 times to connect, we remove the constructionsite
	if (eo_to_connect->failed_connection_tries > 3 and eo_to_connect->flags.size() == 1) {
		Building* bld = eo_to_connect->flags.front()->get_building();

		if (bld) {
			BuildingObserver& bo = get_building_observer(bld->name().c_str());

			if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
				game().send_player_bulldoze(*const_cast<Flag*>(eo_to_connect->flags.front()));
				eo_to_connect->flags.pop_front();
				// Block the field at constructionsites coords for 5 minutes
				// against new construction tries.
				BlockedField blocked(
				   game().map().get_fcoords(bld->get_position()), game().get_gametime() + 300000);
				blocked_fields.push_back(blocked);
			}
		}
	}

	// Unable to connect, so we let this economy wait for 30 seconds.
	eo_to_connect->next_connection_try = gametime + 30000;
	++eo_to_connect->failed_connection_tries;
	return false;
}

/// improves current road system
bool DefaultAI::improve_roads(int32_t gametime) {
	// Remove flags of dead end roads, as long as no more wares are stored on them
	container_iterate(std::list<EconomyObserver*>, economies, i)
	container_iterate(std::list<Flag const*>, (*i.current)->flags, j)

	if ((*j.current)->is_dead_end() && (*j.current)->current_wares() == 0) {
		game().send_player_bulldoze(*const_cast<Flag*>((*j.current)));
		j.current = (*i.current)->flags.erase(j.current);
		return true;
	}

	// force a split on roads that are longer than 3 parts
	// actually we do not care for loss of building capabilities - normal maps
	// should have enough space and the computer can expand it's territory.
	if (!roads.empty()) {
		const Path& path = roads.front()->get_path();

		if (path.get_nsteps() > 3) {
			const Map& map = game().map();
			CoordPath cp(map, path);
			// try to split after two steps
			CoordPath::Step_Vector::size_type i = cp.get_nsteps() - 1, j = 1;

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
	}

	if (!economies.empty() && inhibit_road_building_ <= gametime) {
		EconomyObserver* eco = economies.front();

		if (!eco->flags.empty()) {
			bool finish = false;
			const Flag& flag = *eco->flags.front();

			// try to connect to another economy
			if (economies.size() > 1)
				finish = connect_flag_to_another_economy(flag);

			// try to improve the roads at this flag
			//  TODO do this only on useful places - the attempt below
			//  TODO  unfortunatey did not work as it should...
			//  if the flag is full of wares or if it is not yet a fork.
			if (!finish)  //&& (!flag.has_capacity() || flag.nr_of_roads() < 3))
				finish = improve_transportation_ways(flag);

			// cycle through flags one at a time
			eco->flags.push_back(eco->flags.front());
			eco->flags.pop_front();
			// and cycle through economies
			economies.push_back(eco);
			economies.pop_front();
			return finish;
		} else
			// If the economy has no flag, the observers need to be updated.
			return check_economies();
	}

	return false;
}

// connects a specific flag to another economy
bool DefaultAI::connect_flag_to_another_economy(const Flag& flag) {
	FindNodeWithFlagOrRoad functor;
	CheckStepRoadAI check(player, MOVECAPS_WALK, true);
	std::vector<Coords> reachable;
	// first look for possible destinations
	functor.economy = flag.get_economy();
	Map& map = game().map();
	map.find_reachable_fields(
	   Area<FCoords>(map.get_fcoords(flag.get_position()), 16), &reachable, check, functor);

	if (reachable.empty())
		return false;

	// then choose the one with the shortest path
	Path* path = new Path();
	bool found = false;
	check.set_openend(false);
	Coords closest;
	container_iterate_const(std::vector<Coords>, reachable, i) {
		Path* path2 = new Path();

		if (map.findpath(flag.get_position(), *i.current, 0, *path2, check) >= 0) {
			if (!found || path->get_nsteps() > path2->get_nsteps()) {
				delete path;
				path = path2;
				path2 = nullptr;
				closest = *i.current;
				found = true;
			}
		}

		delete path2;
	}

	if (found) {
		// if we join a road and there is no flag yet, build one
		if (dynamic_cast<const Road*>(map[closest].get_immovable()))
			game().send_player_build_flag(player_number(), closest);

		// and finally build the road
		game().send_player_build_road(player_number(), *path);
		return true;
	} else {
		delete path;
		return false;
	}
}

/// adds alternative ways to already existing ones
bool DefaultAI::improve_transportation_ways(const Flag& flag) {
	// First of all try to remove old building flags to clean up the road web if possible
	container_iterate(std::list<Widelands::Coords>, flags_to_be_removed, i) {
		// Maybe the flag was already removed?
		FCoords f = game().map().get_fcoords(*(i.current));

		if (upcast(Flag, other_flag, f.field->get_immovable())) {
			// Check if building is dismantled, but don't waste precious wares
			if (!other_flag->get_building() && other_flag->current_wares() == 0) {
				game().send_player_bulldoze(*other_flag);
				flags_to_be_removed.erase(i.current);
				break;
			}
		} else {
			flags_to_be_removed.erase(i.current);
			break;
		}
	}
	std::priority_queue<NearFlag> queue;
	std::vector<NearFlag> nearflags;
	queue.push(NearFlag(flag, 0, 0));
	Map& map = game().map();

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

			if (!road)
				continue;

			Flag* endflag = &road->get_flag(Road::FlagStart);

			if (endflag == nf.flag)
				endflag = &road->get_flag(Road::FlagEnd);

			int32_t dist = map.calc_distance(flag.get_position(), endflag->get_position());

			if (dist > 12)  //  out of range
				continue;

			queue.push(NearFlag(*endflag, nf.cost_ + road->get_path().get_nsteps(), dist));
		}
	}

	std::sort(nearflags.begin(), nearflags.end(), CompareDistance());
	CheckStepRoadAI check(player, MOVECAPS_WALK, false);

	for (uint32_t i = 1; i < nearflags.size(); ++i) {
		NearFlag& nf = nearflags.at(i);

		if (2 * nf.distance_ + 2 < nf.cost_) {
			Path& path = *new Path();

			if (map.findpath(flag.get_position(), nf.flag->get_position(), 0, path, check) >=
			    0 and static_cast<int32_t>(2 * path.get_nsteps() + 2) < nf.cost_) {
				game().send_player_build_road(player_number(), path);
				return true;
			}

			delete &path;
		}
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

	container_iterate(std::list<EconomyObserver*>, economies, i) {
		// check if any flag has changed its economy
		std::list<Flag const*>& fl = (*i.current)->flags;

		for (std::list<Flag const*>::iterator j = fl.begin(); j != fl.end();) {
			if (&(*i.current)->economy != &(*j)->economy()) {
				get_economy_observer((*j)->economy())->flags.push_back(*j);
				j = fl.erase(j);
			} else
				++j;
		}

		// if there are no more flags in this economy,
		// we no longer need it's observer
		if ((*i.current)->flags.empty()) {
			delete *i.current;
			economies.erase(i.current);
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
bool DefaultAI::check_productionsites(int32_t gametime) {
	if ((next_productionsite_check_due_ > gametime) || productionsites.empty())
		return false;

	next_productionsite_check_due_ = gametime + 5000;

	// Get link to productionsite that should be checked
	ProductionSiteObserver& productionsite_observer = productionsites.front();
	productionsites.pop_front();

	ProductionSite* productionsite = productionsite_observer.site.get(game());
	if (!productionsite) {
		// Site has vanished.
		return false;
	}
	// Push the back for future consideration.
	productionsites.push_back(productionsite_observer);

	bool changed = false;
	// Get max radius of recursive workarea
	Workarea_Info::size_type radius = 0;
	const Workarea_Info& workarea_info = productionsite_observer.bo->desc->m_workarea_info;
	container_iterate_const(Workarea_Info, workarea_info, i)

	if (radius < i.current->first)
		radius = i.current->first;

	Map& map = game().map();

	// do not dismantle same type of building too soon - to give some time to update statistics
	// yes it interferes with building updates, but not big problem here
	if (productionsite_observer.bo->last_dismantle_time_ > game().get_gametime() - 30 * 1000)
		return false;

	// Lumberjack / Woodcutter handling
	if (productionsite_observer.bo->need_trees_) {
		if (map.find_immovables(
		       Area<FCoords>(map.get_fcoords(productionsite->get_position()), radius),
		       nullptr,
		       FindImmovableAttribute(Map_Object_Descr::get_attribute_id("tree"))) < 6) {
			// Do not destruct the last lumberjack - perhaps some small trees are
			// near, a forester will plant some trees or some new trees will seed
			// in reach. Computer player_s can easily run out of wood if this check
			// is not done.
			if (productionsite_observer.bo->cnt_built_ <=
			    3 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 20) {
				if (kWoodDebug)
					log(" TDEBUG: %1d: cutter without trees, but not dismantling due to low numbers of "
					    "cutters (%2d)\n",
					    player_number(),
					    productionsite_observer.bo->cnt_built_);

				return false;
			}

			if (productionsite->get_statistics_percent() <= 20) {
				// destruct the building and it's flag (via flag destruction)
				// the destruction of the flag avoids that defaultAI will have too many
				// unused roads - if needed the road will be rebuild directly.
				// log (" TDEBUG: dismantling lumberjacks hut\n");
				productionsite_observer.bo->last_dismantle_time_ = game().get_gametime();
				flags_to_be_removed.push_back(productionsite->base_flag().get_position());
				game().send_player_dismantle(*productionsite);

				if (kWoodDebug)
					log(" TDEBUG %1d: cutter without trees, dismantling..., remaining cutters: %2d\n",
					    player_number(),
					    productionsite_observer.bo->cnt_built_);

				return true;
			}
			return false;
		}
		return false;
	}

	// Wells handling
	if (productionsite_observer.bo->mines_water_) {
		if (productionsite_observer.built_time_ + 6 * 60 * 1000 <
		       game().get_gametime() and productionsite->get_statistics_percent() ==
		    0) {
			if (kWellDebug)
				log(" TDEBUG: dismantling Well, statistics: %3d,\n",
				    productionsite->get_statistics_percent());
			productionsite_observer.bo->last_dismantle_time_ = game().get_gametime();
			flags_to_be_removed.push_back(productionsite->base_flag().get_position());
			game().send_player_dismantle(*productionsite);

			return true;
		}
		return false;
	}

	// Quarry handling
	if (productionsite_observer.bo->need_stones_) {
		if (kQuarryDismDebug) {
			log(" QUARRY at %3d x %3d: statistics: %3d/%3d, age: %5d(>360s), stones:%3d\n",
			    productionsite->get_position().x,
			    productionsite->get_position().y,
			    productionsite->get_statistics_percent(),
			    productionsite->get_crude_statistics(),
			    (gametime - productionsite_observer.built_time_) / 1000,
			    map.find_immovables(
			       Area<FCoords>(map.get_fcoords(productionsite->get_position()), radius),
			       nullptr,
			       FindImmovableAttribute(Map_Object_Descr::get_attribute_id("stone"))));
		}

		if (map.find_immovables(
		       Area<FCoords>(map.get_fcoords(productionsite->get_position()), radius),
		       nullptr,
		       FindImmovableAttribute(Map_Object_Descr::get_attribute_id("stone"))) == 0) {
			// destruct the building and it's flag (via flag destruction)
			// the destruction of the flag avoids that defaultAI will have too many
			// unused roads - if needed the road will be rebuild directly.
			flags_to_be_removed.push_back(productionsite->base_flag().get_position());
			game().send_player_dismantle(*productionsite);
			return true;
		}

		if (productionsite_observer.built_time_ + 6 * 60 * 1000 <
		       game().get_gametime() and productionsite->get_statistics_percent() ==
		    0) {
			// it is possible that there are stones but quary is not able to mine them
			productionsite_observer.bo->last_dismantle_time_ = game().get_gametime();
			flags_to_be_removed.push_back(productionsite->base_flag().get_position());
			game().send_player_dismantle(*productionsite);

			return true;
		}

		return false;
	}

	// All other SPACE_CONSUMERS without input and above target_count
	if (kSpaceDebug and productionsite_observer.bo->space_consumer_ and not
	       productionsite_observer.bo->plants_trees_)
		log(" TDEBUG: space consumer here: %15s at %3d x %3d: statistics: %3d, age: %5d(>360s)\n",
		    productionsite_observer.bo->name,
		    productionsite->get_position().x,
		    productionsite->get_position().y,
		    productionsite->get_statistics_percent(),
		    (gametime - productionsite_observer.built_time_) / 1000);

	if (productionsite_observer.bo->inputs_.empty()  // does not consume anything
	    and productionsite_observer.bo->production_hint_ ==
	    -1  // not a renewing building (forester...)
	          and productionsite_observer.built_time_ +
	          6 * 60 * 1000 <
	       game().get_gametime()                    // > 10 minutes old
	       and productionsite->can_start_working()  // building is occupied
	       and productionsite_observer.bo->space_consumer_ and not
	          productionsite_observer.bo->plants_trees_) {
		if (productionsite_observer.bo->cnt_built_ > productionsite_observer.bo->cnt_target_) {
			if (productionsite_observer.bo->stocklevel_time < game().get_gametime() - 5 * 1000) {
				productionsite_observer.bo->stocklevel_ = get_stocklevel(*productionsite_observer.bo);
				productionsite_observer.bo->stocklevel_time = game().get_gametime();
			}

			if (kSpaceDebug)
				log(" TDEBUG: considering dismantle of space consumer: %15s, count %2d/T:%2d, stock "
				    "level:%3d(>100)\n",
				    productionsite_observer.bo->name,
				    productionsite_observer.bo->cnt_built_,
				    productionsite_observer.bo->cnt_target_,
				    productionsite_observer.bo->stocklevel_);

			if (productionsite->get_statistics_percent()<
			       95 and productionsite_observer.bo->stocklevel_> 100) {  // production stats == 0%
				productionsite_observer.bo->last_dismantle_time_ = game().get_gametime();
				flags_to_be_removed.push_back(productionsite->base_flag().get_position());
				game().send_player_dismantle(*productionsite);
				return true;
			}
		}

		if (productionsite->get_statistics_percent() <= 20) {
			if (kSpaceDebug)
				log(" TDEBUG: dismantling: %15s at %3d x %3d: due to low performance: %2d\n",
				    productionsite_observer.bo->name,
				    productionsite->get_position().x,
				    productionsite->get_position().y,
				    productionsite->get_statistics_percent());

			flags_to_be_removed.push_back(productionsite->base_flag().get_position());
			game().send_player_dismantle(*productionsite);
			return true;
		}

		return false;
	}

	// buildings with inputs_, checking if we can a dismantle some due to low performance
	if (!productionsite_observer.bo->inputs_.empty()
	    and productionsite_observer.bo->cnt_built_ >= 3
	    and productionsite_observer.bo->current_stats_ < 30) {
		if (kIdleDismantle)
			log(" kIdleDismantle: dismantling due to too many buildings: %15s at %3d x %3d, total "
			    "counts: %2d, stat: %2d\n",
			    productionsite_observer.bo->name,
			    productionsite->get_position().x,
			    productionsite->get_position().y,
			    productionsite_observer.bo->cnt_built_,
			    productionsite_observer.bo->current_stats_);
		productionsite_observer.bo->last_dismantle_time_ = game().get_gametime();
		flags_to_be_removed.push_back(productionsite->base_flag().get_position());
		game().send_player_dismantle(*productionsite);
		return true;
	}

	// supporting productionsites (rangers)
	// stop/start them based on stock avaiable
	if (productionsite_observer.bo->production_hint_ >= 0) {
		if (productionsite_observer.bo->stocklevel_time < game().get_gametime() - 5 * 1000) {
			productionsite_observer.bo->stocklevel_ =
			   get_stocklevel_by_hint(productionsite_observer.bo->production_hint_);
			productionsite_observer.bo->stocklevel_time = game().get_gametime();
		}

		if (kStandbyDebug)
			log("  TDEBUG: standby review: %-16s(%dx):stock level: %3d, status: %s\n",
			    productionsite_observer.bo->name,
			    productionsite_observer.bo->cnt_built_,
			    productionsite_observer.bo->stocklevel_,
			    productionsite->is_stopped() ? "stopped" : "running");

		if (productionsite_observer.bo->stocklevel_ > 200 and productionsite_observer.bo->cnt_built_ >
		    productionsite_observer.bo->cnt_target_) {
			if (kStandbyDebug)
				log("   * dismantling the building\n");

			productionsite_observer.bo->last_dismantle_time_ = game().get_gametime();
			flags_to_be_removed.push_back(productionsite->base_flag().get_position());
			game().send_player_dismantle(*productionsite);
			return true;
		}

		if (productionsite_observer.bo->stocklevel_ > 150 and not productionsite->is_stopped()) {
			if (kStandbyDebug)
				log("   * stopping building\n");

			game().send_player_start_stop_building(*productionsite);
		}

		if (productionsite_observer.bo->stocklevel_ < 100 and productionsite->is_stopped()) {
			if (kStandbyDebug)
				log("   * starting building\n");

			game().send_player_start_stop_building(*productionsite);
		}
	}

	// Upgrading policy
	// a) if there are two buildings and none enhanced, one is enhanced
	// b) if there are two buildings and at least one functional
	// statistics percents are decisive

	// do not upgrade if current building is only one in operation
	if ((productionsite_observer.bo->cnt_built_ - productionsite_observer.bo->unoccupied_) <= 1)
		return false;

	// Check whether building is enhanceable and if wares of the enhanced
	// buildings are needed. If yes consider an upgrade.
	Building_Index enhancement = productionsite->enhancement();
	int32_t maxprio = 0;
	Building_Index enbld = INVALID_INDEX;
	BuildingObserver* bestbld = nullptr;

	// Only enhance buildings that are allowed (scenario mode)
	if (player->is_building_type_allowed(*enhancement.current)) {
		const Building_Descr& bld = *tribe->get_building_descr(*enhancement.current);
		BuildingObserver& en_bo = get_building_observer(bld.name().c_str());

		// do not build the same building so soon (kind of duplicity check)
		if (gametime - en_bo.construction_decision_time_ >= kBuildingMinInterval)
			{
			// Don't enhance this building, if there is already one of same type
			// under construction or unoccupied_
			if (en_bo.cnt_under_construction_ + en_bo.unoccupied_ <= 0)
				{
				// don't upgrade without workers
				if (productionsite->has_workers(*enhancement.current, game()))
					{
					// forcing first upgrade
					if ((en_bo.cnt_under_construction_ + en_bo.cnt_built_ + en_bo.unoccupied_) == 0
						and (productionsite_observer.bo->cnt_built_ -
						productionsite_observer.bo->unoccupied_) >= 1
						and (game().get_gametime() - productionsite_observer.built_time_) > 30 * 60 * 1000
						and !mines_.empty()) {

						if (kUpgradeDebug)
							log(" UPGRADE: upgrading (forcing as first) %12s at %3d x %3d: age %d min.\n",
							productionsite_observer.bo->name,
							productionsite->get_position().x,
							productionsite->get_position().y,
							(game().get_gametime() - productionsite_observer.built_time_) / 60000);

						game().send_player_enhance_building(*productionsite, (*enhancement.current));
						return true;
					}
				}
			}
		}

		// now, let consider normal upgrade

		if (kUpgradeDebug)
			log(" UPGRADE: %1d: working enhanced buildings (%15s): %1d, statitistics: %2d\n",
				player_number(),
				en_bo.name,
				en_bo.cnt_built_ - en_bo.unoccupied_,
				en_bo.current_stats_);

		// do not upgrade if candidate production % is too low
		if ((en_bo.cnt_built_ - en_bo.unoccupied_) !=
			0 or(en_bo.cnt_under_construction_ + en_bo.unoccupied_) <= 0 or en_bo.current_stats_ >= 50)
			{

			int32_t prio = 0;

			if (en_bo.current_stats_ > 65) {
				prio = en_bo.current_stats_ -
				       productionsite_observer.bo->current_stats_;  // priority for enhancement
				prio += en_bo.current_stats_ - 65;

				if (kUpgradeDebug)
					log(" UPGRADE:  proposing upgrade (non-first building) %12s at %3d x %3d: prio: "
					    "%2d, target statistics: %2d\n",
					    productionsite_observer.bo->name,
					    productionsite->get_position().x,
					    productionsite->get_position().y,
					    prio,
					    en_bo.current_stats_);
			}

			if (prio > maxprio) {
				maxprio = prio;
				enbld = (*enhancement.current);
				bestbld = &en_bo;
			}
		}
	}

	// Enhance if enhanced building is useful
	// additional: we dont want to lose the old building
	if (maxprio > 0) {
		if (kUpgradeDebug)
			log(" UPGRADE:   upgrading %15s(as non first)\n", bestbld->name);

		game().send_player_enhance_building(*productionsite, enbld);
		bestbld->construction_decision_time_ = gametime;
		changed = true;
	}

	return changed;
}

/**
 * checks the first mine in list, takes care if it runs out of
 * resources and finally reenqueues it at the end of the list.
 *
 * \returns true, if something was changed.
 */
bool DefaultAI::check_mines_(int32_t const gametime) {
	if ((next_mine_check_due_ > gametime) || mines_.empty())
		return false;

	next_mine_check_due_ = gametime + 10000;  // 10 seconds is enough
	// also statistics must be recalculated
	// Get link to productionsite that should be checked
	ProductionSiteObserver& productionsite_observer = mines_.front();
	mines_.pop_front();

	ProductionSite* mine = productionsite_observer.site.get(game());
	if (!mine) {
		// Building has vanished.
		return false;
	}

	Map& map = game().map();
	Field* field = map.get_fcoords(mine->get_position()).field;
	// Reorder and set new values; - due to returns within the function
	mines_.push_back(productionsite_observer);

	if (kMinesUpdateDebug)
		log(" MINES_UPDATE: %1d: reviewing %-15s at %3dx%3d, statistics: %3d, left resources: %2d\n",
		    player_number(),
		    productionsite_observer.bo->name,
		    mine->get_position().x,
		    mine->get_position().y,
		    mine->get_statistics_percent(),
		    field->get_resources_amount());

	// It takes some time till performance gets to 0
	// so I use 40% as a limit to check if there are some resources left
	if (mine->get_statistics_percent() > 40)
		return false;

	// Check if mine ran out of resources
	uint8_t current = field->get_resources_amount();

	if (current < 1) {
		// destruct the building and it's flag (via flag destruction)
		// the destruction of the flag avoids that defaultAI will have too many
		// unused roads - if needed the road will be rebuild directly.
		flags_to_be_removed.push_back(mine->base_flag().get_position());
		game().send_player_dismantle(*mine);

		if (kMinesUpdateDebug)
			log(" MINES_UPDATE: Dismantling...\n");

		return true;
	}

	// Check whether building is enhanceable. If yes consider an upgrade.
	std::set<Building_Index> enhancements = mine->enhancements();
	int32_t maxprio = 0;
	Building_Index enbld = INVALID_INDEX;
	BuildingObserver* bestbld = nullptr;
	bool changed = false;
	container_iterate_const(std::set<Building_Index>, enhancements, x) {
		// Only enhance buildings that are allowed (scenario mode)
		if (player->is_building_type_allowed(*x.current)) {
			// first exclude possibility there are enhancements in construction or unoccupied_
			const Building_Descr& bld = *tribe->get_building_descr(*x.current);
			BuildingObserver& en_bo = get_building_observer(bld.name().c_str());

			if (kMinesUpdateDebug)
				log(" MINES_UPDATE:   considering upgrade to %15s, count B:%1d(stat:%3d)  U:%1d  "
				    "C:%1d\n",
				    en_bo.name,
				    en_bo.cnt_built_,
				    en_bo.current_stats_,
				    en_bo.unoccupied_,
				    en_bo.cnt_under_construction_);

			if (en_bo.unoccupied_ + en_bo.cnt_under_construction_ > 0)
				continue;

			// do not upgrade target building are not working properly (probably do not have food)
			if (en_bo.cnt_built_ > 0 and en_bo.current_stats_ < 60)
				continue;

			// do not build the same building so soon (kind of duplicity check)
			if (gametime - en_bo.construction_decision_time_ < kBuildingMinInterval)
				continue;

			// Check if mine needs an enhancement to mine more resources
			uint8_t const until = field->get_starting_res_amount() *
			                      (100 - productionsite_observer.bo->mines_percent_) / 100;

			if (kMinesUpdateDebug)
				log(" MINES_UPDATE:    until:%3d ?>, current: %3d\n", until, current);

			if (until >= current) {
				// add some randomness - just for the case if more than one
				// enhancement is available (not in any tribe yet)
				int32_t const prio = time(nullptr) % 3 + 1;

				if (prio > maxprio) {
					maxprio = prio;
					enbld = (*x.current);
					bestbld = &en_bo;

					if (kMinesUpdateDebug)
						log(" MINES_UPDATE:    ..is candidate\n");
				}
			}
		}
	}

	// Enhance if enhanced building is useful
	if (maxprio > 0) {
		game().send_player_enhance_building(*mine, enbld);
		bestbld->construction_decision_time_ = gametime;
		changed = true;

		if (kMinesUpdateDebug)
			log(" MINES_UPDATE:    ..enhancing\n");
	}

	//// Reorder and set new values;
	// mines_.push_back(mines_.front());
	// mines_.pop_front();
	return changed;
}

// this count ware as hints
uint32_t DefaultAI::get_stocklevel_by_hint(size_t hintoutput) {
	uint32_t count = 0;
	Ware_Index wt(hintoutput);
	container_iterate(std::list<EconomyObserver*>, economies, l) {
		// Don't check if the economy has no warehouse.
		if ((*l.current)->economy.warehouses().empty())
			continue;

		count += (*l.current)->economy.stock_ware(wt);
	}

	if (kStockDebug)
		log(" TDEBUG: stock : %3u for hint: %2" PRIuS ", time: %3d\n",
		    count,
		    hintoutput,
		    game().get_gametime() / 1000);

	return count;
}

// this count all stock for all output
uint32_t DefaultAI::get_stocklevel(BuildingObserver& bo) {
	uint32_t count = 0;

	if (!bo.outputs_.empty()) {
		container_iterate(std::list<EconomyObserver*>, economies, l) {
			// Don't check if the economy has no warehouse.
			if ((*l.current)->economy.warehouses().empty())
				continue;

			for (uint32_t m = 0; m < bo.outputs_.size(); ++m) {
				Ware_Index wt(static_cast<size_t>(bo.outputs_.at(m)));
				count += (*l.current)->economy.stock_ware(wt);
			}
		}
	}

	if (kStockDebug)
		log(" TDEBUG: stock : %3d for building: %s, time: %3d\n",
		    count,
		    bo.name,
		    game().get_gametime() / 1000);

	return count;
}

/**
 * Updates the first military building in list and reenques it at the end of
 * the list afterwards. If a militarysite is in secure area but holds more than
 * one soldier, the number of stationed soldiers is decreased. If the building
 * is near a border, the number of stationed soldiers is maximized
 *
 * \returns true if something was changed
 */
bool DefaultAI::check_militarysites(int32_t gametime) {
	if (next_militarysite_check_due_ > gametime)
		return false;

	// just to be sure the value is reset
	next_militarysite_check_due_ = gametime + 5 * 1000;  // 10 seconds is really fine
	// even if there are no finished & attended military sites, probably there are ones just in
	// construction
	unstationed_milit_buildings_ = 0;

	for (std::list<MilitarySiteObserver>::iterator it = militarysites.begin();
	     it != militarysites.end();
	     ++it)
		if (it->site->stationedSoldiers().size() == 0)
			unstationed_milit_buildings_ += 1;

	// count militarysites in construction
	military_under_constr_ = 0;

	for (uint32_t j = 0; j < buildings.size(); ++j) {
		BuildingObserver& bo = buildings.at(j);

		if (bo.type == BuildingObserver::MILITARYSITE)
			military_under_constr_ += bo.cnt_under_construction_;
	}

	// Only useable, if defaultAI owns at least one militarysite
	if (militarysites.empty())
		return false;

	// Check next militarysite
	bool changed = false;
	Map& map = game().map();
	MilitarySite* ms = militarysites.front().site;
	uint32_t const vision = ms->vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	// look if there is any enemy land nearby
	FindNodeEnemy find_enemy(player, game());

	// first if there are enemies nearby
	if (map.find_fields(Area<FCoords>(f, vision), nullptr, find_enemy) == 0) {
		// If no enemy in sight - decrease the number of stationed soldiers
		// as long as it is > 1 - BUT take care that there is a warehouse in the
		// same economy where the thrown out soldiers can go to.
		if (ms->economy().warehouses().size()) {
			uint32_t const j = ms->soldierCapacity();

			if (MilitarySite::kPrefersRookies != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersRookies);
			} else if (j > 1)
				game().send_player_change_soldier_capacity(*ms, -1);
			// if the building is in inner land and other militarysites still
			// hold the miliary influence of the field, consider to destruct the
			// building to free some building space.
			else {
				// treat this field like a buildable and write military info to it.
				BuildableField bf(f);
				update_buildable_field(bf, vision, true);
				const int32_t size_penalty = ms->get_size() - 1;

				if (kMilDismDebug)
					log(" DISMANTLE CHECK: testing finished building %3dx%3d, capacity: %2d(>9), "
					    "presence: %2d(>3), loneliness: %4d(<160), stationed: %1d(>2+%d as size "
					    "penalty)\n",
					    f.x,
					    f.y,
					    bf.military_capacity_,
					    bf.military_presence_,
					    bf.military_loneliness_,
					    bf.military_stationed_,
					    size_penalty);

				if (bf.military_capacity_ > 9 and bf.military_presence_ >
				    3 and bf.military_loneliness_<160 and bf.military_stationed_>(2 + size_penalty)) {
					if (kMilDismDebug)
						log(" DISMANTLE CHECK:  * dismantling the building on %5d, last dismantle: %5d\n",
						    game().get_gametime() / 1000,
						    military_last_dismantle_ / 1000);

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
		// If an enemy is in sight and the number of stationed soldier is not
		// at maximum - set it to maximum.
		uint32_t const j = ms->maxSoldierCapacity();
		uint32_t const k = ms->soldierCapacity();

		if (j > k)
			game().send_player_change_soldier_capacity(*ms, j - k);

		if (MilitarySite::kPrefersHeroes != ms->get_soldier_preference())
			game().send_player_militarysite_set_soldier_preference(*ms, MilitarySite::kPrefersHeroes);

		changed = true;
	}

	// reorder:;
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();
	next_militarysite_check_due_ = gametime + 5 * 1000;  // 10 seconds is really fine
	return changed;
}

/**
 * This function takes care about the unowned and opposing territory and
 * recalculates the priority for none military buildings depending on the
 * initialisation type of a defaultAI
 *
 * \arg bf   = BuildableField to be checked
 * \arg prio = priority until now.
 *
 * \returns the recalculated priority
 */
int32_t DefaultAI::recalc_with_border_range(const BuildableField& bf, int32_t prio) {
	// Prefer building space in the inner land.
	prio /= (1 + (bf.unowned_land_nearby_ / 4));

	if (bf.unowned_land_nearby_ > 15)
		prio -= (bf.unowned_land_nearby_ - 15);

	// Especially places near the frontier to the enemies are unlikely
	//  NOTE take care about the type of computer player_. The more
	//  NOTE aggressive a computer player_ is, the more important is
	//  NOTE this check. So we add \var type as bonus.
	if (bf.enemy_nearby_ and prio > 0)
		prio /= (3 + type);

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

	if (bo.inputs_.empty())
		prio += 4;

	int32_t output_prio = 0;

	for (uint32_t k = 0; k < bo.outputs_.size(); ++k) {
		WareObserver& wo = wares.at(bo.outputs_.at(k));

		if (wo.consumers_ > 0) {
			output_prio += wo.preciousness_;
			output_prio += wo.consumers_ * 2;
			output_prio -= wo.producers_ * 2;

			if (bo.total_count() == 0)
				output_prio += 10;  // add a big bonus
		}
	}

	if (bo.outputs_.size() > 1)
		output_prio =
		   static_cast<int32_t>(ceil(output_prio / sqrt(static_cast<double>(bo.outputs_.size()))));

	prio += 2 * output_prio;

	// If building consumes some wares, multiply with current statistics of all
	// other buildings of this type to avoid constructing buildings where already
	// some are running on low resources.
	// Else at least add a part of the stats t the calculation.
	if (!bo.inputs_.empty()) {
		prio *= bo.current_stats_;
		prio /= 100;
	} else
		prio = ((prio * bo.current_stats_) / 100) + (prio / 2);

	return prio;
}

void DefaultAI::consider_productionsite_influence(BuildableField& field,
                                                  Coords coords,
                                                  const BuildingObserver& bo) {
	if (bo.space_consumer_ and game().map().calc_distance(coords, field.coords) < 4)
		++field.space_consumers_nearby_;

	for (size_t i = 0; i < bo.inputs_.size(); ++i)
		++field.consumers_nearby_.at(bo.inputs_.at(i));

	for (size_t i = 0; i < bo.outputs_.size(); ++i)
		++field.producers_nearby_.at(bo.outputs_.at(i));
}

/// \returns the economy observer containing \arg economy
EconomyObserver* DefaultAI::get_economy_observer(Economy& economy) {
	for (std::list<EconomyObserver*>::iterator i = economies.begin(); i != economies.end(); ++i)
		if (&(*i)->economy == &economy)
			return *i;

	economies.push_front(new EconomyObserver(economy));
	return economies.front();
}

/// \returns the building observer
BuildingObserver& DefaultAI::get_building_observer(char const* const name) {
	if (tribe == nullptr)
		late_initialization();

	for (uint32_t i = 0; i < buildings.size(); ++i)
		if (!strcmp(buildings.at(i).name, name))
			return buildings.at(i);

	throw wexception("Help: I do not know what to do with a %s", name);
}

/// this is called whenever we gain ownership of a PlayerImmovable
void DefaultAI::gain_immovable(PlayerImmovable& pi) {
	if (upcast(Building, building, &pi))
		gain_building(*building);
	else if (upcast(Flag const, flag, &pi))
		new_flags.push_back(flag);
	else if (upcast(Road const, road, &pi))
		roads.push_front(road);
}

/// this is called whenever we lose ownership of a PlayerImmovable
void DefaultAI::lose_immovable(const PlayerImmovable& pi) {
	if (upcast(Building const, building, &pi))
		lose_building(*building);
	else if (upcast(Flag const, flag, &pi)) {
		container_iterate_const(std::list<EconomyObserver*>, economies, i)
		container_iterate(std::list<Flag const*>, (*i.current)->flags, j)

		if (*j.current == flag) {
			(*i.current)->flags.erase(j.current);
			return;
		}

		container_iterate(std::list<Flag const*>, new_flags, i)

		if (*i.current == flag) {
			new_flags.erase(i.current);
			return;
		}
	} else if (upcast(Road const, road, &pi))
		roads.remove(road);
}

/// this is called whenever we gain a new building
void DefaultAI::gain_building(Building& b) {
	BuildingObserver& bo = get_building_observer(b.name().c_str());

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver& target_bo =
		   get_building_observer(ref_cast<ConstructionSite, Building>(b).building().name().c_str());
		++target_bo.cnt_under_construction_;
		++total_constructionsites;
		// Let defaultAI try to directly connect the constructionsite
		next_road_due_ = game().get_gametime();
	} else {
		++bo.cnt_built_;

		if (bo.type == BuildingObserver::PRODUCTIONSITE) {
			productionsites.push_back(ProductionSiteObserver());
			productionsites.back().site = &ref_cast<ProductionSite, Building>(b);
			productionsites.back().bo = &bo;
			productionsites.back().built_time_ = game().get_gametime();
			productionsites.back().stats_zero_ = 0;

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				++wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				++wares.at(bo.inputs_.at(i)).consumers_;
		} else if (bo.type == BuildingObserver::MINE) {
			mines_.push_back(ProductionSiteObserver());
			mines_.back().site = &ref_cast<ProductionSite, Building>(b);
			mines_.back().bo = &bo;

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				++wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				++wares.at(bo.inputs_.at(i)).consumers_;
		} else if (bo.type == BuildingObserver::MILITARYSITE) {
			militarysites.push_back(MilitarySiteObserver());
			militarysites.back().site = &ref_cast<MilitarySite, Building>(b);
			militarysites.back().bo = &bo;
			militarysites.back().checks = bo.desc->get_size();
		} else if (bo.type == BuildingObserver::WAREHOUSE)
			++numof_warehouses_;
	}
}

/// this is called whenever we lose a building
void DefaultAI::lose_building(const Building& b) {
	BuildingObserver& bo = get_building_observer(b.name().c_str());

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver& target_bo = get_building_observer(
		   ref_cast<ConstructionSite const, Building const>(b).building().name().c_str());
		--target_bo.cnt_under_construction_;
		--total_constructionsites;
	} else {
		--bo.cnt_built_;

		if (bo.type == BuildingObserver::PRODUCTIONSITE) {
			for (std::list<ProductionSiteObserver>::iterator i = productionsites.begin();
			     i != productionsites.end();
			     ++i)
				if (i->site.get(game()) == &b) {
					productionsites.erase(i);
					break;
				}

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				--wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				--wares.at(bo.inputs_.at(i)).consumers_;
		} else if (bo.type == BuildingObserver::MINE) {
			for (std::list<ProductionSiteObserver>::iterator i = mines_.begin(); i != mines_.end();
			     ++i)
				if (i->site.get(game()) == &b) {
					mines_.erase(i);
					break;
				}

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				--wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				--wares.at(bo.inputs_.at(i)).consumers_;
		} else if (bo.type == BuildingObserver::MILITARYSITE) {
			for (std::list<MilitarySiteObserver>::iterator i = militarysites.begin();
			     i != militarysites.end();
			     ++i)
				if (i->site == &b) {
					militarysites.erase(i);
					break;
				}
		} else if (bo.type == BuildingObserver::WAREHOUSE) {
			assert(numof_warehouses_ > 0);
			--numof_warehouses_;
		}
	}

	m_buildable_changed = true;
	m_mineable_changed = true;
}

/// Checks that supply line exists for given building.
/// Recurcsively verify that all inputs_ have a producer.
// TODO: this function leads to periodic freezes of ~1 second on big games on my system.
// TODO: It needs profiling and optimization.
// NOTE: This is not needed anymore and it seems it is not missed neither
bool DefaultAI::check_supply(const BuildingObserver& bo) {
	size_t supplied = 0;
	container_iterate_const(std::vector<int16_t>, bo.inputs_, i)
	container_iterate_const(std::vector<BuildingObserver>, buildings, j)

	if (j.current->cnt_built_ &&
	    std::find(j.current->outputs_.begin(), j.current->outputs_.end(), *i.current) !=
	       j.current->outputs_.end() &&
	    check_supply(*j.current)) {
		++supplied;
		break;
	}

	return supplied == bo.inputs_.size();
}

/**
 * The defaultAi "considers" via this function whether to attack an
 * enemy, if opposing military buildings are in sight. In case of an attack it
 * sends all available forces.
 *
 * \returns true, if attack was started.
 */
bool DefaultAI::consider_attack(int32_t const gametime) {
	// Only useable, if it owns at least one militarysite
	if (militarysites.empty())
		return false;

	Map& map = game().map();
	uint16_t const pn = player_number();
	// Check next militarysite
	MilitarySite* ms = militarysites.front().site;
	uint32_t const vision = ms->vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	Building* target = ms;  // dummy initialisation to silence the compiler
	int32_t chance = 0;
	uint32_t attackers = 0;
	// Search in a radius of the vision of the militarysite and collect
	// information about immovables in the area
	std::vector<ImmovableFound> immovables;
	map.find_immovables(Area<FCoords>(f, vision), &immovables, FindImmovableAttackable());

	for (uint32_t j = 0; j < immovables.size(); ++j)
		if (upcast(MilitarySite, bld, immovables.at(j).object)) {
			if (!player->is_hostile(bld->owner()))
				continue;

			if (bld->canAttack()) {
				int32_t ta = player->findAttackSoldiers(bld->base_flag());

				if (type == NORMAL)
					ta = ta * 2 / 3;

				if (ta < 1)
					continue;

				int32_t const tc = ta - bld->presentSoldiers().size();

				if (tc > chance) {
					target = bld;
					chance = tc;
					attackers = ta;
				}
			}
		} else if (upcast(Warehouse, wh, immovables.at(j).object)) {
			if (!player->is_hostile(wh->owner()))
				continue;

			if (wh->canAttack()) {
				int32_t ta = player->findAttackSoldiers(wh->base_flag());

				if (ta < 1)
					continue;

				// extra priority push!
				int32_t tc = ta * 2;

				if (tc > chance) {
					target = wh;
					chance = tc;
					attackers = ta;
				}
			}
		}

	// Reenque militarysite at the end of list
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();

	// Return if chance to win is too low
	if (chance < 3) {
		next_attack_consideration_due_ = gametime % 7 * 1000 + gametime;
		return false;
	}

	// Attack the selected target.
	game().send_player_enemyflagaction(target->base_flag(), pn, attackers);
	//  Do not attack again too soon - returning soldiers must get healed first.
	next_attack_consideration_due_ = (gametime % 51 + 10) * 1000 + gametime;
	return true;
}
