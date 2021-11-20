/*
 * Copyright (C) 2019-2021 by the Widelands Development Team
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

#include "economy/shipping_schedule.h"

#include <memory>
#include <set>

#include "base/log.h"
#include "economy/expedition_bootstrap.h"
#include "economy/portdock.h"
#include "economy/ship_fleet.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*******************************************************************************
              Weighting factors to tweak the algorithm's decisions
*******************************************************************************/
// All times in this file are given in milliseconds of gametime.

// Performance tradeoff in the First Pass
constexpr Duration kActualDurationsRecalculationInterval(60 * 1000);

// Give a ship the highest score for assigning wares to it if it will
// arrive at the port of interest within the next 10 s gametime,
// or the lowest score if this will take longer than 10 min gametime
constexpr uint64_t kWonderfullyShortDuration = 10 * 1000;   // 10 s
constexpr uint64_t kHorriblyLongDuration = 10 * 60 * 1000;  // 10 min

// Only assign wares to a ship in 5.1 if it's score is higher than a certain
// threshold based on minimal distance. Ships with lower scores will only be
// accepted if not enough idle ships are found.
constexpr uint64_t kMinScoreForImmediateAcceptFactor = 14;

// Average sailing-time distance for two ports to be considered "close by" in 5.4.
// Needs to be int16_t rather than uint16_t to avoid a signed-unsigned-comparison.
constexpr int16_t kDockGroupMaxDistanceFactor = 12 * 1800;

// Distributing idle ships between ports in the Sixth Pass:
// Sailing-time distance between a ship and a port so that the ship is considered
// to be within convenient reach when the port needs a ship. Used to ensure that
// idle ships will always stay close to a dock.
// Signed for the same reason as the previous constant.
constexpr int16_t kNearbyDockMaxDistanceFactor = 8 * 1800;

/*******************************************************************************
                             Actual implementation
*******************************************************************************/

#define sslog(...) verb_log_dbg_time(game.get_gametime(), "[Shipping Schedule] " __VA_ARGS__)

ShippingSchedule::ShippingSchedule(ShipFleet& f) : fleet_(f), last_updated_(0), loader_(nullptr) {
	assert(!fleet_.active());
}

bool ShippingSchedule::empty() const {
	for (const auto& pair : plans_) {
		if (!pair.second.empty()) {
			return false;
		}
	}
	return true;
}

bool ShippingSchedule::is_busy(const Ship& ship) const {
	if (ship.get_nritems()) {
		return true;
	}
	for (const SchedulingState& ss : plans_.at(const_cast<Ship*>(&ship))) {
		if (ss.expedition || !ss.load_there.empty()) {
			return true;
		}
	}
	return false;
}

void ShippingSchedule::start_expedition(Game& game, Ship& ship, PortDock& port) {
	sslog("Loading expedition\n\n");
	assert(port.expedition_ready_);
	assert(ship.get_nritems() == 0);

	std::vector<Worker*> workers;
	std::vector<WareInstance*> wares;
	port.expedition_bootstrap_->get_waiting_workers_and_wares(
	   game, port.owner().tribe(), &workers, &wares);
	for (Worker* worker : workers) {
		ship.add_item(game, ShippingItem(*worker));
	}
	for (WareInstance* ware : wares) {
		ship.add_item(game, ShippingItem(*ware));
	}

	ship.set_destination(game, nullptr);
	ship.start_task_expedition(game);
	port.cancel_expedition(game);

	// The ship is technically not a part of the fleet any more.
	// It will call ship_removed() now, erasing its plan from `plans_`.
	// The ship will re-add itself when the expedition is completed or cancelled.
}

void ShippingSchedule::ship_arrived(Game& game, Ship& ship, PortDock& port) {
	sslog(
	   "\nShippingSchedule::ship_arrived(%s at %u)\n", ship.get_shipname().c_str(), port.serial());
	auto plan = plans_.find(&ship);
	assert(plan != plans_.end());
	const size_t plan_size = plan->second.size();
	assert(plan_size);

	const SchedulingState& scheduling_state = plan->second.front();
	assert(scheduling_state.dock == &port);

	if (scheduling_state.expedition) {
		assert(plan_size == 1);  // no planning beyond the expedition
		assert(scheduling_state.load_there.empty());
		return start_expedition(game, ship, port);
	}

	assert(plan_size >= 1 + scheduling_state.load_there.size());  // besides the current portdock
	                                                              // we should at least visit all
	                                                              // the destinations for which
	                                                              // we are loading wares
	for (const auto& pair : scheduling_state.load_there) {
		assert(pair.first.get(game));
		assert(pair.first != &port);
		assert(pair.second > 0);
		for (uint32_t i = 0; i < pair.second; ++i) {
			if (!port.load_one_item(game, ship, *pair.first.get(game))) {
				// We planned for more items than we may take. Can happen when
				// transfers are cancelled in the last moment. Ignore.
				break;
			}
		}
	}

	plan->second.pop_front();
	if (plan_size > 1) {
		ship.set_destination(game, plan->second.front().dock.get(game));
		sslog("Loaded cargo and sending to %u\n\n", plan->second.front().dock.serial());
	} else {
		ship.set_destination(game, nullptr);
		assert(ship.get_nritems() == 0);
		sslog("Loaded nothing, idle now\n\n");
	}
}

// Helper function for port_removed().
// Returns whether this ship was planning to visit this dock at all.
bool ShippingSchedule::do_remove_port_from_plan(Game& game,
                                                PortDock* dock,
                                                Ship& ship,
                                                ShipPlan& ship_plan) {
	size_t index_of_deleted_dock = 0;
	const size_t nr_entries = ship_plan.size();
	bool dock_found = false;
	for (auto& ss : ship_plan) {
		if (ss.dock == dock) {
			dock_found = true;
		}
		auto it = ss.load_there.find(dock);
		if (it != ss.load_there.end()) {
			assert(!dock_found);
			ss.load_there.erase(it);
			// TODO(Nordfriese): Perhaps the ship could now skip some destinations
			// (like in the third pass)
		}
		if (!dock_found) {
			++index_of_deleted_dock;
		}
	}
	if (!dock_found) {
		return false;
	}
	if (index_of_deleted_dock == 0) {
		// reroute to next dock
		ship_plan.pop_front();
		if (ship_plan.empty()) {
			// no other docks to visit, but possibly wares left, reroute to the closest dock
			PortDock* closest = nullptr;
			int32_t dist = 0;
			for (PortDock* pd : fleet_.get_ports()) {
				Path path;
				int32_t d = -1;
				ship.calculate_sea_route(game, *pd, &path);
				game.map().calc_cost(path, &d, nullptr);
				assert(d >= 0);
				if (!closest || d < dist) {
					dist = d;
					closest = pd;
				}
			}
			if (closest) {
				sslog("Ship %s is carrying %u items, rerouting to NEW destination %u\n",
				      ship.get_shipname().c_str(), ship.get_nritems(), closest->serial());
				ship_plan.push_back(SchedulingState(closest, false, Duration(dist)));
				ship.set_destination(game, closest);
			} else {
				// PANIC! There are no ports at all left!!
				// But we might still have cargo!!! What should we do????
				// Stay calm. Just do nothing. Nothing at all.
				sslog("Ship %s is carrying %u items and there are no ports left, setting NO "
				      "destination\n",
				      ship.get_shipname().c_str(), ship.get_nritems());
				ship.set_destination(game, nullptr);
			}
		} else {  // the ships has more destinations in its plan, just reroute to the next one
			ship.set_destination(game, ship_plan.front().dock.get(game));
			sslog("Rerouted %s to %u\n", ship.get_shipname().c_str(), ship_plan.front().dock.serial());
			Path path;
			ship.calculate_sea_route(game, *ship_plan.front().dock.get(game), &path);
			int32_t d = -1;
			game.map().calc_cost(path, &d, nullptr);
			assert(d >= 0);
			ship_plan.front().duration_from_previous_location = Duration(d);
		}
	} else {  // the deleted dock is not the next one, this is a bit easier to handle
		sslog("no rerouting for %s, only recalc schedule\n", ship.get_shipname().c_str());
		// no rerouting needed, just recalc the schedule time
		auto deleteme = ship_plan.begin();
		for (size_t i = index_of_deleted_dock; i; --i) {
			++deleteme;
		}
		ship_plan.erase(deleteme);
		if (index_of_deleted_dock + 1 < nr_entries) {
			auto i1 = ship_plan.begin();
			auto i2 = ship_plan.begin();
			for (size_t i = index_of_deleted_dock; i; --i) {
				if (i != index_of_deleted_dock) {
					++i1;
				}
				++i2;
			}
			assert(i1 != i2);
			Path path;
			fleet_.get_path(*i1->dock.get(game), *i2->dock.get(game), path);
			int32_t d = -1;
			game.map().calc_cost(path, &d, nullptr);
			assert(d >= 0);
			i2->duration_from_previous_location = Duration(d);
		}
	}
	return true;
}

