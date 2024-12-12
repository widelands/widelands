/*
 * Copyright (C) 2009-2024 by the Widelands Development Team
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
bool DefaultAI::marine_main_decisions(const Time& /* gametime */) {
	if (!game().map().allows_seafaring()) {
		for (const ProductionSiteObserver& sy_obs : shipyardsites) {
			// In very rare situation, we might have non-seafaring map but there is a shipyard
			verb_log_dbg_time(
			   game().get_gametime(),
			   "AI %u: we have a shipyard in a non seafaring economy, dismantling it...\n",
			   static_cast<unsigned>(player_number()));
			if (!sy_obs.site->get_economy(Widelands::wwWORKER)->warehouses().empty()) {
				game().send_player_dismantle(*sy_obs.site, true);
			} else {
				game().send_player_bulldoze(*sy_obs.site);
			}
		}

		// If non-seafaring economy, no sense to go on with this function
		return false;
	}

	// getting some base statistics
	player_ = game().get_player(player_number());
	ports_count = 0;
	ports_finished_count = 0;
	expeditions_in_prep = 0;
	expeditions_ready = 0;

	BuildingObserver& port_obs = get_building_observer(BuildingAttribute::kPort);
	ports_finished_count = port_obs.cnt_built;
	ports_count = ports_finished_count + port_obs.cnt_under_construction;
	// goes over all warehouses (these includes ports)
	for (const PortSiteObserver& p_obs : portsites) {
		if (const Widelands::PortDock* pd = p_obs.site->get_portdock()) {
			if (pd->expedition_started()) {
				++expeditions_in_prep;
			}
			if (pd->is_expedition_ready()) {
				++expeditions_ready;
			}
		}
	}

	evaluate_fleet();  // sets some more of this AI's globals for the other parts
	manage_shipyards();
	manage_ports();

	return true;
}

/**
 * Part of marine_main_decisions() to make decisions and set this AI's globals for the other parts
 */
void DefaultAI::evaluate_fleet() {
	const bool tradeship_surplus_needed =
	   tradeships_count >= ports_count && persistent_data->ships_utilization > 5000;

	const uint32_t min_tradeships = (ports_count + 1) / kPortsPerTradeShip;
	const uint32_t tradeships_target = tradeship_surplus_needed ? tradeships_count + 1 : ports_count;

	// Make sure we get a free ship for an expedition or a warship
	const uint32_t new_fleet_target = std::max(tradeships_target, min_tradeships + 1);

	const bool ship_free = tradeships_count > min_tradeships;
	const bool ships_full = tradeships_count >= tradeships_target;

	const uint32_t warships_target = ports_finished_count * kWarshipsPerPort;
	const bool warship_shortage =
	   game().naval_warfare_allowed() && (warships_target > warships_count);

	const bool consider_expedition =
	   ports_count > 0 && !persistent_data->no_more_expeditions &&
	   (expeditions_in_progress + expeditions_in_prep + expeditions_ready) == 0 &&
	   basic_economy_established;

	// Help the AI get out of small starting islands
	const bool prioritise_expedition = ports_count < 2;

	// Set this AI's global variables
	if (tradeship_surplus_needed) {
		// Don't increase fleet_target for heavy utilization until old value is reached
		if (tradeships_count >= fleet_target) {
			fleet_target = new_fleet_target;
		}
	} else {
		fleet_target = new_fleet_target;
	}

	if (ship_free) {
		tradeship_refit_needed = false;
		if (prioritise_expedition) {
			start_expedition = consider_expedition;

			const uint32_t free_ships_count = tradeships_count - min_tradeships;

			// We also need to keep a ship ready for previously ordered expeditions
			const bool allow_warship =
			   (!start_expedition && (expeditions_in_prep + expeditions_ready) == 0) ||
			   free_ships_count > 1;

			warship_needed = allow_warship && warship_shortage;
		} else {
			warship_needed = warship_shortage;
			start_expedition = !warship_needed && consider_expedition &&
			                   (ships_full || persistent_data->ships_utilization < 2000);
		}
	} else {  // no free ships
		start_expedition = false;
		warship_needed = false;
		tradeship_refit_needed = (tradeships_count < min_tradeships) && (warships_count > 0);
	}

	if (tradeship_refit_needed) {
		// This shouldn't happen... except when a new port is built from land
		verb_log_dbg_time(game().get_gametime(),
		                  "AI %u backfit needed: %u ports, %" PRIuS
		                  " ships total: %u expeditions, %u tradeships, %u warships",
		                  static_cast<unsigned>(player_number()), ports_count, allships.size(),
		                  expeditions_in_progress, tradeships_count, warships_count);
	}
}

