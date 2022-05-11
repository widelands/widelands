/*
 * Copyright (C) 2009-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ai/defaultai.h"

#include <cstdlib>

#include "base/log.h"
#include "economy/ship_fleet.h"
#include "economy/wares_queue.h"

namespace AI {

// this scores spot for potential colony
uint8_t DefaultAI::spot_scoring(Widelands::Coords candidate_spot) {

	const Widelands::Map& map = game().map();
	Widelands::PlayerNumber const pn = player_->player_number();
	uint8_t score = 0;
	uint16_t mineable_fields_count = 0;
	uint32_t tested_fields = 0;

	// protocol out
	verb_log_dbg_time(game().get_gametime(),
	                  "%d: (%3dx%3d) expedition spot scoring, colony_scan_area == %u\n", pn,
	                  candidate_spot.x, candidate_spot.y, persistent_data->colony_scan_area);

	// abort if any player - including self - is too near to the spot (radius 10)
	if (other_player_accessible(Widelands::Player::AiPersistentState::kColonyScanMinArea,
	                            &tested_fields, &mineable_fields_count, candidate_spot,
	                            WalkSearch::kAnyPlayer)) {
		return 0;
	}

	// now check with variant radius (depending on expedition time)
	// on the beginning we search for completely deserted area,
	// later we will accept also area nearer to friendly territory
	WalkSearch search_type = WalkSearch::kAnyPlayer;
	if (persistent_data->colony_scan_area < 25) {
		search_type = WalkSearch::kEnemy;
	}

	// abort if we run into other player
	if (other_player_accessible(persistent_data->colony_scan_area, &tested_fields,
	                            &mineable_fields_count, candidate_spot, search_type)) {
		return 0;
	}

	// abort if available area (island) is too small...
	// colony_scan_area is a radius (distance) and has no direct relevance to the size of area,
	// but it seems a good measurement
	if (tested_fields < persistent_data->colony_scan_area) {
		return 0;
	}

	// if we are here we put score
	score = 1;
	if (mineable_fields_count > 0) {
		++score;
	}

	// here we check for surface rocks + trees
	static std::vector<Widelands::ImmovableFound> immovables;
	immovables.clear();
	immovables.reserve(50);
	// Search in a radius of range
	map.find_immovables(game(),
	                    Widelands::Area<Widelands::FCoords>(map.get_fcoords(candidate_spot), 10),
	                    &immovables);

	int32_t const rocks_attr = Widelands::MapObjectDescr::get_attribute_id("rocks");
	uint16_t rocks = 0;
	int32_t const tree_attr = Widelands::MapObjectDescr::get_attribute_id("tree");
	uint16_t trees = 0;

	for (const Widelands::ImmovableFound& imm_found : immovables) {
		if (imm_found.object->has_attribute(rocks_attr)) {
			++rocks;
		}
		if (imm_found.object->has_attribute(tree_attr)) {
			++trees;
		}
	}
	if (rocks > 1) {
		++score;
	}
	if (trees > 1) {
		++score;
	}

	return score;
}

// This function scans current situation with shipyards, ports, ships, ongoing expeditions
// and makes two decisions:
// - build a ship
// - start preparation for expedition
bool DefaultAI::marine_main_decisions(const Time& gametime) {
	if (gametime > last_seafaring_check_ + Duration(20 * 1000)) {
		const Widelands::Map& map = game().map();
		map_allows_seafaring_ = map.allows_seafaring();
		last_seafaring_check_ = gametime;
	}
	if (!map_allows_seafaring_ &&
	    count_buildings_with_attribute(BuildingAttribute::kShipyard) == 0 && allships.empty()) {
		return false;
	}

	// getting some base statistics
	player_ = game().get_player(player_number());
	uint16_t ports_count = 0;
	uint16_t shipyards_count = 0;
	uint16_t expeditions_in_prep = 0;
	uint16_t expeditions_ready = 0;
	uint16_t expeditions_in_progress = 0;
	bool idle_shipyard_stocked = false;

	// goes over productionsites and gets status of shipyards
	for (const ProductionSiteObserver& ps_obs : productionsites) {
		if (ps_obs.bo->is(BuildingAttribute::kShipyard)) {
			++shipyards_count;

			// In very rare situation, we might have non-seafaring map but there is a shipyard
			if (!map_allows_seafaring_) {
				verb_log_dbg_time(
				   game().get_gametime(),
				   "  %1d: we have a shipyard in a non seafaring economy, dismantling it...\n",
				   player_number());
				if (!ps_obs.site->get_economy(Widelands::wwWORKER)->warehouses().empty()) {
					game().send_player_dismantle(*ps_obs.site, true);
				} else {
					game().send_player_bulldoze(*ps_obs.site);
				}
				return false;
			}
		}
	}

	// If non-seafaring economy, no sense to go on with this function
	if (!map_allows_seafaring_) {
		return false;
	}

	// goes over all warehouses (these includes ports)
	for (const WarehouseSiteObserver& wh_obs : warehousesites) {
		if (wh_obs.bo->is(BuildingAttribute::kPort)) {
			++ports_count;
			if (const Widelands::PortDock* pd = wh_obs.site->get_portdock()) {
				if (pd->expedition_started()) {
					++expeditions_in_prep;
				}
				if (pd->is_expedition_ready()) {
					++expeditions_ready;
				}
			}
		}
	}

	// iterates over all ships
	for (const ShipObserver& observer : allships) {
		if (observer.ship->state_is_expedition()) {
			++expeditions_in_progress;
		}
	}

	assert(allships.size() >= expeditions_in_progress);
	bool ship_free = allships.size() - expeditions_in_progress > 0;

	/* Now we decide whether we have enough ships or need to build another:
	 * - We always need at least one ship in transport mode
	 * - We want at least as many free ships as we have ports
	 * - If ships utilization is too high
	 */
	const bool need_ship =
	   ports_count > 0 && shipyards_count > 0 && basic_economy_established &&
	   (!ship_free || persistent_data->ships_utilization > 5000 ||
	    static_cast<int>(allships.size()) - ports_count - expeditions_in_progress < 0);

	// goes over productionsites finds shipyards and configures them
	for (const ProductionSiteObserver& ps_obs : productionsites) {
		if (ps_obs.bo->is(BuildingAttribute::kShipyard)) {
			// counting stocks
			uint8_t stocked_wares = 0;
			std::vector<Widelands::InputQueue*> const inputqueues = ps_obs.site->inputqueues();
			for (Widelands::InputQueue* queue : inputqueues) {
				if (queue->get_type() == Widelands::wwWARE) {
					stocked_wares += queue->get_filled();
				}
			}
			if (stocked_wares == 16 && ps_obs.site->is_stopped() && ps_obs.site->can_start_working()) {
				idle_shipyard_stocked = true;
			}

			if (expeditions_ready > 0 && idle_shipyard_stocked && allships.size() - ports_count == 0) {
				if (potential_wrong_shipyard_) {
					if (!ps_obs.site->get_economy(Widelands::wwWORKER)->warehouses().empty()) {
						game().send_player_dismantle(*ps_obs.site, true);
					} else {
						game().send_player_bulldoze(*ps_obs.site);
					}
					game().send_player_sink_ship(*allships.back().ship);
					allships.pop_back();
					potential_wrong_shipyard_ = false;
					return true;
				}
				potential_wrong_shipyard_ = true;
				return false;
			}
			if (need_ship && idle_shipyard_stocked) {
				game().send_player_start_stop_building(*ps_obs.site);
			}
		}
	}

	// starting an expedition? if yes, find a port and order it to start an expedition
	if (ports_count > 0 && expeditions_in_progress == 0 && expeditions_in_prep == 0 &&
	    !persistent_data->no_more_expeditions && ship_free && basic_economy_established) {

		// we need to find a port
		for (const WarehouseSiteObserver& wh_obs : warehousesites) {
			if (wh_obs.bo->is(BuildingAttribute::kPort)) {
				verb_log_dbg_time(game().get_gametime(),
				                  "  %1d: Starting preparation for expedition in port at %3dx%3d\n",
				                  player_number(), wh_obs.site->get_position().x,
				                  wh_obs.site->get_position().y);
				game().send_player_start_or_cancel_expedition(*wh_obs.site);
				return true;
			}
		}
	}
	return true;
}

