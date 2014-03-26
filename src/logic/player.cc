/*
 * Copyright (C) 2002-2003, 2006-2013 by the Widelands Development Team
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

#include "logic/player.h"

#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "i18n.h"
#include "log.h"
#include "logic/building.h"
#include "logic/checkstep.h"
#include "logic/cmd_expire_message.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/constructionsite.h"
#include "logic/findimmovable.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/militarysite.h"
#include "logic/playercommand.h"
#include "logic/soldier.h"
#include "logic/soldiercontrol.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "sound/sound_handler.h"
#include "upcast.h"
#include "warning.h"
#include "wexception.h"
#include "wui/interactive_player.h"


namespace {
void terraform_for_building
	(Widelands::Editor_Game_Base& egbase, const Widelands::Player_Number player_number,
	 const Widelands::Coords location, const Widelands::Building_Descr* descr)
{
	Widelands::Map & map = egbase.map();
	Widelands::FCoords c[4]; //  Big buildings occupy 4 locations.
	c[0] = map.get_fcoords(location);
	map.get_brn(c[0], &c[1]);
	if (Widelands::BaseImmovable * const immovable = c[0].field->get_immovable())
		immovable->remove(egbase);
	{
		size_t nr_locations = 1;
		if ((descr->get_size() & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG)
		{
			nr_locations = 4;
			map.get_trn(c[0], &c[1]);
			map.get_tln(c[0], &c[2]);
			map.get_ln (c[0], &c[3]);
		}
		for (size_t i = 0; i < nr_locations; ++i) {
			//  Make sure that the player owns the area around.
			egbase.conquer_area_no_building
				(Widelands::Player_Area<Widelands::Area<Widelands::FCoords> >
				 	(player_number, Widelands::Area<Widelands::FCoords>(c[i], 1)));

			if (Widelands::BaseImmovable * const immovable = c[i].field->get_immovable())
				immovable->remove(egbase);
		}
	}
}



}

namespace Widelands {

extern const Map_Object_Descr g_road_descr;

const RGBColor Player::Colors[MAX_PLAYERS] = {
	RGBColor(2,     2, 198),  // blue
	RGBColor(255,  41,   0),  // red
	RGBColor(255, 232,   0),  // yellow
	RGBColor(59,  223,   3),  // green
	RGBColor(57,   57,  57),  // black/dark gray
	RGBColor(255, 172,   0),  // orange
	RGBColor(215,   0, 218),  // purple
	RGBColor(255, 255, 255),  // white
};

/**
 * Find the longest possible enhancement chain leading to the given
 * building descr. The FormerBuildings given in reference must be empty and will be
 * filled with the Building_Descr.
 */
void find_former_buildings
	(const Widelands::Tribe_Descr & tribe_descr, const Widelands::Building_Index bi,
	 Widelands::Building::FormerBuildings* former_buildings)
{
	assert(former_buildings && former_buildings->empty());
	former_buildings->push_back(bi);

	for (;;) {
		Widelands::Building_Index oldest_idx = former_buildings->front();
		const Widelands::Building_Descr * oldest = tribe_descr.get_building_descr(oldest_idx);
		if (!oldest->is_enhanced()) {
			break;
		}
		for
			(Widelands::Building_Index i = Widelands::Building_Index::First();
			 i < tribe_descr.get_nrbuildings();
			 ++i)
		{
			const Widelands::Building_Descr* ob = tribe_descr.get_building_descr(i);
			if (ob->enhancements().count(oldest_idx)) {
				former_buildings->insert(former_buildings->begin(), i);
				break;
			}
		}
	}
}

Player::Player
	(Editor_Game_Base  & the_egbase,
	 Player_Number         const plnum,
	 uint8_t               const initialization_index,
	 const Tribe_Descr   &       tribe_descr,
	 const std::string   &       name)
	:
	m_egbase              (the_egbase),
	m_initialization_index(initialization_index),
	m_frontier_style_index(0),
	m_flag_style_index    (0),
	m_team_number(0),
	m_team_player_uptodate(false),
	m_see_all           (false),
	m_plnum             (plnum),
	m_tribe             (tribe_descr),
	m_casualties        (0),
	m_kills             (0),
	m_msites_lost        (0),
	m_msites_defeated    (0),
	m_civil_blds_lost    (0),
	m_civil_blds_defeated(0),
	m_allow_retreat_change(false),
	m_retreat_percentage  (50),
	m_fields            (nullptr),
	m_allowed_worker_types  (tribe_descr.get_nrworkers  (), true),
	m_allowed_building_types(tribe_descr.get_nrbuildings(), true),
	m_ai(""),
	m_current_produced_statistics(tribe_descr.get_nrwares    ()),
	m_current_consumed_statistics(tribe_descr.get_nrwares    ()),
	m_ware_productions  (tribe_descr.get_nrwares    ()),
	m_ware_consumptions  (tribe_descr.get_nrwares    ()),
	m_ware_stocks  (tribe_descr.get_nrwares          ())
{
	set_name(name);
}


Player::~Player() {
	delete[] m_fields;
}