/**
 * Part of marine_main_decisions() to handle shipyards: set target / start / stop / dismantle
 */
void DefaultAI::manage_shipyards() {
	Widelands::ShipFleet* fleet = get_main_fleet();

	bool update_ships_target = fleet != nullptr && fleet->get_ships_target() != fleet_target;

	for (const ProductionSiteObserver& sy_obs : shipyardsites) {

		// Verify whether only the correct fleet is reachable
		auto yard_interfaces = sy_obs.site->get_ship_fleet_interfaces();
		bool dismantle = yard_interfaces.empty();  // wrong place?
		// TODO(tothxa): When the AI is made to handle multiple fleets, this needs to be
		//    changed to only dismantle if it has interfaces to more than one fleet.
		if (fleet != nullptr) {
			for (const Widelands::ShipFleetYardInterface* yard_if : yard_interfaces) {
				if (yard_if->get_fleet()->serial() != fleet->serial()) {
					dismantle = true;
					break;
				}
			}
		}
		if (dismantle) {
			verb_log_dbg_time(game().get_gametime(), "AI %u: Dismantling shipyard in second fleet",
			                  static_cast<unsigned>(player_number()));
			if (!sy_obs.site->get_economy(Widelands::wwWORKER)->warehouses().empty()) {
				game().send_player_dismantle(*sy_obs.site, true);
			} else {
				game().send_player_bulldoze(*sy_obs.site);
			}
			continue;
		}
		if (fleet == nullptr) {
			assert(ports_finished_count == 0);
			Widelands::ShipFleet* yard_fleet = yard_interfaces.front()->get_fleet();

			// Set target to avoid building too many ships before the port is ready, because
			// the default is infinite ships. It may even turn out that we're building a
			// fleet for a different ocean that will have to be sunk.
			// TODO(tothxa): May need update when the AI is made to handle multiple fleets.
			update_ships_target =
			   yard_fleet != nullptr && yard_fleet->get_ships_target() != fleet_target;
		}

		if (update_ships_target) {
			verb_log_dbg_time(game().get_gametime(), "AI %u: setting ships target to %u",
			                  static_cast<unsigned>(player_number()), fleet_target);
			game().send_player_fleet_targets(
			   player_number(), yard_interfaces.front()->serial(), fleet_target);
			update_ships_target = false;
		}

		const bool stopped = sy_obs.site->is_stopped();

		if (basic_economy_established) {
			// checking stocks
			bool shipyard_stocked = true;
			std::vector<Widelands::InputQueue*> const inputqueues = sy_obs.site->inputqueues();
			for (Widelands::InputQueue* queue : inputqueues) {
				if (queue->get_type() == Widelands::wwWARE) {
					if (!stopped && sy_obs.site->get_priority(Widelands::wwWARE, queue->get_index()) !=
					                   Widelands::WarePriority::kHigh) {
						game().send_player_set_ware_priority(*sy_obs.site, Widelands::wwWARE,
						                                     queue->get_index(),
						                                     Widelands::WarePriority::kHigh);
					}
					if (!stopped && queue->get_max_fill() < queue->get_max_size()) {
						game().send_player_set_input_max_fill(
						   *sy_obs.site, queue->get_index(), Widelands::wwWARE, queue->get_max_size());
						shipyard_stocked = false;
					} else if (queue->get_missing() > (stopped ? 0 : queue->get_max_size() / 3)) {
						shipyard_stocked = false;
					}
				}
			}
			if (ports_count > 0 && shipyard_stocked && stopped && sy_obs.site->can_start_working()) {
				verb_log_dbg_time(game().get_gametime(), "AI %u: Starting shipyard.",
				                  static_cast<unsigned>(player_number()));
				game().send_player_start_stop_building(*sy_obs.site);
			} else if (!stopped && (!shipyard_stocked || ports_count == 0)) {
				verb_log_dbg_time(game().get_gametime(), "AI %u: Stopping shipyard %s.",
				                  static_cast<unsigned>(player_number()),
				                  (ports_count == 0) ? "without port" : "with poor supply");
				game().send_player_start_stop_building(*sy_obs.site);
			}
		} else {  // basic economy not established
			verb_log_warn_time(game().get_gametime(), "AI %u: Shipyard found in weak economy!",
			                   static_cast<unsigned>(player_number()));
			// give back all wares and stop
			for (uint32_t k = 0; k < sy_obs.bo->inputs.size(); ++k) {
				game().send_player_set_input_max_fill(
				   *sy_obs.site, sy_obs.bo->inputs.at(k), Widelands::wwWARE, 0);
			}
			if (!stopped) {
				game().send_player_start_stop_building(*sy_obs.site);
			}
		}
	}
}

