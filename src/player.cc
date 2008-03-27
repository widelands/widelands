/*
 * Copyright (C) 2002-2003, 2006-2008 by the Widelands Development Team
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

#include "player.h"

#include "checkstep.h"
#include "cmd_queue.h"
#include "constructionsite.h"
#include "findimmovable.h"
#include "log.h"
#include "game.h"
#include "militarysite.h"
#include "soldier.h"
#include "soldiercontrol.h"
#include "sound/sound_handler.h"
#include "transport.h"
#include "trainingsite.h"
#include "tribe.h"
#include "warehouse.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"

#include "upcast.h"

namespace Widelands {

extern Map_Object_Descr g_road_descr;

//
//
// class Player
//
//
Player::Player
	(Editor_Game_Base  & the_egbase,
	 Player_Number         const plnum,
	 Tribe_Descr   const &       tribe_descr,
	 std::string   const &       name,
	 uint8_t       const * const playercolor)
	:
	m_see_all           (false),
	m_egbase            (the_egbase),
	m_plnum             (plnum),
	m_tribe             (tribe_descr),
	m_fields            (0),
	m_allowed_buildings (tribe_descr.get_nrbuildings(), true),
	m_current_statistics(tribe().get_nrwares()),
	m_ware_productions  (tribe().get_nrwares())
{
	for (int32_t i = 0; i < 4; ++i)
		m_playercolor[i] = RGBColor(playercolor[i*3 + 0], playercolor[i*3 + 1], playercolor[i*3 + 2]);

	set_name(name);
}


Player::~Player() {
	delete[] m_fields;
}


/*
===============
Player::init

Prepare the player for in-game action
===============
*/
void Player::init(const bool place_headquarters) {
	const Map & map = egbase().map();
	if (place_headquarters) {
		const Tribe_Descr & trdesc = m_tribe;
		Coords starting_pos = map.get_starting_pos(m_plnum);

		if (!starting_pos)
			throw wexception("Player %u has no starting point", m_plnum);

		FCoords fpos = map.get_fcoords(starting_pos);

		if ((fpos.field->get_caps() & BUILDCAPS_SIZEMASK) < BUILDCAPS_BIG)
			throw wexception("Starting point of player %u is too small", m_plnum);

		Player_Area<Area<FCoords> > starting_area
			(m_plnum,
			 Area<FCoords>(fpos, 0));

		//try {
			Warehouse & headquarter = dynamic_cast<Warehouse &>
				(*egbase().warp_building
				 	(starting_area,
				 	 starting_area.player_number,
				 	 trdesc.get_building_index("headquarters")));
			starting_area.radius = headquarter.get_conquers();
			egbase().conquer_area(starting_area);
			trdesc.load_warehouse_with_start_wares(egbase(), headquarter);
		//} catch (const Descr_Maintainer<Building_Descr>::Nonexistent) {
			//throw wexception("Tribe %s lacks headquarters", tribe.get_name());
		//}
	}
}


/**
 * Allocate the fields array that contains player-specific field information.
 */
void Player::allocate_map()
{
	const Map & map = egbase().map();
	assert(map.get_width ());
	assert(map.get_height());
	m_fields = new Field[map.max_index()];
}


/*
===============
Player::get_buildcaps

Return filtered buildcaps that take the player's territory into account.
===============
*/
FieldCaps Player::get_buildcaps(const FCoords fc) const {
	const Map & map = egbase().map();
	uint8_t buildcaps = fc.field->get_caps();
	const uint8_t player_number = m_plnum;

	if (not fc.field->is_interior(player_number)) buildcaps = 0;

	// Check if a building's flag can't be build due to ownership
	else if (buildcaps & BUILDCAPS_BUILDINGMASK) {
		FCoords flagcoords;
		map.get_brn(fc, &flagcoords);
		if (not flagcoords.field->is_interior(player_number))
			buildcaps &= ~BUILDCAPS_BUILDINGMASK;

		//  Prevent big buildings that would swell over borders.
		if
			((buildcaps & BUILDCAPS_BIG) == BUILDCAPS_BIG
			 and
			 (not map.tr_n(fc).field->is_interior(player_number)
			  or
			  not map.tl_n(fc).field->is_interior(player_number)
			  or
			  not map. l_n(fc).field->is_interior(player_number)))
			buildcaps &= ~BUILDCAPS_SMALL;
	}

	return static_cast<FieldCaps>(buildcaps);
}


