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

#include "economy/fleet.h"

#include <memory>

#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/routing_node.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/mapastar.h"
#include "logic/path.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

namespace {
// Every MapObject() needs to have a description. So we make a dummy one for
// Fleet.
FleetDescr g_fleet_descr("fleet", "Fleet");
}  // namespace

const FleetDescr& Fleet::descr() const {
	return g_fleet_descr;
}

/**
 * Fleets are initialized empty.
 *
 * Intended use: @ref Ship and @ref PortDock, when created, create a new @ref Fleet
 * instance, then add themselves \em before calling the \ref init function.
 * The Fleet takes care of merging with existing fleets, if any.
 */
Fleet::Fleet(Player* player) : MapObject(&g_fleet_descr), act_pending_(false) {
	owner_ = player;
}

/**
 * Whether the fleet is in fact useful for transporting goods.
 */
bool Fleet::active() const {
	return !ships_.empty() && !ports_.empty();
}

/**
 * Inform the Fleet about the change of @ref Economy of one of the docks.
 *
 * Note that we always associate ourselves with the economy of the first dock.
 */
void Fleet::set_economy(Economy* e) {
	if (!ships_.empty()) {
		if (!ports_.empty()) {
			e = ports_[0]->get_economy();
		}
#ifndef NDEBUG
		else
			assert(e == nullptr);
#endif

		if (upcast(Game, game, &get_owner()->egbase())) {
			for (Ship* temp_ship : ships_) {
				temp_ship->set_economy(*game, e);
			}
		}
	}
}

/**
 * Initialize the fleet, including a search through the map
 * to rejoin with the next other fleet we can find.
 */
bool Fleet::init(EditorGameBase& egbase) {
	MapObject::init(egbase);

	if (ships_.empty() && ports_.empty()) {
		molog("Empty fleet initialized; disband immediately\n");
		remove(egbase);
		return false;
	}

	return find_other_fleet(egbase);
}

struct StepEvalFindFleet {
	int32_t estimate(Map& /* map */, FCoords /* pos */) const {
		return 0;
	}
	int32_t stepcost(
	   Map& /* map */, FCoords from, int32_t fromcost, WalkingDir /* dir */, FCoords to) const {
		if (!(to.field->nodecaps() & (MOVECAPS_SWIM | MOVECAPS_WALK)))
			return -1;

		if (!(from.field->nodecaps() & MOVECAPS_SWIM)) {
			// We are allowed to land on and leave the shore,
			// but not in the middle of a path
			if (fromcost > 0)
				return -1;

			return 1;
		}

		return 1;
	}
};

/**
 * Search the map, starting at our ships and ports, for another fleet
 * of the same player.
 */
bool Fleet::find_other_fleet(EditorGameBase& egbase) {
	MapAStar<StepEvalFindFleet> astar(*egbase.mutable_map(), StepEvalFindFleet());
	for (const Ship* temp_ship : ships_) {
		astar.push(temp_ship->get_position());
	}

	for (const PortDock* temp_port : ports_) {
		BaseImmovable::PositionList pos = temp_port->get_positions(egbase);

		for (const Coords& temp_pos : pos) {
			astar.push(temp_pos);
		}
	}

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (BaseImmovable* imm = cur.field->get_immovable()) {
			if (imm->descr().type() == MapObjectType::PORTDOCK) {
				if (upcast(PortDock, dock, imm)) {
					// here might be a problem so I (tiborb) put here
					// this test, might be removed after some time
					if (dock->get_fleet() == nullptr) {
						log("The dock on %3dx%3d withouth a fleet!\n", dock->dockpoints_.front().x,
						    dock->dockpoints_.front().y);
					}
					if (dock->get_fleet() != this && dock->get_owner() == get_owner()) {
						return dock->get_fleet()->merge(egbase, this);
					}
				}
			}
		}

		for (Bob* bob = cur.field->get_first_bob(); bob != nullptr; bob = bob->get_next_bob()) {
			if (bob->descr().type() != MapObjectType::SHIP)
				continue;

			if (upcast(Ship, ship, bob)) {
				if (ship->get_fleet() != nullptr && ship->get_fleet() != this &&
				    ship->get_owner() == get_owner()) {
					return ship->get_fleet()->merge(egbase, this);
				}
			}
		}
	}
	if (active()) {
		update(egbase);
		return true;
	}
	return false;
}