void Player::create_default_infrastructure() {
	const Map & map = egbase().map();
	if (map.get_starting_pos(m_plnum)) {
		try {
			const Tribe_Descr::Initialization & initialization =
				tribe().initialization(m_initialization_index);

			Game & game = ref_cast<Game, Editor_Game_Base>(egbase());

			// Run the corresponding script
			std::unique_ptr<LuaCoroutine> cr =
			   game.lua().run_script(initialization.script)->get_coroutine("func");
			cr->push_arg(this);
			game.enqueue_command(new Cmd_LuaCoroutine(game.get_gametime(), cr.release()));

			// Check if other starting positions are shared in and initialize them as well
			for (uint8_t n = 0; n < m_further_shared_in_player.size(); ++n) {
				Coords const further_pos = map.get_starting_pos(m_further_shared_in_player.at(n));

				// Run the corresponding script
				std::unique_ptr<LuaCoroutine> ncr =
				   game.lua()
				      .run_script(tribe().initialization(m_further_initializations.at(n)).script)
				      ->get_coroutine("func");
				ncr->push_arg(this);
				ncr->push_arg(further_pos);
				game.enqueue_command(new Cmd_LuaCoroutine(game.get_gametime(), ncr.release()));
			}
		} catch (Tribe_Descr::Nonexistent &) {
			throw game_data_error
				("the selected initialization index (%u) is outside the range "
				 "(tribe edited between preload and game start?)",
				 m_initialization_index);
		}
	} else
		throw warning
			(_("Missing starting position"),
			 _
			 	("Widelands could not start the game, because player %u has no "
			 	 "starting position.\n"
			 	 "You can manually add a starting position with Widelands Editor, "
			 	 "to fix this problem."),
			 m_plnum);
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

/**
 * Assign the player the given team number.
 */
void Player::set_team_number(TeamNumber team)
{
	m_team_number = team;
	m_team_player_uptodate = false;
}

/**
 * Returns whether this player and the given other player can attack
 * each other.
 */
bool Player::is_hostile(const Player & other) const
{
	return
		&other != this &&
		(!m_team_number || m_team_number != other.m_team_number);
}

/**
 * Updates the vector containing allied players
 */
void Player::update_team_players() {
	m_team_player.clear();
	m_team_player_uptodate = true;

	if (!m_team_number)
		return;

	for (Player_Number i = 1; i <= MAX_PLAYERS; ++i) {
		Player * other = egbase().get_player(i);
		if (!other)
			continue;
		if (other == this)
			continue;
		if (m_team_number == other->m_team_number)
			m_team_player.push_back(other);
	}
}


/*
 * Plays the corresponding sound when a message is received and if sound is
 * enabled.
 */
void Player::play_message_sound(const std::string & sender) {
#define MAYBE_PLAY(a, b) if (sender == a) { \
	g_sound_handler.play_fx(b, 200, PRIO_ALWAYS_PLAY); \
	return; \
	}

	if (g_options.pull_section("global").get_bool("sound_at_message", true)) {
		MAYBE_PLAY("site_occupied", "sound/military/site_occupied");
		MAYBE_PLAY("under_attack", "sound/military/under_attack");

		g_sound_handler.play_fx("sound/message", 200, PRIO_ALWAYS_PLAY);
	}
}

Message_Id Player::add_message
	(Game & game, Message & message, bool const popup)
{
	// Expire command
	Message_Id id = messages().add_message(message);
	Duration const duration = message.duration();
	if (duration != Forever()) {
		game.cmdqueue().enqueue
			(new Cmd_ExpireMessage
			 	(game.get_gametime() + duration, player_number(), id));
	}

	// Map_Object connection
	if (message.serial() > 0) {
		Map_Object* mo = egbase().objects().get_object(message.serial());
		mo->removed.connect
		 (boost::bind(&Player::message_object_removed, this, id));
	}

	// Sound & popup
	if (Interactive_Player * const iplayer = game.get_ipl()) {
		if (&iplayer->player() == this) {
			play_message_sound(message.sender());
			if (popup)
				iplayer->popup_message(id, message);
		}
	}

	return id;
}


Message_Id Player::add_message_with_timeout
	(Game & game, Message & m, uint32_t const timeout, uint32_t const radius)
{
	const Map &       map      = game.map         ();
	uint32_t    const gametime = game.get_gametime();
	Coords      const position = m   .position    ();
	container_iterate_const(MessageQueue, messages(), i)
		if
			(i.current->second->sender() == m.sender()                      and
			 gametime < i.current->second->sent() + timeout                 and
			 map.calc_distance(i.current->second->position(), position) <= radius)
		{
			delete &m;
			return Message_Id::Null();
		}
	return add_message(game, m);
}

void Player::message_object_removed(Message_Id m_id) const
{
	// Send expire command
	upcast(Game, game, &m_egbase);
	if (!game) {
		return;
	}

	game->cmdqueue().enqueue
		(new Cmd_ExpireMessage
			(game->get_gametime(), m_plnum, m_id));
}



/*
===============
Return filtered buildcaps that take the player's territory into account.
===============
*/
NodeCaps Player::get_buildcaps(FCoords const fc) const {
	const Map & map = egbase().map();
	uint8_t buildcaps = fc.field->nodecaps();

	if (not fc.field->is_interior(m_plnum))
		buildcaps = 0;

	// Check if a building's flag can't be build due to ownership
	else if (buildcaps & BUILDCAPS_BUILDINGMASK) {
		FCoords flagcoords;
		map.get_brn(fc, &flagcoords);
		if (not flagcoords.field->is_interior(m_plnum))
			buildcaps &= ~BUILDCAPS_BUILDINGMASK;

		//  Prevent big buildings that would swell over borders.
		if
			((buildcaps & BUILDCAPS_BIG) == BUILDCAPS_BIG
			 and
			 (not map.tr_n(fc).field->is_interior(m_plnum)
			  or
			  not map.tl_n(fc).field->is_interior(m_plnum)
			  or
			  not map. l_n(fc).field->is_interior(m_plnum)))
			buildcaps &= ~BUILDCAPS_SMALL;
	}

	return static_cast<NodeCaps>(buildcaps);
}


/**
 * Build a flag, checking that it's legal to do so. Returns
 * the flag in case of success, else returns 0;
 */
Flag * Player::build_flag(Coords const c) {
	int32_t buildcaps = get_buildcaps(egbase().map().get_fcoords(c));

	if (buildcaps & BUILDCAPS_FLAG)
		return new Flag(egbase(), *this, c);
	return nullptr;
}


