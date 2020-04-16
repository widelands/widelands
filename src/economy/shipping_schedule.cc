/*
 * Copyright (C) 2011-2019 by the Widelands Development Team
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

#include <set>

#include "economy/portdock.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/ship.h"

namespace Widelands {

ShippingSchedule::ShippingSchedule(ShipFleet& f) : fleet_(f), last_updated_(0), last_actual_durations_recalculation_(0) {
	assert(!fleet_.active());
}

void ShippingSchedule::ship_arrived(Game& game, Ship& ship, PortDock& port) {
	auto plan = plans_.find(&ship);
	assert(plan != plans_.end());
	if (plan.second.empty()) {
		// nothing to do
		assert(ship.get_nritems() == 0);
		ship.set_destination(game, nullptr);
	}

	const SchedulingState& ss = plan.second.front();
	assert(ss.dock == &port);

	if (ss.expedition) {
		assert(port.expedition_ready_);
		assert(ship.get_nritems() == 0);
		assert(plan.second.size() == 1); // no planning beyond the expedition
		assert(ss.load_there.empty());

		std::vector<Worker*> workers;
		std::vector<WareInstance*> wares;
		port.expedition_bootstrap_->get_waiting_workers_and_wares(game, port.owner().tribe(), &workers, &wares);
		for (Worker* worker : workers) {
			ship.add_item(game, ShippingItem(*worker));
		}
		for (WareInstance* ware : wares) {
			ship.add_item(game, ShippingItem(*ware));
		}
		ship.start_task_expedition(game);
		port.cancel_expedition(game);

		// The ship is technically not a part of the fleet any more.
		// It will re-add itself when the expedition is completed or cancelled.
		plans_.erase(plan);
		return;
	}

	assert(plan.second.size() >= 1 + ss.load_there.size()); // besides the current portdock we should at least visit all the destinations for which we are loading wares
	for (const auto& pair : ss.load_there) {
		assert(pair.first);
		assert(pair.first != &port);
		assert(pair.second > 0);
		for (uint32_t i = 0; i < pair.second; ++i) {
			if (!port.load_one_item(game, ship, *pair.first)) {
				// We planned for more items than we may take. Can happen when
				// transfers are cancelled in the last moment. Ignore.
				break;
			}
		}
	}

	plan.second.pop_front();
	ship.set_destination(game, plan.second.front().dock);
}

// `dock` is not a dangling reference yet, but this function is called
// via `ShipFleet::remove_port()` from `PortDock::cleanup()`
void ShippingSchedule::port_removed(Game& game, PortDock* dock) {
	// Find all ships planning to visit this dock and reroute them.
	std::vector<Ship*> ships_heading_there;
	for (auto& pair : plans_) {
		ShipPlan::iterator iterator_to_deleted_dock = pair.second.end();
		ShipPlan::iterator iterator_before_deleted_dock = pair.second.begin();
		for (ShipPlan::iterator it = pair.second.begin(); it != pair.second.end(); ++it) {
			if (it->dock == dock) {
				iterator_to_deleted_dock = it;
				break;
			}
			++iterator_before_deleted_dock;
		}
		if (iterator_to_deleted_dock != pair.second.end()) {
			ships_heading_there.push_back(pair.first);
			if (iterator_to_deleted_dock == pair.second.begin()) {
				// reroute to next dock
				pair.second.pop_front();
				if (pair.second.empty()) {
					if (pair.first->get_nritems()) {
						// no other docks to visit, but wares left, reroute to the closest one
						if (fleet_.get_ports().empty()) {
							// PANIC! There are no ports at all left!!
							// But we still have cargo!!! What should we do????
							// Stay calm. Just do nothing. Nothing at all.
							log("Ship %s is carrying %u items and there are no ports left\n", pair.first->get_shipname().c_str(), pair.first->get_nritems());
							pair.first->set_destination(game, nullptr);
						}
						PortDock* closest = nullptr;
						uint32_t dist = 0;
						for (const PortDock& pd : fleet_.get_ports()) {
							Path path;
							uint32_t d;
							pair.first->calculate_sea_route(game, pd, &path);
							game.map().calc_cost(path, &d, nullptr);
							if (!closest || d < dist) {
								dist = d;
								closest = &pd;
							}
						}
						assert(closest);
					}
				} else {
					pair.first->set_destination(game, pair.second.front().dock);
					Path path;
					pair.first->calculate_sea_route(game, pair.second.front().dock, &path);
					game.map().calc_cost(path, &pair.second.front().duration_from_previous_location, nullptr);
				}
			} else {
				// no rerouting needed, just recalc the schedule time
				iterator_to_deleted_dock = pair.second.erase(iterator_to_deleted_dock);
				// points now to the dock after the deleted dock
				if (iterator_to_deleted_dock != pair.second.end()) {
					Path path;
					fleet_.get_path(*iterator_before_deleted_dock->dock, *iterator_to_deleted_dock->dock, path)
					game.map().calc_cost(path, iterator_to_deleted_dock->duration_from_previous_location, nullptr);
				}
			}
			
		}
	}

	// Find all shippingitems heading for the deleted dock.
	// Those in warehouses can just be told to recalculate their route.
	// Those on ships will be rerouted to whichever portdock the ships will visit next;
	// they will be unloaded there and then recalculate their route.

	for (const PortDock& pd : fleet_.get_ports()) {
		for (auto it = pd.waiting_.begin(); it != dock.waiting_.end(); ++it) {
			if (it->destination_dock_.serial() == dock->serial()) {
				it->set_location(game, pd.warehouse_);
				it->end_shipping(game);
				it = pd.waiting_.erase(it);
			} else {
				++it;
			}
		}
	}

	for (Ship* ship : ships_heading_there) {
		for (ShippingItem& si : ship->items_) {
			if (si.destination_dock_.serial() == dock->serial()) {
				si.destination_dock_ = ship->get_destination();
			}
		}
	}
}

void ShippingSchedule::ship_removed(const Game&, Ship* ship) {
	auto it = plans_.find(ship);
	assert(it != plans_.end());
	plans_.erase(it);
	// Handling any items that were intended to be transported by this ship
	// is deferred to the next call to update()
}

void ShippingSchedule::update(Game& game) {
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
	 * makes sense), or leave them for now for lack of capacity.
	 * Additionally, we will distribute idle ships more or less equally among ports
	 * so that every port will have a ship at hand immediately when it requires one
	 * (provided that we have enough ships, of course).
	 * In theory, it would be good to implement intelligent reordering of ships'
	 * destinations, so as to avoid routes like A-B-C where A and C are close and B
	 * is far away. We used to have such an algorithm, but it was shown to
	 * necessarily be a performance killer (I only say Travelling Salesman Problem),
	 * so we dropped support for this approach and instead prefer the GOLDEN RULE that
	 * a ship should never service too many destinations at once, regardless of their
	 * relative distances. One ship from B to A plus one ship from B to C are better
	 * than one ship from B to both A and C. Instead we prefer to distribute tasks
	 * among many ships. This produces the best results when the player builds a large
	 * naval force. (When the player has few ships for many ports, this approach will
	 * work suboptimally, but that is a bad strategy so the player deserves no more.)
	 */

	/* FIRST PASS:
	 * Scan all ships. Refresh the prediction when they will arrive at the next port.
	 * Most of the time, a simple estimate is enough.
	 * Now and then (every 20s), we calculate the exact time though to account for
	 * delays resulting e.g. from ships stopping to let another ship pass.
	 */
	constexpr uint32_t kActualDurationsRecalculationInterval = 20000;
	const uint32_t time = game.get_gametime();
	const uint32_t time_since_last_update = time - last_updated_;
	if (time - last_actual_durations_recalculation_ > kActualDurationsRecalculationInterval) {
		for (auto& pair : plans_) {
			if (!pair.second.empty()) {
				Path path;
				pair.first->calculate_sea_route(game, *pair.second.front().dock, &path);
				game.map().calc_cost(path, &pair.second.front().duration_from_previous_location, nullptr);
			}
		}
		last_actual_durations_recalculation_ = time;
	} else {
		for (auto& pair : plans_) {
			if (!pair.second.empty()) {
				if (pair.second.front().duration_from_previous_location > time_since_last_update) {
					pair.second.front().duration_from_previous_location -= time_since_last_update;
				} else {
					// She said five more seconds, and that was ten seconds ago…
					// The ship is behind schedule, so this is an
					// arbitrary estimate about the arrival time.
					pair.second.front().duration_from_previous_location /= 2;
				}
			}
		}
	}
	last_updated_ = time;

	/* SECOND PASS:
	 * Scan all ports. Make lists of waiting items.
	 * Figure out when the items will be picked up.
	 * Also cancel orders where we provided more capacity than is actually needed.
	 * (This can happen when a transfer is cancelled when the item is still in the portdock.)
	 */
	const size_t nr_ports = fleet_.get_ports().size();