// `dock` is not a dangling reference yet, but this function is called
// via `ShipFleet::remove_port()` from `PortDock::cleanup()`
void ShippingSchedule::port_removed(Game& game, PortDock* dock) {
	sslog("\nShippingSchedule::port_removed (%u)\n", dock->serial());
	// Find all ships planning to visit this dock and reroute them.
	std::vector<Ship*> ships_heading_there;
	for (auto& ship_and_plan : plans_) {
		if (do_remove_port_from_plan(
		       game, dock, *ship_and_plan.first.get(game), ship_and_plan.second)) {
			ships_heading_there.push_back(ship_and_plan.first.get(game));
		}
	}

	// Find all shippingitems heading for the deleted dock.
	// Those in warehouses can just be told to recalculate their route.
	// Those on ships will be rerouted to whichever portdock the ships will visit next;
	// they will be unloaded there and then recalculate their route.

	for (PortDock* pd : fleet_.get_ports()) {
		for (auto it = pd->waiting_.begin(); it != pd->waiting_.end();) {
			if (it->destination_dock_.serial() == dock->serial()) {
				sslog("found a shippingitem in port %u\n", pd->serial());
				it->set_location(game, pd->warehouse_);
				it->end_shipping(game);
				it = pd->waiting_.erase(it);
			} else {
				++it;
			}
		}
	}

	for (Ship* ship : ships_heading_there) {
		for (ShippingItem& si : ship->items_) {
			if (si.destination_dock_.serial() == dock->serial()) {
				sslog("found a shippingitem on %s\n", ship->get_shipname().c_str());
				si.destination_dock_ = ship->get_destination();
			}
		}
	}
	sslog("--- port_removed maintenance complete ---\n\n");
}

void ShippingSchedule::ship_removed(const Game&, Ship* ship) {
	auto it = plans_.find(ship);
	assert(it != plans_.end());
	plans_.erase(it);

	auto i = last_actual_duration_recalculation_.find(ship);
	if (i != last_actual_duration_recalculation_.end()) {
		last_actual_duration_recalculation_.erase(i);
	}

	// Handling any items that were intended to be transported by this ship
	// is deferred to the next call to update()
}

void ShippingSchedule::ship_added(Game& game, Ship& s) {
	sslog("\nShippingSchedule::ship_added (%s)\n", s.get_shipname().c_str());
	assert(!s.get_destination());
	plans_[&s] = ShipPlan();
	last_actual_duration_recalculation_[&s] = game.get_gametime();
	if (fleet_.get_ports().empty()) {
#ifndef NDEBUG
		for (ShippingItem& si : s.items_) {
			assert(!si.destination_dock_.is_set());
		}
#endif
		sslog("No ports!\n\n");
		return;
	}

	PortDock* closest = nullptr;
	int32_t dist = 0;
	for (PortDock* dock : fleet_.get_ports()) {
		Path path;
		int32_t d = -1;
		s.calculate_sea_route(game, *dock, &path);
		game.map().calc_cost(path, &d, nullptr);
		assert(d >= 0);
		if (!closest || d < dist) {
			dist = d;
			closest = dock;
		}
	}
	assert(closest);
	for (ShippingItem& si : s.items_) {
		si.destination_dock_ = closest;
	}
	plans_[&s].push_back(SchedulingState(closest, false, Duration(dist)));
	s.set_destination(game, closest);
	// Check for closest to make clang-tidy happy
	sslog("Sent to %u\n\n", closest ? closest->serial() : 0);
}

void ShippingSchedule::port_added(Game& game, PortDock& dock) {
	sslog("\nShippingSchedule::port_added (%u)\n", dock.serial());
	if (fleet_.count_ports() > 1) {
		// nothing to do currently
		sslog("nothing to do\n\n");
		return;
	}
	// All ships are most likely panicking because they have
	// no destination. Send them all to the new port.
	for (Ship* ship : fleet_.get_ships()) {
		assert(!ship->get_destination());
		sslog("Rerouting %s there\n", ship->get_shipname().c_str());
		ship->set_destination(game, &dock);
		for (ShippingItem& si : ship->items_) {
			assert(!si.destination_dock_.is_set());
			si.destination_dock_ = &dock;
		}
	}
	sslog("--- port_added maintenance complete ---\n\n");
}

// Helper struct used by a PrioritisedPortPair
struct ScoredShip {
	Ship* ship;
	uint64_t score;     // higher scores are good, lower scores bad
	uint32_t capacity;  // number of items that can be loaded onto the ship
	Duration eta;       // time of arrival at the starting port
	Duration detour;    // the total time the ship will need from the starting port to the
	                    // destination minus the minimum time required for the direct route

	static inline uint64_t calc_score(uint64_t capacity, uint64_t eta, uint64_t detour) {
		// This needs to use uint64_t because the intermediate results will overflow uint32_t
		return eta > kHorriblyLongDuration ? 0 :
                                           capacity * kMinScoreForImmediateAcceptFactor *
		                                        kHorriblyLongDuration * kHorriblyLongDuration /
		                                        (std::max(eta, kWonderfullyShortDuration) *
		                                         std::max(detour, kWonderfullyShortDuration));
	}

	ScoredShip(Ship* s, uint32_t c, Duration e, Duration d)
	   : ship(s), score(calc_score(c, e.get(), d.get())), capacity(c), eta(e), detour(d) {
	}
	ScoredShip(const ScoredShip&) = default;
	ScoredShip& operator=(const ScoredShip&) = default;
	~ScoredShip() = default;

	// allow deterministic sorting in sets
	// "smaller" comparison means "better"
	bool operator<(const ScoredShip& ss) const {
		if (score != ss.score) {
			return score > ss.score;
		}
		if (eta != ss.eta) {
			return eta < ss.eta;
		}
		if (detour != ss.detour) {
			return detour < ss.detour;
		}
		if (capacity != ss.capacity) {
			return capacity > ss.capacity;
		}
		return ship->serial() < ss.ship->serial();
	}
};

struct PrioritisedPortPair {
	PrioritisedPortPair(PortDock* p1, PortDock* p2, uint32_t o, uint32_t p)
	   : start(p1), end(p2), open_count(o), priority(p) {
		assert(open_count > 0);
		assert(start);
		assert(end);
	}
	PrioritisedPortPair(const PrioritisedPortPair&) = default;
	PrioritisedPortPair& operator=(const PrioritisedPortPair&) = default;
	~PrioritisedPortPair() = default;

	PortDock* start;      // starting port
	PortDock* end;        // destination port
	uint32_t open_count;  // number of wares waiting at `start` and heading for
	                      // `end` that have not been assigned to a ship yet
	uint32_t priority;  // higher priority means that this PPP should be serviced as soon as possible

	// cache for the functions in update()
	std::list<ScoredShip> ships;

	// allow deterministic sorting in sets
	// "smaller" comparison means "higher importance"
	bool operator<(const PrioritisedPortPair& pp) const {
		if (priority == pp.priority) {
			if (open_count == pp.open_count) {
				if (start != pp.start) {
					return start->serial() < pp.start->serial();
				}
				return end->serial() < pp.end->serial();
			}
			return open_count > pp.open_count;
		}
		return priority > pp.priority;
	}
};

