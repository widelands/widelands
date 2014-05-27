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
/**
 * Default AI
 */
/* *
 * Comment by TB
 * This is strongly modified AI, as by now not merged to widelands trunk
 * As long as it is not merged to trunk any changes here are owned by me :) 
 * formatted witch: astyle -f -xe --style=java --indent=tab
 */
 
#include "ai/defaultai.h"

#include <algorithm>
#include <ctime>
#include <queue>
#include <typeinfo>

#include "ai/ai_hints.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "log.h"
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
#include "logic/world.h"
#include "profile/profile.h"
#include "upcast.h"

//#include <valgrind/callgrind.h> //remove

#define FIELD_UPDATE_INTERVAL 1000
#define IDLE_MINE_UPDATE_INTERVAL 22000
#define BUSY_MINE_UPDATE_INTERVAL 2000
#define BUILDING_MIN_INTERVAL 25*1000 // building of the same building can be started after 25s at earliest
#define MILITARY_DEBUG 		false
#define MIL_DISM_DEBUG 		false
#define MILIT_SCORE_DEBUG	false
#define PRODUCTION_DEBUG 	false
#define HINT_DEBUG 			false
//#define ENABLE_CALLGRIND 	false
#define WINNER_DEBUG 		false
#define NEW_BUILDING_DEBUG 	false
#define STANDBY_DEBUG 		false
#define MINES_DEBUG 		false
#define UPGRADE_DEBUG 		false
#define STOCK_DEBUG			false
#define MINES_UPDATE_DEBUG	false
#define WOOD_DEBUG 			false
#define SPACE_DEBUG			false

using namespace Widelands;

DefaultAI::AggressiveImpl DefaultAI::aggressiveImpl;
DefaultAI::NormalImpl DefaultAI::normalImpl;
DefaultAI::DefensiveImpl DefaultAI::defensiveImpl;

/// Constructor of DefaultAI
DefaultAI::DefaultAI(Game & ggame, Player_Number const pid, uint8_t const t) :
	Computer_Player              (ggame, pid),
	type                         (t),
	m_buildable_changed          (true),
	m_mineable_changed           (true),
	player                       (nullptr),
	tribe                        (nullptr),
	total_constructionsites      (0),
	next_road_due                (2000),
	next_stats_update_due        (30000),
	next_construction_due        (1000),
	next_mine_construction_due   (0),
	next_productionsite_check_due(0),
	next_mine_check_due          (0),
	next_militarysite_check_due  (0),
	next_attack_consideration_due(300000),
	next_helpersites_check_due   (180000),
	inhibit_road_building        (0),
	time_of_last_construction    (0),
	numof_warehouses             (0),
	new_buildings_stop	         (false),
	unstationed_milit_buildings  (0),
	military_under_constr        (0),
	military_last_dismantle      (0),
	military_last_build          (0)
{}

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
 * Main loop of computer player "defaultAI"
 *
 * General behaviour is defined here.
 */
void DefaultAI::think () {
	//if (ENABLE_CALLGRIND and game().get_gametime()>70*60*1000) {
	//CALLGRIND_START_INSTRUMENTATION;
	//printf (" Valgrind profiling in progress: %10d/%10d!\n",game().get_gametime(),75*60*1000);}
	//if (ENABLE_CALLGRIND and game().get_gametime()>75*60*1000) {
	//CALLGRIND_STOP_INSTRUMENTATION;
	//printf (" Valgrind profiling ended!\n");
	//exit(0);
	//}

	//printf (" TDEBUG: last dismantle: %d\n",military_last_dismantle);
	if (tribe == nullptr)
		late_initialization ();

	const int32_t gametime = game().get_gametime();

	//printf (" next_mine_construction_due: %10d / %10d\n",next_mine_construction_due,gametime);

	if (m_buildable_changed) {
		// update statistics about buildable fields
		update_all_buildable_fields(gametime);
	}

	//if (m_mineable_changed) {
	//// do the same for mineable fields
	//update_all_mineable_fields(gametime);
	//}
	m_buildable_changed = false;
	//m_mineable_changed = false;

	// if there are more than one economy try to connect them with a road.
	if (next_road_due <= gametime) {
		next_road_due = gametime + 1000;

		if (construct_roads (gametime)) {
			m_buildable_changed = true;
			//m_mineable_changed = true;
			return;
		}
	} else
		// only go on, after defaultAI tried to connect all economies.
		return;

	// NOTE Because of the check above, the following parts of think() are used
	// NOTE only once every second at maximum. This increases performance and as
	// NOTE human players can not even react that fast, it should not be a
	// NOTE disadvantage for the defaultAI.
	// This must be checked every time as changes of bobs in AI area aren't
	// handled by the AI itself.
	update_all_not_buildable_fields();

	// IF defaultAI is AGGRESSIVE - we definitely should consider to attack as
	// often as possible.
	if (type == AGGRESSIVE)
		if (next_attack_consideration_due <= gametime)
			consider_attack(gametime);

	// check if anything in the economies changed.
	// This needs to be done before new buildings are placed, to ensure that no
	// empty economy is left.
	if (check_economies())
		return;

	// Before thinking about a new construction, update current stats, to have
	// a better view on current economy.
	if (next_stats_update_due <= gametime)
		update_productionsite_stats(gametime);

	// Now try to build something if possible
	if (next_construction_due <= gametime) {
		next_construction_due = gametime + 2000;

		if (construct_building(gametime)) {
			time_of_last_construction = gametime;
			return;
		}
	}

	// verify that our production sites are doing well
	if (check_productionsites(gametime))
		return;

	// Check the mines and consider upgrading or destroying one
	if (check_mines(gametime))
		return;

	// consider whether a change of the soldier capacity of some militarysites
	// would make sense.
	if (check_militarysites(gametime))
		return;

	//  Finally consider military actions if defaultAI type is Aggressive or
	// Normal.
	if (!(type == DEFENSIVE))
		if (next_attack_consideration_due <= gametime)
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
		inhibit_road_building = gametime + 2500;
		return;
	}
}

/// called by Widelands game engine when an immovable changed
void DefaultAI::receive(const NoteImmovable & note) {
	if (note.lg == LOSE)
		lose_immovable(*note.pi);
	else
		gain_immovable(*note.pi);
}

/// called by Widelands game engine when a field changed
void DefaultAI::receive(const NoteFieldPossession & note) {
	if (note.lg == GAIN)
		unusable_fields.push_back(note.fc);
}


/**
 * Cares for all variables not initialised during construction
 *
 * When DefaultAI is constructed, some information is not yet available (e.g.
 * world), so this is done after complete loading of the map.
 */
