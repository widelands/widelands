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
#include "logic/findbob.h"
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

// following is in miliseconds (widelands counts time in ms)
constexpr int kFieldUpdateInterval = 1000;
constexpr int kIdleMineUpdateInterval = 22000;
constexpr int kBusyMineUpdateInterval = 2000;
// building of the same building can be started after 25s at earliest
constexpr int kBuildingMinInterval = 25 * 1000;
constexpr int kBaseInfrastructureTime = 20 * 60 * 1000;
// buildings marked as is_food_basic will be forced after 15 minutes, even though their outputs are
// not needed yet
constexpr int kPrimaryFoodStartTime = 15 * 60 * 1000;

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
     military_last_build_(0),
     spots_(0)  {
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

	// considering attack
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
	if (note.lg == LOSE) {

		lose_immovable(*note.pi);
	} else
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
		bo.is_food_basic_ = false;
		bo.is_buildable_ = bld.is_buildable();
		bo.need_trees_ = bh.is_logproducer();
		bo.need_stones_ = bh.is_stoneproducer();
		bo.mines_marble_ = bh.is_marbleproducer();
		bo.need_water_ = bh.get_needs_water();
		bo.mines_water_ = bh.mines_water();
		bo.recruitment_ = bh.for_recruitment();
		bo.space_consumer_ = bh.is_space_consumer();
		bo.expansion_type_ = bh.is_expansion_type();
		bo.fighting_type_ = bh.is_fighting_type();
		bo.mountain_conqueror_ = bh.is_mountain_conqueror();
		if (char const* const s = bh.get_renews_map_resource()) {
			bo.production_hint_ = tribe->safe_ware_index(s);

		}

		// I just presume cut wood is named "log" in the game
		if (tribe->safe_ware_index("log") == bo.production_hint_)
			bo.plants_trees_ = true;
		else
			bo.plants_trees_ = false;

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
			bo.is_food_basic_ = bh.is_food_basic();
			bo.prod_build_material_ = bh.prod_build_material();

			// here we identify hunters
			if (bo.outputs_.size() == 1 and tribe->safe_ware_index("meat") == bo.outputs_.at(0)) {
				bo.is_hunter_ = true;
			} else
				bo.is_hunter_ = false;

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
	const World& world = game().world();
	field.unowned_land_nearby_ =
	   map.find_fields(Area<FCoords>(field.coords, range), nullptr, find_unowned);

	field.near_border_ = false;
	if (field.unowned_land_nearby_ > 0) {
		if (map.find_fields(Area<FCoords>(field.coords, 4), nullptr, find_unowned) > 0)
			field.near_border_ = true;
	}

	// to save some CPU
	if (mines_.size() > 8 and game().get_gametime() % 3 > 0)
		field.unowned_mines_pots_nearby_ = 0;
	else
		field.unowned_mines_pots_nearby_ = map.find_fields(
		   Area<FCoords>(field.coords, range + 4),
		   nullptr,
		   find_unowned_mines_pots);  //+2: a mine can mine raw materials from some range

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
		std::vector<Coords> resource_list;
		std::vector<Bob*> critters_list;

		if (field.water_nearby_ == -1) {  //-1 means "value has never been calculated"
			FindNodeWater find_water(game().world());
			map.find_fields(Area<FCoords>(field.coords, 6), &water_list, find_water);
			field.water_nearby_ = water_list.size();
		}

		// counting fields with fish
		if (field.water_nearby_ > 0 and game().get_gametime() % 10 == 0) {
			map.find_fields(Area<FCoords>(field.coords, 6),
			                &resource_list,
			                FindNodeResource(world.get_resource("fish")));
			field.fish_nearby_ = resource_list.size();
		}

		// counting fields with critters (game)
		// not doing this always, this does not change fast
		if (game().get_gametime() % 10 == 0) {
			map.find_bobs(Area<FCoords>(field.coords, 6), &critters_list, FindBobCritter());
			field.critters_nearby_ = critters_list.size();
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

		// ground water is not renewable and its amount can only fall, we will count them only if
		// previous state si nonzero
		if (field.ground_water_ > 0) {
			field.ground_water_ = field.coords.field->get_resources_amount();
		}
	}

	// folowing is done allways (regardless of military or not)

	field.military_stationed_ = 0;
	field.military_in_constr_nearby_ =0; 
	field.military_capacity_ = 0;
	field.military_loneliness_ =1000; 
	field.military_presence_=0; 


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

					field.military_capacity_ += militarysite->maxSoldierCapacity();
					field.military_presence_ += militarysite->stationedSoldiers().size();

					if (!militarysite->stationedSoldiers().empty()) {
						field.military_stationed_ += 1;
					}
					else
						//the name does not match much
						field.military_in_constr_nearby_ += 1;

					field.military_loneliness_ *= static_cast<double_t>(dist) / radius;
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
		assert(productionsites.front().bo->cnt_built_ > 0);
		// Add statistics value
		productionsites.front().bo->current_stats_ +=
		   productionsites.front().site->get_crude_statistics();

		// Check whether this building is completely occupied
		productionsites.front().bo->unoccupied_ |= !productionsites.front().site->can_start_working();

		// Now reorder the buildings
		productionsites.push_back(productionsites.front());
		productionsites.pop_front();
	}

	// for mines_ also
	// Check all available productionsites
	for (uint32_t i = 0; i < mines_.size(); ++i) {
		assert(mines_.front().bo->cnt_built_ > 0);
		// Add statistics value
		mines_.front().bo->current_stats_ += mines_.front().site->get_statistics_percent();
		// Check whether this building is completely occupied
		mines_.front().bo->unoccupied_ |= !mines_.front().site->can_start_working();
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
// - buildings identified as 'direct food supplier' as built after 15 min.
//   from game start
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
	uint32_t consumers_nearby_count = 0;
	// this is to increase score so also building near borders can be built
	int32_t bulgarian_constant = 12;
	std::vector<int32_t> spots_avail;
	spots_avail.resize(4);
	// uint16_t const pn = player_number();

	for (int32_t i = 0; i < 4; ++i)
		spots_avail.at(i) = 0;

	for (std::list<BuildableField*>::iterator i = buildable_fields.begin();
	     i != buildable_fields.end();
	     ++i)
		++spots_avail.at((*i)->coords.field->nodecaps() & BUILDCAPS_SIZEMASK);

	spots_ = spots_avail.at(BUILDCAPS_SMALL);
	spots_ += spots_avail.at(BUILDCAPS_MEDIUM);
	spots_ += spots_avail.at(BUILDCAPS_BIG);

	// checking amount of free spots, if needed setting new building stop flag
	new_buildings_stop_ = false;

	if ((militarysites.size() * 2 + 20) <
	    productionsites.size()
	    or spots_<(3 + (static_cast<int32_t>(productionsites.size()) / 5))or total_constructionsites>(
	       (militarysites.size() + productionsites.size()) / 2)) {
		new_buildings_stop_ = true;
	}

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

	}

	if (unstationed_milit_buildings_ + military_under_constr_ / 3 > 2 * treshold) {
		near_enemy_b_buildings_stop = true;

	}

	// here we deal with situation when for some time no new military building was built
	// in fact this is a last time when there were any military buildings in construction
	if ((unstationed_milit_buildings_ + military_under_constr_) > 0)
		military_last_build_ = game().get_gametime();

	if (military_last_build_ + 1 * 60 * 1000 < game().get_gametime()) {
		military_boost = 200;
	}

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

			if (not(bo.type == BuildingObserver::MILITARYSITE) and bo.cnt_under_construction_ >= 2)
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

			int32_t prio = 0;  // score of a bulding on a field

			// if current field is not big enough
			if (bo.desc->get_size() > maxsize)
				continue;

			if (bo.type == BuildingObserver::PRODUCTIONSITE) {

				// exclude spots on border
				if (bf->near_border_ and not bo.need_trees_ and not bo.need_stones_)
					continue;

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
					prio = bf->ground_water_ - 2;
					prio = recalc_with_border_range(*bf, prio);

				} else if (bo.need_trees_) {  // LUMBERJACS

					if (bo.cnt_built_ + bo.cnt_under_construction_ + bo.unoccupied_ <= 2)
						prio = bulgarian_constant + 200 + bf->trees_nearby_;
					else if (bo.cnt_under_construction_ + bo.unoccupied_ <= 1) {
						prio =
						   bf->trees_nearby_ - 5 - bf->producers_nearby_.at(bo.outputs_.at(0)) * 5 -
						   new_buildings_stop_ * 15;  //+ bf->producers_nearby_.at(bo.outputs_.at(0))*5;
					}

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
						prio = prio * 5;

					// to prevent to many quaries on one spot
					prio = prio - 50 * bf->producers_nearby_.at(bo.outputs_.at(0));

				} else if (bo.production_hint_ >= 0) {
					// first setting targets (needed also for dismantling)
					if (bo.plants_trees_)
						bo.cnt_target_ =
						   2 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 15;
					else
						bo.cnt_target_ =
						   1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 20;

					if ((bo.cnt_under_construction_ + bo.unoccupied_) > 1)
						continue;

					// production hint (f.e. associate forester with logs)

					if (bo.need_water_ and bf->water_nearby_ < 5)  // probably some of them needs water
						continue;

					if (bo.plants_trees_) {  // RANGERS

						// if there are too many trees nearby
						if (bf->trees_nearby_ > 25 and bo.total_count() >= 2)
							continue;

						// sometimes all area is blocked by trees so this is to prevent this
						if (buildable_fields.size() < 4)
							continue;

						// prevent too many rangers
						if (bo.total_count() * 3 > static_cast<int32_t>(productionsites.size()))
							continue;

						if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
							bo.stocklevel_ =
							   get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
							bo.stocklevel_time = game().get_gametime();
						}
						prio = 0;
						// if we need wood
						if (bo.stocklevel_ < 50)
							prio =
							   (50 - bo.stocklevel_) + bf->producers_nearby_.at(bo.production_hint_) * 5;

						// if we just need some rangers to be on safe side
						if (bo.total_count() < 2)
							prio += (60 - bf->trees_nearby_) * 3 +
							        bf->producers_nearby_.at(bo.production_hint_) * 5;
						else if (bo.total_count() < bo.cnt_target_)
							prio += 30 + bf->producers_nearby_.at(bo.production_hint_) * 5;

					} else if (gametime > kBaseInfrastructureTime and not
					                         new_buildings_stop_) {  // gamekeepers or so
						if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
							bo.stocklevel_ =
							   get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint_));
							bo.stocklevel_time = game().get_gametime();
						}

						// especially for fish breeders
						if (bo.need_water_)
							prio = bf->water_nearby_;

						if (bo.total_count() == 0)
							prio += 5;

						if (bo.total_count() < bo.cnt_target_) {
							prio += bf->producers_nearby_.at(bo.production_hint_) * 10;
							prio += recalc_with_border_range(*bf, prio);

						} else if (bo.stocklevel_ < 50 and not new_buildings_stop_) {
							prio += bf->producers_nearby_.at(bo.production_hint_) * 5;
							prio += recalc_with_border_range(*bf, prio);  // only for not wood producers_
						} else
							continue;
					}

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

					// if hunter and too little critters nearby skipping
					if (bo.is_hunter_ and bf->critters_nearby_ < 5)
						continue;
					// similarly for fishers
					if (bo.need_water_ and bf->fish_nearby_ <= 1)
						continue;

					// first eliminate buildings needing water if there is short supplies
					if (bo.need_water_ and bf->water_nearby_ < 4)
						continue;

					if (bo.is_basic_ and bo.total_count() == 0)
						prio = 150 + max_preciousness;
					else if (bo.is_food_basic_ and game().get_gametime() >
					            kPrimaryFoodStartTime and bo.total_count() ==
					         0) {
						prio = 40 + max_preciousness;
					} else if (game().get_gametime() <
					           kBaseInfrastructureTime or
					              new_buildings_stop_)  // leave 15 minutes for basic infrastructure only
						continue;
					else if ((bo.is_basic_ and bo.total_count() <=
					          1)or(output_is_needed and bo.total_count() == 0))
						prio = 80 + max_preciousness;
					else if (bo.inputs_.size() == 0) {
						bo.cnt_target_ =
						   1 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 8;

						if (bo.cnt_built_ >
						    bo.cnt_target_ and not(
						       bo.space_consumer_ or bo.is_food_basic_))  // spaceconsumers_ and basic_s
							                                               // can be built more then target
							continue;

						if (bo.stocklevel_time < game().get_gametime() - 5 * 1000) {
							bo.stocklevel_ = get_stocklevel(bo);
							bo.stocklevel_time = game().get_gametime();
						}

						if (bo.stocklevel_ < 50) {
							prio = max_preciousness + bulgarian_constant;

							if (bo.space_consumer_)  // need to consider trees nearby
								prio += 20 - (bf->trees_nearby_ / 3);

							if (not bo.space_consumer_)
								prio -= bf->producers_nearby_.at(bo.outputs_.at(0)) *
								        20;  // leave some free space between them

							if (bo.cnt_built_ < 2)
								prio += 5;

							prio = recalc_with_border_range(*bf, prio);

							if (bo.stocklevel_ < 20)
								prio += 20 - bo.stocklevel_;

							// fisher
							if (bo.need_water_) {
								prio += bf->fish_nearby_ - 4;
							}

							// hunters
							if (bo.is_hunter_) {
								prio += (bf->critters_nearby_ * 2) - 8;
							}

						}
					} else if (!bo.inputs_.empty()) {
						// to have two buildings from everything (intended for upgradeable buildings)
						// but I do not know how to identify such buildings
						if (bo.cnt_built_ == 1 and game().get_gametime() >
						                        60 * 60 * 1000 and bo.desc->enhancements().size() >
						                        0 and !mines_.empty()) {
							prio = max_preciousness + bulgarian_constant;
						}
						// if output is needed and there are no idle buildings
						else if (output_is_needed) {
							if (bo.cnt_built_ > 0 and bo.current_stats_ > 80) {
								prio = max_preciousness + bulgarian_constant + 30;

							} else if (bo.cnt_built_ > 0 and bo.current_stats_ > 55) {
								prio = max_preciousness + bulgarian_constant;

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

				if (new_military_buildings_stop and not bf->enemy_nearby_)
					continue;

				if (near_enemy_b_buildings_stop and bf->enemy_nearby_)
					continue;

				if (bf->enemy_nearby_ and bo.fighting_type_)
					;  // it is ok, go on
				else if (bf->unowned_mines_pots_nearby_ >
				         0 and(bo.mountain_conqueror_ or bo.expansion_type_))
					;  // it is ok, go on
				else if (bf->unowned_land_nearby_ and bo.expansion_type_) {
					// decreasing probability for big buidlings
					if (bo.desc->get_size() == 2 and gametime % 5 >= 1)
						continue;
					if (bo.desc->get_size() == 3 and gametime % 15 >= 1)
						continue;
				}
				// it is ok, go on
				else
					continue;  // the building is not suitable for situation

				if (bo.desc->get_size() ==
				    3 and game().get_gametime() <
				       15 * 60 * 1000)  // do not built fortresses in first half of hour of game
					continue;

				if (!bf->unowned_land_nearby_)
					continue;

				// not to build so many military buildings nearby
				if (!bf->enemy_nearby_ and bf->military_in_constr_nearby_ > 0)
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

				if (bo.desc->get_size() < maxsize)
					prio = prio - 5;  // penalty

				if (bf->enemy_nearby_ and bf->military_capacity_ < 12) {
					prio += 100;
				}

			} else if (bo.type == BuildingObserver::WAREHOUSE) {

				// exclude spots on border
				if (bf->near_border_)
					continue;

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

				// exclude spots on border
				if (bf->near_border_)
					continue;

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

				// Only try to build mines_ that produce needed wares.
				if (((bo.cnt_built_ - bo.unoccupied_) > 0 and bo.current_stats_ < 20)or bo.stocklevel_ >
				    40 + static_cast<uint32_t>(bo.mines_marble_) * 30) {

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

						continue;
					}

					if (prio > proposed_priority) {
						// proposed_building = bo.id;
						best_building = &bo;
						proposed_priority = prio;
						proposed_coords = (*j)->coords;
						mine = true;

					}
				}  // end of evaluation of field
			}

		}  // section if mine size >0
	}     // end of mines_ section

	// if there is no winner:
	// if (proposed_building == INVALID_INDEX) {
	if (best_building == nullptr) {

		mine = false;
		return false;
	}

	// send the command to construct a new building
	game().send_player_build(player_number(), proposed_coords, best_building->id);
	BlockedField blocked(
	   game().map().get_fcoords(proposed_coords), game().get_gametime() + 120000);  // two minutes
	blocked_fields.push_back(blocked);

	// we block also nearby fields
	//if farms and so on, for quite a long time
	//if military sites only for short time for AI can update information on near buildable fields
	if ( (best_building->space_consumer_ and not best_building->plants_trees_) or 
		best_building->type == BuildingObserver::MILITARYSITE) {
		uint32_t block_time=0;
		uint32_t block_area=0;		
		if (best_building->space_consumer_){
			block_time=45 * 60 * 1000;
			block_area=3;}
		else {//militray buildings for a very short time
			block_time=25 * 1000;
			block_area=6;}
		Map& map = game().map();

		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(map.get_fcoords(proposed_coords), block_area));
		do {
			BlockedField blocked2(
			   map.get_fcoords(*(mr.location().field)), game().get_gametime() + block_time);
			blocked_fields.push_back(blocked2);
		} while (mr.advance(map));
	}

	if (not(best_building->type == BuildingObserver::MILITARYSITE))
		best_building->construction_decision_time_ = gametime;
	else  // very ugly hack here
		best_building->construction_decision_time_ = gametime - kBuildingMinInterval / 2;

	// set the type of update that is needed
	if (mine) {
		next_mine_construction_due_ = gametime + kBusyMineUpdateInterval;

	} else
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