// Helper functions to determine how much capacity the given ship will have
// after at the given port. Returns 0 if the ship is not planning to go
// there or will launch an expedition from there.
uint32_t ShippingSchedule::get_free_capacity_at(Game& game, Ship& ship, PortDock& dock) {
	assert(plans_.find(&ship) != plans_.end());
	CargoList cargo_tracker;
	for (const ShippingItem& si : ship.items_) {
		PortDock* dest = si.destination_dock_.get(game);
		auto it = cargo_tracker.find(dest);
		if (it == cargo_tracker.end()) {
			cargo_tracker[dest] = 1;
		} else {
			++it->second;
		}
	}
	for (const SchedulingState& ss : plans_[&ship]) {
		if (ss.expedition) {
			return 0;
		}
		{
			auto it = cargo_tracker.find(ss.dock);
			if (it != cargo_tracker.end()) {
				cargo_tracker.erase(it);
			}
		}
		for (const auto& _load : ss.load_there) {
			auto it = cargo_tracker.find(_load.first);
			if (it == cargo_tracker.end()) {
				cargo_tracker[_load.first] = _load.second;
			} else {
				it->second += _load.second;
			}
		}
		if (ss.dock == &dock) {
			uint32_t cap = ship.get_capacity();
			for (const auto& pair : cargo_tracker) {
				assert(cap >= pair.second);
				cap -= pair.second;
			}
			return cap;
		}
	}
	return 0;
}

void ShippingSchedule::get_free_capacity_between(Game& game,
                                                 const Ship& ship,
                                                 ShipPlan& plan,
                                                 PortDock& start,
                                                 PortDock& end,
                                                 bool& found_start,
                                                 bool& found_end,
                                                 bool& expedition,
                                                 bool& start_is_last,
                                                 Duration& arrival_time,
                                                 Duration& detour_start_end,
                                                 uint32_t& free_capacity) {
	found_start = false;
	found_end = false;
	start_is_last = false;
	expedition = false;
	arrival_time = Duration(0);
	detour_start_end = Duration(0);
	free_capacity = 0;

	uint32_t max_load = 0;
	CargoList cargo_tracker;
	for (const ShippingItem& si : ship.items_) {
		PortDock* dest = si.destination_dock_.get(game);
		auto it = cargo_tracker.find(dest);
		if (it == cargo_tracker.end()) {
			cargo_tracker[dest] = 1;
		} else {
			++it->second;
		}
	}
	for (SchedulingState& ss : plan) {
		start_is_last = ss.dock == &start;
		if (ss.expedition) {
			expedition = true;
			break;
		}
		{
			auto unload = cargo_tracker.find(ss.dock);
			if (unload != cargo_tracker.end()) {
				cargo_tracker.erase(unload);
			}
		}
		for (const auto& _load : ss.load_there) {
			auto it = cargo_tracker.find(_load.first);
			if (it == cargo_tracker.end()) {
				cargo_tracker[_load.first] = _load.second;
			} else {
				it->second += _load.second;
			}
		}
		uint32_t _load = 0;
		for (const auto& pair : cargo_tracker) {
			_load += pair.second;
		}
		max_load = std::max(max_load, _load);
		if (found_start) {
			assert(!found_end);
			assert(ss.dock != &start);
			if (ss.dock == &end) {
				found_end = true;
				break;
			}
			detour_start_end += ss.duration_from_previous_location;
		} else if (ss.dock == &end) {
			break;  // give A-B-A plans no chance
		} else {
			arrival_time += ss.duration_from_previous_location;
			if (ss.dock == &start) {
				found_start = true;
			}
		}
	}
	if (found_start && !expedition) {
		assert(max_load <= ship.get_capacity());
		free_capacity = ship.get_capacity() - max_load;
	}
}

// Shared logic for steps 5.1 and 5.3
void ShippingSchedule::load_on_ship(Game& game,
                                    PrioritisedPortPair& ppp,
                                    std::list<PrioritisedPortPair>& all_ppps) {
	const uint32_t take = std::min(ppp.open_count, ppp.ships.front().capacity);
	Ship* ship = ppp.ships.front().ship;
	sslog("load_on_ship: PPP %u –> %u (open_count %u): assigning %u items (capacity %u) to %s\n",
	      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take, ppp.ships.front().capacity,
	      ship->get_shipname().c_str());
	assert(take);
	// We assume that EITHER both end points are already part of the plan,
	// or that the start point is the last entry in the plan
	if (plans_[ship].back().dock == ppp.start) {
		assert(!plans_[ship].back().expedition);
		auto it = plans_[ship].back().load_there.find(ppp.end);
		if (it != plans_[ship].back().load_there.end()) {
			it->second += take;
		} else {
			plans_[ship].back().load_there[ppp.end] = take;
		}
		Path path;
		int32_t d = -1;
		fleet_.get_path(*ppp.start, *ppp.end, path);
		game.map().calc_cost(path, &d, nullptr);
		assert(d >= 0);
		plans_[ship].push_back(SchedulingState(ppp.end, false, Duration(d)));
	} else {
		for (SchedulingState& ss : plans_[ship]) {
			if (ss.dock == ppp.start) {
				auto it = ss.load_there.find(ppp.end);
				if (it != ss.load_there.end()) {
					it->second += take;
				} else {
					ss.load_there[ppp.end] = take;
				}
				break;
			}
		}
	}
	ppp.ships.front().capacity -= take;
	ppp.open_count -= take;
	if (!ppp.ships.front().capacity) {
		ppp.ships.erase(ppp.ships.begin());
	}
	// update all other PPPs that were considering to load wares onto this ship
	for (PrioritisedPortPair& p : all_ppps) {
		auto ship_it = p.ships.end();
		for (auto it = p.ships.begin(); it != p.ships.end(); ++it) {
			if (it->ship == ship) {
				ship_it = it;
				break;
			}
		}
		if (ship_it == p.ships.end()) {
			continue;
		}
		// recalc capacity and score for this ship and reorder list
		bool found_start = false;
		bool found_end = false;
		bool expedition = false;
		bool start_is_last = false;
		Duration arrival_time(0);
		Duration detour_start_end(0);
		uint32_t free_capacity = 0;
		get_free_capacity_between(game, *ship, plans_.at(ship), *p.start, *p.end, found_start,
		                          found_end, expedition, start_is_last, arrival_time,
		                          detour_start_end, free_capacity);
		p.ships.erase(ship_it);
		if (free_capacity && found_start && !expedition && (found_end || start_is_last)) {
			ScoredShip updated_ship(ship, free_capacity, arrival_time, detour_start_end);
			sslog("load_on_ship: PPP %u –> %u (open_count %u) UPDATED: may assign up to %u items "
			      "to %s (score %" PRIu64 ")\n",
			      p.start->serial(), p.end->serial(), ppp.open_count, free_capacity,
			      ship->get_shipname().c_str(), updated_ship.score);
			for (auto it = p.ships.begin();; ++it) {
				if (it == p.ships.end() || updated_ship < *it) {
					p.ships.insert(it, updated_ship);
					break;
				}
			}
		} else {
			sslog("load_on_ship: PPP %u –> %u (open_count %u) REMOVED %s\n", p.start->serial(),
			      p.end->serial(), ppp.open_count, ship->get_shipname().c_str());
		}
	}
}