/*
===============
Player::build_flag

Build a flag, checking that it's legal to do so.
===============
*/
void Player::build_flag(Coords const c) {
	int32_t buildcaps = get_buildcaps(egbase().map().get_fcoords(c));

	if (buildcaps & BUILDCAPS_FLAG)
		Flag::create(&egbase(), this, c);
}


Flag & Player::force_flag(FCoords const c) {
	log("Forcing flag at (%i, %i)\n", c.x, c.y);
	Map const & map = egbase().map();
	if (BaseImmovable * const immovable = c.field->get_immovable())
		if (upcast(Flag, existing_flag, immovable)) {
			if (&existing_flag->owner() == this)
				return *existing_flag;
		} else if (not dynamic_cast<Road const *>(immovable)) //  A road is OK.
			immovable->remove(&egbase()); //  Make room for the flag.
	MapRegion<Area<FCoords> > mr(map, Area<FCoords>(c, 1));
	do if (upcast(Flag, flag, mr.location().field->get_immovable()))
		flag->remove(&egbase()); //  Remove all flags that are too close.
	while (mr.advance(map));
	egbase().conquer_area //  Make sure that the player owns the area around.
		(Player_Area<Area<FCoords> >(get_player_number(), Area<FCoords>(c, 1)));
	return *Flag::create(&egbase(), this, c);
}

/*
===============
Player::build_road

Build a road along the given path.
Perform sanity checks (ownership, flags).

Note: the diagnostic log messages aren't exactly errors. They might happen
in some situations over the network.
===============
*/
void Player::build_road(const Path & path) {
	Map & map = egbase().map();
	FCoords fc = map.get_fcoords(path.get_start());
	if (upcast(Flag, start, fc.field->get_immovable())) {
		if (upcast(Flag, end, map.get_immovable(path.get_end()))) {

			//  Verify ownership of the path.
			const int32_t laststep = path.get_nsteps() - 1;
			for (int32_t i = 0; i < laststep; ++i) {
				fc = map.get_neighbour(fc, path[i]);

				if (BaseImmovable * const imm = fc.field->get_immovable())
					if (imm->get_size() >= BaseImmovable::SMALL) {
						log
							("%i: building road, immovable in the way, type=%d\n",
							 get_player_number(), imm->get_type());
						return;
					}
				if (!(get_buildcaps(fc) & MOVECAPS_WALK)) {
					log("%i: building road, unwalkable\n", get_player_number());
					return;
				}
			}
			Road::create(egbase(), *start, *end, path);
		} else
			log("%i: building road, missed end flag\n", get_player_number());
	} else
		log("%i: building road, missed start flag\n", get_player_number());
}


void Player::force_road(Path const & path, bool const create_carrier) {
	Map & map = egbase().map();
	FCoords c = map.get_fcoords(path.get_start());
	Flag & start = force_flag(c);
	Flag & end   = force_flag(map.get_fcoords(path.get_end()));

	Path::Step_Vector::size_type const laststep = path.get_nsteps() - 1;
	for (Path::Step_Vector::size_type i = 0; i < laststep; ++i) {
		c = map.get_neighbour(c, path[i]);
		log("Clearing for road at (%i, %i)\n", c.x, c.y);
		egbase().conquer_area
			(Player_Area<Area<FCoords> >
			 	(get_player_number(), Area<FCoords>(c, 1)));
		if (BaseImmovable * const immovable = c.field->get_immovable()) {
			assert(immovable != &start);
			assert(immovable != &end);
			immovable->remove(&egbase());
		}
	}
	Road::create(egbase(), start, end, path, create_carrier);
}