// improves current road system
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
	// with exemption when there is too few building spots
	if (spots_>20 and !roads.empty()) {
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

			// try to improve the roads at this flag, effectively
			// to build a 'shortcut' from the flag, if reasonable
			//  second arguments means:
			// 'FORCE a shortcut from here as the flag is full of wares'
			if (!finish)  
				finish = improve_transportation_ways(flag,!flag.has_capacity() );

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

// adds alternative ways to already existing ones
// (shortcuts if the road would be shortened significantly)
bool DefaultAI::improve_transportation_ways(const Flag& flag, const bool force) {
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
	//shortcut is made (attempted) if  (current_road/possible_shortcut)>mindiff
	const uint16_t mindif=4; 
	
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

			if (dist > 13)  //  out of range
				continue;

			queue.push(NearFlag(*endflag, nf.cost_ + road->get_path().get_nsteps(), dist));
		}
	}

	//usually we create consider shortest shortcut, but sometimes
	//we seek biggest reduction
	if (game().get_gametime()%5>0)
		std::sort(nearflags.begin(), nearflags.end(), CompareDistance());
	else
		std::sort(nearflags.begin(), nearflags.end(), CompareShortening());
		
	CheckStepRoadAI check(player, MOVECAPS_WALK, false);

	//doing only 3 attempts we presume that if the first one fails, road is not buildable at all
	for (uint32_t i = 1; i < nearflags.size() and i < 4; ++i) {
		NearFlag& nf = nearflags.at(i);

		//one of three condidtions must be met:
		// mindif is excessed
		// absolute difference (current_road-shortcut) >20
		// or the shortcut is forced (due to wares on flag)
		if ( (mindif*(nf.distance_ + 1) < nf.cost_) or  (nf.cost_-nf.distance_)>20  or force) { //was 2
			
			Path& path = *new Path();

			if (map.findpath(flag.get_position(), nf.flag->get_position(), 0, path, check) >=
			    0 ){
					if ( (!force and static_cast<int32_t>(mindif * (path.get_nsteps() + 1)) < nf.cost_) 
					or (force)){
						game().send_player_build_road(player_number(), path);
						return true;
				}
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

	next_productionsite_check_due_ = gametime + 4000;

	bool changed = false;
	// Reorder and set new values; - better now because there are multiple returns in the function
	productionsites.push_back(productionsites.front());
	productionsites.pop_front();

	// Get link to productionsite that should be checked
	ProductionSiteObserver& site = productionsites.front();

	// first we werify if site is working yet (can be unoccupied since the start)
	if (!site.site->can_start_working()) {
		site.unoccupied_till_ = game().get_gametime();
	}

	// Get max radius of recursive workarea
	Workarea_Info::size_type radius = 0;
	const Workarea_Info& workarea_info = site.bo->desc->m_workarea_info;
	container_iterate_const(Workarea_Info, workarea_info, i)

	if (radius < i.current->first)
		radius = i.current->first;

	Map& map = game().map();

	// do not dismantle same type of building too soon - to give some time to update statistics
	// yes it interferes with building updates, but not big problem here
	if (site.bo->last_dismantle_time_ > game().get_gametime() - 30 * 1000)
		return false;

	// Lumberjack / Woodcutter handling
	if (site.bo->need_trees_) {
		if (map.find_immovables(Area<FCoords>(map.get_fcoords(site.site->get_position()), radius),
		                        nullptr,
		                        FindImmovableAttribute(Map_Object_Descr::get_attribute_id("tree"))) <
		    6) {
			// Do not destruct the last lumberjack - perhaps some small trees are
			// near, a forester will plant some trees or some new trees will seed
			// in reach. Computer player_s can easily run out of wood if this check
			// is not done.
			if (site.bo->cnt_built_ <=
			    3 + static_cast<int32_t>(mines_.size() + productionsites.size()) / 20) {

				return false;
			}

			if (site.site->get_statistics_percent() <= 20) {
				// destruct the building and it's flag (via flag destruction)
				// the destruction of the flag avoids that defaultAI will have too many
				// unused roads - if needed the road will be rebuild directly.
				// log (" TDEBUG: dismantling lumberjacks hut\n");
				site.bo->last_dismantle_time_ = game().get_gametime();
				flags_to_be_removed.push_back(site.site->base_flag().get_position());
				game().send_player_dismantle(*site.site);

				return true;
			}
			return false;
		}
		return false;
	}

	// Wells handling
	if (site.bo->mines_water_) {
		if (site.unoccupied_till_ + 6 * 60 * 1000 < game().get_gametime()
		                                            and site.site->get_statistics_percent() ==
		    0) {
			site.bo->last_dismantle_time_ = game().get_gametime();
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			game().send_player_dismantle(*site.site);

			return true;
		}
		return false;
	}

	// Quarry handling
	if (site.bo->need_stones_) {

		if (map.find_immovables(
		       Area<FCoords>(map.get_fcoords(site.site->get_position()), radius),
		       nullptr,
		       FindImmovableAttribute(Map_Object_Descr::get_attribute_id("stone"))) == 0) {
			// destruct the building and it's flag (via flag destruction)
			// the destruction of the flag avoids that defaultAI will have too many
			// unused roads - if needed the road will be rebuild directly.
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			game().send_player_dismantle(*site.site);
			return true;
		}

		if (site.unoccupied_till_ + 6 * 60 * 1000 < game().get_gametime()
		                                            and site.site->get_statistics_percent() ==
		    0) {
			// it is possible that there are stones but quary is not able to mine them
			site.bo->last_dismantle_time_ = game().get_gametime();
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			game().send_player_dismantle(*site.site);

			return true;
		}

		return false;
	}

	// All other SPACE_CONSUMERS without input and above target_count
	if (site.bo->inputs_.empty()  // does not consume anything
	    and site.bo->production_hint_ ==
	    -1  // not a renewing building (forester...)
	          and site.unoccupied_till_ +
	          10 * 60 * 1000 <
	       game().get_gametime()               // > 10 minutes old
	       and site.site->can_start_working()  // building is occupied
	       and site.bo->space_consumer_ and not site.bo->plants_trees_) {

		// if we have more buildings then target
		if (site.bo->cnt_built_ > site.bo->cnt_target_) {
			if (site.bo->stocklevel_time < game().get_gametime() - 5 * 1000) {
				site.bo->stocklevel_ = get_stocklevel(*site.bo);
				site.bo->stocklevel_time = game().get_gametime();
			}

			if (site.site->get_statistics_percent()<
			       30 and site.bo->stocklevel_> 100) {  // production stats == 0%
				site.bo->last_dismantle_time_ = game().get_gametime();
				flags_to_be_removed.push_back(site.site->base_flag().get_position());
				game().send_player_dismantle(*site.site);
				return true;
			}
		}

		// regardless of count of sites a building can be dismanteld if it performs too bad
		if (site.site->get_statistics_percent() <= 10) {

			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			game().send_player_dismantle(*site.site);
			return true;
		}

		return false;
	}

	// buildings with inputs_, checking if we can a dismantle some due to low performance
	if (!site.bo->inputs_.empty() and(site.bo->cnt_built_ - site.bo->unoccupied_) >=
	    3 and site.site->can_start_working() and site.site->get_statistics_percent() <
	    20 and                             // statistics for the building
	       site.bo->current_stats_<30 and  // overall statistics
	                               (game().get_gametime() - site.unoccupied_till_)> 10 *
	       60 * 1000) {

		site.bo->last_dismantle_time_ = game().get_gametime();
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		game().send_player_dismantle(*site.site);
		return true;
	}

	// remaining buildings without inputs and not supporting ones (fishers only left probably and
	// huters)
	// first if is only for log, second one is "executive"

	if (site.bo->inputs_.size() ==
	    0 and site.bo->production_hint_ <
	       0 and site.site->can_start_working()
	       and not site.bo->space_consumer_ and site.site->get_statistics_percent() <
	       10 and((game().get_gametime() - site.built_time_) > 10 * 60 * 1000)) {

		site.bo->last_dismantle_time_ = game().get_gametime();
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		game().send_player_dismantle(*site.site);
		return true;
	}

	// supporting productionsites (rangers)
	// stop/start them based on stock avaiable
	if (site.bo->production_hint_ >= 0) {

		if (site.bo->stocklevel_time < game().get_gametime() - 5 * 1000) {
			site.bo->stocklevel_ = get_stocklevel_by_hint(site.bo->production_hint_);
			site.bo->stocklevel_time = game().get_gametime();
		}

		uint16_t score = site.bo->stocklevel_;


		if (score > 150 and site.bo->cnt_built_ > site.bo->cnt_target_) {

			site.bo->last_dismantle_time_ = game().get_gametime();
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			game().send_player_dismantle(*site.site);
			return true;
		}

		if (score > 70 and not site.site->is_stopped()) {

			game().send_player_start_stop_building(*site.site);
		}

		if (score < 50 and site.site->is_stopped()) {

			game().send_player_start_stop_building(*site.site);
		}
	}

	// Upgrading policy
	// a) if there are two buildings and none enhanced, one is enhanced
	// b) if there are two buildings and at least one functional
	// statistics percents are decisive

	// do not upgrade if current building is only one in operation
	if ((site.bo->cnt_built_ - site.bo->unoccupied_) <= 1)
		return false;

	// Check whether building is enhanceable and if wares of the enhanced
	// buildings are needed. If yes consider an upgrade.
	std::set<Building_Index> enhancements = site.site->enhancements();
	int32_t maxprio = 0;
	Building_Index enbld = INVALID_INDEX;  // to get rid of this
	BuildingObserver* bestbld = nullptr;
	container_iterate_const(std::set<Building_Index>, enhancements, x) {
		// Only enhance buildings that are allowed (scenario mode)
		if (player->is_building_type_allowed(*x.current)) {
			const Building_Descr& bld = *tribe->get_building_descr(*x.current);
			BuildingObserver& en_bo = get_building_observer(bld.name().c_str());

			// do not build the same building so soon (kind of duplicity check)
			if (gametime - en_bo.construction_decision_time_ < kBuildingMinInterval)
				continue;

			// Don't enhance this building, if there is already one of same type
			// under construction or unoccupied_
			if (en_bo.cnt_under_construction_ + en_bo.unoccupied_ > 0)
				continue;

			// don't upgrade without workers
			if (!site.site->has_workers(*x.current, game()))
				continue;

			// forcing first upgrade
			if ((en_bo.cnt_under_construction_ + en_bo.cnt_built_ + en_bo.unoccupied_) ==
			    0 and(site.bo->cnt_built_ - site.bo->unoccupied_) >=
			       1 and(game().get_gametime() - site.unoccupied_till_) >
			       30 * 60 * 1000 and !mines_.empty()) {

				game().send_player_enhance_building(*site.site, (*x.current));
				return true;
			}

			// now, let consider normal upgrade

			// do not upgrade if candidate production % is too low
			if ((en_bo.cnt_built_ - en_bo.unoccupied_) ==
			    0 or(en_bo.cnt_under_construction_ + en_bo.unoccupied_) > 0 or en_bo.current_stats_ <
			       50)
				continue;

			int32_t prio = 0;

			if (en_bo.current_stats_ > 65) {
				prio = en_bo.current_stats_ - site.bo->current_stats_;  // priority for enhancement
				prio += en_bo.current_stats_ - 65;

			}

			if (prio > maxprio) {
				maxprio = prio;
				enbld = (*x.current);
				bestbld = &en_bo;
			}
		}
	}

	// Enhance if enhanced building is useful
	// additional: we dont want to lose the old building
	if (maxprio > 0) {
		game().send_player_enhance_building(*site.site, enbld);
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
	// Reorder and set new values; - due to returns within the function
	mines_.push_back(mines_.front());
	mines_.pop_front();
	// also statistics must be recalculated
	// Get link to productionsite that should be checked
	ProductionSiteObserver& site = mines_.front();
	Map& map = game().map();
	Field* field = map.get_fcoords(site.site->get_position()).field;

	// first get rid of mines that are missing workers for some time (5 minutes)
	// released worker (if any) can be usefull elsewhere !
	if (site.built_time_ + 5 * 60 * 1000 < gametime and not site.site->can_start_working()) {
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		game().send_player_dismantle(*site.site);
		return true;
	}

	// It takes some time till performance gets to 0
	// so I use 40% as a limit to check if there are some resources left
	if (site.site->get_statistics_percent() > 40)
		return false;

	// Check if mine ran out of resources
	uint8_t current = field->get_resources_amount();

	if (current < 1) {
		// destruct the building and it's flag (via flag destruction)
		// the destruction of the flag avoids that defaultAI will have too many
		// unused roads - if needed the road will be rebuild directly.
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		game().send_player_dismantle(*site.site);

		return true;
	}

	// Check whether building is enhanceable. If yes consider an upgrade.
	std::set<Building_Index> enhancements = site.site->enhancements();
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

			if (en_bo.unoccupied_ + en_bo.cnt_under_construction_ > 0)
				continue;

			// do not upgrade target building are not working properly (probably do not have food)
			if (en_bo.cnt_built_ > 0 and en_bo.current_stats_ < 60)
				continue;

			// do not build the same building so soon (kind of duplicity check)
			if (gametime - en_bo.construction_decision_time_ < kBuildingMinInterval)
				continue;

			// Check if mine needs an enhancement to mine more resources
			uint8_t const until =
			   field->get_starting_res_amount() * (100 - site.bo->mines_percent_) / 100;

			if (until >= current) {
				// add some randomness - just for the case if more than one
				// enhancement is available (not in any tribe yet)
				int32_t const prio = time(nullptr) % 3 + 1;

				if (prio > maxprio) {
					maxprio = prio;
					enbld = (*x.current);
					bestbld = &en_bo;

				}
			}
		}
	}

	// Enhance if enhanced building is useful
	if (maxprio > 0) {
		game().send_player_enhance_building(*site.site, enbld);
		bestbld->construction_decision_time_ = gametime;
		changed = true;

	}

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
	// FindNodeEnemy find_enemy(player, game());
	// look if there is any enemies building
	FindNodeEnemiesBuilding find_enemy(player, game());

	// first if there are enemies nearby, check for buildings not land
	if (map.find_fields(Area<FCoords>(f, vision + 4), nullptr, find_enemy) == 0) {
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

				int16_t score=0;  // <<=========
				score+=(bf.military_capacity_ > 9);
				score+=(bf.military_presence_ > 3);
				score+=(bf.military_loneliness_ < 160);
				score+=(bf.military_stationed_ > (2+size_penalty));	

				if (score >=3) {

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

		//TODO (tiborb): verify this part of code

		if (j > k)
			// game().send_player_change_soldier_capacity(*ms, j - k);

			if (MilitarySite::kPrefersHeroes != ms->get_soldier_preference())
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersHeroes);

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

// \returns the building observer
BuildingObserver& DefaultAI::get_building_observer(char const* const name) {
	if (tribe == nullptr)
		late_initialization();

	for (uint32_t i = 0; i < buildings.size(); ++i)
		if (!strcmp(buildings.at(i).name, name))
			return buildings.at(i);

	throw wexception("Help: I do not know what to do with a %s", name);
}

// this is called whenever we gain ownership of a PlayerImmovable
void DefaultAI::gain_immovable(PlayerImmovable& pi) {
	if (upcast(Building, building, &pi))
		gain_building(*building);
	else if (upcast(Flag const, flag, &pi))
		new_flags.push_back(flag);
	else if (upcast(Road const, road, &pi))
		roads.push_front(road);
}

// this is called whenever we lose ownership of a PlayerImmovable
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

// this is called whenever we gain a new building
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
			productionsites.back().unoccupied_till_ = game().get_gametime();
			productionsites.back().stats_zero_ = 0;

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				++wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				++wares.at(bo.inputs_.at(i)).consumers_;
		} else if (bo.type == BuildingObserver::MINE) {
			mines_.push_back(ProductionSiteObserver());
			mines_.back().site = &ref_cast<ProductionSite, Building>(b);
			mines_.back().bo = &bo;
			mines_.back().built_time_ = game().get_gametime();

			for (uint32_t i = 0; i < bo.outputs_.size(); ++i)
				++wares.at(bo.outputs_.at(i)).producers_;

			for (uint32_t i = 0; i < bo.inputs_.size(); ++i)
				++wares.at(bo.inputs_.at(i)).consumers_;
		} else if (bo.type == BuildingObserver::MILITARYSITE) {
			militarysites.push_back(MilitarySiteObserver());
			militarysites.back().site = &ref_cast<MilitarySite, Building>(b);
			militarysites.back().bo = &bo;
			militarysites.back().checks = bo.desc->get_size();
			militarysites.back().enemies_nearby = true;
		} else if (bo.type == BuildingObserver::WAREHOUSE)
			++numof_warehouses_;
	}
}

