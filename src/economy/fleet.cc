/*
 * Copyright (C) 2011-2013 by the Widelands Development Team
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
#include "logic/mapastar.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/warehouse.h"
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
Fleet::Fleet(Player & player) :
	MapObject(&g_fleet_descr),
	m_owner(player),
	m_act_pending(false)
{
}


/**
 * Whether the fleet is in fact useful for transporting goods.
 */
bool Fleet::active() const
{
	return !m_ships.empty() && !m_ports.empty();
}

/**
 * Inform the Fleet about the change of @ref Economy of one of the docks.
 *
 * Note that we always associate ourselves with the economy of the first dock.
 */
void Fleet::set_economy(Economy * e)
{
	if (!m_ships.empty()) {
		if (!m_ports.empty()) {
			e = m_ports[0]->get_economy();
		}
#ifndef NDEBUG
		else
			assert(e == nullptr);
#endif

		if (upcast(Game, game, &owner().egbase())) {
			for (Ship * temp_ship : m_ships) {
				temp_ship->set_economy(*game, e);
			}
		}
	}
}

/**
 * Initialize the fleet, including a search through the map
 * to rejoin with the next other fleet we can find.
 */
void Fleet::init(EditorGameBase & egbase)
{
	MapObject::init(egbase);

	if (m_ships.empty() && m_ports.empty()) {
		molog("Empty fleet initialized; disband immediately\n");
		remove(egbase);
		return;
	}

	find_other_fleet(egbase);

	if (active())
		update(egbase);
}

