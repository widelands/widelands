/*
 * Copyright (C) 2004 by The Widelands Development Team
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

#include "error.h"
#include "map.h"
#include "transport.h"
#include "player.h"
#include "tribe.h"
#include "constructionsite.h"
#include "computer_player.h"

class CheckStepRoadAI : public CheckStep {
public:
	CheckStepRoadAI(Player* player, uchar movecaps)
		: m_player(player), m_movecaps(movecaps) { }

	virtual bool allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const;
	virtual bool reachabledest(Map* map, FCoords dest) const;

private:
	Player*							m_player;
	uchar								m_movecaps;
};

Computer_Player::Computer_Player (Game *g, uchar pid)
{
	game = g;
	map = game->get_map();
	
	player_number = pid;
	player = g->get_player(player_number);
	tribe = player->get_tribe();
	
	log ("new Comp_Player for player %d\n", pid);

	// collect information about which buildings our tribe can construct
	for (int i=0; i<tribe->get_nrbuildings();i++) {
		const char* name=tribe->get_building_descr(i)->get_name();
		log ("\tcan build '%s', id is %d\n",name,i);
		
		BuildingObserver& bo=buildings[name];
		bo.id=i;
		bo.is_constructionsite=false;
		bo.cnt_built=0;
		bo.cnt_under_construction=0;
	}
	
	buildings["constructionsite"].is_constructionsite=true;
	
	total_constructionsites=0;
	delay_frontierhouse=0;
}

Computer_Player::~Computer_Player ()
{
}

void Computer_Player::think ()
{
/*	// update our fields 
	for (std::list<BuildableField>::iterator i=buildable_fields.begin(); i!=buildable_fields.end();) {
		// check whether we lost ownership of the field
		if (i->field->get_owned_by()!=player_number) {
			log ("AI player %d lost field (%d,%d)\n", player_number, i->x, i->y);
			i=buildable_fields.erase(i);
			continue;
		}
		
		// check whether we can still build on the field
		if ((player->get_buildcaps(*i) & BUILDCAPS_SIZEMASK)==0) {
			log ("Field (%d,%d) can no longer be built upon\n", i->x, i->y);
			unusable_fields.push_back (*i);
			i=buildable_fields.erase(i);
			continue;
		}
		
		i++;
	}
	
	// update statistics about only one field at a time
	if (!buildable_fields.empty()) {
		update_buildable_field (buildable_fields.front());
		
		buildable_fields.push_back (buildable_fields.front());
		buildable_fields.pop_front ();
	}

	for (std::list<FCoords>::iterator i=unusable_fields.begin(); i!=unusable_fields.end();) {
		// check whether we lost ownership of the field
		if (i->field->get_owned_by()!=player_number) {
			log ("AI player %d lost field (%d,%d)\n", player_number, i->x, i->y);
			i=unusable_fields.erase(i);
			continue;
		}

		// check whether building capabilities have improved
		if ((player->get_buildcaps(*i) & BUILDCAPS_SIZEMASK) != 0) {
			log ("Field (%d,%d) can now be built upon\n", i->x, i->y);
			buildable_fields.push_back (*i);
			i=unusable_fields.erase(i);

			update_buildable_field (buildable_fields.back());
			continue;
		}
		
		i++;
	}
	
	// now build something if possible
	int proposed_building=-1;
	int proposed_priority=0;
	Coords proposed_coords;
	
	for (std::list<BuildableField>::iterator i=buildable_fields.begin(); i!=buildable_fields.end(); i++) {
		if (!i->reachable)
			continue;
		
		int maxsize=i->field->get_caps() & BUILDCAPS_SIZEMASK;
		int prio;
		
		// frontierhouse
		if (delay_frontierhouse<=game->get_gametime()) {
			prio=i->unowned_land;
			prio-=i->frontierhouses*64;
				
			if (prio>proposed_priority) {
				proposed_building=buildings["frontierhouse"].id;
				proposed_priority=prio;
				proposed_coords=*i;
			}
		}
		
		// lumberjack
		prio=i->trees;
		prio-=i->lumberjacks*8;
		prio+=i->foresters*4;
		prio-=buildings["lumberjack"].cnt_built*4;
		prio-=buildings["lumberjack"].cnt_under_construction*16;
		
		if (maxsize>BUILDCAPS_SMALL)
			prio-=6;
		
		if (prio>proposed_priority) {
			proposed_building=buildings["lumberjack"].id;
			proposed_priority=prio;
			proposed_coords=*i;
		}
		
		// forester
		prio=-4;
		prio+=i->lumberjacks*4;
		prio-=i->foresters*12;
		prio-=buildings["forrester"].cnt_built*4;
		prio-=buildings["forrester"].cnt_under_construction*16;
		
		if (prio>proposed_priority) {
			proposed_building=buildings["forrester"].id;
			proposed_priority=prio;
			proposed_coords=*i;
		}
		
		// quarry
		prio=i->stones;
		prio-=i->quarries*8;
		prio-=buildings["quarry"].cnt_built*4;
		prio-=buildings["quarry"].cnt_under_construction*16;
		
		if (maxsize>BUILDCAPS_SMALL)
			prio-=6;
		
		if (prio>proposed_priority) {
			proposed_building=buildings["quarry"].id;
			proposed_priority=prio;
			proposed_coords=*i;
		}
		
		// sawmill
		if (maxsize>=BUILDCAPS_MEDIUM) {
			prio=8;
    			prio-=buildings["sawmill"].get_total_count() * 8;
		
			if (prio>proposed_priority) {
				proposed_building=buildings["sawmill"].id;
				proposed_priority=prio;
				proposed_coords=*i;
			}
		}
		
		// farm
		if (maxsize==BUILDCAPS_BIG) {
			prio=4;
    			prio-=buildings["farm"].get_total_count() * 2;
			prio-=i->trees;
			prio-=i->stones;
			prio-=i->foresters*8;
			if (total_constructionsites>4)
				prio-=6;
		
			if (prio>proposed_priority) {
				proposed_building=buildings["farm"].id;
				proposed_priority=prio;
				proposed_coords=*i;
			}
		}
		
		// mill
		if (maxsize>=BUILDCAPS_MEDIUM) {
			prio=buildings["farm"].get_total_count() * 8;
    			prio-=buildings["mill"].get_total_count() * 8;
			if (total_constructionsites>4)
				prio-=6;
		
			if (prio>proposed_priority) {
				proposed_building=buildings["mill"].id;
				proposed_priority=prio;
				proposed_coords=*i;
			}
		}
		
		// bakery
		if (maxsize>=BUILDCAPS_MEDIUM) {
			prio=buildings["mill"].get_total_count() * 8;
    			prio-=buildings["bakery"].get_total_count() * 8;
			if (total_constructionsites>4)
				prio-=6;
		
			if (prio>proposed_priority) {
				proposed_building=buildings["bakery"].id;
				proposed_priority=prio;
				proposed_coords=*i;
			}
		}
	}
	
	// if we want to construct a new building, send the command now
	if (proposed_building>=0) {
		game->send_player_build (player_number, proposed_coords, proposed_building);
		
		if (buildings["frontierhouse"].id==proposed_building)
			delay_frontierhouse=game->get_gametime() + 10000;
		
		return;
	}
	
	// if nothing else is to do, update flags and economies
	while (!new_flags.empty()) {
		Flag* flag=new_flags.front();
		new_flags.pop_front();
		
		get_economy_observer(flag->get_economy()).flags.push_back (flag);
	}
	
	for (std::list<EconomyObserver>::iterator i=economies.begin(); i!=economies.end();) {
		// check if any flag has changed its economy
		for (std::list<Flag*>::iterator j=i->flags.begin(); j!=i->flags.end();) {
			if (i->economy!=(*j)->get_economy()) {
				log ("Flag at (%d,%d) changed economy\n", (*j)->get_position().x, (*j)->get_position().y);
				
				get_economy_observer((*j)->get_economy()).flags.push_back (*j);
				j=i->flags.erase(j);
				continue;
			}
			
			j++;
		}
		
		// if there are no more flags in this economy, we no longer need its observer
		if (i->flags.empty()) {
			i=economies.erase(i);
			continue;
		}
		
		// try to connect to another economy
		if (economies.size()>1) {
			connect_flag_to_another_economy (i->flags.front());

			// cycle through flags one at a time
			i->flags.push_back (i->flags.front());
			i->flags.pop_front ();
		}
		
		i++;
	}
	
	// force a split on roads that are extremely long
	// note that having too many flags causes a loss of building capabilities
	if (!roads.empty()) {
		const Path& path=roads.front()->get_path();
		
		if (path.get_nsteps()>6) {
			CoordPath cp(path);
			int i;
			
			// try to split near the middle
			for (i=0;i<cp.get_nsteps()/2-2;i++) {
				Field* f;
				
				f=map->get_field(cp.get_coords()[cp.get_nsteps()/2-i]);
				if ((f->get_caps()&BUILDCAPS_FLAG)!=0) {
					game->send_player_build_flag (player_number, cp.get_coords()[cp.get_nsteps()/2-i]);
					return;
				}
				
				f=map->get_field(cp.get_coords()[cp.get_nsteps()/2+i+1]);
				if ((f->get_caps()&BUILDCAPS_FLAG)!=0) {
					game->send_player_build_flag (player_number, cp.get_coords()[cp.get_nsteps()/2+i+1]);
					return;
				}
			}
		}
		
		roads.push_back (roads.front());
		roads.pop_front ();
	}
*/
}