// this is called whenever we lose a building
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
				if (i->site == &b) {
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
				if (i->site == &b) {
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

// Checks that supply line exists for given building.
// Recurcsively verify that all inputs_ have a producer.
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

	// First we iterate over all players and define which ones (if any)
	// are attackable (comparing overal strength)
	// counting players in game
	uint32_t plr_in_game = 0;
	std::vector<bool> player_attackable;
	Player_Number const nr_players = game().map().get_nrplayers();
	player_attackable.resize(nr_players);
	bool any_attackable = false;
	bool any_attacked = false;
	uint16_t const pn = player_number();

	// defining treshold ratio of own_strenght/enemy's_strength
	uint32_t treshold_ratio = 100;
	if (type == AGGRESSIVE)
		treshold_ratio = 80;
	if (type == DEFENSIVE)
		treshold_ratio = 120;

	iterate_players_existing_novar(p, nr_players, game())++ plr_in_game;

	// receiving games statistics and parsing it (reading latest entry)
	const Game::General_Stats_vector& genstats = game().get_general_statistics();
	for (uint8_t j = 1; j <= plr_in_game; ++j) {
		if (pn == j) {
			player_attackable[j - 1] = false;
			continue;
		}

		if (genstats[j - 1].miltary_strength.back() == 0) {
			// to avoid improbable zero division
			player_attackable[j - 1] = true;
			any_attackable = true;
		} else if ((genstats[pn - 1].miltary_strength.back() * 100 /
		            genstats[j - 1].miltary_strength.back()) > treshold_ratio) {
			player_attackable[j - 1] = true;
			any_attackable = true;
		} else {
			player_attackable[j - 1] = false;
		}

	}

	if (not any_attackable) {
		next_attack_consideration_due_ = 120 * 1000 + (gametime % 30 + 2) * 1000 + gametime;
		return false;
	}

	// the logic of attacking is to pick n military buildings - random ones
	// and test them for possible attack
	const uint16_t attempts = militarysites.size() / 6 + 1;
	Map& map = game().map();

	uint16_t position = 0;
	for (uint32_t i = 0; i < attempts && not any_attacked; ++i) {
		position = (game().get_gametime() + (3 * i)) % militarysites.size();

		// picking random military sites
		// using gametime as a random value, but it is constant so each next is on position +3
		// iterating over fields
		// (std::vector would be much better here)

		std::list<MilitarySiteObserver>::iterator mso = militarysites.begin();
		std::advance(mso, position);

		MilitarySite* ms = mso->site;
		Building* target = ms;  // dummy initialisation to silence the compiler
		uint32_t const vision = ms->vision_range();
		FCoords f = map.get_fcoords(ms->get_position());
		int32_t chance = 0;
		uint32_t attackers = 0;
		uint32_t defenders = 0;
		uint32_t defend_ready_enemies = 0;  // enemy soldiers that can come to defend the attacked
		                                    // building (one soldier has to stay)
		// uint8_t retreat = ms->owner().get_retreat_percentage();

		// skipping if based on  "enemies nearby" there are probably no enemies nearby
		if (not mso->enemies_nearby and gametime % 8 > 0) {
			continue;  // go on with next attempt
		}

		// setting as default
		mso->enemies_nearby = false;

		// Search in a radius of the vision of the militarysite and collect
		// information about immovables in the area
		std::vector<ImmovableFound> immovables;
		map.find_immovables(Area<FCoords>(f, vision), &immovables, FindImmovableAttackable());

		for (uint32_t j = 0; j < immovables.size(); ++j) {
			if (upcast(MilitarySite, bld, immovables.at(j).object)) {
				if (!player->is_hostile(bld->owner()))
					continue;

				mso->enemies_nearby = true;

				if (not player_attackable[bld->owner().player_number() - 1]) {
					continue;
				}

				if (bld->canAttack()) {

					// any_attackable_building=true;

					int32_t ta = player->findAttackSoldiers(bld->base_flag());

					if (type == NORMAL)
						ta = ta * 2 / 3;

					if (ta < 1)
						continue;

					int32_t const tc = ta - bld->presentSoldiers().size();

					if (bld->presentSoldiers().size() > 1)
						defend_ready_enemies += bld->presentSoldiers().size() - 1;

					if (tc > chance) {
						target = bld;
						chance = tc;
						attackers = ta;
						defenders = bld->presentSoldiers().size();
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

					// we presume that there are no soldiers in warehouse
					// after long fights this tend to be true :)

					if (tc > chance) {
						target = wh;
						chance = tc;
						attackers = ta;
						defenders = 0;
					}
				}
			}

			// here we consider enemy soldiers in near buildings.
			int32_t penalty;
			if (defend_ready_enemies > 0)
				penalty = (defenders * 100) / 5 * (defend_ready_enemies * 100) / 10 * 10 / 100;
			else
				penalty = 0;

			// Return if chance to win is too low
			if ((chance - penalty / 100) < 2) {
				continue;
			}

			game().send_player_enemyflagaction(target->base_flag(), pn, attackers);

			any_attacked = true;
			break;
		}
	}

	//  Do not attack again too soon - returning soldiers must get healed first.
	if (any_attacked)
		next_attack_consideration_due_ = (gametime % 40 + 10) * 1000 + gametime;
	else {
		next_attack_consideration_due_ = (gametime % 80 + 10) * 1000 + gametime;
	}

	if (any_attacked)
		return true;
	else {
		return false;
	}
}
