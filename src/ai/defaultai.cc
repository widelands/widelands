/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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
/**
 * Default AI
 */

#include "defaultai.h"

#include <ctime>

#include "ai/ai_hints.h"
#include "constructionsite.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "findimmovable.h"
#include "findnode.h"
#include "log.h"
#include "map.h"
#include "militarysite.h"
#include "player.h"
#include "productionsite.h"
#include "profile.h"
#include "tribe.h"
#include "upcast.h"
#include "warehouse.h"
#include "world.h"

#include <algorithm>
#include <queue>
#include <typeinfo>

#define FIELD_UPDATE_INTERVAL 1000

using namespace Widelands;

DefaultAI::Implementation DefaultAI::implementation;

/// Constructor of DefaultAI
DefaultAI::DefaultAI(Game & g, const Player_Number pid) :
Computer_Player(g, pid),
m_buildable_changed(true),
m_mineable_changed(true),
tribe(0),
next_militarysite_check_due(0),
next_attack_consideration_due(300000),
time_of_last_construction(0),
numof_warehouses(0)
{}


/**
 * Main loop of computer player "defaultAI"
 *
 * General behaviour is defined here.
 */
void DefaultAI::think ()
{
	if (tribe == 0)
		late_initialization ();

	const int32_t gametime = game().get_gametime();

	if (m_buildable_changed) {
		// update statistics about buildable fields
		update_all_buildable_fields(gametime);
	}
	if (m_mineable_changed) {
		// do the same for mineable fields
		update_all_mineable_fields(gametime);
	}
	m_buildable_changed = false;
	m_mineable_changed = false;

	// This must be checked every time as changes of bobs in AI area aren't
	// handled by the AI itself.
	update_all_not_buildable_fields();

	// wait a moment so that all fields are classified
	if (next_construction_due == 0) next_construction_due = gametime + 1000;

	// if there are more than one economy try to connect them with a road.
	if (next_road_due <= gametime) {
		next_road_due = gametime + 1000;
		if (construct_roads (gametime)) {
			m_buildable_changed = true;
			m_mineable_changed = true;
			return;
		}
	} else
		// only go on, after defaultAI tried to connect all economies.
		return;

	// improve existing roads
	if (improve_roads(gametime)) {
		m_buildable_changed = true;
		m_mineable_changed = true;
		inhibit_road_building = gametime + 2500;
		return;
	}

	// check if anything in the economies changed.
	// This needs to be done before new buildings are placed, to ensure that no
	// empty economy is left.
	if (check_economies())
		return;

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

	// consider whether a change of the soldier capacity of some militarysites
	// would make sense.
	if (check_militarysites(gametime))
		return;

	// Finally consider military actions
	if (next_attack_consideration_due <= gametime)
		consider_attack(gametime);
}


void DefaultAI::receive(NoteImmovable const & note)
{
	if (note.lg == LOSE)
		lose_immovable(*note.pi);
	else
		gain_immovable(*note.pi);
}

void DefaultAI::receive(NoteField const & note)
{
	if (note.lg == GAIN)
		unusable_fields.push_back(note.fc);
}


/**
 * Cares for all variables not initialised during construction
 *
 * When DefaultAI is constructed, some information is not yet available (e.g.
 * world), so this is done after complete loading of the map.
 */
