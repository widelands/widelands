/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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

#include "ai/defaultai.h"

using namespace Widelands;

// this scores spot for potential colony
uint8_t DefaultAI::spot_scoring(Widelands::Coords candidate_spot) {

	Map& map = game().map();
	uint8_t score = 0;
	uint16_t mineable_fields_count = 0;
	uint32_t tested_fields = 0;

	// abort if any player - including self - is too near to the spot (radius 8)
	// TODO(toptopple): extract function for player distance only
	if (other_player_accessible(
	       8, &tested_fields, &mineable_fields_count, candidate_spot, WalkSearch::kAnyPlayer)) {
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
		score += 1;
	}

	// here we check for surface rocks + trees
	std::vector<ImmovableFound> immovables;
	// Search in a radius of range
	map.find_immovables(Area<FCoords>(map.get_fcoords(candidate_spot), 10), &immovables);

	int32_t const rocks_attr = MapObjectDescr::get_attribute_id("rocks");
	uint16_t rocks = 0;
	int32_t const tree_attr = MapObjectDescr::get_attribute_id("tree");
	uint16_t trees = 0;

	for (uint32_t j = 0; j < immovables.size(); ++j) {
		if (immovables.at(j).object->has_attribute(rocks_attr)) {
			++rocks;
		}
		if (immovables.at(j).object->has_attribute(tree_attr)) {
			++trees;
		}
	}
	if (rocks > 1) {
		score += 1;
	}
	if (trees > 1) {
		score += 1;
	}

	return score;
}

// This function scans current situation with shipyards, ports, ships, ongoing expeditions
// and makes two decisions:
// - build a ship
// - start preparation for expedition
bool DefaultAI::marine_main_decisions() {

	if (!seafaring_economy) {
		set_taskpool_task_time(kNever, SchedulerTaskId::KMarineDecisions);
		return false;
	}

	// getting some base statistics
	player_ = game().get_player(player_number());
	uint16_t ports_count = 0;
	uint16_t shipyards_count = 0;
	uint16_t expeditions_in_prep = 0;
	uint16_t expeditions_in_progress = 0;
	bool idle_shipyard_stocked = false;

	// goes over all warehouses (these includes ports)
	for (const WarehouseSiteObserver& wh_obs : warehousesites) {
		if (wh_obs.bo->is_port) {
			ports_count += 1;
			if (Widelands::PortDock* pd = wh_obs.site->get_portdock()) {
				if (pd->expedition_started()) {
					expeditions_in_prep += 1;
				}
			}
		}
	}

	// goes over productionsites and gets status of shipyards
	for (const ProductionSiteObserver& ps_obs : productionsites) {
		if (ps_obs.bo->is_shipyard) {
			shipyards_count += 1;

			// counting stocks
			uint8_t stocked_wares = 0;
			std::vector<InputQueue*> const inputqueues = ps_obs.site->inputqueues();
			for (InputQueue *queue : inputqueues) {
				if (queue->get_type() == wwWARE) {
					stocked_wares += queue->get_filled();
				}
			}
			if (stocked_wares == 16 && ps_obs.site->is_stopped() && ps_obs.site->can_start_working()) {
				idle_shipyard_stocked = true;
			}
		}
	}

	// and now over ships
	for (std::list<ShipObserver>::iterator sp_iter = allships.begin(); sp_iter != allships.end();
	     ++sp_iter) {
		if (sp_iter->ship->state_is_expedition()) {
			expeditions_in_progress += 1;
		}
	}

	assert(allships.size() >= expeditions_in_progress);
	bool ship_free = allships.size() - expeditions_in_progress > 0;

	enum class FleetStatus : uint8_t { kNeedShip = 0, kEnoughShips = 1, kDoNothing = 2 };

	// now we decide whether we have enough ships or need to build another
	// three values: kDoNothing, kNeedShip, kEnoughShips
	FleetStatus enough_ships = FleetStatus::kDoNothing;
	if (ports_count > 0 && shipyards_count > 0 && idle_shipyard_stocked) {

		// we always need at least one ship in transport mode
		if (!ship_free) {
			enough_ships = FleetStatus::kNeedShip;

			// we want at least as many free ships as we have ports
		} else if (int(allships.size()) - ports_count - expeditions_in_progress < 0) {
			enough_ships = FleetStatus::kNeedShip;

			// if ships utilization is too high
		} else if (persistent_data->ships_utilization > 5000) {
			enough_ships = FleetStatus::kNeedShip;

		} else {
			enough_ships = FleetStatus::kEnoughShips;
		}
	}

	// building a ship? if yes, find a shipyard and order it to build a ship
	if (enough_ships == FleetStatus::kNeedShip) {

		for (const ProductionSiteObserver& ps_obs : productionsites) {
			if (ps_obs.bo->is_shipyard && ps_obs.site->can_start_working() &&
			    ps_obs.site->is_stopped()) {
				// make sure it is fully stocked
				// counting stocks
				uint8_t stocked_wares = 0;
				std::vector<InputQueue*> const inputqueues = ps_obs.site->inputqueues();
				for (InputQueue *queue : inputqueues) {
					if (queue->get_type() == wwWARE) {
						stocked_wares += queue->get_filled();
					}
				}
				if (stocked_wares < 16) {
					continue;
				}

				game().send_player_start_stop_building(*ps_obs.site);
				return true;
			}
		}
	}

	// starting an expedition? if yes, find a port and order it to start an expedition
	if (ports_count > 0 && expeditions_in_progress == 0 && expeditions_in_prep == 0 &&
	    persistent_data->no_more_expeditions == kFalse && ship_free) {

		// we need to find a port
		for (const WarehouseSiteObserver& wh_obs : warehousesites) {
			if (wh_obs.bo->is_port) {
				game().send_player_start_or_cancel_expedition(*wh_obs.site);
				return true;
			}
		}
	}
	return true;
}

