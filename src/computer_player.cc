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

#include <queue>
#include <typeinfo>
#include "error.h"
#include "map.h"
#include "transport.h"
#include "player.h"
#include "tribe.h"
#include "constructionsite.h"
#include "productionsite.h"
#include "militarysite.h"
#include "computer_player.h"

class CheckStepRoadAI : public CheckStep {
public:
	CheckStepRoadAI(Player* pl, uchar mc, bool oe)
		: player(pl), movecaps(movecaps), openend(oe)
	{ }
	
	void set_openend (bool oe)
	{ openend=oe; }

	virtual bool allowed(Map* map, FCoords start, FCoords end, int dir, StepId id) const;
	virtual bool reachabledest(Map* map, FCoords dest) const;

private:
	Player*		player;
	uchar		movecaps;
	bool		openend;
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
		Building_Descr* bld=tribe->get_building_descr(i);
		log ("\tcan build '%s', id is %d\n",bld->get_name(),i);
		
		buildings.push_back (BuildingObserver());
		
		BuildingObserver& bo=buildings.back();
		bo.name=bld->get_name();
		bo.id=i;
		bo.desc=bld;
		bo.type=BuildingObserver::BORING;
		bo.cnt_built=0;
		bo.cnt_under_construction=0;
		
		bo.is_buildable=bld->get_buildable();
		
		bo.need_trees=false;
		bo.need_stones=false;
		
		// FIXME: define these properties in the building's conf file
		if (!strcmp(bld->get_name(), "quarry"))
		    bo.need_stones=true;

		if (!strcmp(bld->get_name(), "lumberjack"))
		    bo.need_trees=true;
		
		if (typeid(*bld)==typeid(ConstructionSite_Descr)) {
			bo.type=BuildingObserver::CONSTRUCTIONSITE;
			continue;
		}
		
		if (typeid(*bld)==typeid(MilitarySite_Descr)) {
			bo.type=BuildingObserver::MILITARYSITE;
			continue;
		}
		
		if (typeid(*bld)==typeid(ProductionSite_Descr)) {
		    ProductionSite_Descr* prod=static_cast<ProductionSite_Descr*>(bld);
		    
		    bo.type=BuildingObserver::PRODUCTIONSITE;
		    
		    std::set<std::string>::iterator j;
		    for (j=prod->get_outputs()->begin();j!=prod->get_outputs()->end();j++)
			bo.outputs.push_back (tribe->get_ware_index(j->c_str()));

		    continue;
		}
	}
	
	total_constructionsites=0;
	next_construction_due=0;
	inhibit_road_building=0;
}

Computer_Player::~Computer_Player ()
{
}

Computer_Player::BuildingObserver& Computer_Player::get_building_observer (const char* name)
{
	std::list<BuildingObserver>::iterator i;
    
	for (i=buildings.begin();i!=buildings.end();i++)
		if (!strcmp(i->name, name))
			return *i;
    
	throw wexception("Help: I don't know what to do with a %s", name);
}