struct FindFieldUnowned:FindField {
	virtual bool accept (const FCoords) const;
};

bool FindFieldUnowned::accept (const FCoords fc) const
{
	return fc.field->get_owned_by()==0;
}

void Computer_Player::update_buildable_field (BuildableField& field)
{
	// look if there is any unowned land nearby
	FindFieldUnowned find_unowned;
	
	field.unowned_land=map->find_fields(field, 8, 0, find_unowned);
	
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;

	const int tree_attr=Map_Object_Descr::get_attribute_id("tree");
	const int stone_attr=Map_Object_Descr::get_attribute_id("stone");
	
	map->find_immovables (field, 8, &immovables);
	
	field.reachable=false;	
	field.frontierhouses=0;
	field.lumberjacks=0;
	field.foresters=0;
	field.quarries=0;
	field.trees=0;
	field.stones=0;
	
	for (unsigned int i=0;i<immovables.size();i++) {
		if (immovables[i].object->get_type()>=BaseImmovable::BUILDING)
			field.reachable=true;

		if (immovables[i].object->get_type()==BaseImmovable::BUILDING && map->calc_distance(field,immovables[i].coords)<=6) {
			std::string name=static_cast<Building*>(immovables[i].object)->get_name();
			
			if (buildings[name].is_constructionsite)
				name=static_cast<ConstructionSite*>(immovables[i].object)->get_building()->get_name();
			
			if (name=="frontierhouse")
				field.frontierhouses++;

			if (name=="lumberjack")
				field.lumberjacks++;

			if (name=="forrester")
				field.foresters++;

			if (name=="quarry")
				field.quarries++;

			continue;
		}
		
		if (immovables[i].object->has_attribute(tree_attr))
			field.trees++;

		if (immovables[i].object->has_attribute(stone_attr))
			field.stones++;
	}
}

