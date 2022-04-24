/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#include "economy/ship_fleet.h"

#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/routing_node.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map_objects/checkstep.h"
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
ShipFleetDescr g_ship_fleet_descr("ship_fleet", "Ship Fleet");
}  // namespace

const ShipFleetDescr& ShipFleet::descr() const {
	return g_ship_fleet_descr;
}

/**
 * Fleets are initialized empty.
 *
 * Intended use: @ref Ship and @ref PortDock, when created, create a new @ref ShipFleet
 * instance, then add themselves \em before calling the \ref init function.
 * The Fleet takes care of merging with existing fleets, if any.
 */
ShipFleet::ShipFleet(Player* player)
   : MapObject(&g_ship_fleet_descr), act_pending_(false), schedule_(*this) {
	owner_ = player;
}

/**
 * Whether the fleet is in fact useful for transporting goods.
 * This is the case if there is a ship and a port.
 */
bool ShipFleet::active() const {
	return !ships_.empty() && !ports_.empty();
}

/**
 * Inform the Fleet about the change of @ref Economy of one of the docks.
 *
 * Note that we always associate ourselves with the economy of the first dock.
 */
void ShipFleet::set_economy(Economy* e, WareWorker type) {
	if (!ships_.empty()) {
		if (!ports_.empty()) {
			e = ports_[0]->get_economy(type);
		}
#ifndef NDEBUG
		else {
			assert(e == nullptr);
		}
#endif

		if (upcast(Game, game, &get_owner()->egbase())) {
			for (Ship* temp_ship : ships_) {
				temp_ship->set_economy(*game, e, type);
			}
		}
	}
}

/**
 * Initialize the fleet, including a search through the map
 * to rejoin with the next other fleet we can find.
 */
bool ShipFleet::init(EditorGameBase& egbase) {
	MapObject::init(egbase);

	if (empty()) {
		molog(egbase.get_gametime(), "Empty fleet initialized; disband immediately\n");
		remove(egbase);
		return false;
	}

	return find_other_fleet(egbase);
}

struct StepEvalFindShipFleet {
	int32_t estimate(Map& /* map */, FCoords /* pos */) const {
		return 0;
	}
	int32_t stepcost(
	   Map& /* map */, FCoords from, int32_t fromcost, WalkingDir /* dir */, FCoords to) const {
		if ((to.field->nodecaps() & (MOVECAPS_SWIM | MOVECAPS_WALK)) == 0) {
			return -1;
		}

		if ((from.field->nodecaps() & MOVECAPS_SWIM) == 0) {
			// We are allowed to land on and leave the shore,
			// but not in the middle of a path
			if (fromcost > 0) {
				return -1;
			}

			return 1;
		}

		return 1;
	}
};

/**
 * Search the map, starting at our ships and ports, for another fleet
 * of the same player.
 */