// This identifies ships that are waiting for command
bool DefaultAI::check_ships(const Time& gametime) {
	// There is possibility that the map is not seafaring but we still have ships and/or shipyards
	if (!map_allows_seafaring_ &&
	    count_buildings_with_attribute(BuildingAttribute::kShipyard) == 0 && allships.empty()) {
		// False indicates that we can postpone next call of this function
		return false;
	}

	if (!allships.empty()) {
		// iterating over ships and doing what is needed
		for (ShipObserver& so : allships) {

			const Widelands::Ship::ShipStates ship_state = so.ship->get_ship_state();

			// Here we manage duration of expedition and related variables
			if (ship_state == Widelands::Ship::ShipStates::kExpeditionWaiting ||
			    ship_state == Widelands::Ship::ShipStates::kExpeditionScouting ||
			    ship_state == Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {

				// the function below will take care of variables like
				// - expedition_ship_
				// - expedition_start_time
				// - expected_colony_scan
				// - no_more_expeditions_
				check_ship_in_expedition(so, gametime);

				// We are not in expedition mode (or perhaps building a colonisation port)
				// so resetting start time
			} else if (expedition_ship_ == so.ship->serial()) {
				// Obviously expedition just ended
				persistent_data->expedition_start_time =
				   Widelands::Player::AiPersistentState::kNoExpedition;
				expedition_ship_ = kNoShip;
			}

			// only two states need an attention
			if ((so.ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionWaiting ||
			     so.ship->get_ship_state() ==
			        Widelands::Ship::ShipStates::kExpeditionPortspaceFound) &&
			    !so.waiting_for_command_) {
				if (gametime - so.last_command_time > Duration(180 * 1000)) {
					so.waiting_for_command_ = true;
					verb_log_warn_time(
					   gametime,
					   "  %1d: last command for ship %s at %3dx%3d was %3d seconds ago, something wrong "
					   "here?...\n",
					   player_number(), so.ship->get_shipname().c_str(), so.ship->get_position().x,
					   so.ship->get_position().y, (gametime - so.last_command_time).get() / 1000);
				}
			}

			// if ship is waiting for command
			if (so.waiting_for_command_) {
				expedition_management(so);

				// Sometimes we look for other direction even if ship is still scouting,
				// escape mode here indicates that we are going over known ports, that means that last
				// port space we found when circumventing the island was already known to the ship.
				// Or(!) this is a island without a port and ship would sail around forever
			} else if ((so.escape_mode ||
			            (so.last_command_time + Duration(5 * 60 * 1000)) < gametime) &&
			           so.ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionScouting) {
				attempt_escape(so);
			}

			// Checking utilization
			if (so.ship->get_ship_state() == Widelands::Ship::ShipStates::kTransport) {
				// Good utilization is 10 pieces of ware onboard, to track utilization we use range
				// 0-10000
				// to avoid float or rounding errors if integers in range 0-100
				const int16_t tmp_util =
				   (so.ship->get_nritems() > 10) ? 10000 : so.ship->get_nritems() * 1000;
				// This number is kind of average
				persistent_data->ships_utilization =
				   persistent_data->ships_utilization * 19 / 20 + tmp_util / 20;

				// Arithmetics check
				assert(persistent_data->ships_utilization >= 0 &&
				       persistent_data->ships_utilization <= 10000);
			}
		}
	}

	// processing marine_task_queue
	while (!marine_task_queue.empty()) {
		if (marine_task_queue.back() == kStopShipyard) {
			// iterate over all production sites searching for shipyard
			for (const ProductionSiteObserver& observer : productionsites) {
				if (observer.bo->is(BuildingAttribute::kShipyard)) {
					if (!observer.site->is_stopped()) {
						game().send_player_start_stop_building(*observer.site);
					}
				}
			}
		}

		if (marine_task_queue.back() == kReprioritize) {
			for (const ProductionSiteObserver& observer : productionsites) {
				if (observer.bo->is(BuildingAttribute::kShipyard)) {
					for (uint32_t k = 0; k < observer.bo->inputs.size(); ++k) {
						game().send_player_set_ware_priority(*observer.site, Widelands::wwWARE,
						                                     observer.bo->inputs.at(k),
						                                     Widelands::WarePriority::kHigh);
					}
				}
			}
		}

		marine_task_queue.pop_back();
	}

	// If map_allows_seafaring_, we indicate that normal frequency check makes sense
	return map_allows_seafaring_;
}

/**
 * This is part of check_ships() function separated for readability
 */
void DefaultAI::check_ship_in_expedition(ShipObserver& so, const Time& gametime) {
	Widelands::PlayerNumber const pn = player_->player_number();

	// There is theoretical possibility that we have more than one ship in expedition mode,
	// and this one is not the one listed in expedition_ship_ variable, so we quit expedition of this
	// one
	if (expedition_ship_ != so.ship->serial() && expedition_ship_ != kNoShip) {
		verb_log_warn_time(
		   gametime,
		   "%d: WARNING: ship %s in expedition, but we have more then one in expedition mode and "
		   "this is not supported, cancelling the expedition\n",
		   pn, so.ship->get_shipname().c_str());
		game().send_player_cancel_expedition_ship(*so.ship);
		return;
	}

	// consistency check
	assert(expedition_ship_ == so.ship->serial() || expedition_ship_ == kNoShip);
	Duration expedition_time = gametime - persistent_data->expedition_start_time;

	// Obviously a new expedition
	if (expedition_ship_ == kNoShip) {
		assert(persistent_data->expedition_start_time ==
		       Widelands::Player::AiPersistentState::kNoExpedition);
		persistent_data->expedition_start_time = gametime;
		persistent_data->colony_scan_area =
		   Widelands::Player::AiPersistentState::kColonyScanStartArea;
		expedition_ship_ = so.ship->serial();

		// Expedition is overdue: cancel expedition, set no_more_expeditions = true
		// Also we attempt to cancel expedition (the code for cancellation may not work properly)
		// TODO(toptopple): - test expedition cancellation deeply (may need to be fixed)
	} else if (expedition_time >= expedition_max_duration) {
		assert(persistent_data->expedition_start_time > Time(0));
		persistent_data->colony_scan_area = Widelands::Player::AiPersistentState::kColonyScanMinArea;
		persistent_data->no_more_expeditions = true;
		game().send_player_cancel_expedition_ship(*so.ship);
		verb_log_dbg_time(gametime, "%d: %s at %3dx%3d: END OF EXPEDITION due to time-out\n", pn,
		                  so.ship->get_shipname().c_str(), so.ship->get_position().x,
		                  so.ship->get_position().y);

		// In case there is no port left to get back to, continue exploring
		if ((so.ship->get_fleet() == nullptr) || !so.ship->get_fleet()->has_ports()) {
			verb_log_dbg_time(
			   gametime, "%d: %s at %3dx%3d: END OF EXPEDITION without port, continue exploring\n", pn,
			   so.ship->get_shipname().c_str(), so.ship->get_position().x, so.ship->get_position().y);
			persistent_data->expedition_start_time = gametime;
			return;
		}

		// For known and running expedition
	} else {
		// set persistent_data->colony_scan_area based on elapsed expedition time
		assert(persistent_data->expedition_start_time >
		       Widelands::Player::AiPersistentState::kNoExpedition);
		assert(expedition_time < expedition_max_duration);

		// calculate percentage of remaining expedition time (range 0-100)
		const uint8_t remaining_time =
		   100 - ((gametime - persistent_data->expedition_start_time).get() /
		          (expedition_max_duration.get() / 100));
		assert(remaining_time <= 100);

		// calculate a new persistent_data->colony_scan_area
		const uint32_t expected_colony_scan =
		   Widelands::Player::AiPersistentState::kColonyScanMinArea +
		   (Widelands::Player::AiPersistentState::kColonyScanStartArea -
		    Widelands::Player::AiPersistentState::kColonyScanMinArea) *
		      remaining_time / 100;
		assert(expected_colony_scan >= Widelands::Player::AiPersistentState::kColonyScanMinArea &&
		       expected_colony_scan <= Widelands::Player::AiPersistentState::kColonyScanStartArea);
		persistent_data->colony_scan_area = expected_colony_scan;
	}
}

// this is called whenever we gain ownership of a Ship
void DefaultAI::gain_ship(Widelands::Ship& ship, NewShip type) {

	allships.emplace_back();
	allships.back().ship = &ship;
	allships.back().island_circ_direction = randomExploreDirection();

	if (type == NewShip::kBuilt) {
		marine_task_queue.push_back(kStopShipyard);
	} else {
		if (ship.state_is_expedition()) {
			if (expedition_ship_ == kNoShip) {
				// OK, this ship is in expedition
				expedition_ship_ = ship.serial();
			} else {
				// What? Another ship in expedition? AI is not able to manage two expedition ships...
				verb_log_warn_time(
				   game().get_gametime(),
				   " %d: AI will not control ship %s, as there is already another one in expedition\n",
				   player_number(), ship.get_shipname().c_str());
			}
		}
	}
}

Widelands::IslandExploreDirection DefaultAI::randomExploreDirection() {
	return RNG::static_rand(20) < 10 ? Widelands::IslandExploreDirection::kClockwise :
                                      Widelands::IslandExploreDirection::kCounterClockwise;
}

// this is called whenever ship received a notification that requires
// navigation decisions (these notifications are processes not in 'real time')
void DefaultAI::expedition_management(ShipObserver& so) {

	const Time& gametime = game().get_gametime();
	Widelands::PlayerNumber const pn = player_->player_number();

	// second we put current spot into expedition visited_spots
	bool first_time_here = expedition_visited_spots.count(so.ship->get_position().hash()) == 0;
	if (first_time_here) {
		expedition_visited_spots.insert(so.ship->get_position().hash());
		so.escape_mode = false;
	} else {
		so.escape_mode = true;
	}

	// if we have a port-space we can build a Port or continue exploring
	// 1. examine to build a port (colony founding)
	if (!so.ship->exp_port_spaces().empty()) {

		// we score the place (value max == 8)
		const uint8_t spot_score = spot_scoring(so.ship->exp_port_spaces().front()) * 2;
		verb_log_dbg_time(gametime, "%d: %s at %3dx%3d: PORTSPACE found, we valued it: %d\n", pn,
		                  so.ship->get_shipname().c_str(), so.ship->get_position().x,
		                  so.ship->get_position().y, spot_score);

		// we make a decision based on the score value and random
		if (RNG::static_rand(8) < spot_score) {
			// we build a port here
			game().send_player_ship_construct_port(*so.ship, so.ship->exp_port_spaces().front());
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;

			return;
		}
	}

	// 2. Go on with expedition
	// 2a) Ship is first time here
	if (first_time_here) {
		verb_log_dbg_time(gametime, "%d: %s at %3dx%3d: explore uphold, visited first time\n", pn,
		                  so.ship->get_shipname().c_str(), so.ship->get_position().x,
		                  so.ship->get_position().y);

		// Determine direction of island circle movement
		// Note: if the ship doesn't own an island-explore-direction it is in inter-island exploration
		// in this case we create a new direction at random, otherwise continue circle movement
		if (!so.ship->is_exploring_island()) {
			so.island_circ_direction = randomExploreDirection();
			verb_log_dbg_time(gametime, "%d: %s: new island exploration - direction: %u\n", pn,
			                  so.ship->get_shipname().c_str(),
			                  static_cast<uint32_t>(so.island_circ_direction));
		} else {
			verb_log_dbg_time(gametime, "%d: %s: continue island circumvention, dir=%u\n", pn,
			                  so.ship->get_shipname().c_str(),
			                  static_cast<uint32_t>(so.island_circ_direction));
		}

		// send the ship to circle island
		game().send_player_ship_explore_island(*so.ship, so.island_circ_direction);

		// 2b) We were here before, let try break for open sea
	} else {
		if (!attempt_escape(so)) {  // return true if the ship was sent to open sea
			// otherwise we continue circumnavigating the island
			game().send_player_ship_explore_island(*so.ship, so.island_circ_direction);
			verb_log_dbg_time(gametime, "%d: %s: in JAMMING spot, continue circumvention, dir=%u\n",
			                  pn, so.ship->get_shipname().c_str(),
			                  static_cast<uint32_t>(so.island_circ_direction));
		}
	}

	so.last_command_time = gametime;
	so.waiting_for_command_ = false;
}

// Here we investigate possibility to go for open sea, preferably to unexplored territories
bool DefaultAI::attempt_escape(ShipObserver& so) {

	const Widelands::Map& map = game().map();
	Widelands::PlayerNumber const pn = player_->player_number();

	// Determine swimmable directions first:
	// This vector contains directions that lead to unexplored sea
	static std::vector<Widelands::Direction> new_teritory_directions;
	new_teritory_directions.clear();
	new_teritory_directions.reserve(6);
	// This one contains any directions with open sea (superset of above one)
	static std::vector<Widelands::Direction> possible_directions;
	possible_directions.clear();
	possible_directions.reserve(6);
	for (Widelands::Direction dir = Widelands::FIRST_DIRECTION; dir <= Widelands::LAST_DIRECTION;
	     ++dir) {
		// testing distance of 30 fields (or as long as the sea goes, and until
		// unknown territory is reached)
		Widelands::Coords tmp_coords = so.ship->get_position();

		for (int8_t i = 0; i < 30; ++i) {
			map.get_neighbour(tmp_coords, dir, &tmp_coords);
			if ((map.get_fcoords(tmp_coords).field->nodecaps() & Widelands::MOVECAPS_SWIM) == 0) {
				break;
			}
			if (i <= 4) {  // Four fields from the ship is too close for "open sea"
				continue;
			}
			if (i == 5) {
				// If open sea goes at least 5 fields from the ship this is considerd a
				// candidate, but worse than directions in new_teritory_directions
				// Of course, this direction can be inserted also into new_teritory_directions
				// below
				possible_directions.push_back(dir);
			}
			if (player_->get_vision(map.get_index(tmp_coords, map.get_width())) ==
			    Widelands::VisibleState::kUnexplored) {
				// So this field was never seen before, the direction is inserted into
				// new_teritory_directions, and searching in this direction quits here
				new_teritory_directions.push_back(dir);
				break;
			}
		}
	}

	assert(possible_directions.size() >= new_teritory_directions.size());

	// If only open sea (no unexplored sea) is found, we don't always divert the ship
	if (new_teritory_directions.empty() && RNG::static_rand(100) < 80) {
		return false;
	}

	if (!possible_directions.empty() || !new_teritory_directions.empty()) {
		const Widelands::Direction direction =
		   !new_teritory_directions.empty() ?
            new_teritory_directions.at(RNG::static_rand(new_teritory_directions.size())) :
            possible_directions.at(RNG::static_rand(possible_directions.size()));
		game().send_player_ship_scouting_direction(
		   *so.ship, static_cast<Widelands::WalkingDir>(direction));

		verb_log_dbg_time(game().get_gametime(),
		                  "%d: %s: exploration - breaking for %s sea, dir=%u\n", pn,
		                  so.ship->get_shipname().c_str(),
		                  !new_teritory_directions.empty() ? "unexplored" : "free", direction);
		so.escape_mode = false;
		return true;  // we were successful
	}
	return false;
}
}  // namespace AI