/**
 * Merge the @p other fleet into this fleet, and remove the other fleet.
 *
 * Returns true if 'other' is the resulting fleet and "false" if 'this' is
 * the resulting fleet. The values are reversed because we originally call this from
 * another 'other' for efficiency reasons.
 */
bool Fleet::merge(EditorGameBase& egbase, Fleet* other) {
	if (ports_.empty() && !other->ports_.empty()) {
		other->merge(egbase, this);
		return true;
	}

	while (!other->ships_.empty()) {
		Ship* ship = other->ships_.back();
		other->ships_.pop_back();
		add_ship(ship);
	}

	uint32_t old_nrports = ports_.size();
	ports_.insert(ports_.end(), other->ports_.begin(), other->ports_.end());
	portpaths_.resize((ports_.size() * (ports_.size() - 1)) / 2);

	for (uint32_t j = 1; j < other->ports_.size(); ++j) {
		for (uint32_t i = 0; i < j; ++i) {
			portpath(old_nrports + i, old_nrports + j) = other->portpath(i, j);
		}
	}

	for (uint32_t idx = old_nrports; idx < ports_.size(); ++idx) {
		ports_[idx]->set_fleet(this);
	}

	if (!ships_.empty() && !ports_.empty())
		check_merge_economy();

	other->ports_.clear();
	other->portpaths_.clear();
	other->remove(egbase);

	update(egbase);
	return false;
}

/**
 * If we have ports and ships, ensure that all ports belong to the same economy.
 */
void Fleet::check_merge_economy() {
	if (ports_.empty() || ships_.empty())
		return;

	Flag& base = ports_[0]->base_flag();
	for (uint32_t i = 1; i < ports_.size(); ++i) {
		// Note: economy of base flag may of course be changed by the merge!
		base.get_economy()->check_merge(base, ports_[i]->base_flag());
	}
}

void Fleet::cleanup(EditorGameBase& egbase) {
	while (!ports_.empty()) {
		PortDock* pd = ports_.back();
		ports_.pop_back();

		pd->set_fleet(nullptr);
		if (!ports_.empty() && !ships_.empty()) {
			// This is required when, during end-of-game cleanup,
			// the fleet gets removed before the ports
			Flag& base = ports_[0]->base_flag();
			Economy::check_split(base, pd->base_flag());
		}
	}
	portpaths_.clear();

	while (!ships_.empty()) {
		Ship* ship = ships_.back();
		// Check if the ship still exists to avoid heap-use-after-free when ship has already been
		// deleted while processing EditorGameBase::cleanup_objects()
		if (egbase.objects().object_still_available(ship)) {
			ship->set_fleet(nullptr);
		}
		ships_.pop_back();
	}

	MapObject::cleanup(egbase);
}

Fleet::PortPath& Fleet::portpath(uint32_t i, uint32_t j) {
	assert(i < j);

	return portpaths_[((j - 1) * j) / 2 + i];
}

const Fleet::PortPath& Fleet::portpath(uint32_t i, uint32_t j) const {
	assert(i < j);

	return portpaths_[((j - 1) * j) / 2 + i];
}

Fleet::PortPath& Fleet::portpath_bidir(uint32_t i, uint32_t j, bool& reverse) {
	reverse = false;
	if (i > j) {
		reverse = true;
		std::swap(i, j);
	}
	return portpath(i, j);
}

const Fleet::PortPath& Fleet::portpath_bidir(uint32_t i, uint32_t j, bool& reverse) const {
	reverse = false;
	if (i > j) {
		reverse = true;
		std::swap(i, j);
	}
	return portpath(i, j);
}