Flag & Player::force_flag(FCoords const c) {
	log("Forcing flag at (%i, %i)\n", c.x, c.y);
	const Map & map = egbase().map();
	if (BaseImmovable * const immovable = c.field->get_immovable()) {
		if (upcast(Flag, existing_flag, immovable)) {
			if (&existing_flag->owner() == this)
				return *existing_flag;
		} else if (not dynamic_cast<Road const *>(immovable)) //  A road is OK.
			immovable->remove(egbase()); //  Make room for the flag.
	}
	MapRegion<Area<FCoords> > mr(map, Area<FCoords>(c, 1));
	do if (upcast(Flag, flag, mr.location().field->get_immovable()))
		flag->remove(egbase()); //  Remove all flags that are too close.
	while (mr.advance(map));

	//  Make sure that the player owns the area around.
	egbase().conquer_area_no_building
		(Player_Area<Area<FCoords> >(player_number(), Area<FCoords>(c, 1)));
	return *new Flag(egbase(), *this, c);
}

/*
===============
Build a road along the given path.
Perform sanity checks (ownership, flags).

Note: the diagnostic log messages aren't exactly errors. They might happen
in some situations over the network.
===============
*/
Road * Player::build_road(const Path & path) {
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
							 player_number(), imm->get_type());
						return nullptr;
					}
				if (!(get_buildcaps(fc) & MOVECAPS_WALK)) {
					log("%i: building road, unwalkable\n", player_number());
					return nullptr;
				}
			}
			return &Road::create(egbase(), *start, *end, path);
		} else
			log("%i: building road, missed end flag\n", player_number());
	} else
		log("%i: building road, missed start flag\n", player_number());

	return nullptr;
}


Road & Player::force_road(const Path & path) {
	Map & map = egbase().map();
	FCoords c = map.get_fcoords(path.get_start());
	Flag & start = force_flag(c);
	Flag & end   = force_flag(map.get_fcoords(path.get_end()));

	Path::Step_Vector::size_type const laststep = path.get_nsteps() - 1;
	for (Path::Step_Vector::size_type i = 0; i < laststep; ++i) {
		c = map.get_neighbour(c, path[i]);
		log("Clearing for road at (%i, %i)\n", c.x, c.y);

		//  Make sure that the player owns the area around.
		ref_cast<Game, Editor_Game_Base>(egbase()).conquer_area_no_building
			(Player_Area<Area<FCoords> >(player_number(), Area<FCoords>(c, 1)));

		if (BaseImmovable * const immovable = c.field->get_immovable()) {
			assert(immovable != &start);
			assert(immovable != &end);
			immovable->remove(egbase());
		}
	}
	return Road::create(egbase(), start, end, path);
}

Building & Player::force_building
	(Coords                const location,
	 const Building_Descr::FormerBuildings & former_buildings)
{
	Map & map = egbase().map();
	Building_Index idx = former_buildings.back();
	const Building_Descr* descr = tribe().get_building_descr(idx);
	terraform_for_building(egbase(), player_number(), location, descr);
	FCoords flag_loc;
	map.get_brn(map.get_fcoords(location), &flag_loc);
	force_flag(flag_loc);

	return
		descr->create
			(egbase(), *this, map.get_fcoords(location), false, false, former_buildings);
}

Building& Player::force_csite
	(Coords const location, Building_Index b_idx,
	 const Building_Descr::FormerBuildings & former_buildings)
{
	Map & map = egbase().map();
	if (!former_buildings.empty()) {
		Building_Index idx = former_buildings.back();
		const Building_Descr * descr = tribe().get_building_descr(idx);
		terraform_for_building(egbase(), player_number(), location, descr);
	}
	FCoords flag_loc;
	map.get_brn(map.get_fcoords(location), &flag_loc);
	force_flag(flag_loc);

	return
		egbase().warp_constructionsite
			(map.get_fcoords(location), m_plnum, b_idx, false, former_buildings);
}



/*
===============
Place a construction site or building, checking that it's legal to do so.
===============
*/
Building * Player::build
	(Coords c, Building_Index const idx, bool constructionsite,
	 Building_Descr::FormerBuildings & former_buildings)
{
	int32_t buildcaps;

	// Validate building type
	if (not (idx and idx < tribe().get_nrbuildings()))
		return nullptr;
	const Building_Descr & descr = *tribe().get_building_descr(idx);

	if (!descr.is_buildable())
		return nullptr;


	// Validate build position
	const Map & map = egbase().map();
	map.normalize_coords(c);
	buildcaps = get_buildcaps(map.get_fcoords(c));

	if (descr.get_ismine()) {
		if (!(buildcaps & BUILDCAPS_MINE))
			return nullptr;
	} else {
		if ((buildcaps & BUILDCAPS_SIZEMASK) < descr.get_size() - BaseImmovable::SMALL + 1)
			return nullptr;
		if (descr.get_isport() && !(buildcaps & BUILDCAPS_PORT))
			return nullptr;
	}

	if (constructionsite)
		return &egbase().warp_constructionsite(c, m_plnum, idx, false, former_buildings);
	else {
		return &descr.create(egbase(), *this, c, false, false, former_buildings);
	}
}



