/*
 * Copyright (C) 2019-2020 by the Widelands Development Team
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

// Performance tradeoff in the First Pass
constexpr uint32_t kActualDurationsRecalculationInterval = 20 * 1000;

// Give a ship the highest score for assigning wares to it if it will
// arrive at the port of interest within the next 10 s,
// or the lowest score if this will take longer than 10 min
constexpr Duration kWonderfullyShortDuration = 10 * 1000;   // 10 s
constexpr Duration kHorriblyLongDuration = 10 * 60 * 1000;  // 10 min

// Only assign wares to a ship in 5.1 if it's score is higher than a certain
// threshold based on minimal distance. Ships with lower scores will only be
// accepted if not enough idle ships are found.
constexpr uint16_t kMinScoreForImmediateAcceptFactor = 100;

// Average sailing-time distance for two ports to be considered "close by" in 5.4
constexpr int16_t kDockGroupMaxDistanceFactor = 12 * 1800;

// Distributing idle ships between ports in the Sixth Pass:
// Sailing-time distance between a ship and a port so that the ship is considered
// to be within convenient reach when the port needs a ship
constexpr int16_t kNearbyDockMaxDistanceFactor = 8 * 1800;

/*******************************************************************************
                             Actual implementation
*******************************************************************************/

// #define sslog(...) if (g_verbose) log(__VA_ARGS__);
#define sslog(...) log("NOCOM: " __VA_ARGS__);  // NOCOM

ShippingSchedule::ShippingSchedule(ShipFleet& f)
   : fleet_(f), last_updated_(0), last_actual_durations_recalculation_(0), loader_(nullptr) {
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

void ShippingSchedule::ship_arrived(Game& game, Ship& ship, PortDock& port) {
	sslog(
	   "\nShippingSchedule::ship_arrived(%s at %u)\n", ship.get_shipname().c_str(), port.serial());
	auto plan = plans_.find(&ship);
	assert(plan != plans_.end());
	const size_t plan_size = plan->second.size();
	assert(plan_size);

	const SchedulingState& ss = plan->second.front();
	assert(ss.dock == &port);

	if (ss.expedition) {
		sslog("Loading expedition\n\n");
		assert(port.expedition_ready_);
		assert(ship.get_nritems() == 0);
		assert(plan_size == 1);  // no planning beyond the expedition
		assert(ss.load_there.empty());

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
		// It will call ship_removed() now, erasing `plan` from `plans_`.
		// The ship will re-add itself when the expedition is completed or cancelled.
		return;
	}

	assert(plan_size >= 1 + ss.load_there.size());  // besides the current portdock we
	                                                // should at least visit all the
	                                                // destinations for which we are
	                                                // loading wares
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

	plan->second.pop_front();
	if (plan_size > 1) {
		ship.set_destination(game, plan->second.front().dock);
		sslog("Loaded cargo and sending to %u\n\n", plan->second.front().dock->serial());
	} else {
		ship.set_destination(game, nullptr);
		assert(ship.get_nritems() == 0);
		sslog("Loaded nothing, idle now\n\n");
	}
}

// `dock` is not a dangling reference yet, but this function is called
// via `ShipFleet::remove_port()` from `PortDock::cleanup()`
void ShippingSchedule::port_removed(Game& game, PortDock* dock) {
	sslog("\nShippingSchedule::port_removed (%u)\n", dock->serial());
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
					// no other docks to visit, but wares left, reroute to the closest one
					if (fleet_.get_ports().empty() || pair.first->get_nritems() == 0) {
						// PANIC! There are no ports at all left!!
						// But we still have cargo!!! What should we do????
						// Stay calm. Just do nothing. Nothing at all.
						log("Ship %s is carrying %u items OR there are no ports left, setting NO "
						    "destination\n",
						    pair.first->get_shipname().c_str(), pair.first->get_nritems());
						pair.first->set_destination(game, nullptr);
					} else {
						PortDock* closest = nullptr;
						int32_t dist = 0;
						for (PortDock* pd : fleet_.get_ports()) {
							Path path;
							int32_t d = -1;
							pair.first->calculate_sea_route(game, *pd, &path);
							game.map().calc_cost(path, &d, nullptr);
							assert(d >= 0);
							if (!closest || d < dist) {
								dist = d;
								closest = pd;
							}
						}
						assert(closest);
						log("Ship %s is carrying %u items, rerouting to NEW destination %u\n",
						    pair.first->get_shipname().c_str(), pair.first->get_nritems(),
						    closest->serial());
						pair.second.push_back(SchedulingState(closest, false, dist));
						pair.first->set_destination(game, closest);
					}
				} else {
					pair.first->set_destination(game, pair.second.front().dock);
					sslog("Rerouted %s to %u\n", pair.first->get_shipname().c_str(),
					      pair.second.front().dock->serial());
					Path path;
					pair.first->calculate_sea_route(game, *pair.second.front().dock, &path);
					int32_t d = -1;
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					pair.second.front().duration_from_previous_location = d;
				}
			} else {
				sslog(
				   "no rerouting for %s, only recalc schedule\n", pair.first->get_shipname().c_str());
				// no rerouting needed, just recalc the schedule time
				iterator_to_deleted_dock = pair.second.erase(iterator_to_deleted_dock);
				// points now to the dock after the deleted dock
				if (iterator_to_deleted_dock != pair.second.end()) {
					Path path;
					fleet_.get_path(
					   *iterator_before_deleted_dock->dock, *iterator_to_deleted_dock->dock, path);
					int32_t d = -1;
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					iterator_to_deleted_dock->duration_from_previous_location = d;
				}
			}
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
	// Handling any items that were intended to be transported by this ship
	// is deferred to the next call to update()
}