/**
 * Find the two docks in the fleet, and fill in the path between them.
 *
 * @return true if successful, or false if the docks are not actually part of the fleet.
 */
bool Fleet::get_path(const PortDock& start, const PortDock& end, Path& path) {
	uint32_t startidx = std::find(ports_.begin(), ports_.end(), &start) - ports_.begin();
	uint32_t endidx = std::find(ports_.begin(), ports_.end(), &end) - ports_.begin();

	if (startidx >= ports_.size() || endidx >= ports_.size())
		return false;

	bool reverse;
	const PortPath& pp(portpath_bidir(startidx, endidx, reverse));

	if (pp.cost < 0) {
		connect_port(get_owner()->egbase(), startidx);
	}

	if (pp.cost < 0)
		return false;

	path = *pp.path;
	if (reverse)
		path.reverse();

	return true;
}

uint32_t Fleet::count_ships() const {
	return ships_.size();
}

uint32_t Fleet::count_ships_heading_here(EditorGameBase& egbase, PortDock* port) const {
	uint32_t ships_on_way = 0;
	for (uint16_t s = 0; s < ships_.size(); ++s) {
		if (ships_[s]->get_destination(egbase) == port) {
			++ships_on_way;
		}
	}

	return ships_on_way;
}

uint32_t Fleet::count_ports() const {
	return ports_.size();
}
bool Fleet::get_act_pending() const {
	return act_pending_;
}

void Fleet::add_neighbours(PortDock& pd, std::vector<RoutingNodeNeighbour>& neighbours) {
	uint32_t idx = std::find(ports_.begin(), ports_.end(), &pd) - ports_.begin();

	for (uint32_t otheridx = 0; otheridx < ports_.size(); ++otheridx) {
		if (idx == otheridx)
			continue;

		bool reverse;
		PortPath& pp(portpath_bidir(idx, otheridx, reverse));

		if (pp.cost < 0) {
			// Lazily discover routes between ports
			connect_port(get_owner()->egbase(), idx);
		}

		if (pp.cost >= 0) {
			// TODO(unknown): keep statistics on average transport time instead of using the arbitrary
			// 2x factor
			RoutingNodeNeighbour neighb(&ports_[otheridx]->base_flag(), 2 * pp.cost);
			neighbours.push_back(neighb);
		}
	}
}

void Fleet::add_ship(Ship* ship) {
	ships_.push_back(ship);
	ship->set_fleet(this);
	if (upcast(Game, game, &get_owner()->egbase())) {
		if (ports_.empty())
			ship->set_economy(*game, nullptr);
		else
			ship->set_economy(*game, ports_[0]->get_economy());
	}

	if (ships_.size() == 1) {
		check_merge_economy();
	}
}

void Fleet::remove_ship(EditorGameBase& egbase, Ship* ship) {
	std::vector<Ship*>::iterator it = std::find(ships_.begin(), ships_.end(), ship);
	if (it != ships_.end()) {
		*it = ships_.back();
		ships_.pop_back();
	}
	ship->set_fleet(nullptr);
	if (upcast(Game, game, &egbase))
		ship->set_economy(*game, nullptr);

	if (ship->get_destination(egbase)) {
		ship->get_destination(egbase)->ship_coming(false);
		update(egbase);
	}

	if (ships_.empty()) {
		if (ports_.empty()) {
			remove(egbase);
		} else {
			Flag& base = ports_[0]->base_flag();
			for (uint32_t i = 1; i < ports_.size(); ++i) {
				// since two ports can be connected by land, it is possible that
				// disconnecting a previous port also disconnects later ports
				if (base.get_economy() == ports_[i]->base_flag().get_economy())
					Economy::check_split(base, ports_[i]->base_flag());
			}
		}
	}
}

struct StepEvalFindPorts {
	struct Target {
		uint32_t idx;
		Coords pos;
	};
	std::vector<Target> targets;