Computer_Player::EconomyObserver& Computer_Player::get_economy_observer (Economy* economy)
{
	std::list<EconomyObserver>::iterator i;
	
	for (i=economies.begin(); i!=economies.end(); i++)
		if (i->economy==economy)
			return *i;
	
	economies.push_front (EconomyObserver(economy));
	
	return economies.front();
}

void Computer_Player::gain_building (Building* b)
{
	BuildingObserver& bo=buildings[b->get_name()];
	
	if (bo.is_constructionsite) {
		buildings[static_cast<ConstructionSite*>(b)->get_building()->get_name()].cnt_under_construction++;
		total_constructionsites++;
	}
	else
		bo.cnt_built++;
}

void Computer_Player::lose_building (Building* b)
{
	BuildingObserver& bo=buildings[b->get_name()];
	
	if (bo.is_constructionsite) {
		buildings[static_cast<ConstructionSite*>(b)->get_building()->get_name()].cnt_under_construction--;
		total_constructionsites--;
	}
	else
		bo.cnt_built--;
}

// Road building
struct FindFieldWithFlagOrRoad:FindField {
	Economy* economy;
	virtual bool accept(FCoords coord) const;
};

bool FindFieldWithFlagOrRoad::accept (FCoords fc) const
{
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

void Computer_Player::connect_flag_to_another_economy (Flag* flag)
{
	FindFieldWithFlagOrRoad functor;
	CheckStepRoadAI check(player, MOVECAPS_WALK);
	std::vector<Coords> reachable;
	
	// first look for possible destinations
	functor.economy=flag->get_economy();
	map->find_reachable_fields (flag->get_position(), 16, &reachable, &check, functor);
	
	if (reachable.empty())
		return;
	
	// then choose the one closest to the originating flag
	int closest, distance;
	
	closest=0;
	distance=map->calc_distance(flag->get_position(), reachable[0]);
	for (unsigned int i=1; i<reachable.size(); i++) {
		int d=map->calc_distance(flag->get_position(), reachable[i]);
		
		if (d<distance) {
		    closest=i;
		    distance=d;
		}
	}
	
	// if we join a road and there is no flag yet, build one
	Field* field=map->get_field(reachable[closest]);
	if (field->get_immovable()->get_type()==BaseImmovable::ROAD)
		game->send_player_build_flag (player_number, reachable[closest]);
	
	// and finally build the road
	Path* path=new Path();
	if (map->findpath(flag->get_position(), reachable[closest], 0, path, &check) < 0)
		return;
	
	game->send_player_build_road (player_number, path);
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
	    case BaseImmovable::ROAD:
		roads.remove (static_cast<Road*>(pi));
		break;
	}
}

