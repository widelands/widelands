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

#include "attack_controller.h"
#include "cmd_queue.h"
#include "constructionsite.h"
#include "log.h"
#include "game.h"
#include "militarysite.h"
#include "soldier.h"
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
 const int32_t type,
 const Player_Number plnum,
 const Tribe_Descr & tribe_descr,
 const std::string & name,
 const uint8_t * const playercolor)
:
m_see_all(false),
m_egbase (the_egbase),
m_type   (type),
m_plnum  (plnum),
m_tribe  (tribe_descr),
m_fields (0)
{
	for (int32_t i = 0; i < 4; ++i)
		m_playercolor[i] = RGBColor(playercolor[i*3 + 0], playercolor[i*3 + 1], playercolor[i*3 + 2]);

	set_name(name);

	// Allow all buildings per default
	m_allowed_buildings.resize(m_tribe.get_nrbuildings());
	for (int32_t i = 0; i < m_tribe.get_nrbuildings(); ++i)
		m_allowed_buildings[i]=true;

	m_ware_productions.resize(tribe().get_nrwares());
	m_current_statistics.resize(tribe().get_nrwares());
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
		Player_Area<Area<FCoords> > starting_area
			(m_plnum,
			 Area<FCoords>(map.get_fcoords(map.get_starting_pos(m_plnum)), 0));
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
void Player::build_flag(Coords c)
{
	int32_t buildcaps = get_buildcaps(egbase().map().get_fcoords(c));

	if (buildcaps & BUILDCAPS_FLAG) Flag::create(&m_egbase, this, c);
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
			Road::create(&m_egbase, Road_Normal, start, end, path);
		} else
			log("%i: building road, missed end flag\n", get_player_number());
	} else
		log("%i: building road, missed start flag\n", get_player_number());
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

      // Get workers and soldiers
		//  Make copies of the vectors, because the originals are destroyed with
		//  the building.
		const std::vector<Worker  *> workers  = building->get_workers();
		const std::vector<Soldier *> soldiers = building->has_soldiers() ?
			dynamic_cast<ProductionSite &>(*building).get_soldiers()
			:
			std::vector<Soldier *>();

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

		// Reassign the soldier
		const std::vector<Soldier *>::const_iterator soldiers_end =
			soldiers.end();
		for
			(std::vector<Soldier *>::const_iterator it = soldiers.begin();
			 it != soldiers_end;
			 ++it)
		{
			Soldier & soldier = **it;
			soldier.set_location(building);
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
void Player::allow_building(int32_t i, bool t)
{
	assert(i < m_tribe.get_nrbuildings());
	m_allowed_buildings.resize(m_tribe.get_nrbuildings());

	m_allowed_buildings[i]=t;
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
	if (upcast(TrainingSite, ts, imm))
		if (val>0)
			ts->add_pri(static_cast<tAttribute>(atr));
		else
			ts->sub_pri(static_cast<tAttribute>(atr));
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
	if
		(soldier->get_worker_type() == Worker_Descr::SOLDIER
		 and
		 imm->get_type() >= Map_Object::BUILDING)
	{
		Building* ms= static_cast<Building*>(imm);
		ms->drop_soldier (soldier->get_serial());
	}
}

//TODO val might (theoretically) be >1 or <-1, but there's always an inc/dec by one
void Player::change_soldier_capacity (PlayerImmovable* imm, int32_t val) {
	if (imm->get_owner() != this)
		return;
	if (upcast(Building, building, imm)) {
		if (val>0)
			building->soldier_capacity_up  ();
		else
			building->soldier_capacity_down();
	}
}

/*
===============
Player::enemyflagaction

Perform an action on the given enemy flag.
===============
*/
void Player::enemyflagaction(Flag* flag, int32_t action, int32_t attacker, int32_t num, int32_t)
{
	if (attacker != get_player_number())
		throw wexception ("Player (%d) is not the sender of an attack (%d)", attacker, get_player_number());

	if (upcast(Game, game, &egbase())) {
		assert (num >= 0);

		log("++Player::EnemyFlagAction()\n");
		// Additional security check LOOK, if equal exit!!
		if (flag->get_owner() == this)
			return;
		log("--Player::EnemyFlagAction() Checkpoint!\n");

		switch (action) {
		case ENEMYFLAGACTION_ATTACK:
			game->create_attack_controller
				(flag,
				 attacker,
				 flag->get_owner()->get_player_number(),
				 static_cast<uint32_t>(num));
			break;

		default:
			log("Player sent bad enemyflagaction = %i\n", action);
		}
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
	assert(wareid < static_cast<uint32_t>(tribe().get_nrwares()));

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
 * Gain an immovable
 */
void Player::gain_immovable(PlayerImmovable* imm) {
	if
	(const Building * const building =
		dynamic_cast<const Building *>(imm))
	{
		const ConstructionSite * const constructionsite =
			dynamic_cast<const ConstructionSite *>(building);
		const std::string & building_name =
			constructionsite ?
			constructionsite->building().name() : building->name();

		const Building_Descr::Index nr_buildings = tribe().get_nrbuildings();

		// Get the valid vector for this
		if (m_building_stats.size() < nr_buildings)
			m_building_stats.resize(nr_buildings);

		std::vector<Building_Stats> & stat =
			m_building_stats[tribe().get_building_index(building_name.c_str())];

		Building_Stats new_building;
		new_building.is_constructionsite = constructionsite;
		new_building.pos = building->get_position();
		stat.push_back(new_building);
	}
}

/**
 * Loose an immovable
 */
void Player::lose_immovable(PlayerImmovable* imm)
{
	if
	(const Building * const building =
		dynamic_cast<const Building *>(imm))
	{
		const ConstructionSite * const constructionsite =
			dynamic_cast<const ConstructionSite *>(building);
		const std::string & building_name =
			constructionsite ?
			constructionsite->building().name() : building->name();

		const Building_Descr::Index nr_buildings = tribe().get_nrbuildings();

		// Get the valid vector for this
		if (m_building_stats.size() < nr_buildings)
			m_building_stats.resize(nr_buildings);

		std::vector<Building_Stats> & stat =
			m_building_stats[tribe().get_building_index(building_name.c_str())];

		const Coords building_position = building->get_position();
		for (uint32_t i = 0; i < stat.size(); ++i) {
			if (stat[i].pos == building_position) {
				stat.erase(stat.begin() + i);
				return;
			}
		}

		throw wexception
			("Interactive_Player::loose_immovable(): A building shoud be removed "
			 "at the location (%i, %i), but nothing is known about this building!",
			 building_position.x, building_position.y);
	}
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
