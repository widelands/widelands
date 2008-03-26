#include <time.h>
/*
 * Copyright (C) 2004, 2006-2008 by the Widelands Development Team
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

#include "computer_player.h"

#include "computer_player_hints.h"
#include "map.h"
#include "world.h"
#include "transport.h"
#include "player.h"
#include "profile.h"
#include "findnode.h"
#include "tribe.h"
#include "constructionsite.h"
#include "productionsite.h"
#include "militarysite.h"
#include "findimmovable.h"

#include "log.h"

#include "upcast.h"

#include <algorithm>
#include <queue>
#include <typeinfo>

#define FIELD_UPDATE_INTERVAL 1000

using namespace Widelands;

struct CheckStepRoadAI : public CheckStep {
	CheckStepRoadAI(Player* pl, uint8_t mc, bool oe)
		: player(pl), movecaps(mc), openend(oe)
	{}

	void set_openend (bool oe)
	{openend=oe;}

	virtual bool allowed(Map* map, FCoords start, FCoords end, int32_t dir, StepId id) const;
	virtual bool reachabledest(Map* map, FCoords dest) const;

//private:
	Player * player;
	uint8_t    movecaps;
	bool     openend;
};

Computer_Player::Computer_Player(Game & g, const Player_Number pid) :
m_game(g), player_number(pid), tribe(0)
{}

// when Computer_Player is constructed, some information is not yet available (e.g. world)
void Computer_Player::late_initialization ()
{
	player = game().get_player(player_number);
	NoteReceiver<NoteImmovable>::connect(*player);
	NoteReceiver<NoteField>::connect(*player);
	tribe = &player->tribe();

	log ("ComputerPlayer(%d): initializing\n", player_number);

	wares=new WareObserver[tribe->get_nrwares()];
	for (int32_t i = 0; i < tribe->get_nrwares(); ++i) {
		wares[i].producers    = 0;
		wares[i].consumers    = 0;
		wares[i].preciousness = 0;
	}

	// Building hints for computer player
	std::string stoneproducer = "lumberjack";
	std::string trunkproducer = "forester";
	std::string forester = "forester";

	// Read the computerplayer hints of the tribe
	// FIXME: this is only a temporary workaround. Better define all this stuff
	//        in each buildings conf-file.
	std::string tribehints = "tribes/"+ tribe->name() + "/cphints";
	if (g_fs->FileExists(tribehints)) {
		Profile prof(tribehints.c_str());
		Section *hints;
		hints = prof.get_section("global");

		char warename[32];
		char wareprec[32];
		for (int32_t i = 0; i < hints->get_safe_int("numofwares"); ++i) {
			sprintf(warename, "ware_n_%i", i);
			sprintf(wareprec, "ware_p_%i", i);
			int32_t wprec = hints->get_safe_int(wareprec);
			wares[tribe->get_safe_ware_index(hints->get_safe_string(warename))]
				.preciousness
				=
				wprec;
		}
		stoneproducer = hints->get_safe_string("stoneproducer");
		trunkproducer = hints->get_safe_string("trunkproducer");
		forester = hints->get_safe_string("forester");

	} else {
		log("   WARNING: No computerplayer hints for tribe %s found\n", tribe->name().c_str());
		log("   This will lead to stupid behaviour of said player!\n");
	}

	// collect information about which buildings our tribe can construct
	for (Building_Index::value_t i = 0; i < tribe->get_nrbuildings(); ++i) {
		const Building_Descr & bld = *tribe->get_building_descr(i);
		const std::string & building_name = bld.name();

		buildings.push_back (BuildingObserver());

		BuildingObserver& bo      = buildings.back();
		bo.name                   = building_name.c_str();
		bo.id                     = i;
		bo.desc                   = &bld;
		bo.hints                  = &bld.hints();
		bo.type                   = BuildingObserver::BORING;
		bo.cnt_built              = 0;
		bo.cnt_under_construction = 0;
		bo.production_hint        = -1;

		bo.is_buildable           = bld.get_buildable();

		bo.need_trees             = false;
		bo.need_stones            = false;

		if (building_name == stoneproducer.c_str()) bo.need_stones = true;
		if (building_name == trunkproducer.c_str()) bo.need_trees  = true;
		if (building_name == forester.c_str())
			bo.production_hint = tribe->get_safe_ware_index("trunk");

		if (typeid(bld) == typeid(ConstructionSite_Descr)) {
			bo.type=BuildingObserver::CONSTRUCTIONSITE;
			continue;
		}

		if (typeid(bld) == typeid(MilitarySite_Descr)) {
			bo.type=BuildingObserver::MILITARYSITE;
			continue;
		}

		if (typeid(bld) == typeid(ProductionSite_Descr)) {
			const ProductionSite_Descr & prod =
				static_cast<const ProductionSite_Descr &>(bld);

			bo.type = bld.get_ismine() ?
				BuildingObserver::MINE : BuildingObserver::PRODUCTIONSITE;

			const std::vector<Input>::const_iterator inputs_end =
				prod.get_inputs()->end();
			for
				(std::vector<Input>::const_iterator it =
				 prod.get_inputs()->begin();
				 it != inputs_end;
				 ++it)
				bo.inputs.push_back
					(tribe->get_safe_ware_index(it->ware_descr().name().c_str()));

			const std::set<std::string>::const_iterator outputs_end =
				prod.get_outputs()->end();
			for
				(std::set<std::string>::const_iterator it =
				 prod.get_outputs()->begin();
				 it != outputs_end;
				 ++it)
				bo.outputs.push_back(tribe->get_safe_ware_index(it->c_str()));

			continue;
		}
	}

	total_constructionsites=0;
	next_construction_due=0;
	next_road_due=1000;
	next_productionsite_check_due=0;
	inhibit_road_building=0;

	// Add all fields that we own
	Map& map = game().map();
	std::set<OPtr<PlayerImmovable> > found_immovables;

	for (Y_Coordinate y = 0; y < map.get_height(); ++y) {
		for (X_Coordinate x = 0; x < map.get_width(); ++x) {
			FCoords f = map.get_fcoords(Coords(x, y));

			if (f.field->get_owned_by() != player_number)
				continue;

			unusable_fields.push_back (map.get_fcoords(Coords(x, y)));

			upcast(PlayerImmovable, imm, f.field->get_immovable());

			if (imm && imm->get_owner() == player) {
				// Guard by a set because immovables might be on several fields at once
				if (found_immovables.find(imm) == found_immovables.end()) {
					found_immovables.insert(imm);
					gain_immovable(imm);
				}
			}
		}
	}
}

Computer_Player::~Computer_Player () {}

Computer_Player::BuildingObserver& Computer_Player::get_building_observer (const char* name)
{
	if (tribe == 0)
		late_initialization ();

	for
		(std::list<BuildingObserver>::iterator i = buildings.begin();
		 i != buildings.end();
		 ++i)
		if (!strcmp(i->name, name))
			return *i;

	throw wexception("Help: I don't know what to do with a %s", name);
}

void Computer_Player::think ()
{
	if (tribe == 0)
		late_initialization ();

	const int32_t gametime = game().get_gametime();
	//printf("Computer_Player: Staring planner at GT %i.\n", gametime);

	// update statistics about buildable fields
	while
		(not buildable_fields.empty()
		 and
		 buildable_fields.front()->next_update_due <= gametime)
	{
		BuildableField* bf=buildable_fields.front();

		// check whether we lost ownership of the field
		if (bf->coords.field->get_owned_by()!=player_number) {
			buildable_fields.pop_front();
			continue;
		}

		// check whether we can still construct regular buildings on the field
		if ((player->get_buildcaps(bf->coords) & BUILDCAPS_SIZEMASK)==0) {
			unusable_fields.push_back (bf->coords);
			delete bf;

			buildable_fields.pop_front();
			continue;
		}

		update_buildable_field (bf);
		bf->next_update_due = gametime + FIELD_UPDATE_INTERVAL;

		buildable_fields.push_back (bf);
		buildable_fields.pop_front ();
	}
	//printf("Computer_Player: Done looking for buildable fields. %i found.\n", buildable_fields.size());

	// do the same for mineable fields
	while
		(not mineable_fields.empty()
		 and
		 mineable_fields.front()->next_update_due <= gametime)
	{
		MineableField* mf=mineable_fields.front();

		// check whether we lost ownership of the field
		if (mf->coords.field->get_owned_by()!=player_number) {
			mineable_fields.pop_front();
			continue;
		}

		// check whether we can still construct regular buildings on the field
		if ((player->get_buildcaps(mf->coords) & BUILDCAPS_MINE)==0) {
			unusable_fields.push_back (mf->coords);
			delete mf;

			mineable_fields.pop_front();
			continue;
		}

		update_mineable_field (mf);
		mf->next_update_due = gametime + FIELD_UPDATE_INTERVAL;

		mineable_fields.push_back (mf);
		mineable_fields.pop_front ();
	}
	//printf("Computer_Player: Done looking for minenable fields. %i found.\n", mineable_fields.size());
	for (std::list<FCoords>::iterator i=unusable_fields.begin(); i!=unusable_fields.end();) {
		// check whether we lost ownership of the field
		if (i->field->get_owned_by()!=player_number) {
			i=unusable_fields.erase(i);
			continue;
		}

		// check whether building capabilities have improved
		if ((player->get_buildcaps(*i) & BUILDCAPS_SIZEMASK) != 0) {
			buildable_fields.push_back (new BuildableField(*i));
			i=unusable_fields.erase(i);

			update_buildable_field (buildable_fields.back());
			continue;
		}

		if ((player->get_buildcaps(*i) & BUILDCAPS_MINE) != 0) {
			mineable_fields.push_back (new MineableField(*i));
			i=unusable_fields.erase(i);

			update_mineable_field (mineable_fields.back());
			continue;
		}

		++i;
	}

	// wait a moment so that all fields are classified
	if (next_construction_due == 0) next_construction_due = gametime + 1000;

	// now build something if possible
	if (next_construction_due <= gametime) {
		next_construction_due = gametime + 2000;
		printf("Computer_Player: Time to build.\n");
		if (construct_building()) {
			//inhibit_road_building = gametime + 2500;
			//Inhibiting roadbuilding is not a good idea, it causes
			//computer players to get into deadlock at certain circumstances.
			// printf("Computer_Player: Built something, waiting until road can be built.\n");
			return;
		}
	}

	// verify that our production sites are doing well
	if
		(next_productionsite_check_due <= gametime
		 and
		 not productionsites.empty())
	{
		next_productionsite_check_due = gametime + 2000;

		check_productionsite (productionsites.front());

		productionsites.push_back (productionsites.front());
		productionsites.pop_front ();
	}
	//printf("Computer_Player: Done checking up on construction sites.\n");
	// if nothing else is to do, update flags and economies
	while (!new_flags.empty()) {
		Flag* flag=new_flags.front();
		new_flags.pop_front();

		get_economy_observer(flag->get_economy())->flags.push_back (flag);
	}

	for
		(std::list<EconomyObserver *>::iterator i = economies.begin();
		 i != economies.end();
		 ++i)
	{
		// check if any flag has changed its economy
		for
			(std::list<Flag *>::iterator j = (*i)->flags.begin();
			 j != (*i)->flags.end();
			 ++j)
		{
			if ((*i)->economy!=(*j)->get_economy()) {
				get_economy_observer((*j)->get_economy())->flags.push_back (*j);
				j=(*i)->flags.erase(j);
				continue;
			}
		}

		// if there are no more flags in this economy, we no longer need its observer
		if ((*i)->flags.empty()) {
			delete *i;
			i=economies.erase(i);
			continue;
		}
	}

	if (next_road_due <= gametime) {
		//next_road_due+=1000;
		//if (true) {
		next_road_due = gametime + 1000;
		construct_roads ();
		//printf("Computer_Player: Building a road. next road due at %i, current GT %i\n", next_road_due, gametime);
	}
	//else printf("Computer_Player: Cant do roads yet, next at %i or %i GT  from %i GT.\n",
	//     (inhibit_road_building), (next_road_due), gametime);

#if 0
	if (not economies.empty() and inhibit_road_building <= gametime) {
		EconomyObserver* eco=economies.front();

		bool finish=false;

		// try to connect to another economy
		if (economies.size() > 1)
			finish = connect_flag_to_another_economy(eco->flags.front());

		if (!finish)
			finish = improve_roads(eco->flags.front());

		// cycle through flags one at a time
		eco->flags.push_back (eco->flags.front());
		eco->flags.pop_front ();

		// and cycle through economies
		economies.push_back (eco);
		economies.pop_front();

		if (finish)
			return;
	}
#endif

	// force a split on roads that are extremely long
	// note that having too many flags causes a loss of building capabilities
	if (!roads.empty()) {
		const Path& path=roads.front()->get_path();

		if (path.get_nsteps()>6) {
			const Map & map = game().map();
			CoordPath cp(map, path);

			// try to split near the middle
			CoordPath::Step_Vector::size_type i = cp.get_nsteps() / 2, j = i + 1;
			for (; i > 1; --i, ++j) {
				{
					const Coords c = cp.get_coords()[i];
					if (map[c].get_caps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag (player_number, c);
						return;
					}
				}
				{
					const Coords c = cp.get_coords()[j];
					if (map[c].get_caps() & BUILDCAPS_FLAG) {
						game().send_player_build_flag (player_number, c);
						return;
					}
				}
			}
		}

		roads.push_back (roads.front());
		roads.pop_front ();
	}
	//printf("Computer_Player: Done inspecting road infrastructure.\n");
}

bool Computer_Player::construct_building ()
{
	int32_t spots_avail[4];

	for (int32_t i = 0; i < 4; ++i)
		spots_avail[i]=0;

	for
		(std::list<BuildableField *>::iterator i = buildable_fields.begin();
		 i != buildable_fields.end();
		 ++i)
		spots_avail[(*i)->coords.field->get_caps() & BUILDCAPS_SIZEMASK]++;

	int32_t expand_factor=1;

	if (spots_avail[BUILDCAPS_BIG]<2)
		++expand_factor;
	if (spots_avail[BUILDCAPS_MEDIUM]+spots_avail[BUILDCAPS_BIG]<4)
		++expand_factor;
	if (spots_avail[BUILDCAPS_SMALL]+spots_avail[BUILDCAPS_MEDIUM]+spots_avail[BUILDCAPS_BIG]<8)
		++expand_factor;

	int32_t proposed_building=-1;
	int32_t proposed_priority=0;
	Coords proposed_coords;

	// first scan all buildable fields for regular buildings
	for
		(std::list<BuildableField *>::iterator i = buildable_fields.begin();
		 i != buildable_fields.end();
		 ++i)
	{
		BuildableField* bf=*i;

		if (!bf->reachable)
			continue;

		int32_t maxsize=bf->coords.field->get_caps() & BUILDCAPS_SIZEMASK;
		int32_t prio;

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

			prio = 0;

			if (j->type==BuildingObserver::MILITARYSITE) {
				prio  = bf->unowned_land_nearby - bf->military_influence * 2;
				prio *= expand_factor;
				prio /= 4;

				if (bf->avoid_military) {
					prio /= 3;
					prio -= 6;
				}

				prio -= spots_avail[BUILDCAPS_BIG]    / 2;
				prio -= spots_avail[BUILDCAPS_MEDIUM] / 4;
				prio -= spots_avail[BUILDCAPS_SMALL]  / 8;
			}

			if (j->type==BuildingObserver::PRODUCTIONSITE) {
				if (j->need_trees)
					prio += bf->trees_nearby - 6*bf->tree_consumers_nearby - 2;

				if (j->need_stones)
					prio += bf->stones_nearby - 6*bf->stone_consumers_nearby - 2;

				if
					((j->need_trees || j->need_stones)
					 &&
					 j->cnt_built == 0
					 &&
					 j->cnt_under_construction == 0)
					prio *= 2;

				if (!j->need_trees && !j->need_stones) {
					if (j->cnt_built+j->cnt_under_construction==0)
						prio += 2;

					for (uint32_t k = 0; k < j->inputs.size(); ++k) {
						prio += 8 * wares[j->inputs[k]].producers;
						prio -= 4 * wares[j->inputs[k]].consumers;
					}

					for (uint32_t k = 0; k < j->outputs.size(); ++k) {
						prio -= 12 * wares[j->outputs[k]].producers;
						prio +=  8 * wares[j->outputs[k]].consumers;
						prio +=  4 * wares[j->outputs[k]].preciousness;

						if
							(j->cnt_built+j->cnt_under_construction == 0
							 &&
							 wares[j->outputs[k]].consumers > 0)
							prio += 8; // add a big bonus
					}

					if (j->production_hint >= 0) {
						prio -= 6 * (j->cnt_built + j->cnt_under_construction);
						prio += 4 * wares[j->production_hint].consumers;
						prio += 2 * wares[j->production_hint].preciousness;
					}
				}

				prio -=
					2 * j->cnt_under_construction * (j->cnt_under_construction + 1);
			}

			prio += bf->preferred ? prio / 2 + 1 : -1;

			//  don't waste good land for small huts
			prio -= (maxsize - j->desc->get_size()) * 3;

			if (prio > proposed_priority) {
				proposed_building = j->id;
				proposed_priority = prio;
				proposed_coords   = bf->coords;
			}
		}
	}

#if 0 //FIXME
	// then try all mines
	const World & world = game().map().world();
	for (std::list<BuildingObserver>::iterator i = buildings.begin();i != buildings.end(); ++i) {
		if (!i->is_buildable || i->type!=BuildingObserver::MINE)
			continue;

		for (std::list<MineableField *>::iterator j = mineable_fields.begin(); j != mineable_fields.end(); ++j) {
			MineableField* mf=*j;
			int32_t prio=-1;

			if (i->hints->get_need_map_resource()!=0) {
				int32_t res = world.get_resource(i->hints->get_need_map_resource());

				if (mf->coords.field->get_resources()!=res)
					continue;

				prio+=mf->coords.field->get_resources_amount();
			}

			WareObserver& output=wares[i->outputs[0]];
			if (output.consumers>0)
				prio*=2;

			prio-=2 * mf->mines_nearby * mf->mines_nearby;
			prio-=i->cnt_built*3;
			prio-=i->cnt_under_construction*8;

			if (prio>proposed_priority) {
				proposed_building=i->id;
				proposed_priority=prio;
				proposed_coords=mf->coords;
			}
		}
	}
#endif

	if (proposed_building<0)
		return false;

	//  do not have too many construction sites
	if (proposed_priority < total_constructionsites * total_constructionsites)
		return false;

	// if we want to construct a new building, send the command now
	log ("ComputerPlayer(%d): want to construct building %d\n", player_number, proposed_building);
	game().send_player_build (player_number, proposed_coords, proposed_building);

	return true;
}

void Computer_Player::check_productionsite (ProductionSiteObserver& site)
{
	log
		("ComputerPlayer(%d): checking %s\n",
		 player_number,
		 site.bo->desc->name().c_str());

	// Get max radius of recursive workarea
	Workarea_Info::size_type radius = 0;

	const Workarea_Info & workarea_info = site.bo->desc->m_recursive_workarea_info;
	for
		(Workarea_Info::const_iterator it = workarea_info.begin();
		 it != workarea_info.end();
		 ++it)
		if (it->first > radius) radius = it->first;

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

		log
			("ComputerPlayer(%d): out of resources, destructing\n",
			 player_number);
		m_game.send_player_bulldoze (site.site);
		return;
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

		log
			("ComputerPlayer(%d): out of resources, destructing\n",
			 player_number);
		game().send_player_bulldoze (site.site);
		return;
	}
}

struct FindNodeUnowned {
	bool accept (Map const &, FCoords) const;
};

bool FindNodeUnowned::accept (const Map &, const FCoords fc) const
{
	// when looking for unowned terrain to acquire, we are actually
	// only interested in fields we can walk on
	return fc.field->get_owned_by()==0 && (fc.field->get_caps()&MOVECAPS_WALK);
}

void Computer_Player::update_buildable_field (BuildableField* field)
{
	// look if there is any unowned land nearby
	FindNodeUnowned find_unowned;
	Map & map = game().map();

	field->unowned_land_nearby =
		map.find_fields(Area<FCoords>(field->coords, 8), 0, find_unowned);

	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;

	const int32_t tree_attr=Map_Object_Descr::get_attribute_id("tree");
	const int32_t stone_attr=Map_Object_Descr::get_attribute_id("stone");

	map.find_immovables (Area<FCoords>(field->coords, 8), &immovables);

	field->reachable=false;
	field->preferred=false;
	field->avoid_military=false;

	field->military_influence=0;
	field->trees_nearby=0;
	field->stones_nearby=0;
	field->tree_consumers_nearby=0;
	field->stone_consumers_nearby=0;

	FCoords fse;
	map.get_neighbour (field->coords, Map_Object::WALK_SE, &fse);

	if (BaseImmovable const * const imm = fse.field->get_immovable())
		if
			(dynamic_cast<Flag const *>(imm)
			 or
			 dynamic_cast<Road const *>(imm)
			 &&
			 fse.field->get_caps() & BUILDCAPS_FLAG)
		field->preferred=true;

	for (uint32_t i = 0;i < immovables.size(); ++i) {
		const BaseImmovable & base_immovable = *immovables[i].object;
		if (dynamic_cast<const Flag *>(&base_immovable))
			field->reachable=true;
		if (upcast(PlayerImmovable const, player_immovable, &base_immovable))
			if (player_immovable->owner().get_player_number() != player_number)
				continue;

		if (upcast(Building const, building, &base_immovable)) {

			if (upcast(ConstructionSite const, constructionsite, building)) {
				const Building_Descr & target_descr = constructionsite->building();

				if (upcast(MilitarySite_Descr const, target_ms_d, &target_descr)) {
					const int32_t v =
						target_ms_d->get_conquers()
						-
						map.calc_distance(field->coords, immovables[i].coords);

					if (0 < v) {
						field->military_influence += v * (v + 2) * 6;
						field->avoid_military = true;
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
					map.calc_distance(field->coords, immovables[i].coords);

				if (v > 0)
					field->military_influence +=
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
			++field->trees_nearby;

		if (immovables[i].object->has_attribute(stone_attr))
			++field->stones_nearby;
	}
}

void Computer_Player::update_mineable_field (MineableField* field)
{
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;
	Map & map = game().map();

	map.find_immovables (Area<FCoords>(field->coords, 6), &immovables);

	field->reachable=false;
	field->preferred=false;
	field->mines_nearby=true;

	FCoords fse;
	map.get_neighbour (field->coords, Map_Object::WALK_SE, &fse);

	if (BaseImmovable const * const imm = fse.field->get_immovable())
		if
			(dynamic_cast<Flag const *>(imm)
			 or
			 dynamic_cast<Road const *>(imm)
			 &&
			 fse.field->get_caps() & BUILDCAPS_FLAG)
		field->preferred=true;

	for (uint32_t i = 0; i < immovables.size(); ++i) {
		if (immovables[i].object->get_type()==BaseImmovable::FLAG)
			field->reachable=true;

		if (upcast(Building const, bld, immovables[i].object))
			if
				(player->get_buildcaps(map.get_fcoords(immovables[i].coords))
				 &
				 BUILDCAPS_MINE)
			{

			if
				(dynamic_cast<ConstructionSite const *>(bld) or
				 dynamic_cast<ProductionSite   const *>(bld))
				++field->mines_nearby;
			}
	}
}

void Computer_Player::consider_productionsite_influence
(BuildableField * field, Coords const, const BuildingObserver & bo)
{
	if (bo.need_trees)
		++field->tree_consumers_nearby;

	if (bo.need_stones)
		++field->stone_consumers_nearby;
}

Computer_Player::EconomyObserver* Computer_Player::get_economy_observer (Economy* economy)
{
	for
		(std::list<EconomyObserver*>::iterator i = economies.begin();
		 i != economies.end();
		 ++i)
		if ((*i)->economy==economy)
			return *i;

	economies.push_front (new EconomyObserver(economy));

	return economies.front();
}

void Computer_Player::gain_building (Building* b)
{
	BuildingObserver & bo = get_building_observer(b->name().c_str());

	if (bo.type==BuildingObserver::CONSTRUCTIONSITE) {
		++get_building_observer
			(dynamic_cast<const ConstructionSite &>(*b)
			 .building().name().c_str())
			.cnt_under_construction;
		++total_constructionsites;
	}
	else {
		++bo.cnt_built;

		if (bo.type==BuildingObserver::PRODUCTIONSITE) {
			productionsites.push_back (ProductionSiteObserver());
			productionsites.back().site=static_cast<ProductionSite*>(b);
			productionsites.back().bo=&bo;

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				++wares[bo.outputs[i]].producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				++wares[bo.inputs[i]].consumers;
		}
	}
}

void Computer_Player::lose_building (Building* b)
{
	BuildingObserver & bo = get_building_observer(b->name().c_str());

	if (bo.type==BuildingObserver::CONSTRUCTIONSITE) {
		--get_building_observer
			(dynamic_cast<const ConstructionSite &>(*b)
			 .building().name().c_str())
			.cnt_under_construction;
		--total_constructionsites;
	}
	else {
		--bo.cnt_built;

		if (bo.type==BuildingObserver::PRODUCTIONSITE) {
			for
				(std::list<ProductionSiteObserver>::iterator i =
				 productionsites.begin();
				 i != productionsites.end();
				 ++i)
				if (i->site==b) {
					productionsites.erase (i);
					break;
				}

			for (uint32_t i = 0; i < bo.outputs.size(); ++i)
				--wares[bo.outputs[i]].producers;

			for (uint32_t i = 0; i < bo.inputs.size(); ++i)
				--wares[bo.inputs[i]].consumers;
		}
	}
}

// Road building
struct FindNodeWithFlagOrRoad {
	Economy* economy;
	bool accept(const Map &, FCoords) const;
};

bool FindNodeWithFlagOrRoad::accept (const Map &, FCoords fc) const {
	BaseImmovable* imm=fc.field->get_immovable();

	if (imm==0)
		return false;

	if (imm->get_type()>=BaseImmovable::BUILDING && static_cast<PlayerImmovable*>(imm)->get_economy()==economy)
		return false;

	if (imm->get_type()==BaseImmovable::FLAG)
		return true;

	if (imm->get_type()==BaseImmovable::ROAD && (fc.field->get_caps()&BUILDCAPS_FLAG)!=0)
		return true;

	return false;
}

bool Computer_Player::connect_flag_to_another_economy (Flag* flag)
{
	FindNodeWithFlagOrRoad functor;
	CheckStepRoadAI check(player, MOVECAPS_WALK, true);
	std::vector<Coords> reachable;

	// first look for possible destinations
	functor.economy=flag->get_economy();
	Map & map = game().map();
	map.find_reachable_fields
		(Area<FCoords>(map.get_fcoords(flag->get_position()), 16),
		 &reachable,
		 check,
		 functor);

	if (reachable.empty())
		return false;

	// then choose the one closest to the originating flag
	int32_t closest_distance = std::numeric_limits<int32_t>::max();
	Coords closest;
	std::vector<Coords>::const_iterator reachable_end = reachable.end();
	for
		(std::vector<Coords>::const_iterator it = reachable.begin();
		 it != reachable_end;
		 ++it)
	{
		const int32_t distance = map.calc_distance(flag->get_position(), *it);
		if (distance < closest_distance) {
			closest = *it;
			closest_distance = distance;
		}
	}
	assert(closest_distance != std::numeric_limits<int32_t>::max());

	// if we join a road and there is no flag yet, build one
	if (dynamic_cast<const Road *> (map[closest].get_immovable()))
		game().send_player_build_flag (player_number, closest);

	// and finally build the road
	Path & path = *new Path();
	check.set_openend (false);
	if (map.findpath(flag->get_position(), closest, 0, path, check) < 0) {
		delete &path;
		return false;
	}

	game().send_player_build_road (player_number, path);
	return true;
}

struct NearFlag {
	Flag * flag;
	int32_t   cost;
	int32_t   distance;

	NearFlag (Flag * const f, int32_t const c, int32_t const d) {
		flag     = f;
		cost     = c;
		distance = d;
	}

	bool operator< (NearFlag const & f) const {return cost > f.cost;}

	bool operator== (Flag const * const f) const {return flag == f;}
};

struct CompareDistance {
	bool operator() (const NearFlag& a, const NearFlag& b) const {
		return a.distance < b.distance;
	}
};

bool Computer_Player::improve_roads (Flag* flag)
{
	std::priority_queue<NearFlag> queue;
	std::vector<NearFlag> nearflags;

	queue.push (NearFlag(flag, 0, 0));
	Map & map = game().map();

	while (!queue.empty()) {
		std::vector<NearFlag>::iterator f = find(nearflags.begin(), nearflags.end(), queue.top().flag);
		if (f != nearflags.end()) {
			queue.pop ();
			continue;
		}

		nearflags.push_back (queue.top());
		queue.pop ();

		NearFlag & nf = nearflags.back();

		for (uint8_t i = 1; i <= 6; ++i) {
		Road* road=nf.flag->get_road(i);

		if (!road) continue;

		Flag* endflag=road->get_flag(Road::FlagStart);
		if (endflag==nf.flag)
			endflag = road->get_flag(Road::FlagEnd);

			int32_t dist =
				map.calc_distance(flag->get_position(), endflag->get_position());
		if (dist > 16) //  out of range
			continue;

			queue.push
				(NearFlag(endflag, nf.cost+road->get_path().get_nsteps(), dist));
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
				 	(flag->get_position(), nf.flag->get_position(), 0, path, check)
				 >=
				 0
				 and
				 static_cast<int32_t>(2 * path.get_nsteps() + 2) < nf.cost)
			{
				game().send_player_build_road (player_number, path);
				return true;
			}

			delete &path;
		}
	}

	return false;
}


void Computer_Player::receive(const NoteImmovable& note)
{
	if (note.lg == LOSE)
		lose_immovable(note.pi);
	else
		gain_immovable(note.pi);
}

void Computer_Player::receive(const NoteField& note)
{
	if (note.lg == GAIN)
		unusable_fields.push_back(note.fc);
}

// this is called whenever we gain ownership of a PlayerImmovable
void Computer_Player::gain_immovable (PlayerImmovable* pi)
{
	switch (pi->get_type()) {
	case BaseImmovable::BUILDING:
		gain_building (static_cast<Building*>(pi));
		break;
	case BaseImmovable::FLAG:
		new_flags.push_back (static_cast<Flag*>(pi));
		break;
	case BaseImmovable::ROAD:
		roads.push_front (static_cast<Road*>(pi));
		break;
	}
}

// this is called whenever we lose ownership of a PlayerImmovable
void Computer_Player::lose_immovable (PlayerImmovable* pi)
{
	switch (pi->get_type()) {
	case BaseImmovable::BUILDING:
		lose_building (static_cast<Building*>(pi));
		break;
	case BaseImmovable::FLAG:
		for
			(std::list<EconomyObserver *>::iterator i = economies.begin();
			 i!=economies.end();
			 ++i)
			for
				(std::list<Flag *>::iterator j = (*i)->flags.begin();
				 j != (*i)->flags.end();
				 ++j)
			if (*j==pi) {
				(*i)->flags.erase (j);
				break;
			}

		break;
	case BaseImmovable::ROAD:
		roads.remove (static_cast<Road*>(pi));
		break;
	}
}


/* CheckStepRoadAI */
bool CheckStepRoadAI::allowed
(Map * map, FCoords, FCoords end, int32_t, StepId id) const
{
	uint8_t endcaps = player->get_buildcaps(end);

	// Calculate cost and passability
	if (!(endcaps & movecaps)) {
		return false;
		//uint8_t startcaps = player->get_buildcaps(start);

		//if (!((endcaps & MOVECAPS_WALK) && (startcaps & movecaps & MOVECAPS_SWIM)))
			//return false;
	}

	// Check for blocking immovables
	BaseImmovable *imm = map->get_immovable(end);
	if (imm && imm->get_size() >= BaseImmovable::SMALL) {
		if (id!=stepLast && !openend)
			return false;

		if (imm->get_type()==Map_Object::FLAG)
			return true;

		if ((imm->get_type() != Map_Object::ROAD || !(endcaps & BUILDCAPS_FLAG)))
			return false;
	}

	return true;
}

