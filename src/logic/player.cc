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

#include <memory>

#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_delete_message.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/constants.h"
#include "logic/findimmovable.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/playercommand.h"
#include "scripting/lua_table.h"
#include "sound/sound_handler.h"
#include "wui/interactive_player.h"


namespace {

void terraform_for_building
	(Widelands::EditorGameBase& egbase, const Widelands::PlayerNumber player_number,
	 const Widelands::Coords location, const Widelands::BuildingDescr* descr)
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
				(Widelands::PlayerArea<Widelands::Area<Widelands::FCoords> >
				 	(player_number, Widelands::Area<Widelands::FCoords>(c[i], 1)));

			if (Widelands::BaseImmovable * const immovable = c[i].field->get_immovable())
				immovable->remove(egbase);
		}
	}
}



}

namespace Widelands {

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
 * filled with the BuildingDescr.
 */
void find_former_buildings
	(const Tribes& tribes, const Widelands::DescriptionIndex bi,
	 Widelands::Building::FormerBuildings* former_buildings)
{
	assert(former_buildings && former_buildings->empty());
	former_buildings->push_back(bi);

	for (;;) {
		Widelands::DescriptionIndex oldest_idx = former_buildings->front();
		const Widelands::BuildingDescr * oldest = tribes.get_building_descr(oldest_idx);
		if (!oldest->is_enhanced()) {
			break;
		}
		for (DescriptionIndex i = 0; i < tribes.nrbuildings(); ++i) {
			const BuildingDescr* building_descr = tribes.get_building_descr(i);
			if (building_descr->enhancement() == oldest_idx) {
				former_buildings->insert(former_buildings->begin(), i);
				break;
			}
		}
	}
}

Player::Player
	(EditorGameBase  & the_egbase,
	 PlayerNumber         const plnum,
	 uint8_t               const initialization_index,
	 const TribeDescr   &       tribe_descr,
	 const std::string   &       name)
	:
	m_egbase              (the_egbase),
	m_initialization_index(initialization_index),
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
	m_fields            (nullptr),
	m_allowed_worker_types(the_egbase.tribes().nrworkers(), true),
	m_allowed_building_types(the_egbase.tribes().nrbuildings(), true),
	m_ai(""),
	m_current_produced_statistics(the_egbase.tribes().nrwares()),
	m_current_consumed_statistics(the_egbase.tribes().nrwares()),
	m_ware_productions(the_egbase.tribes().nrwares()),
	m_ware_consumptions(the_egbase.tribes().nrwares()),
	m_ware_stocks(the_egbase.tribes().nrwares())
{
	set_name(name);

	// Disallow workers that the player's tribe doesn't have.
	for (size_t worker_index = 0; worker_index < m_allowed_worker_types.size(); ++worker_index) {
		if (!tribe().has_worker(static_cast<DescriptionIndex>(worker_index))) {
			m_allowed_worker_types[worker_index] = false;
		}
	}

	// Disallow buildings that the player's tribe doesn't have and
	// that aren't militarysites that the tribe could conquer.
	for (size_t i = 0; i < m_allowed_building_types.size(); ++i) {
		const DescriptionIndex& building_index = static_cast<DescriptionIndex>(i);
		const BuildingDescr& descr = *tribe().get_building_descr(building_index);
		if (!tribe().has_building(building_index) && descr.type() != MapObjectType::MILITARYSITE) {
			m_allowed_building_types[i] = false;
		}
	}

	// Subscribe to NoteImmovables.
	immovable_subscriber_ =
		Notifications::subscribe<NoteImmovable>([this](const NoteImmovable& note) {
			if (note.pi->owner().player_number() == player_number()) {
				if (upcast(Building, building, note.pi))
					update_building_statistics(*building, note.ownership);
			}
		});

	// Subscribe to NoteFieldTerrainChanged.
	field_terrain_changed_subscriber_ =
		Notifications::subscribe<NoteFieldTerrainChanged>([this](const NoteFieldTerrainChanged& note) {
			if (vision(note.map_index) > 1) {
				rediscover_node(egbase().map(), egbase().map()[0], note.fc);
			}
		});

	//Populating remaining_shipnames vector
	for (auto shipname : *tribe_descr.get_ship_names()) {
		m_remaining_shipnames.insert(shipname);
	}

}


Player::~Player() {
	delete[] m_fields;
}


void Player::create_default_infrastructure() {
	const Map & map = egbase().map();
	if (map.get_starting_pos(m_plnum)) {
		const TribeBasicInfo::Initialization & initialization =
			tribe().initialization(m_initialization_index);

		Game & game = dynamic_cast<Game&>(egbase());

		// Run the corresponding script
		std::unique_ptr<LuaTable> table(game.lua().run_script(initialization.script));
		table->do_not_warn_about_unaccessed_keys();
		std::unique_ptr<LuaCoroutine> cr = table->get_coroutine("func");
		cr->push_arg(this);
		game.enqueue_command(new CmdLuaCoroutine(game.get_gametime(), cr.release()));

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
			game.enqueue_command(new CmdLuaCoroutine(game.get_gametime(), ncr.release()));
		}
	} else
		throw WLWarning
			(_("Missing starting position"),
			 _
				("Widelands could not start the game, because player %u has "
				 "no starting position.\n"
				 "You can manually add a starting position with the Widelands "
				 "Editor to fix this problem."),
			 static_cast<unsigned int>(m_plnum));
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