	int32_t estimate(Map& map, FCoords pos) const {
		int32_t est = std::numeric_limits<int32_t>::max();
		for (const Target& temp_target : targets) {
			est = std::min(est, map.calc_cost_estimate(pos, temp_target.pos));
		}
		return std::max(0, est - 5 * map.calc_cost(0));
	}

	int32_t
	stepcost(Map& map, FCoords from, int32_t /* fromcost */, WalkingDir dir, FCoords to) const {
		if (!(to.field->nodecaps() & MOVECAPS_SWIM))
			return -1;

		return map.calc_bidi_cost(from, dir);
	}
};

/**
 * Fill in all unknown paths to connect the port ports_[idx] to the rest of the ports.
 *
 * Note that this is done lazily, i.e. the first time a path is actually requested,
 * because path finding is flaky during map loading.
 */
void Fleet::connect_port(EditorGameBase& egbase, uint32_t idx) {
	StepEvalFindPorts se;

	for (uint32_t i = 0; i < ports_.size(); ++i) {
		if (i == idx)
			continue;

		bool reverse;
		if (portpath_bidir(i, idx, reverse).cost >= 0)
			continue;

		StepEvalFindPorts::Target tgt;
		tgt.idx = i;
		tgt.pos = ports_[i]->get_warehouse()->get_position();
		se.targets.push_back(tgt);
	}

	if (se.targets.empty())
		return;

	MapAStar<StepEvalFindPorts> astar(*egbase.mutable_map(), se);

	BaseImmovable::PositionList src(ports_[idx]->get_positions(egbase));
	for (const Coords& temp_pos : src) {
		astar.push(temp_pos);
	}

	int32_t cost;
	FCoords cur;
	while (!se.targets.empty() && astar.step(cur, cost)) {
		BaseImmovable* imm = cur.field->get_immovable();
		if (!imm || imm->descr().type() != MapObjectType::PORTDOCK)
			continue;

		if (upcast(PortDock, pd, imm)) {
			if (pd->get_owner() != get_owner())
				continue;

			if (pd->get_fleet() && pd->get_fleet() != this) {
				log("Fleet::connect_port: different fleets despite reachability\n");
				continue;
			}

			uint32_t otheridx = std::find(ports_.begin(), ports_.end(), pd) - ports_.begin();
			if (idx == otheridx)
				continue;

			bool reverse;
			PortPath& ppath(portpath_bidir(idx, otheridx, reverse));

			if (ppath.cost >= 0)
				continue;

			ppath.cost = cost;
			ppath.path.reset(new Path);
			astar.pathto(cur, *ppath.path);
			if (reverse)
				ppath.path->reverse();

			for (StepEvalFindPorts::Target& temp_target : se.targets) {
				if (temp_target.idx == otheridx) {
					temp_target = se.targets.back();
					se.targets.pop_back();
					break;
				}
			}
		}
	}

	if (!se.targets.empty()) {
		log("Fleet::connect_port: Could not reach all ports!\n");
	}
}

void Fleet::add_port(EditorGameBase& /* egbase */, PortDock* port) {
	ports_.push_back(port);
	port->set_fleet(this);
	if (ports_.size() == 1) {
		set_economy(ports_[0]->get_economy());
	} else {
		if (!ships_.empty())
			ports_[0]->get_economy()->check_merge(ports_[0]->base_flag(), port->base_flag());
	}

	portpaths_.resize((ports_.size() * (ports_.size() - 1)) / 2);
}