/**
 * Part of marine_main_decisions() to handle ports: start expedition and set garrison
 */
void DefaultAI::manage_ports() {
	Widelands::ShipFleet* fleet = get_main_fleet();

	for (PortSiteObserver& p_obs : portsites) {
		if (p_obs.site == nullptr) {
			// Race condition?
			log_warn_time(game().get_gametime(), "AI %u: manage_ports(): Invalid port",
			              static_cast<unsigned>(player_number()));
			continue;
		}

		if (fleet != nullptr && p_obs.site->get_portdock() != nullptr &&
		    p_obs.site->get_portdock()->get_fleet() != nullptr &&
		    p_obs.site->get_portdock()->get_fleet()->serial() != fleet->serial()) {
			// Only allow one fleet for now.
			// TODO(tothxa): v1.3: Make the AI handle multiple fleets.
			// TODO(tothxa): In building decisions:
			//                * When evaluating portspaces for building additional ports, check whether
			//                  portsites.front() is reachable. (or when to start new fleet when it's
			//                  implemented)
			//                * Be more careful with placing buildings near portspaces. AIs often fail
			//                  to escape from starting island if it has only one portspace.
			verb_log_dbg_time(game().get_gametime(), "AI %u: Dismantling port %s in second fleet",
			                  static_cast<unsigned>(player_number()),
			                  p_obs.site->get_warehouse_name().c_str());
			game().send_player_dismantle(*p_obs.site, true);
			continue;
		}

		if (start_expedition) {
			verb_log_dbg_time(game().get_gametime(),
			                  "  %1d: Starting preparation for expedition in port at %3dx%3d\n",
			                  player_number(), p_obs.site->get_position().x,
			                  p_obs.site->get_position().y);
			game().send_player_start_or_cancel_expedition(*p_obs.site);
			start_expedition = false;
		}

		const Widelands::Quantity current_garrison = p_obs.site->get_desired_soldier_count();
		const bool full =
		   p_obs.site->soldier_control()->associated_soldiers().size() >= current_garrison;

		Widelands::Quantity desired_garrison = kPortDefaultGarrison;
		int32_t change_value = 0;

		switch (soldier_status_) {
		case SoldiersStatus::kBadShortage:
			// reduce minimum to allow garrison of some milsites to ensure expansion
			desired_garrison = 2;
			if (current_garrison > desired_garrison) {
				change_value = -1;
			}
			break;
		case SoldiersStatus::kShortage:
			desired_garrison = kPortDefaultGarrison;
			if (current_garrison > desired_garrison) {
				change_value = -1;
			}
			break;
		case SoldiersStatus::kEnough:
			desired_garrison = kPortDefaultGarrison * 2;
			if (full && current_garrison < desired_garrison) {
				change_value = 1;
			}
			break;
		case SoldiersStatus::kFull:
			desired_garrison = kPortDefaultGarrison * 3;
			if (full && current_garrison < desired_garrison) {
				change_value = 1;
			}
			break;
		default:
			NEVER_HERE();
		}

		// Check soldiers requirement of port and set garrison to desired value
		if (change_value != 0) {
			// ports should always require Heroes
			game().send_player_set_soldier_preference(
			   *p_obs.site, Widelands::SoldierPreference::kHeroes);
			verb_log_dbg_time(game().get_gametime(),
			                  "AI %u: Set garrison for port %s, desired garrison %u, actual garrison "
			                  "%u, change value %d\n",
			                  static_cast<unsigned>(player_number()),
			                  p_obs.site->get_warehouse_name().c_str(), desired_garrison,
			                  p_obs.site->get_desired_soldier_count(), change_value);
			game().send_player_change_soldier_capacity(*p_obs.site, change_value);
		}
		// Warships assign themselves
	}
}