	for (PlayerNumber i = 1; i <= MAX_PLAYERS; ++i) {
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
void Player::play_message_sound(const Message::Type & msgtype) {
#define MAYBE_PLAY(type, file) if (msgtype == type) { \
	g_sound_handler.play_fx(file, 200, PRIO_ALWAYS_PLAY); \
	return; \
	}

	if (g_options.pull_section("global").get_bool("sound_at_message", true)) {
		MAYBE_PLAY(Message::Type::kEconomySiteOccupied, "sound/military/site_occupied");
		MAYBE_PLAY(Message::Type::kWarfareUnderAttack, "sound/military/under_attack");

		g_sound_handler.play_fx("sound/message", 200, PRIO_ALWAYS_PLAY);
	}
}

MessageId Player::add_message
	(Game & game, Message & message, bool const popup)
{
	MessageId id = messages().add_message(message);

	// MapObject connection
	if (message.serial() > 0) {
		MapObject* mo = egbase().objects().get_object(message.serial());
		mo->removed.connect
		 (boost::bind(&Player::message_object_removed, this, id));
	}

	// Sound & popup
	if (InteractivePlayer * const iplayer = game.get_ipl()) {
		if (&iplayer->player() == this) {
			play_message_sound(message.type());
			if (popup)
				iplayer->popup_message(id, message);
		}
	}

	return id;
}


MessageId Player::add_message_with_timeout
	(Game & game, Message & m, uint32_t const timeout, uint32_t const radius)
{
	const Map &       map      = game.map         ();
	uint32_t    const gametime = game.get_gametime();
	Coords      const position = m   .position    ();
	for (std::pair<MessageId, Message *>  tmp_message : messages()) {
		if
			(tmp_message.second->type() == m.type()      &&
			 gametime < tmp_message.second->sent() + timeout &&
			 map.calc_distance(tmp_message.second->position(), position) <= radius)
		{
			delete &m;
			return MessageId::null();
		}
	}
	return add_message(game, m);
}

void Player::message_object_removed(MessageId m_id) const
{
	// Send delete command
	upcast(Game, game, &m_egbase);
	if (!game) {
		return;
	}

	game->cmdqueue().enqueue
		(new CmdDeleteMessage
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

	if (!fc.field->is_interior(m_plnum))
		buildcaps = 0;

	// Check if a building's flag can't be build due to ownership
	else if (buildcaps & BUILDCAPS_BUILDINGMASK) {
		FCoords flagcoords;
		map.get_brn(fc, &flagcoords);
		if (!flagcoords.field->is_interior(m_plnum))
			buildcaps &= ~BUILDCAPS_BUILDINGMASK;

		//  Prevent big buildings that would swell over borders.
		if
			((buildcaps & BUILDCAPS_BIG) == BUILDCAPS_BIG
			 &&
			 (!map.tr_n(fc).field->is_interior(m_plnum)
			  ||
			  !map.tl_n(fc).field->is_interior(m_plnum)
			  ||
			  !map. l_n(fc).field->is_interior(m_plnum)))
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
		} else if (!dynamic_cast<Road const *>(immovable)) //  A road is OK.
			immovable->remove(egbase()); //  Make room for the flag.
	}
	MapRegion<Area<FCoords> > mr(map, Area<FCoords>(c, 1));
	do if (upcast(Flag, flag, mr.location().field->get_immovable()))
		flag->remove(egbase()); //  Remove all flags that are too close.
	while (mr.advance(map));

	//  Make sure that the player owns the area around.
	egbase().conquer_area_no_building
		(PlayerArea<Area<FCoords> >(player_number(), Area<FCoords>(c, 1)));
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

	Path::StepVector::size_type const laststep = path.get_nsteps() - 1;
	for (Path::StepVector::size_type i = 0; i < laststep; ++i) {
		c = map.get_neighbour(c, path[i]);
		log("Clearing for road at (%i, %i)\n", c.x, c.y);

		//  Make sure that the player owns the area around.
		dynamic_cast<Game&>(egbase()).conquer_area_no_building
			(PlayerArea<Area<FCoords> >(player_number(), Area<FCoords>(c, 1)));

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
	 const BuildingDescr::FormerBuildings & former_buildings)
{
	Map & map = egbase().map();
	DescriptionIndex idx = former_buildings.back();
	const BuildingDescr* descr = egbase().tribes().get_building_descr(idx);
	terraform_for_building(egbase(), player_number(), location, descr);
	FCoords flag_loc;
	map.get_brn(map.get_fcoords(location), &flag_loc);
	force_flag(flag_loc);

	return
		descr->create
			(egbase(), *this, map.get_fcoords(location), false, false, former_buildings);
}

Building& Player::force_csite
	(Coords const location, DescriptionIndex b_idx,
	 const BuildingDescr::FormerBuildings & former_buildings)
{
	Map & map = egbase().map();
	if (!former_buildings.empty()) {
		DescriptionIndex idx = former_buildings.back();
		const BuildingDescr * descr = egbase().tribes().get_building_descr(idx);
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
	(Coords c, DescriptionIndex const idx, bool constructionsite,
	 BuildingDescr::FormerBuildings & former_buildings)
{
	int32_t buildcaps;

	// Validate building type
	if (!tribe().has_building(idx)) {
		return nullptr;
	}

	const BuildingDescr* descr = egbase().tribes().get_building_descr(idx);

	if (!descr->is_buildable()) {
		return nullptr;
	}

	// Validate build position
	const Map & map = egbase().map();
	map.normalize_coords(c);
	buildcaps = get_buildcaps(map.get_fcoords(c));

	if (descr->get_ismine()) {
		if (!(buildcaps & BUILDCAPS_MINE))
			return nullptr;
	} else {
		if ((buildcaps & BUILDCAPS_SIZEMASK) < descr->get_size() - BaseImmovable::SMALL + 1)
			return nullptr;
		if (descr->get_isport() && !(buildcaps & BUILDCAPS_PORT))
			return nullptr;
	}

	if (constructionsite)
		return &egbase().warp_constructionsite(c, m_plnum, idx, false, former_buildings);
	else {
		return &descr->create(egbase(), *this, c, false, false, former_buildings);
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
	(Building * building, DescriptionIndex const index_of_new_building)
{
	_enhance_or_dismantle(building, index_of_new_building);
}

/*
 * rip this building down, but slowly: a builder will take it gradually
 * apart.
 */
void Player::dismantle_building(Building * building) {
	_enhance_or_dismantle(building, INVALID_INDEX);
}
void Player::_enhance_or_dismantle
	(Building * building, DescriptionIndex const index_of_new_building)
{
	if (&building->owner() ==
	    this && (index_of_new_building == INVALID_INDEX ||
			building->descr().enhancement() == index_of_new_building)) {
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
		if (index_of_new_building != INVALID_INDEX)
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
		for (Worker * temp_worker : workers) {
			temp_worker->set_location(building);
		}
	}
}


/*
===============
Perform an action on the given flag.
===============
*/
void Player::flagaction(Flag & flag)
{
	if (&flag.owner() == this) { //  Additional security check.
		flag.add_flag_job(dynamic_cast<Game&>(egbase()), tribe().geologist(), "expedition");
	}
}


void Player::allow_worker_type(DescriptionIndex const i, bool const allow) {
	assert(i < static_cast<int>(m_allowed_worker_types.size()));
	assert(!allow || tribe().get_worker_descr(i)->is_buildable());
	m_allowed_worker_types[i] = allow;
}


/*
 * allow building
 *
 * Disable or enable a building for a player
 */
void Player::allow_building_type(DescriptionIndex const i, bool const allow) {
	assert(i < m_allowed_building_types.size());
	m_allowed_building_types[i] = allow;
}

/*
 * Economy stuff below
 */
void Player::add_economy(Economy & economy)
{
	if (!has_economy(economy))
		m_economies.push_back(&economy);
}


void Player::remove_economy(Economy & economy) {
	for (std::vector<Economy *>::iterator economy_iter = m_economies.begin();
		 economy_iter != m_economies.end(); ++economy_iter)
		if (*economy_iter == &economy) {
			m_economies.erase(economy_iter);
			return;
		}
}

bool Player::has_economy(Economy & economy) const {
	for (Economy * temp_economy : m_economies) {
		if (temp_economy == &economy) {
			return true;
		}
	}
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
		TrainingAttribute const attr = static_cast<TrainingAttribute>(atr);
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
	if (soldier.descr().type() != MapObjectType::SOLDIER)
		return;
	if (upcast(SoldierControl, ctrl, &imm))
		ctrl->drop_soldier(soldier);
}

/*
===========
===========
*/

/**
 * Get a list of soldiers that this player can use to attack the
 * building at the given flag.
 *
 * The default attack should just take the first N soldiers of the
 * returned array.
 */
// TODO(unknown): Perform a meaningful sort on the soldiers array.
uint32_t Player::find_attack_soldiers
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

	for (BaseImmovable * temp_flag : flags) {
		upcast(Flag, attackerflag, temp_flag);
		upcast(MilitarySite, ms, attackerflag->get_building());
		std::vector<Soldier *> const present = ms->present_soldiers();
		uint32_t const nr_staying = ms->min_soldier_capacity();
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
			if (!nr_wanted)
				break;
		}
	}

	return count;
}


// TODO(unknown): Clean this mess up. The only action we really have right now is
// to attack, so pretending we have more types is pointless.
void Player::enemyflagaction
	(Flag & flag, PlayerNumber const attacker, uint32_t const count)
{
	if      (attacker != player_number())
		log
			("Player (%d) is not the sender of an attack (%d)\n",
			 attacker, player_number());
	else if (count == 0)
		log("enemyflagaction: count is 0\n");
	else if (is_hostile(flag.owner())) {
		if (Building * const building = flag.get_building()) {
			if (upcast(Attackable, attackable, building)) {
				if (attackable->can_attack()) {
					std::vector<Soldier *> attackers;
					find_attack_soldiers(flag, &attackers, count);
					assert(attackers.size() <= count);

					for (Soldier * temp_attacker : attackers) {
						upcast(MilitarySite, ms, temp_attacker->get_location(egbase()));
						ms->send_attacker(*temp_attacker, *building);
					}
				}
			}
		}
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
		PlayerNumber  r_owner_number = r.field->get_owned_by();
		MapIndex      r_index        = map.get_index(r, mapwidth);
		Vision         r_vision       = vision(r_index);
		// top right neighbour
		FCoords       tr = map.tr_n(f);
		PlayerNumber tr_owner_number = tr.field->get_owned_by();
		//MapIndex     tr_index        = map.get_index(tr, mapwidth);
		//Vision        tr_vision       = vision(tr_index);
		// bottom right neighbour
		FCoords       br = map.br_n(f);
		PlayerNumber br_owner_number = br.field->get_owned_by();
		MapIndex     br_index        = map.get_index(br, mapwidth);
		Vision        br_vision       = vision(br_index);
		// bottom left neighbour
		FCoords       bl = map.bl_n(f);
		PlayerNumber bl_owner_number = bl.field->get_owned_by();
		MapIndex     bl_index        = map.get_index(bl, mapwidth);
		Vision        bl_vision       = vision(bl_index);
		// left neighbour
		FCoords        l = map.l_n(f);
		PlayerNumber  l_owner_number = l.field->get_owned_by();
		//MapIndex      l_index        = map.get_index(l, mapwidth);
		//Vision         l_vision       = vision(l_index);


		field.border    = f.field->is_border();
		field.border_r  =
			((1 |  r_vision) && (r_owner_number  == field.owner)
			&&
			((tr_owner_number == field.owner) ^ (br_owner_number == field.owner)));
		field.border_br =
			((1 | bl_vision) && (bl_owner_number == field.owner)
			&&
			((l_owner_number  == field.owner) ^ (br_owner_number == field.owner)));
		field.border_bl =
			((1 | br_vision) && (br_owner_number == field.owner)
			&&
			((r_owner_number  == field.owner) ^ (bl_owner_number == field.owner)));

		{ //  map_object_descr[TCoords::None]

			const MapObjectDescr * map_object_descr;
			field.constructionsite.becomes = nullptr;
			if (const BaseImmovable * base_immovable = f.field->get_immovable()) {
				map_object_descr = &base_immovable->descr();

				if (Road::is_road_descr(map_object_descr))
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
			tr_field.roads &= ~(RoadType::kMask << RoadType::kSouthWest);
			tr_field.roads |= RoadType::kMask << RoadType::kSouthWest & tr.field->get_roads();
		}
	}
	{ //  discover both triangles and the SE edge of the top left  neighbour
		FCoords tl = map.tl_n(f);
		Field & tl_field = m_fields[tl.field - &first_map_field];
		if (tl_field.vision <= 1) {
			tl_field.terrains = tl.field->get_terrains();
			tl_field.roads &= ~(RoadType::kMask << RoadType::kSouthEast);
			tl_field.roads |= RoadType::kMask << RoadType::kSouthEast & tl.field->get_roads();
		}
	}
	{ //  discover the R triangle and the  E edge of the     left  neighbour
		FCoords l = map.l_n(f);
		Field & l_field = m_fields[l.field - &first_map_field];
		if (l_field.vision <= 1) {
			l_field.terrains.r = l.field->terrain_r();
			l_field.roads &= ~(RoadType::kMask << RoadType::kEast);
			l_field.roads |= RoadType::kMask << RoadType::kEast & l.field->get_roads();
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
	(MapIndex const i, Time const gametime, bool const forward)
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
	assert (m_ware_productions.size() == egbase().tribes().nrwares());
	assert (m_ware_consumptions.size() == egbase().tribes().nrwares());
	assert (m_ware_stocks.size() == egbase().tribes().nrwares());

	//calculate stocks
	std::vector<uint32_t> stocks(egbase().tribes().nrwares());

	const uint32_t nrecos = get_nr_economies();
	for (uint32_t i = 0; i < nrecos; ++i) {
		const std::vector<Widelands::Warehouse *> & warehouses =
			get_economy_by_number(i)->warehouses();

		for (Widelands::Warehouse * warehouse : warehouses) {
			const Widelands::WareList& wares = warehouse->get_wares();
			for (size_t id = 0; id < stocks.size(); ++id) {
				stocks[id] += wares.stock(DescriptionIndex(id));
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
void Player::ware_produced(DescriptionIndex const wareid) {
	assert (m_ware_productions.size() == egbase().tribes().nrwares());
	assert(egbase().tribes().ware_exists(wareid));

	++m_current_produced_statistics[wareid];
}


/**
 * Some units from one kind of ware were consumed.
 * Update the corresponding statistics
 *
 * \param wareid the ID of the consumed wares
 * \param count the number of consumed wares
 */
void Player::ware_consumed(DescriptionIndex const wareid, uint8_t const count) {
	assert (m_ware_consumptions.size() == egbase().tribes().nrwares());
	assert(egbase().tribes().ware_exists(wareid));

	m_current_consumed_statistics[wareid] += count;
}


/**
 * Get current ware production statistics
 */
const std::vector<uint32_t> * Player::get_ware_production_statistics
		(DescriptionIndex const ware) const
{
	assert(ware < static_cast<int>(m_ware_productions.size()));
	return &m_ware_productions[ware];
}


/**
 * Get current ware consumption statistics
 */
const std::vector<uint32_t> * Player::get_ware_consumption_statistics
		(DescriptionIndex const ware) const {

	assert(ware < static_cast<int>(m_ware_consumptions.size()));

	return &m_ware_consumptions[ware];
}

const std::vector<uint32_t> * Player::get_ware_stock_statistics
		(DescriptionIndex const ware) const
{
	assert(ware < static_cast<int>(m_ware_stocks.size()));

	return &m_ware_stocks[ware];
}

const Player::BuildingStatsVector& Player::get_building_statistics(const DescriptionIndex& i) const {
	return *const_cast<Player*>(this)->get_mutable_building_statistics(i);
}

Player::BuildingStatsVector* Player::get_mutable_building_statistics(const DescriptionIndex& i) {
	DescriptionIndex const nr_buildings = egbase().tribes().nrbuildings();
	if (m_building_stats.size() < nr_buildings)
		m_building_stats.resize(nr_buildings);
	return &m_building_stats[i];
}

/**
 * Add or remove the given building from building statistics.
 * Only to be called by \ref receive
 */
void Player::update_building_statistics
	(Building & building, NoteImmovable::Ownership ownership)
{
	upcast(ConstructionSite const, constructionsite, &building);
	const std::string & building_name =
		constructionsite ?
		constructionsite->building().name() : building.descr().name();

	const size_t nr_buildings = egbase().tribes().nrbuildings();

	// Get the valid vector for this
	if (m_building_stats.size() < nr_buildings)
		m_building_stats.resize(nr_buildings);

	std::vector<BuildingStats>& stat =
		*get_mutable_building_statistics(egbase().tribes().building_index(building_name.c_str()));

	if (ownership == NoteImmovable::Ownership::GAINED) {
		BuildingStats new_building;
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
			("InteractivePlayer::loose_immovable(): A building should be "
			 "removed at (%i, %i), but nothing is known about this building!",
			 building_position.x, building_position.y);
	}
}
/**
 * Functions used by AI to save/read AI data stored in Player class.
 */

void Player::set_ai(const std::string & ai) {
	m_ai = ai;
}

const std::string & Player::get_ai() const {
	return m_ai;
}

/**
 * Pick random name from remaining names (if any)
 */
const std::string Player::pick_shipname() {
	if (!m_remaining_shipnames.empty()) {
		Game & game = dynamic_cast<Game&>(egbase());
		assert (is_a(Game, &egbase()));
		const uint32_t index = game.logic_rand() % m_remaining_shipnames.size();
		std::unordered_set<std::string>::iterator it = m_remaining_shipnames.begin();
		std::advance(it, index);
		std::string new_name = *it;
		m_remaining_shipnames.erase(it);
		return new_name;
	}
	return "Ship";
}

/**
 * Read remaining ship indexes to the give file
 *
 * \param fr source stream
 */
void Player::read_remaining_shipnames(FileRead & fr) {
	const uint16_t count = fr.unsigned_16();
	for (uint16_t i = 0; i < count; ++i) {
		m_remaining_shipnames.insert(fr.string());
	}
}

/**
 * Read statistics data from a file.
 *
 * \param fr source stream
 */
void Player::read_statistics(FileRead & fr)
{
	uint16_t nr_wares = fr.unsigned_16();
	uint16_t nr_entries = fr.unsigned_16();

	for (uint32_t i = 0; i < m_current_produced_statistics.size(); ++i)
		m_ware_productions[i].resize(nr_entries);

	for (uint16_t i = 0; i < nr_wares; ++i) {
		std::string name = fr.c_string();
		DescriptionIndex idx = egbase().tribes().ware_index(name);
		if (!egbase().tribes().ware_exists(idx)) {
			log
				("Player %u statistics: unknown ware name %s",
				 player_number(), name.c_str());
			continue;
		}

		m_current_produced_statistics[idx] = fr.unsigned_32();

		for (uint32_t j = 0; j < nr_entries; ++j)
			m_ware_productions[idx][j] = fr.unsigned_32();
	}

	//read consumption statistics
	nr_wares = fr.unsigned_16();
	nr_entries = fr.unsigned_16();

	for (uint32_t i = 0; i < m_current_consumed_statistics.size(); ++i)
		m_ware_consumptions[i].resize(nr_entries);

	for (uint16_t i = 0; i < nr_wares; ++i) {
		std::string name = fr.c_string();
		DescriptionIndex idx = egbase().tribes().ware_index(name);
		if (!egbase().tribes().ware_exists(idx)) {
			log
				("Player %u consumption statistics: unknown ware name %s",
				player_number(), name.c_str());
			continue;
		}

		m_current_consumed_statistics[idx] = fr.unsigned_32();

		for (uint32_t j = 0; j < nr_entries; ++j)
			m_ware_consumptions[idx][j] = fr.unsigned_32();
	}

	//read stock statistics
	nr_wares = fr.unsigned_16();
	nr_entries = fr.unsigned_16();

	for (uint32_t i = 0; i < m_ware_stocks.size(); ++i)
		m_ware_stocks[i].resize(nr_entries);

	for (uint16_t i = 0; i < nr_wares; ++i) {
		std::string name = fr.c_string();
		DescriptionIndex idx = egbase().tribes().ware_index(name);
		if (!egbase().tribes().ware_exists(idx)) {
			log
				("Player %u stock statistics: unknown ware name %s",
				player_number(), name.c_str());
			continue;
		}

		for (uint32_t j = 0; j < nr_entries; ++j)
			m_ware_stocks[idx][j] = fr.unsigned_32();
	}

	//all statistics should have the same size
	assert(m_ware_productions.size() == m_ware_consumptions.size());
	assert(m_ware_productions[0].size() == m_ware_consumptions[0].size());

	assert(m_ware_productions.size() == m_ware_stocks.size());
	assert(m_ware_productions[0].size() == m_ware_stocks[0].size());
}

/**
 * Write remaining ship indexes to the give file
 */
void Player::write_remaining_shipnames(FileWrite & fw) const {
	fw.unsigned_16(m_remaining_shipnames.size());
	for (auto shipname : m_remaining_shipnames){
		fw.string(shipname);
	}
}

/**
 * Write statistics data to the give file
 */
void Player::write_statistics(FileWrite & fw) const {
	//write produce statistics
	fw.unsigned_16(m_current_produced_statistics.size());
	fw.unsigned_16(m_ware_productions[0].size());

	for (uint8_t i = 0; i < m_current_produced_statistics.size(); ++i) {
		fw.c_string
			(egbase().tribes().get_ware_descr(i)->name());
		fw.unsigned_32(m_current_produced_statistics[i]);
		for (uint32_t j = 0; j < m_ware_productions[i].size(); ++j)
			fw.unsigned_32(m_ware_productions[i][j]);
	}

	//write consume statistics
	fw.unsigned_16(m_current_consumed_statistics.size());
	fw.unsigned_16(m_ware_consumptions[0].size());

	for (uint8_t i = 0; i < m_current_consumed_statistics.size(); ++i) {
		fw.c_string
			(egbase().tribes().get_ware_descr(i)->name());
		fw.unsigned_32(m_current_consumed_statistics[i]);
		for (uint32_t j = 0; j < m_ware_consumptions[i].size(); ++j)
			fw.unsigned_32(m_ware_consumptions[i][j]);
	}

	//write stock statistics
	fw.unsigned_16(m_ware_stocks.size());
	fw.unsigned_16(m_ware_stocks[0].size());

	for (uint8_t i = 0; i < m_ware_stocks.size(); ++i) {
		fw.c_string(egbase().tribes().get_ware_descr(i)->name());
		for (uint32_t j = 0; j < m_ware_stocks[i].size(); ++j)
			fw.unsigned_32(m_ware_stocks[i][j]);
	}
}

}