struct StepEvalFindFleet {
	int32_t estimate(Map & /* map */, FCoords /* pos */) const {return 0;}
	int32_t stepcost(Map & /* map */, FCoords from, int32_t fromcost, WalkingDir /* dir */, FCoords to) const {
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
void Fleet::find_other_fleet(EditorGameBase & egbase)
{
	Map & map = egbase.map();
	MapAStar<StepEvalFindFleet> astar(map, StepEvalFindFleet());

	for (const Ship * temp_ship : m_ships) {
		astar.push(temp_ship->get_position());
	}

	for (const PortDock * temp_port : m_ports) {
		BaseImmovable::PositionList pos = temp_port->get_positions(egbase);

		for (const Coords& temp_pos : pos) {
			astar.push(temp_pos);
		}
	}

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (BaseImmovable * imm = cur.field->get_immovable()) {
			if (imm->descr().type() == MapObjectType::PORTDOCK) {
				if (upcast(PortDock, dock, imm)) {
					// here might be a problem so I (tiborb) put here
					// this test, might be removed after some time
					if (dock->get_fleet() == nullptr) {
						log ("The dock on %3dx%3d withouth a fleet!\n",
						dock->m_dockpoints.front().x,
						dock->m_dockpoints.front().y);
					}
					if (dock->get_fleet() != this && dock->get_owner() == get_owner()) {
						dock->get_fleet()->merge(egbase, this);
						return;
					}
				}
			}
		}

		for (Bob * bob = cur.field->get_first_bob(); bob != nullptr; bob = bob->get_next_bob()) {
			if (bob->descr().type() != MapObjectType::SHIP)
				continue;

			if (upcast(Ship, ship, bob)) {
				if
					(ship->get_fleet() != nullptr &&
					 ship->get_fleet() != this &&
					 ship->get_owner() == get_owner())
				{
					ship->get_fleet()->merge(egbase, this);
					return;
				}
			}
		}
	}
}

/**
 * Merge the @p other fleet into this fleet, and remove the other fleet.
 */
void Fleet::merge(EditorGameBase & egbase, Fleet * other)
{
	if (m_ports.empty() && !other->m_ports.empty()) {
		other->merge(egbase, this);
		return;
	}

	while (!other->m_ships.empty()) {
		Ship * ship = other->m_ships.back();
		other->m_ships.pop_back();
		add_ship(ship);
	}

	uint32_t old_nrports = m_ports.size();
	m_ports.insert(m_ports.end(), other->m_ports.begin(), other->m_ports.end());
	m_portpaths.resize((m_ports.size() * (m_ports.size() - 1)) / 2);

	for (uint32_t j = 1; j < other->m_ports.size(); ++j) {
		for (uint32_t i = 0; i < j; ++i) {
			portpath(old_nrports + i, old_nrports + j) = other->portpath(i, j);
		}
	}

	for (uint32_t idx = old_nrports; idx < m_ports.size(); ++idx) {
		m_ports[idx]->set_fleet(this);
	}

	if (!m_ships.empty() && !m_ports.empty())
		check_merge_economy();

	other->m_ports.clear();
	other->m_portpaths.clear();
	other->remove(egbase);

	update(egbase);
}

/**
 * If we have ports and ships, ensure that all ports belong to the same economy.
 */
void Fleet::check_merge_economy()
{
	if (m_ports.empty() || m_ships.empty())
		return;

	Flag & base = m_ports[0]->base_flag();
	for (uint32_t i = 1; i < m_ports.size(); ++i) {
		// Note: economy of base flag may of course be changed by the merge!
		base.get_economy()->check_merge(base, m_ports[i]->base_flag());
	}
}

void Fleet::cleanup(EditorGameBase & egbase)
{
	while (!m_ports.empty()) {
		PortDock * pd = m_ports.back();
		m_ports.pop_back();

		pd->set_fleet(nullptr);
		if (!m_ports.empty() && !m_ships.empty()) {
			// This is required when, during end-of-game cleanup,
			// the fleet gets removed before the ports
			Flag & base = m_ports[0]->base_flag();
			Economy::check_split(base, pd->base_flag());
		}
	}
	m_portpaths.clear();

	while (!m_ships.empty()) {
		m_ships.back()->set_fleet(nullptr);
		m_ships.pop_back();
	}

	MapObject::cleanup(egbase);
}

Fleet::PortPath & Fleet::portpath(uint32_t i, uint32_t j)
{
	assert(i < j);

	return m_portpaths[((j - 1) * j) / 2 + i];
}

const Fleet::PortPath & Fleet::portpath(uint32_t i, uint32_t j) const
{
	assert(i < j);

	return m_portpaths[((j - 1) * j) / 2 + i];
}

Fleet::PortPath & Fleet::portpath_bidir(uint32_t i, uint32_t j, bool & reverse)
{
	reverse = false;
	if (i > j) {
		reverse = true;
		std::swap(i, j);
	}
	return portpath(i, j);
}

const Fleet::PortPath & Fleet::portpath_bidir(uint32_t i, uint32_t j, bool & reverse) const
{
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
bool Fleet::get_path(PortDock & start, PortDock & end, Path & path)
{
	uint32_t startidx = std::find(m_ports.begin(), m_ports.end(), &start) - m_ports.begin();
	uint32_t endidx = std::find(m_ports.begin(), m_ports.end(), &end) - m_ports.begin();

	if (startidx >= m_ports.size() || endidx >= m_ports.size())
		return false;

	bool reverse;
	const PortPath & pp(portpath_bidir(startidx, endidx, reverse));

	if (pp.cost < 0)
		connect_port(owner().egbase(), startidx);

	if (pp.cost < 0)
		return false;

	path = *pp.path;
	if (reverse)
		path.reverse();

	return true;
}

uint32_t Fleet::count_ships(){
	return m_ships.size();
}

uint32_t Fleet::count_ships_heading_here(EditorGameBase & egbase, PortDock * port){
	uint32_t ships_on_way = 0;
	for (uint16_t s = 0; s < m_ships.size(); s += 1){
		if (m_ships[s]->get_destination(egbase) == port){
			ships_on_way += 1;
		}
	}

	return ships_on_way;
}

uint32_t Fleet::count_ports(){
	return m_ports.size();
}
bool Fleet::get_act_pending(){
	return m_act_pending;
}

void Fleet::add_neighbours(PortDock & pd, std::vector<RoutingNodeNeighbour> & neighbours)
{
	uint32_t idx = std::find(m_ports.begin(), m_ports.end(), &pd) - m_ports.begin();

	for (uint32_t otheridx = 0; otheridx < m_ports.size(); ++otheridx) {
		if (idx == otheridx)
			continue;

		bool reverse;
		PortPath & pp(portpath_bidir(idx, otheridx, reverse));

		if (pp.cost < 0) {
			// Lazily discover routes between ports
			connect_port(owner().egbase(), idx);
		}

		if (pp.cost >= 0) {
			// TODO(unknown): keep statistics on average transport time instead of using the arbitrary 2x factor
			RoutingNodeNeighbour neighb(&m_ports[otheridx]->base_flag(), 2 * pp.cost);
			neighbours.push_back(neighb);
		}
	}
}

void Fleet::add_ship(Ship * ship)
{
	m_ships.push_back(ship);
	ship->set_fleet(this);
	if (upcast(Game, game, &owner().egbase())) {
		if (m_ports.empty())
			ship->set_economy(*game, nullptr);
		else
			ship->set_economy(*game, m_ports[0]->get_economy());
	}

	if (m_ships.size() == 1) {
		check_merge_economy();
	}
}

void Fleet::remove_ship(EditorGameBase & egbase, Ship * ship)
{
	std::vector<Ship *>::iterator it = std::find(m_ships.begin(), m_ships.end(), ship);
	if (it != m_ships.end()) {
		*it = m_ships.back();
		m_ships.pop_back();
	}
	ship->set_fleet(nullptr);
	if (upcast(Game, game, &egbase))
		ship->set_economy(*game, nullptr);

	if (ship->get_destination(egbase)) {
		update(egbase);
	}

	if (m_ships.empty()) {
		if (m_ports.empty()) {
			remove(egbase);
		} else {
			Flag & base = m_ports[0]->base_flag();
			for (uint32_t i = 1; i < m_ports.size(); ++i) {
				// since two ports can be connected by land, it is possible that
				// disconnecting a previous port also disconnects later ports
				if (base.get_economy() == m_ports[i]->base_flag().get_economy())
					Economy::check_split(base, m_ports[i]->base_flag());
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

	int32_t estimate(Map & map, FCoords pos) const
	{
		int32_t est = std::numeric_limits<int32_t>::max();
		for (const Target& temp_target : targets) {
			est = std::min(est, map.calc_cost_estimate(pos, temp_target.pos));
		}
		return std::max(0, est - 5 * map.calc_cost(0));
	}

	int32_t stepcost(Map & map, FCoords from, int32_t /* fromcost */, WalkingDir dir, FCoords to) const
	{
		if (!(to.field->nodecaps() & MOVECAPS_SWIM))
			return -1;

		return map.calc_bidi_cost(from, dir);
	}
};

/**
 * Fill in all unknown paths to connect the port m_ports[idx] to the rest of the ports.
 *
 * Note that this is done lazily, i.e. the first time a path is actually requested,
 * because path finding is flaky during map loading.
 */
void Fleet::connect_port(EditorGameBase & egbase, uint32_t idx)
{
	Map & map = egbase.map();
	StepEvalFindPorts se;

	for (uint32_t i = 0; i < m_ports.size(); ++i) {
		if (i == idx)
			continue;

		bool reverse;
		if (portpath_bidir(i, idx, reverse).cost >= 0)
			continue;

		StepEvalFindPorts::Target tgt;
		tgt.idx = i;
		tgt.pos = m_ports[i]->get_warehouse()->get_position();
		se.targets.push_back(tgt);
	}

	if (se.targets.empty())
		return;

	MapAStar<StepEvalFindPorts> astar(map, se);

	BaseImmovable::PositionList src(m_ports[idx]->get_positions(egbase));
	for (const Coords& temp_pos : src) {
		astar.push(temp_pos);
	}

	int32_t cost;
	FCoords cur;
	while (!se.targets.empty() && astar.step(cur, cost)) {
		BaseImmovable * imm = cur.field->get_immovable();
		if (!imm || imm->descr().type() != MapObjectType::PORTDOCK)
			continue;

		if (upcast(PortDock, pd, imm)) {
			if (pd->get_owner() != get_owner())
				continue;

			if (pd->get_fleet() && pd->get_fleet() != this) {
				log("Fleet::connect_port: different fleets despite reachability\n");
				continue;
			}

			uint32_t otheridx = std::find(m_ports.begin(), m_ports.end(), pd) - m_ports.begin();
			if (idx == otheridx)
				continue;

			bool reverse;
			PortPath & ppath(portpath_bidir(idx, otheridx, reverse));

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

void Fleet::add_port(EditorGameBase & /* egbase */, PortDock * port)
{
	m_ports.push_back(port);
	port->set_fleet(this);
	if (m_ports.size() == 1) {
		set_economy(m_ports[0]->get_economy());
	} else {
		if (!m_ships.empty())
			m_ports[0]->get_economy()->check_merge(m_ports[0]->base_flag(), port->base_flag());
	}

	m_portpaths.resize((m_ports.size() * (m_ports.size() - 1)) / 2);
}

void Fleet::remove_port(EditorGameBase & egbase, PortDock * port)
{
	std::vector<PortDock *>::iterator it = std::find(m_ports.begin(), m_ports.end(), port);
	if (it != m_ports.end()) {
		uint32_t gap = it - m_ports.begin();
		for (uint32_t i = 0; i < gap; ++i) {
			portpath(i, gap) = portpath(i, m_ports.size() - 1);
		}
		for (uint32_t i = gap + 1; i < m_ports.size() - 1; ++i) {
			portpath(gap, i) = portpath(i, m_ports.size() - 1);
			if (portpath(gap, i).path)
				portpath(gap, i).path->reverse();
		}
		m_portpaths.resize((m_ports.size() * (m_ports.size() - 1)) / 2);

		*it = m_ports.back();
		m_ports.pop_back();
	}
	port->set_fleet(nullptr);

	if (m_ports.empty()) {
		set_economy(nullptr);
	} else {
		set_economy(m_ports[0]->get_economy());
		if (!m_ships.empty())
			Economy::check_split(m_ports[0]->base_flag(), port->base_flag());
	}

	if (m_ships.empty() && m_ports.empty())
		remove(egbase);
}

/**
 * Search among the docks of the fleet for the one that has the given flag as its base.
 *
 * @return the dock, or 0 if not found.
 */
PortDock * Fleet::get_dock(Flag & flag) const
{
	for (PortDock * temp_port : m_ports) {
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
PortDock * Fleet::get_dock(EditorGameBase & egbase, Coords field_coords) const
{
	for (PortDock * temp_port : m_ports) {
		for (Coords tmp_coords :  temp_port->get_positions(egbase)) {
			if (tmp_coords == field_coords){
				return temp_port;
			}
		}
	}

	return nullptr;
}

/**
 * @return an arbitrary dock of the fleet, or 0 if the fleet has no docks
 */
PortDock * Fleet::get_arbitrary_dock() const
{
	if (m_ports.empty())
		return nullptr;
	return m_ports[0];
}

/**
 * Trigger an update of ship scheduling
 */
void Fleet::update(EditorGameBase & egbase)
{
	if (m_act_pending){
		return;
	}

	if (upcast(Game, game, &egbase)) {
		schedule_act(*game, 100);
		m_act_pending = true;
	}
}

/**
 * Act callback updates ship scheduling. All decisions about where transport ships
 * are supposed to go are made via this function.
 *
 * @note Do not call this directly; instead, trigger it via @ref update
 */
void Fleet::act(Game & game, uint32_t /* data */)
{
	m_act_pending = false;

	if (!active()) {
		// If we are here, most likely act() was called by a port with waiting wares or an expedition ready
		// although there are still no ships. We can't handle it now, so we reschedule the act()
		schedule_act(game, 5000); // retry in the next time
		m_act_pending = true;
		return;
	}

	molog("Fleet::act\n");

	// we need to calculate what ship is to be send to which port
	// for this we will have temporary data structure with format
	// <<ship,port>,score>
	// where ship and port are not objects but positions in m_ports and m_ships
	// this is to allow native hashing
	std::map<std::pair<uint16_t, uint16_t>, uint16_t> scores;

	// so we will identify all pairs: idle ship : ports, and score all such
	// pairs. We consider
	// - count of wares onboard, first ware (oldest) is counted as 8 (prioritization)
	//   (counting wares for particular port only)
	// - count wares waiting at the port/3
	// - distance between ship and a port (0-10 points, the closer the more points)
	// - is another ship heading there right now?

	// at the end we must know if requrests of all ports asking for ship were addressed
	// if any unsatisfied, we must schedule new run of this function
	// when we send a ship there, the port is removed from list
	std::list<uint16_t> waiting_ports;

	// this is just helper - first member of scores map
	std::pair<uint16_t, uint16_t> mapping; //ship number, port number

	// first we go over ships - idle ones (=without destination)
	// then over wares on these ships and create first ship-port
	// pairs with score
	for (uint16_t s = 0; s < m_ships.size(); s += 1){
		if (m_ships[s]->get_destination(game)) {
			continue;
		}
		if (m_ships[s]->get_ship_state() != Ship::TRANSPORT) {
			continue; // in expedition obviously
		}

		for (uint16_t i = 0; i < m_ships[s]->get_nritems(); i += 1){
			PortDock * dst = m_ships[s]->m_items[i].get_destination(game);
			if (!dst) {
				// if wares without destination on ship without destination
				// such ship can be send to any port, and should be sent
				// to some port, so we add 1 point to score for each port
				for (uint16_t p = 0; p < m_ports.size(); p += 1){
					mapping.first = s;
					mapping.second = p;
					scores[mapping] += 1;
				}
				continue;
			}

			bool destination_found = false; //just a functional check
			for (uint16_t p = 0; p < m_ports.size(); p += 1){
				if (m_ports[p] ==  m_ships[s]->m_items[i].get_destination(game)){
					mapping.first = s;
					mapping.second = p;
					scores[mapping] += (i == 0)?8:1;
					destination_found = true;
				}
			}
			if (!destination_found){
				// Perhaps the throw here is too strong
				// we can still remove it before stable release if it proves too much
				// during my testing this situation never happened
				throw wexception("A ware with destination that does not match any of player's"
				" ports, ship %u, ware's destination: %u",
				m_ships[s]->serial(),
				m_ships[s]->m_items[i].get_destination(game)->serial());
			}
		}
	}

	// now opposite aproach - we go over ports to find out those that have wares
	// waiting for ship then find candidate ships to satisfy the requests
	for (uint16_t p = 0; p < m_ports.size(); p += 1){
		PortDock & pd = *m_ports[p];
		if (!pd.get_need_ship()){
			continue;
		}

		// general stategy is "one ship for port is enough", but sometimes
		// amount of ware waiting for ship is too high
		if (count_ships_heading_here(game, &pd) * 25 > pd.count_waiting()) {
			continue;
		}

		waiting_ports.push_back(p);

		// scoring and entering the pair into scores (or increasing existing
		// score if the pair is already there)
		for (uint16_t s = 0; s < m_ships.size(); s += 1){

			if (m_ships[s]->get_destination(game)) {
				continue; // already has destination
			}

			if (m_ships[s]->get_ship_state() != Ship::TRANSPORT) {
				continue; // in expedition obviously
			}

			mapping.first = s;
			mapping.second = p;
			// folowing aproximately considers free capacity of a ship
			scores[mapping] += ((m_ships[s]->get_nritems() > 15)?1:3)
			+
			std::min(
				m_ships[s]->descr().get_capacity() - m_ships[s]->get_nritems(),
				m_ports[p]->count_waiting()) / 3;
		}
	}

	//now adding score for distance
	for (std::pair<std::pair<uint16_t, uint16_t>, uint16_t> ship_port_relation : scores) {

		// here we get distance ship->port
		// possibilities are:
		// - we are in port and it is the same as target port
		// - we are in other port, then we use get_dock() function to fetch precalculated path
		// - if above fails, we calculate path "manually"
		int16_t route_length = -1;

		PortDock * current_portdock = get_dock(game, m_ships[ship_port_relation.first.first]->get_position());

		if (current_portdock) { // we try to use precalculated paths of game

			// we are in the same portdock
			if (current_portdock == m_ports[ship_port_relation.first.second]) {
				route_length = 0;
			} else { // it is different portdock then
				Path tmp_path;
				if (get_path(*current_portdock, *m_ports[ship_port_relation.first.second], tmp_path)) {
					route_length = tmp_path.get_nsteps();
				}
			}
		}

		// most probably the ship is not in a portdock (should not happen frequently)
		if (route_length == -1) {
			route_length = m_ships[ship_port_relation.first.first]->calculate_sea_route
			(game, *m_ports[ship_port_relation.first.second]);
		}

		// now we have length of route, so we need to calculate score
		int16_t score_for_distance = 0;
		if (route_length < 3) {
			score_for_distance = 10;
		} else {
			score_for_distance = 8 - route_length / 50;
		}
		// must not be negative
		score_for_distance = (score_for_distance < 0)?0:score_for_distance;

		scores[ship_port_relation.first] += score_for_distance;
	}

	// looking for best scores and sending ships accordingly
	uint16_t best_ship = 0;
	uint16_t best_port = 0;
	uint16_t best_score;

	// after sending a ship we will remove one or more items from scores
	while (!scores.empty()){
		best_score = 0;

		// searching for combination with highest score
		for (std::pair<std::pair<uint16_t, uint16_t>, uint16_t> combination : scores) {
			if (combination.second > best_score){
				best_score = combination.second;
				best_ship = combination.first.first;
				best_port = combination.first.second;
			}
		}
		if (best_score == 0){
			// this is check of correctnes of this algorithm, this should not happen
			throw wexception("Fleet::act(): No port-destination pair selected or its score is zero");
		}

		// making sure the winner has no destination set
		assert(!m_ships[best_ship]->get_destination(game));

		// now actual setting destination for "best ship"
		m_ships[best_ship]->set_destination(game, *m_ports[best_port]);
		molog("... ship %u sent to port %u, wares onboard: %2d, the port is asking for a ship: %s\n",
		m_ships[best_ship]->serial(),
		m_ports[best_port]->serial(),
		m_ships[best_ship]->get_nritems(),
		(m_ports[best_port]->get_need_ship())?"yes":"no");

		// pruning the scores table
		// the ship that was just sent somewhere cannot be send elsewhere :)
		for (auto it = scores.cbegin(); it != scores.cend();){

			// decreasing score for target port as there was a ship just sent there
			if (it->first.second == best_port) {
				mapping.first = it->first.first;
				mapping.second = it->first.second;
				scores[mapping] /= 2;
				// just make sure it is nonzero
				scores[mapping] = (scores[mapping] == 0)?1:scores[mapping];
			}

			// but removing all pairs where best ship is participating as it is not available anymore
			// (because it was sent to "best port")
			if (it->first.first == best_ship) {
				scores.erase(it++);
			} else {
				++it;
			}
		}

		// also removing the port from waiting_ports
		waiting_ports.remove(best_port);
	}

	if (!waiting_ports.empty()) {
		molog("... there are %" PRIuS " ports requesting ship(s) we cannot satisfy yet\n",
		waiting_ports.size());
		schedule_act(game, 5000); // retry next time
		m_act_pending = true;
	}
}

void Fleet::log_general_info(const EditorGameBase & egbase)
{
	MapObject::log_general_info(egbase);

	molog ("%" PRIuS " ships and %" PRIuS " ports\n",  m_ships.size(), m_ports.size());
}

#define FLEET_SAVEGAME_VERSION 4

Fleet::Loader::Loader()
{
}

void Fleet::Loader::load(FileRead & fr, uint8_t version)
{
	MapObject::Loader::load(fr);

	Fleet & fleet = get<Fleet>();

	uint32_t nrships = fr.unsigned_32();
	m_ships.resize(nrships);
	for (uint32_t i = 0; i < nrships; ++i)
		m_ships[i] = fr.unsigned_32();

	uint32_t nrports = fr.unsigned_32();
	m_ports.resize(nrports);
	for (uint32_t i = 0; i < nrports; ++i)
		m_ports[i] = fr.unsigned_32();

	if (version >= 2) {
		fleet.m_act_pending = fr.unsigned_8();
		if (version < 3)
			fleet.m_act_pending = false;
		if (version < 4)
			fr.unsigned_32(); // m_roundrobin
	}
}

void Fleet::Loader::load_pointers()
{
	MapObject::Loader::load_pointers();

	Fleet & fleet = get<Fleet>();

	// Act commands created during loading are not persistent, so we need to undo any
	// changes to the pending state.
	bool save_act_pending = fleet.m_act_pending;

	for (const uint32_t& temp_ship : m_ships) {
		fleet.m_ships.push_back(&mol().get<Ship>(temp_ship));
		fleet.m_ships.back()->set_fleet(&fleet);
	}
	for (const uint32_t& temp_port: m_ports) {
		fleet.m_ports.push_back(&mol().get<PortDock>(temp_port));
		fleet.m_ports.back()->set_fleet(&fleet);
	}

	fleet.m_portpaths.resize((fleet.m_ports.size() * (fleet.m_ports.size() - 1)) / 2);

	fleet.m_act_pending = save_act_pending;
}

void Fleet::Loader::load_finish()
{
	MapObject::Loader::load_finish();

	Fleet & fleet = get<Fleet>();

	if (!fleet.m_ports.empty()) {
		if (!fleet.m_ships.empty())
			fleet.check_merge_economy();

		fleet.set_economy(fleet.m_ports[0]->get_economy());
	}
}

MapObject::Loader * Fleet::load
		(EditorGameBase & egbase, MapObjectLoader & mol, FileRead & fr)
{
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const version = fr.unsigned_8();
		if (1 <= version && version <= FLEET_SAVEGAME_VERSION) {
			PlayerNumber owner_number = fr.unsigned_8();
			if (!owner_number || owner_number > egbase.map().get_nrplayers())
				throw GameDataError
					("owner number is %u but there are only %u players",
					 owner_number, egbase.map().get_nrplayers());

			Player * owner = egbase.get_player(owner_number);
			if (!owner)
				throw GameDataError("owning player %u does not exist", owner_number);

			loader->init(egbase, mol, *(new Fleet(*owner)));
			loader->load(fr, version);
		} else
			throw GameDataError("unknown/unhandled version %u", version);
	} catch (const std::exception & e) {
		throw wexception("loading portdock: %s", e.what());
	}

	return loader.release();
}

void Fleet::save(EditorGameBase & egbase, MapObjectSaver & mos, FileWrite & fw)
{
	fw.unsigned_8(HeaderFleet);
	fw.unsigned_8(FLEET_SAVEGAME_VERSION);

	fw.unsigned_8(m_owner.player_number());

	MapObject::save(egbase, mos, fw);

	fw.unsigned_32(m_ships.size());
	for (const Ship * temp_ship : m_ships) {
		fw.unsigned_32(mos.get_object_file_index(*temp_ship));
	}
	fw.unsigned_32(m_ports.size());
	for (const PortDock * temp_port : m_ports) {
		fw.unsigned_32(mos.get_object_file_index(*temp_port));
	}

	fw.unsigned_8(m_act_pending);
}

} // namespace Widelands