// this is called whenever we gain ownership of a field on the map
void Computer_Player::gain_field (const FCoords& fc)
{
	unusable_fields.push_back (fc);
}

// we don't use this - instead we check or fields regularly, see think()
void Computer_Player::lose_field (const FCoords& fc)
{
}


/* CheckStepRoadAI */
bool CheckStepRoadAI::allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const
{
	uchar endcaps = m_player->get_buildcaps(end);

	// Calculate cost and passability
	if (!(endcaps & m_movecaps)) {
		uchar startcaps = m_player->get_buildcaps(start);

		if (!((endcaps & MOVECAPS_WALK) && (startcaps & m_movecaps & MOVECAPS_SWIM)))
			return false;
	}

	// Check for blocking immovables
	BaseImmovable *imm = map->get_immovable(end);
	if (imm && imm->get_size() >= BaseImmovable::SMALL) {
//		if (id != stepLast)
//			return false;
		if (imm->get_type()==Map_Object::FLAG)
			return true;

		if ((imm->get_type() != Map_Object::ROAD || !(endcaps & BUILDCAPS_FLAG)))
			return false;
	}

	return true;
}

bool CheckStepRoadAI::reachabledest(Map* map, FCoords dest) const
{
	uchar caps = dest.field->get_caps();

	if (!(caps & m_movecaps)) {
		if (!((m_movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map->can_reach_by_water(dest))
			return false;
	}

	return true;
}