#ifndef NDEBUG
	for (const auto& plan : plans_) {
		assert(plan.second.size() <= nr_ports);
	}
#endif

	if (nr_ports == 0) {
		// Nothing to do. Ships stay where they are, or do whatever they want.
		return;
	}
	std::list<Ship*> ships_with_reduced_orders;
	// Don't even think about trying to cache any of this. It is impossible to maintain.
	std::map<PortDock* /* start */,
	         std::map<PortDock* /* dest */,
	                  std::pair<std::map<Ship* /* by whom */,
	                                     std::pair<Duration /* when at `start` */,
	                                               Quantity /* accept how much from `start` to `dest` */
	                            int32_t /* capacity missing (-) or extra (+) */
	        >>>>> items_in_ports;
	for (PortDock& dock : fleet_.get_ports()) {
		std::map<PortDock*, std::pair<std::map<Ship*, std::pair<Duration, Quantity>>, int32_t>> map;
		for (const auto& plan : plans_) {
			Duration eta = 0;
			CargoList* load = nullptr;
			for (const SchedulingState& ss : plan.second) {
				eta += ss.duration_from_previous_location;
				if (ss.dock == &dock) {
					load = &ss.load_there;
					break;
				}
			}
			if (load) {
				map[cargo.first].second = 0;
				for (const auto& cargo : *load) {
					map[cargo.first].first[plan.first] = std::make_pair(eta, cargo.second);
				}
			}
		}

		for (PortDock& dest : fleet_.get_ports()) {
			int32_t waiting_items = dock.count_waiting(&dest);
			std::multiset<Duration> arrival_times; // one entry per item that will be picked up
			for (const auto& pair : map[&dest]) {
				for (uint32_t i = pair.second.second; i; --i) {
					arrival_times.insert(pair.second.first);
				}
			}

			const int32_t planned_capacity = arrival_times.size();
#ifndef NDEBUG
			if (&dock == &dest) {
				assert(waiting_items = 0);
				assert(planned_capacity == 0);
			}
#endif
			int32_t delta = planned_capacity - waiting_items;
			while (delta > 0) {
				// reduce or cancel the last order in the queue
				const uint32_t last_arrival = *arrival_times.crbegin();
				Ship* ship = nullptr;
				for (auto pair_it = map[&dest].first.first.begin(); pair_it != map[&dest].first.first.end(); ++pair_it) {
					assert(pair_it->second.first <= last_arrival);
					if (pair_it->second.first == last_arrival) {
						uint32_t reducedby;
						bool erase = false;
						// cancel in the overview…
						if (pair_it->second.second > static_cast<unsigned<(delta)) {
							reducedby = delta;
							pair_it->second.second -= delta;
							delta = 0;
						} else {
							reducedby = pair_it->second.second;
							delta -= pair_it->second.second;
							pair_it->second.second = 0;
							erase = true;
						}
						for (uint32_t i = reducedby; i; --i) {
							assert(*arrival_times.rbegin() == last_arrival);
							arrival_times.erase(arrival_times.rbegin());
						}
						// …and in the schedule
						bool found = false;
						for (SchedulingState& ss : plans_.at(pair_it->first)) {
							if (ss.dock == &dock) {
								for (auto it = ss.load_there.begin(); it != ss.load_there.end(); ++it) {
									if (it->first == &dest) {
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
						ships_with_reduced_orders.push_back(pair_it->first);
						if (erase) {
							map[&dest].first.first.erase(pair_it);
						}
						break;
					}
				}
			}
			map[&dest].first = delta;
		}
		items_in_ports[&dock] = map;
	}

	/* THIRD PASS:
	 * Go through the list of ships that had orders cancelled, and check whether we might
	 * even skip some of their destinations altogether.
	 */

	
	#nocom
	

	/* FOURTH PASS:
	 * Go through the lists of start-end pairs where we need more capacity.
	 * For each pair, check it a ship is coming that will visit the destination
	 * shortly afterwards and still has capacity for more items.
	 * If so, we'll assign the extra capacity to this ship – but only if the ship
	 * will go straight or with a very little detour from here to there, and the
	 * time from now to this ship's arrival here is not too high. If this
	 * condition is not met, make a note of the ship and the expected arrival
	 * time at the destination anyway.
	 * If we didn't assign the entire required capacity yet, look for idle ships
	 * and assign one or more of them (preferably the closest ones) the task of
	 * transporting those items.
	 * If we still have open demand for capacity then, also accept the ships we
	 * didn't like at first.
	 * And if that still isn't enough, check if there are other ports within a
	 * low radius of the start and end ports, and also accept ships that have
	 * a destination in the start group *directly followed by* a destination in
	 * the end group, and has free capacity between these destinations, and tell
	 * such a ship to additionally visit the start and end port between its two
	 * existing targets.
	 */

	

	
	#nocom
	
	
	/* FIFTH PASS:
	 * Make a list of all ships that are idle, and distribute them more or less evenly among ports:
	 * For each port, count how many ships are heading there or already located close by.
	 * Distribute the number of idle ships among the ports with the fewest ships.
	 * Send each ship to one of these ports (preferably a close by one).
	 */

	#nocom
	
}

}