void DefaultAI::late_initialization () {
	player = game().get_player(player_number());
	NoteReceiver<NoteImmovable>::connect(*player);
	NoteReceiver<NoteFieldPossession>::connect(*player);
	tribe = &player->tribe();
	log ("ComputerPlayer(%d): initializing (%u)\n", player_number(), type);
	Ware_Index const nr_wares = tribe->get_nrwares();
	wares.resize(nr_wares);

	for (Ware_Index i = 0; i < nr_wares; ++i) {
		wares.at(i).producers    = 0;
		wares.at(i).consumers    = 0;
		wares.at(i).preciousness = tribe->get_ware_descr(i)->preciousness();
	}

	// collect information about the different buildings our tribe can construct
	Building_Index const nr_buildings = tribe->get_nrbuildings();
	const World & world = game().map().world();

	for (Building_Index i = 0; i < nr_buildings; ++i) {
		const Building_Descr & bld = *tribe->get_building_descr(i);
		const std::string & building_name = bld.name();
		const BuildingHints & bh = bld.hints();
		buildings.resize (buildings.size() + 1);
		BuildingObserver & bo     = buildings.back();
		bo.name                   = building_name.c_str();
		bo.id                     = i;
		bo.desc                   = &bld;
		bo.type                   = BuildingObserver::BORING;
		bo.cnt_built              = 0;
		bo.cnt_under_construction = 0;
		bo.stocklevel			  = 0;
		bo.stocklevel_time	    	=0;
		bo.last_dismantle_time		=0;
		bo.construction_decision_time =0;
		bo.production_hint        = -1;
		bo.current_stats          = 0;
		bo.unoccupied             = false;
		bo.is_basic               = false;
		bo.is_buildable           = bld.is_buildable();
		bo.need_trees             = bh.is_logproducer();
		bo.need_stones            = bh.is_stoneproducer();
		bo.need_water             = bh.get_needs_water();
		bo.recruitment            = bh.for_recruitment();
		bo.space_consumer         = bh.is_space_consumer();

		if (char const * const s = bh.get_renews_map_resource()) {
			bo.production_hint = tribe->safe_ware_index(s);

			if (HINT_DEBUG)
				printf (" TDEBUG: %-20s get production hint: %d\n",bo.name,bo.production_hint);
		}

		// Read all interesting data from ware producing buildings
		if (typeid(bld) == typeid(ProductionSite_Descr)) {
			const ProductionSite_Descr & prod =
			    ref_cast<ProductionSite_Descr const, Building_Descr const>(bld);
			bo.type =
			    bld.get_ismine() ? BuildingObserver::MINE :
			    BuildingObserver::PRODUCTIONSITE;
			container_iterate_const(BillOfMaterials, prod.inputs(), j)
			bo.inputs.push_back(j.current->first);
			container_iterate_const
			(ProductionSite_Descr::Output, prod.output_ware_types(), j)
			bo.outputs.push_back(*j.current);

			if (bo.type == BuildingObserver::MINE) {
				// get the resource needed by the mine
				if (char const * const s = bh.get_mines())
					bo.mines = world.get_resource(s);

				bo.mines_percent = bh.get_mines_percent();
			}

			bo.is_basic            = bh.is_basic();
			bo.prod_build_material = bh.prod_build_material();
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

	total_constructionsites       =    0;
	next_construction_due         =    0;
	next_road_due                 = 1000;
	next_productionsite_check_due =    0;
	inhibit_road_building         =    0;
	// Add all fields that we own
	Map & map = game().map();
	std::set<OPtr<PlayerImmovable> > found_immovables;

	for (Y_Coordinate y = 0; y < map.get_height(); ++y) {
		for (X_Coordinate x = 0; x < map.get_width(); ++x) {
			FCoords f = map.get_fcoords(Coords(x, y));

			if (f.field->get_owned_by() != player_number())
				continue;

			unusable_fields.push_back (f);

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
	while
	(not buildable_fields.empty()
	        and
	        buildable_fields.front()->next_update_due <= gametime) {
		BuildableField & bf = *buildable_fields.front();

		//  check whether we lost ownership of the node
		if (bf.coords.field->get_owned_by() != player_number()) {
			delete &bf;
			buildable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player->get_buildcaps(bf.coords) & BUILDCAPS_SIZEMASK) == 0) {
			unusable_fields.push_back (bf.coords);
			delete &bf;
			buildable_fields.pop_front();
			continue;
		}

		update_buildable_field (bf);
		bf.next_update_due = gametime + FIELD_UPDATE_INTERVAL;
		buildable_fields.push_back (&bf);
		buildable_fields.pop_front ();
	}
}

/**
 * Checks ALL available mineable fields.
 *
 * this shouldn't be used often, as it might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_mineable_fields(const int32_t gametime) {
	while
	(not mineable_fields.empty()
	        and
	        mineable_fields.front()->next_update_due <= gametime) {
		MineableField * mf = mineable_fields.front();

		//  check whether we lost ownership of the node
		if (mf->coords.field->get_owned_by() != player_number()) {
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player->get_buildcaps(mf->coords) & BUILDCAPS_MINE) == 0) {
			unusable_fields.push_back (mf->coords);
			delete mf;
			mineable_fields.pop_front();
			continue;
		}

		update_mineable_field (*mf);
		mf->next_update_due = gametime +FIELD_UPDATE_INTERVAL; //in fact this has very small effect
		mineable_fields.push_back (mf);
		mineable_fields.pop_front ();
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
			buildable_fields.push_back
			(new BuildableField(unusable_fields.front()));
			unusable_fields.pop_front();
			update_buildable_field (*buildable_fields.back());
			continue;
		}

		if (player->get_buildcaps(unusable_fields.front()) & BUILDCAPS_MINE) {
			mineable_fields.push_back
			(new MineableField(unusable_fields.front()));
			unusable_fields.pop_front();
			update_mineable_field (*mineable_fields.back());
			continue;
		}

		unusable_fields.push_back(unusable_fields.front());
		unusable_fields.pop_front();
	}
}


/// Updates one buildable field
void DefaultAI::update_buildable_field
(BuildableField & field, uint16_t range, bool military) {
	// look if there is any unowned land nearby
	Map & map = game().map();
	FindNodeUnowned find_unowned(player, game());
	FindNodeUnownedMineable find_unowned_minespots(player, game());
	Player_Number const pn = player->player_number();
	field.unowned_land_nearby =
	    map.find_fields(Area<FCoords>(field.coords, range), nullptr, find_unowned);

	if (field.unowned_land_nearby>2) // 2 is 'reasonably low' number here
		field.unowned_minespots_nearby =
		    map.find_fields(Area<FCoords>(field.coords, range+2), nullptr, find_unowned_minespots); //+2: a mine can mine raw materials from some range
	else
		field.unowned_minespots_nearby = 0;

	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	// Search in a radius of range
	map.find_immovables (Area<FCoords>(field.coords, range), &immovables);

	// Is this a general update or just for military consideration
	// (second is used in check_militarysites)
	if (!military) {
		int32_t const tree_attr  = Map_Object_Descr::get_attribute_id("tree");
		//int32_t const stone_attr = Map_Object_Descr::get_attribute_id("stone");
		field.reachable      = false;
		field.preferred      = false;
		field.avoid_military = false;
		field.enemy_nearby   = false;
		field.military_influence     = 0;
		field.military_capacity =0;
		field.military_loneliness    =1000; //instead of floats(v-
		field.military_presence    =0;
		field.military_stationed   =0;
		field.trees_nearby           = 0;
		//field.stones_nearby          = 0;
		field.space_consumers_nearby = 0;
		field.producers_nearby.clear();
		field.producers_nearby.resize(wares.size());
		field.consumers_nearby.clear();
		field.consumers_nearby.resize(wares.size());
		std::vector<Coords> water_list;

		if (field.water_nearby==-1) { //-1 means "value has never been calculated"
			FindNodeWater find_water;
			map.find_fields(Area<FCoords>(field.coords, 4), &water_list, find_water);
			field.water_nearby = water_list.size();
		}

		FCoords fse;
		map.get_neighbour (field.coords, WALK_SE, &fse);

		if (BaseImmovable const * const imm = fse.field->get_immovable())
			if
			(dynamic_cast<Flag const *>(imm)
			        or
			        (dynamic_cast<Road const *>(imm)
			         &&
			         (fse.field->nodecaps() & BUILDCAPS_FLAG)))
				field.preferred = true;

		for (uint32_t i = 0; i < immovables.size(); ++i) {
			const BaseImmovable & base_immovable = *immovables.at(i).object;

			if (dynamic_cast<const Flag *>(&base_immovable))
				field.reachable = true;

			//if (upcast(PlayerImmovable const, player_immovable, &base_immovable))
			//// TODO  Only continue; if this is an opposing site
			//// TODO  allied sites should be counted for military influence
			//if (player_immovable->owner().player_number() != pn) {
			//if (player->is_hostile(player_immovable->owner()))
			//field.enemy_nearby = true;
			//continue;
			//}

			//if (upcast(Building const, building, &base_immovable)) {

			//if (upcast(ConstructionSite const, constructionsite, building)) {
			//const Building_Descr & target_descr =
			//constructionsite->building();

			//if
			//(upcast
			//(MilitarySite_Descr const, target_ms_d, &target_descr))
			//{
			//const int32_t v =
			//target_ms_d->get_conquers()
			//-
			//map.calc_distance(field.coords, immovables.at(i).coords);

			//if (0 < v) {
			//field.military_influence += v * (v + 2) * 6;
			//field.avoid_military = true;
			//}
			//}

			//if (dynamic_cast<ProductionSite_Descr const *>(&target_descr))
			//consider_productionsite_influence
			//(field,
			//immovables.at(i).coords,
			//get_building_observer(constructionsite->name().c_str()));
			//}

			//if (upcast(MilitarySite const, militarysite, building)) {
			//const int32_t v =
			//militarysite->get_conquers()
			//-
			//map.calc_distance(field.coords, immovables.at(i).coords);

			//if (v > 0)
			//field.military_influence +=
			//v * v * militarysite->soldierCapacity();
			//}

			//if (dynamic_cast<const ProductionSite *>(building))
			//consider_productionsite_influence
			//(field,
			//immovables.at(i).coords,
			//get_building_observer(building->name().c_str()));

			//continue;
			//}

			if (immovables.at(i).object->has_attribute(tree_attr))
				++field.trees_nearby;

			//if (immovables.at(i).object->has_attribute(stone_attr))
			//	++field.stones_nearby;
		}

		//stones are not renewable, we will count them only if previous state si nonzero
		if (field.stones_nearby>0) {
			//printf (" TDEBUG: calculating stones: old state: %d at %3d x %3d\n",field.stones_nearby,field.coords.x,field.coords.y);
			int32_t const stone_attr = Map_Object_Descr::get_attribute_id("stone");
			field.stones_nearby          = 0;

			for (uint32_t i = 0; i < immovables.size(); ++i) {
				//const BaseImmovable & base_immovable = *immovables.at(i).object;
				if (immovables.at(i).object->has_attribute(stone_attr))
					++field.stones_nearby;
			}

			//printf (" TDEBUG: calculating stones: new state: %d\n",field.stones_nearby);
		} //else printf (" TDEBUG: calculating stones not needed: old state: %d at %3dx%3d\n",field.stones_nearby,field.coords.x,field.coords.y);
	}

	//folowing is done allways (regardless of military or not)
	// calculates MILITARY INFLUENCE
	//else { // the small update just for military consideration

	if (MILIT_SCORE_DEBUG)
		printf (" FIELD SCORE: central building: %3dx%3d\n",field.coords.x,field.coords.y);

	field.military_stationed=0;

	for (uint32_t i = 0; i < immovables.size(); ++i) {
		const BaseImmovable & base_immovable = *immovables.at(i).object;

		//testing if it is enemy-owned field
		//TODO count such fields...
		if (upcast(PlayerImmovable const, player_immovable, &base_immovable))

			// TODO  Only continue; if this is an opposing site
			// TODO  allied sites should be counted for military influence
			if (player_immovable->owner().player_number() != pn) {
				if (player->is_hostile(player_immovable->owner()))
					field.enemy_nearby = true;

				continue;
			}

		if (upcast(Building const, building, &base_immovable)) {
			if (upcast(ConstructionSite const, constructionsite, building)) {
				const Building_Descr & target_descr =
				    constructionsite->building();

				if	(upcast (MilitarySite_Descr const, target_ms_d, &target_descr)) {
					const int32_t dist=map.calc_distance(field.coords, immovables.at(i).coords);
					const int32_t radius = target_ms_d->get_conquers() + 4;
					const int32_t v = radius - dist;

					if (v > 0) {
						if (MILIT_SCORE_DEBUG)
							printf (" FIELD SCORE:  testing near military construction site at %3dx%3d, presumed capacity: %d, loneliness:%4f (%2d:%2d)\n"
							        ,immovables.at(i).coords.x,immovables.at(i).coords.y,2,static_cast<double>(dist)/radius,dist,radius);

						field.military_influence += v * (v + 2) * 6;
						field.avoid_military = true;
						field.military_capacity+=2;
						field.military_loneliness*=static_cast<double_t>(dist)/radius;
					}
				}
			}

			if (upcast(MilitarySite const, militarysite, building)) {
				const int32_t dist=map.calc_distance(field.coords, immovables.at(i).coords);
				const int32_t radius =militarysite->get_conquers() + 4;
				const int32_t v = 	radius - dist;

				if (v > 0 and dist>0) {
					if (MILIT_SCORE_DEBUG)
						printf (" FIELD SCORE:  testing near military building at %3dx%3d, capacity: %d, loneliness:%3f (%2d:%2d), stationed: %1d\n"
						        ,immovables.at(i).coords.x,immovables.at(i).coords.y,militarysite->maxSoldierCapacity(),static_cast<double>(dist)/ radius,dist,radius,militarysite->stationedSoldiers().size());

					field.military_influence +=
					    v * v * militarysite->soldierCapacity();
					field.military_capacity+=militarysite->maxSoldierCapacity();
					field.military_presence+=militarysite->stationedSoldiers().size();

					if (militarysite->stationedSoldiers().size()>0)
						field.military_stationed+=1;

					field.military_loneliness*=static_cast<double_t>(dist) / radius;
				}
			}
		}
	}

	if (MILIT_SCORE_DEBUG)
		printf (" FIELD SCORE:  results: capacity: %d, presence: %d, loneliness: %4d, stationed: %1d\n",
		        field.military_capacity,field.military_presence,field.military_loneliness,field.military_stationed);

	//}
}

/// Updates one mineable field
void DefaultAI::update_mineable_field (MineableField & field) {
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	Map & map = game().map();
	map.find_immovables (Area<FCoords>(field.coords, 5), &immovables);
	field.reachable    = false;
	field.preferred    = false;
	field.mines_nearby = 1;
	FCoords fse;
	map.get_brn(field.coords, &fse);

	if (BaseImmovable const * const imm = fse.field->get_immovable())
		if
		(dynamic_cast<Flag const *>(imm)
		        or
		        (dynamic_cast<Road const *>(imm)
		         &&
		         (fse.field->nodecaps() & BUILDCAPS_FLAG)))
			field.preferred = true;

	container_iterate_const(std::vector<ImmovableFound>, immovables, i) {
		if (dynamic_cast<Flag const *>(i.current->object))
			field.reachable = true;
		else
			if (upcast(Building const, bld, i.current->object)) {
				if (bld->descr().get_ismine()) {
					++field.mines_nearby;
				} else
					if (upcast(ConstructionSite const, cs, bld)) {
						if (cs->building().get_ismine())
							++field.mines_nearby;
					}
			}
	}
}


/// Updates the production and MINE sites statistics needed for construction decision.
void DefaultAI::update_productionsite_stats(int32_t const gametime) {
	// Updating the stats every 10 seconds should be enough
	next_stats_update_due = gametime + 10000;

	// Reset statistics for all buildings
	for (uint32_t i = 0; i < buildings.size(); ++i) {
		if (buildings.at(i).cnt_built > 0)
			buildings.at(i).current_stats = 0;
		// If there are no buildings of that type set the current_stats to 100
		else
			buildings.at(i).current_stats = 0;  //there was 100, this confuses algorithm

		buildings.at(i).unoccupied = false;
	}

	// Check all available productionsites
	for (uint32_t i = 0; i < productionsites.size(); ++i) {
		assert(productionsites.front().bo->cnt_built > 0);
		// Add statistics value
		productionsites.front().bo->current_stats +=
		    productionsites.front().site->get_statistics_percent();
		// Check whether this building is completely occupied
		productionsites.front().bo->unoccupied |=
		    !productionsites.front().site->can_start_working();
		// Now reorder the buildings
		productionsites.push_back(productionsites.front());
		productionsites.pop_front();
	}

	//for mines also
	// Check all available productionsites
	for (uint32_t i = 0; i < mines.size(); ++i) {
		assert(mines.front().bo->cnt_built > 0);
		// Add statistics value
		mines.front().bo->current_stats +=
		    mines.front().site->get_statistics_percent();
		// Check whether this building is completely occupied
		mines.front().bo->unoccupied |=
		    !mines.front().site->can_start_working();
		// Now reorder the buildings
		mines.push_back(mines.front());
		mines.pop_front();
	}

	// Scale statistics down
	for (uint32_t i = 0; i < buildings.size(); ++i) {
		if (buildings.at(i).cnt_built > 0)
			buildings.at(i).current_stats /= buildings.at(i).cnt_built;
	}
}

//constructs the most needed building
//algorithm goes over all avaiable spots and all allowed buildings,
//scores every combination and one with highest and positive score
//is built.
//buildings are split into cathegories
bool DefaultAI::construct_building (int32_t gametime) { // (int32_t gametime)
	//  Just used for easy checking whether a mine or something else was built.
	bool mine = false;
	bool field_blocked=false;
	int32_t spots=0;
	uint32_t consumers_nearby_count=0;
	int32_t bulgarian_constant=12;  //some building get preciousness as priority at that
	// can be too low in many cases
	std::vector<int32_t> spots_avail;
	spots_avail.resize(4);

	for (int32_t i = 0; i < 4; ++i)
		spots_avail.at(i) = 0;

	for
	(std::list<BuildableField *>::iterator i = buildable_fields.begin();
	        i != buildable_fields.end();
	        ++i)
		++spots_avail.at((*i)->coords.field->nodecaps() & BUILDCAPS_SIZEMASK);

	//calculating expand factor
	int32_t expand_factor = 0;

	if (type != DEFENSIVE) {
		++expand_factor;

		// check space and set the need for expansion
		if
		(spots_avail.at(BUILDCAPS_BIG)
		        <
		        static_cast<uint16_t>(2 + (productionsites.size() / 50)))
			expand_factor += 2;

		if
		(spots_avail.at(BUILDCAPS_MEDIUM) + spots_avail.at(BUILDCAPS_BIG)
		        <
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

		if
		(spots_avail.at(BUILDCAPS_SMALL)  +
		        spots_avail.at(BUILDCAPS_MEDIUM) +
		        spots_avail.at(BUILDCAPS_BIG)
		        <
		        16)
			expand_factor *= 3;
	}

	//checking amount of free spots, if needed setting new building stop flag
	new_buildings_stop=false;

	if (militarysites.size()*2+20<	productionsites.size() or spots<(6+ static_cast<int32_t>(productionsites.size())/5) ) {
		new_buildings_stop=true;
	}

	if (NEW_BUILDING_DEBUG)
		printf (" TDEBUG new buildings stop: %s; milit: %3d vs prod: %3d buildings, spots: %4d\n",new_buildings_stop?"Y":"N",militarysites.size(),productionsites.size(),spots);

	bool new_military_buildings_stop;
	bool near_enemy_b_buildings_stop;
	int32_t military_boost=1;
	new_military_buildings_stop=false;
	near_enemy_b_buildings_stop=false;
	int32_t treshold=(militarysites.size()+productionsites.size())/100+1;

	if (unstationed_milit_buildings + military_under_constr/3 > treshold ) {
		new_military_buildings_stop=true;

		if (MILITARY_DEBUG)
			printf (" TDEBUG new military buildings stop ON, %d %d \n",unstationed_milit_buildings,military_under_constr);
	} else
		if (MILITARY_DEBUG)
			printf (" TDEBUG new military buildings stop OFF, %d %d\n",unstationed_milit_buildings,military_under_constr);

	//if (new_buildings_stop and new_military_buildings_stop and militarysites.size()*2 +30 >	productionsites.size()) {
	//if (MILITARY_DEBUG or NEW_BUILDING_DEBUG) printf (" TDEBUG Allowing production buildings because of lack of soldiers and military buildings stop\n");
	//new_buildings_stop=false;
	//}
	if (unstationed_milit_buildings + military_under_constr/3 > 2*treshold ) {
		near_enemy_b_buildings_stop=true;

		if (MILITARY_DEBUG)
			printf (" TDEBUG new military near-enemy buildings stop ON, %d %d \n",unstationed_milit_buildings,military_under_constr);
	}

	//here we deal with situation when for some time no new military building was built (and there are no unoccupied ones)
	if ((unstationed_milit_buildings + military_under_constr)>0)
		military_last_build=game().get_gametime();

	if (military_last_build+2*60*1000<game().get_gametime()) {
		if (MILITARY_DEBUG)
			printf (" TDEBUG: Boosting military building\n");

		military_boost=200;
	}

	// Defensive AIs also attack sometimes (when they want to expand)
	if (type == DEFENSIVE && expand_factor > 1)
		if (next_attack_consideration_due <= game().get_gametime())
			consider_attack(game().get_gametime());

	//Building_Index proposed_building = INVALID_INDEX; // I need BuildingObserver not index
	BuildingObserver * best_building = nullptr;
	int32_t proposed_priority = 0;
	Coords proposed_coords;

	// Remove outdated fields from blocker list
	for
	(std::list<BlockedField>::iterator i = blocked_fields.begin();
	        i != blocked_fields.end();)
		if (i->blocked_until < game().get_gametime()) {
			i = blocked_fields.erase(i);
		} else
			++i;

	//these are 3 helping variables
	bool output_is_needed=false;
	int16_t max_preciousness=0; //preciousness of most precious output
	int16_t max_needed_preciousness=0; //preciousness of most precious NEEDED output

	// first scan all buildable fields for regular buildings
	for
	(std::list<BuildableField *>::iterator i = buildable_fields.begin();
	        i != buildable_fields.end();
	        ++i) {
		BuildableField * const bf = *i;

		if (!bf->reachable)
			continue;

		if (time(nullptr) % 5 ==0)
			continue;   //add randomnes and ease AI

		// Continue if field is blocked at the moment
		field_blocked=false;

		for
		(std::list<BlockedField>::iterator j = blocked_fields.begin();
		        j != blocked_fields.end();
		        ++j)
			if (j->coords == bf->coords)
				field_blocked=true;

		//continue;
		if (field_blocked)
			continue;

		assert(player);
		int32_t const maxsize =
		    player->get_buildcaps(bf->coords) & BUILDCAPS_SIZEMASK;

		// For every field test all buildings
		for (uint32_t j = 0; j < buildings.size(); ++j) {
			BuildingObserver & bo = buildings.at(j);

			if (!bo.buildable(*player))
				continue;

			if (time(nullptr) % 5 ==0)
				continue;   //add randomnes and ease AI

			if (bo.type == BuildingObserver::MINE)
				continue;

			if (gametime-bo.construction_decision_time<BUILDING_MIN_INTERVAL)
				continue;

			if (bo.unoccupied)
				continue;

			if (not bo.type == BuildingObserver::MILITARYSITE and bo.cnt_under_construction >=2)
				continue;

			//so we are going to seriously evaluate this building on this field,
			//first some base info
			output_is_needed=false;
			max_preciousness=0;
			max_needed_preciousness=0;

			// Check if the produced wares are needed (if it is producing anything)
			if (bo.outputs.size()>0) {
				container_iterate(std::list<EconomyObserver *>, economies, l) {
					// Don't check if the economy has no warehouse.
					if ((*l.current)->economy.warehouses().empty())
						continue;

					for (uint32_t m = 0; m < bo.outputs.size(); ++m) {
						Ware_Index wt(static_cast<size_t>(bo.outputs.at(m)));

						if ((*l.current)->economy.needs_ware(wt)) {
							output_is_needed=true;

							if (wares.at(bo.outputs.at(m)).preciousness>max_needed_preciousness)
								max_needed_preciousness=wares.at(bo.outputs.at(m)).preciousness;

							max_preciousness=wares.at(bo.outputs.at(m)).preciousness;
						} else {
							if (wares.at(bo.outputs.at(m)).preciousness>max_preciousness)
								max_preciousness=wares.at(bo.outputs.at(m)).preciousness;
						}
					}
				}
			}

			//if current field is not sufficient (by building size)
			if (bo.desc->get_size() > maxsize)
				continue;

			int32_t prio = 0; //score of a bulding on a field

			if (bo.type == BuildingObserver::PRODUCTIONSITE) {
				if (bo.need_trees) { //LUMBERJACS
					if (bo.cnt_built+bo.cnt_under_construction+bo.unoccupied<=2)
						prio=bulgarian_constant+200+bf->trees_nearby;
					else
						if ((bo.cnt_under_construction+bo.unoccupied)<=1 and  (bo.cnt_built + bo.cnt_under_construction + bo.unoccupied)< (3+static_cast<int32_t>(mines.size() + productionsites.size())/20) )
							prio=bf->trees_nearby*2 + bf->producers_nearby.at(bo.outputs.at(0))*5;
						else
							if ((bo.cnt_under_construction+bo.unoccupied)<=1 and  (bf->trees_nearby>10) and not new_buildings_stop )
								prio=bf->trees_nearby*2 + bf->producers_nearby.at(bo.outputs.at(0))*5;

					if (WOOD_DEBUG and prio>0)
						printf (" TDEBUG: %1d: suggesting woodcutter with prio: %2d, total: %2d\n", player_number(),prio,bo.cnt_built + bo.cnt_under_construction + bo.unoccupied);
				}  else
					if (bo.need_stones) {
						if (bo.total_count()==0 and output_is_needed and bo.cnt_under_construction==0 and bf->stones_nearby>2)
							prio=45+bf->stones_nearby;
						else
							if (bf->producers_nearby.at(bo.outputs.at(0))<=1 and bf->stones_nearby >0 and bo.cnt_under_construction<1)
								prio=bulgarian_constant+bf->stones_nearby/3;
					} else
						if (bo.production_hint >= 0) {   //SUPPORTING PRODUCTIONS
							if ((bo.cnt_under_construction+bo.unoccupied)>0)
								continue;

							// production hint (f.e. associate forester with logs)

							if (bo.need_water and  bf->water_nearby < 3) //probably some of them needs water
								continue;

							//the goal is to have '3+(mines.size() + productionsites.size())/20' supporting buildings
							const int32_t goal=3+static_cast<int32_t>(mines.size() + productionsites.size())/15;

							if (bo.total_count()<=1)
								prio=70;
							else
								if (bo.total_count() < goal) {
									prio=bulgarian_constant+10;
								} else {// even when we are above goal we need to consider level of stock
									if (bo.stocklevel_time<game().get_gametime()-5*1000) {
										bo.stocklevel=get_stocklevel_by_hint(static_cast<size_t>(bo.production_hint));
										bo.stocklevel_time=game().get_gametime();
									}

									if 	(bo.stocklevel<50 and not new_buildings_stop)
										prio=bulgarian_constant+7;
									else
										continue;
								}

							//additionally we will increase score if producers are nearby
							prio+=bf->producers_nearby.at(bo.production_hint)*5;
							//as usuall:
							prio = recalc_with_border_range(*bf, prio);

							if (WOOD_DEBUG)
								printf (" TDEBUG: %1d: suggesting new %16s, current count: %2d, goal: %2d, prio: %d\n",player_number(),bo.name,bo.total_count(),goal,prio);

							if (prio<=0)
								continue;
						} else
							if (bo.recruitment) {
								//this will depend on number of mines and productionsites
								if (static_cast<int32_t>((productionsites.size() + mines.size())/30)>bo.total_count() and bo.cnt_under_construction==0)
									prio=4+bulgarian_constant;
							} else { //finally normal productionsites
								if (bo.production_hint>=0)
									continue;

								if ((bo.cnt_under_construction + bo.unoccupied)>0)
									continue;

								//first eliminate buildings needing water if there is short supplies
								if (bo.need_water and  bf->water_nearby < 3)
									continue;

								if ((bo.is_basic or bo.prod_build_material) and bo.total_count()==0)
									prio=150+max_preciousness;
									
								else if (game().get_gametime()<15*60*1000)
									continue;
									
								else
									if  ( ((bo.is_basic or bo.prod_build_material) and  bo.total_count()<=1) or
									        (output_is_needed and bo.total_count()==0) )
										prio=80+max_preciousness;
									else
										if (bo.inputs.size()==0) {
											if (bo.stocklevel_time<game().get_gametime()-5*1000) {
												bo.stocklevel=get_stocklevel(bo);
												bo.stocklevel_time=game().get_gametime();
											}

											if (output_is_needed or  bo.stocklevel<50) {
												prio=max_preciousness+bulgarian_constant;

												if (bo.space_consumer)
													prio+=5;

												if (bo.cnt_built<2)
													prio+=5;

												if (PRODUCTION_DEBUG)
													printf (" TDEBUG: %2d/%-15s without inputs: stats: %3d/%2d, setting priority: %2d, on %3d %3d\n",bo.id,bo.name,bo.current_stats,bo.total_count(),prio,bf->coords.x,bf->coords.y);
											}
										} else
											if (bo.inputs.size()>0) {
												//to have two buildings from everything (intended for upgradeable buildings)
												// but I do not know how to identify such buildings
												if ( bo.cnt_built==1 and game().get_gametime()>60*60*1000 and bo.desc->enhancements().size() > 0 and mines.size()>0) {
													prio=max_preciousness+bulgarian_constant;
													//if (UPGRADE_DEBUG) printf (" TDEBUG: proposing %-16s at %3dx%3d as second building of a type, score: %2d\n",bo.name,bf->coords.x,bf->coords.y,prio);
												}
												//if output is needed and there are no idle buildings
												else
													if (output_is_needed ) {
														if (bo.cnt_built>0 and bo.current_stats>80) {
															prio=max_preciousness+bulgarian_constant+30;

															if (PRODUCTION_DEBUG)
																printf (" TDEBUG: %2d/%-15s with inputs: stats: %3d>90, setting priority: %2d, on %3d %3d\n",bo.id,bo.name,bo.current_stats,prio,bf->coords.x,bf->coords.y);
														} else
															if (bo.cnt_built>0 and bo.current_stats>55) {
																prio=max_preciousness+bulgarian_constant;

																if (PRODUCTION_DEBUG)
																	printf (" TDEBUG: %2d/%-15s with inputs: stats: %3d>60, setting priority: %2d, on %3d %3d\n",bo.id,bo.name,bo.current_stats,prio,bf->coords.x,bf->coords.y);
															}
													}
											}

								//if (game().get_gametime() < 8*60*1000 ) {
								//if ((bo.is_basic or bo.prod_build_material) and bo.total_count()==0)
								//prio=150+max_preciousness;
								//} else if (game().get_gametime() < 15*60*1000) {
								//if ((bo.is_basic or bo.prod_build_material) and  bo.total_count()==0)
								//prio=max_preciousness+150+bulgarian_constant;
								//else if ((bo.is_basic or bo.prod_build_material) and  output_is_needed and bo.total_count()<=1)
								//prio=max_preciousness+80+bulgarian_constant;
								////else if (bo.is_basic and  bo.total_count()<=0 and bo.cnt_under_construction==0 )
								////prio=max_preciousness;
								//else if (! bo.is_basic and  bo.total_count()<=1 and  output_is_needed){
								//prio=max_preciousness+bulgarian_constant+50; //+50 to make sure at least one of such building is built
								//if (PRODUCTION_DEBUG) printf (" TDEBUG: %2d/%-15s in second period,  setting priority: %2d, on %3d %3d\n",bo.id,bo.name,prio,bf->coords.x,bf->coords.y);
								//}
								//} else if (new_buildings_stop and not ((bo.is_basic or bo.prod_build_material) and bo.cnt_built<2)){
								//continue;
								//} else if (bo.inputs.size()==0){
								//if (bo.stocklevel_time<game().get_gametime()-5*1000){
								//bo.stocklevel=get_stocklevel(bo);
								//bo.stocklevel_time=game().get_gametime();}
								//if (output_is_needed or  bo.stocklevel<50){
								//prio=max_preciousness+bulgarian_constant;
								//if (bo.space_consumer) prio+=5;
								//if (bo.cnt_built<2)    prio+=5;
								//if (PRODUCTION_DEBUG) printf (" TDEBUG: %2d/%-15s without inputs: stats: %3d/%2d, setting priority: %2d, on %3d %3d\n",bo.id,bo.name,bo.current_stats,bo.total_count(),prio,bf->coords.x,bf->coords.y);
								//}
								//} else if (bo.inputs.size()>0) {

								////to have two buildings from everything (intended for upgradeable buildings)
								//// but I do not know how to identify such buildings
								//if ( bo.cnt_built==1 and game().get_gametime()>60*60*1000 and bo.desc->enhancements().size() > 0 and mines.size()>0){
								//prio=max_preciousness+bulgarian_constant;
								////if (UPGRADE_DEBUG) printf (" TDEBUG: proposing %-16s at %3dx%3d as second building of a type, score: %2d\n",bo.name,bf->coords.x,bf->coords.y,prio);
								//}

								////if output is needed and there are no idle buildings
								//if (output_is_needed ){
								////if (bo.cnt_built>0)
								////printf (" TDEBUG: building: %d, built: %d, utilization: %d, in construction: %d\n",bo.id,bo.cnt_built,bo.current_stats,bo.cnt_under_construction);
								//if (bo.cnt_built>0 and bo.current_stats>90){
								//prio=max_preciousness+bulgarian_constant+30;
								//if (PRODUCTION_DEBUG) printf (" TDEBUG: %2d/%-15s with inputs: stats: %3d>90, setting priority: %2d, on %3d %3d\n",bo.id,bo.name,bo.current_stats,prio,bf->coords.x,bf->coords.y);
								//}
								//else if (bo.cnt_built>0 and bo.current_stats>60){
								//prio=max_preciousness+bulgarian_constant;
								//if (PRODUCTION_DEBUG) printf (" TDEBUG: %2d/%-15s with inputs: stats: %3d>60, setting priority: %2d, on %3d %3d\n",bo.id,bo.name,bo.current_stats,prio,bf->coords.x,bf->coords.y);
								//}
								//else if (bo.cnt_built==0){
								//prio=max_preciousness+80+bulgarian_constant;
								//if (PRODUCTION_DEBUG) printf (" TDEBUG: %2d/%-15s as first building of the type, setting priority: %2d, on %3d %3d\n",bo.id,bo.name,prio,bf->coords.x,bf->coords.y);
								//}
								//}
								//}

								if (prio<=0 )
									continue;

								//then we consider borders and enemies nearby (if any)
								//printf ("  TDEBUG: 	%s: prio before: %d, unowned nearby %d,coords: %3d x %3d,\n",
								//bo.name,prio,bf->unowned_land_nearby,bf->coords.x,bf->coords.y);
								prio = recalc_with_border_range(*bf, prio);
								//printf ("  TDEBUG: 	 prio after: %d\n",prio);
								//+1 if any consumers are nearby
								consumers_nearby_count=0;

								for (size_t k = 0; k < bo.outputs.size(); ++k)
									consumers_nearby_count += bf->consumers_nearby.at(bo.outputs.at(k));

								if (consumers_nearby_count>0)
									prio += 1;

								// do not construct more than one building,
								// if supply line is already broken.
								// is this needed?
								//if (!check_supply(bo) && bo.total_count() > 0)
								//prio -= 12;
							}
			}  //production sites done
			else
				if (bo.type == BuildingObserver::MILITARYSITE) {
					if (military_boost>1 and MILITARY_DEBUG)
						printf (" TDEBUG: boosting: unowned land %d \n",bf->unowned_land_nearby);

					if (new_military_buildings_stop and not bf->enemy_nearby)
						continue;

					if (near_enemy_b_buildings_stop and bf->enemy_nearby)
						continue;

					if (bo.desc->get_size()==3 and game().get_gametime() < 15*60*1000) //do not built fortresses in first half of hour of game
						continue;

					if (!bf->unowned_land_nearby)
						continue;

					int32_t minespots_score=0;
					minespots_score=bf->unowned_minespots_nearby;

					if (minespots_score>0) {
						minespots_score*=4;
						minespots_score+=8;
					}

					//if (MILITARY_DEBUG) printf (" TDEBUG field: %3dx%3d: unowned land: %3d, mine spots: %3d\n",bf->coords.x,bf->coords.y,bf->unowned_land_nearby,bf->unowned_minespots_nearby);
					prio  = (bf->unowned_land_nearby-4 + minespots_score + bf->stones_nearby/2 +bf->military_loneliness/5-100 + military_boost);// * (1 + type);

					if (MILITARY_DEBUG)
						printf (" NEW MILITARY: %3dx%3d: unowned: %3d(%3d), mines: %3d, lonel.: %4d (%4d), stones: %2d, boost: %3d,result: %3d, enemy:% 2d, cur stat: %2d/%2d/%2d, stops:%s %s\n"
						        ,bf->coords.x , bf->coords.y , bf->unowned_land_nearby-4 , minespots_score , bf->unowned_minespots_nearby , bf->military_loneliness/5-100,
						        bf->military_loneliness , bf->stones_nearby/2 , military_boost , prio , bf->enemy_nearby , bo.cnt_built , unstationed_milit_buildings , bo.cnt_under_construction,
						        new_military_buildings_stop?"Y":"N" , near_enemy_b_buildings_stop?"Y":"N");

					//if (military_boost>1 and MILITARY_DEBUG)
					//printf (" TDEBUG: Original priority before boosting: %d \n",prio);
					//prio *= military_boost;
					//prio -= bf->military_influence * (5 - type);
					// set to at least 1
					//prio  = prio > 0 ? prio : 1;
					//prio *= expand_factor;
					//prio /= 2;

					if (bo.desc->get_size() < maxsize)
						prio=prio-5; //penalty

					if (bf->enemy_nearby and bf->military_capacity<12) {
						if (MILITARY_DEBUG)
							printf (" NEW MILITARY: Military capacity: %2d, boosting priority by 100d\n",bf->military_capacity);

						prio += 100;
					}

					//else
					//prio -= bf->military_influence * 2;

					//if (bf->avoid_military)
					//prio /= 5;

					//prio -= militarysites.size() - productionsites.size() / (3 - type);
					if (MILITARY_DEBUG and prio>0)
						printf (" NEW MILITARY:  candidate's final priority: %d \n",prio);
				}	else
					if (bo.type == BuildingObserver::WAREHOUSE) {
						//  Build one warehouse for ~every 35 productionsites and mines.
						//  Militarysites are slightly important as well, to have a bigger
						//  chance for a warehouses (containing waiting soldiers or wares
						//  needed for soldier training) near the frontier.
						if ((static_cast<int32_t>(productionsites.size() + mines.size()))/35 > static_cast<int32_t>(numof_warehouses) and bo.cnt_under_construction==0)
							prio = 13;

						// take care about borders and enemies
						prio = recalc_with_border_range(*bf, prio);
					} else
						if (bo.type == BuildingObserver::TRAININGSITE) {
							// build after 20 production sites and then after each 50 production site
							if (static_cast<int32_t>((productionsites.size()+30)/50)>bo.total_count() and bo.cnt_under_construction==0)
								prio=4;

							// take care about borders and enemies
							prio = recalc_with_border_range(*bf, prio);
						}

			// think of space consuming buildings nearby like farms or vineyards
			prio -=  bf->space_consumers_nearby*10;

			// Stop here, if priority is 0 or less.
			if (prio <= 0)
				continue;

			// Prefer road side fields
			prio += bf->preferred ?  1 : 0;
			// don't waste good land for small huts
			prio -= (maxsize - bo.desc->get_size()) * 5;

			if (prio > proposed_priority) {
				//proposed_building = bo.id;
				best_building = &bo;
				proposed_priority = prio;
				proposed_coords   = bf->coords;
			}
		}	//ending loop over buildings
	}  //ending loop over fields

	//if (MINES_DEBUG ) printf(" TDEBUG: a\n");
	// then try all mines - as soon as basic economy is build up.
	if (gametime>next_mine_construction_due) {
		//if (MINES_UPDATE_DEBUG) printf (" TDEBUG: testing mines and updating mineable fields (on %10d)\n",gametime);
		update_all_mineable_fields(gametime);
		next_mine_construction_due=gametime+IDLE_MINE_UPDATE_INTERVAL;

		//if (MINES_UPDATE_DEBUG) printf (" new next_mine_construction_due=%10d\n",next_mine_construction_due);
		for
		(uint32_t i = 0; i < buildings.size() && productionsites.size() > 8; ++i) {
			BuildingObserver & bo = buildings.at(i);

			if (!bo.buildable(*player) || bo.type != BuildingObserver::MINE)
				continue;

			if (gametime-bo.construction_decision_time<BUILDING_MIN_INTERVAL)
				continue;

			if (game().get_gametime() < 15*60*1000)
				continue;

			if (game().get_gametime() < 30*60*1000 and (bo.total_count() + bo.unoccupied + bo.cnt_under_construction)>0)
				continue;

			//if (MINES_DEBUG ) printf(" TDEBUG: c\n");

			// Don't build another building of this type, if there is already
			// one that is unoccupied at the moment
			// or under construction
			if ((bo.cnt_under_construction + bo.unoccupied )> 0)
				continue;

			/* - uninteresting if a mine ware is needed - we exploit the raw material
			// Check if the produced wares are needed
			bool needed = false;
			container_iterate(std::list<EconomyObserver *>, economies, l) {
				// Don't check if the economy has no warehouse.
				if ((*l.current)->economy.warehouses().empty())
					continue;
				for (uint32_t m = 0; m < bo.outputs.size(); ++m) {
					Ware_Index wt(static_cast<size_t>(bo.outputs.at(m)));
					if ((*l.current)->economy.needs_ware(wt)) {
						needed = true;
						break;
					}
				}
				if (needed)
					break;
			}

			// Only try to build mines that produce needed wares.
			if (!needed)
				continue;
			*/

			//calculating actual amount of mined raw materials
			if (bo.stocklevel_time<game().get_gametime()-5*1000) {
				bo.stocklevel=get_stocklevel(bo);
				bo.stocklevel_time=game().get_gametime();
			}

			if (MINES_DEBUG )
				printf (" TDEBUG: considering %12s/%1d: stat: %3d(50), stocklevel: %2d(50), count %2d / %2d / %2d\n",bo.name,bo.mines,bo.current_stats,bo.stocklevel,bo.total_count(),bo.unoccupied,bo.cnt_under_construction);

			// Only try to build mines that produce needed wares.
			if (((bo.cnt_built-bo.unoccupied)>0 and bo.current_stats<50) or bo.stocklevel>50)
				continue;

			//iterating over fields
			for
			(std::list<MineableField *>::iterator j = mineable_fields.begin();
			        j != mineable_fields.end();
			        ++j) {
				int32_t prio = 0;

				if ((*j)->coords.field->get_resources() != bo.mines)
					continue;
				else
					if ((*j)->mines_nearby>1)
						prio += (*j)->coords.field->get_resources_amount() * 4 / 3 - (*j)->mines_nearby*10;
					else
						prio += (*j)->coords.field->get_resources_amount() * 4 / 3;

				// Only build mines on locations where some material can be mined
				if (prio < 2)
					continue;

				// Continue if field is blocked at the moment
				bool blocked = false;

				for
				(std::list<BlockedField>::iterator k = blocked_fields.begin();
				        k != blocked_fields.end();
				        ++k)
					if ((*j)->coords == k->coords) {
						blocked = true;
						break;
					}

				if (blocked)
					continue;

				if (MINES_DEBUG)
					printf ("  TDEBUG: priority of potential mines: %3d (%3d - %3d), at  %3d x %3d\n",
					        prio,(*j)->coords.field->get_resources_amount() * 4 / 3, (*j)->mines_nearby*10,(*j)->coords.x,(*j)->coords.y);

				//if mines nearby - this check mines in too big radius - no sense to consider this
				//prio -= 4 * (*j)->mines_nearby ;
				//if (MINES_DEBUG) printf ("  TDEBUG: priority after near mines consideration: %3d; value: %2d\n",prio,(*j)->mines_nearby);
				if (prio > proposed_priority) {
					//proposed_building = bo.id;
					best_building  = &bo;
					proposed_priority = prio;
					proposed_coords = (*j)->coords;
					mine = true;

					if (MINES_DEBUG)
						printf ("   TDEBUG: using %-12s as a candidate\n",bo.name);
				}
			} //ending interation over fields
		} //ending iteration over buildings
	} //end of mines section

	//if there is no winner:
	//if (proposed_building == INVALID_INDEX) {
	if (best_building == nullptr) {
		if (WINNER_DEBUG)
			printf (" TDEBUG:  no building picked up\n");

		mine=false;
		return false;
	}

	// send the command to construct a new building
	game().send_player_build
	//(player_number(), proposed_coords, proposed_building);
	(player_number(), proposed_coords, best_building->id);
	BlockedField blocked
	(game().map().get_fcoords(proposed_coords), game().get_gametime() + 120000);  //two minutes
	blocked_fields.push_back(blocked);

	if ( not best_building->type == BuildingObserver::MILITARYSITE)
		best_building->construction_decision_time=gametime;
	else //very ugly hack here
		best_building->construction_decision_time=gametime-BUILDING_MIN_INTERVAL/2;

	if (WINNER_DEBUG)
		printf (" TDEBUG:  winning priority %4d, building %2d, coords: %3d x %3d, M: %s\n",
		        proposed_priority,best_building->id,proposed_coords.x,proposed_coords.y,mine?"Y":"N");

	// set the type of update that is needed
	if (mine) {
		next_mine_construction_due=gametime+BUSY_MINE_UPDATE_INTERVAL;

		if (MINES_UPDATE_DEBUG)
			printf (" TDEBUG expanding mine update by: %d,building %2d, coords: %3d x %3d  \n",BUSY_MINE_UPDATE_INTERVAL,best_building->id,proposed_coords.x,proposed_coords.y);

		if (MINES_UPDATE_DEBUG)
			printf ("  new next_mine_construction_due=%10d\n",next_mine_construction_due);

		//m_mineable_changed = true;
	} else
		//last_mine_constr_time=gametime;
		m_buildable_changed = true;

	return true;
}