bool CheckStepRoadAI::reachabledest(Map* map, FCoords dest) const
{
	uint8_t caps = dest.field->get_caps();

	if (!(caps & movecaps)) {
		if (!((movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map->can_reach_by_water(dest))
			return false;
	}

	return true;
}

struct WalkableSpot {
	Coords coords;
	bool   hasflag;

	int32_t    cost;
	void * eco;

	int16_t  from;
	int16_t  neighbours[6];
};

void Computer_Player::construct_roads ()
{
	std::vector<WalkableSpot> spots;
	std::queue<int32_t> queue;
	Map & map = game().map();

	for
		(std::list<EconomyObserver *>::iterator i = economies.begin();
		 i != economies.end();
		 ++i)
		for
			(std::list<Flag *>::iterator j = (*i)->flags.begin();
			 j != (*i)->flags.end();
			 ++j)
		{
			queue.push (spots.size());

			spots.push_back(WalkableSpot());
			spots.back().coords  = (*j)->get_position();
			spots.back().hasflag = true;
			spots.back().cost    = 0;
			spots.back().eco     = (*i)->economy;
			spots.back().from    = -1;
		}

	for
		(std::list<BuildableField *>::iterator i = buildable_fields.begin();
		 i != buildable_fields.end();
		 ++i)
	{
		spots.push_back(WalkableSpot());
		spots.back().coords=(*i)->coords;
		spots.back().hasflag=false;
		spots.back().cost=-1;
		spots.back().eco=0;
		spots.back().from=-1;
	}

	for
		(std::list<FCoords>::iterator i = unusable_fields.begin();
		 i != unusable_fields.end();
		 ++i)
	{
		if ((player->get_buildcaps(*i)&MOVECAPS_WALK)==0)
			continue;

		if (BaseImmovable * const imm = map.get_immovable(*i)) {
			if (upcast(Road, road, imm)) {
				if ((player->get_buildcaps(*i) & BUILDCAPS_FLAG) == 0)
					continue;

				queue.push (spots.size());

				spots.push_back(WalkableSpot());
				spots.back().coords  = *i;
				spots.back().hasflag = false;
				spots.back().cost    = 0;
				spots.back().eco = road->get_flag(Road::FlagStart)->get_economy();
				spots.back().from    = -1;

				continue;
			}

			if (imm->get_size() >= BaseImmovable::SMALL) continue;
		}

		spots.push_back(WalkableSpot());
		spots.back().coords=*i;
		spots.back().hasflag=false;
		spots.back().cost=-1;
		spots.back().eco=0;
		spots.back().from=-1;
	}

	const clock_t time_before = clock();
	int32_t i, j, k;
	for (i = 0; i < static_cast<int32_t>(spots.size()); ++i)
		for (j = 0; j < 6; ++j) {
			Coords nc;
			map.get_neighbour (spots[i].coords, j + 1, &nc);

			for (k = 0; k < static_cast<int32_t>(spots.size()); ++k)
				if (spots[k].coords == nc)
					break;

			spots[i].neighbours[j]=(k<static_cast<int32_t>(spots.size())) ? k : -1;
		}

	log
		("Computer_Player(%u): %u spots for road building (%f seconds) \n", player_number, spots.size(), static_cast<double>(clock() - time_before) / CLOCKS_PER_SEC);

	while (!queue.empty()) {
		WalkableSpot & from = spots[queue.front()];
		queue.pop();

		for (i = 0; i < 6; ++i) if (from.neighbours[i] >= 0) {
			WalkableSpot &to = spots[from.neighbours[i]];

			if (to.cost < 0) {
				to.cost = from.cost + 1;
				to.eco=from.eco;
				to.from=&from - &spots.front();

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
					game().send_player_build_flag (player_number, pc.back());

				pc.push_front (from.coords);
				i = from.from;
				hasflag = from.hasflag;
				while (i>=0) {
					pc.push_front (spots[i].coords);
					hasflag = spots[i].hasflag;
					i = spots[i].from;
				}

				if (!hasflag)
					game().send_player_build_flag (player_number, pc.front());

				log
					("Computer_Player(%u): New road has length %u\n",
					 player_number,
					 pc.size());
				Path & path = *new Path(pc.front());
				pc.pop_front();

				for
					(std::list<Coords>::iterator c = pc.begin(); c != pc.end(); ++c)
				{
					const int32_t n = map.is_neighbour(path.get_end(), *c);
					assert (n>=1 && n<=6);

					path.append (map, n);
					assert (path.get_end() == *c);
				}

				game().send_player_build_road (player_number, path);
				return;
			}
		}
	}
}
