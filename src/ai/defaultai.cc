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

#include "computer_player_hints.h"
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
tribe(0)
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

	// verify that our production sites are doing well
	if
		(next_productionsite_check_due <= gametime
		 and
		 not productionsites.empty())
	{
		next_productionsite_check_due = gametime + 2000;

		if (check_productionsite(productionsites.front())) {
			m_buildable_changed = true;
			m_mineable_changed = true;
		}

		productionsites.push_back (productionsites.front());
		productionsites.pop_front ();
	}

	// check if anything in the economies changed.
	// This needs to be done before new buildings are placed, to ensure that no
	// empty economy is left.
	check_economies();

	// improve existing roads
	if (improve_roads()) {
		m_buildable_changed = true;
		m_mineable_changed = true;
		return;
	}

	// build some roads if needed
	if (next_road_due <= gametime) {
		next_road_due = gametime + 1000;
		if(construct_roads ()) {
			m_buildable_changed = true;
			m_mineable_changed = true;
			return;
		}
	} else
		return; // don't build new buildings as long as the cp has not tried to
		        // connect all economies.

	// finally try to build something if possible
	if (next_construction_due <= gametime) {
		next_construction_due = gametime + 2000;
		if (construct_building()) {
			//inhibit_road_building = gametime + 2500;
			//Inhibiting roadbuilding is not a good idea, it causes
			//computer players to get into deadlock at certain circumstances.
			return;
		}
	}
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

	// Building hints for computer player
	std::string stoneproducer = "quarry";
	std::string trunkproducer = "lumberjack";
	std::string forester      = "forester";
	std::string fisher        = "fisher";
	std::string coalmine      = "coalmine";
	std::string oremine       = "oremine";
	std::string goldmine      = "goldmine";
	std::string granitmine    = "granitmine";

	// Read the computerplayer hints of the tribe
	// FIXME: this is only a temporary workaround. Better define all this stuff
	//        in each buildings conf-file.
	std::string tribehints = "tribes/" + tribe->name() + "/cphints";
	if (g_fs->FileExists(tribehints)) {
		Profile prof(tribehints.c_str());
		Section & hints = prof.get_safe_section("global");

		stoneproducer = hints.get_safe_string("stoneproducer");
		trunkproducer = hints.get_safe_string("trunkproducer");
		forester      = hints.get_safe_string("forester");
		fisher        = hints.get_safe_string("fisher");
		coalmine      = hints.get_safe_string("coalmine");
		oremine       = hints.get_safe_string("oremine");
		goldmine      = hints.get_safe_string("goldmine");
		granitmine    = hints.get_safe_string("granitmine");

	} else {
		log("   WARNING: No computerplayer hints for tribe %s found\n", tribe->name().c_str());
		log("   This will lead to stupid behaviour of said player!\n");
	}

	// collect information about which buildings our tribe can construct
	Building_Index const nr_buildings = tribe->get_nrbuildings();
	const World & world = game().map().world();
	for (Building_Index i = Building_Index::First(); i < nr_buildings; ++i) {
		const Building_Descr & bld = *tribe->get_building_descr(i);
		const std::string & building_name = bld.name();

		buildings.resize (buildings.size() + 1);

		BuildingObserver & bo     = buildings.back();
		bo.name                   = building_name.c_str();
		bo.id                     = i;
		bo.desc                   = &bld;
		bo.hints                  = &bld.hints();
		bo.type                   = BuildingObserver::BORING;
		bo.cnt_built              = 0;
		bo.cnt_under_construction = 0;
		bo.production_hint        = -1;

		bo.is_buildable = bld.buildable() & player->is_building_allowed(i);

		bo.need_trees             = building_name == trunkproducer;
		bo.need_stones            = building_name == stoneproducer;
		if (building_name == forester)
			bo.production_hint = tribe->safe_ware_index("trunk").value();
		bo.need_water = (building_name == fisher);

		// mines
		if (building_name == coalmine)
			bo.mines = world.get_resource("coal");
		if (building_name == oremine)
			bo.mines = world.get_resource("iron");
		if (building_name == goldmine)
			bo.mines = world.get_resource("gold");
		if (building_name == granitmine)
			bo.mines = world.get_resource("granit");

		if (typeid(bld) == typeid(ConstructionSite_Descr)) {
			bo.type = BuildingObserver::CONSTRUCTIONSITE;
			continue;
		}

		if (typeid(bld) == typeid(MilitarySite_Descr)) {
			bo.type = BuildingObserver::MILITARYSITE;
			continue;
		}

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

			unusable_fields.push_back (map.get_fcoords(Coords(x, y)));

			if (upcast(PlayerImmovable, imm, f.field->get_immovable()))
				// Guard by a set because immovables might be on several fields at once
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
 * Checks ALL fields that weren't buildable the last time.
 *
 * this might hang the game for some 100
 * milliseconds if the area the computer owns is big.
 */
void DefaultAI::update_all_not_buildable_fields()
{
	for
		(std::list<FCoords>::iterator i = unusable_fields.begin();
		 i != unusable_fields.end();)
	{
		//  check whether we lost ownership of the node
		if (i->field->get_owned_by() != get_player_number()) {
			i = unusable_fields.erase(i);
			continue;
		}

		// check whether building capabilities have improved
		if ((player->get_buildcaps(*i) & BUILDCAPS_SIZEMASK) != 0) {
			buildable_fields.push_back (new BuildableField(*i));
			i = unusable_fields.erase(i);

			update_buildable_field (*buildable_fields.back());
			continue;
		}

		if ((player->get_buildcaps(*i) & BUILDCAPS_MINE) != 0) {
			mineable_fields.push_back (new MineableField(*i));
			i = unusable_fields.erase(i);

			update_mineable_field (*mineable_fields.back());
			continue;
		}

		++i;
	}
}


/// Updates one buildable field
void DefaultAI::update_buildable_field (BuildableField & field)
{
	// look if there is any unowned land nearby
	FindNodeUnowned find_unowned;
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
			if (player_immovable->owner().get_player_number() != get_player_number())
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
	field.mines_nearby = true;

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
bool DefaultAI::construct_building ()
{
	bool mine = false; // just used for easy checking whether a mine or something
	                   // else was build.

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

			if (j->type == BuildingObserver::MILITARYSITE) {
				prio  = bf->unowned_land_nearby - bf->military_influence * 4;
				prio  = prio > 0 ? prio : 1;
				prio *= expand_factor;
				prio /= 2;

				if (bf->avoid_military) {
					prio /= 5;
				}

			}

			if (j->type == BuildingObserver::PRODUCTIONSITE) {
				if (j->need_trees) {
					prio += bf->trees_nearby - 2;
					prio /= 2 * (1 + bf->tree_consumers_nearby);
					if (j->total_count() == 0)
						prio *= 8; // big bonus for the basics
				} else if (j->need_stones) {
					prio +=
						bf->stones_nearby * 3 / 2 - 6 * bf->stone_consumers_nearby;
					if (j->total_count() == 0)
						prio *= 8; // big bonus for the basics
				} else if (j->production_hint >= 0) {
					// production hint associates forester with trunk production
					prio -= 6 * (j->cnt_built + j->cnt_under_construction);
					prio += 4 * wares[j->production_hint].consumers;
					prio += 2 * wares[j->production_hint].preciousness;

					// add bonus near buildings outputting production_hint ware
					if (bf->producers_nearby[j->production_hint] > 0)
						prio += 2;
				} else {
					if (j->total_count() == 0)
						prio += 2;

					// don't make more than one building, if supply line is broken.
					if (!check_supply(*j) && j->total_count() > 0)
						prio -= 12;

					// Check if the produced wares are needed
						container_iterate(std::list<EconomyObserver *>, economies, l) {
						for (uint32_t m = 0; m < j->outputs.size(); ++m) {
							Ware_Index wt(static_cast<size_t>(j->outputs[m]));
							//FIXME segfault in economy.h -> target_quantity(Ware_Index const i)
							if ((*l.current)->economy.needs_ware(wt)) {
								prio += 1 + wares[j->outputs[m]].preciousness;
							}
						}
					}

					// If the produced wares are needed, check if current economy can
					// supply enough material for production.
					if (prio > 0)
						for (uint32_t k = 0; k < j->inputs.size(); ++k) {
							prio += 2 * wares[j->inputs[k]].producers;
							prio -= 4 * wares[j->inputs[k]].consumers;
						}

					// normalize by output count so that multipurpose
					// buildings are not too good
					int32_t output_prio = 0;
					for (uint32_t k = 0; k < j->outputs.size(); ++k) {
						WareObserver & wo = wares[j->outputs[k]];
						if (j->total_count() == 0 && wo.consumers > 0)
							output_prio += 8 + wo.preciousness; // add a big bonus
					}

					if (j->outputs.size() > 0)
						output_prio = static_cast<int32_t>
							(ceil(output_prio / sqrt(j->outputs.size())));
					prio += output_prio;

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
			}

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
	for (std::list<BuildingObserver>::iterator i = buildings.begin(); i != buildings.end(); ++i) {
		if (!i->is_buildable || i->type != BuildingObserver::MINE)
			continue;

		for (std::list<MineableField *>::iterator j = mineable_fields.begin(); j != mineable_fields.end(); ++j) {
			MineableField * mf = *j;
			int32_t prio = -1;

			if (mf->coords.field->get_resources() != i->mines)
				continue;
			else
				prio += mf->coords.field->get_resources_amount();

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
			prio /= 1 + i->cnt_built * 2;
			prio /= 1 + i->cnt_under_construction * 4;

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

	//  do not have too many construction sites
	if (proposed_priority < total_constructionsites * total_constructionsites)
		return false;

	// if we want to construct a new building, send the command now
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
bool DefaultAI::construct_roads ()
{
	if (economies.size() < 2) {
		// only one economy, no need for new roads
		return false;
	}

	std::vector<WalkableSpot> spots;
	std::queue<int32_t> queue;
	Map & map = game().map();

	container_iterate_const(std::list<EconomyObserver *>, economies, i)
		container_iterate_const(std::list<Flag const *>, (*i.current)->flags, j)
		{
			queue.push (spots.size());

			spots.push_back(WalkableSpot());
			spots.back().coords  = (*j.current)->get_position();
			spots.back().hasflag = true;
			spots.back().cost    = 0;
			spots.back().eco     = &(*i.current)->economy;
			spots.back().from    = -1;
		}

	container_iterate_const(std::list<BuildableField *>, buildable_fields, i) {
		spots.push_back(WalkableSpot());
		spots.back().coords  = (*i.current)->coords;
		spots.back().hasflag = false;
		spots.back().cost    = -1;
		spots.back().eco     = 0;
		spots.back().from    = -1;
	}


	container_iterate_const(std::list<FCoords>, unusable_fields, i) {
		if ((player->get_buildcaps(*i.current) & MOVECAPS_WALK) == 0)
			continue;

		if (BaseImmovable * const imm = map.get_immovable(*i.current)) {
			if (upcast(Road, road, imm)) {
				if ((player->get_buildcaps(*i.current) & BUILDCAPS_FLAG) == 0)
					continue;

				queue.push (spots.size());

				spots.push_back(WalkableSpot());
				spots.back().coords  = *i.current;
				spots.back().hasflag = false;
				spots.back().cost    = 0;
				spots.back().eco = road->get_flag(Road::FlagStart).get_economy();
				spots.back().from    = -1;

				continue;
			}

			if (imm->get_size() >= BaseImmovable::SMALL) continue;
		}

		spots.push_back(WalkableSpot());
		spots.back().coords  = *i.current;
		spots.back().hasflag = false;
		spots.back().cost    = -1;
		spots.back().eco     = 0;
		spots.back().from    = -1;
	}

	int32_t i, j, k;
	for (i = 0; i < static_cast<int32_t>(spots.size()); ++i)
		for (j = 0; j < 6; ++j) { // the 6 different directions
			Coords nc;
			map.get_neighbour (spots[i].coords, j + 1, &nc);

			for (k = 0; k < static_cast<int32_t>(spots.size()); ++k)
				if (spots[k].coords == nc)
					break;

			spots[i].neighbours[j] =
				k < static_cast<int32_t>(spots.size()) ? k : -1;
		}

	while (!queue.empty()) {
		WalkableSpot & from = spots[queue.front()];
		queue.pop();

		for (i = 0; i < 6; ++i) // the 6 different directions
			if (from.neighbours[i] >= 0) {
				WalkableSpot &to = spots[from.neighbours[i]];

				if (to.cost < 0) {
					to.cost = from.cost + 1;
					to.eco  = from.eco;
					to.from = &from - &spots.front();

					queue.push (&to - &spots.front());
					continue;
				}

				if (from.eco != to.eco and to.cost > 0) {
					std::list<Coords> pc;
					bool hasflag;

					pc.push_back (to.coords);
					i = to.from;
					hasflag = to.hasflag;
					while (0 <= i) {
						pc.push_back (spots[i].coords);
						hasflag = spots[i].hasflag;
						i = spots[i].from;
					}

					if (!hasflag)
						game().send_player_build_flag (get_player_number(), pc.back());

					pc.push_front (from.coords);
					i = from.from;
					hasflag = from.hasflag;
					while (i >= 0) {
						pc.push_front (spots[i].coords);
						hasflag = spots[i].hasflag;
						i = spots[i].from;
					}

					if (!hasflag)
						game().send_player_build_flag (get_player_number(), pc.front());

					Path & path = *new Path(pc.front());
					pc.pop_front();

					for
						(std::list<Coords>::iterator c = pc.begin(); c != pc.end(); ++c)
					{
						const int32_t n = map.is_neighbour(path.get_end(), *c);
						assert (1 <= n);
						assert (n <= 6); // one of the 6 directions

						path.append (map, n);
						assert (path.get_end() == *c);
					}

					game().send_player_build_road (get_player_number(), path);
					return true;
				}
			}
	}
	return false;
}

/// improves current road system
bool DefaultAI::improve_roads ()
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
		}

		roads.push_back (roads.front());
		roads.pop_front ();
	}
	return false;
}


/**
 * Checks if anything in one of the economies changed and takes care for these
 * changes.
 */
void DefaultAI::check_economies ()
{
	while (!new_flags.empty()) {
		Flag const & flag = *new_flags.front();
		new_flags.pop_front();

		get_economy_observer(flag.economy())->flags.push_back (&flag);
	}

	container_iterate(std::list<EconomyObserver *>, economies, i) {
		// check if any flag has changed its economy
		container_iterate(std::list<Flag const *>, (*i.current)->flags, j) {
			if (&(*i.current)->economy != &(*j.current)->economy()) {
				get_economy_observer((*j.current)->economy())->flags.push_back
					(*j.current);
				j.current = (*i.current)->flags.erase(j.current);
				continue;
			}
		}

		// if there are no more flags in this economy, we no longer need its observer
		if ((*i.current)->flags.empty()) {
			delete *i.current;
			i.current = economies.erase(i.current);
			continue;
		}
	}
}

/**
 * checks the productionsites and takes care about buildings that run out of
 * ressources.
 *
 * \returns true, if something was changed.
 */
bool DefaultAI::check_productionsite (ProductionSiteObserver & site)
{
	// Get max radius of recursive workarea
	Workarea_Info::size_type radius = 0;

	Workarea_Info const & workarea_info = site.bo->desc->m_workarea_info;
	container_iterate_const(Workarea_Info, workarea_info, i)
		if (radius < i.current->first)
			radius = i.current->first;

	Map & map = game().map();
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
	return false;
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


/// \returns the economy observer
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
			productionsites.back().site =
				&dynamic_cast<ProductionSite &>(b);
			productionsites.back().bo = &bo;

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				++wares[bo.outputs[i]].producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				++wares[bo.inputs[i]].consumers;
		}
	}
}

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
					productionsites.erase (i);
					break;
				}

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				--wares[bo.outputs[i]].producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				--wares[bo.inputs[i]].consumers;
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