/*
===============
Bulldoze the given road, flag or building.
===============
*/
void Player::bulldoze(PlayerImmovable & _imm, bool const recurse)
{
	std::vector<OPtr<PlayerImmovable> > bulldozelist;
	bulldozelist.push_back(&_imm);

	while (!bulldozelist.empty()) {
		PlayerImmovable * imm = bulldozelist.back().get(egbase());
		bulldozelist.pop_back();
		if (!imm)
			continue;

		// General security check
		if (imm->get_owner() != this)
			return;

		// Destroy, after extended security check
		if (upcast(Building, building, imm)) {
			if (!(building->get_playercaps() & Building::PCap_Bulldoze))
				return;

			Flag & flag = building->base_flag();
			building->destroy(egbase());
			//  Now imm and building are dangling reference/pointer! Do not use!

			if (recurse && flag.is_dead_end())
				bulldozelist.push_back(&flag);
		} else if (upcast(Flag, flag, imm)) {
			if (Building * const flagbuilding = flag->get_building())
				if (!(flagbuilding->get_playercaps() & Building::PCap_Bulldoze)) {
					log
						("Player trying to rip flag (%u) with undestroyable "
						 "building (%u)\n",
						 flag->serial(), flagbuilding->serial());
					return;
				}

			OPtr<Flag> flagcopy = flag;
			if (recurse) {
				for
					(uint8_t primary_road_id = 6; primary_road_id; --primary_road_id)
				{
					// Recursive bulldoze calls may cause flag to disappear
					if (!flagcopy.get(egbase()))
						return;

					if (Road * const primary_road = flag->get_road(primary_road_id))
					{
						Flag & primary_start =
							primary_road->get_flag(Road::FlagStart);
						Flag & primary_other =
							flag == &primary_start ?
							primary_road->get_flag(Road::FlagEnd) : primary_start;
						primary_road->destroy(egbase());
						log
							("destroying road from (%i, %i) going in dir %u\n",
							 flag->get_position().x, flag->get_position().y,
							 primary_road_id);
						//  The primary road is gone. Now see if the flag at the other
						//  end of it is a dead-end.
						if (primary_other.is_dead_end())
							bulldozelist.push_back(&primary_other);
					}
				}
			}

			// Recursive bulldoze calls may cause flag to disappear
			if (flagcopy.get(egbase()))
				flag->destroy(egbase());
		} else if (upcast(Road, road, imm)) {
			Flag & start = road->get_flag(Road::FlagStart);
			Flag & end = road->get_flag(Road::FlagEnd);

			road->destroy(egbase());
			//  Now imm and road are dangling reference/pointer! Do not use!

			if (recurse) {
				// Destroy all roads between the flags, not just selected
				while (Road * const r = start.get_road(end))
					r->destroy(egbase());

				OPtr<Flag> endcopy = &end;
				if (start.is_dead_end())
					bulldozelist.push_back(&start);
				// At this point, end may have become dangling
				if (Flag * pend = endcopy.get(egbase())) {
					if (pend->is_dead_end())
						bulldozelist.push_back(&end);
				}
			}
		} else
			throw wexception
				("Player::bulldoze(%u): bad immovable type", imm->serial());
	}
}


void Player::start_stop_building(PlayerImmovable & imm) {
	if (&imm.owner() == this)
		if (upcast(ProductionSite, productionsite, &imm))
			productionsite->set_stopped(!productionsite->is_stopped());
}

void Player::start_or_cancel_expedition(Warehouse & wh) {
	if (&wh.owner() == this)
		if (PortDock * pd = wh.get_portdock()) {
			if (pd->expedition_started()) {
				upcast(Game, game, &egbase());
				pd->cancel_expedition(*game);
			} else
				pd->start_expedition();
		}
}

void Player::military_site_set_soldier_preference(PlayerImmovable & imm, uint8_t m_soldier_preference)
{
	if (&imm.owner() == this)
		if (upcast(MilitarySite, milsite, &imm))
			milsite->set_soldier_preference(static_cast<MilitarySite::SoldierPreference>(m_soldier_preference));
}


/*
 * enhance this building, remove it, but give the constructionsite
 * an idea of enhancing
 */
void Player::enhance_building
	(Building * building, Building_Index const index_of_new_building)
{
	_enhance_or_dismantle(building, index_of_new_building);
}

/*
 * rip this building down, but slowly: a builder will take it gradually
 * apart.
 */
void Player::dismantle_building(Building * building) {
	_enhance_or_dismantle(building);
}
void Player::_enhance_or_dismantle
	(Building * building, Building_Index const index_of_new_building)
{
	if
		(&building->owner() == this
		 and
		 (!index_of_new_building or building->descr().enhancements().count(index_of_new_building)))
	{
		Building::FormerBuildings former_buildings = building->get_former_buildings();
		const Coords position = building->get_position();

		//  Get workers and soldiers
		//  Make copies of the vectors, because the originals are destroyed with
		//  the building.
		std::vector<Worker  *> workers;
		upcast(Warehouse, wh, building);
		if (wh) {
			workers = wh->get_incorporated_workers();
		} else {
			workers = building->get_workers();
		}

		building->remove(egbase()); //  no fire or stuff
		//  Hereafter the old building does not exist and building is a dangling
		//  pointer.
		if (index_of_new_building)
			building =
				&egbase().warp_constructionsite
					(position, m_plnum, index_of_new_building, false, former_buildings);
		else
			building =
				&egbase().warp_dismantlesite
					(position, m_plnum, false, former_buildings);
		//  Hereafter building points to the new building.

		// Reassign the workers and soldiers.
		// Note that this will make sure they stay within the economy;
		// However, they are no longer associated with the building as
		// workers of that buiding, which is why they will leave for a
		// warehouse.
		container_iterate_const(std::vector<Worker *>, workers, i)
			(*i.current)->set_location(building);
	}
}


/*
===============
Perform an action on the given flag.
===============
*/
void Player::flagaction(Flag & flag)
{
	if (&flag.owner() == this) //  Additional security check.
		flag.add_flag_job
			(ref_cast<Game, Editor_Game_Base>(egbase()),
			 tribe().worker_index("geologist"),
			 "expedition");
}