void DefaultAI::late_initialization ()
{
	player = game().get_player(get_player_number());
	NoteReceiver<NoteImmovable>::connect(*player);
	NoteReceiver<NoteField>::connect(*player);
	tribe = &player->tribe();

	log ("ComputerPlayer(%d): initializing\n", get_player_number());

	Ware_Index const nr_wares = tribe->get_nrwares();
	wares.resize(nr_wares.value());
	for (Ware_Index i = Ware_Index::First(); i < nr_wares; ++i) {
		wares[i].producers    = 0;
		wares[i].consumers    = 0;
		wares[i].preciousness = tribe->get_ware_descr(i)->preciousness();
	}

	// collect information about the different buildings our tribe can construct
	Building_Index const nr_buildings = tribe->get_nrbuildings();
	const World & world = game().map().world();
	for (Building_Index i = Building_Index::First(); i < nr_buildings; ++i) {
		Building_Descr & bld = *tribe->get_building_descr(i);
		const std::string & building_name = bld.name();
		BuildingHints * bh = &bld.hints();

		buildings.resize (buildings.size() + 1);

		BuildingObserver & bo     = buildings.back();
		bo.name                   = building_name.c_str();
		bo.id                     = i;
		bo.desc                   = &bld;
		bo.type                   = BuildingObserver::BORING;
		bo.cnt_built              = 0;
		bo.cnt_under_construction = 0;
		bo.production_hint        = -1;

		bo.is_buildable = bld.buildable() & player->is_building_allowed(i);

		bo.need_trees             = bh->is_trunkproducer();
		bo.need_stones            = bh->is_stoneproducer();
		bo.need_water             = bh->get_needs_water();

		if (char const * const s = bh->get_renews_map_resource())
			bo.production_hint = tribe->safe_ware_index(strdup(s)).value();

		// Read all interesting data from ware producing buildings
		if (typeid(bld) == typeid(ProductionSite_Descr)) {
			const ProductionSite_Descr & prod =
				static_cast<const ProductionSite_Descr &>(bld);

			bo.type = bld.get_ismine() ?
			BuildingObserver::MINE : BuildingObserver::PRODUCTIONSITE;

			container_iterate_const(Ware_Types, prod.inputs(), j)
				bo.inputs.push_back(j.current->first.value());

			container_iterate_const
				(ProductionSite_Descr::Output, prod.output(), j)
				bo.outputs.push_back(j.current->     value());

			if (bo.type == BuildingObserver::MINE) {
				// get the resource needed by the mine
				if (char const * const s = bh->get_mines())
					bo.mines = world.get_resource(strdup(s));
			}

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

			if (f.field->get_owned_by() != get_player_number())
				continue;

			unusable_fields.push_back (f);

			if (upcast(PlayerImmovable, imm, f.field->get_immovable()))
				// Guard by a set - immovables might be on several fields at once
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
void DefaultAI::update_all_buildable_fields(const int32_t gametime)
{
	while
		(not buildable_fields.empty()
		 and
		 buildable_fields.front()->next_update_due <= gametime)
	{
		BuildableField * bf = buildable_fields.front();

		//  check whether we lost ownership of the node
		if (bf->coords.field->get_owned_by() != get_player_number()) {
			buildable_fields.pop_front();
			continue;
		}

		//  check whether we can still construct regular buildings on the node
		if ((player->get_buildcaps(bf->coords) & BUILDCAPS_SIZEMASK) == 0) {
			unusable_fields.push_back (bf->coords);
			delete bf;

			buildable_fields.pop_front();
			continue;
		}

		update_buildable_field (*bf);
		bf->next_update_due = gametime + FIELD_UPDATE_INTERVAL;

		buildable_fields.push_back (bf);
		buildable_fields.pop_front ();
	}
}

/**
 * Checks ALL available mineable fields.
 *
 * this shouldn't be used often, as it might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_mineable_fields(const int32_t gametime)
{
	while
		(not mineable_fields.empty()
		 and
		 mineable_fields.front()->next_update_due <= gametime)
	{
		MineableField * mf = mineable_fields.front();

		//  check whether we lost ownership of the node
		if (mf->coords.field->get_owned_by() != get_player_number()) {
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
		mf->next_update_due = gametime + FIELD_UPDATE_INTERVAL;

		mineable_fields.push_back (mf);
		mineable_fields.pop_front ();
	}
}

/**
 * Checks up to 50 fields that weren't buildable the last time.
 *
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_not_buildable_fields()
{
	int32_t pn = get_player_number();
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
		if
			((player->get_buildcaps(unusable_fields.front())
			 & BUILDCAPS_SIZEMASK) != 0)
		{

			buildable_fields.push_back
				(new BuildableField(unusable_fields.front()));
			unusable_fields.pop_front();

			update_buildable_field (*buildable_fields.back());
			continue;
		}

		if
			((player->get_buildcaps(unusable_fields.front())
			 & BUILDCAPS_MINE) != 0)
		{

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
void DefaultAI::update_buildable_field (BuildableField & field)
{
	// look if there is any unowned land nearby
	FindNodeUnowned find_unowned(get_player_number());
	Map & map = game().map();

	field.unowned_land_nearby =
		map.find_fields(Area<FCoords>(field.coords, 7), 0, find_unowned);

	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;

	int32_t const tree_attr  = Map_Object_Descr::get_attribute_id("tree");
	int32_t const stone_attr = Map_Object_Descr::get_attribute_id("stone");

	// Search in a radius of 7
	map.find_immovables (Area<FCoords>(field.coords, 7), &immovables);

	field.reachable      = false;
	field.preferred      = false;
	field.avoid_military = false;

	field.military_influence     = 0;
	field.trees_nearby           = 0;
	field.stones_nearby          = 0;
	field.tree_consumers_nearby  = 0;
	field.stone_consumers_nearby = 0;
	field.producers_nearby.clear();
	field.producers_nearby.resize(wares.size());
	field.consumers_nearby.clear();
	field.consumers_nearby.resize(wares.size());
	std::vector<Coords> water_list;
	FindNodeWater find_water;
	map.find_fields(Area<FCoords>(field.coords, 4), &water_list, find_water);
	field.water_nearby = water_list.size();

	FCoords fse;
	map.get_neighbour (field.coords, Map_Object::WALK_SE, &fse);

	if (BaseImmovable const * const imm = fse.field->get_immovable())
		if
			(dynamic_cast<Flag const *>(imm)
			 or
			 (dynamic_cast<Road const *>(imm)
			  &&
			  fse.field->get_caps() & BUILDCAPS_FLAG))
		field.preferred = true;

	for (uint32_t i = 0; i < immovables.size(); ++i) {
		const BaseImmovable & base_immovable = *immovables[i].object;
		if (dynamic_cast<const Flag *>(&base_immovable))
			field.reachable = true;
		if (upcast(PlayerImmovable const, player_immovable, &base_immovable))
			if
				(player_immovable->owner().get_player_number()
				 != get_player_number())
				continue;

		if (upcast(Building const, building, &base_immovable)) {

			if (upcast(ConstructionSite const, constructionsite, building)) {
				const Building_Descr & target_descr = constructionsite->building();

				if (upcast(MilitarySite_Descr const, target_ms_d, &target_descr)) {
					const int32_t v =
						target_ms_d->get_conquers()
						-
						map.calc_distance(field.coords, immovables[i].coords);

					if (0 < v) {
						field.military_influence += v * (v + 2) * 6;
						field.avoid_military = true;
					}
				}

				if (dynamic_cast<ProductionSite_Descr const *>(&target_descr))
					consider_productionsite_influence
						(field,
						 immovables[i].coords,
						 get_building_observer(constructionsite->name().c_str()));
			}

			if (upcast(MilitarySite const, militarysite, building)) {
				const int32_t v =
					militarysite->get_conquers()
					-
					map.calc_distance(field.coords, immovables[i].coords);

				if (v > 0)
					field.military_influence +=
						v * v * militarysite->soldierCapacity();
			}

			if (dynamic_cast<const ProductionSite *>(building))
				consider_productionsite_influence
					(field,
					 immovables[i].coords,
					 get_building_observer(building->name().c_str()));

			continue;
		}

		if (immovables[i].object->has_attribute(tree_attr))
			++field.trees_nearby;

		if (immovables[i].object->has_attribute(stone_attr))
			++field.stones_nearby;
	}
}

/// Updates one mineable field
void DefaultAI::update_mineable_field (MineableField & field)
{
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	Map & map = game().map();

	map.find_immovables (Area<FCoords>(field.coords, 6), &immovables);

	field.reachable    = false;
	field.preferred    = false;
	field.mines_nearby = 1;

	FCoords fse;
	map.get_neighbour (field.coords, Map_Object::WALK_SE, &fse);

	if (BaseImmovable const * const imm = fse.field->get_immovable())
		if
			(dynamic_cast<Flag const *>(imm)
			 or
			 (dynamic_cast<Road const *>(imm)
			  &&
			  fse.field->get_caps() & BUILDCAPS_FLAG))
		field.preferred = true;

	for (uint32_t i = 0; i < immovables.size(); ++i) {
		if (dynamic_cast<Flag const *>(immovables[i].object))
			field.reachable = true;
		else if (upcast(Building const, bld, immovables[i].object))
			if
				(player->get_buildcaps(map.get_fcoords(immovables[i].coords))
				 &
				 BUILDCAPS_MINE)
			{

			if
				(dynamic_cast<ConstructionSite const *>(bld) or
				 dynamic_cast<ProductionSite   const *>(bld))
				++field.mines_nearby;
			}
	}
}


/**
 * constructs the most needed building
 *
 * \ToDo: this function holds a lot of calculations that are hard to understand
 * at first and even second view - explain what's going on here
 */
bool DefaultAI::construct_building (int32_t) // (int32_t gametime)
{
	//  Just used for easy checking whether a mine or something else was built.
	bool mine = false;

	int32_t spots_avail[4];

	for (int32_t i = 0; i < 4; ++i)
		spots_avail[i] = 0;

	for
		(std::list<BuildableField *>::iterator i = buildable_fields.begin();
		 i != buildable_fields.end();
		 ++i)
		++spots_avail[(*i)->coords.field->get_caps() & BUILDCAPS_SIZEMASK];

	int32_t expand_factor = 1;

	// check space and set the need for expansion
	if (spots_avail[BUILDCAPS_BIG] < 2)
		expand_factor *= 2;
	if (spots_avail[BUILDCAPS_MEDIUM] + spots_avail[BUILDCAPS_BIG] < 4)
		expand_factor *= 2;
	if
		(spots_avail[BUILDCAPS_SMALL]  +
		 spots_avail[BUILDCAPS_MEDIUM] +
		 spots_avail[BUILDCAPS_BIG]
		 <
		 8)
		expand_factor *= 2;

	Building_Index proposed_building;
	int32_t proposed_priority = 0;
	Coords proposed_coords;

	// first scan all buildable fields for regular buildings
	for
		(std::list<BuildableField *>::iterator i = buildable_fields.begin();
		 i != buildable_fields.end();
		 ++i)
	{
		BuildableField * const bf = *i;

		if (!bf->reachable)
			continue;

		int32_t const maxsize =
			player->get_buildcaps(bf->coords) & BUILDCAPS_SIZEMASK;

		for
			(std::list<BuildingObserver>::iterator j = buildings.begin();
			 j != buildings.end();
			 ++j)
		{
			if (!j->is_buildable)
				continue;

			if (j->type == BuildingObserver::MINE)
				continue;

			if (j->desc->get_size() > maxsize)
				continue;

			int32_t prio = 0;

			if (j->type == BuildingObserver::PRODUCTIONSITE) {
				if (j->need_trees) {
					// Priority of woodcutters depend on the number of near trees
					prio += bf->trees_nearby * 5 / 3;
					prio /= 2 * (1 + bf->tree_consumers_nearby);
					if (j->total_count() < 2)
						prio *= 4; // big bonus for the basics
					if (j->total_count() == 0)
						prio *= 2; // even more for the absolute basics
				} else if (j->need_stones) {
					// Priority of quarries depend on the number of near stones
					prio += bf->stones_nearby * 5 / 3;
					prio /= 2 * (1 + bf->stone_consumers_nearby);
					if (j->total_count() == 0)
						prio *= 8; // big bonus for the basics
				} else if (j->production_hint >= 0) {
					// production hint (f.e. associate forester with trunks)
					// add bonus near buildings outputting production_hint ware
					prio += 2 * bf->producers_nearby[j->production_hint];

					// Do not build too many of these buildings, but still care
					// to build at least one.
					if (j->total_count() > 1)
						prio -= 10 * (j->total_count());
					else
						prio += 5;

					if (prio < 0)
						continue;

					// Calculate the need for this building
					prio += 4 * wares[j->production_hint].consumers;
					prio += wares[j->production_hint].preciousness;
				} else {
					// Check if the produced wares are needed
					container_iterate(std::list<EconomyObserver *>, economies, l) {
						for (uint32_t m = 0; m < j->outputs.size(); ++m) {
							Ware_Index wt(static_cast<size_t>(j->outputs[m]));
							if ((*l.current)->economy.needs_ware(wt)) {
								prio += 1 + wares[j->outputs[m]].preciousness;
								if (j->total_count() == 0)
									prio *= 4; // big bonus, this site might be elemental
							}
						}
					}

					// If the produced wares are needed
					if (prio > 0) {
						prio = calculate_need_for_ps(*j, prio);

						int32_t iosum = 0;
						for (size_t k = 0; k < j->inputs.size(); ++k)
							if (bf->producers_nearby[j->inputs[k]]>0)
								++iosum;
							else if (bf->consumers_nearby[j->inputs[k]]>0)
								--iosum;
						if (iosum < -2)
							iosum = -2;
						for (size_t k = 0; k < j->outputs.size(); ++k)
							if (bf->consumers_nearby[j->outputs[k]] > 0)
								++iosum;
						prio += 2 * iosum;
					}

					// do not construct more than one building,
					// if supply line is already broken.
					if (!check_supply(*j) && j->total_count() > 0)
						prio -= 12;

				}
			} else if (j->type == BuildingObserver::MILITARYSITE) {
				prio  = bf->unowned_land_nearby - bf->military_influence * 4;
				prio  = prio > 0 ? prio : 1;
				prio *= expand_factor;
				prio /= 2;

				if (bf->avoid_military) {
					prio /= 5;
				}

			} else if (j->type == BuildingObserver::WAREHOUSE) {
				// Build one warehouse (hq included) for ~every 25 productionsites
				prio += productionsites.size();
				prio -= (j->cnt_under_construction + numof_warehouses) * 25;
			}

			// avoid to have too many construction sites
			prio -=
				2 * j->cnt_under_construction * (j->cnt_under_construction + 1);

			// add big penalty if water is needed, but is not near
			if (j->need_water) {
				int effect = bf->water_nearby - 12;
				prio += effect > 0 ? static_cast<int>(sqrt(effect)) : effect;
				// if same producers are nearby, then give some penalty
				for (size_t k = 0; k < j->outputs.size(); ++k)
					if (bf->producers_nearby[j->outputs[k]] > 0)
						prio -= 3;
			}

			// Prefer road side fields
			prio += bf->preferred ?  1 : 0;

			// don't waste good land for small huts
			prio -= (maxsize - j->desc->get_size()) * 3;
			if (prio > proposed_priority) {
				proposed_building = j->id;
				proposed_priority = prio;
				proposed_coords   = bf->coords;
			}
		}
	}

	// then try all mines
	for
		(std::list<BuildingObserver>::iterator i = buildings.begin();
		 i != buildings.end();
		 ++i)
	{

		if (!i->is_buildable || i->type != BuildingObserver::MINE)
			continue;

		for
			(std::list<MineableField *>::iterator j = mineable_fields.begin();
			 j != mineable_fields.end();
			 ++j)
		{

			MineableField * mf = *j;
			int32_t prio = -1;

			if (mf->coords.field->get_resources() != i->mines)
				continue;
			else
				prio += mf->coords.field->get_resources_amount() * 2 / 3;

			// If the produced wares are needed, check if current economy can
			// supply enough material for production.
			for (uint32_t k = 0; k < i->inputs.size(); ++k) {
				prio += 2 * wares[i->inputs[k]].producers;
				prio -= 6 * wares[i->inputs[k]].consumers;
			}

			// Check if the produced wares are needed
			container_iterate(std::list<EconomyObserver *>, economies, l) {
				for (uint32_t m = 0; m < i->outputs.size(); ++m) {
					Ware_Index wt(static_cast<size_t>(i->outputs[m]));
					if ((*l.current)->economy.needs_ware(wt)) {
						prio *= 2;
					}
				}
			}

			prio -= 2 * mf->mines_nearby * mf->mines_nearby;
			prio /= 1 + i->cnt_built * 3;
			prio /= 1 + i->cnt_under_construction * 7;

			if (prio > proposed_priority) {
				proposed_building = i->id;
				proposed_priority = prio;
				proposed_coords = mf->coords;
				mine = true;
			}
		}
	}

	if (not proposed_building)
		return false;

	// do not have too many construction sites
	if (proposed_priority < total_constructionsites * total_constructionsites)
		return false;

	// send the command to construct a new building
	game().send_player_build
		(get_player_number(), proposed_coords, proposed_building);

	// set the type of update that is needed
	if (mine)
		m_mineable_changed = true;
	else
		m_buildable_changed = true;

	return true;
}

/**
 * This function searches for places where a new road is needed to connect two
 * economies. It then sends the request to build the road.
 */
bool DefaultAI::construct_roads (int32_t gametime)
{
	if (economies.size() < 2) {
		// only one economy, no need for new roads
		return false;
	}

	uint32_t economies_to_connect = 0;
	EconomyObserver * eo_to_connect;

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

	// Try to connect - this should work fine as in nearly all cases we simply
	// connect a constructionsite
	bool done = connect_flag_to_another_economy(*eo_to_connect->flags.front());
	eo_to_connect->flags.push_back(eo_to_connect->flags.front());
	eo_to_connect->flags.pop_front();

	if (done)
		return true;

	// Unable to connect, so we let this economy wait for 30 seconds if it fails.
	eo_to_connect->next_connection_try = gametime + 30000;
	return false;
}

/// improves current road system
bool DefaultAI::improve_roads (int32_t gametime)
{
	// Remove dead end roads
	container_iterate(std::list<EconomyObserver *>, economies, i)
		container_iterate(std::list<Flag const *>, (*i.current)->flags, j)
			if ((*j.current)->is_dead_end()) {
				game().send_player_bulldoze(*const_cast<Flag *>((*j.current)));
				j.current = (*i.current)->flags.erase(j.current);
				return true;
			}

	// force a split on roads that are longer than 3 parts
	// actually we do not care for loss of building capabilities - normal maps
	// should have enough space and the computer can expand it's territory.
	if (!roads.empty()) {
		Path const & path = roads.front()->get_path();

		if (path.get_nsteps() > 3) {
			const Map & map = game().map();
			CoordPath cp(map, path);

			// try to split near the middle
			CoordPath::Step_Vector::size_type i = cp.get_nsteps() / 2, j = i + 1;
			for (; i > 1; --i, ++j) {
				{
					const Coords c = cp.get_coords()[i];
					if (map[c].get_caps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag (get_player_number(), c);
						return true;
					}
				}
				{
					const Coords c = cp.get_coords()[j];
					if (map[c].get_caps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag (get_player_number(), c);
						return true;
					}
				}
			}
			// Unable to set a flag - perhaps the road was build stupid
			game().send_player_bulldoze (*const_cast<Road *>(roads.front()));
		}

		roads.push_back (roads.front());
		roads.pop_front ();
	}

	if (!economies.empty() & (inhibit_road_building <= gametime)) {
		EconomyObserver * eco = economies.front();
		bool finish = false;

		// try to connect to another economy
		if (economies.size() > 1)
			finish = connect_flag_to_another_economy(*eco->flags.front());

		if (!finish)
			finish = improve_transportation_ways(*eco->flags.front());

		// cycle through flags one at a time
		eco->flags.push_back(eco->flags.front());
		eco->flags.pop_front();

		// and cycle through economies
		economies.push_back(eco);
		economies.pop_front();

		if (finish)
			return true;
	}

	return false;
}


/// connects a specific flag to another economy
bool DefaultAI::connect_flag_to_another_economy (const Flag & flag)
{
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

	// then choose the one closest to the originating flag
	int32_t closest_distance = std::numeric_limits<int32_t>::max();
	Coords closest;
	container_iterate_const(std::vector<Coords>, reachable, i) {
		int32_t const distance =
			map.calc_distance(flag.get_position(), *i.current);
		if (distance < closest_distance) {
			closest = *i.current;
			closest_distance = distance;
		}
	}
	assert(closest_distance != std::numeric_limits<int32_t>::max());

	// if we join a road and there is no flag yet, build one
	if (dynamic_cast<const Road *> (map[closest].get_immovable()))
		game().send_player_build_flag (get_player_number(), closest);

	// and finally build the road
	Path & path = *new Path();
	check.set_openend (false);
	if (map.findpath(flag.get_position(), closest, 0, path, check) < 0) {
		delete &path;
		return false;
	}

	game().send_player_build_road (get_player_number(), path);
	return true;
}

/// adds alternative ways to already existing ones
bool DefaultAI::improve_transportation_ways (const Flag & flag)
{
	std::priority_queue<NearFlag> queue;
	std::vector<NearFlag> nearflags;

	queue.push (NearFlag(flag, 0, 0));
	Map & map = game().map();

	while (!queue.empty()) {
		std::vector<NearFlag>::iterator f =
			find(nearflags.begin(), nearflags.end(), queue.top().flag);
		if (f != nearflags.end()) {
			queue.pop ();
			continue;
		}

		nearflags.push_back (queue.top());
		queue.pop ();

		NearFlag & nf = nearflags.back();

		for (uint8_t i = 1; i <= 6; ++i) {
		Road * const road = nf.flag->get_road(i);

		if (!road) continue;

		Flag * endflag = &road->get_flag(Road::FlagStart);
		if (endflag == nf.flag)
			endflag = &road->get_flag(Road::FlagEnd);

			int32_t dist =
				map.calc_distance(flag.get_position(), endflag->get_position());
		if (dist > 16) //  out of range
			continue;

			queue.push
				(NearFlag
				 	(*endflag, nf.cost + road->get_path().get_nsteps(), dist));
		}
	}

	std::sort (nearflags.begin(), nearflags.end(), CompareDistance());

	CheckStepRoadAI check(player, MOVECAPS_WALK, false);

	for (uint32_t i = 1; i < nearflags.size(); ++i) {
		NearFlag & nf = nearflags[i];

		if (2 * nf.distance + 2 < nf.cost) {

			Path & path = *new Path();
			if
				(map.findpath
				 	(flag.get_position(), nf.flag->get_position(), 0, path, check)
				 >=
				 0
				 and
				 static_cast<int32_t>(2 * path.get_nsteps() + 2) < nf.cost)
			{
				game().send_player_build_road (get_player_number(), path);
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
bool DefaultAI::check_economies ()
{
	while (!new_flags.empty()) {
		Flag const & flag = *new_flags.front();
		new_flags.pop_front();

		get_economy_observer(flag.economy())->flags.push_back (&flag);
	}

	container_iterate(std::list<EconomyObserver *>, economies, i) {
		// check if any flag has changed its economy
		std::list<Flag const *> &fl = (*i.current)->flags;
		for (std::list<Flag const *>::iterator j = fl.begin();j != fl.end();) {
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
bool DefaultAI::check_productionsites(int32_t gametime)
{
	if ((next_productionsite_check_due > gametime) || productionsites.empty())
		return false;
	next_productionsite_check_due = gametime + 5000;

	// Get link to productionsite that should be checked
	ProductionSiteObserver & site = productionsites.front();
	bool changed = false;

	// Get max radius of recursive workarea
	Workarea_Info::size_type radius = 0;

	Workarea_Info const & workarea_info = site.bo->desc->m_workarea_info;
	container_iterate_const(Workarea_Info, workarea_info, i)
		if (radius < i.current->first)
			radius = i.current->first;

	Map & map = game().map();

	// Lumberjack / Woodcutter handling
	if
		(site.bo->need_trees
		 and
		 map.find_immovables
		 	(Area<FCoords>(map.get_fcoords(site.site->get_position()), radius),
		 	 0,
		 	 FindImmovableAttribute(Map_Object_Descr::get_attribute_id("tree")))
		 ==
		 0)
	{
		if (site.site->get_statistics_percent() == 0) {
			game().send_player_bulldoze (*site.site);
			return true;
		}
	}

	// Quarry handling
	if
		(site.bo->need_stones
		 and
		 map.find_immovables
		 	(Area<FCoords>(map.get_fcoords(site.site->get_position()), radius),
		 	 0,
		 	 FindImmovableAttribute(Map_Object_Descr::get_attribute_id("stone")))
		 ==
		 0)
	{
		game().send_player_bulldoze (*site.site);
		return true;
	}

	// Check whether building is enhanceable and if wares of the enhanced
	// buildings are needed. If yes consider an upgrade.
	std::set<Building_Index> enhancements = site.site->enhancements();
	int32_t maxprio = 0;
	Building_Index enbld;
	container_iterate_const(std::set<Building_Index>, enhancements, x) {
		// Only enhance buildings that are allowed (scenario mode)
		if (player->is_building_allowed((*x.current))) {
			const Building_Descr & bld = *tribe->get_building_descr((*x.current));
			BuildingObserver & en_bo = get_building_observer(bld.name().c_str());

			// Don't enhance this building, if there is already one of same type
			// under construction
			if (en_bo.cnt_under_construction > 0)
				continue;

			//if (site.site->workers()

			int32_t prio = 0; // priority for enhancement

			// Find new outputs of enhanced building
			std::vector<int16_t> & current_outputs = site.bo->outputs;
			std::vector<int16_t> new_outputs;
			for (uint16_t i = 0; i < en_bo.outputs.size(); ++i) {
				for (uint16_t j = 0; j < current_outputs.size(); ++j)
					if (current_outputs[j] == en_bo.outputs[i]) {
						Ware_Index wt(static_cast<size_t>(current_outputs[j]));
						if (site.site->economy().needs_ware(wt))
							prio -= (2 + wares[current_outputs[j]].preciousness) / 2;
						continue;
					}
				new_outputs.push_back(static_cast<int16_t>(i));
			}

			// Check if the new wares are needed in economy of the building
			for (uint32_t i = 0; i < new_outputs.size(); ++i) {
				Ware_Index wt(static_cast<size_t>(new_outputs[i]));
				if (site.site->economy().needs_ware(wt))
					prio += 2 + wares[new_outputs[i]].preciousness;
			}

			// Compare the number of buildings of current type with the number
			// of buildings of enhanced type
			prio += (site.bo->total_count() - en_bo.total_count()) * 2;

			// If the new wares are needed
			if (prio > 0) {
				prio = calculate_need_for_ps(en_bo, prio);
				if (prio > maxprio) {
					maxprio = prio;
					enbld = (*x.current);
				}
			}
		}
	}

	// Enhance if enhanced building is useful
	if (maxprio > 0) {
		game().send_player_enhance_building(*site.site, enbld);
		changed = true;
	}

	// Reorder and set new values;
	productionsites.push_back(productionsites.front());
	productionsites.pop_front();
	return changed;
}

/**
 * Updates the first military building in list and reenques it at the end of
 * the list afterwards. If a militarysite is in secure area but holds more than
 * one soldier, the number of stationed soldiers is decreased. If the building
 * is near a border, the number of stationed soldiers is maximized
 *
 * \returns true if something was changed
 */
bool DefaultAI::check_militarysites  (int32_t gametime)
{
	if (next_militarysite_check_due > gametime)
		return false;

	// Only useable, if defaultAI owns at least one militarysite
	if (militarysites.empty())
		return false;

	// Check next militarysite
	bool changed = false;
	Map & map = game().map();
	uint16_t pn = get_player_number();
	MilitarySite * ms = militarysites.front().site;
	uint32_t vision = ms->vision_range();
	FCoords f = map.get_fcoords(ms->get_position());

	// look if there is any enemy land nearby
	FindNodeUnowned find_unowned(pn, true);

	if (map.find_fields(Area<FCoords>(f, vision), 0, find_unowned) == 0) {
		// If no enemy in sight - decrease the number of stationed soldiers
		// as long as it is > 1
		if (ms->economy().get_nr_warehouses() > 0) {
			uint32_t j = ms->soldierCapacity();
			if (j > 1)
				game().send_player_change_soldier_capacity(*ms, -1);
		}
	} else {
		// If an enemy is in sight and the number of stationed soldier is not
		// at maximum - set it to maximum.
		uint32_t j = ms->maxSoldierCapacity();
		uint32_t k = ms->soldierCapacity();
		if (j > k)
			game().send_player_change_soldier_capacity(*ms, j - k);
		changed = true;
	}
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();
	next_militarysite_check_due = gametime + 1000;
	return changed;
}


/**
 * calculates how much a productionsite of type \arg bo is needed inside it's
 * economy. \arg prio is initial value for this calculation
 *
 * \returns the calculated priority
 */
int32_t DefaultAI::calculate_need_for_ps(BuildingObserver & bo, int32_t prio)
{
	// some randomness to avoid that defaultAI is building always
	// the same (always == another game but same map with
	// defaultAI on same coords)
	prio += time(0) % 3 - 1;

	// check if current economy can supply enough material for
	// production.
	for (uint32_t k = 0; k < bo.inputs.size(); ++k) {
		prio += 2 * wares[bo.inputs[k]].producers;
		prio -= 4 * wares[bo.inputs[k]].consumers;
	}

	int32_t output_prio = 0;
	for (uint32_t k = 0; k < bo.outputs.size(); ++k) {
		WareObserver & wo = wares[bo.outputs[k]];
		if (bo.total_count() == 0 && wo.consumers > 0)
			output_prio += 8 + wo.preciousness; // add a big bonus
	}

	if (bo.outputs.size() > 0)
		output_prio = static_cast<int32_t>
			(ceil(output_prio / sqrt(bo.outputs.size())));
	prio += output_prio;

	return prio;
}


void DefaultAI::consider_productionsite_influence
	(BuildableField & field, Coords, BuildingObserver const & bo)
{
	if (bo.need_trees)
		++field.tree_consumers_nearby;

	if (bo.need_stones)
		++field.stone_consumers_nearby;
	for (size_t i = 0; i < bo.inputs.size(); ++i)
		++field.consumers_nearby[bo.inputs[i]];
	for (size_t i = 0; i < bo.outputs.size(); ++i)
		++field.producers_nearby[bo.outputs[i]];
}


/// \returns the economy observer containing \arg economy
EconomyObserver * DefaultAI::get_economy_observer
	(Economy & economy)
{
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
BuildingObserver & DefaultAI::get_building_observer
	(char const * const name)
{
	if (tribe == 0)
		late_initialization ();

	for
		(std::list<BuildingObserver>::iterator i = buildings.begin();
		 i != buildings.end();
		 ++i)
		if (!strcmp(i->name, name))
			return *i;

	throw wexception("Help: I do not know what to do with a %s", name);
}


/// this is called whenever we gain ownership of a PlayerImmovable
void DefaultAI::gain_immovable (PlayerImmovable & pi)
{
	if      (upcast(Building,       building, &pi))
		gain_building (*building);
	else if (upcast(Flag     const, flag,     &pi))
		new_flags.push_back  (flag);
	else if (upcast(Road     const, road,     &pi))
		roads    .push_front (road);
}

/// this is called whenever we lose ownership of a PlayerImmovable
void DefaultAI::lose_immovable (PlayerImmovable const & pi)
{
	if        (upcast(Building const, building, &pi))
		lose_building (*building);
	else if   (upcast(Flag     const, flag,     &pi)) {
		container_iterate_const(std::list<EconomyObserver *>, economies, i)
			container_iterate(std::list<Flag const *>, (*i.current)->flags, j)
				if (*j.current == flag) {
					(*i.current)->flags.erase (j.current);
					return;
				}
	} else if (upcast(Road     const, road,     &pi))
		roads.remove (road);
}

/// this is called whenever we gain a new building
void DefaultAI::gain_building (Building & b)
{
	BuildingObserver & bo = get_building_observer(b.name().c_str());

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver & target_bo =
			get_building_observer
				(dynamic_cast<ConstructionSite &>(b)
				 .building().name().c_str());
		++target_bo.cnt_under_construction;
		++total_constructionsites;
	}
	else {
		++bo.cnt_built;

		if (bo.type == BuildingObserver::PRODUCTIONSITE) {
			productionsites.push_back (ProductionSiteObserver());
			productionsites.back().site = &dynamic_cast<ProductionSite &>(b);
			productionsites.back().bo = &bo;

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				++wares[bo.outputs[i]].producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				++wares[bo.inputs[i]].consumers;
		} else if (bo.type == BuildingObserver::MILITARYSITE) {
			militarysites.push_back (MilitarySiteObserver());
			militarysites.back().site = &dynamic_cast<MilitarySite &>(b);
			militarysites.back().bo = &bo;
		} else if (bo.type == BuildingObserver::WAREHOUSE)
			++numof_warehouses;
	}
}

/// this is called whenever we lose a building
void DefaultAI::lose_building (Building const & b)
{
	BuildingObserver & bo = get_building_observer(b.name().c_str());

	if (bo.type == BuildingObserver::CONSTRUCTIONSITE) {
		BuildingObserver &target_bo =
			get_building_observer
				(dynamic_cast<ConstructionSite const &>(b)
				 .building().name().c_str());
		--target_bo.cnt_under_construction;
		--total_constructionsites;
	}
	else {
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
				--wares[bo.outputs[i]].producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				--wares[bo.inputs[i]].consumers;
		} else if (bo.type == BuildingObserver::MILITARYSITE) {
			for
				(std::list<MilitarySiteObserver>::iterator i =
				 militarysites.begin();
				 i != militarysites.end();
				 ++i)
				if (i->site == &b) {
					militarysites.erase(i);
					break;
				}
		} else if (bo.type == BuildingObserver::WAREHOUSE) {
			assert(numof_warehouses > 0);
			--numof_warehouses;
		}
	}
	m_buildable_changed = true;
	m_mineable_changed = true;
}


/// Checks that supply line exists for given building.
/// Recurcsively verify that all inputs have a producer.
bool DefaultAI::check_supply(BuildingObserver const &bo)
{
	size_t supplied = 0;
	container_iterate_const(std::vector<int16_t>, bo.inputs, i)
		container_iterate_const(std::list<BuildingObserver>, buildings, j)
			if
				(j.current->cnt_built &&
				 std::find
				 	(j.current->outputs.begin(), j.current->outputs.end(),
				 	 *i.current)
				 !=
				 j.current->outputs.end()
				 &&
				 check_supply(*j.current))
			{
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
bool DefaultAI::consider_attack(int32_t gametime) {
	// Only useable, if it owns at least one militarysite
	if (militarysites.empty())
		return false;

	Map & map = game().map();
	uint16_t pn = get_player_number();

	Building * target;
	int32_t    chance = 0;
	int32_t    attackers = 0;

	// Check next militarysite
	MilitarySite * ms = militarysites.front().site;
	uint32_t vision = ms->vision_range();
	FCoords f = map.get_fcoords(ms->get_position());

	// Search in a radius of the vision of the militarysite and collect
	// information about immovables in the area
	std::vector<ImmovableFound> immovables;
	map.find_immovables
		(Area<FCoords>(f, vision), &immovables, FindImmovableAttackable());

	for (uint32_t j = 0; j < immovables.size(); ++j) {
		if (upcast(MilitarySite, bld, immovables[j].object)) {
			if (bld->owner().get_player_number() == pn)
				continue;
			if (bld->canAttack()) {
				int32_t ta = player->findAttackSoldiers(bld->base_flag());
				if (ta < 1)
					continue;

				int32_t tc = ta - bld->presentSoldiers().size();
				if (tc > chance) {
					target = bld;
					chance = tc;
					attackers = ta;
				}
			}
		} else if (upcast(Warehouse, wh, immovables[j].object)) {
			if (wh->owner().get_player_number() == pn)
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
	}

	// Reenque militarysite at the end of list
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();

	// Return if chance to win is too low
	if (chance < 3) {
		next_attack_consideration_due = (gametime % 7) * 1000 + gametime;
		return false;
	}

	// Attack the selected target.
	game().send_player_enemyflagaction(target->base_flag(), pn, attackers);

	// Don't attack to fast again - homecoming soldiers must get healed first
	next_attack_consideration_due = (gametime % 51 + 10) * 1000 + gametime;
	return true;
}