Duration ShippingSchedule::update(Game& game) {
	/*
	 * This function is the heart of the shipping system.
	 * All decisions (except emergency decisions on port destruction) are made here.
	 * Here, we decide which ship will when arrive at which port and how many items
	 * for which destinations it will pick up.
	 * When we were notified that a ship or port was added or lost, we do some
	 * maintenance around this fact in other functions, but our main job is to check
	 * on every call every single port whether it has wares that will not be
	 * transported anytime soon. If so, we can tell a nearby idle ship to pick up
	 * wares, or order a ship that is already heading there to pick them up (if it
	 * makes sense), or leave them for now for lack of capacity. We may also cancel
	 * coming ships if transfers were cancelled so we need less capacity than
	 * previously planned.
	 * Additionally, we will distribute idle ships more or less equally among ports
	 * so that every port will have a ship at hand immediately when it requires one
	 * (provided that we have enough ships, of course).
	 * In theory, it would be good to implement intelligent reordering of ships'
	 * destinations, so as to avoid routes like A-B-C where A and C are close and B
	 * is far away. We used to have such an algorithm, but it was shown to
	 * necessarily be a performance killer (I only say Travelling Salesman Problem),
	 * so we dropped support for this approach and instead prefer the GOLDEN RULE that
	 * a ship should never service too many destinations at once, REGARDLESS of their
	 * relative distances. One ship from B to A plus one ship from B to C are better
	 * than one ship from B to both A and C. Instead we prefer to distribute tasks
	 * among many ships. This produces the best results when the player builds a large
	 * naval force. (When the player has few ships for many ports, this approach will
	 * work suboptimally, but that is a bad strategy so the player deserves no more.)
	 */

	sslog("\nShippingSchedule::update\n");
	assert(plans_.size() == fleet_.get_ships().size());
	const size_t nr_ports = fleet_.get_ports().size();
	if (fleet_.get_ships().empty() || nr_ports == 0) {
		sslog("No ships or no ports\n");
		// Nothing to do. Ships stay where they are, or do whatever they want.
		// Ports have to wait until we have ships.
		return Duration();
	}

	/* FIRST PASS:
	 * Scan all ships. Refresh the prediction when they will arrive at the next port.
	 * Most of the time, a simple estimate is enough.
	 * Now and then (every 60s), we calculate the exact time though to account for
	 * delays resulting e.g. from ships stopping to let another ship pass.
	 * This can be very costly, so we do it for only one ship per update.
	 */
	const Time& time = game.get_gametime();
	const Duration time_since_last_update = time - last_updated_;

	Time earliest_real_update =
	   time - std::min(Duration(time.get()), kActualDurationsRecalculationInterval);
	Ship* ship_to_update = nullptr;
	for (auto& pair : last_actual_duration_recalculation_) {
		if (pair.second < earliest_real_update) {
			earliest_real_update = pair.second;
			ship_to_update = pair.first.get(game);
		}
	}
	if (ship_to_update) {
		last_actual_duration_recalculation_[ship_to_update] = time;
	}
	sslog("FIRST PASS at %u (last %u, delta %u); will recalc for %s\n", time.get(),
	      last_updated_.get(), time_since_last_update.get(),
	      ship_to_update ? ship_to_update->get_shipname().c_str() : "(nil)");
	for (auto& pair : plans_) {
		Ship& ship = *pair.first.get(game);
		if (pair.second.empty()) {
			sslog("%s is idle\n", ship.get_shipname().c_str());
		} else if (ship_to_update && ship_to_update->serial() == pair.first.serial()) {
			sslog("Recalculate for %s\n", ship.get_shipname().c_str());
			Path path;
			ship.calculate_sea_route(game, *pair.second.front().dock.get(game), &path);
			int32_t d = -1;
			game.map().calc_cost(path, &d, nullptr);
			assert(d >= 0);
			pair.second.front().duration_from_previous_location = Duration(d);
		} else {
			if (pair.second.front().duration_from_previous_location > time_since_last_update) {
				pair.second.front().duration_from_previous_location -= time_since_last_update;
				sslog("Regular-type heuristic update for %s\n", ship.get_shipname().c_str());
			} else {
				// She said five more seconds, and that was ten seconds ago…
				// The ship is behind schedule, so this is an arbitrary estimate
				// about the arrival time. Doesn't matter if it's inaccurate,
				// the ship will most likely arrive within a few seconds.
				pair.second.front().duration_from_previous_location /= 2;
				sslog("UNEXPECTED-type heuristic update for %s\n", ship.get_shipname().c_str());
			}
		}
	}
	last_updated_ = time;

	/* SECOND PASS:
	 * Scan all ports. Make lists of waiting items.
	 * Figure out when the items will be picked up.
	 * Also cancel orders where we provided more capacity than is actually needed (which
	 * can happen when a transfer is cancelled when the item is still in the portdock),
	 * and cancel expedition ships in spe whose expeditions were cancelled.
	 */
	sslog("SECOND PASS: %" PRIuS " ports\n", nr_ports);

#ifndef NDEBUG
	for (const auto& plan : plans_) {
		assert(plan.second.size() <= nr_ports);
	}
#endif

	std::list<Ship*> ships_with_reduced_orders;
	std::list<PortDock*> ports_with_unserviced_expeditions;

	// Don't even think about trying to cache any of these. It is impossible to maintain.
	using Pair_DurationAndQuantity =
	   std::pair<Duration /* when at `start` */,
	             Quantity /* accept how much from `start` to `dest` */>;
	using Map_ShipsAndInfo = std::map<OPtr<Ship> /* by whom */, Pair_DurationAndQuantity>;
	using Pair_ShipsWithInfoAndCapacity =
	   std::pair<Map_ShipsAndInfo, int32_t /* capacity missing (-) or extra (+) */>;
	using Map_ToDestination_ShipsWithInfoAndCapacity =
	   std::map<OPtr<PortDock> /* destination */, Pair_ShipsWithInfoAndCapacity>;
	using Map_FromStartToDestination_ShipsWithInfoAndCapacity =
	   std::map<OPtr<PortDock> /* start */, Map_ToDestination_ShipsWithInfoAndCapacity>;
	Map_FromStartToDestination_ShipsWithInfoAndCapacity items_in_ports;

	for (PortDock* dock : fleet_.get_ports()) {
		const bool expedition_ready = dock->is_expedition_ready();
		sslog("Iteration: dock %u (expedition ready %s)\n", dock->serial(),
		      expedition_ready ? "true" : "false");
		Ship* expedition_ship_coming = nullptr;
		Map_ToDestination_ShipsWithInfoAndCapacity map;
		for (auto& plan : plans_) {
			Duration eta = Duration(0);
			CargoList* _load = nullptr;
			for (SchedulingState& ss : plan.second) {
				eta += ss.duration_from_previous_location;
				if (ss.dock == dock) {
					if (ss.expedition) {
						assert(!expedition_ship_coming);
						expedition_ship_coming = plan.first.get(game);
						assert(ss.load_there.empty());
					}
					_load = &ss.load_there;
					break;
				}
			}
			if (_load) {
				for (const auto& cargo : *_load) {
					map[cargo.first].first[plan.first] = std::make_pair(eta, cargo.second);
				}
			}
		}

		if (expedition_ready && !expedition_ship_coming) {
			sslog("Iteration: expedition unserviced\n");
			ports_with_unserviced_expeditions.push_back(dock);
		} else if (expedition_ship_coming && !expedition_ready) {
			for (ShipPlan::iterator it = plans_[expedition_ship_coming].begin();
			     it != plans_[expedition_ship_coming].end(); ++it) {
				if (it->dock == dock) {
					assert(it->expedition);
					plans_[expedition_ship_coming].erase(it);
					break;
				}
			}
			sslog("Iteration: expedition cancelled\n");
			if (std::find(ships_with_reduced_orders.begin(), ships_with_reduced_orders.end(),
			              expedition_ship_coming) == ships_with_reduced_orders.end()) {
				ships_with_reduced_orders.push_back(expedition_ship_coming);
			}
		}

		for (PortDock* dest : fleet_.get_ports()) {
			const int32_t waiting_items = dock->count_waiting(dest);
			sslog("Iteration: Iteration: dest %u, waiting %d\n", dest->serial(), waiting_items);
			std::multiset<Duration> arrival_times;  // one entry per item that will be picked up
			for (const auto& pair : map[dest].first) {
				for (uint32_t i = pair.second.second; i; --i) {
					arrival_times.insert(pair.second.first);
				}
			}

			const int32_t planned_capacity = arrival_times.size();
			int32_t delta = planned_capacity - waiting_items;
			sslog("Iteration: Iteration: planned_capacity %d, delta %d\n", planned_capacity, delta);
#ifndef NDEBUG
			if (dock == dest) {
				assert(waiting_items == 0);
				assert(planned_capacity == 0);
			}
#endif

			while (delta > 0) {
				// reduce or cancel the last order in the queue
				const Duration last_arrival = *arrival_times.crbegin();
				for (auto pair_it = map[dest].first.begin(); pair_it != map[dest].first.end();
				     ++pair_it) {
					assert(pair_it->second.first <= last_arrival);
					if (pair_it->second.first == last_arrival) {
						uint32_t reducedby;
						bool erase = false;
						// cancel in the overview…
						if (pair_it->second.second > static_cast<unsigned>(delta)) {
							reducedby = delta;
							pair_it->second.second -= delta;
							delta = 0;
						} else {
							reducedby = pair_it->second.second;
							delta -= pair_it->second.second;
							pair_it->second.second = 0;
							erase = true;
						}
						sslog("Iteration: Iteration: planned_capacity reduced by %d (ship %s)\n",
						      reducedby, pair_it->first.get(game)->get_shipname().c_str());
						for (uint32_t i = reducedby; i; --i) {
							assert(*std::prev(arrival_times.end()) == last_arrival);
							arrival_times.erase(std::prev(arrival_times.end()));
						}
						// …and in the schedule
						bool found = false;
						for (SchedulingState& ss : plans_.at(pair_it->first.get(game))) {
							if (ss.dock == dock) {
								for (auto it = ss.load_there.begin(); it != ss.load_there.end(); ++it) {
									if (it->first == dest) {
										assert(it->second >= reducedby);
										it->second -= reducedby;
										if (it->second == 0) {
											ss.load_there.erase(it);
										}
										found = true;
										break;
									}
								}
								if (found) {
									break;
								}
							}
						}
						assert(found);
						if (std::find(ships_with_reduced_orders.begin(), ships_with_reduced_orders.end(),
						              pair_it->first.get(game)) == ships_with_reduced_orders.end()) {
							ships_with_reduced_orders.push_back(pair_it->first.get(game));
						}
						if (erase) {
							map[dest].first.erase(pair_it);
						}
						break;
					}
				}
			}
			if (delta < 0) {
				map[dest].second = delta;
			}
		}
		items_in_ports[dock] = map;
	}

	/* THIRD PASS:
	 * Go through the list of ships that had orders cancelled, and check whether we might
	 * even skip some of their destinations altogether.
	 */
	for (Ship* ship : ships_with_reduced_orders) {
		sslog("THIRD PASS: Iteration %s\n", ship->get_shipname().c_str());
		assert(plans_.find(ship) != plans_.end());
		ShipPlan::iterator previt = plans_[ship].end();
		std::set<Serial> cargo_to;
		for (const ShippingItem& si : ship->items_) {
			cargo_to.insert(si.destination_dock_.serial());
		}
		for (auto it = plans_[ship].begin(); it != plans_[ship].end();) {
			if (it->load_there.empty() && !it->expedition && !cargo_to.count(it->dock.serial())) {
				it = plans_[ship].erase(it);
				if (it != plans_[ship].end()) {
					if (previt == plans_[ship].end()) {
						Path path;
						ship->calculate_sea_route(game, *it->dock.get(game), &path);
						int32_t d = -1;
						game.map().calc_cost(path, &d, nullptr);
						assert(d >= 0);
						it->duration_from_previous_location = Duration(d);
					} else {
						Path path;
						fleet_.get_path(*previt->dock.get(game), *it->dock.get(game), path);
						int32_t d = -1;
						game.map().calc_cost(path, &d, nullptr);
						assert(d >= 0);
						it->duration_from_previous_location = Duration(d);
					}
				}
			} else {
				for (const auto& pair : it->load_there) {
					cargo_to.insert(pair.first.serial());
				}
				previt = it;
				++it;
			}
		}
		if (plans_[ship].empty()) {
			sslog("No orders left, setting to idle\n");
			ship->set_destination(game, nullptr);
		} else if (plans_[ship].front().dock != ship->get_destination()) {
			ship->set_destination(game, plans_[ship].front().dock.get(game));
			sslog("Rerouted to %u\n", ship->get_destination()->serial());
		}
	}

	/* FOURTH PASS:
	 * First of all, check the waiting unserviced expeditions. If a ship is
	 * heading for such a port and will not pick up anything there, and
	 * has no plans beyond, make it an expedition ship there.
	 * Then go through all ports with still unserviced expeditions ready,
	 * and make a list of all idle or trivial ships. (A ship is called
	 * trivial if it is not planning to pick up any wares or service an
	 * expedition.) Assign every idle/trivial ship the closest unserviced
	 * expedition, until we run out of idle ships or all expeditions are
	 * serviced.
	 */
	for (auto dock = ports_with_unserviced_expeditions.begin();
	     dock != ports_with_unserviced_expeditions.end();) {
		sslog("FOURTH PASS: Iteration %u\n", (*dock)->serial());
		bool assigned = false;
		for (auto& plan : plans_) {
			bool has_further_plans = false;
			SchedulingState* heading_there = nullptr;
			for (SchedulingState& ss : plan.second) {
				if (ss.dock == *dock) {
					assert(!heading_there);
					heading_there = &ss;
					if (ss.expedition || !ss.load_there.empty()) {
						has_further_plans = true;
						break;
					}
				} else if (heading_there) {
					has_further_plans = true;
					break;
				}
			}
			if (heading_there && !has_further_plans) {
				// success
				sslog("assigning to %s\n", plan.first.get(game)->get_shipname().c_str());
				heading_there->expedition = true;
				assigned = true;
				break;
			}
		}
		if (assigned) {
			dock = ports_with_unserviced_expeditions.erase(dock);
		} else {
			sslog("unassigned at first\n");
			++dock;
		}
	}
	if (!ports_with_unserviced_expeditions.empty()) {
		std::list<Ship*> ships_for_expeditions;
		for (auto& plan : plans_) {
			bool trivial = plan.second.empty();
			if (!trivial) {
				if (plan.first.get(game)->get_nritems() == 0) {
					trivial = true;
					for (const SchedulingState& ss : plan.second) {
						if (ss.expedition || !ss.load_there.empty()) {
							trivial = false;
							break;
						}
					}
				}
			}
			if (trivial) {
				assert(plan.first.get(game)->get_nritems() == 0);
				ships_for_expeditions.push_back(plan.first.get(game));
			}
		}
		sslog("found %" PRIuS "expedition ships for %" PRIuS " unserviced expeditions\n",
		      ships_for_expeditions.size(), ports_with_unserviced_expeditions.size());
		for (size_t matches =
		        std::min(ports_with_unserviced_expeditions.size(), ships_for_expeditions.size());
		     matches; --matches) {
			Ship* ship = ships_for_expeditions.front();
			std::list<PortDock*>::iterator closest = ports_with_unserviced_expeditions.end();
			int32_t dist = 0;
			for (auto dock = ports_with_unserviced_expeditions.begin();
			     dock != ports_with_unserviced_expeditions.end(); ++dock) {
				Path path;
				int32_t d = -1;
				ship->calculate_sea_route(game, **dock, &path);
				game.map().calc_cost(path, &d, nullptr);
				assert(d >= 0);
				if (d < dist || closest == ports_with_unserviced_expeditions.end()) {
					dist = d;
					closest = dock;
				}
			}
			sslog("assigned %s to dock %u\n", ship->get_shipname().c_str(), (*closest)->serial());
			ship->set_destination(game, *closest);
			plans_[ship].clear();
			plans_[ship].push_back(SchedulingState(*closest, true, Duration(dist)));
			ports_with_unserviced_expeditions.erase(closest);
			ships_for_expeditions.pop_front();
		}
	}

	/* FIFTH PASS:
	 * Go through the lists of start-end pairs where we need more capacity.
	 * Assign each pair a priority based on the sum of the transfer
	 * priorities of the individual wares and workers.
	 * 1) For each pair, check it a ship is coming that will visit the
	 *    destination shortly afterwards and still has capacity for more
	 *    items.
	 *    If so, we'll assign the extra capacity to this ship, but only
	 *    if the ship will go straight from here to there – and the time
	 *    from now to this ship's arrival here is not too high.
	 *    If the latter condition is not met, make a note of this ship.
	 * 2) If we didn't assign the entire required capacity yet, look for idle
	 *    ships and assign one or more of them (preferably the closest ones)
	 *    the task of transporting those items.
	 * 3) Still capacity left? Also accept the ships we noted in step 1.
	 * 4) And if that still isn't enough, check if there are other ports
	 *    within a low radius of the start and end ports, and also accept
	 *    ships that have a destination in the start group *directly followed
	 *    by* a destination in the end group, and has free capacity between
	 *    these destinations, and tell such a ship to additionally visit the
	 *    start and end port between its two existing targets.
	 */
	std::set<PrioritisedPortPair> _open_pairs;
	for (auto& start_to_map : items_in_ports) {
		for (auto& dest_to_shipsinfos : start_to_map.second) {
			assert(dest_to_shipsinfos.second.second <= 0);
			if (dest_to_shipsinfos.second.second < 0) {
				const int32_t maxprio = start_to_map.first.get(game)->calc_max_priority(
				   game, *dest_to_shipsinfos.first.get(game));
				const int32_t total_waiting =
				   start_to_map.first.get(game)->count_waiting(dest_to_shipsinfos.first.get(game));
				const int32_t open = -dest_to_shipsinfos.second.second;
				assert(total_waiting >= open);
				assert(maxprio >= total_waiting);  // a priority of at least 1 per item
				const int32_t prio = maxprio * open / total_waiting;
				assert(prio >= 0);
				_open_pairs.insert(PrioritisedPortPair(
				   start_to_map.first.get(game), dest_to_shipsinfos.first.get(game), open, prio));
			}
		}
	}

	// ensure the order stays constant from now on
	std::list<PrioritisedPortPair> open_pairs;
	for (const PrioritisedPortPair& ppp : _open_pairs) {
		open_pairs.push_back(ppp);
	}
	sslog("FIFTH PASS: Found %" PRIuS " open pairs\n", open_pairs.size());

	// 1) check for coming ships already going there, or planning to go nowhere after here
	for (PrioritisedPortPair& ppp : open_pairs) {
		assert(ppp.ships.empty());
		std::set<ScoredShip> _ships;
		for (auto& plan : plans_) {
			bool found_start = false;
			bool found_end = false;
			bool expedition = false;
			bool start_is_last = false;
			Duration arrival_time(0);
			Duration detour_start_end(0);
			uint32_t free_capacity = 0;
			get_free_capacity_between(game, *plan.first.get(game), plan.second, *ppp.start, *ppp.end,
			                          found_start, found_end, expedition, start_is_last, arrival_time,
			                          detour_start_end, free_capacity);
			if (free_capacity && found_start && !expedition && (found_end || start_is_last)) {
				ScoredShip ss(plan.first.get(game), free_capacity, arrival_time, detour_start_end);
				sslog("Phase 5.0: PPP %u –> %u (open_count %u): may assign up to %u items to %s (score "
				      "%" PRIu64 ")\n",
				      ppp.start->serial(), ppp.end->serial(), ppp.open_count, free_capacity,
				      plan.first.get(game)->get_shipname().c_str(), ss.score);
				_ships.insert(ss);
			}
		}
		for (const ScoredShip& ss : _ships) {
			ppp.ships.push_back(ss);
		}
		int32_t threshold = -1;
		Path path;
		fleet_.get_path(*ppp.start, *ppp.end, path);
		game.map().calc_cost(path, &threshold, nullptr);
		assert(threshold > 0);
		while (ppp.open_count > 0 && !ppp.ships.empty()) {
			if (ppp.ships.front().score < static_cast<unsigned>(threshold)) {
				break;
			}
			load_on_ship(game, ppp, open_pairs);
		}
		sslog("Phase 5.1: PPP %u –> %u: %u open_count remaining\n", ppp.start->serial(),
		      ppp.end->serial(), ppp.open_count);
	}

	// 2) assign idle ships
	std::list<Ship*> idle_ships;
	for (auto& plan : plans_) {
		if (plan.second.empty() ||
		    (plan.second.size() == 1 && !plan.second.front().expedition &&
		     plan.second.front().load_there.empty() && plan.first.get(game)->get_nritems() == 0)) {
			idle_ships.push_back(plan.first.get(game));
		}
	}
	sslog("Phase 5.2: %" PRIuS " idle ships found\n", idle_ships.size());
	for (PrioritisedPortPair& ppp : open_pairs) {
		while (ppp.open_count && !idle_ships.empty()) {
			Ship* closest = nullptr;
			int32_t dist = 0;
			for (Ship* ship : idle_ships) {
				Path path;
				int32_t d = -1;
				ship->calculate_sea_route(game, *ppp.start, &path);
				game.map().calc_cost(path, &d, nullptr);
				assert(d >= 0);
				if (!closest || d < dist) {
					dist = d;
					closest = ship;
				}
			}
			assert(closest);
			const uint32_t take = std::min(ppp.open_count, closest->get_capacity());
			assert(take);
			sslog("Phase 5.2: PPP %u –> %u (open_count %u): assigning %u items to %s\n",
			      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
			      closest->get_shipname().c_str());
			plans_[closest].clear();
			plans_[closest].push_back(SchedulingState(ppp.start, false, Duration(dist)));
			plans_[closest].front().load_there[ppp.end] = take;
			closest->set_destination(game, ppp.start);
			ppp.open_count -= take;
			dist = -1;
			Path path;
			fleet_.get_path(*ppp.start, *ppp.end, path);
			game.map().calc_cost(path, &dist, nullptr);
			assert(dist >= 0);
			plans_[closest].push_back(SchedulingState(ppp.end, false, Duration(dist)));
			idle_ships.erase(std::find(idle_ships.begin(), idle_ships.end(), closest));
			for (PrioritisedPortPair& p : open_pairs) {
				for (auto it = p.ships.begin(); it != p.ships.end(); ++it) {
					if (it->ship == closest) {
						p.ships.erase(it);
						break;
					}
				}
			}
		}
	}

	// 3) accept suboptimal ships already heading here
	std::list<PortDock*> open_count_left;
	for (PrioritisedPortPair& ppp : open_pairs) {
		while (ppp.open_count && !ppp.ships.empty()) {
			sslog("Phase 5.3: PPP %u –> %u (open_count %u): assigning items…\n", ppp.start->serial(),
			      ppp.end->serial(), ppp.open_count);
			load_on_ship(game, ppp, open_pairs);
		}
		sslog("%u open_count remaining\n", ppp.open_count);
		if (ppp.open_count) {
			bool found1 = false;
			bool found2 = false;
			for (const PortDock* pd : open_count_left) {
				found1 |= pd == ppp.start;
				found2 |= pd == ppp.end;
				if (found1 && found2) {
					break;
				}
			}
			if (!found1) {
				open_count_left.push_back(ppp.start);
			}
			if (!found2) {
				open_count_left.push_back(ppp.end);
			}
		}
	}

	// 4) Make lists of all docks within a certain radius of the start and end docks,
	//    and search for all ships that will service any port in the start group and
	//    then either nothing, or any port in the end group (the latter only if the
	//    ship has free capacity in-between). Sort all candidates using SortedShip
	//    functionality, and then assign as many items as possible.
	if (!open_count_left.empty()) {
		std::map<OPtr<PortDock>, std::set<OPtr<PortDock>>> groups;
		// only calculate the groups for those docks where we need them
		for (PortDock* dock : open_count_left) {
			for (PortDock* other : fleet_.get_ports()) {
				if (other == dock) {
					groups[dock].insert(other);
					continue;
				}
				Path path;
				fleet_.get_path(*dock, *other, path);
				int32_t c1 = 0;
				int32_t c2 = 0;
				game.map().calc_cost(path, &c1, &c2);
				assert(c1 >= 0);
				assert(c2 >= 0);
				if (c1 + c2 < 2 * kDockGroupMaxDistanceFactor) {
					groups[dock].insert(other);
				}
			}
			assert(!groups.at(dock).empty());
		}
		sslog("Phase 5.4: Created groups for %" PRIuS " ports\n", groups.size());

		for (PrioritisedPortPair& ppp : open_pairs) {
			for (auto& plan : plans_) {
				if (!ppp.open_count) {
					break;
				}
				int32_t index_of_start = -1;
				int32_t index_of_end = -1;
				std::set<uint32_t> indices_near_start;
				std::set<uint32_t> indices_near_end;
				uint32_t idx = 0;
				int32_t expedition = -1;
				assert(!plan.second.empty());
				for (const SchedulingState& ss : plan.second) {
					if (ss.expedition) {
						assert(expedition < 0);
						expedition = idx;
					}
					if (ss.dock == ppp.start) {
						assert(index_of_start < 0);
						index_of_start = idx;
					} else if (groups.at(ppp.start).count(ss.dock)) {
						indices_near_start.insert(idx);
					}
					if (ss.dock == ppp.end) {
						assert(index_of_end < 0);
						index_of_end = idx;
					} else if (groups.at(ppp.end).count(ss.dock)) {
						indices_near_end.insert(idx);
					}
					++idx;
				}
				if (index_of_start >= 0 && index_of_end >= 0) {
					continue;  // give illegal A-B-A plans no chance
				}
				if (index_of_start >= 0 && index_of_end < 0) {
					if (expedition >= 0 && expedition <= index_of_start) {
						continue;
					}
					if (indices_near_end.count(index_of_start + 1)) {
						// ship will visit start and directly afterwards a port close to end (but never
						// end)
						// → a) insert items at start, b) push a State to end, and c) update time for the
						// state after that
						const uint32_t capacity =
						   get_free_capacity_at(game, *plan.first.get(game), *ppp.start);
						if (!capacity) {
							continue;
						}
						const uint32_t take = std::min(capacity, ppp.open_count);
						sslog("Phase 5.4.A: PPP %u –> %u (open_count %u): assigning %u items to %s\n",
						      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
						      plan.first.get(game)->get_shipname().c_str());
						assert(take);
						ppp.open_count -= take;
						// c
						auto it_after_end = plan.second.begin();
						for (uint32_t i = index_of_start + 1; i; --i) {
							++it_after_end;
						}
						Path path;
						int32_t d = -1;
						fleet_.get_path(*ppp.end, *it_after_end->dock.get(game), path);
						game.map().calc_cost(path, &d, nullptr);
						assert(d >= 0);
						it_after_end->duration_from_previous_location = Duration(d);
						// a
						auto it_start = plan.second.begin();
						for (uint32_t i = index_of_start; i; --i) {
							++it_start;
						}
						auto cargoit = it_start->load_there.find(ppp.end);
						if (cargoit != it_start->load_there.end()) {
							cargoit->second += take;
						} else {
							it_start->load_there[ppp.end] = take;
						}
						// b
						d = -1;
						fleet_.get_path(*ppp.start, *ppp.end, path);
						game.map().calc_cost(path, &d, nullptr);
						assert(d >= 0);
						plan.second.insert(it_after_end, SchedulingState(ppp.end, false, Duration(d)));
					} else if (static_cast<unsigned>(index_of_start + 1) == plan.second.size()) {
						// ship will visit start and nothing after that (but never end)
						// → a) add items for start and b) push a State to end
						const uint32_t take =
						   std::min(plan.first.get(game)->get_capacity(), ppp.open_count);
						sslog("Phase 5.4.B: PPP %u –> %u (open_count %u): assigning %u items to %s\n",
						      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
						      plan.first.get(game)->get_shipname().c_str());
						assert(take);
						ppp.open_count -= take;
						// a
						assert(plan.second.back().dock == ppp.start);
						assert(!plan.second.back().expedition);
						assert(plan.second.back().load_there.empty());
						plan.second.back().load_there[ppp.end] = take;
						// b
						int32_t d = -1;
						Path path;
						fleet_.get_path(*ppp.start, *ppp.end, path);
						game.map().calc_cost(path, &d, nullptr);
						assert(d >= 0);
						plan.second.push_back(SchedulingState(ppp.end, false, Duration(d)));
					}
				} else if (index_of_start < 0 && index_of_end < 0 &&
				           indices_near_start.count(plan.second.size() - 1)) {
					if (expedition >= 0) {
						continue;
					}
					// ship will visit a port close to start and nothing after that (but never start
					// or end)
					// → a) insert a new state with items for start and b) push a State to end
					const uint32_t take = std::min(plan.first.get(game)->get_capacity(), ppp.open_count);
					sslog("Phase 5.4.C: PPP %u –> %u (open_count %u): assigning %u items to %s\n",
					      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
					      plan.first.get(game)->get_shipname().c_str());
					assert(take);
					ppp.open_count -= take;
					// a
					Path path;
					int32_t d = -1;
					fleet_.get_path(*plan.second.back().dock.get(game), *ppp.start, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					plan.second.push_back(SchedulingState(ppp.start, false, Duration(d)));
					plan.second.back().load_there[ppp.end] = take;
					// b
					d = -1;
					fleet_.get_path(*ppp.start, *ppp.end, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					plan.second.push_back(SchedulingState(ppp.end, false, Duration(d)));
				} else if (index_of_start < 0 && index_of_end > 0 &&
				           indices_near_start.count(index_of_end - 1)) {
					if (expedition >= 0 && expedition < index_of_end) {
						continue;
					}
					// ship will visit a port close to start and directly afterwards end (but never
					// start)
					// → a) insert a new state with items for start, and b) update time for end
					auto it_end = plan.second.begin();
					auto it_before_end = plan.second.begin();
					for (int32_t i = index_of_end; i > 0; --i) {
						++it_end;
						if (i != index_of_end) {
							++it_before_end;
						}
					}
					const uint32_t capacity =
					   get_free_capacity_at(game, *plan.first.get(game), *it_before_end->dock.get(game));
					if (!capacity) {
						continue;
					}
					const uint32_t take = std::min(capacity, ppp.open_count);
					sslog("Phase 5.4.D: PPP %u –> %u (open_count %u): assigning %u items to %s\n",
					      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
					      plan.first.get(game)->get_shipname().c_str());
					assert(take);
					ppp.open_count -= take;
					// b
					Path path;
					int32_t d = -1;
					fleet_.get_path(*ppp.start, *ppp.end, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					it_end->duration_from_previous_location = Duration(d);
					// a
					d = -1;
					fleet_.get_path(*it_before_end->dock.get(game), *ppp.start, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					SchedulingState ss(ppp.start, false, Duration(d));
					ss.load_there[ppp.end] = take;
					plan.second.insert(it_end, ss);
				} else {
					for (uint32_t i_s : indices_near_start) {
						if (indices_near_end.count(i_s + 1)) {
							if (expedition >= 0 && expedition <= static_cast<int>(i_s)) {
								break;
							}
							// ship will visit a port close to start and directly afterwards a port close
							// to end (but never start or end)
							// → a) insert a new state with items for start and b) a new state for end, and
							// c) update the time for the state after end
							auto it_near_end = plan.second.begin();
							auto it_near_start = plan.second.begin();
							++it_near_end;
							for (uint32_t i = i_s; i; --i) {
								++it_near_end;
								++it_near_start;
							}
							const uint32_t capacity = get_free_capacity_at(
							   game, *plan.first.get(game), *it_near_start->dock.get(game));
							if (!capacity) {
								continue;
							}
							const uint32_t take = std::min(capacity, ppp.open_count);
							sslog("Phase 5.4.E: PPP %u –> %u (open_count %u): assigning %u items to %s\n",
							      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
							      plan.first.get(game)->get_shipname().c_str());
							assert(take);
							ppp.open_count -= take;
							// c
							Path path;
							int32_t d = -1;
							fleet_.get_path(*ppp.end, *it_near_end->dock.get(game), path);
							game.map().calc_cost(path, &d, nullptr);
							assert(d >= 0);
							it_near_end->duration_from_previous_location = Duration(d);
							// b
							d = -1;
							fleet_.get_path(*ppp.start, *ppp.end, path);
							game.map().calc_cost(path, &d, nullptr);
							assert(d >= 0);
							plan.second.insert(it_near_end, SchedulingState(ppp.end, false, Duration(d)));
							// a
							d = -1;
							fleet_.get_path(*it_near_start->dock.get(game), *ppp.start, path);
							game.map().calc_cost(path, &d, nullptr);
							assert(d >= 0);
							SchedulingState ss(ppp.start, false, Duration(d));
							ss.load_there[ppp.end] = take;
							plan.second.insert(++it_near_start, ss);
							break;
						}
					}
				}
			}
		}
	}

	/* SIXTH PASS:
	 * Make a list of all ships that are idle, and distribute them more or less evenly
	 * among ports: For each port, count how many ships are heading there or already
	 * located close by. Distribute the idle ships among the ports with the fewest
	 * ships: Send each ship to one of these ports (preferably a close by one).
	 */
	idle_ships.clear();
	std::list<std::pair<PortDock*, uint32_t>> ships_per_port;
	auto increment_ships_per_port = [](std::list<std::pair<PortDock*, uint32_t>>& s, PortDock* pd) {
		for (auto& pair : s) {
			if (pair.first == pd) {
				++pair.second;
				return;
			}
		}
		s.push_back(std::make_pair(pd, 1));
	};
	for (auto& plan : plans_) {
		if (plan.second.empty()) {
			idle_ships.push_back(plan.first.get(game));
			for (PortDock* dock : fleet_.get_ports()) {
				Path path;
				if (game.map().findpath(plan.first.get(game)->get_position(),
				                        dock->get_positions(game).back(), kNearbyDockMaxDistanceFactor,
				                        path, CheckStepDefault(MOVECAPS_SWIM)) >= 0) {
					increment_ships_per_port(ships_per_port, dock);
				}
			}
		} else {
			for (const SchedulingState& ss : plan.second) {
				increment_ships_per_port(ships_per_port, ss.dock.get(game));
			}
		}
	}
	sslog("SIXTH PASS: Found %" PRIuS " idle ships\n", idle_ships.size());
	for (Ship* ship : idle_ships) {
		std::list<PortDock*> candidates;
		uint32_t fewest = std::numeric_limits<uint32_t>::max();
		for (const auto& pair : ships_per_port) {
			fewest = std::min(fewest, pair.second);
		}
		for (PortDock* pd : fleet_.get_ports()) {
			bool is_candidate = true;
			for (auto& pair : ships_per_port) {
				if (pair.first == pd) {
					is_candidate = pair.second == fewest;
					break;
				}
			}
			if (is_candidate) {
				candidates.push_back(pd);
			}
		}
		assert(!candidates.empty());
		PortDock* closest = nullptr;
		int32_t dist = 0;
		for (PortDock* dock : candidates) {
			Path path;
			int32_t d = -1;
			ship->calculate_sea_route(game, *dock, &path);
			game.map().calc_cost(path, &d, nullptr);
			assert(d >= 0);
			if (!closest || d < dist) {
				dist = d;
				closest = dock;
			}
		}
		assert(closest);
		if (dist < kNearbyDockMaxDistanceFactor) {
			// Check for closest to make clang-tidy happy
			sslog("%s is already near %u\n", ship->get_shipname().c_str(),
			      closest ? closest->serial() : 0);
		} else {
			plans_[ship].push_back(SchedulingState(closest, false, Duration(dist)));
			ship->set_destination(game, closest);
			sslog("Sending %s to %u\n", ship->get_shipname().c_str(), closest->serial());
		}
		for (auto it = ships_per_port.begin(); it != ships_per_port.end(); ++it) {
			if (it->first == closest) {
				assert(it->second > 0);
				if (it->second > 1) {
					--it->second;
				} else {
					ships_per_port.erase(it);
				}
				break;
			}
		}
	}
	sslog("--- End of ShippingSchedule::update ---\n\n");
	return kFleetInterval;
}

void ShippingSchedule::log_general_info(const EditorGameBase& e) const {
	const Time& t = e.get_gametime();
	for (const auto& plan : plans_) {
		log_dbg_time(t, "· %s: carrying %u items (capacity %u)\n",
		             plan.first.get(e)->get_shipname().c_str(), plan.first.get(e)->get_nritems(),
		             plan.first.get(e)->get_capacity());
		std::map<Serial, uint32_t> dests;
		for (uint32_t i = plan.first.get(e)->get_nritems(); i; --i) {
			const Serial si = plan.first.get(e)->get_item(i - 1).destination_dock_.serial();
			auto it = dests.find(si);
			if (it == dests.end()) {
				dests[si] = 1;
			} else {
				++it->second;
			}
		}
		for (const auto& pair : dests) {
			log_dbg_time(t, "  – %u items to %u\n", pair.second, pair.first);
		}
		log_dbg_time(t, "  SCHEDULE: %" PRIuS " stations\n", plan.second.size());
		for (const SchedulingState& ss : plan.second) {
			log_dbg_time(t, "          · in %u ms at %u\n", ss.duration_from_previous_location.get(),
			             ss.dock.serial());
			log_dbg_time(t, "            load there: ");
			if (ss.expedition) {
				log_dbg_time(t, "expedition\n");
				assert(ss.load_there.empty());
			} else {
				log_dbg_time(t, "cargo for %" PRIuS " destinations\n", ss.load_there.size());
				for (const auto& pair : ss.load_there) {
					log_dbg_time(t, "            – %u items to %u\n", pair.second, pair.first.serial());
				}
			}
		}
	}
}

constexpr uint16_t kCurrentPacketVersion = 1;
void ShippingSchedule::save(const EditorGameBase& egbase,
                            MapObjectSaver& mos,
                            FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersion);

	last_updated_.save(fw);

	fw.unsigned_32(plans_.size());
	for (const auto& pair : plans_) {
		fw.unsigned_32(mos.get_object_file_index(*pair.first.get(egbase)));

		last_actual_duration_recalculation_.at(pair.first).save(fw);

		fw.unsigned_32(pair.second.size());
		for (const SchedulingState& ss : pair.second) {
			fw.unsigned_32(mos.get_object_file_index_or_zero(ss.dock.get(egbase)));
			ss.duration_from_previous_location.save(fw);
			fw.unsigned_8(ss.expedition ? 1 : 0);
			fw.unsigned_32(ss.load_there.size());
			for (const auto& cargo : ss.load_there) {
				fw.unsigned_32(mos.get_object_file_index_or_zero(cargo.first.get(egbase)));
				fw.unsigned_32(cargo.second);
			}
		}
	}
}

void ShippingSchedule::load(FileRead& fr) {
	assert(!loader_);
	loader_.reset(new ScheduleLoader());
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			last_updated_ = Time(fr);
			for (uint32_t nr_plans = fr.unsigned_32(); nr_plans; --nr_plans) {
				const Serial ship = fr.unsigned_32();
				loader_->recalc[ship] = Time(fr);
				std::list<SchedulingStateT<Serial, CargoListLoader>> states_for_this_ship;
				for (uint32_t nr_states = fr.unsigned_32(); nr_states; --nr_states) {
					const Serial dock = fr.unsigned_32();
					const Duration time(fr);
					const bool exp = fr.unsigned_8();
					SchedulingStateT<Serial, CargoListLoader> state(dock, exp, time);
					for (uint32_t nr_cargo = fr.unsigned_32(); nr_cargo; --nr_cargo) {
						const Serial dest = fr.unsigned_32();
						const Quantity nr = fr.unsigned_32();
						state.load_there[dest] = nr;
					}
					states_for_this_ship.push_back(state);
				}
				loader_->plan[ship] = states_for_this_ship;
			}
		} else {
			throw UnhandledVersionError("ShippingSchedule", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading shipping schedule: %s", e.what());
	}
}

void ShippingSchedule::load_pointers(MapObjectLoader& mol) {
	assert(loader_);
	last_actual_duration_recalculation_.clear();
	for (const auto& times : loader_->recalc) {
		last_actual_duration_recalculation_[&mol.get<Ship>(times.first)] = times.second;
	}
	for (const auto& plan : loader_->plan) {
		ShipPlan plan_for_this_ship;
		for (const auto& state_loader : plan.second) {
			SchedulingState state(&mol.get<PortDock>(state_loader.dock), state_loader.expedition,
			                      state_loader.duration_from_previous_location);
			for (const auto& pair : state_loader.load_there) {
				state.load_there[&mol.get<PortDock>(pair.first)] = pair.second;
			}
			plan_for_this_ship.push_back(state);
		}
		plans_[&mol.get<Ship>(plan.first)] = plan_for_this_ship;
	}
	loader_.reset(nullptr);
}

}  // namespace Widelands