bool ShipFleet::find_other_fleet(EditorGameBase& egbase) {
	MapAStar<StepEvalFindShipFleet> astar(*egbase.mutable_map(), StepEvalFindShipFleet(), wwWORKER);
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
			const MapObjectType type = imm->descr().type();
			if (type == MapObjectType::PORTDOCK) {
				upcast(PortDock, dock, imm);
				// here might be a problem so I (tiborb) put here
				// this test, might be removed after some time
				if (dock->get_fleet() == nullptr) {
					log_warn_time(egbase.get_gametime(), "The dock on %3dx%3d without a fleet!\n",
					              dock->dockpoints_.front().x, dock->dockpoints_.front().y);
				}
				if (dock->get_fleet() != this && dock->get_owner() == get_owner()) {
					return dock->get_fleet()->merge(egbase, this);
				}
			}
		}

		for (Bob* bob = cur.field->get_first_bob(); bob != nullptr; bob = bob->get_next_bob()) {
			const MapObjectType type = bob->descr().type();
			if (type == MapObjectType::SHIP) {
				upcast(Ship, ship, bob);
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
bool ShipFleet::merge(EditorGameBase& egbase, ShipFleet* other) {
	if (ports_.empty() && !other->ports_.empty()) {
		other->merge(egbase, this);
		return true;
	}

	// TODO(Nordfriese): Currently fleets merge only directly after creation, so the fleet being
	// merged has either no ports or no ships, and therefore should not yet have scheduled anything.
	// Scripts could also enforce fleet merges by changing the terrain so that two previously
	// separate oceans, both with a functional system of ships and ports, are now connected.
	// (Would this even be detected? Not sure.) The schedule side of such a merge is not supported.
	assert(other->get_schedule().empty());

	while (!other->ships_.empty()) {
		Ship* ship = other->ships_.back();
		other->ships_.pop_back();
		add_ship(egbase, ship);
	}

	uint32_t old_nrports = ports_.size();
	ports_.insert(ports_.end(), other->ports_.begin(), other->ports_.end());
	port_paths_.insert(other->port_paths_.begin(), other->port_paths_.end());

	for (uint32_t idx = old_nrports; idx < ports_.size(); ++idx) {
		ports_[idx]->set_fleet(this);
	}

	if (!ships_.empty() && !ports_.empty()) {
		check_merge_economy();
	}

	other->ports_.clear();
	other->port_paths_.clear();
	other->remove(egbase);

	update(egbase);
	return false;
}

/**
 * If we have ports and ships, ensure that all ports belong to the same economy.
 */
void ShipFleet::check_merge_economy() {
	if (ports_.empty() || ships_.empty()) {
		return;
	}

	const Flag& base = ports_[0]->base_flag();
	for (uint32_t i = 1; i < ports_.size(); ++i) {
		// Note: economy of base flag may of course be changed by the merge!
		Economy::check_merge(base, ports_[i]->base_flag(), wwWARE);
		Economy::check_merge(base, ports_[i]->base_flag(), wwWORKER);
	}
}

void ShipFleet::cleanup(EditorGameBase& egbase) {
	while (!ports_.empty()) {
		PortDock* pd = ports_.back();
		ports_.pop_back();

		pd->set_fleet(nullptr);
		if (!ports_.empty() && !ships_.empty()) {
			// This is required when, during end-of-game cleanup,
			// the fleet gets removed before the ports
			Flag& base = ports_[0]->base_flag();
			Economy::check_split(base, pd->base_flag(), wwWARE);
			Economy::check_split(base, pd->base_flag(), wwWORKER);
		}
	}
	port_paths_.clear();

	while (!ships_.empty()) {
		Ship* ship = ships_.back();
		ship->set_fleet(nullptr);
		ships_.pop_back();
	}

	MapObject::cleanup(egbase);
}

ShipFleet::PortPath& ShipFleet::portpath(uint32_t i, uint32_t j) {
	assert(i < j);
	assert(j < ports_.size());
	// This creates a default-constructed portpath if it did not yet exist.
	return port_paths_[std::make_pair(ports_[i]->serial(), ports_[j]->serial())];
}

const ShipFleet::PortPath& ShipFleet::portpath(uint32_t i, uint32_t j) const {
	assert(i < j);
	assert(j < ports_.size());
	// This throws an exception if the port path was never computed yet.
	return port_paths_.at(std::make_pair(ports_[i]->serial(), ports_[j]->serial()));
}

ShipFleet::PortPath& ShipFleet::portpath_bidir(uint32_t i, uint32_t j, bool& reverse) {
	reverse = false;
	if (i > j) {
		reverse = true;
		std::swap(i, j);
	}
	return portpath(i, j);
}

const ShipFleet::PortPath& ShipFleet::portpath_bidir(uint32_t i, uint32_t j, bool& reverse) const {
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
bool ShipFleet::get_path(const PortDock& start, const PortDock& end, Path& path) {
	uint32_t startidx = std::find(ports_.begin(), ports_.end(), &start) - ports_.begin();
	uint32_t endidx = std::find(ports_.begin(), ports_.end(), &end) - ports_.begin();

	if (startidx >= ports_.size() || endidx >= ports_.size()) {
		return false;
	}

	bool reverse;
	const PortPath& pp(portpath_bidir(startidx, endidx, reverse));

	if (pp.cost < 0) {
		// try filling in pp's data
		connect_port(get_owner()->egbase(), startidx);
	}

	if (pp.cost < 0) {
		return false;
	}

	path = *pp.path;
	if (reverse) {
		path.reverse();
	}

	return true;
}

uint32_t ShipFleet::count_ships() const {
	return ships_.size();
}

uint32_t ShipFleet::count_ports() const {
	return ports_.size();
}
bool ShipFleet::get_act_pending() const {
	return act_pending_;
}

void ShipFleet::add_neighbours(PortDock& pd, std::vector<RoutingNodeNeighbour>& neighbours) {
	uint32_t idx = std::find(ports_.begin(), ports_.end(), &pd) - ports_.begin();

	for (uint32_t otheridx = 0; otheridx < ports_.size(); ++otheridx) {
		if (idx == otheridx) {
			continue;
		}

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

void ShipFleet::add_ship(EditorGameBase& egbase, Ship* ship) {
	ships_.push_back(ship);
	assert(std::count(ships_.begin(), ships_.end(), ship) == 1);

	ship->set_fleet(this);

	if (upcast(Game, game, &get_owner()->egbase())) {
		if (ports_.empty()) {
			ship->set_economy(*game, nullptr, wwWARE);
			ship->set_economy(*game, nullptr, wwWORKER);
		} else {
			ship->set_economy(*game, ports_[0]->get_economy(wwWARE), wwWARE);
			ship->set_economy(*game, ports_[0]->get_economy(wwWORKER), wwWORKER);
		}
		schedule_.ship_added(*game, *ship);
	}

	if (ships_.size() == 1) {
		check_merge_economy();
	}
	update(egbase);
}

void ShipFleet::remove_ship(EditorGameBase& egbase, Ship* ship) {
	std::vector<Ship*>::iterator it = std::find(ships_.begin(), ships_.end(), ship);
	while (it != ships_.end()) {
		*it = ships_.back();
		ships_.pop_back();
		it = std::find(ships_.begin(), ships_.end(), ship);
		if (it != ships_.end()) {
			log_err_time(
			   egbase.get_gametime(), "Multiple instances of the same ship were in the ship fleet\n");
		}
	}
	assert(std::count(ships_.begin(), ships_.end(), ship) == 0);

	ship->set_fleet(nullptr);

	if (upcast(Game, game, &egbase)) {
		ship->set_economy(*game, nullptr, wwWARE);
		ship->set_economy(*game, nullptr, wwWORKER);
		schedule_.ship_removed(*game, ship);
	}

	if (ships_.empty()) {
		if (empty()) {
			remove(egbase);
		} else {
			Flag& base = ports_[0]->base_flag();
			for (uint32_t i = 1; i < ports_.size(); ++i) {
				// since two ports can be connected by land, it is possible that
				// disconnecting a previous port also disconnects later ports
				if (base.get_economy(wwWARE) == ports_[i]->base_flag().get_economy(wwWARE)) {
					Economy::check_split(base, ports_[i]->base_flag(), wwWARE);
				}
				if (base.get_economy(wwWORKER) == ports_[i]->base_flag().get_economy(wwWORKER)) {
					Economy::check_split(base, ports_[i]->base_flag(), wwWORKER);
				}
			}
		}
	}
}

bool ShipFleet::empty() const {
	return ships_.empty() && ports_.empty();
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

	int32_t stepcost(
	   const Map& map, FCoords from, int32_t /* fromcost */, WalkingDir dir, FCoords to) const {
		if ((to.field->nodecaps() & MOVECAPS_SWIM) == 0) {
			return -1;
		}
		return map.calc_bidi_cost(from, dir);
	}
};

/**
 * Fill in all unknown paths to connect the port ports_[idx] to the rest of the ports.
 *
 * Note that this is done lazily, i.e. the first time a path is actually requested,
 * because path finding is flaky during map loading.
 */
void ShipFleet::connect_port(EditorGameBase& egbase, uint32_t idx) {
	StepEvalFindPorts se;

	for (uint32_t i = 0; i < ports_.size(); ++i) {
		if (i == idx) {
			continue;
		}

		bool reverse;
		if (portpath_bidir(i, idx, reverse).cost >= 0) {
			continue;
		}

		StepEvalFindPorts::Target tgt;
		tgt.idx = i;
		tgt.pos = ports_[i]->get_warehouse()->get_position();
		se.targets.push_back(tgt);
	}

	if (se.targets.empty()) {
		return;
	}

	MapAStar<StepEvalFindPorts> astar(*egbase.mutable_map(), se, wwWORKER);

	BaseImmovable::PositionList src(ports_[idx]->get_positions(egbase));
	for (const Coords& temp_pos : src) {
		astar.push(temp_pos);
	}

	int32_t cost;
	FCoords cur;
	while (!se.targets.empty() && astar.step(cur, cost)) {
		BaseImmovable* imm = cur.field->get_immovable();
		if ((imm == nullptr) || imm->descr().type() != MapObjectType::PORTDOCK) {
			continue;
		}

		if (upcast(PortDock, pd, imm)) {
			if (pd->get_owner() != get_owner()) {
				continue;
			}

			if ((pd->get_fleet() != nullptr) && pd->get_fleet() != this) {
				log_err_time(egbase.get_gametime(),
				             "ShipFleet::connect_port: different fleets despite reachability\n");
				continue;
			}

			uint32_t otheridx = std::find(ports_.begin(), ports_.end(), pd) - ports_.begin();
			if (idx == otheridx) {
				continue;
			}

			bool reverse;
			PortPath& ppath(portpath_bidir(idx, otheridx, reverse));

			if (ppath.cost >= 0) {
				continue;
			}

			ppath.cost = cost;
			ppath.path.reset(new Path);
			astar.pathto(cur, *ppath.path);
			if (reverse) {
				ppath.path->reverse();
			}

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
		log_err_time(egbase.get_gametime(), "ShipFleet::connect_port: Could not reach all ports!\n");
	}
}

void ShipFleet::add_port(EditorGameBase& egbase, PortDock* port) {
	ports_.push_back(port);
	port->set_fleet(this);
	if (ports_.size() == 1) {
		set_economy(ports_[0]->get_economy(wwWARE), wwWARE);
		set_economy(ports_[0]->get_economy(wwWORKER), wwWORKER);
	} else {
		if (!ships_.empty()) {
			Economy::check_merge(ports_[0]->base_flag(), port->base_flag(), wwWARE);
			Economy::check_merge(ports_[0]->base_flag(), port->base_flag(), wwWORKER);
		}
	}

	if (upcast(Game, g, &egbase)) {
		schedule_.port_added(*g, *port);
	}
	update(egbase);
}

void ShipFleet::remove_port(EditorGameBase& egbase, PortDock* port) {
	for (auto it = port_paths_.begin(); it != port_paths_.end();) {
		if (it->first.first == port->serial() || it->first.second == port->serial()) {
			it = port_paths_.erase(it);
		} else {
			++it;
		}
	}
	{
		std::vector<PortDock*>::iterator it = std::find(ports_.begin(), ports_.end(), port);
		if (it != ports_.end()) {
			ports_.erase(it);
		}
	}
	port->set_fleet(nullptr);

	if (ports_.empty()) {
		set_economy(nullptr, wwWARE);
		set_economy(nullptr, wwWORKER);
	} else {
		set_economy(ports_[0]->get_economy(wwWARE), wwWARE);
		set_economy(ports_[0]->get_economy(wwWORKER), wwWORKER);
		if (!ships_.empty()) {
			Economy::check_split(ports_[0]->base_flag(), port->base_flag(), wwWARE);
			Economy::check_split(ports_[0]->base_flag(), port->base_flag(), wwWORKER);
		}
	}

	if (empty()) {
		remove(egbase);
	} else if (upcast(Game, g, &egbase)) {
		// Some ship perhaps lose their destination now, so new a destination must be appointed (if
		// any)
		molog(egbase.get_gametime(), "Port removed from fleet, triggering fleet update\n");
		schedule_.port_removed(*g, port);
		update(egbase);
	}
}

bool ShipFleet::has_ports() const {
	return !ports_.empty();
}

/**
 * Search among the docks of the fleet for the one that has the given flag as its base.
 *
 * @return the dock, or 0 if not found.
 */
PortDock* ShipFleet::get_dock(Flag& flag) const {
	for (PortDock* temp_port : ports_) {
		if (&temp_port->base_flag() == &flag) {
			return temp_port;
		}
	}

	return nullptr;
}

/**
 * Search among the docks of the fleet for the one that has matches given coordinates.
 * Intended for a ship querying in what portdock it is now.
 *
 * @return the dock, or 0 if not found.
 */
PortDock* ShipFleet::get_dock(const EditorGameBase& egbase, Coords field_coords) const {
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
PortDock* ShipFleet::get_arbitrary_dock() const {
	if (ports_.empty()) {
		return nullptr;
	}
	return ports_[0];
}

/**
 * Trigger an update of ship scheduling
 */
void ShipFleet::update(EditorGameBase& egbase) {
	if (act_pending_ || (serial() == 0u)) {
		return;
	}

	if (upcast(Game, game, &egbase)) {
		schedule_act(*game, Duration(100));
		act_pending_ = true;
	}
}

/**
 * Act callback updates ship scheduling of idle ships.
 *
 * @note Do not call this directly; instead, trigger it via @ref update
 */
void ShipFleet::act(Game& game, uint32_t /*data*/) {
	assert(act_pending_);
	act_pending_ = false;

	if (empty()) {
		molog(game.get_gametime(), "ShipFleet::act: remove empty fleet\n");
		remove(game);
		return;
	}

	molog(game.get_gametime(), "ShipFleet::act\n");

	// All the work is done by the schedule
	const Duration next = schedule_.update(game);
	if (next.is_valid()) {
		schedule_act(game, next);
		act_pending_ = true;
	}
}

void ShipFleet::log_general_info(const EditorGameBase& egbase) const {
	MapObject::log_general_info(egbase);

	molog(egbase.get_gametime(), "%" PRIuS " ships and %" PRIuS " ports\n", ships_.size(),
	      ports_.size());
	molog(egbase.get_gametime(), "Schedule:\n");
	schedule_.log_general_info(egbase);
	molog(egbase.get_gametime(), "\n");
}

// Changelog of version 4 → 5: Added ShippingSchedule
constexpr uint8_t kCurrentPacketVersion = 5;

void ShipFleet::Loader::load(FileRead& fr) {
	MapObject::Loader::load(fr);

	ShipFleet& fleet = get<ShipFleet>();

	const uint32_t nrships = fr.unsigned_32();
	ships_.resize(nrships);
	for (uint32_t i = 0; i < nrships; ++i) {
		ships_[i] = fr.unsigned_32();
	}

	const uint32_t nrports = fr.unsigned_32();
	ports_.resize(nrports);
	for (uint32_t i = 0; i < nrports; ++i) {
		ports_[i] = fr.unsigned_32();
	}

	fleet.act_pending_ = (fr.unsigned_8() != 0u);
	fleet.schedule_.load(fr);
}

void ShipFleet::Loader::load_pointers() {
	MapObject::Loader::load_pointers();

	ShipFleet& fleet = get<ShipFleet>();

	// Act commands created during loading are not persistent, so we need to undo any
	// changes to the pending state.
	bool save_act_pending = fleet.act_pending_;

	MapObjectLoader& map_object_loader = mol();

	for (const uint32_t& temp_ship : ships_) {
		fleet.ships_.push_back(&map_object_loader.get<Ship>(temp_ship));
		fleet.ships_.back()->set_fleet(&fleet);
	}
	for (const uint32_t& temp_port : ports_) {
		fleet.ports_.push_back(&map_object_loader.get<PortDock>(temp_port));
		fleet.ports_.back()->set_fleet(&fleet);
	}

	fleet.act_pending_ = save_act_pending;

	fleet.schedule_.load_pointers(map_object_loader);
}

void ShipFleet::Loader::load_finish() {
	MapObject::Loader::load_finish();

	ShipFleet& fleet = get<ShipFleet>();

	if (!fleet.ports_.empty()) {
		if (!fleet.empty()) {
			fleet.check_merge_economy();
		}

		fleet.set_economy(fleet.ports_[0]->get_economy(wwWARE), wwWARE);
		fleet.set_economy(fleet.ports_[0]->get_economy(wwWORKER), wwWORKER);
	}
}

MapObject::Loader* ShipFleet::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		const uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			PlayerNumber owner_number = fr.unsigned_8();
			if ((owner_number == 0u) || owner_number > egbase.map().get_nrplayers()) {
				throw GameDataError("owner number is %u but there are only %u players", owner_number,
				                    egbase.map().get_nrplayers());
			}

			Player* owner = egbase.get_player(owner_number);
			if (owner == nullptr) {
				throw GameDataError("owning player %u does not exist", owner_number);
			}
			loader->init(egbase, mol, *(new ShipFleet(owner)));
			loader->load(fr);
		} else {
			throw UnhandledVersionError("ShipFleet", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading ship fleet: %s", e.what());
	}

	return loader.release();
}

void ShipFleet::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderShipFleet);
	fw.unsigned_8(kCurrentPacketVersion);

	fw.unsigned_8(owner_.load()->player_number());

	MapObject::save(egbase, mos, fw);

	fw.unsigned_32(ships_.size());
	for (const Ship* temp_ship : ships_) {
		fw.unsigned_32(mos.get_object_file_index(*temp_ship));
	}
	fw.unsigned_32(ports_.size());
	for (const PortDock* temp_port : ports_) {
		fw.unsigned_32(mos.get_object_file_index(*temp_port));
	}

	fw.unsigned_8(static_cast<uint8_t>(act_pending_));

	schedule_.save(egbase, mos, fw);
}

}  // namespace Widelands