void Fleet::remove_port(EditorGameBase& egbase, PortDock* port) {
	std::vector<PortDock*>::iterator it = std::find(ports_.begin(), ports_.end(), port);
	if (it != ports_.end()) {
		uint32_t gap = it - ports_.begin();
		for (uint32_t i = 0; i < gap; ++i) {
			portpath(i, gap) = portpath(i, ports_.size() - 1);
		}
		for (uint32_t i = gap + 1; i < ports_.size() - 1; ++i) {
			portpath(gap, i) = portpath(i, ports_.size() - 1);
			if (portpath(gap, i).path)
				portpath(gap, i).path->reverse();
		}
		portpaths_.resize((ports_.size() * (ports_.size() - 1)) / 2);

		*it = ports_.back();
		ports_.pop_back();
	}
	port->set_fleet(nullptr);

	if (ports_.empty()) {
		set_economy(nullptr);
	} else {
		set_economy(ports_[0]->get_economy());
		if (!ships_.empty())
			Economy::check_split(ports_[0]->base_flag(), port->base_flag());
	}

	if (ships_.empty() && ports_.empty()) {
		remove(egbase);
	} else if (is_a(Game, &egbase)) {
		// Some ship perhaps lose their destination now, so new a destination must be appointed (if
		// any)
		molog("Port removed from fleet, triggering fleet update\n");
		update(egbase);
	}
}

bool Fleet::has_ports() const {
	return !ports_.empty();
}

/**
 * Search among the docks of the fleet for the one that has the given flag as its base.
 *
 * @return the dock, or 0 if not found.
 */
PortDock* Fleet::get_dock(Flag& flag) const {
	for (PortDock* temp_port : ports_) {
		if (&temp_port->base_flag() == &flag)
			return temp_port;
	}

	return nullptr;
}

/**
 * Search among the docks of the fleet for the one that has matches given coordinates.
 * Intended for a ship querying in what portdock it is now.
 *
 * @return the dock, or 0 if not found.
 */
PortDock* Fleet::get_dock(EditorGameBase& egbase, Coords field_coords) const {
	for (PortDock* temp_port : ports_) {
		for (Coords tmp_coords : temp_port->get_positions(egbase)) {
			if (tmp_coords == field_coords) {
				return temp_port;
			}
		}
	}

	return nullptr;
}

/**
 * @return an arbitrary dock of the fleet, or 0 if the fleet has no docks
 */
PortDock* Fleet::get_arbitrary_dock() const {
	if (ports_.empty())
		return nullptr;
	return ports_[0];
}

/**
 * Trigger an update of ship scheduling
 */
void Fleet::update(EditorGameBase& egbase) {
	if (act_pending_) {
		return;
	}

	if (upcast(Game, game, &egbase)) {
		schedule_act(*game, 100);
		act_pending_ = true;
	}
}

/**
 * Act callback updates ship scheduling of idle ships.
 *
 * @note Do not call this directly; instead, trigger it via @ref update
 */