void Player::allow_worker_type(Ware_Index const i, bool const allow) {
	assert(i.value() < m_allowed_worker_types.size());
	assert(not allow or tribe().get_worker_descr(i)->is_buildable());
	m_allowed_worker_types[i] = allow;
}


/*
 * allow building
 *
 * Disable or enable a building for a player
 */
void Player::allow_building_type(Building_Index const i, bool const allow) {
	assert(i.value() < m_allowed_building_types.size());
	m_allowed_building_types[i] = allow;
}

/*
 * Economy stuff below
 */
void Player::add_economy(Economy & economy)
{
	if (not has_economy(economy))
		m_economies.push_back(&economy);
}


void Player::remove_economy(Economy & economy) {
	container_iterate(Economies, m_economies, i)
		if (*i.current == &economy) {
			m_economies.erase(i.current);
			return;
		}
}

bool Player::has_economy(Economy & economy) const {
	container_iterate_const(Economies, m_economies, i)
		if (*i.current == &economy)
			return true;
	return false;
}

Player::Economies::size_type Player::get_economy_number
	(Economy const * const economy) const
{
	Economies::const_iterator const
		economies_end = m_economies.end(), economies_begin = m_economies.begin();
	for
		(Economies::const_iterator it = economies_begin;
		 it != economies_end;
		 ++it)
		if (*it == economy)
			return it - economies_begin;
	assert(false); // never here
	return 0;
}

/************  Military stuff  **********/

/*
==========
Change the training priotity values
==========
*/
void Player::change_training_options
	(TrainingSite & trainingsite, int32_t const atr, int32_t const val)
{
	if (trainingsite.get_owner() == this) {
		tAttribute const attr = static_cast<tAttribute>(atr);
		trainingsite.set_pri(attr, trainingsite.get_pri(attr) + val);
	}
}

/*
===========
Forces the drop of given soldier at given house
===========
*/
void Player::drop_soldier(PlayerImmovable & imm, Soldier & soldier) {
	if (&imm.owner() != this)
		return;
	if (soldier.get_worker_type() != Worker_Descr::SOLDIER)
		return;
	if (upcast(SoldierControl, ctrl, &imm))
		ctrl->dropSoldier(soldier);
}

/*
===========
===========
*/
void Player::allow_retreat_change(bool allow) {
	m_allow_retreat_change = allow;
}

/**
 *   Added check limits of valid values. Percentage limits are configured at
 * tribe level
 * Automatically adjust value if out of limits.
 */