/**
 * This function searches for places where a new road is needed to connect two
 * economies. It then sends the request to build the road.
 */
bool DefaultAI::construct_roads (int32_t gametime) {
	if (economies.size() < 2) {
		// only one economy, no need for new roads
		return false;
	}

	uint32_t economies_to_connect = 0;
	EconomyObserver * eo_to_connect = economies.front(); // dummy initialisation

	//  fetch first two economies that might be connectable
	for
	(std::list<EconomyObserver *>::iterator i = economies.begin();
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
	if
	(eo_to_connect->failed_connection_tries > 3
	        and
	        eo_to_connect->flags.size() == 1) {
		Building * bld = eo_to_connect->flags.front()->get_building();

		if (bld) {
			BuildingObserver & bo = get_building_observer(bld->name().c_str());

			if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
				game().send_player_bulldoze(*const_cast<Flag *>(eo_to_connect->flags.front()));
				eo_to_connect->flags.pop_front();
				// Block the field at constructionsites coords for 5 minutes
				// against new construction tries.
				BlockedField blocked
				(game().map().get_fcoords(bld->get_position()),
				 game().get_gametime() + 300000);
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
bool DefaultAI::improve_roads (int32_t gametime) {
	// Remove flags of dead end roads, as long as no more wares are stored on them
	container_iterate(std::list<EconomyObserver *>, economies, i)
	container_iterate(std::list<Flag const *>, (*i.current)->flags, j)

	if ((*j.current)->is_dead_end() && (*j.current)->current_wares() == 0) {
		game().send_player_bulldoze(*const_cast<Flag *>((*j.current)));
		j.current = (*i.current)->flags.erase(j.current);
		return true;
	}

	// force a split on roads that are longer than 3 parts
	// actually we do not care for loss of building capabilities - normal maps
	// should have enough space and the computer can expand it's territory.
	if (!roads.empty()) {
		const Path & path = roads.front()->get_path();

		if (path.get_nsteps() > 3) {
			const Map & map = game().map();
			CoordPath cp(map, path);
			// try to split after two steps
			CoordPath::Step_Vector::size_type i = cp.get_nsteps() - 1, j = 1;

			for (; i >= j; --i, ++j) {
				{
					const Coords c = cp.get_coords().at(i);

					if (map[c].nodecaps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag (player_number(), c);
						return true;
					}
				}
				{
					const Coords c = cp.get_coords().at(j);

					if (map[c].nodecaps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag (player_number(), c);
						return true;
					}
				}
			}

			// Unable to set a flag - perhaps the road was build stupid
			game().send_player_bulldoze(*const_cast<Road *>(roads.front()));
		}

		roads.push_back (roads.front());
		roads.pop_front ();
	}

	if (!economies.empty() && inhibit_road_building <= gametime) {
		EconomyObserver * eco = economies.front();

		if (!eco->flags.empty()) {
			bool finish = false;
			const Flag & flag = *eco->flags.front();

			// try to connect to another economy
			if (economies.size() > 1)
				finish = connect_flag_to_another_economy(flag);

			// try to improve the roads at this flag
			//  TODO do this only on useful places - the attempt below
			//  TODO  unfortunatey did not work as it should...
			//  if the flag is full of wares or if it is not yet a fork.
			if (!finish) //&& (!flag.has_capacity() || flag.nr_of_roads() < 3))
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
bool DefaultAI::connect_flag_to_another_economy (const Flag & flag) {
	FindNodeWithFlagOrRoad functor;
	CheckStepRoadAI check(player, MOVECAPS_WALK, true);
	std::vector<Coords> reachable;
	// first look for possible destinations
	functor.economy = flag.get_economy();
	Map & map = game().map();
	map.find_reachable_fields
	(Area<FCoords>(map.get_fcoords(flag.get_position()), 16),
	 &reachable,
	 check,
	 functor);

	if (reachable.empty())
		return false;

	// then choose the one with the shortest path
	Path * path = new Path();
	bool found = false;
	check.set_openend(false);
	Coords closest;
	container_iterate_const(std::vector<Coords>, reachable, i) {
		Path * path2 = new Path();

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
		if (dynamic_cast<const Road *>(map[closest].get_immovable()))
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
bool DefaultAI::improve_transportation_ways (const Flag & flag) {
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
	queue.push (NearFlag(flag, 0, 0));
	Map & map = game().map();

	while (!queue.empty()) {
		std::vector<NearFlag>::iterator f = find(nearflags.begin(), nearflags.end(), queue.top().flag);

		if (f != nearflags.end()) {
			queue.pop();
			continue;
		}

		nearflags.push_back(queue.top());
		queue.pop();
		NearFlag & nf = nearflags.back();

		for (uint8_t i = 1; i <= 6; ++i) {
			Road * const road = nf.flag->get_road(i);

			if (!road)
				continue;

			Flag * endflag = &road->get_flag(Road::FlagStart);

			if (endflag == nf.flag)
				endflag = &road->get_flag(Road::FlagEnd);

			int32_t dist = map.calc_distance(flag.get_position(), endflag->get_position());

			if (dist > 12) //  out of range
				continue;

			queue.push(NearFlag(*endflag, nf.cost + road->get_path().get_nsteps(), dist));
		}
	}

	std::sort (nearflags.begin(), nearflags.end(), CompareDistance());
	CheckStepRoadAI check(player, MOVECAPS_WALK, false);

	for (uint32_t i = 1; i < nearflags.size(); ++i) {
		NearFlag & nf = nearflags.at(i);

		if (2 * nf.distance + 2 < nf.cost) {
			Path & path = *new Path();

			if
			(map.findpath
			        (flag.get_position(), nf.flag->get_position(), 0, path, check)
			        >=
			        0
			        and
			        static_cast<int32_t>(2 * path.get_nsteps() + 2) < nf.cost) {
				game().send_player_build_road (player_number(), path);
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
bool DefaultAI::check_economies () {
	while (!new_flags.empty()) {
		const Flag & flag = *new_flags.front();
		new_flags.pop_front();
		get_economy_observer(flag.economy())->flags.push_back (&flag);
	}

	container_iterate(std::list<EconomyObserver *>, economies, i) {
		// check if any flag has changed its economy
		std::list<Flag const *> &fl = (*i.current)->flags;

		for (std::list<Flag const *>::iterator j = fl.begin(); j != fl.end();) {
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
	if ((next_productionsite_check_due > gametime) || productionsites.empty())
		return false;

	next_productionsite_check_due = gametime + 10000;
	// Get link to productionsite that should be checked
	ProductionSiteObserver & site = productionsites.front();
	bool changed = false;
	// Reorder and set new values; - better now because there are multiple returns in the function
	productionsites.push_back(productionsites.front());
	productionsites.pop_front();
	// Get max radius of recursive workarea
	Workarea_Info::size_type radius = 0;
	const Workarea_Info & workarea_info = site.bo->desc->m_workarea_info;
	container_iterate_const(Workarea_Info, workarea_info, i)

	if (radius < i.current->first)
		radius = i.current->first;

	Map & map = game().map();

	//do not dismantle same type of building too soon - to give some time to update statistics
	//yes it interferes with building updates, but not big problem here
	if (site.bo->last_dismantle_time>game().get_gametime()-30*1000)
		return false;

	//getting rid of spaceconsumers who doesnt work
	if (SPACE_DEBUG and site.bo->space_consumer and site.bo->production_hint<0)
		printf(" TDEBUG: space consumer here: %15s at %3d x %3d: statistics: %3d, age: %5d(>180)\n",
		       site.bo->name,site.site->get_position().x,site.site->get_position().y,site.site->get_statistics_percent(),(gametime-site.builttime)/1000);

	if (site.bo->space_consumer and site.bo->production_hint<0 and (gametime-site.builttime)>3*60*1000 and site.site->get_statistics_percent()<=5) {
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		game().send_player_dismantle(*site.site);

		if (SPACE_DEBUG)
			printf(" TDEBUG: dismantling: %15s at %3d x %3d: statistics: %2d\n",
			       site.bo->name,site.site->get_position().x,site.site->get_position().y,site.site->get_statistics_percent());
	}

	// Lumberjack / Woodcutter handling
	if
	(site.bo->need_trees
	        and
	        map.find_immovables
	        (Area<FCoords>(map.get_fcoords(site.site->get_position()), radius),
	         nullptr,
	         FindImmovableAttribute(Map_Object_Descr::get_attribute_id("tree")))
	        < 3) {
		// Do not destruct the last lumberjack - perhaps some small trees are
		// near, a forester will plant some trees or some new trees will seed
		// in reach. Computer players can easily run out of wood if this check
		// is not done.
		if (site.bo->cnt_built <= 3+static_cast<int32_t>(mines.size() + productionsites.size())/20) {
			if (WOOD_DEBUG)
				printf (" TDEBUG: %1d: cutter without trees, but not dismantling due to low numbers of cutters (%2d)\n",player_number(),site.bo->cnt_built)	;

			return false;
		}

		if (site.site->get_statistics_percent() < 10 ) {
			// destruct the building and it's flag (via flag destruction)
			// the destruction of the flag avoids that defaultAI will have too many
			// unused roads - if needed the road will be rebuild directly.
			//printf (" TDEBUG: dismantling lumberjacks hut\n");
			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			game().send_player_dismantle(*site.site);

			if (WOOD_DEBUG)
				printf (" TDEBUG %1d: cutter without trees, dismantling..., remaining cutters: %2d\n",player_number(),site.bo->cnt_built);

			site.bo->last_dismantle_time=game().get_gametime();
			return true;
		}
	}

	// Quarry handling
	if
	(site.bo->need_stones
	        and
	        map.find_immovables
	        (Area<FCoords>(map.get_fcoords(site.site->get_position()), radius),
	         nullptr,
	         FindImmovableAttribute(Map_Object_Descr::get_attribute_id("stone")))
	        ==
	        0) {
		// destruct the building and it's flag (via flag destruction)
		// the destruction of the flag avoids that defaultAI will have too many
		// unused roads - if needed the road will be rebuild directly.
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		game().send_player_dismantle(*site.site);
		return true;
	}

	// All other productionsites without input and not supporting ones (rangers...)...
	if
	(site.bo->inputs.empty() // does not consume anything
	        and
	        site.bo->production_hint == -1 // not a renewing building (forester...)
	        and
	        site.builttime + 600000 < game().get_gametime() // > 10 minutes old
	        and
	        site.site->can_start_working()) { // building is occupied
		if (site.site->get_statistics_percent() == 0) { // production stats == 0%
			++site.statszero;

			// Only continue here, if at least 3 following times, the stats were 0%
			if (site.statszero >= 3) {
				// Do not destruct building, if it's basic and the last of this
				// type left.
				if (site.bo->is_basic && site.bo->cnt_built <= 1)
					return false;

				// If building seems to be useless, think about destructing it and
				// it's flag (via flag destruction) more or less randomly. The
				// destruction of the flag avoids that defaultAI will have too many
				// unused roads - if needed the road will be rebuild directly.
				//
				// Add a bonus if one building of this type is still unoccupied
				if (((game().get_gametime() % 4) + site.bo->unoccupied) > 2) {
					flags_to_be_removed.push_back(site.site->base_flag().get_position());
					game().send_player_dismantle(*site.site);
					site.bo->last_dismantle_time=game().get_gametime();
					return true;
				} else
					return false;
			}

			return false;
		} else
			site.statszero = 0; // reset zero counter
	}

	//supporting productionsites (rangers)
	//stop/start them based on stock avaiable
	if (site.bo->production_hint >=0) {
		//if (STANDBY_DEBUG) printf ("  TDEBUG: check_productionsites(): testing building %s\n",site.bo->name);
		if (site.bo->stocklevel_time<game().get_gametime()-5*1000) {
			site.bo->stocklevel=get_stocklevel_by_hint(site.bo->production_hint);
			site.bo->stocklevel_time=game().get_gametime();
		}

		if (STANDBY_DEBUG)
			printf ("  TDEBUG: standby review: %-16s(%dx):stock level: %3d, status: %s\n",site.bo->name,site.bo->cnt_built,site.bo->stocklevel,site.site->is_stopped()?"stopped":"running");

		if (site.bo->stocklevel>220 and site.bo->cnt_built>3) {
			if (STANDBY_DEBUG)
				printf ("   * dismantling the building\n");

			flags_to_be_removed.push_back(site.site->base_flag().get_position());
			game().send_player_dismantle(*site.site);
			site.bo->last_dismantle_time=game().get_gametime();
			return true;
		}

		if (site.bo->stocklevel>190 and not site.site->is_stopped()) {
			if (STANDBY_DEBUG)
				printf ("   * stopping building\n");

			game().send_player_start_stop_building (*site.site);
		}

		if (site.bo->stocklevel<150 and site.site->is_stopped()) {
			if (STANDBY_DEBUG)
				printf ("   * starting building\n");

			game().send_player_start_stop_building (*site.site);
		}
	}

	//Upgrading policy
	//a) if there are two buildings and none enhanced, one is enhanced
	//b) if there are two buildings and at least one functional
	//statistics percents are decisive

	//do not upgrade if current building is only one in operation
	if ( (site.bo->cnt_built-site.bo->unoccupied)<=1)
		return false;

	// Check whether building is enhanceable and if wares of the enhanced
	// buildings are needed. If yes consider an upgrade.
	std::set<Building_Index> enhancements = site.site->enhancements();
	int32_t maxprio = 0;
	Building_Index enbld;  //to get rid of this
	BuildingObserver * bestbld=nullptr;
	container_iterate_const(std::set<Building_Index>, enhancements, x) {
		// Only enhance buildings that are allowed (scenario mode)
		if (player->is_building_type_allowed(*x.current)) {
			const Building_Descr & bld = *tribe->get_building_descr(*x.current);
			BuildingObserver & en_bo = get_building_observer(bld.name().c_str());

			//do not build the same building so soon (kind of duplicity check)
			if (gametime-en_bo.construction_decision_time<BUILDING_MIN_INTERVAL)
				continue;

			// Don't enhance this building, if there is already one of same type
			// under construction or unoccupied
			if (en_bo.cnt_under_construction + en_bo.unoccupied > 0)
				continue;

			// don't upgrade without workers
			if (!site.site->has_workers(*x.current, game()))
				continue;

			//forcing first upgrade
			if ((en_bo.cnt_under_construction + en_bo.cnt_built + en_bo.unoccupied)==0 and
			        (site.bo->cnt_built - site.bo->unoccupied)>=1 and
			        (game().get_gametime()-site.builttime)>30*60*1000 and  mines.size()>0) {
				if (UPGRADE_DEBUG)
					printf (" UPGRADE: upgrading (forcing as first) %12s at %3d x %3d: age %d min.\n",
					        site.bo->name,site.site->get_position().x,site.site->get_position().y,(game().get_gametime()-site.builttime)/60000);

				game().send_player_enhance_building(*site.site, (*x.current));
				return true;
			}

			//now, let consider normal upgrade

			if (UPGRADE_DEBUG)
				printf (" UPGRADE: %1d: working enhanced buildings (%15s): %1d, statitistics: %2d\n"
				        ,player_number(),en_bo.name , en_bo.cnt_built-en_bo.unoccupied , en_bo.current_stats);

			//do not upgrade if candidate production % is too low
			if ( (en_bo.cnt_built-en_bo.unoccupied)==0 or (en_bo.cnt_under_construction+en_bo.unoccupied)>0 or en_bo.current_stats<50)
				continue;

			int32_t prio = 0;

			if (en_bo.current_stats>65) {
				prio = en_bo.current_stats-site.bo->current_stats; // priority for enhancement
				prio+=en_bo.current_stats-65;

				if (UPGRADE_DEBUG)
					printf (" UPGRADE:  proposing upgrade (non-first building) %12s at %3d x %3d: prio: %2d, target statistics: %2d\n"
					        ,site.bo->name,site.site->get_position().x,site.site->get_position().y,prio,en_bo.current_stats);
			}

			if (prio > maxprio) {
				maxprio = prio;
				enbld = (*x.current);
				bestbld=&en_bo;
			}
		}
	}

	// Enhance if enhanced building is useful
	// additional: we dont want to lose the old building
	if (maxprio > 0) {
		if (UPGRADE_DEBUG)
			printf (" UPGRADE:   upgrading %15s(as non first)\n",bestbld->name);

		game().send_player_enhance_building(*site.site, enbld);
		bestbld->construction_decision_time=gametime;
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
bool DefaultAI::check_mines(int32_t const gametime) {
	if ((next_mine_check_due > gametime) || mines.empty())
		return false;

	next_mine_check_due = gametime + 10000;  //10 seconds is enough
	//also statistics must be recalculated
	// Get link to productionsite that should be checked
	ProductionSiteObserver & site = mines.front();
	Map & map = game().map();
	Field * field = map.get_fcoords(site.site->get_position()).field;
	// Reorder and set new values; - due to returns within the function
	mines.push_back(mines.front());
	mines.pop_front();

	if (MINES_UPDATE_DEBUG)
		printf (" MINES_UPDATE: %1d: reviewing %-15s at %3dx%3d, statistics: %3d, left resources: %2d\n",
		        player_number(), site.bo->name , site.site->get_position().x,site.site->get_position().y,site.site->get_statistics_percent(),field->get_resources_amount());

	// Don't try to enhance as long as stats are not down to 0% - it is possible,
	// that some neighbour fields still have resources
	if (site.site->get_statistics_percent() > 10)
		return false;

	// Check if mine ran out of resources
	uint8_t current = field->get_resources_amount();

	if (current < 1) {
		// destruct the building and it's flag (via flag destruction)
		// the destruction of the flag avoids that defaultAI will have too many
		// unused roads - if needed the road will be rebuild directly.
		flags_to_be_removed.push_back(site.site->base_flag().get_position());
		game().send_player_dismantle(*site.site);

		if (MINES_UPDATE_DEBUG)
			printf(" MINES_UPDATE: Dismantling...\n");

		return true;
	}

	// Check whether building is enhanceable. If yes consider an upgrade.
	std::set<Building_Index> enhancements = site.site->enhancements();
	int32_t maxprio = 0;
	Building_Index enbld;
	BuildingObserver * bestbld=nullptr;
	bool changed = false;
	container_iterate_const(std::set<Building_Index>, enhancements, x) {
		// Only enhance buildings that are allowed (scenario mode)
		if (player->is_building_type_allowed(*x.current)) {
			//first exclude possibility there are enhancements in construction or unoccupied
			const Building_Descr & bld =
			    *tribe->get_building_descr(*x.current);
			BuildingObserver & en_bo =
			    get_building_observer(bld.name().c_str());

			if (MINES_UPDATE_DEBUG)
				printf (" MINES_UPDATE:   considering upgrade to %15s, count B:%1d(stat:%3d)  U:%1d  C:%1d\n",
				        en_bo.name , en_bo.cnt_built , en_bo.current_stats , en_bo.unoccupied , en_bo.cnt_under_construction);

			if (en_bo.unoccupied + en_bo.cnt_under_construction>0)
				continue;

			//do not upgrade target building are not working properly (probably do not have food)
			if (en_bo.cnt_built>0 and en_bo.current_stats<60)
				continue;

			//do not build the same building so soon (kind of duplicity check)
			if (gametime-en_bo.construction_decision_time<BUILDING_MIN_INTERVAL)
				continue;

			// Check if mine needs an enhancement to mine more resources
			uint8_t const until =
			    field->get_starting_res_amount() * (100 - site.bo->mines_percent)
			    /
			    100;

			if (MINES_UPDATE_DEBUG)
				printf (" MINES_UPDATE:    until:%3d ?>, current: %3d\n",until,current);

			if (until >= current) {
				// add some randomness - just for the case if more than one
				// enhancement is available (not in any tribe yet)
				int32_t const prio = time(nullptr) % 3 + 1;

				if (prio > maxprio) {
					maxprio = prio;
					enbld = (*x.current);
					bestbld=&en_bo;

					if (MINES_UPDATE_DEBUG)
						printf (" MINES_UPDATE:    ..is candidate\n");
				}
			}
		}
	}

	// Enhance if enhanced building is useful
	if (maxprio > 0) {
		game().send_player_enhance_building(*site.site, enbld);
		bestbld->construction_decision_time=gametime;
		changed = true;

		if (MINES_UPDATE_DEBUG)
			printf (" MINES_UPDATE:    ..enhancing\n");
	}

	//// Reorder and set new values;
	//mines.push_back(mines.front());
	//mines.pop_front();
	return changed;
}


//this count ware as hints
uint32_t DefaultAI::get_stocklevel_by_hint(size_t hintoutput) {
	uint32_t count=0;
	Ware_Index wt(hintoutput);
	container_iterate(std::list<EconomyObserver *>, economies, l) {
		// Don't check if the economy has no warehouse.
		if ((*l.current)->economy.warehouses().empty())
			continue;

		count+=(*l.current)->economy.stock_ware(wt);
	}

	if (STOCK_DEBUG)
		printf (" TDEBUG: stock : %3d for hint: %2d, time: %3d\n",count,hintoutput,game().get_gametime()/1000);

	return count;
}

//this count all stock for all output
uint32_t DefaultAI::get_stocklevel(BuildingObserver & bo) {
	uint32_t count=0;

	if (bo.outputs.size()>0) {
		container_iterate(std::list<EconomyObserver *>, economies, l) {
			// Don't check if the economy has no warehouse.
			if ((*l.current)->economy.warehouses().empty())
				continue;

			for (uint32_t m = 0; m < bo.outputs.size(); ++m) {
				Ware_Index wt(static_cast<size_t>(bo.outputs.at(m)));
				count+=(*l.current)->economy.stock_ware(wt);
			}
		}
	}

	if (STOCK_DEBUG)
		printf (" TDEBUG: stock : %3d for building: %s, time: %3d\n",count,bo.name,game().get_gametime()/1000);

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
	if (next_militarysite_check_due > gametime)
		return false;

	//just to be sure the value is reset
	next_militarysite_check_due = gametime + 5*1000; //10 seconds is really fine
	//even if there are no finished & attended military sites, probably there are ones just in construction
	unstationed_milit_buildings=0;

	for (std::list<MilitarySiteObserver >::iterator it = militarysites.begin(); it != militarysites.end(); ++it)
		if (it->site->stationedSoldiers().size()==0)
			unstationed_milit_buildings+=1;

	//count militarysites in construction
	military_under_constr=0;

	for (uint32_t j = 0; j < buildings.size(); ++j) {
		BuildingObserver & bo = buildings.at(j);

		if (bo.type == BuildingObserver::MILITARYSITE)
			military_under_constr+=bo.cnt_under_construction;
	}

	// Only useable, if defaultAI owns at least one militarysite
	if (militarysites.empty())
		return false;

	// Check next militarysite
	bool changed = false;
	Map & map = game().map();
	//uint32_t ratio;
	MilitarySite * ms = militarysites.front().site;
	uint32_t const vision = ms->vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	// look if there is any enemy land nearby
	FindNodeUnowned find_unowned(player, game(), true);

	//printf (" TDEBUG: check_militarysites()\n");

	if (map.find_fields(Area<FCoords>(f, vision), nullptr, find_unowned) == 0) {
		// If no enemy in sight - decrease the number of stationed soldiers
		// as long as it is > 1 - BUT take care that there is a warehouse in the
		// same economy where the thrown out soldiers can go to.
		if (ms->economy().warehouses().size()) {
			uint32_t const j = ms->soldierCapacity();

			if (MilitarySite::kPrefersRookies != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(*ms, MilitarySite::kPrefersRookies);
			} else
				if (j > 1)
					game().send_player_change_soldier_capacity(*ms, -1);
			// if the building is in inner land and other militarysites still
			// hold the miliary influence of the field, consider to destruct the
			// building to free some building space.
				else {
					// treat this field like a buildable and write military info to it.
					BuildableField bf(f);
					update_buildable_field(bf, vision, true);
					const int32_t size_penalty=ms-> get_size()-1;

					if (MIL_DISM_DEBUG)
						printf (" DISMANTLE CHECK: testing finished building %3dx%3d, capacity: %2d(>9), presence: %2d(>3), loneliness: %4d(<160), stationed: %1d(>2+%d as size penalty)\n",
						        f.x,f.y,bf.military_capacity,bf.military_presence,bf.military_loneliness,bf.military_stationed,size_penalty);

					if (bf.military_capacity>9 and bf.military_presence>3  and bf.military_loneliness<160 and bf.military_stationed>(2+size_penalty)) {
						if (MIL_DISM_DEBUG)
							printf (" DISMANTLE CHECK:  * dismantling the building on %5d, last dismantle: %5d\n",game().get_gametime()/1000,military_last_dismantle/1000);

						if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
							flags_to_be_removed.push_back(ms->base_flag().get_position());
							game().send_player_dismantle(*ms);
							military_last_dismantle=game().get_gametime();
						} else {
							game().send_player_bulldoze(*ms);
							military_last_dismantle=game().get_gametime();
						}
					}

					//regardless of following check, if there is too big military influence
					//and no enemy in sight, destroying the building
					//if (military_last_dismantle<(game().get_gametime()-20*60*1000)) {
					//ratio=bf.military_influence+ms->maxSoldierCapacity()*10+bf.unowned_land_nearby*3;
					//if (MIL_DISM_DEBUG) printf (" TDEBUG: considering milit. build. dismantle at %3dx%3d: Unowned land: %3d, enemy: %1d, mil. influence: %4d,  soldiers: %d,%d, ratio:%5d, unstationed: %d\n",
					//f.x,f.y,bf.unowned_land_nearby,bf.enemy_nearby,bf.military_influence,j,ms->maxSoldierCapacity(),ratio,unstationed_milit_buildings);
					//if ((unstationed_milit_buildings>5 and ratio<190 and not bf.enemy_nearby) or
					//(ratio<140 and not bf.enemy_nearby) ) {
					//if (MIL_DISM_DEBUG) printf ("  * dismantling the building on %5d, last dismantle: %5d\n",game().get_gametime()/1000,military_last_dismantle/1000);
					//if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
					//flags_to_be_removed.push_back(ms->base_flag().get_position());
					//game().send_player_dismantle(*ms);
					//} else {
					//game().send_player_bulldoze(*ms);
					//}
					//military_last_dismantle=game().get_gametime();
					////if (MIL_DISM_DEBUG) printf (" last dismantle set on: %3d\n",military_last_dismantle/1000);
					//return true;	//do not go on with building, we need to refresh statistics
					//}
					//}
					// watch out if there is any unowned land in vision range. If there
					// is none, there must be another building nearer to the frontier.
					//else if (bf.unowned_land_nearby == 0) {
					//// bigger buildings are only checked after all smaller
					//// ones are at least one time checked.
					//if (militarysites.front().checks == 0) {
					//// If the military influence of other near buildings is higher
					//// than the own doubled max SoldierCapacity destruct the
					//// building and it's flag (via flag destruction)
					//// the destruction of the flag avoids that defaultAI will have
					//// too many unused roads - if needed the road will be rebuild
					//// directly.
					//if (static_cast<int32_t>(ms->maxSoldierCapacity() * 4) < bf.military_influence ) {
					////if (bf.military_influence>1000) { // HERE CHANGED
					//if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
					//flags_to_be_removed.push_back(ms->base_flag().get_position());
					//game().send_player_dismantle(*ms);
					//} else {
					//game().send_player_bulldoze(*ms);
					//}
					//}
					//// Else consider enhancing the building (if possible)
					//else {
					//// Do not have too many constructionsites
					//uint32_t producers = mines.size() + productionsites.size();
					//if (total_constructionsites >= (5 + (producers / 10)))
					//goto reorder;
					//std::set<Building_Index> enhancements = ms->enhancements();
					//int32_t maxprio = 10000; // surely never reached
					//Building_Index enbld;
					//container_iterate_const
					//(std::set<Building_Index>, enhancements, x)
					//{
					//// Only enhance building to allowed (scenario mode)
					//if (player->is_building_type_allowed(*x.current)) {
					//const Building_Descr & bld =
					//*tribe->get_building_descr(*x.current);
					//BuildingObserver & en_bo =
					//get_building_observer(bld.name().c_str());
					//// Don't enhance this building, if there is
					//// already one of same type under construction
					//if (en_bo.cnt_under_construction > 0)
					//continue;
					//if (en_bo.cnt_built < maxprio) {
					//maxprio = en_bo.cnt_built;
					//enbld = (*x.current);
					//}
					//}
					//}
					//// Enhance if enhanced building is useful
					//if (maxprio < 10000) {
					//game().send_player_enhance_building(*ms, enbld);
					//changed = true;
					//}
					//}
					//} else
					//--militarysites.front().checks;
					//}
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

	//reorder:;
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();
	next_militarysite_check_due = gametime + 5*1000; //10 seconds is really fine
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
int32_t DefaultAI::recalc_with_border_range(const BuildableField & bf, int32_t prio) {
	// Prefer building space in the inner land.
	prio /= (1 + (bf.unowned_land_nearby / 4));

	if (bf.unowned_land_nearby>15)
		prio-=(bf.unowned_land_nearby-15);

	// Especially places near the frontier to the enemies are unlikely
	//  NOTE take care about the type of computer player. The more
	//  NOTE aggressive a computer player is, the more important is
	//  NOTE this check. So we add \var type as bonus.
	if (bf.enemy_nearby and prio>0)
		prio /= (3 + type);

	return prio;
}



/**
 * calculates how much a productionsite of type \arg bo is needed inside it's
 * economy. \arg prio is initial value for this calculation
 *
 * \returns the calculated priority
 */
int32_t DefaultAI::calculate_need_for_ps(BuildingObserver & bo, int32_t prio) {
	// some randomness to avoid that defaultAI is building always
	// the same (always == another game but same map with
	// defaultAI on same coords)
	prio += time(nullptr) % 3 - 1;

	// check if current economy can supply enough material for
	// production.
	for (uint32_t k = 0; k < bo.inputs.size(); ++k) {
		prio += 2 * wares.at(bo.inputs.at(k)).producers;
		prio -= wares.at(bo.inputs.at(k)).consumers;
	}

	if (bo.inputs.empty())
		prio += 4;

	int32_t output_prio = 0;

	for (uint32_t k = 0; k < bo.outputs.size(); ++k) {
		WareObserver & wo = wares.at(bo.outputs.at(k));

		if (wo.consumers > 0) {
			output_prio += wo.preciousness;
			output_prio += wo.consumers * 2;
			output_prio -= wo.producers * 2;

			if (bo.total_count() == 0)
				output_prio += 10; // add a big bonus
		}
	}

	if (bo.outputs.size() > 1)
		output_prio = static_cast<int32_t>
		              (ceil(output_prio / sqrt(static_cast<double>(bo.outputs.size()))));

	prio += 2 * output_prio;

	// If building consumes some wares, multiply with current statistics of all
	// other buildings of this type to avoid constructing buildings where already
	// some are running on low resources.
	// Else at least add a part of the stats t the calculation.
	if (!bo.inputs.empty()) {
		prio *= bo.current_stats;
		prio /= 100;
	} else
		prio = ((prio * bo.current_stats) / 100) + (prio / 2);

	return prio;
}


void DefaultAI::consider_productionsite_influence
(BuildableField & field, Coords coords, const BuildingObserver & bo) {
	if
	(bo.space_consumer
	        and
	        game().map().calc_distance(coords, field.coords) < 4)
		++field.space_consumers_nearby;

	for (size_t i = 0; i < bo.inputs.size(); ++i)
		++field.consumers_nearby.at(bo.inputs.at(i));

	for (size_t i = 0; i < bo.outputs.size(); ++i)
		++field.producers_nearby.at(bo.outputs.at(i));
}


/// \returns the economy observer containing \arg economy
EconomyObserver * DefaultAI::get_economy_observer(Economy & economy) {
	for
	(std::list<EconomyObserver *>::iterator i = economies.begin();
	        i != economies.end();
	        ++i)
		if (&(*i)->economy == &economy)
			return *i;

	economies.push_front (new EconomyObserver(economy));
	return economies.front();
}

/// \returns the building observer
BuildingObserver & DefaultAI::get_building_observer(char const * const name) {
	if (tribe == nullptr)
		late_initialization();

	for (uint32_t i = 0; i < buildings.size(); ++i)
		if (!strcmp(buildings.at(i).name, name))
			return buildings.at(i);

	throw wexception("Help: I do not know what to do with a %s", name);
}


/// this is called whenever we gain ownership of a PlayerImmovable
void DefaultAI::gain_immovable(PlayerImmovable & pi) {
	if      (upcast(Building,   building, &pi))
		gain_building (*building);
	else
		if (upcast(Flag const, flag,     &pi))
			new_flags.push_back (flag);
		else
			if (upcast(Road const, road,     &pi))
				roads    .push_front(road);
}

/// this is called whenever we lose ownership of a PlayerImmovable
void DefaultAI::lose_immovable(const PlayerImmovable & pi) {
	if        (upcast(Building const, building, &pi))
		lose_building (*building);
	else
		if   (upcast(Flag     const, flag,     &pi)) {
			container_iterate_const(std::list<EconomyObserver *>, economies, i)
			container_iterate(std::list<Flag const *>, (*i.current)->flags, j)

			if (*j.current == flag) {
				(*i.current)->flags.erase (j.current);
				return;
			}

			container_iterate(std::list<Flag const *>, new_flags, i)

			if (*i.current == flag) {
				new_flags.erase(i.current);
				return;
			}
		} else
			if (upcast(Road     const, road,     &pi))
				roads.remove (road);
}

/// this is called whenever we gain a new building
void DefaultAI::gain_building(Building & b) {
	BuildingObserver & bo = get_building_observer(b.name().c_str());

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver & target_bo =
		    get_building_observer
		    (ref_cast<ConstructionSite, Building>(b)
		     .building().name().c_str());
		++target_bo.cnt_under_construction;
		++total_constructionsites;
		// Let defaultAI try to directly connect the constructionsite
		next_road_due = game().get_gametime();
	} else {
		++bo.cnt_built;

		if (bo.type == BuildingObserver::PRODUCTIONSITE) {
			productionsites.push_back (ProductionSiteObserver());
			productionsites.back().site = &ref_cast<ProductionSite, Building>(b);
			productionsites.back().bo = &bo;
			productionsites.back().builttime = game().get_gametime();
			productionsites.back().statszero = 0;

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				++wares.at(bo.outputs.at(i)).producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				++wares.at(bo.inputs.at(i)).consumers;
		} else
			if (bo.type == BuildingObserver::MINE) {
				mines.push_back (ProductionSiteObserver());
				mines.back().site = &ref_cast<ProductionSite, Building>(b);
				mines.back().bo = &bo;

				for (uint32_t i = 0; i < bo.outputs.size(); ++i)
					++wares.at(bo.outputs.at(i)).producers;

				for (uint32_t i = 0; i < bo.inputs.size(); ++i)
					++wares.at(bo.inputs.at(i)).consumers;
			} else
				if (bo.type == BuildingObserver::MILITARYSITE) {
					militarysites.push_back (MilitarySiteObserver());
					militarysites.back().site = &ref_cast<MilitarySite, Building>(b);
					militarysites.back().bo = &bo;
					militarysites.back().checks = bo.desc->get_size();
				} else
					if (bo.type == BuildingObserver::WAREHOUSE)
						++numof_warehouses;
	}
}

/// this is called whenever we lose a building
void DefaultAI::lose_building(const Building & b) {
	BuildingObserver & bo = get_building_observer(b.name().c_str());

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver & target_bo =
		    get_building_observer
		    (ref_cast<ConstructionSite const, Building const>(b)
		     .building().name().c_str());
		--target_bo.cnt_under_construction;
		--total_constructionsites;
	} else {
		--bo.cnt_built;

		if (bo.type == BuildingObserver::PRODUCTIONSITE) {
			for
			(std::list<ProductionSiteObserver>::iterator i =
			            productionsites.begin();
			        i != productionsites.end();
			        ++i)
				if (i->site == &b) {
					productionsites.erase(i);
					break;
				}

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				--wares.at(bo.outputs.at(i)).producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				--wares.at(bo.inputs.at(i)).consumers;
		} else
			if (bo.type == BuildingObserver::MINE) {
				for
				(std::list<ProductionSiteObserver>::iterator i =
				            mines.begin();
				        i != mines.end();
				        ++i)
					if (i->site == &b) {
						mines.erase(i);
						break;
					}

				for (uint32_t i = 0; i < bo.outputs.size(); ++i)
					--wares.at(bo.outputs.at(i)).producers;

				for (uint32_t i = 0; i < bo.inputs.size(); ++i)
					--wares.at(bo.inputs.at(i)).consumers;
			} else
				if (bo.type == BuildingObserver::MILITARYSITE) {
					for
					(std::list<MilitarySiteObserver>::iterator i =
					            militarysites.begin();
					        i != militarysites.end();
					        ++i)
						if (i->site == &b) {
							militarysites.erase(i);
							break;
						}
				} else
					if (bo.type == BuildingObserver::WAREHOUSE) {
						assert(numof_warehouses > 0);
						--numof_warehouses;
					}
	}

	m_buildable_changed = true;
	m_mineable_changed = true;
}


/// Checks that supply line exists for given building.
/// Recurcsively verify that all inputs have a producer.
// TODO: this function leads to periodic freezes of ~1 second on big games on my system.
// TODO: It needs profiling and optimization.
bool DefaultAI::check_supply(const BuildingObserver & bo) {
	size_t supplied = 0;
	container_iterate_const(std::vector<int16_t>, bo.inputs, i)
	container_iterate_const(std::vector<BuildingObserver>, buildings, j)

	if
	(j.current->cnt_built &&
	        std::find
	        (j.current->outputs.begin(), j.current->outputs.end(),
	         *i.current)
	        !=
	        j.current->outputs.end()
	        &&
	        check_supply(*j.current)) {
		++supplied;
		break;
	}

	return supplied == bo.inputs.size();
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

	Map & map = game().map();
	uint16_t const pn = player_number();
	// Check next militarysite
	MilitarySite * ms = militarysites.front().site;
	uint32_t const vision = ms->vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	Building * target = ms; // dummy initialisation to silence the compiler
	int32_t    chance    = 0;
	uint32_t   attackers = 0;
	uint8_t    retreat   = ms->owner().get_retreat_percentage();
	// Search in a radius of the vision of the militarysite and collect
	// information about immovables in the area
	std::vector<ImmovableFound> immovables;
	map.find_immovables
	(Area<FCoords>(f, vision), &immovables, FindImmovableAttackable());

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
		} else
			if (upcast(Warehouse, wh, immovables.at(j).object)) {
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
		next_attack_consideration_due = gametime % 7 * 1000 + gametime;
		return false;
	}

	if (ms->owner().is_retreat_change_allowed()) {
		// \todo Player is allowed to modify his retreat value
	}

	// Attack the selected target.
	game().send_player_enemyflagaction
	(target->base_flag(), pn, attackers, retreat);
	//  Do not attack again too soon - returning soldiers must get healed first.
	next_attack_consideration_due = (gametime % 51 + 10) * 1000 + gametime;
	return true;
}