void Fleet::act(Game& game, uint32_t /* data */) {
	act_pending_ = false;

	if (!active()) {
		// If we are here, most likely act() was called by a port with waiting wares or
		// with an expedition ready, although there are still no ships.
		// We can't handle it now, so we reschedule the act()
		schedule_act(game, kFleetInterval);  // retry in the next time
		act_pending_ = true;
		return;
	}

	molog("Fleet::act\n");

	// For each waiting port, try to find idle ships and send to it the closest one.
	uint16_t waiting_ports = ports_.size();
	for (PortDock* p : ports_) {
		if (p->get_need_ship() == 0) {
			--waiting_ports;
			continue;
		}

		Ship* closest_ship = nullptr;
		uint32_t shortest_dist = kRouteNotCalculated;
		bool waiting = true;

		for (Ship* s : ships_) {
			if (s->get_destination(game)) {
				if (s->get_destination(game) == p) {
					waiting = false;
					--waiting_ports;
					break;
				}
				continue;  // The ship already has a destination
			}
			if (s->get_ship_state() != Ship::ShipStates::kTransport) {
				continue;  // Ship is not available, e.g. in expedition
			}

			// Here we get distance ship->port
			uint32_t route_length = kRouteNotCalculated;

			// Get precalculated distance for ships available at ports
			{
				PortDock* cur_port = get_dock(game, s->get_position());
				if (cur_port) {          // Ship is at a port
					if (cur_port == p) {  // Same port
						route_length = 0;
					} else {  // Different port
						Path precalculated_path;
						if (get_path(*cur_port, *p, precalculated_path)) {
							route_length = precalculated_path.get_nsteps();
						}
					}
				}
			}

			// Get distance for ships available but not at a port (should not happen frequently)
			if (route_length == kRouteNotCalculated) {
				route_length = s->calculate_sea_route(game, *p);
			}

			if (route_length < shortest_dist) {
				shortest_dist = route_length;
				closest_ship = s;
			}
		}

		if (waiting && closest_ship) {
			--waiting_ports;
			closest_ship->set_destination(p);
			closest_ship->send_signal(game, "wakeup");
		}
	}

	if (waiting_ports > 0) {
		molog("... there are %u ports requesting ship(s) we cannot satisfy yet\n", waiting_ports);
		schedule_act(game, kFleetInterval);  // retry next time
		act_pending_ = true;
	}

	// Deal with edge-case of losing destination before reaching it
	for (Ship* s : ships_) {
		if (s->get_destination(game)) {
			continue;  // The ship has a destination
		}
		if (s->get_ship_state() != Ship::ShipStates::kTransport) {
			continue;  // Ship is not available, e.g. in expedition
		}
		if (s->items_.empty()) {
			continue;  // No pending wares/workers
		}

		// Send ship to the closest port
		PortDock* closest_port = nullptr;
		uint32_t shortest_dist = kRouteNotCalculated;

		for (PortDock* p : ports_) {
			uint32_t route_length = s->calculate_sea_route(game, *p);
			if (route_length < shortest_dist) {
				shortest_dist = route_length;
				closest_port = p;
			}
		}

		if (closest_port) {
			s->set_destination(closest_port);
			s->send_signal(game, "wakeup");
		}
	}
}

/**
 * For the given three consecutive ports, decide if their path is favourable or not.
 * \return true if the path from start to finish >= the path from middle to finish
 */
bool Fleet::is_path_favourable(const PortDock& start,
                               const PortDock& middle,
                               const PortDock& finish) {
	if (&middle != &finish) {
		Path path_start_to_finish;
		Path path_middle_to_finish;
#ifndef NDEBUG
		assert(get_path(start, finish, path_start_to_finish));
#else
		get_path(start, finish, path_start_to_finish);
#endif
		if (get_path(middle, finish, path_middle_to_finish)) {
			if (path_middle_to_finish.get_nsteps() > path_start_to_finish.get_nsteps()) {
				return false;
			}
		}
	}
	return true;  // default
}

/**
 * For the given ship, go through all ports of this fleet
 * and find the one with the best score.
 * \return that port
 */
PortDock* Fleet::find_next_dest(Game& game, const Ship& ship, const PortDock& from_port) {
	PortDock* best_port = nullptr;
	float best_score = 0.0f;

	for (PortDock* p : ports_) {
		if (p == &from_port) {
			continue;  // same port
		}

		float score = 0.0f;
		WareInstance* ware;
		Worker* worker;

		// Score for wares/workers onboard that ship for that port
		for (const ShippingItem& si : ship.items_) {
			if (si.get_destination(game) == p) {
				si.get(game, &ware, &worker);
				if (ware) {
					++score;  // TODO(ypopezios): increase by ware's importance
				} else {        // worker
					score += 4;
				}
			}
		}

		// Score for wares/workers waiting at that port
		for (const ShippingItem& si : from_port.waiting_) {
			if (si.get_destination(game) == p) {
				si.get(game, &ware, &worker);
				if (ware) {
					++score;  // TODO(ypopezios): increase by ware's importance
				} else {        // worker
					score += 4;
				}
			}
		}

		if (score == 0.0f && p->get_need_ship() == 0) {
			continue;  // empty ship to empty port
		}

		// Here we get distance ship->port
		uint32_t route_length = kRouteNotCalculated;

		// Get precalculated distance if the ship is at a port
		{
			Path precalculated_path;
			if (get_path(from_port, *p, precalculated_path)) {  // try to use precalculated path
				route_length = precalculated_path.get_nsteps();
			}
		}

		// Get distance for when the ship is not at a port (should not happen frequently)
		if (route_length == kRouteNotCalculated) {
			route_length = ship.calculate_sea_route(game, *p);
		}

		score = (score + 1.0f) * (score + p->get_need_ship());
		score = score * (1.0f - route_length / (score + route_length));
		if (score > best_score) {
			best_score = score;
			best_port = p;
		}
	}

	return best_port;
}