void Computer_Player::think ()
{
	// update our fields 
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
	if (!buildable_fields.empty() && buildable_fields.front().next_update_due<=game->get_gametime()) {
		update_buildable_field (buildable_fields.front());
		
		buildable_fields.front().next_update_due=game->get_gametime() + 750;
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
	
	// wait a moment so that all fields are classified
	if (next_construction_due==0)
	    next_construction_due=game->get_gametime() + 3000;
	
	// now build something if possible
	if (next_construction_due<=game->get_gametime()) {
	    next_construction_due=game->get_gametime() + 1500;
	
	    if (construct_building()) {
		inhibit_road_building=game->get_gametime() + 2000;
		return;
	    }
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
		
		bool finish=false;
		
		if (inhibit_road_building<=game->get_gametime()) {
		    // try to connect to another economy
		    if (economies.size()>1)
			finish=connect_flag_to_another_economy(i->flags.front());
		
		    if (!finish)
			finish=improve_roads(i->flags.front());
		}

		// cycle through flags one at a time
		i->flags.push_back (i->flags.front());
		i->flags.pop_front ();
		
		if (finish)
		    return;
		
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
}

bool Computer_Player::construct_building ()
{
	int spots_avail[4];
	int i;
	
	for (i=0;i<4;i++)
		spots_avail[i]=0;
	
	for (std::list<BuildableField>::iterator i=buildable_fields.begin(); i!=buildable_fields.end(); i++)
		spots_avail[i->field->get_caps() & BUILDCAPS_SIZEMASK]++;
	
	int expand_factor=1;
	
	if (spots_avail[BUILDCAPS_BIG]<2)
		expand_factor++;
	if (spots_avail[BUILDCAPS_MEDIUM]+spots_avail[BUILDCAPS_BIG]<4)
		expand_factor++;
	if (spots_avail[BUILDCAPS_SMALL]+spots_avail[BUILDCAPS_MEDIUM]+spots_avail[BUILDCAPS_BIG]<8)
		expand_factor++;
	
	int proposed_building=-1;
	int proposed_priority=0;
	Coords proposed_coords;
	
	for (std::list<BuildableField>::iterator i=buildable_fields.begin(); i!=buildable_fields.end(); i++) {
		if (!i->reachable)
			continue;
		
		int maxsize=i->field->get_caps() & BUILDCAPS_SIZEMASK;
		int prio;
		
		std::list<BuildingObserver>::iterator j;
		for (j=buildings.begin();j!=buildings.end();j++) {
		    if (!j->is_buildable)
			    continue;
		
		    if (j->type==BuildingObserver::MINE)
			    continue;
		
		    if (j->desc->get_size()>maxsize)
			    continue;
		    
		    prio=0;
		    
		    if (j->type==BuildingObserver::MILITARYSITE)
			    prio=(i->unowned_land_nearby - i->military_influence*2) * expand_factor / 4;

		    if (j->type==BuildingObserver::PRODUCTIONSITE) {
			    if (j->need_trees)
				    prio+=i->trees_nearby - 8*i->tree_consumers_nearby;

			    if (j->need_stones)
				    prio+=i->stones_nearby - 8*i->stone_consumers_nearby;
		    }

		    if (i->preferred)
			prio+=prio/2 + 1;
		    else
			prio--;

		    // don't waste good land for small huts
		    prio-=(maxsize - j->desc->get_size()) * 6;
		    
		    // don't have too many construction sites
		    prio-=total_constructionsites*total_constructionsites;

		    if (prio>proposed_priority) {
			    proposed_building=j->id;
			    proposed_priority=prio;
			    proposed_coords=*i;
		    }
		}
	}
	
	// if we want to construct a new building, send the command now
	if (proposed_building>=0) {
		log ("ComputerPlayer(%d): want to construct building %d\n", player_number, proposed_building);
		
		game->send_player_build (player_number, proposed_coords, proposed_building);
		
		return true;
	}
	
	return false;
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
	
	field.unowned_land_nearby=map->find_fields(field, 8, 0, find_unowned);
	
	// collect information about resources in the area
	std::vector<ImmovableFound> immovables;

	const int tree_attr=Map_Object_Descr::get_attribute_id("tree");
	const int stone_attr=Map_Object_Descr::get_attribute_id("stone");
	
	map->find_immovables (field, 8, &immovables);
	
	field.reachable=false;	
	field.preferred=false;
	
	field.military_influence=0;
	field.trees_nearby=0;
	field.stones_nearby=0;
	field.tree_consumers_nearby=0;
	field.stone_consumers_nearby=0;
	
	FCoords fse;
	map->get_neighbour (field, Map_Object::WALK_SE, &fse);
	
	BaseImmovable* imm=fse.field->get_immovable();
	if (imm!=0) {
	    if (imm->get_type()==BaseImmovable::FLAG)
		field.preferred=true;
		
	    if (imm->get_type()==BaseImmovable::ROAD && (fse.field->get_caps() & BUILDCAPS_FLAG))
		field.preferred=true;
	}
	
	for (unsigned int i=0;i<immovables.size();i++) {
		if (immovables[i].object->get_type()==BaseImmovable::FLAG)
			field.reachable=true;

		if (immovables[i].object->get_type()==BaseImmovable::BUILDING) {
			Building* bld=static_cast<Building*>(immovables[i].object);

			if (bld->get_building_type()==Building::CONSTRUCTIONSITE) {
			    Building_Descr* con=static_cast<ConstructionSite*>(bld)->get_building();
			    
			    if (typeid(*con)==typeid(MilitarySite_Descr)) {
				MilitarySite_Descr* mil=static_cast<MilitarySite_Descr*>(con);
				
				int v=mil->get_conquers() - map->calc_distance(field, immovables[i].coords);
				
				if (v>0)
				    field.military_influence+=(v*v+v)*6;
			    }
			    
			    if (typeid(*con)==typeid(ProductionSite_Descr))
				consider_productionsite_influence (field, immovables[i].coords,
					get_building_observer(con->get_name()));
			}

			if (bld->get_building_type()==Building::MILITARYSITE) {
			    MilitarySite* mil=static_cast<MilitarySite*>(bld);
			    
			    int v=mil->get_conquers() - map->calc_distance(field, immovables[i].coords);
			    
			    if (v>0)
				field.military_influence+=v*v*mil->get_capacity();
			}
			
			if (bld->get_building_type()==Building::PRODUCTIONSITE)
			    consider_productionsite_influence (field, immovables[i].coords,
				    get_building_observer(bld->get_name()));

			continue;
		}
		
		if (immovables[i].object->has_attribute(tree_attr))
			field.trees_nearby++;

		if (immovables[i].object->has_attribute(stone_attr))
			field.stones_nearby++;
	}
	
	log ("Military influence for updated field is %d\n", field.military_influence);
}

void Computer_Player::consider_productionsite_influence (BuildableField& field, const Coords& coord, const BuildingObserver& bo)
{
	if (bo.need_trees)
		field.tree_consumers_nearby++;

	if (bo.need_stones)
		field.stone_consumers_nearby++;
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
	BuildingObserver& bo=get_building_observer(b->get_name());
	
	if (bo.type==BuildingObserver::CONSTRUCTIONSITE) {
		get_building_observer(static_cast<ConstructionSite*>(b)->get_building()->get_name()).cnt_under_construction++;
	    	total_constructionsites++;
	}
	else
		bo.cnt_built++;
}

void Computer_Player::lose_building (Building* b)
{
	BuildingObserver& bo=get_building_observer(b->get_name());
	
	if (bo.type==BuildingObserver::CONSTRUCTIONSITE) {
		get_building_observer(static_cast<ConstructionSite*>(b)->get_building()->get_name()).cnt_under_construction--;
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

bool Computer_Player::connect_flag_to_another_economy (Flag* flag)
{
	FindFieldWithFlagOrRoad functor;
	CheckStepRoadAI check(player, MOVECAPS_WALK, true);
	std::vector<Coords> reachable;
	
	// first look for possible destinations
	functor.economy=flag->get_economy();
	map->find_reachable_fields (flag->get_position(), 16, &reachable, &check, functor);
	
	if (reachable.empty())
		return false;
	
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
	check.set_openend (false);
	if (map->findpath(flag->get_position(), reachable[closest], 0, path, &check) < 0)
		return false;
	
	game->send_player_build_road (player_number, path);
	return true;
}

struct NearFlag {
    Flag*	flag;
    long	cost;
    long	distance;
    
    NearFlag (Flag* f, long c, long d)
    { flag=f; cost=c; distance=d; }
    
    bool operator< (const NearFlag& f) const
    { return cost>f.cost; }
    
    bool operator== (const Flag* f) const
    { return flag==f; }
};

struct CompareDistance {
    bool operator() (const NearFlag& a, const NearFlag& b) const
    { return a.distance < b.distance; }
};

bool Computer_Player::improve_roads (Flag* flag)
{
	std::priority_queue<NearFlag> queue;
	std::vector<NearFlag> nearflags;
	int i;
	
	queue.push (NearFlag(flag, 0, 0));
	
	while (!queue.empty()) {
    	    std::vector<NearFlag>::iterator f=find(nearflags.begin(), nearflags.end(), queue.top().flag);
	    if (f!=nearflags.end()) {
		queue.pop ();
		continue;
	    }
	    
	    nearflags.push_back (queue.top());
	    queue.pop ();
	    
	    NearFlag& nf=nearflags.back();
	    
	    for (i=1;i<=6;i++) {
		Road* road=nf.flag->get_road(i);
		
		if (!road) continue;
		
		Flag* endflag=road->get_flag(Road::FlagStart);
		if (endflag==nf.flag)
		    endflag=road->get_flag(Road::FlagEnd);
		
		long dist=map->calc_distance(flag->get_position(), endflag->get_position());
		if (dist>16)	// out of range
		    continue;
		
		queue.push (NearFlag(endflag, nf.cost+road->get_path().get_nsteps(), dist));
	    }
	}
	
	sort (nearflags.begin(), nearflags.end(), CompareDistance());

	CheckStepRoadAI check(player, MOVECAPS_WALK, false);
	
	for (i=1;i<nearflags.size();i++) {
	    NearFlag& nf=nearflags[i];
	    
	    if (2*nf.distance+2>=nf.cost)
		continue;
		
	    Path* path=new Path();
	    if (map->findpath(flag->get_position(), nf.flag->get_position(), 0, path, &check)>=0 &&
		2*path->get_nsteps()+2<nf.cost) {

		log ("Improved road graph: %ld -> %d\n", nf.cost, path->get_nsteps());

		game->send_player_build_road (player_number, path);
		return true;
	    }
	    
	    delete path;
	}

	return false;
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
	uchar endcaps = player->get_buildcaps(end);

	// Calculate cost and passability
	if (!(endcaps & movecaps)) {
		return false;
//		uchar startcaps = player->get_buildcaps(start);

//		if (!((endcaps & MOVECAPS_WALK) && (startcaps & movecaps & MOVECAPS_SWIM)))
//			return false;
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
	uchar caps = dest.field->get_caps();

	if (!(caps & movecaps)) {
		if (!((movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map->can_reach_by_water(dest))
			return false;
	}

	return true;
}