void ShippingSchedule::ship_added(Game& game, Ship& s) {
	sslog("\nShippingSchedule::ship_added (%s)\n", s.get_shipname().c_str());
	assert(!s.get_destination());
	plans_[&s] = ShipPlan();
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
	plans_[&s].push_back(SchedulingState(closest, false, dist));
	s.set_destination(game, closest);
	sslog("Sent to %u\n\n", closest->serial());
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

	assert(plans_.size() == fleet_.get_ships().size());
	if (fleet_.get_ships().empty()) {
		return endless();  // wait until we get a ship
	}

	/* FIRST PASS:
	 * Scan all ships. Refresh the prediction when they will arrive at the next port.
	 * Most of the time, a simple estimate is enough.
	 * Now and then (every 20s), we calculate the exact time though to account for
	 * delays resulting e.g. from ships stopping to let another ship pass.
	 */
	const uint32_t time = game.get_gametime();
	const uint32_t time_since_last_update = time - last_updated_;
	sslog("\nShippingSchedule::update at %u (last %u, %u)\n", time, last_updated_,
	      last_actual_durations_recalculation_);
	if (time - last_actual_durations_recalculation_ > kActualDurationsRecalculationInterval) {
		for (auto& pair : plans_) {
			if (!pair.second.empty()) {
				Path path;
				pair.first->calculate_sea_route(game, *pair.second.front().dock, &path);
				int32_t d = -1;
				game.map().calc_cost(path, &d, nullptr);
				assert(d >= 0);
				pair.second.front().duration_from_previous_location = d;
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
					// The ship is behind schedule, so this is an arbitrary estimate
					// about the arrival time. Doesn't matter if it's inaccurate,
					// the ship will most likely arrive within a few seconds.
					pair.second.front().duration_from_previous_location /= 2;
				}
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
	const size_t nr_ports = fleet_.get_ports().size();
	sslog("SECOND PASS: %" PRIuS " ports\n", nr_ports);

#ifndef NDEBUG
	for (const auto& plan : plans_) {
		assert(plan.second.size() <= nr_ports);
	}
#endif

	if (nr_ports == 0) {
		// Nothing to do. Ships stay where they are, or do whatever they want.
		return endless();
	}
	std::list<Ship*> ships_with_reduced_orders;
	std::list<PortDock*> ports_with_unserviced_expeditions;
	// Don't even think about trying to cache any of this. It is impossible to maintain.
	std::map<
	   OPtr<PortDock> /* start */,
	   std::map<OPtr<PortDock> /* dest */,
	            std::pair<std::map<OPtr<Ship> /* by whom */,
	                               std::pair<Duration /* when at `start` */,
	                                         Quantity /* accept how much from `start` to `dest` */>>,
	                      int32_t /* capacity missing (-) or extra (+) */>>>
	   items_in_ports;
	for (PortDock* dock : fleet_.get_ports()) {
		const bool expedition_ready = dock->is_expedition_ready();
		sslog("Iteration: dock %u (expedition ready %s)\n", dock->serial(),
		      expedition_ready ? "true" : "false");
		Ship* expedition_ship_coming = nullptr;
		std::map<OPtr<PortDock>, std::pair<std::map<OPtr<Ship>, std::pair<Duration, Quantity>>, int32_t>> map;
		for (auto& plan : plans_) {
			Duration eta = 0;
			CargoList* _load = nullptr;
			for (SchedulingState& ss : plan.second) {
				eta += ss.duration_from_previous_location;
				if (ss.dock == dock) {
					if (ss.expedition) {
						assert(!expedition_ship_coming);
						expedition_ship_coming = plan.first;
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
				const uint32_t last_arrival = *arrival_times.crbegin();
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
			map[dest].second = delta;
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
		for (auto it = plans_[ship].begin(); it != plans_[ship].end();) {
			if (it->load_there.empty() && !it->expedition) {
				it = plans_[ship].erase(it);
				if (it != plans_[ship].end()) {
					if (previt == plans_[ship].end()) {
						Path path;
						ship->calculate_sea_route(game, *it->dock, &path);
						int32_t d = -1;
						game.map().calc_cost(path, &d, nullptr);
						assert(d >= 0);
						it->duration_from_previous_location = d;
					} else {
						Path path;
						fleet_.get_path(*previt->dock, *it->dock, path);
						int32_t d = -1;
						game.map().calc_cost(path, &d, nullptr);
						assert(d >= 0);
						it->duration_from_previous_location = d;
					}
				}
			} else {
				previt = it;
				++it;
			}
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
				sslog("assigning to %s\n", plan.first->get_shipname().c_str());
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
				if (plan.first->get_nritems() == 0) {
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
				assert(plan.first->get_nritems() == 0);
				ships_for_expeditions.push_back(plan.first);
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
			plans_[ship].push_back(SchedulingState(*closest, true, dist));
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
	struct ScoredShip {
		Ship* ship;
		uint32_t score;
		uint32_t capacity;
		Duration eta;

		static inline uint32_t calc_score(uint32_t c, Duration eta) {
			return eta > kHorriblyLongDuration ? 0 : kMinScoreForImmediateAcceptFactor * c *
			                                            kHorriblyLongDuration /
			                                            std::max(eta, kWonderfullyShortDuration);
		}
		ScoredShip(Ship* s, uint32_t c, Duration d)
		   : ship(s), score(calc_score(c, d)), capacity(c), eta(d) {
		}
		ScoredShip(const ScoredShip&) = default;
		ScoredShip& operator=(const ScoredShip&) = default;
		// "smaller" comparison means "better"
		bool operator<(const ScoredShip& ss) const {
			if (score != ss.score) {
				return score > ss.score;
			}
			if (eta != ss.eta) {
				return eta < ss.eta;
			}
			if (capacity != ss.capacity) {
				return capacity > ss.capacity;
			}
			return ship->serial() < ss.ship->serial();
		}
		~ScoredShip() {
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
		~PrioritisedPortPair() {
		}

		PortDock* start;
		PortDock* end;
		uint32_t open_count;
		uint32_t priority;

		// cache for the functions below
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
		};
	};
	std::set<PrioritisedPortPair> _open_pairs;
	for (auto& start__map : items_in_ports) {
		for (auto& dest__shipsinfos : start__map.second) {
			assert(dest__shipsinfos.second.second <= 0);
			if (dest__shipsinfos.second.second < 0) {
				const int32_t maxprio =
				   start__map.first.get(game)->calc_max_priority(game, *dest__shipsinfos.first.get(game));
				const int32_t total_waiting = start__map.first.get(game)->count_waiting(dest__shipsinfos.first.get(game));
				const int32_t open = -dest__shipsinfos.second.second;
				assert(total_waiting >= open);
				assert(maxprio >= total_waiting);  // a priority of at least 1 per item
				const int32_t prio = maxprio * open / total_waiting;
				assert(prio >= 0);
				_open_pairs.insert(
				   PrioritisedPortPair(start__map.first.get(game), dest__shipsinfos.first.get(game), open, prio));
			}
		}
	}

	// Shared logic for steps 1 and 3, pulled out as a lambda function.
	auto load_on_ship = [this, &game](PrioritisedPortPair& ppp) {
		const uint32_t take = std::min(ppp.open_count, ppp.ships.front().capacity);
		sslog("load_on_ship: PPP %u –> %u (open_count %u): assigning %u items (capacity %u) to %s\n",
		      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
		      ppp.ships.front().capacity, ppp.ships.front().ship->get_shipname().c_str());
		assert(take);
		for (auto ss = plans_[ppp.ships.front().ship].begin();
		     ss != plans_[ppp.ships.front().ship].end(); ++ss) {
			assert(!ss->expedition);
			if (ss->dock != ppp.start) {
				continue;
			}
			bool found = false;
			for (auto& pair : ss->load_there) {
				if (pair.first == ppp.end) {
					pair.second += take;
					found = true;
					break;
				}
			}
			if (!found) {
				ss->load_there.push_back(std::make_pair(ppp.end, take));
			}

			auto next_dest = ss;
			++next_dest;
			int32_t d = -1;
			Path path;
			fleet_.get_path(*ppp.start, *ppp.end, path);
			game.map().calc_cost(path, &d, nullptr);
			assert(d >= 0);
			if (next_dest == plans_[ppp.ships.front().ship].end()) {
				// just push the next dest (end)
				plans_[ppp.ships.front().ship].insert(next_dest, SchedulingState(ppp.end, false, d));
			} else if (next_dest->dock != ppp.end) {
				// push next dest (end) and adjust the duration of the one after that
				d = -1;
				fleet_.get_path(*ppp.end, *next_dest->dock, path);
				game.map().calc_cost(path, &d, nullptr);
				assert(d >= 0);
				next_dest->duration_from_previous_location = d;
				++next_dest;
				plans_[ppp.ships.front().ship].insert(next_dest, SchedulingState(ppp.end, false, d));
			}
			break;
		}
		ppp.ships.front().capacity -= take;
		ppp.open_count -= take;
		if (!ppp.ships.front().capacity) {
			ppp.ships.erase(ppp.ships.begin());
		}
	};
	// Helper function to determine how much capacity the given ship will have
	// after at the given port. Returns 0 if the ship is not planning to go
	// there or will launch an expedition from there.
	auto get_free_capacity_at = [this, &game](Ship& ship, PortDock& dock) {
		assert(plans_.find(&ship) != plans_.end());
		CargoList cargo_tracker;
		for (const ShippingItem& si : ship.items_) {
			PortDock* dest = si.destination_dock_.get(game);
			bool found = false;
			for (auto& pair : cargo_tracker) {
				if (pair.first == dest) {
					++pair.second;
					found = true;
					break;
				}
			}
			if (!found) {
				cargo_tracker.push_back(std::make_pair(dest, 1));
			}
		}
		for (const SchedulingState& ss : plans_[&ship]) {
			if (ss.expedition) {
				return 0u;
			}
			for (auto unload = cargo_tracker.begin(); unload != cargo_tracker.end(); ++unload) {
				if (unload->first == ss.dock) {
					cargo_tracker.erase(unload);
					break;
				}
			}
			for (const auto& load : ss.load_there) {
				bool found = false;
				for (auto& pair : cargo_tracker) {
					if (pair.first == load.first) {
						pair.second += load.second;
						found = true;
						break;
					}
				}
				if (!found) {
					cargo_tracker.push_back(load);
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
		return 0u;
	};

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
			bool found_wrong_end = false;
			Duration arrival_time = 0;
			for (SchedulingState& ss : plan.second) {
				if (found_start) {
					if (ss.dock == ppp.end) {
						found_end = true;
					} else {
						found_wrong_end = true;
					}
					break;
				} else {
					if (ss.dock == ppp.end) {
						// goes to the end point before visiting the start point :(
						break;
					}
					arrival_time += ss.duration_from_previous_location;
				}
				if (ss.dock == ppp.start) {
					found_start = true;
				}
			}
			const uint32_t free_capacity = get_free_capacity_at(*plan.first, *ppp.start);
			assert(free_capacity <= plan.first->get_capacity());
			if (found_start && free_capacity && (found_end || !found_wrong_end)) {
				sslog("Phase 5.1: PPP %u –> %u (open_count %u): assigning up to %u items to %s\n",
				      ppp.start->serial(), ppp.end->serial(), ppp.open_count, free_capacity,
				      plan.first->get_shipname().c_str());
				_ships.insert(ScoredShip(plan.first, free_capacity, arrival_time));
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
			load_on_ship(ppp);
		}
		sslog("Phase 5.1: PPP %u –> %u: %u open_count remaining\n", ppp.start->serial(),
		      ppp.end->serial(), ppp.open_count);
	}

	// 2) assign idle ships
	std::list<Ship*> idle_ships;
	for (auto& plan : plans_) {
		if (plan.second.empty() ||
		    (plan.second.size() == 1 && !plan.second.front().expedition &&
		     plan.second.front().load_there.empty() && plan.first->get_nritems() == 0)) {
			idle_ships.push_back(plan.first);
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
			plans_[closest].push_back(SchedulingState(ppp.start, false, dist));
			plans_[closest].front().load_there.push_back(std::make_pair(ppp.end, take));
			closest->set_destination(game, ppp.start);
			ppp.open_count -= take;
			dist = -1;
			Path path;
			fleet_.get_path(*ppp.start, *ppp.end, path);
			game.map().calc_cost(path, &dist, nullptr);
			assert(dist >= 0);
			plans_[closest].push_back(SchedulingState(ppp.end, false, dist));
			idle_ships.erase(std::find(idle_ships.begin(), idle_ships.end(), closest));
		}
	}

	// 3) accept suboptimal ships already heading here
	std::list<PortDock*> open_count_left;
	for (PrioritisedPortPair& ppp : open_pairs) {
		while (ppp.open_count && !ppp.ships.empty()) {
			sslog("Phase 5.3: PPP %u –> %u (open_count %u): assigning items…\n", ppp.start->serial(),
			      ppp.end->serial(), ppp.open_count);
			load_on_ship(ppp);
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
				const auto end = plan.second.end();
				ShipPlan::iterator dock1 = end;
				ShipPlan::iterator dock2 = end;
				for (auto ss = plan.second.begin(); ss != end; ++ss) {
					if (dock1 != end && groups.at(ppp.end).count(ss->dock)) {
						dock2 = ss;
						break;
					} else {
						if (ss->expedition) {
							break;
						} else if (groups.at(ppp.start).count(ss->dock)) {
							dock1 = ss;
						} else {
							// goes there, but goes straight away again
							dock1 = end;
							break;
						}
					}
				}
				if (dock1 == end) {
					// not planning to go anywhere near where we need it
					assert(dock2 == end);
					continue;
				}
				const uint32_t capacity = get_free_capacity_at(*plan.first, *dock1->dock);
				if (!capacity) {
					continue;
				}
				const uint32_t take = std::min(capacity, ppp.open_count);
				sslog("Phase 5.4: PPP %u –> %u (open_count %u): assigning %u items to %s\n",
				      ppp.start->serial(), ppp.end->serial(), ppp.open_count, take,
				      plan.first->get_shipname().c_str());
				ppp.open_count -= take;

				std::unique_ptr<SchedulingState> state__start(nullptr);
				std::unique_ptr<SchedulingState> state__end(nullptr);
				int32_t update_dock2_time = -1;
				// Four cases
				if (dock1->dock == ppp.start) {
					assert(dock2 != end);
					assert(dock2->dock != ppp.end);

					Path path;
					fleet_.get_path(*ppp.end, *dock2->dock, path);
					game.map().calc_cost(path, &update_dock2_time, nullptr);
					assert(update_dock2_time >= 0);

					int32_t d = -1;
					fleet_.get_path(*ppp.start, *ppp.end, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					state__end.reset(new SchedulingState(ppp.end, false, d));

					bool found = false;
					for (auto& cargo : dock1->load_there) {
						if (cargo.first == ppp.end) {
							cargo.second += take;
							found = true;
							break;
						}
					}
					if (!found) {
						dock1->load_there.push_back(std::make_pair(ppp.end, take));
					}
				} else if (dock2 != end && dock2->dock == ppp.end) {
					Path path;
					fleet_.get_path(*ppp.start, *ppp.end, path);
					game.map().calc_cost(path, &update_dock2_time, nullptr);
					assert(update_dock2_time >= 0);

					int32_t d = -1;
					fleet_.get_path(*dock1->dock, *ppp.start, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					state__start.reset(new SchedulingState(ppp.start, false, d));
					state__start->load_there.push_back(std::make_pair(ppp.end, take));
				} else {
					if (dock2 != end) {
						Path path;
						fleet_.get_path(*ppp.end, *dock2->dock, path);
						game.map().calc_cost(path, &update_dock2_time, nullptr);
						assert(update_dock2_time >= 0);
					}

					int32_t d = -1;
					Path path;
					fleet_.get_path(*dock1->dock, *ppp.start, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					state__start.reset(new SchedulingState(ppp.start, false, d));
					state__start->load_there.push_back(std::make_pair(ppp.end, take));

					d = -1;
					fleet_.get_path(*ppp.start, *ppp.end, path);
					game.map().calc_cost(path, &d, nullptr);
					assert(d >= 0);
					state__end.reset(new SchedulingState(ppp.end, false, d));
				}

				assert(state__start || state__end);
				if (update_dock2_time >= 0) {
					assert(dock2 != end);
					dock2->duration_from_previous_location = update_dock2_time;
				}
				if (state__end) {
					plan.second.insert(dock1, *state__end);
				}
				if (state__start) {
					plan.second.insert(dock1, *state__start);
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
			idle_ships.push_back(plan.first);
			for (PortDock* dock : fleet_.get_ports()) {
				Path path;
				if (game.map().findpath(plan.first->get_position(), dock->get_positions(game).back(),
				                        kNearbyDockMaxDistanceFactor, path,
				                        CheckStepDefault(MOVECAPS_SWIM)) >= 0) {
					increment_ships_per_port(ships_per_port, dock);
				}
			}
		} else {
			for (const SchedulingState& ss : plan.second) {
				increment_ships_per_port(ships_per_port, ss.dock);
			}
		}
	}
	sslog("SIXTH PASS: Found %" PRIuS " idle ships\n", idle_ships.size());
	for (Ship* ship : idle_ships) {
		std::list<PortDock*> candidates;
		uint32_t fewest = std::numeric_limits<uint32_t>::max();
		for (const auto& pair : ships_per_port) {
			if (pair.second < fewest) {
				fewest = pair.second;
				candidates.clear();
			}
			if (pair.second == fewest) {
				candidates.push_back(pair.first);
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
			sslog("%s is already near %u\n", ship->get_shipname().c_str(), closest->serial());
		} else {
			plans_[ship].push_back(SchedulingState(closest, false, dist));
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

constexpr uint16_t kCurrentPacketVersion = 1;
void ShippingSchedule::save(const EditorGameBase&, MapObjectSaver& mos, FileWrite& fw) const {
	fw.unsigned_16(kCurrentPacketVersion);

	fw.unsigned_32(last_updated_);
	fw.unsigned_32(last_actual_durations_recalculation_);

	fw.unsigned_32(plans_.size());
	for (const auto& pair : plans_) {
		fw.unsigned_32(mos.get_object_file_index(*pair.first));
		fw.unsigned_32(pair.second.size());
		for (const SchedulingState& ss : pair.second) {
			fw.unsigned_32(mos.get_object_file_index(*ss.dock));
			fw.unsigned_32(ss.duration_from_previous_location);
			fw.unsigned_8(ss.expedition ? 1 : 0);
			fw.unsigned_32(ss.load_there.size());
			for (const auto& cargo : ss.load_there) {
				fw.unsigned_32(mos.get_object_file_index(*cargo.first));
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
			last_updated_ = fr.unsigned_32();
			last_actual_durations_recalculation_ = fr.unsigned_32();
			for (uint32_t nr_plans = fr.unsigned_32(); nr_plans; --nr_plans) {
				const Serial ship = fr.unsigned_32();
				std::list<SchedulingStateT<Serial, CargoListLoader>> states_for_this_ship;
				for (uint32_t nr_states = fr.unsigned_32(); nr_states; --nr_states) {
					const Serial dock = fr.unsigned_32();
					const Duration time = fr.unsigned_32();
					const bool exp = fr.unsigned_8();
					SchedulingStateT<Serial, CargoListLoader> state(dock, exp, time);
					for (uint32_t nr_cargo = fr.unsigned_32(); nr_cargo; --nr_cargo) {
						const Serial dest = fr.unsigned_32();
						const Quantity nr = fr.unsigned_32();
						state.load_there.push_back(std::make_pair(dest, nr));
					}
					states_for_this_ship.push_back(state);
				}
				(*loader_)[ship] = states_for_this_ship;
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
	for (const auto& plan : *loader_) {
		ShipPlan plan_for_this_ship;
		for (const auto& state_loader : plan.second) {
			SchedulingState state(&mol.get<PortDock>(state_loader.dock), state_loader.expedition,
			                      state_loader.duration_from_previous_location);
			const size_t cargo = state_loader.load_there.size();
			state.load_there.resize(cargo);
			for (size_t s = 0; s < cargo; ++s) {
				state.load_there[s].first = &mol.get<PortDock>(state_loader.load_there[s].first);
				state.load_there[s].second = state_loader.load_there[s].second;
			}
			plan_for_this_ship.push_back(state);
		}
		plans_[&mol.get<Ship>(plan.first)] = plan_for_this_ship;
	}
	loader_.reset(nullptr);
}

// TODO(Nordfriese): DELETE this function when we break savegame compatibility
void ShippingSchedule::load_finish(EditorGameBase& egbase) {
	log("Initializing ShippingSchedule from legacy game state. Pray to Lutas that your ships will "
	    "sail more or less where you want them to go to.\n");
	assert(!loader_);
	assert(empty());
	for (Ship* ship : fleet_.get_ships()) {
		ShipPlan& sp = plans_[ship];
		assert(sp.empty());
		if (PortDock* pd = ship->get_destination()) {
			Path path;
			int32_t d = -1;
			ship->calculate_sea_route(egbase, *pd, &path);
			egbase.map().calc_cost(path, &d, nullptr);
			assert(d >= 0);
			sp.push_back(SchedulingState(pd, false, d));
		}
	}
}
}
