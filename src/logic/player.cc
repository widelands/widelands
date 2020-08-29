/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include <cassert>
#include <memory>

#include <boost/algorithm/string.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/warning.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/expedition_bootstrap.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/cmd_delete_message.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/mapregion.h"
#include "logic/playercommand.h"
#include "scripting/lua_table.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"
#include "wui/interactive_player.h"

namespace {

void terraform_for_building(Widelands::EditorGameBase& egbase,
                            const Widelands::PlayerNumber player_number,
                            const Widelands::Coords& location,
                            const Widelands::BuildingDescr* descr) {
	const Widelands::Map& map = egbase.map();
	Widelands::FCoords c[4];  //  Big buildings occupy 4 locations.
	c[0] = map.get_fcoords(location);
	map.get_brn(c[0], &c[1]);
	if (Widelands::BaseImmovable* const immovable = c[0].field->get_immovable()) {
		immovable->remove(egbase);
	}

	size_t nr_locations = 1;
	if ((descr->get_size() & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG) {
		nr_locations = 4;
		map.get_trn(c[0], &c[1]);
		map.get_tln(c[0], &c[2]);
		map.get_ln(c[0], &c[3]);
	}
	for (size_t i = 0; i < nr_locations; ++i) {
		//  Make sure that the player owns the area around.
		egbase.conquer_area_no_building(Widelands::PlayerArea<Widelands::Area<Widelands::FCoords>>(
		   player_number, Widelands::Area<Widelands::FCoords>(c[i], 1)));

		if (Widelands::BaseImmovable* const immovable = c[i].field->get_immovable()) {
			immovable->remove(egbase);
		}
	}
}
}  // namespace

namespace Widelands {

Player::Field::PartiallyFinishedBuildingDetails::PartiallyFinishedBuildingDetails() {
	memset(this, 0, sizeof(Player::Field::PartiallyFinishedBuildingDetails));
}

/**
 * Find the longest possible enhancement chain leading to the given
 * building descr. The FormerBuildings given in reference must be empty and will be
 * filled with the BuildingDescr.
 */
void find_former_buildings(const Tribes& tribes,
                           const Widelands::DescriptionIndex bi,
                           Widelands::FormerBuildings* former_buildings) {
	assert(former_buildings && former_buildings->empty());
	former_buildings->push_back(std::make_pair(bi, ""));

	for (;;) {
		Widelands::DescriptionIndex oldest_idx = former_buildings->front().first;
		const Widelands::BuildingDescr* oldest = tribes.get_building_descr(oldest_idx);
		if (!oldest->is_enhanced()) {
			break;
		}
		for (DescriptionIndex i = 0; i < tribes.nrbuildings(); ++i) {
			const BuildingDescr* building_descr = tribes.get_building_descr(i);
			if (building_descr->enhancement() == oldest_idx) {
				former_buildings->insert(former_buildings->begin(), std::make_pair(i, ""));
				break;
			}
		}
	}
}

Player::Player(EditorGameBase& the_egbase,
               PlayerNumber const plnum,
               uint8_t const initialization_index,
               const TribeDescr& tribe_descr,
               const std::string& name)
   : egbase_(the_egbase),
     initialization_index_(initialization_index),
     team_number_(0),
     see_all_(false),
     player_number_(plnum),
     tribe_(tribe_descr),
     casualties_(0),
     kills_(0),
     msites_lost_(0),
     msites_defeated_(0),
     civil_blds_lost_(0),
     civil_blds_defeated_(0),
     ship_name_counter_(0),
     fields_(nullptr),
     is_picking_custom_starting_position_(false),
     message_fx_(SoundHandler::register_fx(SoundType::kMessage, "sound/message")),
     attack_fx_(SoundHandler::register_fx(SoundType::kMessage, "sound/military/under_attack")),
     occupied_fx_(SoundHandler::register_fx(SoundType::kMessage, "sound/military/site_occupied")) {
	set_name(name);

	init_statistics();

	// Allow workers that the player's tribe has.
	for (DescriptionIndex worker_index : tribe().workers()) {
		allow_worker_type(worker_index, true);
	}

	// Allow buildings that the player's tribe has
	for (DescriptionIndex building_index : tribe().buildings()) {
		allow_building_type(building_index, true);
	}

	// Subscribe to NoteImmovables.
	immovable_subscriber_ =
	   Notifications::subscribe<NoteImmovable>([this](const NoteImmovable& note) {
		   if (note.pi->owner().player_number() == player_number()) {
			   if (upcast(Building, building, note.pi)) {
				   update_building_statistics(*building, note.ownership);
			   }
		   }
	   });

	// Subscribe to NoteFieldTerrainChanged.
	field_terrain_changed_subscriber_ = Notifications::subscribe<NoteFieldTerrainChanged>(
	   [this](const NoteFieldTerrainChanged& note) {
		   if (is_seeing(note.map_index)) {
			   rediscover_node(egbase().map(), note.fc);
		   }
	   });

	// Populating remaining_shipnames vector
	for (const auto& shipname : tribe_descr.get_ship_names()) {
		remaining_shipnames_.push_back(shipname);
	}

	update_team_players();
}

Player::~Player() {
}

void Player::create_default_infrastructure() {
	if (is_picking_custom_starting_position_) {
		return;
	}
	const Map& map = egbase().map();
	if (map.get_starting_pos(player_number_)) {
		const Widelands::TribeBasicInfo::Initialization& initialization =
		   tribe().initialization(initialization_index_);

		Game& game = dynamic_cast<Game&>(egbase());

		// Run the corresponding script
		std::unique_ptr<LuaTable> table(game.lua().run_script(initialization.script));
		table->do_not_warn_about_unaccessed_keys();
		std::unique_ptr<LuaCoroutine> cr = table->get_coroutine("func");
		cr->push_arg(this);
		game.enqueue_command(new CmdLuaCoroutine(game.get_gametime(), std::move(cr)));

		// Check if other starting positions are shared in and initialize them as well
		for (uint8_t n = 0; n < further_shared_in_player_.size(); ++n) {
			Coords const further_pos = map.get_starting_pos(further_shared_in_player_.at(n));

			// Run the corresponding script
			std::unique_ptr<LuaCoroutine> ncr =
			   game.lua()
			      .run_script(tribe().initialization(further_initializations_.at(n)).script)
			      ->get_coroutine("func");
			ncr->push_arg(this);
			ncr->push_arg(further_pos);
			game.enqueue_command(new CmdLuaCoroutine(game.get_gametime(), std::move(ncr)));
		}
	} else {
		throw WLWarning(_("Missing starting position"),
		                _("Widelands could not start the game, because player %u has "
		                  "no starting position.\n"
		                  "You can manually add a starting position with the Widelands "
		                  "Editor to fix this problem."),
		                static_cast<unsigned int>(player_number_));
	}
}

/**
 * Allocate the fields array that contains player-specific field information.
 */
void Player::allocate_map() {
	const Map& map = egbase().map();
	assert(map.get_width());
	assert(map.get_height());
	fields_.reset(new Field[map.max_index()]);
}

bool Player::pick_custom_starting_position(const Coords& c) {
	assert(is_picking_custom_starting_position_);
	if (!get_starting_position_suitability(c)) {
		return false;
	}
	dynamic_cast<Game&>(egbase()).send_player_command(
	   new CmdPickCustomStartingPosition(egbase().get_gametime(), player_number(), c));
	return true;
}

void Player::do_pick_custom_starting_position(const Coords& c) {
	if (!is_picking_custom_starting_position_) {
		return;
	}
	is_picking_custom_starting_position_ = false;
	egbase().mutable_map()->set_starting_pos(player_number(), c);
	create_default_infrastructure();
}

bool Player::get_starting_position_suitability(const Coords& c) const {
	const Map& map = egbase().map();
	const FCoords f = map.get_fcoords(c);

	if (f.field->get_owned_by() != 0 || (f.field->nodecaps() & BUILDCAPS_BIG) != BUILDCAPS_BIG) {
		return false;
	}

	const Widelands::Field& neighbour = map[map.br_n(c)];
	if (neighbour.get_owned_by() != 0 || !(neighbour.nodecaps() & BUILDCAPS_FLAG)) {
		return false;
	}

	bool is_starting_position = false;
	for (unsigned p = map.get_nrplayers(); p; --p) {
		if (map.get_starting_pos(p) == c) {
			is_starting_position = true;
			break;
		}
	}
	if (!is_starting_position) {
		// Check that it is not surrounded by rocks etc.
		// We assume that the default starting positions are fine.
		unsigned obstacles_nearby = 0;
		MapRegion<Area<FCoords>> mr(map, Area<FCoords>(f, 4));
		do {
			if (mr.location().field->get_owned_by()) {
				return false;
			}
			if (!(mr.location().field->nodecaps() & BUILDCAPS_FLAG)) {
				++obstacles_nearby;
				if (obstacles_nearby > 3) {
					return false;
				}
			}
		} while (mr.advance(map));
	}

	// Check for enemy players nearby
	MapRegion<Area<FCoords>> mr(map, Area<FCoords>(f, kMinSpaceAroundPlayers));
	do {
		if (mr.location().field->get_owned_by()) {
			return false;
		}
	} while (mr.advance(map));
	return true;
}

/**
 * Assign the player the given team number.
 */
void Player::set_team_number(TeamNumber team) {
	team_number_ = team;
	update_team_players();
}

/**
 * Returns whether this player and the given other player can attack
 * each other.
 */
bool Player::is_hostile(const Player& other) const {
	return &other != this && (!team_number_ || team_number_ != other.team_number_) &&
	       !is_attack_forbidden(other.player_number());
}

bool Player::is_defeated() const {
	for (const auto& economy : economies()) {
		if (!economy.second->warehouses().empty()) {
			return false;
		}
	}
	return true;
}

void Player::AiPersistentState::initialize() {
	colony_scan_area = kColonyScanStartArea;
	trees_around_cutters = 0;
	last_attacked_player = std::numeric_limits<int16_t>::max();
	expedition_start_time = kNoExpedition;
	ships_utilization = 200;
	no_more_expeditions = false;
	target_military_score = 100;
	least_military_score = 0;
	ai_productionsites_ratio = std::rand() % 5 + 7;  // NOLINT
	ai_personality_mil_upper_limit = 100;

	// all zeroes
	assert(neuron_weights.size() == Widelands::Player::AiPersistentState::kNeuronPoolSize);
	assert(neuron_functs.size() == Widelands::Player::AiPersistentState::kNeuronPoolSize);
	assert(f_neurons.size() == Widelands::Player::AiPersistentState::kFNeuronPoolSize);
	assert(magic_numbers.size() == Widelands::Player::AiPersistentState::kMagicNumbersSize);
	for (size_t i = 0; i < neuron_weights.size(); ++i) {
		neuron_weights.at(i) = 0;
	}
	for (size_t i = 0; i < neuron_functs.size(); ++i) {
		neuron_functs.at(i) = 0;
	}
	for (size_t i = 0; i < f_neurons.size(); ++i) {
		f_neurons.at(i) = 0;
	}
	for (size_t i = 0; i < magic_numbers.size(); ++i) {
		magic_numbers.at(i) = 0;
	}
	remaining_basic_buildings.clear();

	initialized = true;
}

/**
 * Updates the vector containing allied players
 */
void Player::update_team_players() {
	// Tell all our old allies to forget us…
	for (const PlayerNumber& p : team_player_) {
		if (p != player_number()) {
			Player& player = *egbase().get_player(p);
			player.team_player_.erase(player_number());
			player.update_vision_whole_map();
		}
	}

	team_player_ = {player_number()};

	// …and tell our new allies to note us
	if (team_number_) {
		for (PlayerNumber p = 1; p <= kMaxPlayers; ++p) {
			if (p != player_number()) {
				if (Player* player = egbase().get_player(p)) {
					if (player->team_number_ == team_number_) {
						team_player_.insert(p);
						player->team_player_.insert(player_number());
						player->update_vision_whole_map();
					}
				}
			}
		}
	}

	update_vision_whole_map();
}

/*
 * Plays the corresponding sound when a message is received and if sound is
 * enabled.
 */
void Player::play_message_sound(const Message* message) {
	if (g_sh->is_sound_enabled(SoundType::kMessage)) {
		FxId fx;
		switch (message->type()) {
		case Message::Type::kEconomySiteOccupied:
			fx = occupied_fx_;
			break;
		case Message::Type::kWarfareUnderAttack:
			fx = attack_fx_;
			break;
		default:
			fx = message_fx_;
		}
		Notifications::publish(
		   NoteSound(SoundType::kMessage, fx, message->position(), kFxMaximumPriority, true));
	}
}

MessageId Player::add_message(Game& game, std::unique_ptr<Message> new_message, bool const popup) {
	MessageId id = get_messages()->add_message(std::move(new_message));
	const Message* message = messages()[id];

	// MapObject connection
	if (message->serial() > 0) {
		MapObject* mo = egbase().objects().get_object(message->serial());
		mo->removed.connect([this, id](unsigned) { message_object_removed(id); });
	}

	// Sound & popup
	if (InteractivePlayer* const iplayer = game.get_ipl()) {
		if (&iplayer->player() == this) {
			play_message_sound(message);
			if (popup) {
				iplayer->popup_message(id, *message);
			}
		}
	}

	return id;
}

MessageId Player::add_message_with_timeout(Game& game,
                                           std::unique_ptr<Message> message,
                                           uint32_t const timeout,
                                           uint32_t const radius) {
	const Map& map = game.map();
	uint32_t const gametime = game.get_gametime();
	Coords const position = message->position();
	for (const auto& tmp_message : messages()) {
		if (tmp_message.second->type() == message->type() &&
		    tmp_message.second->sub_type() == message->sub_type() &&
		    gametime < tmp_message.second->sent() + timeout &&
		    map.calc_distance(tmp_message.second->position(), position) <= radius) {
			return MessageId::null();
		}
	}
	return add_message(game, std::move(message));
}

void Player::message_object_removed(MessageId message_id) const {
	// Send delete command
	upcast(Game, game, &egbase_);
	if (!game) {
		return;
	}

	game->cmdqueue().enqueue(new CmdDeleteMessage(game->get_gametime(), player_number_, message_id));
}

const std::set<Serial>& Player::ships() const {
	return ships_;
}
void Player::add_ship(Serial ship) {
	ships_.insert(ship);
}
void Player::remove_ship(Serial ship) {
	auto it = ships_.find(ship);
	if (it != ships_.end()) {
		ships_.erase(it);
	}
}

/*
===============
Return filtered buildcaps that take the player's territory into account.
===============
*/
NodeCaps Player::get_buildcaps(const FCoords& fc) const {
	const Map& map = egbase().map();
	uint8_t buildcaps = fc.field->nodecaps();

	if (!fc.field->is_interior(player_number_)) {
		buildcaps = 0;
	} else if (buildcaps & BUILDCAPS_BUILDINGMASK) {  // Check if a building's flag can't be build
		                                               // due to ownership
		FCoords flagcoords;
		map.get_brn(fc, &flagcoords);
		if (!flagcoords.field->is_interior(player_number_)) {
			buildcaps &= ~BUILDCAPS_BUILDINGMASK;
		}

		//  Prevent big buildings that would swell over borders.
		if ((buildcaps & BUILDCAPS_BIG) == BUILDCAPS_BIG &&
		    (!map.tr_n(fc).field->is_interior(player_number_) ||
		     !map.tl_n(fc).field->is_interior(player_number_) ||
		     !map.l_n(fc).field->is_interior(player_number_))) {
			buildcaps &= ~BUILDCAPS_SMALL;
		}
	}

	return static_cast<NodeCaps>(buildcaps);
}

/**
 * Build a flag, checking that it's legal to do so. Returns
 * the flag in case of success, else returns 0;
 */
Flag* Player::build_flag(const Coords& c) {
	int32_t buildcaps = get_buildcaps(egbase().map().get_fcoords(c));

	if (buildcaps & BUILDCAPS_FLAG) {
		return new Flag(egbase(), this, c);
	}
	return nullptr;
}

Flag& Player::force_flag(const FCoords& c) {
	log_info_time(egbase().get_gametime(), "Forcing flag at (%i, %i)\n", c.x, c.y);
	const Map& map = egbase().map();
	if (BaseImmovable* const immovable = c.field->get_immovable()) {
		if (upcast(Flag, existing_flag, immovable)) {
			if (existing_flag->get_owner() == this) {
				return *existing_flag;
			}
		} else if (!dynamic_cast<RoadBase const*>(immovable)) {  //  A road or waterway is OK
			immovable->remove(egbase());                          //  Make room for the flag.
		}
	}
	MapRegion<Area<FCoords>> mr(map, Area<FCoords>(c, 1));
	do {
		if (upcast(Flag, flag, mr.location().field->get_immovable())) {
			flag->remove(egbase());  //  Remove all flags that are too close.
		}
	} while (mr.advance(map));

	//  Make sure that the player owns the area around.
	egbase().conquer_area_no_building(
	   PlayerArea<Area<FCoords>>(player_number(), Area<FCoords>(c, 1)));
	return *new Flag(egbase(), this, c);
}

/*
===============
Build a road along the given path.
Perform sanity checks (ownership, flags).

Note: the diagnostic log messages aren't exactly errors. They might happen
in some situations over the network.
===============
*/
Road* Player::build_road(const Path& path) {
	const Map& map = egbase().map();
	FCoords fc = map.get_fcoords(path.get_start());
	if (upcast(Flag, start, fc.field->get_immovable())) {
		if (upcast(Flag, end, map.get_immovable(path.get_end()))) {

			//  Verify ownership of the path.
			const int32_t laststep = path.get_nsteps() - 1;
			for (int32_t i = 0; i < laststep; ++i) {
				fc = map.get_neighbour(fc, path[i]);

				if (BaseImmovable* const imm = fc.field->get_immovable()) {
					if (imm->get_size() >= BaseImmovable::SMALL) {
						return nullptr;
					}
				}
				if (!(get_buildcaps(fc) & MOVECAPS_WALK)) {
					log_warn_time(
					   egbase().get_gametime(), "%i: building road, unwalkable\n", player_number());
					return nullptr;
				}
			}
			return &Road::create(egbase(), *start, *end, path);
		} else {
			log_warn_time(
			   egbase().get_gametime(), "%i: building road, missed end flag\n", player_number());
		}
	} else {
		log_warn_time(
		   egbase().get_gametime(), "%i: building road, missed start flag\n", player_number());
	}

	return nullptr;
}

Road& Player::force_road(const Path& path) {
	const Map& map = egbase().map();
	FCoords c = map.get_fcoords(path.get_start());
	Flag& start = force_flag(c);
	Flag& end = force_flag(map.get_fcoords(path.get_end()));

	Path::StepVector::size_type const laststep = path.get_nsteps() - 1;
	for (Path::StepVector::size_type i = 0; i < laststep; ++i) {
		c = map.get_neighbour(c, path[i]);
		log_info_time(egbase().get_gametime(), "Clearing for road at (%i, %i)\n", c.x, c.y);

		//  Make sure that the player owns the area around.
		dynamic_cast<Game&>(egbase()).conquer_area_no_building(
		   PlayerArea<Area<FCoords>>(player_number(), Area<FCoords>(c, 1)));

		if (BaseImmovable* const immovable = c.field->get_immovable()) {
			assert(immovable != &start);
			assert(immovable != &end);
			immovable->remove(egbase());
		}
	}
	return Road::create(egbase(), start, end, path);
}

Waterway* Player::build_waterway(const Path& path) {
	const Map& map = egbase().map();

	if (path.get_nsteps() > map.get_waterway_max_length()) {
		log_warn_time(
		   egbase().get_gametime(),
		   "%d: Refused to build a waterway because it is too long. Permitted length %d, actual "
		   "length %" PRIuS ".",
		   static_cast<unsigned int>(player_number()), map.get_waterway_max_length(),
		   path.get_nsteps());
		return nullptr;
	}

	FCoords fc = map.get_fcoords(path.get_start());
	if (upcast(Flag, start, fc.field->get_immovable())) {
		if (upcast(Flag, end, map.get_immovable(path.get_end()))) {
			//  Verify ownership of the path.
			const int32_t laststep = path.get_nsteps() - 1;
			for (int32_t i = 0; i < laststep; ++i) {
				fc = map.get_neighbour(fc, path[i]);

				if (BaseImmovable* const imm = fc.field->get_immovable()) {
					if (imm->get_size() >= BaseImmovable::SMALL) {
						return nullptr;
					}
				}
				if (!CheckStepFerry(egbase()).reachable_dest(map, fc)) {
					log_warn_time(egbase().get_gametime(),
					              "%i: building waterway aborted, unreachable for ferries\n",
					              static_cast<unsigned int>(player_number()));
					return nullptr;
				}
			}
			return &Waterway::create(egbase(), *start, *end, path);
		} else {
			log_warn_time(egbase().get_gametime(), "%i: building waterway aborted, missing end flag\n",
			              static_cast<unsigned int>(player_number()));
		}
	} else {
		log_warn_time(egbase().get_gametime(), "%i: building waterway aborted, missing start flag\n",
		              static_cast<unsigned int>(player_number()));
	}
	return nullptr;
}

Waterway& Player::force_waterway(const Path& path) {
	const Map& map = egbase().map();
	FCoords c = map.get_fcoords(path.get_start());
	Flag& start = force_flag(c);
	Flag& end = force_flag(map.get_fcoords(path.get_end()));

	Path::StepVector::size_type const laststep = path.get_nsteps() - 1;
	for (Path::StepVector::size_type i = 0; i < laststep; ++i) {
		c = map.get_neighbour(c, path[i]);
		log_info_time(egbase().get_gametime(), "Clearing for waterway at (%i, %i)\n", c.x, c.y);

		//  Make sure that the player owns the area around.
		dynamic_cast<Game&>(egbase()).conquer_area_no_building(
		   PlayerArea<Area<FCoords>>(player_number(), Area<FCoords>(c, 1)));

		if (BaseImmovable* const immovable = c.field->get_immovable()) {
			assert(immovable != &start);
			assert(immovable != &end);
			immovable->remove(egbase());
		}
	}
	return Waterway::create(egbase(), start, end, path);
}

Building& Player::force_building(Coords const location, const FormerBuildings& former_buildings) {
	const Map& map = egbase().map();
	DescriptionIndex idx = former_buildings.back().first;
	const BuildingDescr* descr = egbase().tribes().get_building_descr(idx);
	terraform_for_building(egbase(), player_number(), location, descr);
	FCoords flag_loc;
	map.get_brn(map.get_fcoords(location), &flag_loc);
	force_flag(flag_loc);

	return descr->create(egbase(), this, map.get_fcoords(location), false, false, former_buildings);
}

Building& Player::force_csite(Coords const location,
                              DescriptionIndex b_idx,
                              const FormerBuildings& former_buildings) {
	EditorGameBase& eg = egbase();
	const Map& map = eg.map();
	const Tribes& tribes = eg.tribes();
	const PlayerNumber pn = player_number();

	if (!former_buildings.empty()) {
		DescriptionIndex idx = former_buildings.back().first;
		const BuildingDescr* descr = tribes.get_building_descr(idx);
		terraform_for_building(eg, pn, location, descr);
	}
	FCoords flag_loc;
	map.get_brn(map.get_fcoords(location), &flag_loc);
	force_flag(flag_loc);

	terraform_for_building(eg, pn, location, tribes.get_building_descr(b_idx));

	return eg.warp_constructionsite(
	   map.get_fcoords(location), player_number_, b_idx, false, former_buildings);
}

/*
===============
Place a construction site or building, checking that it's legal to do so.
===============
*/
Building* Player::build(Coords c,
                        DescriptionIndex const idx,
                        bool constructionsite,
                        FormerBuildings& former_buildings) {
	// Validate building type
	if (!tribe().has_building(idx)) {
		return nullptr;
	}

	const BuildingDescr* descr = egbase().tribes().get_building_descr(idx);

	if (!descr->is_buildable()) {
		return nullptr;
	}

	// Validate build position
	const Map& map = egbase().map();
	map.normalize_coords(c);
	const FCoords fc = map.get_fcoords(c);
	const FCoords brn = map.br_n(fc);
	if (!fc.field->is_interior(player_number()) || !brn.field->is_interior(player_number())) {
		return nullptr;
	}
	if (descr->get_size() >= BaseImmovable::BIG &&
	    !((map.l_n(fc).field->is_interior(player_number())) &&
	      (map.tr_n(fc).field->is_interior(player_number())) &&
	      (map.tl_n(fc).field->is_interior(player_number())))) {
		return nullptr;
	}

	if (descr->get_built_over_immovable() != INVALID_INDEX &&
	    !(fc.field->get_immovable() &&
	      fc.field->get_immovable()->has_attribute(descr->get_built_over_immovable()))) {
		return nullptr;
	}

	const NodeCaps buildcaps = descr->get_built_over_immovable() == INVALID_INDEX ?
	                              get_buildcaps(fc) :
	                              map.get_max_nodecaps(egbase(), fc);
	if (descr->get_ismine()) {
		if (!(buildcaps & BUILDCAPS_MINE)) {
			return nullptr;
		}
	} else {
		if ((buildcaps & BUILDCAPS_SIZEMASK) < descr->get_size() - BaseImmovable::SMALL + 1) {
			return nullptr;
		}
		if (descr->get_isport() && !(buildcaps & BUILDCAPS_PORT)) {
			return nullptr;
		}
	}
	if (!(brn.field->get_immovable() &&
	      brn.field->get_immovable()->descr().type() == MapObjectType::FLAG) &&
	    !(get_buildcaps(brn) & BUILDCAPS_FLAG)) {
		return nullptr;
	}

	if (constructionsite) {
		return &egbase().warp_constructionsite(c, player_number_, idx, false, former_buildings);
	} else {
		return &descr->create(egbase(), this, c, false, false, former_buildings);
	}
}

/*
===============
Bulldoze the given road, waterway, flag or building.
===============
*/
void Player::bulldoze(PlayerImmovable& imm, bool const recurse) {
	std::vector<OPtr<PlayerImmovable>> bulldozelist;
	bulldozelist.push_back(&imm);

	while (!bulldozelist.empty()) {
		PlayerImmovable* immovable = bulldozelist.back().get(egbase());
		bulldozelist.pop_back();
		if (!immovable) {
			continue;
		}

		// General security check
		if (immovable->get_owner() != this) {
			return;
		}

		// Destroy, after extended security check
		if (upcast(Building, building, immovable)) {
			if (!(building->get_playercaps() & Building::PCap_Bulldoze)) {
				return;
			}

			Flag& flag = building->base_flag();
			building->destroy(egbase());
			//  Now imm and building are dangling reference/pointer! Do not use!

			if (recurse && flag.is_dead_end()) {
				bulldozelist.push_back(&flag);
			}
		} else if (upcast(Flag, flag, immovable)) {
			if (Building* const flagbuilding = flag->get_building()) {
				if (!(flagbuilding->get_playercaps() & Building::PCap_Bulldoze)) {
					log_warn_time(egbase().get_gametime(),
					              "Player trying to rip flag (%u) with undestroyable "
					              "building (%u)\n",
					              flag->serial(), flagbuilding->serial());
					return;
				}
			}

			OPtr<Flag> flagcopy = flag;
			if (recurse) {
				for (uint8_t primary_road_id = 6; primary_road_id; --primary_road_id) {
					// Recursive bulldoze calls may cause flag to disappear
					if (!flagcopy.get(egbase())) {
						return;
					}

					if (RoadBase* const primary_road = flag->get_roadbase(primary_road_id)) {
						Flag& primary_start = primary_road->get_flag(RoadBase::FlagStart);
						Flag& primary_other = flag == &primary_start ?
						                         primary_road->get_flag(RoadBase::FlagEnd) :
						                         primary_start;
						primary_road->destroy(egbase());
						log_info_time(egbase().get_gametime(),
						              "destroying road/waterway from (%i, %i) going in dir %u\n",
						              flag->get_position().x, flag->get_position().y, primary_road_id);
						//  The primary road is gone. Now see if the flag at the other
						//  end of it is a dead-end.
						if (primary_other.is_dead_end()) {
							bulldozelist.push_back(&primary_other);
						}
					}
				}
			}

			// Recursive bulldoze calls may cause flag to disappear
			if (flagcopy.get(egbase())) {
				flag->destroy(egbase());
			}
		} else if (upcast(RoadBase, road, immovable)) {
			Flag& start = road->get_flag(RoadBase::FlagStart);
			Flag& end = road->get_flag(RoadBase::FlagEnd);

			road->destroy(egbase());
			//  Now imm and road are dangling reference/pointer! Do not use!

			if (recurse) {
				// Destroy all roads and waterways between the flags, not just selected
				while (RoadBase* const r = start.get_roadbase(end)) {
					r->destroy(egbase());
				}

				OPtr<Flag> endcopy = &end;
				if (start.is_dead_end()) {
					bulldozelist.push_back(&start);
				}
				// At this point, end may have become dangling
				if (Flag* pend = endcopy.get(egbase())) {
					if (pend->is_dead_end()) {
						bulldozelist.push_back(&end);
					}
				}
			}
		} else {
			throw wexception("Player::bulldoze(%u): bad immovable type", immovable->serial());
		}
	}
}

void Player::start_stop_building(PlayerImmovable& imm) {
	if (imm.get_owner() == this) {
		if (upcast(ProductionSite, productionsite, &imm)) {
			productionsite->set_stopped(!productionsite->is_stopped());
		}
	}
}

void Player::start_or_cancel_expedition(Warehouse& wh) {
	if (wh.get_owner() == this) {
		if (PortDock* pd = wh.get_portdock()) {
			if (pd->expedition_started()) {
				upcast(Game, game, &egbase());
				pd->cancel_expedition(*game);
			} else {
				pd->start_expedition();
			}
		}
	}
}

void Player::military_site_set_soldier_preference(PlayerImmovable& imm,
                                                  SoldierPreference soldier_preference) {
	if (imm.get_owner() == this) {
		if (upcast(MilitarySite, milsite, &imm)) {
			milsite->set_soldier_preference(soldier_preference);
		}
	}
}

/*
 * enhance this building, remove it, but give the constructionsite
 * an idea of enhancing
 */
void Player::enhance_building(Building* building,
                              DescriptionIndex const index_of_new_building,
                              bool keep_wares) {
	enhance_or_dismantle(building, index_of_new_building, keep_wares);
}

/*
 * rip this building down, but slowly: a builder will take it gradually
 * apart.
 */
void Player::dismantle_building(Building* building, bool keep_wares) {
	enhance_or_dismantle(building, INVALID_INDEX, keep_wares);
}
void Player::enhance_or_dismantle(Building* building,
                                  DescriptionIndex const index_of_new_building,
                                  bool keep_wares) {
	if (building->get_owner() == this &&
	    (index_of_new_building == INVALID_INDEX ||
	     building->descr().enhancement() == index_of_new_building)) {
		FormerBuildings former_buildings = building->get_former_buildings();
		const Coords position = building->get_position();

		//  Get wares, workers, and soldiers
		//  Make copies of the vectors, because the originals are destroyed with
		//  the building.
		std::vector<Worker*> workers;
		std::map<DescriptionIndex, Quantity> wares;
		auto add_to_wares = [&wares](DescriptionIndex di, Quantity add) {
			if (add == 0) {
				return;
			}
			auto it = wares.find(di);
			if (it == wares.end()) {
				wares[di] = add;
			} else {
				it->second += add;
			}
		};

		if (upcast(Warehouse, wh, building)) {
			workers = wh->get_incorporated_workers();
			if (keep_wares) {
				for (DescriptionIndex di = wh->get_wares().get_nrwareids(); di; --di) {
					wares[di - 1] = wh->get_wares().stock(di - 1);
				}
				if (PortDock* pd = wh->get_portdock()) {
					for (DescriptionIndex di : tribe().wares()) {
						add_to_wares(di, pd->count_waiting(wwWARE, di));
					}
					if (ExpeditionBootstrap* x = pd->expedition_bootstrap()) {
						for (const InputQueue* q : x->queues(true)) {
							if (q->get_type() == wwWARE) {
								add_to_wares(q->get_index(), q->get_filled());
							}
						}
					}
				}
			}
		} else {
			workers = building->get_workers();
			if (keep_wares) {
				// TODO(Nordfriese): Add support for markets?
				if (upcast(ProductionSite, ps, building)) {
					for (const InputQueue* q : ps->inputqueues()) {
						if (q->get_type() == wwWARE) {
							auto it = wares.find(q->get_index());
							if (it == wares.end()) {
								wares[q->get_index()] = q->get_filled();
							} else {
								it->second += q->get_filled();
							}
						}
					}
				}
			}
		}
		assert(keep_wares || wares.empty());

		const BuildingSettings* settings = nullptr;
		if (index_of_new_building != INVALID_INDEX) {
			settings = building->create_building_settings();
			// For enhancing, register whether the window was open
			Notifications::publish(NoteBuilding(building->serial(), NoteBuilding::Action::kStartWarp));
		}

		building->remove(egbase());  //  no fire or stuff
		//  Hereafter the old building does not exist and building is a dangling
		//  pointer.

		if (index_of_new_building != INVALID_INDEX) {
			building = &egbase().warp_constructionsite(position, player_number_, index_of_new_building,
			                                           false, former_buildings, settings, wares);
		} else {
			building =
			   &egbase().warp_dismantlesite(position, player_number_, false, former_buildings, wares);
		}

		// Open the new building window if needed
		Notifications::publish(NoteBuilding(building->serial(), NoteBuilding::Action::kFinishWarp));

		//  Hereafter building points to the new building.

		// Reassign the workers and soldiers.
		// Note that this will make sure they stay within the economy;
		// However, they are no longer associated with the building as
		// workers of that buiding, which is why they will leave for a
		// warehouse.
		for (Worker* temp_worker : workers) {
			temp_worker->set_location(building);
		}
	}
}

/*
===============
Perform an action on the given flag.
===============
*/
void Player::flagaction(Flag& flag) {
	if (flag.get_owner() == this) {  //  Additional security check.
		flag.add_flag_job(dynamic_cast<Game&>(egbase()), tribe().geologist(), "expedition");
	}
}

bool Player::is_worker_type_allowed(const DescriptionIndex& i) const {
	return allowed_worker_types_.count(i) == 1;
}

void Player::allow_worker_type(DescriptionIndex const i, bool const allow) {
	if (allow) {
		allowed_worker_types_.insert(i);
	} else {
		auto denyme = allowed_worker_types_.find(i);
		if (denyme != allowed_worker_types_.end()) {
			allowed_worker_types_.erase(denyme);
		}
	}
}

bool Player::is_building_type_allowed(const DescriptionIndex& i) const {
	return allowed_building_types_.count(i) == 1;
}

/*
 * allow building
 *
 * Disable or enable a building for a player
 */
void Player::allow_building_type(DescriptionIndex const i, bool const allow) {
	if (allow) {
		allowed_building_types_.insert(i);
	} else {
		auto denyme = allowed_building_types_.find(i);
		if (denyme != allowed_building_types_.end()) {
			allowed_building_types_.erase(denyme);
		}
	}
}

/*
 * Economy stuff below
 */
Economy* Player::create_economy(WareWorker type) {
	std::unique_ptr<Economy> eco(new Economy(*this, type));
	const Serial serial = eco->serial();

	assert(economies_.count(serial) == 0);
	economies_.emplace(std::make_pair(serial, std::move(eco)));
	assert(economies_.at(serial)->serial() == serial);
	assert(economies_.count(serial) == 1);

	return get_economy(serial);
}

Economy* Player::create_economy(Serial serial, WareWorker type) {
	std::unique_ptr<Economy> eco(new Economy(*this, serial, type));

	assert(economies_.count(serial) == 0);
	economies_.emplace(std::make_pair(serial, std::move(eco)));
	assert(economies_.at(serial)->serial() == serial);
	assert(economies_.count(serial) == 1);

	return get_economy(serial);
}

void Player::remove_economy(Serial serial) {
	assert(has_economy(serial));
	economies_.erase(economies_.find(serial));
	assert(!has_economy(serial));
}

const std::map<Serial, std::unique_ptr<Economy>>& Player::economies() const {
	return economies_;
}

Economy* Player::get_economy(Widelands::Serial serial) const {
	if (economies_.count(serial) == 0) {
		return nullptr;
	}
	return economies_.at(serial).get();
}

bool Player::has_economy(Widelands::Serial serial) const {
	return economies_.count(serial) != 0;
}

/************  Military stuff  **********/

/*
==========
Change the training priotity values
==========
*/
void Player::change_training_options(TrainingSite& trainingsite,
                                     TrainingAttribute attr,
                                     int32_t const val) {
	if (trainingsite.get_owner() == this) {
		trainingsite.set_pri(attr, trainingsite.get_pri(attr) + val);
	}
}

/*
===========
Forces the drop of given soldier at given house
===========
*/
void Player::drop_soldier(PlayerImmovable& imm, Soldier& soldier) {
	if (imm.get_owner() != this) {
		return;
	}
	if (soldier.descr().type() != MapObjectType::SOLDIER) {
		return;
	}
	if (upcast(Building, building, &imm)) {
		SoldierControl* soldier_control = building->mutable_soldier_control();
		if (soldier_control != nullptr) {
			soldier_control->drop_soldier(soldier);
		}
	}
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
uint32_t
Player::find_attack_soldiers(Flag& flag, std::vector<Soldier*>* soldiers, uint32_t nr_wanted) {
	uint32_t count = 0;

	if (soldiers) {
		soldiers->clear();
	}

	const Map& map = egbase().map();
	std::vector<BaseImmovable*> flags;

	map.find_reachable_immovables_unique(
	   egbase(), Area<FCoords>(map.get_fcoords(flag.get_position()), 25), flags,
	   CheckStepDefault(MOVECAPS_WALK), FindFlagOf(FindImmovablePlayerMilitarySite(*this)));

	if (flags.empty()) {
		return 0;
	}

	for (BaseImmovable* temp_flag : flags) {
		upcast(Flag, attackerflag, temp_flag);
		const SoldierControl* soldier_control = attackerflag->get_building()->soldier_control();
		assert(soldier_control != nullptr);
		std::vector<Soldier*> const present = soldier_control->present_soldiers();
		uint32_t const nr_staying = soldier_control->min_soldier_capacity();
		uint32_t const nr_present = present.size();
		if (nr_staying < nr_present) {
			uint32_t const nr_taken = std::min(nr_wanted, nr_present - nr_staying);
			if (soldiers) {
				soldiers->insert(soldiers->end(), present.begin(), present.begin() + nr_taken);
			}
			count += nr_taken;
			nr_wanted -= nr_taken;
			if (!nr_wanted) {
				break;
			}
		}
	}

	return count;
}

// TODO(unknown): Clean this mess up. The only action we really have right now is
// to attack, so pretending we have more types is pointless.
void Player::enemyflagaction(Flag& flag,
                             PlayerNumber const attacker,
                             const std::vector<Widelands::Soldier*>& soldiers) {
	if (attacker != player_number()) {
		log_warn_time(egbase().get_gametime(), "Player (%d) is not the sender of an attack (%d)\n",
		              attacker, player_number());
	} else if (soldiers.empty()) {
		log_warn_time(egbase().get_gametime(), "enemyflagaction: no soldiers given\n");
	} else if (is_hostile(flag.owner())) {
		if (Building* const building = flag.get_building()) {
			if (const AttackTarget* attack_target = building->attack_target()) {
				if (attack_target->can_be_attacked()) {
					for (Soldier* temp_attacker : soldiers) {
						assert(temp_attacker);
						assert(temp_attacker->get_owner() == this);
						if (upcast(MilitarySite, ms, temp_attacker->get_location(egbase()))) {
							assert(ms->get_owner() == this);
							ms->send_attacker(*temp_attacker, *building);
						} else {
							// The soldier may not be in a militarysite anymore if he was kicked out
							// in the short delay between sending and executing a playercommand
							log_warn_time(
							   egbase().get_gametime(),
							   "Player(%u)::enemyflagaction: Not sending soldier %u because he left the "
							   "building\n",
							   player_number(), temp_attacker->serial());
						}
					}
				}
			}
		}
	}
}

void Player::rediscover_node(const Map& map, const FCoords& f) {

	assert(0 <= f.x);
	assert(f.x < map.get_width());
	assert(0 <= f.y);
	assert(f.y < map.get_height());
	const Widelands::Field& first_map_field = map[0];
	assert(&first_map_field <= f.field);
	assert(f.field < &first_map_field + map.max_index());

	Field& field = fields_[f.field - &first_map_field];

	assert(fields_.get() <= &field);
	assert(&field < fields_.get() + map.max_index());

	{  // discover everything (above the ground) in this field
		field.terrains = f.field->get_terrains();
		field.r_e = f.field->get_road(WALK_E);
		field.r_se = f.field->get_road(WALK_SE);
		field.r_sw = f.field->get_road(WALK_SW);
		field.owner = f.field->get_owned_by();

		// Check if this node is part of a border

		int32_t const mapwidth = map.get_width();
		// right neighbour
		const FCoords r = map.r_n(f);
		const MapIndex r_index = map.get_index(r, mapwidth);
		const SeeUnseeNode r_vision = get_vision(r_index);
		// bottom right neighbour
		const FCoords br = map.br_n(f);
		const MapIndex br_index = map.get_index(br, mapwidth);
		const SeeUnseeNode br_vision = get_vision(br_index);
		// bottom left neighbour
		const FCoords bl = map.bl_n(f);
		const MapIndex bl_index = map.get_index(bl, mapwidth);
		const SeeUnseeNode bl_vision = get_vision(bl_index);

		field.border = f.field->is_border();
		// draw a border if both nodes are borders belonging to the same player for all we know
		field.border_r = field.border && r_vision != SeeUnseeNode::kUnexplored &&
		                 fields_[r_index].border && fields_[r_index].owner == field.owner;
		field.border_br = field.border && br_vision != SeeUnseeNode::kUnexplored &&
		                  fields_[br_index].border && fields_[br_index].owner == field.owner;
		field.border_bl = field.border && bl_vision != SeeUnseeNode::kUnexplored &&
		                  fields_[bl_index].border && fields_[bl_index].owner == field.owner;
		{
			const MapObjectDescr* map_object_descr = nullptr;
			if (const BaseImmovable* base_immovable = f.field->get_immovable()) {
				map_object_descr = &base_immovable->descr();

				if (Road::is_road_descr(map_object_descr) ||
				    Waterway::is_waterway_descr(map_object_descr)) {
					map_object_descr = nullptr;
				} else if (upcast(Building const, building, base_immovable)) {
					if (building->get_position() != f) {
						// This is not the building's main position. We don't store the building's
						// description here but in its main position instead.
						map_object_descr = nullptr;
						FCoords main_coords = map.get_fcoords(building->get_position());
						Field& field_main = fields_[main_coords.field - &first_map_field];
						if (field_main.seeing != SeeUnseeNode::kVisible) {
							rediscover_node(map, main_coords);
							field_main.time_node_last_unseen = egbase().get_gametime();
							field_main.seeing = SeeUnseeNode::kPreviouslySeen;
						}
					} else {
						if (upcast(ConstructionSite const, cs, building)) {
							field.partially_finished_building.constructionsite =
							   const_cast<ConstructionSite*>(cs)->get_info();
						} else if (upcast(DismantleSite const, ds, building)) {
							field.partially_finished_building.dismantlesite.progress =
							   ds->get_built_per64k();
							field.partially_finished_building.dismantlesite.building = ds->get_building();
						}
					}
				}
			}
			field.map_object_descr = map_object_descr;
		}
	}
	{  //  discover the D triangle and the SW edge of the top right neighbour
		FCoords tr = map.tr_n(f);
		Field& tr_field = fields_[tr.field - &first_map_field];
		if (tr_field.seeing != SeeUnseeNode::kVisible) {
			tr_field.terrains.d = tr.field->terrain_d();
			tr_field.r_sw = tr.field->get_road(WALK_SW);
			tr_field.owner = tr.field->get_owned_by();
		}
	}
	{  //  discover both triangles and the SE edge of the top left  neighbour
		FCoords tl = map.tl_n(f);
		Field& tl_field = fields_[tl.field - &first_map_field];
		if (tl_field.seeing != SeeUnseeNode::kVisible) {
			tl_field.terrains = tl.field->get_terrains();
			tl_field.r_se = tl.field->get_road(WALK_SE);
			tl_field.owner = tl.field->get_owned_by();
		}
	}
	{  //  discover the R triangle and the  E edge of the     left  neighbour
		FCoords l = map.l_n(f);
		Field& l_field = fields_[l.field - &first_map_field];
		if (l_field.seeing != SeeUnseeNode::kVisible) {
			l_field.terrains.r = l.field->terrain_r();
			l_field.r_e = l.field->get_road(WALK_E);
			l_field.owner = l.field->get_owned_by();
		}
	}
}

SeeUnseeNode Player::get_vision(MapIndex const i) const {
	return see_all_ ? SeeUnseeNode::kVisible : fields_[i].seeing;
}

void Player::add_seer(const MapObject& m, const Area<FCoords>& a) {
	add_seer(m);
	for (const PlayerNumber& p : team_player_) {
		egbase().get_player(p)->update_vision(a, true);
	}
}

void Player::add_seer(const MapObject& m) {
#ifndef NDEBUG
	for (const MapObject* mo : seers_) {
		if (mo == &m) {
			throw wexception("duplicate seer entry for %s %u", m.descr().name().c_str(), m.serial());
		}
	}
#endif
	seers_.push_back(&m);
}

void Player::remove_seer(const MapObject& m, const Area<FCoords>& a) {
	for (auto it = seers_.begin(); it != seers_.end(); ++it) {
		if (*it == &m) {
			seers_.erase(it);
			if (!egbase().objects().is_cleaning_up()) {
				for (const PlayerNumber& p : team_player_) {
					egbase().get_player(p)->update_vision(a, false);
				}
			}
			return;
		}
	}
	NEVER_HERE();
}

// Checks if any of our buildings or bobs, or one of one of our team mates, is seeing this node
bool Player::should_see(const FCoords& f) const {
	for (const PlayerNumber& p : team_player_) {
		const Player& player = *egbase().get_player(p);
		if (player.revealed_fields_.count(f)) {
			return true;
		}
		for (const MapObject* mo : player.seers_) {
			if (mo->descr().type() >= MapObjectType::BUILDING) {
				upcast(const Building, b, mo);
				assert(b);
				if (b->is_seeing() &&
				    egbase().map().calc_distance(f, b->get_position()) <= b->descr().vision_range()) {
					return true;
				}
			} else {
				// currently only buildings and bobs can see fields
				upcast(const Bob, b, mo);
				assert(b);
				if (egbase().map().calc_distance(f, b->get_position()) <= b->descr().vision_range()) {
					return true;
				}
			}
		}
	}
	return false;
}

void Player::update_vision(const FCoords& f, bool force_visible) {
	if (!fields_ || egbase().objects().is_cleaning_up()) {
		return;
	}
	Player::Field& field = fields_[egbase().map().get_index(f)];
	if (force_visible || should_see(f)) {
		if (field.seeing != SeeUnseeNode::kVisible) {
			field.seeing = SeeUnseeNode::kVisible;
			rediscover_node(egbase().map(), f);
		}
	} else if (field.seeing != SeeUnseeNode::kUnexplored) {
		if (field.seeing == SeeUnseeNode::kVisible) {
			rediscover_node(egbase().map(), f);  // make sure all info is up to date
			field.time_node_last_unseen = egbase().get_gametime();
		}
		field.seeing = SeeUnseeNode::kPreviouslySeen;
	}
}

void Player::update_vision_whole_map() {
	if (!fields_ || egbase().objects().is_cleaning_up()) {
		return;
	}
	const MapIndex max = egbase().map().max_index();
	Widelands::Field* f = &egbase().map()[0];
	for (MapIndex i = 0; i < max; ++i, ++f) {
		update_vision(egbase().map().get_fcoords(*f), false);
	}
}

void Player::update_vision(const Area<FCoords>& area, bool force_visible) {
	if (egbase().objects().is_cleaning_up()) {
		return;
	}
	for (const PlayerNumber& p : team_player_) {
		Player& player = *egbase().get_player(p);
		if (!player.fields_) {
			return;
		}
		MapRegion<Area<FCoords>> mr(egbase().map(), area);
		do {
			player.update_vision(mr.location(), force_visible);
		} while (mr.advance(egbase().map()));
	}
}

void Player::hide_or_reveal_field(const Coords& coords, SeeUnseeNode mode) {
	const Map& map = egbase().map();
	FCoords fcoords = map.get_fcoords(coords);
	const Widelands::MapIndex index = fcoords.field - &map[0];

	switch (mode) {
	case SeeUnseeNode::kVisible:
		revealed_fields_.insert(index);
		break;
	case SeeUnseeNode::kUnexplored:
		if (fields_) {
			fields_[index].seeing = SeeUnseeNode::kUnexplored;
		}
		FALLS_THROUGH;
	case SeeUnseeNode::kPreviouslySeen:
		auto it = revealed_fields_.find(index);
		if (it != revealed_fields_.end()) {
			revealed_fields_.erase(it);
		}
		break;
	}

	update_vision(fcoords, mode == SeeUnseeNode::kVisible);
}

/**
 * Called by Game::think to sample statistics data in regular intervals.
 */
void Player::sample_statistics() {
	assert(current_consumed_statistics_.size() == tribe().wares().size());
	assert(current_produced_statistics_.size() == tribe().wares().size());
	assert(ware_productions_.size() == tribe().wares().size());
	assert(ware_consumptions_.size() == tribe().wares().size());
	assert(ware_stocks_.size() == tribe().wares().size());

	// Calculate stocks
	std::map<DescriptionIndex, Quantity> stocks;
	for (DescriptionIndex idx : tribe().wares()) {
		stocks.insert(std::make_pair(idx, 0U));
	}

	for (const auto& economy : economies()) {
		if (economy.second->type() == wwWARE) {
			for (Widelands::Warehouse* warehouse : economy.second->warehouses()) {
				const Widelands::WareList& wares = warehouse->get_wares();
				for (DescriptionIndex idx : tribe().wares()) {
					stocks.at(idx) += wares.stock(idx);
				}
			}
		}
	}

	// Update statistics
	for (DescriptionIndex idx : tribe().wares()) {
		ware_productions_.at(idx).push_back(current_produced_statistics_.at(idx));
		current_produced_statistics_.at(idx) = 0;

		ware_consumptions_.at(idx).push_back(current_consumed_statistics_.at(idx));
		current_consumed_statistics_.at(idx) = 0;

		ware_stocks_.at(idx).push_back(stocks.at(idx));
	}
}

/**
 * A ware was produced. Update the corresponding statistics.
 */
void Player::ware_produced(DescriptionIndex const wareid) {
	assert(tribe().has_ware(wareid));
	assert(egbase().tribes().ware_exists(wareid));
	++current_produced_statistics_.at(wareid);
}

/**
 * Return count of produced wares for ware index
 */
uint32_t Player::get_current_produced_statistics(uint8_t const wareid) {
	assert(tribe().has_ware(wareid));
	uint32_t sum = current_produced_statistics_.at(wareid);
	for (const auto stat : *get_ware_production_statistics(wareid)) {
		sum += stat;
	}
	return sum;
}

/**
 * Some units from one kind of ware were consumed.
 * Update the corresponding statistics
 *
 * \param wareid the ID of the consumed wares
 * \param count the number of consumed wares
 */
void Player::ware_consumed(DescriptionIndex const wareid, uint8_t const count) {
	assert(tribe().has_ware(wareid));
	current_consumed_statistics_.at(wareid) += count;
}

/**
 * Get current ware production statistics
 */
const std::vector<uint32_t>*
Player::get_ware_production_statistics(DescriptionIndex const ware) const {
	assert(tribe().has_ware(ware));
	return &ware_productions_.at(ware);
}

/**
 * Get current ware consumption statistics
 */
const std::vector<uint32_t>*
Player::get_ware_consumption_statistics(DescriptionIndex const ware) const {
	assert(tribe().has_ware(ware));
	return &ware_consumptions_.at(ware);
}

const std::vector<uint32_t>* Player::get_ware_stock_statistics(DescriptionIndex const ware) const {
	assert(tribe().has_ware(ware));
	return &ware_stocks_.at(ware);
}

const Player::BuildingStatsVector&
Player::get_building_statistics(const DescriptionIndex& i) const {
	return *const_cast<Player*>(this)->get_mutable_building_statistics(i);
}

Player::BuildingStatsVector* Player::get_mutable_building_statistics(const DescriptionIndex& i) {
	DescriptionIndex const nr_buildings = egbase().tribes().nrbuildings();
	if (building_stats_.size() < nr_buildings) {
		building_stats_.resize(nr_buildings);
	}
	return &building_stats_[i];
}

/**
 * Add or remove the given building from building statistics.
 * Only to be called by \ref receive
 */
void Player::update_building_statistics(Building& building, NoteImmovable::Ownership ownership) {
	upcast(ConstructionSite const, constructionsite, &building);
	const std::string& building_name =
	   constructionsite ? constructionsite->building().name() : building.descr().name();

	const size_t nr_buildings = egbase().tribes().nrbuildings();

	// Get the valid vector for this
	if (building_stats_.size() < nr_buildings) {
		building_stats_.resize(nr_buildings);
	}

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

		throw wexception("InteractivePlayer::loose_immovable(): A building should be "
		                 "removed at (%i, %i), but nothing is known about this building!",
		                 building_position.x, building_position.y);
	}
}
/**
 * Functions used by AI to save/read AI data stored in Player class.
 */

void Player::set_ai(const std::string& ai) {
	ai_ = ai;
}

const std::string& Player::get_ai() const {
	return ai_;
}

void Player::set_muted(DescriptionIndex di, bool mute) {
	if (mute) {
		muted_building_types_.insert(di);
	} else {
		auto it = muted_building_types_.find(di);
		if (it != muted_building_types_.end()) {
			muted_building_types_.erase(it);
		}
	}
}

bool Player::is_attack_forbidden(PlayerNumber who) const {
	return forbid_attack_.find(who) != forbid_attack_.end();
}

void Player::add_soldier(unsigned h, unsigned a, unsigned d, unsigned e) {
	SoldierStatistics ss(h, a, d, e);
	auto it = std::find(soldier_stats_.begin(), soldier_stats_.end(), ss);
	if (it == soldier_stats_.end()) {
		++ss.total;
		soldier_stats_.push_back(ss);
	} else {
		++it->total;
	}
}
void Player::remove_soldier(unsigned h, unsigned a, unsigned d, unsigned e) {
	auto it = std::find(soldier_stats_.begin(), soldier_stats_.end(), SoldierStatistics(h, a, d, e));
	assert(it != soldier_stats_.end());
	assert(it->total > 0);
	--it->total;
}
uint32_t Player::count_soldiers(unsigned h, unsigned a, unsigned d, unsigned e) const {
	const auto it =
	   std::find(soldier_stats_.begin(), soldier_stats_.end(), SoldierStatistics(h, a, d, e));
	return it == soldier_stats_.end() ? 0 : it->total;
}
uint32_t Player::count_soldiers_h(unsigned value) const {
	uint32_t s = 0;
	for (const SoldierStatistics& ss : soldier_stats_) {
		if (ss.health == value) {
			s += ss.total;
		}
	}
	return s;
}
uint32_t Player::count_soldiers_a(unsigned value) const {
	uint32_t s = 0;
	for (const SoldierStatistics& ss : soldier_stats_) {
		if (ss.attack == value) {
			s += ss.total;
		}
	}
	return s;
}
uint32_t Player::count_soldiers_d(unsigned value) const {
	uint32_t s = 0;
	for (const SoldierStatistics& ss : soldier_stats_) {
		if (ss.defense == value) {
			s += ss.total;
		}
	}
	return s;
}
uint32_t Player::count_soldiers_e(unsigned value) const {
	uint32_t s = 0;
	for (const SoldierStatistics& ss : soldier_stats_) {
		if (ss.evade == value) {
			s += ss.total;
		}
	}
	return s;
}

void Player::set_attack_forbidden(PlayerNumber who, bool forbid) {
	const auto it = forbid_attack_.find(who);
	if (forbid ^ (it == forbid_attack_.end())) {
		return;
	} else if (forbid) {
		forbid_attack_.emplace(who);
	} else {
		forbid_attack_.erase(it);
	}
}

/**
 * Pick random name from remaining names (if any)
 */
const std::string Player::pick_shipname() {
	++ship_name_counter_;

	if (remaining_shipnames_.empty()) {
		return (boost::format(pgettext("shipname", "Ship %d")) % ship_name_counter_).str();
	}

	Game& game = dynamic_cast<Game&>(egbase());
	const size_t index = game.logic_rand() % remaining_shipnames_.size();
	auto it = remaining_shipnames_.begin();
	std::advance(it, index);
	std::string new_name = *it;
	remaining_shipnames_.erase(it);
	return new_name;
}

/**
 * Read remaining ship indexes to the give file
 *
 * \param fr source stream
 */
void Player::read_remaining_shipnames(FileRead& fr) {
	// First get rid of default shipnames
	remaining_shipnames_.clear();
	const uint16_t count = fr.unsigned_16();
	for (uint16_t i = 0; i < count; ++i) {
		remaining_shipnames_.push_back(fr.string());
	}
	ship_name_counter_ = fr.unsigned_32();
}

void Player::init_statistics() {
	current_produced_statistics_.clear();
	current_consumed_statistics_.clear();
	ware_consumptions_.clear();
	ware_productions_.clear();
	ware_stocks_.clear();

	for (DescriptionIndex ware_index : tribe().wares()) {
		current_produced_statistics_.insert(std::make_pair(ware_index, 0U));
		current_consumed_statistics_.insert(std::make_pair(ware_index, 0U));
		ware_consumptions_.insert(std::make_pair(ware_index, std::vector<Quantity>()));
		ware_productions_.insert(std::make_pair(ware_index, std::vector<Quantity>()));
		ware_stocks_.insert(std::make_pair(ware_index, std::vector<Quantity>()));
	}
}

/**
 * Read statistics data from a file.
 *
 * \param fr source stream
 */
void Player::read_statistics(FileRead& fr,
                             const uint16_t /* packet_version */,
                             const TribesLegacyLookupTable& lookup_table) {
	uint16_t nr_wares = fr.unsigned_16();
	size_t nr_entries = fr.unsigned_16();
	assert(tribe().wares().size() >= nr_wares);

	// Stats are saved as a single string to reduce number of hard disk write operations
	const auto parse_stats = [nr_entries](StatisticsMap* stats, const DescriptionIndex ware_index,
	                                      const std::string& stats_string,
	                                      const std::string& description) {
		if (!stats_string.empty()) {
			std::vector<std::string> stats_vector;
			boost::split(stats_vector, stats_string, boost::is_any_of("|"));
			if (stats_vector.size() != nr_entries) {
				throw GameDataError("wrong number of %s statistics - expected %" PRIuS
				                    " but got %" PRIuS,
				                    description.c_str(), nr_entries, stats_vector.size());
			}
			for (size_t j = 0; j < nr_entries; ++j) {
				stats->at(ware_index)[j] =
				   boost::lexical_cast<unsigned int>(stats_vector.at(j).c_str());
			}
		} else if (nr_entries > 0) {
			throw GameDataError("wrong number of %s statistics - expected %" PRIuS " but got 0",
			                    description.c_str(), nr_entries);
		}
	};

	for (DescriptionIndex idx : tribe().wares()) {
		ware_productions_[idx].resize(nr_entries);
	}

	for (uint16_t i = 0; i < nr_wares; ++i) {
		const std::string name = lookup_table.lookup_ware(fr.c_string());
		const DescriptionIndex idx = egbase().tribes().ware_index(name);
		if (!egbase().tribes().ware_exists(idx)) {
			log_warn_time(egbase().get_gametime(), "Player %u statistics: unknown ware name %s",
			              player_number(), name.c_str());
			continue;
		}

		current_produced_statistics_[idx] = fr.unsigned_32();
		parse_stats(&ware_productions_, idx, fr.c_string(), "produced");
	}

	// Read consumption statistics
	nr_wares = fr.unsigned_16();
	nr_entries = fr.unsigned_16();
	assert(tribe().wares().size() >= nr_wares);

	for (DescriptionIndex idx : tribe().wares()) {
		ware_consumptions_[idx].resize(nr_entries);
	}

	for (uint16_t i = 0; i < nr_wares; ++i) {
		const std::string name = lookup_table.lookup_ware(fr.c_string());
		const DescriptionIndex idx = egbase().tribes().ware_index(name);
		if (!egbase().tribes().ware_exists(idx)) {
			log_warn_time(egbase().get_gametime(),
			              "Player %u consumption statistics: unknown ware name %s", player_number(),
			              name.c_str());
			continue;
		}

		current_consumed_statistics_[idx] = fr.unsigned_32();
		parse_stats(&ware_consumptions_, idx, fr.c_string(), "consumed");
	}

	// Read stock statistics
	nr_wares = fr.unsigned_16();
	nr_entries = fr.unsigned_16();
	assert(tribe().wares().size() >= nr_wares);

	for (DescriptionIndex idx : tribe().wares()) {
		ware_stocks_[idx].resize(nr_entries);
	}

	for (uint16_t i = 0; i < nr_wares; ++i) {
		const std::string name = lookup_table.lookup_ware(fr.c_string());
		const DescriptionIndex idx = egbase().tribes().ware_index(name);
		if (!egbase().tribes().ware_exists(idx)) {
			log_warn_time(egbase().get_gametime(), "Player %u stock statistics: unknown ware name %s",
			              player_number(), name.c_str());
			continue;
		}

		parse_stats(&ware_stocks_, idx, fr.c_string(), "stock");
	}

	// All statistics should have the same size
	assert(ware_productions_.size() == ware_consumptions_.size());
	assert(ware_productions_.begin()->second.size() == ware_consumptions_.begin()->second.size());

	assert(ware_productions_.size() == ware_stocks_.size());
	assert(ware_productions_.begin()->second.size() == ware_stocks_.begin()->second.size());
}

/**
 * Write remaining ship indexes to the given file
 */
void Player::write_remaining_shipnames(FileWrite& fw) const {
	fw.unsigned_16(remaining_shipnames_.size());
	for (const auto& shipname : remaining_shipnames_) {
		fw.string(shipname);
	}
	fw.unsigned_32(ship_name_counter_);
}

/**
 * Write statistics data to the given file
 */
void Player::write_statistics(FileWrite& fw) const {
	// Save stats as a single string to reduce number of hard disk write operations
	const auto write_stats = [&fw](const StatisticsMap& stats, const DescriptionIndex ware_index) {
		std::ostringstream oss("");
		const int sizem = stats.at(ware_index).size() - 1;
		if (sizem >= 0) {
			for (int i = 0; i < sizem; ++i) {
				oss << stats.at(ware_index).at(i) << "|";
			}
			oss << stats.at(ware_index).at(sizem);
		}
		fw.c_string(oss.str());
	};

	const Tribes& tribes = egbase().tribes();
	const std::set<DescriptionIndex>& tribe_wares = tribe().wares();
	const size_t nr_wares = tribe_wares.size();

	// Write produce statistics
	fw.unsigned_16(nr_wares);
	fw.unsigned_16(ware_productions_.begin()->second.size());

	for (const DescriptionIndex ware_index : tribe_wares) {
		fw.c_string(tribes.get_ware_descr(ware_index)->name());
		fw.unsigned_32(current_produced_statistics_.at(ware_index));
		write_stats(ware_productions_, ware_index);
	}

	// Write consume statistics
	fw.unsigned_16(nr_wares);
	fw.unsigned_16(ware_consumptions_.begin()->second.size());

	for (const DescriptionIndex ware_index : tribe_wares) {
		fw.c_string(tribes.get_ware_descr(ware_index)->name());
		fw.unsigned_32(current_consumed_statistics_.at(ware_index));
		write_stats(ware_consumptions_, ware_index);
	}

	// Write stock statistics
	fw.unsigned_16(nr_wares);
	fw.unsigned_16(ware_stocks_.begin()->second.size());

	for (const DescriptionIndex ware_index : tribe_wares) {
		fw.c_string(tribes.get_ware_descr(ware_index)->name());
		write_stats(ware_stocks_, ware_index);
	}
}
}  // namespace Widelands