void Player::set_retreat_percentage(uint8_t percentage) {

	if (tribe().get_military_data().get_min_retreat() > percentage)
		m_retreat_percentage = tribe().get_military_data().get_min_retreat();
	else
	if (tribe().get_military_data().get_max_retreat() < percentage)
		m_retreat_percentage = tribe().get_military_data().get_max_retreat();
	else
		m_retreat_percentage = percentage;
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
uint32_t Player::findAttackSoldiers
	(Flag & flag, std::vector<Soldier *> * soldiers, uint32_t nr_wanted)
{
	uint32_t count = 0;

	if (soldiers)
		soldiers->clear();

	Map & map = egbase().map();
	std::vector<BaseImmovable *> flags;

	map.find_reachable_immovables_unique
		(Area<FCoords>(map.get_fcoords(flag.get_position()), 25),
		 flags,
		 CheckStepDefault(MOVECAPS_WALK),
		 FindFlagOf(FindImmovablePlayerMilitarySite(*this)));

	if (flags.empty())
		return 0;

	container_iterate_const(std::vector<BaseImmovable *>, flags, i) {
		const Flag * attackerflag = static_cast<Flag *>(*i.current);
		const MilitarySite * ms = static_cast<MilitarySite *>(attackerflag->get_building());
		std::vector<Soldier *> const present = ms->presentSoldiers();
		uint32_t const nr_staying = ms->minSoldierCapacity();
		uint32_t const nr_present = present.size();
		if (nr_staying < nr_present) {
			uint32_t const nr_taken =
				std::min(nr_wanted, nr_present - nr_staying);
			if (soldiers)
				soldiers->insert
					(soldiers->end(),
					 present.begin(), present.begin() + nr_taken);
			count     += nr_taken;
			nr_wanted -= nr_taken;
			if (not nr_wanted)
				break;
		}
	}

	return count;
}


/**
 * \todo Clean this mess up. The only action we really have right now is
 * to attack, so pretending we have more types is pointless.
 */
void Player::enemyflagaction
	(Flag & flag, Player_Number const attacker, uint32_t const count,
	 uint8_t retreat)
{
	if      (attacker != player_number())
		log
			("Player (%d) is not the sender of an attack (%d)\n",
			 attacker, player_number());
	else if (count == 0)
		log("enemyflagaction: count is 0\n");
	else if (is_hostile(flag.owner()))
		if (Building * const building = flag.get_building())
			if (upcast(Attackable, attackable, building))
				if (attackable->canAttack()) {
					std::vector<Soldier *> attackers;
					findAttackSoldiers(flag, &attackers, count);
					assert(attackers.size() <= count);

					retreat = std::max
						(retreat, tribe().get_military_data().get_min_retreat());
					retreat = std::min
						(retreat, tribe().get_military_data().get_max_retreat());

					container_iterate_const(std::vector<Soldier *>, attackers, i)
						ref_cast<MilitarySite, PlayerImmovable>
							(*(*i.current)->get_location(egbase()))
						.sendAttacker(**i.current, *building, retreat);
				}
}


void Player::rediscover_node
	(const Map              &       map,
	 const Widelands::Field &       first_map_field,
	 FCoords          const f)
{

	assert(0 <= f.x);
	assert(f.x < map.get_width());
	assert(0 <= f.y);
	assert(f.y < map.get_height());
	assert(&map[0] <= f.field);
	assert(f.field < &map[0] + map.max_index());

	Field & field = m_fields[f.field - &first_map_field];

	assert(m_fields <= &field);
	assert(&field < m_fields + map.max_index());

	{ // discover everything (above the ground) in this field
		field.terrains = f.field->get_terrains();
		field.roads    = f.field->get_roads   ();
		field.owner    = f.field->get_owned_by();

		// Check if this node is part of a border
		int32_t const mapwidth = map.get_width();
		// right neighbour
		FCoords        r = map.r_n(f);
		Player_Number  r_owner_number = r.field->get_owned_by();
		Map_Index      r_index        = map.get_index(r, mapwidth);
		Vision         r_vision       = vision(r_index);
		// top right neighbour
		FCoords       tr = map.tr_n(f);
		Player_Number tr_owner_number = tr.field->get_owned_by();
		//Map_Index     tr_index        = map.get_index(tr, mapwidth);
		//Vision        tr_vision       = vision(tr_index);
		// bottom right neighbour
		FCoords       br = map.br_n(f);
		Player_Number br_owner_number = br.field->get_owned_by();
		Map_Index     br_index        = map.get_index(br, mapwidth);
		Vision        br_vision       = vision(br_index);
		// bottom left neighbour
		FCoords       bl = map.bl_n(f);
		Player_Number bl_owner_number = bl.field->get_owned_by();
		Map_Index     bl_index        = map.get_index(bl, mapwidth);
		Vision        bl_vision       = vision(bl_index);
		// left neighbour
		FCoords        l = map.l_n(f);
		Player_Number  l_owner_number = l.field->get_owned_by();
		//Map_Index      l_index        = map.get_index(l, mapwidth);
		//Vision         l_vision       = vision(l_index);


		field.border    = f.field->is_border();
		field.border_r  =
			((1 |  r_vision) and (r_owner_number  == field.owner)
			and
			((tr_owner_number == field.owner) xor (br_owner_number == field.owner)));
		field.border_br =
			((1 | bl_vision) and (bl_owner_number == field.owner)
			and
			((l_owner_number  == field.owner) xor (br_owner_number == field.owner)));
		field.border_bl =
			((1 | br_vision) and (br_owner_number == field.owner)
			and
			((r_owner_number  == field.owner) xor (bl_owner_number == field.owner)));

		{ //  map_object_descr[TCoords::None]

			const Map_Object_Descr * map_object_descr;
			field.constructionsite.becomes = nullptr;
			if (const BaseImmovable * base_immovable = f.field->get_immovable()) {
				map_object_descr = &base_immovable->descr();

				if (Road::IsRoadDescr(map_object_descr))
					map_object_descr = nullptr;
				else if (upcast(Building const, building, base_immovable)) {
					if (building->get_position() != f)
						// This is not the building's main position so we can not see it.
						map_object_descr = nullptr;
					else {
						if (upcast(ConstructionSite const, cs, building)) {
							field.constructionsite = const_cast<ConstructionSite *>(cs)->get_info();
						}
					}
				}
			} else
				map_object_descr = nullptr;
			field.map_object_descr[TCoords<>::None] = map_object_descr;
		}
	}
	{ //  discover the D triangle and the SW edge of the top right neighbour
		FCoords tr = map.tr_n(f);
		Field & tr_field = m_fields[tr.field - &first_map_field];
		if (tr_field.vision <= 1) {
			tr_field.terrains.d = tr.field->terrain_d();
			tr_field.roads &= ~(Road_Mask << Road_SouthWest);
			tr_field.roads |= Road_Mask << Road_SouthWest & tr.field->get_roads();
		}
	}
	{ //  discover both triangles and the SE edge of the top left  neighbour
		FCoords tl = map.tl_n(f);
		Field & tl_field = m_fields[tl.field - &first_map_field];
		if (tl_field.vision <= 1) {
			tl_field.terrains = tl.field->get_terrains();
			tl_field.roads &= ~(Road_Mask << Road_SouthEast);
			tl_field.roads |= Road_Mask << Road_SouthEast & tl.field->get_roads();
		}
	}
	{ //  discover the R triangle and the  E edge of the     left  neighbour
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
	(const Map              &       map,
	 const Widelands::Field &       first_map_field,
	 FCoords                  const f,
	 Time                     const gametime,
	 bool                     const forward)
{
	assert(0 <= f.x);
	assert(f.x < map.get_width());
	assert(0 <= f.y);
	assert(f.y < map.get_height());
	assert(&map[0] <= f.field);
	assert           (f.field < &first_map_field + map.max_index());

	//  If this is not already a forwarded call, we should inform allied players
	//  as well of this change.
	if (!m_team_player_uptodate)
		update_team_players();
	if (!forward && !m_team_player.empty()) {
		for (uint8_t j = 0; j < m_team_player.size(); ++j)
			m_team_player[j]->see_node(map, first_map_field, f, gametime, true);
	}

	Field & field = m_fields[f.field - &first_map_field];
	assert(m_fields <= &field);
	assert            (&field < m_fields + map.max_index());
	Vision fvision = field.vision;
	if (fvision == 0)
		fvision = 1;
	if (fvision == 1)
		rediscover_node(map, first_map_field, f);
	++fvision;
	field.vision = fvision;
}

void Player::unsee_node
	(Map_Index const i, Time const gametime, bool const forward)
{
	Field & field = m_fields[i];
	if (field.vision <= 1) //  Already does not see this
		return;

	//  If this is not already a forwarded call, we should inform allied players
	//  as well of this change.
	if (!m_team_player_uptodate)
		update_team_players();
	if (!forward && !m_team_player.empty()) {
		for (uint8_t j = 0; j < m_team_player.size(); ++j)
			m_team_player[j]->unsee_node(i, gametime, true);
	}

	--field.vision;
	if (field.vision == 1)
		field.time_node_last_unseen = gametime;
	assert(1 <= field.vision);
}


/**
 * Called by Game::think to sample statistics data in regular intervals.
 */
void Player::sample_statistics()
{
	assert (m_ware_productions.size() == tribe().get_nrwares().value());
	assert (m_ware_consumptions.size() == tribe().get_nrwares().value());
	assert (m_ware_stocks.size() == tribe().get_nrwares().value());

	//calculate stocks
	std::vector<uint32_t> stocks(tribe().get_nrwares().value());

	const uint32_t nrecos = get_nr_economies();
	for (uint32_t i = 0; i < nrecos; ++i) {
		const std::vector<Widelands::Warehouse *> & warehouses =
			get_economy_by_number(i)->warehouses();

		for
			(std::vector<Widelands::Warehouse *>::const_iterator it =
			 warehouses.begin();
			 it != warehouses.end();
			 ++it)
		{
			const Widelands::WareList & wares = (*it)->get_wares();
			for (uint32_t id = 0; id < stocks.size(); ++id) {
				stocks[id] += wares.stock(Ware_Index(static_cast<size_t>(id)));
			}
		}
	}


	//update statistics
	for (uint32_t i = 0; i < m_ware_productions.size(); ++i) {
		m_ware_productions[i].push_back(m_current_produced_statistics[i]);
		m_current_produced_statistics[i] = 0;

		m_ware_consumptions[i].push_back(m_current_consumed_statistics[i]);
		m_current_consumed_statistics[i] = 0;

		m_ware_stocks[i].push_back(stocks[i]);
	}
}


/**
 * A ware was produced. Update the corresponding statistics.
 */
void Player::ware_produced(Ware_Index const wareid) {
	assert (m_ware_productions.size() == tribe().get_nrwares().value());
	assert(wareid.value() < tribe().get_nrwares().value());

	++m_current_produced_statistics[wareid];
}


/**
 * Some units from one kind of ware were consumed.
 * Update the corresponding statistics
 *
 * \param wareid the ID of the consumed wares
 * \param count the number of consumed wares
 */
void Player::ware_consumed(Ware_Index const wareid, uint8_t const count) {
	assert (m_ware_consumptions.size() == tribe().get_nrwares().value());
	assert(wareid.value() < tribe().get_nrwares().value());

	m_current_consumed_statistics[wareid] += count;
}


/**
 * Get current ware production statistics
 */
const std::vector<uint32_t> * Player::get_ware_production_statistics
		(Ware_Index const ware) const
{
	assert(ware.value() < m_ware_productions.size());

	return &m_ware_productions[ware];
}


/**
 * Get current ware consumption statistics
 */
const std::vector<uint32_t> * Player::get_ware_consumption_statistics
		(Ware_Index const ware) const {

	assert(ware.value() < m_ware_consumptions.size());

	return &m_ware_consumptions[ware];
}

const std::vector<uint32_t> * Player::get_ware_stock_statistics
		(Ware_Index const ware) const
{
	assert(ware.value() < m_ware_stocks.size());

	return &m_ware_stocks[ware];
}


/**
 * Add or remove the given building from building statistics.
 * Only to be called by \ref receive
 */
void Player::update_building_statistics
	(Building & building, losegain_t const lg)
{
	upcast(ConstructionSite const, constructionsite, &building);
	const std::string & building_name =
		constructionsite ?
		constructionsite->building().name() : building.name();

	Building_Index const nr_buildings = tribe().get_nrbuildings();

	// Get the valid vector for this
	if (m_building_stats.size() < nr_buildings.value())
		m_building_stats.resize(nr_buildings.value());

	std::vector<Building_Stats> & stat =
		m_building_stats[tribe().building_index(building_name.c_str())];

	if (lg == GAIN) {
		Building_Stats new_building;
		new_building.is_constructionsite = constructionsite;
		new_building.pos = building.get_position();
		stat.push_back(new_building);
	} else {
		Coords const building_position = building.get_position();
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


void Player::receive(const NoteImmovable & note)
{
	if (upcast(Building, building, note.pi))
		update_building_statistics(*building, note.lg);

	NoteSender<NoteImmovable>::send(note);
}


void Player::receive(const NoteFieldPossession & note)
{
	NoteSender<NoteFieldPossession>::send(note);
}

void Player::setAI(const std::string & ai)
{
	m_ai = ai;
}

const std::string & Player::getAI() const
{
	return m_ai;
}

/**
 * Read statistics data from a file.
 *
 * \param fr source stream
 * \param version indicates the kind of statistics file, which may be
 *   0 - old style statistics (before WiHack 2010)
 *   1 - statistics with ware names
 *   2 - with consumption statistics
 *   3 - with stock statistics
 */
void Player::ReadStatistics(FileRead & fr, uint32_t const version)
{
	 //version 1, 2 and 3 only differs in an additional statistic.
	 //Use version 1 code for all of them
	if ((version == 2) || (version == 1) || (version == 3)) {
		uint16_t nr_wares = fr.Unsigned16();
		uint16_t nr_entries = fr.Unsigned16();

		for (uint32_t i = 0; i < m_current_produced_statistics.size(); ++i)
			m_ware_productions[i].resize(nr_entries);

		for (uint16_t i = 0; i < nr_wares; ++i) {
			std::string name = fr.CString();
			Ware_Index idx = tribe().ware_index(name);
			if (!idx) {
				log
					("Player %u statistics: unknown ware name %s",
					 player_number(), name.c_str());
				continue;
			}

			m_current_produced_statistics[idx] = fr.Unsigned32();

			for (uint32_t j = 0; j < nr_entries; ++j)
				m_ware_productions[idx][j] = fr.Unsigned32();
		}

		//read consumption statistics if it exists
		if ((version == 2) || (version == 3)) {
			nr_wares = fr.Unsigned16();
			nr_entries = fr.Unsigned16();

			for (uint32_t i = 0; i < m_current_consumed_statistics.size(); ++i)
				m_ware_consumptions[i].resize(nr_entries);

			for (uint16_t i = 0; i < nr_wares; ++i) {
				std::string name = fr.CString();
				Ware_Index idx = tribe().ware_index(name);
				if (!idx) {
					log
						("Player %u consumption statistics: unknown ware name %s",
						player_number(), name.c_str());
					continue;
				}

				m_current_consumed_statistics[idx] = fr.Unsigned32();

				for (uint32_t j = 0; j < nr_entries; ++j)
					m_ware_consumptions[idx][j] = fr.Unsigned32();
			}

			//read stock statistics if it exists
			if (version == 3) {
				nr_wares = fr.Unsigned16();
				nr_entries = fr.Unsigned16();

				for (uint32_t i = 0; i < m_ware_stocks.size(); ++i)
					m_ware_stocks[i].resize(nr_entries);

				for (uint16_t i = 0; i < nr_wares; ++i) {
					std::string name = fr.CString();
					Ware_Index idx = tribe().ware_index(name);
					if (!idx) {
						log
							("Player %u stock statistics: unknown ware name %s",
							player_number(), name.c_str());
						continue;
					}

					for (uint32_t j = 0; j < nr_entries; ++j)
						m_ware_stocks[idx][j] = fr.Unsigned32();
				}
			}
		}
	} else if (version == 0) {
		uint16_t nr_wares = fr.Unsigned16();
		uint16_t nr_entries = fr.Unsigned16();

		if (nr_wares > 0) {
			if (nr_wares == tribe().get_nrwares().value()) {
				assert(m_ware_productions.size() == nr_wares);
				assert(m_current_produced_statistics.size() == nr_wares);

				for (uint32_t i = 0; i < m_current_produced_statistics.size(); ++i) {
					m_current_produced_statistics[i] = fr.Unsigned32();
					m_ware_productions[i].resize(nr_entries);

					for (uint32_t j = 0; j < m_ware_productions[i].size(); ++j)
						m_ware_productions[i][j] = fr.Unsigned32();
				}
			} else {
				log
					("Statistics for player %u (%s) has %u ware types "
					 "(should be %u). Statistics will be discarded.",
					 player_number(), tribe().name().c_str(),
					 nr_wares, tribe().get_nrwares().value());

				// Eat and discard all data
				for (uint32_t i = 0; i < nr_wares; ++i) {
					fr.Unsigned32();

					for (uint32_t j = 0; j < nr_entries; ++j)
						fr.Unsigned32();
				}
			}
		}
	} else
		throw wexception("Unsupported version %i", version);

	//create empty consumption statistic if it is missing
	if (version < 2) {
		uint16_t nr_entries = m_ware_productions[0].size();

		for (uint32_t i = 0; i < m_current_consumed_statistics.size(); ++i) {
			m_ware_consumptions[i].resize(nr_entries);
			m_current_consumed_statistics[i] = 0;

			for (uint32_t j = 0; j < nr_entries; ++j)
				m_ware_consumptions[i][j] = 0;
		}
	}

	//create empty stock statistic if it is missing
	if (version < 3) {
		uint16_t nr_entries = m_ware_productions[0].size();

		for (uint32_t i = 0; i < m_current_consumed_statistics.size(); ++i) {
			m_ware_stocks[i].resize(nr_entries);

			for (uint32_t j = 0; j < nr_entries; ++j)
				m_ware_stocks[i][j] = 0;
		}
	}

	//all statistics should have the same size
	assert(m_ware_productions.size() == m_ware_consumptions.size());
	assert(m_ware_productions[0].size() == m_ware_consumptions[0].size());

	assert(m_ware_productions.size() == m_ware_stocks.size());
	assert(m_ware_productions[0].size() == m_ware_stocks[0].size());
}


/**
 * Write statistics data to the give file
 */
void Player::WriteStatistics(FileWrite & fw) const {
	//write produce statistics
	fw.Unsigned16(m_current_produced_statistics.size());
	fw.Unsigned16(m_ware_productions[0].size());

	for (uint8_t i = 0; i < m_current_produced_statistics.size(); ++i) {
		fw.CString
			(tribe().get_ware_descr
			 (Ware_Index(static_cast<Ware_Index::value_t>(i)))->name());
		fw.Unsigned32(m_current_produced_statistics[i]);
		for (uint32_t j = 0; j < m_ware_productions[i].size(); ++j)
			fw.Unsigned32(m_ware_productions[i][j]);
	}

	//write consume statistics
	fw.Unsigned16(m_current_consumed_statistics.size());
	fw.Unsigned16(m_ware_consumptions[0].size());

	for (uint8_t i = 0; i < m_current_consumed_statistics.size(); ++i) {
		fw.CString
			(tribe().get_ware_descr
			 (Ware_Index(static_cast<Ware_Index::value_t>(i)))->name());
		fw.Unsigned32(m_current_consumed_statistics[i]);
		for (uint32_t j = 0; j < m_ware_consumptions[i].size(); ++j)
			fw.Unsigned32(m_ware_consumptions[i][j]);
	}

	//write stock statistics
	fw.Unsigned16(m_ware_stocks.size());
	fw.Unsigned16(m_ware_stocks[0].size());

	for (uint8_t i = 0; i < m_ware_stocks.size(); ++i) {
		fw.CString
			(tribe().get_ware_descr
			 (Ware_Index(static_cast<Ware_Index::value_t>(i)))->name());
		for (uint32_t j = 0; j < m_ware_stocks[i].size(); ++j)
			fw.Unsigned32(m_ware_stocks[i][j]);
	}
}

}