// This identifies ships that are waiting for command
bool DefaultAI::check_ships(const Time& gametime) {
	// There is possibility that the map is not seafaring but we still have ships and/or shipyards
	if (!game().map().allows_seafaring()) {
		for (ShipObserver& so : allships) {
			// Sink ships if we can't use them.
			verb_log_dbg_time(game().get_gametime(),
			                  "AI %u: Sinking unexpected ship %s on non-seafaring map.",
			                  static_cast<unsigned>(player_number()), so.ship->get_shipname().c_str());
			game().send_player_sink_ship(*so.ship);
		}

		// False indicates that we can postpone next call of this function
		return false;
	}

	expeditions_in_progress = 0;
	warships_count = 0;
	tradeships_count = 0;

	Widelands::ShipFleet* fleet = get_main_fleet();

	// iterating over ships and doing what is needed
	for (ShipObserver& so : allships) {

		if (so.ship == nullptr) {  // good old paranoia
			log_warn_time(game().get_gametime(), "AI %u: check_ships(): Invalid ship",
			              static_cast<unsigned>(player_number()));
			continue;
		}

		// Sink if not in right fleet.
		// TODO(tothxa): Make the AI handle multiple fleets. Then this can be removed.
		if (fleet != nullptr && so.ship->get_fleet() != nullptr &&
		    so.ship->get_fleet()->serial() != fleet->serial()) {
			verb_log_dbg_time(game().get_gametime(), "AI %u: Sinking ship %s in second fleet",
			                  static_cast<unsigned>(player_number()), so.ship->get_shipname().c_str());
			game().send_player_sink_ship(*so.ship);
			continue;
		}

		if (so.ship->get_ship_type() == Widelands::ShipType::kWarship) {
			if (!so.ship->is_refitting()) {
				++warships_count;
			} else {
				++tradeships_count;
			}
		} else {
			if (!so.ship->is_refitting()) {
				if (warship_needed && !so.ship->state_is_expedition()) {
					verb_log_dbg_time(gametime, "AI %u: Refit ship %s to warship",
					                  static_cast<unsigned>(player_number()),
					                  so.ship->get_shipname().c_str());
					game().send_player_refit_ship(*so.ship, Widelands::ShipType::kWarship);
					// transport ships remember soldier capacity
					if (so.ship->get_warship_soldier_capacity() > 0) {
						game().send_player_warship_command(
						   *so.ship, Widelands::WarshipCommand::kSetCapacity, {0u});
					}
					warship_needed = false;
					++warships_count;
				} else {
					++tradeships_count;
				}
			} else {
				++warships_count;
			}
		}
		const Widelands::ShipStates ship_state = so.ship->get_ship_state();

		// Here we manage duration of expedition and related variables
		if (so.ship->get_ship_type() == Widelands::ShipType::kTransport &&
		    (ship_state == Widelands::ShipStates::kExpeditionWaiting ||
		     ship_state == Widelands::ShipStates::kExpeditionScouting ||
		     ship_state == Widelands::ShipStates::kExpeditionPortspaceFound)) {

			// the function below will take care of variables like
			// - expedition_ship_
			// - expedition_start_time
			// - expected_colony_scan
			// - no_more_expeditions_
			check_ship_in_expedition(so, gametime);
			++expeditions_in_progress;
			assert(tradeships_count > 0);
			--tradeships_count;

			// We are not in expedition mode (or perhaps building a colonisation port)
			// so resetting start time
		} else if (expedition_ship_ == so.ship->serial()) {
			// Obviously expedition just ended
			persistent_data->expedition_start_time =
			   Widelands::Player::AiPersistentState::kNoExpedition;
			expedition_ship_ = kNoShip;
		}

		// only two states need an attention
		if ((so.ship->get_ship_state() == Widelands::ShipStates::kExpeditionWaiting ||
		     so.ship->get_ship_state() == Widelands::ShipStates::kExpeditionPortspaceFound) &&
		    !so.waiting_for_command_) {
			if (gametime - so.last_command_time > Duration(180 * 1000)) {
				so.waiting_for_command_ = true;
				if (!so.guarding) {
					verb_log_warn_time(
					   gametime,
					   "AI %1u: last command for ship %s at %3dx%3d was %3u seconds ago, something "
					   "wrong here?...\n",
					   player_number(), so.ship->get_shipname().c_str(), so.ship->get_position().x,
					   so.ship->get_position().y, (gametime - so.last_command_time).get() / 1000);
				}
			}
		}

		// if ship is waiting for command
		if (so.waiting_for_command_) {
			if (so.ship->get_ship_type() == Widelands::ShipType::kTransport) {
				// so.waiting_for_command_ can still be true right after a warship is backfitted to
				// transport.
				if (so.ship->is_expedition_or_warship()) {
					expedition_management(so);
				} else {
					so.waiting_for_command_ = false;
				}
			} else {
				warship_management(so);
			}

			// Sometimes we look for other direction even if ship is still scouting,
			// escape mode here indicates that we are going over known ports, that means that last
			// port space we found when circumventing the island was already known to the ship.
			// Or(!) this is a island without a port and ship would sail around forever
		} else if ((so.escape_mode || (so.last_command_time + Duration(5 * 60 * 1000)) < gametime) &&
		           so.ship->get_ship_state() == Widelands::ShipStates::kExpeditionScouting) {
			attempt_escape(so);

		} else if (so.ship->get_ship_state() == Widelands::ShipStates::kTransport) {
			// Checking utilization

			// Good utilization is 10 pieces of ware onboard, to track utilization we use range
			// 0-10000 to avoid float or rounding errors if integers in range 0-100
			const uint32_t tmp_util = std::min(so.ship->get_nritems(), 10U) * 1000;

			// persistent_data->ships_utilization is uint16_t, and saved in player data in
			// savegames. Let's just work around the limitation, then convert back.
			uint32_t average_util = persistent_data->ships_utilization;
			average_util = (average_util * 19 + tmp_util) / 20;

			// Sanity checks
			if ((persistent_data->ships_utilization == tmp_util && average_util != tmp_util) ||
			    // std::min/max is picky about integer size...
			    (average_util < persistent_data->ships_utilization && average_util < tmp_util) ||
			    (average_util > persistent_data->ships_utilization && average_util > tmp_util) ||
			    // ..._util < 0 is prevented by uint type
			    tmp_util > 10000 || average_util > 10000) {
				log_warn_time(gametime,
				              "AI %u: Ship utilisation calculation error: old: %u current: %u new: %u",
				              static_cast<unsigned>(player_number()),
				              persistent_data->ships_utilization, tmp_util, average_util);
			}

			// Now we can safely assign new value back to uint16_t
			persistent_data->ships_utilization = average_util;
		}
	}

	// Check accounting
	const uint32_t counted_ships = tradeships_count + expeditions_in_progress + warships_count;
	if (counted_ships != allships.size()) {
		const bool counted_more = counted_ships > allships.size();
		// no casting
		const uint32_t diff =
		   counted_more ? counted_ships - allships.size() : allships.size() - counted_ships;
		log_warn_time(gametime,
		              "AI %u check_ships(): Accounting error: allships = %" PRIuS
		              " %s counted = %u (%u tradeships, %u expeditions, %u warships), difference: %u",
		              static_cast<unsigned>(player_number()), allships.size(),
		              counted_more ? "<" : ">", counted_ships, tradeships_count,
		              expeditions_in_progress, warships_count, diff);
	}

	return true;
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

	if (type != NewShip::kBuilt) {
		if (ship.get_ship_type() == Widelands::ShipType::kTransport && ship.state_is_expedition()) {
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

// this is called whenever a transport ship received a notification that requires
// navigation decisions (these notifications are processes not in 'real time')
void DefaultAI::expedition_management(ShipObserver& so) {

	const Time& gametime = game().get_gametime();
	Widelands::PlayerNumber const pn = player_->player_number();

	// We will dereference this all over this function. Let's check at least once.
	if (so.ship == nullptr) {  // Should only happen in race conditions.
		log_warn_time(gametime, "AI %u: expedition_management(): Invalid expedition ship.",
		              static_cast<unsigned>(pn));
		return;
	}

	BuildingObserver& port_obs = get_building_observer(BuildingAttribute::kPort);
	// Check whether a port is allowed to help the AI with "New World" start condition
	bool port_allowed =
	   basic_economy_established || port_obs.cnt_built + port_obs.cnt_under_construction < 1;
	if (!port_allowed) {
		game().send_player_cancel_expedition_ship(*so.ship);
		return;
	}

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
	const Widelands::Coords portspace = so.ship->current_portspace();
	if (portspace.valid()) {

		// we score the place (value max == 8)
		const uint8_t spot_score = spot_scoring(portspace) * 2;
		verb_log_dbg_time(gametime, "%u: %s at %3dx%3d: PORTSPACE found, we valued it: %d\n",
		                  static_cast<unsigned>(pn), so.ship->get_shipname().c_str(),
		                  so.ship->get_position().x, so.ship->get_position().y, spot_score);

		// we make a decision based on the score value and random and basic economy status
		if (RNG::static_rand(8) < spot_score) {
			// we build a port here
			game().send_player_ship_construct_port(*so.ship, portspace);
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

// this is called whenever a warship received a notification that requires
// navigation decisions (these notifications are processes not in 'real time')
void DefaultAI::warship_management(ShipObserver& so) {
	const Time& gametime = game().get_gametime();

	// Should have been set on refit, but it will need updating here when it gets dynamic
	if (so.ship->get_warship_soldier_capacity() > 0) {
		game().send_player_warship_command(*so.ship, Widelands::WarshipCommand::kSetCapacity, {0u});
	}

	if (!so.guarding) {

		if (tradeship_refit_needed) {
			// Let's keep this simple. (for now?)
			// We only backfit if we find an idle warship. Trade ships shouldn't disappear anyway, so
			// backfit requests should be extremely rare.

			verb_log_dbg_time(gametime, "AI %u: Refit warship %s to trade ship",
			                  static_cast<unsigned>(player_->player_number()),
			                  so.ship->get_shipname().c_str());
			game().send_player_refit_ship(*so.ship, Widelands::ShipType::kTransport);
			tradeship_refit_needed = false;
			--warships_count;
			++tradeships_count;
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;
			return;
		}

		verb_log_dbg_time(gametime, "AI %u: Warship %s is looking for assignment.",
		                  static_cast<unsigned>(player_->player_number()),
		                  so.ship->get_shipname().c_str());
		PortSiteObserver* picked_port = nullptr;
		Widelands::Quantity picked_port_guard_ships = kWarshipsPerPort + 1;
		// TODO(tothxa): Should also consider distance to avoid unnecessary travel and
		//               reshuffling on game loading.

		for (PortSiteObserver& p_obs : portsites) {
			verb_log_dbg_time(gametime, "AI %u: Port %s has %u ships assigned",
			                  static_cast<unsigned>(player_->player_number()),
			                  p_obs.site->get_warehouse_name().c_str(), p_obs.ships_assigned);
			if (p_obs.ships_assigned < picked_port_guard_ships) {
				picked_port = &p_obs;
				picked_port_guard_ships = p_obs.ships_assigned;
				if (picked_port_guard_ships == 0) {
					break;
				}
			}
		}

		if (picked_port != nullptr && picked_port_guard_ships < kWarshipsPerPort) {
			assert(picked_port_guard_ships == picked_port->ships_assigned);

			verb_log_dbg_time(gametime, "AI %u: Assigning warship %s to port %s",
			                  static_cast<unsigned>(player_->player_number()),
			                  so.ship->get_shipname().c_str(),
			                  picked_port->site->get_warehouse_name().c_str());
			game().send_player_ship_set_destination(*so.ship, picked_port->site->get_portdock());
			++(picked_port->ships_assigned);
			so.guarding = true;
		} else {
#ifndef NDEBUG
			if (picked_port == nullptr) {
				assert(portsites.empty());
			} else {
				assert(picked_port_guard_ships == kWarshipsPerPort);
				assert(picked_port_guard_ships == picked_port->ships_assigned);
			}
#endif
			if (portsites.empty()) {
				log_dbg_time(gametime, "AI %u: no more ports left, no action for warship %s",
				             static_cast<unsigned>(player_->player_number()),
				             so.ship->get_shipname().c_str());
			} else if (so.ship->has_destination()) {
				log_dbg_time(
				   gametime, "AI %u: all ports have enough guard ships, no action for warship %s",
				   static_cast<unsigned>(player_->player_number()), so.ship->get_shipname().c_str());
			} else {
				// `portsites.front()` is likely the oldest port with the most infrastructure. (or not)
				// TODO(tothxa): But the whole guarding ship business needs proper AI handling.
				log_dbg_time(
				   gametime, "AI %u: all ports have enough guard ships, warship %s retreats to %s",
				   static_cast<unsigned>(player_->player_number()), so.ship->get_shipname().c_str(),
				   portsites.front().site->get_warehouse_name().c_str());
				game().send_player_ship_set_destination(
				   *so.ship, portsites.front().site->get_portdock());
				// Not an assignment, so not setting `so.guarding`, and not increasing
				// `portsites.front().ships_assigned`!
			}
		}

	} else if (so.ship->get_destination_port(game()) == nullptr) {
		// Clearing the destination when it disappears (is destroyed) is handled by the ship logic,
		// but we have to do our own accounting.
		verb_log_dbg_time(gametime, "AI %u: Warship %s lost guarded port",
		                  static_cast<unsigned>(player_->player_number()),
		                  so.ship->get_shipname().c_str());
		so.guarding = false;

		// We want to assign new port as soon as possible (no-op actually)
		so.waiting_for_command_ = true;
		return;
	}

	// Normally we don't want to check again too soon
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

// We need to fetch this fresh all the time unfortunately, because fleets seem to come and go as
// new ones are created for each new ship and port, then merged with the old ones.
Widelands::ShipFleet* DefaultAI::get_main_fleet() {
	if (portsites.empty() || portsites.front().site == nullptr ||
	    portsites.front().site->get_portdock() == nullptr) {
		return nullptr;
	}
	return portsites.front().site->get_portdock()->get_fleet();
}
}  // namespace AI