void Fleet::log_general_info(const EditorGameBase& egbase) const {
	MapObject::log_general_info(egbase);

	molog("%" PRIuS " ships and %" PRIuS " ports\n", ships_.size(), ports_.size());
}

constexpr uint8_t kCurrentPacketVersion = 4;

Fleet::Loader::Loader() {
}

void Fleet::Loader::load(FileRead& fr) {
	MapObject::Loader::load(fr);

	Fleet& fleet = get<Fleet>();

	uint32_t nrships = fr.unsigned_32();
	ships_.resize(nrships);
	for (uint32_t i = 0; i < nrships; ++i)
		ships_[i] = fr.unsigned_32();

	uint32_t nrports = fr.unsigned_32();
	ports_.resize(nrports);
	for (uint32_t i = 0; i < nrports; ++i)
		ports_[i] = fr.unsigned_32();

	fleet.act_pending_ = fr.unsigned_8();
}

void Fleet::Loader::load_pointers() {
	MapObject::Loader::load_pointers();

	Fleet& fleet = get<Fleet>();

	// Act commands created during loading are not persistent, so we need to undo any
	// changes to the pending state.
	bool save_act_pending = fleet.act_pending_;

	for (const uint32_t& temp_ship : ships_) {
		fleet.ships_.push_back(&mol().get<Ship>(temp_ship));
		fleet.ships_.back()->set_fleet(&fleet);
	}
	for (const uint32_t& temp_port : ports_) {
		fleet.ports_.push_back(&mol().get<PortDock>(temp_port));
		fleet.ports_.back()->set_fleet(&fleet);
	}

	fleet.portpaths_.resize((fleet.ports_.size() * (fleet.ports_.size() - 1)) / 2);

	fleet.act_pending_ = save_act_pending;
}

void Fleet::Loader::load_finish() {
	MapObject::Loader::load_finish();

	Fleet& fleet = get<Fleet>();

	if (!fleet.ports_.empty()) {
		if (!fleet.ships_.empty())
			fleet.check_merge_economy();

		fleet.set_economy(fleet.ports_[0]->get_economy());
	}
}

MapObject::Loader* Fleet::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			PlayerNumber owner_number = fr.unsigned_8();
			if (!owner_number || owner_number > egbase.map().get_nrplayers())
				throw GameDataError("owner number is %u but there are only %u players", owner_number,
				                    egbase.map().get_nrplayers());

			Player* owner = egbase.get_player(owner_number);
			if (!owner)
				throw GameDataError("owning player %u does not exist", owner_number);

			loader->init(egbase, mol, *(new Fleet(owner)));
			loader->load(fr);
		} else {
			throw UnhandledVersionError("Fleet", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading portdock: %s", e.what());
	}

	return loader.release();
}

void Fleet::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderFleet);
	fw.unsigned_8(kCurrentPacketVersion);

	fw.unsigned_8(owner_->player_number());

	MapObject::save(egbase, mos, fw);

	fw.unsigned_32(ships_.size());
	for (const Ship* temp_ship : ships_) {
		fw.unsigned_32(mos.get_object_file_index(*temp_ship));
	}
	fw.unsigned_32(ports_.size());
	for (const PortDock* temp_port : ports_) {
		fw.unsigned_32(mos.get_object_file_index(*temp_port));
	}

	fw.unsigned_8(act_pending_);
}

}  // namespace Widelands