void Player::force_building
	(Coords const location, Building_Index const idx, bool const fill)
{
	Map & map = egbase().map();
	FCoords c[4]; //  Big buildings occupy 4 locations.
	c[0] = map.get_fcoords(location);
	map.get_brn(c[0], &c[1]);
	force_flag(c[1]);
	if (BaseImmovable * const immovable = c[0].field->get_immovable())
		immovable->remove(&egbase());
	Building_Descr const & descr = *tribe().get_building_descr(idx);
	{
		size_t nr_locations = 1;
		if ((descr.get_size() & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG) {
			nr_locations = 4;
			map.get_trn(c[0], &c[1]);
			map.get_tln(c[0], &c[2]);
			map.get_ln (c[0], &c[3]);
		}
		for (size_t i = 0; i < nr_locations; ++i) {
			egbase().conquer_area
				(Player_Area<Area<FCoords> >
				 	(get_player_number(), Area<FCoords>(c[i], 1)));
			if (BaseImmovable * const immovable = c[i].field->get_immovable())
				immovable->remove(&egbase());
		}
	}
	descr.create(egbase(), *this, c[0], false, fill);
}


/*
===============
Player::build

Place a construction site, checking that it's legal to do so.
===============
*/
void Player::build(Coords c, int32_t idx)
{
	int32_t buildcaps;
	Building_Descr* descr;

	// Validate building type
	if (idx < 0 || idx >= tribe().get_nrbuildings())
		return;
	descr = tribe().get_building_descr(idx);

	if (!descr->get_buildable())
		return;


	// Validate build position
	const Map & map = egbase().map();
	map.normalize_coords(&c);
	buildcaps = get_buildcaps(map.get_fcoords(c));

	if (descr->get_ismine())
		{
		if (!(buildcaps & BUILDCAPS_MINE))
			return;
		}
	else
		{
		if ((buildcaps & BUILDCAPS_SIZEMASK) < (descr->get_size() - BaseImmovable::SMALL + 1))
			return;
		}

	egbase().warp_constructionsite(c, m_plnum, idx);
}


/*
===============
Player::bulldoze

Bulldoze the given road, flag or building.
===============
*/
void Player::bulldoze(PlayerImmovable* imm)
{
	// General security check
	if (imm->get_owner() != this)
		return;

	// Extended security check
	if (upcast(Building, building, imm)) {
		if (!(building->get_playercaps() & (1 << Building::PCap_Bulldoze)))
			return;
	} else if (upcast(Flag, flag, imm)) {
		if (Building * const flagbuilding = flag->get_building())
		if (!(flagbuilding->get_playercaps() & (1 << Building::PCap_Bulldoze))) {
			log
				("Player trying to rip flag (%u) with undestroyable building "
				 "(%u)\n",
				 flag->get_serial(), flagbuilding->get_serial());
			return;
		}
	} else if (dynamic_cast<Road *>(imm)); // no additional check
	else
		throw wexception
			("Player::bulldoze(%u): bad immovable type", imm->get_serial());

	// Now destroy it
	imm->destroy(&egbase());
}

void Player::start_stop_building(PlayerImmovable* imm) {
	if (imm->get_owner() != this)
		return;
	if (upcast(Building, building, imm))
		building->set_stop(!building->get_stop());
}

/*
 * enhance this building, remove it, but give the constructionsite
 * an idea of enhancing
 */
void Player::enhance_building
(Building * building, const Building_Descr::Index index_of_new_building)
{
	if (building->get_owner() == this) {
		const Building_Descr::Index index_of_old_building =
			tribe().get_building_index(building->name().c_str());
		const Coords position = building->get_position();

		//  Get workers and soldiers
		//  Make copies of the vectors, because the originals are destroyed with
		//  the building.
		const std::vector<Worker  *> workers  = building->get_workers();

		building->remove(&egbase()); //  no fire or stuff
		//  Hereafter the old building does not exist and building is a dangling
		//  pointer.
		building = egbase().warp_constructionsite
			(position, m_plnum, index_of_new_building, index_of_old_building);
		//  Hereafter building points to the new building.

		// Reassign the workers and soldiers.
		// Note that this will make sure they stay within the economy;
		// However, they are no longer associated with the building as
		// workers of that buiding, which is why they will leave for a
		// warehouse.
		const std::vector<Worker *>::const_iterator workers_end = workers.end();
		for
			(std::vector<Worker *>::const_iterator it = workers.begin();
			 it != workers_end;
			 ++it)
		{
			Worker & worker = **it;
			worker.set_location(building);
		}
	}
}


/*
===============
Player::flagaction

Perform an action on the given flag.
===============
*/
void Player::flagaction(Flag* flag, int32_t action)
{
	if (upcast(Game, game, &egbase()))
		if (flag->get_owner() == this) {// Additional security check.
		switch (action) {
		case FLAGACTION_GEOLOGIST:
			//try {
				flag->add_flag_job
					(game, tribe().get_worker_index("geologist"), "expedition");
			/*} catch (Descr_Maintainer<Worker_Descr>::Nonexistent) {
				log("Tribe defines no geologist\n");
			} */
			break;
		default:
			log("Player sent bad flagaction = %i\n", action);
		}
		}
}

/*
 * allow building
 *
 * Disable or enable a building for a player
 */
void Player::allow_building(Building_Index const i, bool const allow) {
	assert(i.value() < m_allowed_buildings.size());
	m_allowed_buildings[i.value()] = allow;
}

/*
 * Economy stuff below
 */
void Player::add_economy(Economy* eco)
{
	if (has_economy(eco))
		return;
	m_economies.push_back(eco);
}

void Player::remove_economy(Economy* eco) {
	if (!has_economy(eco)) return;
	std::vector<Economy*>::iterator i = m_economies.begin();
	while (i!=m_economies.end()) {
		if (*i == eco) {
			m_economies.erase(i);
			return;
		}
		++i;
	}
	assert(false); // Never here
}

bool Player::has_economy(Economy * const economy) const throw () {
	const std::vector<Economy *>::const_iterator economies_end =
		m_economies.end();
	for
		(std::vector<Economy *>::const_iterator it = m_economies.begin();
		 it != economies_end;
		 ++it)
		if (*it == economy) return true;
	return false;
}

std::vector<Economy *>::size_type Player::get_economy_number
(Economy const * const economy) const
throw ()
{
	economy_vector::const_iterator const
		economies_end = m_economies.end(), economies_begin = m_economies.begin();
	for
		(std::vector<Economy *>::const_iterator it = economies_begin;
		 it != economies_end;
		 ++it)
		if (*it == economy) return it - economies_begin;
	assert(false); // never here
	return 0;
}

/************  Military stuff  **********/

/*
==========
Player::change_training_options

Change the training priotity values
==========
*/
void Player::change_training_options(PlayerImmovable* imm, int32_t atr, int32_t val) {
	if (imm->get_owner() != this)
		return;
	if (upcast(TrainingSite, ts, imm)) {
		tAttribute attr = static_cast<tAttribute>(atr);
		int32_t prio = ts->get_pri(attr);
		ts->set_pri(attr, prio+val);
	}
}

/*
===========
Player::drop_soldier

Forces the drop of given soldier at given house
===========
*/
void Player::drop_soldier(PlayerImmovable* imm, Soldier* soldier) {
	if (imm->get_owner() != this)
		return;
	if (soldier->get_worker_type() != Worker_Descr::SOLDIER)
		return;
	if (upcast(SoldierControl, ctrl, imm))
		ctrl->dropSoldier(soldier);
}

void Player::change_soldier_capacity (PlayerImmovable* imm, int32_t val) {
	if (imm->get_owner() != this)
		return;
	if (upcast(SoldierControl, ctrl, imm)) {
		uint32_t current = ctrl->soldierCapacity();
		uint32_t capacity = current + val;

		if (!capacity || (val < 0 && capacity > current))
			capacity = 1;

		ctrl->setSoldierCapacity(capacity);
	}
}

/**
 * Get a list of soldiers that this player can be used to attack the
 * building at the given flag.
 *
 * The default attack should just take the first N soldiers of the
 * returned array.
 *
 * \todo Perform a meaningful sort on the soldiers array.
 */
uint32_t Player::findAttackSoldiers(Flag* flag, std::vector<Soldier*>* soldiers)
{
	uint32_t count = 0;

	if (soldiers)
		soldiers->clear();

	Map& map = egbase().map();
	std::vector<BaseImmovable*> immovables;

	map.find_reachable_immovables_unique
		(Area<FCoords>(map.get_fcoords(flag->get_position()), 25),
		 &immovables,
		 CheckStepWalkOn(MOVECAPS_WALK, false),
		 FindImmovablePlayerMilitarySite(this));

	if (!immovables.size())
		return 0;

	for
		(std::vector<BaseImmovable*>::const_iterator it = immovables.begin();
		 it != immovables.end();
		 ++it)
	{
		upcast(MilitarySite, ms, *it);
		std::vector<Soldier*> present = ms->presentSoldiers();

		if (present.size() > 1) {
			if (soldiers)
				soldiers->insert(soldiers->end(), present.begin()+1, present.end());
			count += present.size() - 1;
		}
	}

	return count;
}


/**
 * \todo Clean this mess up. The only action we really have right now is
 * to attack, so pretending we have more types is pointless.
 */
void Player::enemyflagaction(Flag* flag, int32_t action, int32_t attacker, int32_t num, int32_t)
{
	upcast(Game, game, &egbase());
	assert(game);

	if (attacker != get_player_number()) {
		log("Player (%d) is not the sender of an attack (%d)\n", attacker, get_player_number());
		return;
	}
	if (action != ENEMYFLAGACTION_ATTACK) {
		log("enemyflagaction: unsupported action\n");
		return;
	}
	if (num <= 0) {
		log("enemyflagaction: num == %i\n", num);
		return;
	}

	if (flag->get_owner() == this)
		return;
	Building* building = flag->get_building();
	if (!building)
		return;

	upcast(Attackable, attackable, building);
	if (!attackable || !attackable->canAttack())
		return;

	std::vector<Soldier*> attackers;
	findAttackSoldiers(flag, &attackers);

	num = std::min(static_cast<int32_t>(attackers.size()), num);
	for(int32_t i = 0; i < num; ++i) {
		Soldier* soldier = attackers[i];
		upcast(MilitarySite, ms, soldier->get_location(&egbase()));

		ms->sendAttacker(soldier, building);
	}
}


inline void Player::discover_node
(const Map     & map,
 const Widelands::Field & first_map_field,
 const FCoords   f,
 Field         & field)
throw ()
{
	assert(0 <= f.x);
	assert(f.x < map.get_width());
	assert(0 <= f.y);
	assert(f.y < map.get_height());
	assert(&map[0] <= f.field);
	assert           (f.field < &map[0] + map.max_index());
	assert(m_fields <= &field);
	assert            (&field < m_fields + map.max_index());
	assert(field.vision <= 1);

	{// discover everything (above the ground) in this field
		field.terrains = f.field->get_terrains();
		field.roads    = f.field->get_roads   ();
		field.owner    = f.field->get_owned_by();
		{//  map_object_descr[TCoords::None]

			const Map_Object_Descr * map_object_descr;
			if (const BaseImmovable * base_immovable = f.field->get_immovable()) {
				map_object_descr = &base_immovable->descr();
				if (map_object_descr == &g_road_descr) map_object_descr = 0;
				else if (upcast(Building const, building, base_immovable))
					if (building->get_position() != f)
						//  TODO This is not the buildidng's main position so we can
						//  TODO not see it. But it should be possible to see it from
						//  TODO a distance somehow.
						map_object_descr = 0;
			} else map_object_descr = 0;
			field.map_object_descr[TCoords<>::None] = map_object_descr;
		}
	}
	{//  discover the D triangle and the SW edge of the top right neighbour field
		FCoords tr = map.tr_n(f);
		Field & tr_field = m_fields[tr.field - &first_map_field];
		if (tr_field.vision <= 1) {
			tr_field.terrains.d = tr.field->terrain_d();
			tr_field.roads &= ~(Road_Mask << Road_SouthWest);
			tr_field.roads |= Road_Mask << Road_SouthWest & tr.field->get_roads();
		}
	}
	{//  discover both triangles and the SE edge of the top left  neighbour field
		FCoords tl = map.tl_n(f);
		Field & tl_field = m_fields[tl.field - &first_map_field];
		if (tl_field.vision <= 1) {
			tl_field.terrains = tl.field->get_terrains();
			tl_field.roads &= ~(Road_Mask << Road_SouthEast);
			tl_field.roads |= Road_Mask << Road_SouthEast & tl.field->get_roads();
		}
	}
	{//  discover the R triangle and the  E edge of the     left  neighbour field
		FCoords l = map.l_n(f);
		Field & l_field = m_fields[l.field - &first_map_field];
		if (l_field.vision <= 1) {
			l_field.terrains.r = l.field->terrain_r();
			l_field.roads &= ~(Road_Mask << Road_East);
			l_field.roads |= Road_Mask << Road_East & l.field->get_roads();
		}
	}
}

void Player::see_node
(const Map                  & map,
 const Widelands::Field & first_map_field,
 const FCoords                f,
 const Time    gametime,
 const bool                   lasting)
throw ()
{
	assert(0 <= f.x);
	assert(f.x < map.get_width());
	assert(0 <= f.y);
	assert(f.y < map.get_height());
	assert(&map[0] <= f.field);
	assert           (f.field < &first_map_field + map.max_index());

	Field & field = m_fields[f.field - &first_map_field];
	assert(m_fields <= &field);
	assert            (&field < m_fields + map.max_index());
	Vision fvision = field.vision;
	if (fvision == 0) fvision = 1;
	if (fvision == 1) {
		if (not lasting) field.time_node_last_unseen = gametime;
		discover_node(map, first_map_field, f, field);
	}
	fvision += lasting;
	field.vision = fvision;
}


/**
 * Called by Game::think to sample statistics data in regular intervals.
 */
void Player::sample_statistics()
{
	assert (m_ware_productions.size() == static_cast<uint32_t>(tribe().get_nrwares()));

	for (uint32_t i = 0; i < m_ware_productions.size(); ++i) {
		m_ware_productions[i].push_back(m_current_statistics[i]);
		m_current_statistics[i] = 0;
	}
}


/**
 * A ware was produced. Update the corresponding statistics.
 */
void Player::ware_produced(Ware_Index const wareid) {
	assert (m_ware_productions.size() == static_cast<uint32_t>(tribe().get_nrwares()));
	assert(wareid.value() < static_cast<uint32_t>(tribe().get_nrwares()));

	++m_current_statistics[wareid.value()];
}


/**
 * Get current ware production statistics
 */
const std::vector<uint32_t> * Player::get_ware_production_statistics
		(const int32_t ware) const
{
	assert(ware < static_cast<int32_t>(m_ware_productions.size()));

	return &m_ware_productions[ware];
}


/**
 * Add or remove the given building from building statistics.
 * Only to be called by \ref receive
 */
void Player::update_building_statistics(Building* building, losegain_t lg)
{
	upcast(ConstructionSite const, constructionsite, building);
	const std::string & building_name =
		constructionsite ?
		constructionsite->building().name() : building->name();

	const Building_Descr::Index nr_buildings = tribe().get_nrbuildings();

	// Get the valid vector for this
	if (m_building_stats.size() < nr_buildings)
		m_building_stats.resize(nr_buildings);

	std::vector<Building_Stats> & stat =
		m_building_stats[tribe().get_building_index(building_name.c_str())];

	if (lg == GAIN) {
		Building_Stats new_building;
		new_building.is_constructionsite = constructionsite;
		new_building.pos = building->get_position();
		stat.push_back(new_building);
	} else {
		const Coords building_position = building->get_position();
		for (uint32_t i = 0; i < stat.size(); ++i) {
			if (stat[i].pos == building_position) {
				stat.erase(stat.begin() + i);
				return;
			}
		}

		throw wexception
			("Interactive_Player::loose_immovable(): A building should be "
			 "removed at (%i, %i), but nothing is known about this building!",
			 building_position.x, building_position.y);
	}
}


void Player::receive(const NoteImmovable& note)
{
	if (upcast(Building, building, note.pi))
		update_building_statistics(building, note.lg);

	NoteSender<NoteImmovable>::send(note);
}


void Player::receive(const NoteField& note)
{
	NoteSender<NoteField>::send(note);
}


/**
 * Read statistics data from a file.
 *
 * \param version indicates the kind of statistics file, which may be
 *   0 - old style statistics (from the time when statistics were kept in
 *       Interactive_Player)
 * \param fr UNDOCUMENTED
 *
 * \todo Is the documentation for parameter version complete? Doesn't look like
 * it.
 * \todo Document parameter fr
 */
void Player::ReadStatistics(FileRead& fr, uint32_t version)
{
	if (version == 0) {
		uint16_t nr_wares = fr.Unsigned16();
		uint16_t nr_entries = fr.Unsigned16();

		if (nr_wares > 0) {
			if (nr_wares != tribe().get_nrwares())
				throw wexception
					("Statistics for %u has bad number of wares (%u != %u)",
					 get_player_number(),
					 nr_wares, tribe().get_nrwares());

			assert(m_ware_productions.size() == nr_wares);
			assert(m_current_statistics.size() == nr_wares);

			for (uint32_t i = 0; i < m_current_statistics.size(); ++i) {
				m_current_statistics[i] = fr.Unsigned32();
				m_ware_productions[i].resize(nr_entries);

				for (uint32_t j = 0; j < m_ware_productions[i].size(); ++j)
					m_ware_productions[i][j] = fr.Unsigned32();
			}
		}
	} else
		throw wexception("Unsupported version %i", version);
}


/**
 * Write statistics data to the give file
 */
void Player::WriteStatistics(FileWrite& fw) const {
	fw.Unsigned16(m_current_statistics.size());
	fw.Unsigned16(m_ware_productions[0].size());

	for (uint32_t i = 0; i < m_current_statistics.size(); ++i) {
		fw.Unsigned32(m_current_statistics[i]);
		for (uint32_t j = 0; j < m_ware_productions[i].size(); ++j)
			fw.Unsigned32(m_ware_productions[i][j]);
	}
}

};