// This identifies ships that are waiting for command
bool DefaultAI::check_ships(uint32_t const gametime) {

	if (!seafaring_economy) {
		set_taskpool_task_time(std::numeric_limits<int32_t>::max(), SchedulerTaskId::kCheckShips);
		return false;
	}

	bool action_taken = false;

	if (!allships.empty()) {
		// iterating over ships and doing what is needed
		for (std::list<ShipObserver>::iterator i = allships.begin(); i != allships.end(); ++i) {

			const Widelands::Ship::ShipStates ship_state = i->ship->get_ship_state();

			// Here we manage duration of expedition and related variables
			if (ship_state == Widelands::Ship::ShipStates::kExpeditionWaiting ||
			    ship_state == Widelands::Ship::ShipStates::kExpeditionScouting ||
			    ship_state == Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {

				// the function below will take care of variables like
				// - expedition_ship_
				// - expedition_start_time
				// - expected_colony_scan
				// - no_more_expeditions_
				check_ship_in_expedition(*i, gametime);

				// We are not in expedition mode (or perhaps building a colonisation port)
				// so resetting start time
			} else if (expedition_ship_ == i->ship->serial()) {
				// Obviously expedition just ended
				persistent_data->expedition_start_time = kNoExpedition;
				expedition_ship_ = kNoShip;
			}

			// only two states need an attention
			if ((i->ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionWaiting ||
			     i->ship->get_ship_state() ==
			        Widelands::Ship::ShipStates::kExpeditionPortspaceFound) &&
			    !i->waiting_for_command_) {
				if (gametime - i->last_command_time > 180 * 1000) {
					i->waiting_for_command_ = true;
					log("  %1d: last command for ship at %3dx%3d was %3d seconds ago, something wrong "
					    "here?...\n",
					    player_number(), i->ship->get_position().x, i->ship->get_position().y,
					    (gametime - i->last_command_time) / 1000);
				}
			}
			// if ships is waiting for command
			if (i->waiting_for_command_) {
				expedition_management(*i);
				action_taken = true;
			}

			// Checking utilization
			if (i->ship->get_ship_state() == Widelands::Ship::ShipStates::kTransport) {
				// Good utilization is 10 pieces of ware onboard, to track utilization we use range
				// 0-10000
				// to avoid float or rounding errors if integers in range 0-100
				const int16_t tmp_util =
				   (i->ship->get_nritems() > 10) ? 10000 : i->ship->get_nritems() * 1000;
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
			for (std::list<ProductionSiteObserver>::iterator site = productionsites.begin();
			     site != productionsites.end(); ++site) {
				if (site->bo->is_shipyard) {
					if (!site->site->is_stopped()) {
						game().send_player_start_stop_building(*site->site);
					}
				}
			}
		}

		if (marine_task_queue.back() == kReprioritize) {
			for (std::list<ProductionSiteObserver>::iterator site = productionsites.begin();
			     site != productionsites.end(); ++site) {
				if (site->bo->is_shipyard) {
					for (uint32_t k = 0; k < site->bo->inputs.size(); ++k) {
						game().send_player_set_ware_priority(
						   *site->site, wwWARE, site->bo->inputs.at(k), HIGH_PRIORITY);
					}
				}
			}
		}

		marine_task_queue.pop_back();
	}

	if (action_taken) {
		set_taskpool_task_time(gametime + kShipCheckInterval, SchedulerTaskId::kCheckShips);
	}
	return true;
}

/**
 * This is part of check_ships() function separated due to readibility purpuses
 */
void DefaultAI::check_ship_in_expedition(ShipObserver& so, uint32_t const gametime) {
	// consistency check
	assert(expedition_ship_ == so.ship->serial() || expedition_ship_ == kNoShip);

	// This is obviously new expedition
	if (expedition_ship_ == kNoShip) {
		assert(persistent_data->expedition_start_time == kNoExpedition);
		persistent_data->expedition_start_time = gametime;
		expedition_ship_ = so.ship->serial();

		// Already known expedition, all we do now, is decreasing persistent_data->colony_scan_area
		// based on lapsed time
	} else if (gametime - persistent_data->expedition_start_time < kExpeditionMaxDuration) {
		assert(persistent_data->expedition_start_time > kNoExpedition);
		// remaining_time is a percent so in range 0-100
		const uint32_t remaining_time = 100 - ((gametime - persistent_data->expedition_start_time) /
		                                       (kExpeditionMaxDuration / 100));
		assert(remaining_time <= 100);

		// We calculate expected value and actual value (persistent_data->colony_scan_area
		// is changed only when needed)
		const uint32_t expected_colony_scan =
		   kColonyScanMinArea + (kColonyScanStartArea - kColonyScanMinArea) * remaining_time / 100;
		assert(expected_colony_scan >= kColonyScanMinArea &&
		       expected_colony_scan <= kColonyScanStartArea);

		// So changing it if needed
		if (expected_colony_scan < persistent_data->colony_scan_area) {
			persistent_data->colony_scan_area = expected_colony_scan;
		}

		// Expedition overdue. Setting no_more_expeditions = true
		// But we do not cancel it, the code for cancellation does not work properly now
		// TODO(unknown): - expedition code for cancellation needs to be fixed and afterwareds
		// AI can be changed to cancel overdue expedition
	} else if (gametime - persistent_data->expedition_start_time >= kExpeditionMaxDuration) {
		assert(persistent_data->expedition_start_time > 0);
		persistent_data->colony_scan_area = kColonyScanMinArea;
		persistent_data->no_more_expeditions = kTrue;

		game().send_player_cancel_expedition_ship(*so.ship);
	}
}

// this is called whenever we gain ownership of a Ship
void DefaultAI::gain_ship(Ship& ship, NewShip type) {

	allships.push_back(ShipObserver());
	allships.back().ship = &ship;
	if (game().logic_rand() % 20 < 10) {
		allships.back().island_circ_direction = IslandExploreDirection::kClockwise;
	} else {
		allships.back().island_circ_direction = IslandExploreDirection::kCounterClockwise;
	}

	if (type == NewShip::kBuilt) {
		marine_task_queue.push_back(kStopShipyard);
	} else {
		seafaring_economy = true;
		if (ship.state_is_expedition()) {
			if (expedition_ship_ == kNoShip) {
				// OK, this ship is in expedition
				expedition_ship_ = ship.serial();
			} else {
				// What? Another ship in expedition? AI is not able to manage two expedition ships...
				log(" %d: AI will not control ship %s, as there is already another one in expedition\n",
				    player_number(), ship.get_shipname().c_str());
			}
		}
	}
}

// this is called whenever ship received a notification that requires
// navigation decisions (these notifiation are processes not in 'real time')
void DefaultAI::expedition_management(ShipObserver& so) {

	Map& map = game().map();
	const int32_t gametime = game().get_gametime();
	// probability for island exploration repetition
	const int repeat_island_prob = 20;

	// second we put current spot into visited_spots
	bool first_time_here = so.visited_spots.count(so.ship->get_position().hash()) == 0;
	if (first_time_here) {
		so.visited_spots.insert(so.ship->get_position().hash());
	}

	// if we have portspace following options are avaiable:
	// 1. Build a port there
	if (!so.ship->exp_port_spaces().empty()) {  // making sure we have possible portspaces

		// we score the place (value max == 8)
		const uint8_t spot_score = spot_scoring(so.ship->exp_port_spaces().front()) * 2;

		if (game().logic_rand() % 8 < spot_score) {  // we build a port here
			game().send_player_ship_construct_port(*so.ship, so.ship->exp_port_spaces().front());
			so.last_command_time = gametime;
			so.waiting_for_command_ = false;
			// blocking the area for some time to save AI from idle attempts to built there
			// buildings
			// TODO(TiborB): how long it takes to build a port?
			// I used 5 minutes
			MapRegion<Area<FCoords>> mr(
			   map, Area<FCoords>(map.get_fcoords(so.ship->exp_port_spaces().front()), 8));
			do {
				blocked_fields.add(mr.location(), game().get_gametime() + 5 * 60 * 1000);
			} while (mr.advance(map));

			return;
		}
	}

	// 2. Go on with expedition

	// we were not here before
	if (first_time_here) {
		game().send_player_ship_explore_island(*so.ship, so.island_circ_direction);

		// we were here before but we might randomly repeat island exploration
	} else if (game().logic_rand() % 100 < repeat_island_prob) {
		// let's add randomness to the Brown's molecule movement of ship
		// TODO(toptopple): make the following decision dependent on ship state != circle-island
		Widelands::IslandExploreDirection dir =
		   game().logic_rand() % 10 < 5 ? Widelands::IslandExploreDirection::kClockwise :
		                                  Widelands::IslandExploreDirection::kCounterClockwise;
		game().send_player_ship_explore_island(*so.ship, dir);

		// we head for open sea again
	} else {
		// determine swimmable directions
		std::vector<Direction> possible_directions;
		for (Direction dir = FIRST_DIRECTION; dir <= LAST_DIRECTION; ++dir) {
			// testing distance of 8 fields
			// this would say there is an 'open sea' there
			Widelands::FCoords tmp_fcoords = map.get_fcoords(so.ship->get_position());
			for (int8_t i = 0; i < 8; ++i) {
				tmp_fcoords = map.get_neighbour(tmp_fcoords, dir);
				if (tmp_fcoords.field->nodecaps() & MOVECAPS_SWIM) {
					if (i == 7) {
						possible_directions.push_back(dir);
					}
				} else {
					break;
				}
			}
		}

		// we test if there is open sea
		if (possible_directions.empty()) {
			// 2.A No there is no open sea
			// ## test-out of cancel-expedition method
			// ## circle island has danger of endless repetition
			// TODO(unknown): we should implement a 'rescue' procedure like 'sail for x fields and
			// rethink'
			game().send_player_cancel_expedition_ship(*so.ship);
			// game().send_player_ship_explore_island(*so.ship, so.island_circ_direction);

		} else {
			// 2.B Yes, pick one of available directions
			const Direction final_direction =
			   possible_directions.at(game().logic_rand() % possible_directions.size());
			game().send_player_ship_scouting_direction(
			   *so.ship, static_cast<WalkingDir>(final_direction));
		}
	}

	so.last_command_time = gametime;
	so.waiting_for_command_ = false;
	return;
}
