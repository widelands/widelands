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

#include "container_iterate.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/routing_node.h"
#include "logic/game.h"
#include "logic/mapastar.h"
#include "logic/path.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/warehouse.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "upcast.h"

namespace Widelands {

Map_Object_Descr fleet_descr("fleet", "Fleet");

/**
 * Fleets are initialized empty.
 *
 * Intended use: @ref Ship and @ref PortDock, when created, create a new @ref Fleet
 * instance, then add themselves \em before calling the \ref init function.
 * The Fleet takes care of merging with existing fleets, if any.
 */
Fleet::Fleet(Player & player) :
	Map_Object(&fleet_descr),
	m_owner(player),
	m_act_pending(false)
{
}

int32_t Fleet::get_type() const
{
	return FLEET;
}

char const * Fleet::type_name() const
{
	return "fleet";
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
			container_iterate_const(std::vector<Ship *>, m_ships, shipit) {
				(*shipit.current)->set_economy(*game, e);
			}
		}
	}
}

/**
 * Initialize the fleet, including a search through the map
 * to rejoin with the next other fleet we can find.
 */
void Fleet::init(Editor_Game_Base & egbase)
{
	Map_Object::init(egbase);

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
void Fleet::find_other_fleet(Editor_Game_Base & egbase)
{
	Map & map = egbase.map();
	MapAStar<StepEvalFindFleet> astar(map, StepEvalFindFleet());

	container_iterate_const(std::vector<Ship *>, m_ships, it) {
		astar.push((*it.current)->get_position());
	}

	container_iterate_const(std::vector<PortDock *>, m_ports, it) {
		BaseImmovable::PositionList pos = (*it.current)->get_positions(egbase);

		container_iterate_const(BaseImmovable::PositionList, pos, posit) {
			astar.push(*posit.current);
		}
	}

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (BaseImmovable * imm = cur.field->get_immovable()) {
			if (imm->get_type() == PORTDOCK) {
				if (upcast(PortDock, dock, imm)) {
					if (dock->get_fleet() != this && dock->get_owner() == get_owner()) {
						dock->get_fleet()->merge(egbase, this);
						return;
					}
				}
			}
		}

		for (Bob * bob = cur.field->get_first_bob(); bob != nullptr; bob = bob->get_next_bob()) {
			if (bob->get_bob_type() != Bob::SHIP)
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
void Fleet::merge(Editor_Game_Base & egbase, Fleet * other)
{
	if (m_ports.empty() and not other->m_ports.empty()) {
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

void Fleet::cleanup(Editor_Game_Base & egbase)
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

	Map_Object::cleanup(egbase);
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
			// TODO: keep statistics on average transport time instead of using the arbitrary 2x factor
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

void Fleet::remove_ship(Editor_Game_Base & egbase, Ship * ship)
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
		container_iterate_const(std::vector<Target>, targets, it) {
			est = std::min(est, map.calc_cost_estimate(pos, it.current->pos));
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
void Fleet::connect_port(Editor_Game_Base & egbase, uint32_t idx)
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
	container_iterate_const(BaseImmovable::PositionList, src, it) {
		astar.push(*it.current);
	}

	int32_t cost;
	FCoords cur;
	while (!se.targets.empty() && astar.step(cur, cost)) {
		BaseImmovable * imm = cur.field->get_immovable();
		if (!imm || imm->get_type() != PORTDOCK)
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

			container_iterate(std::vector<StepEvalFindPorts::Target>, se.targets, it) {
				if (it.current->idx == otheridx) {
					*it.current = se.targets.back();
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

void Fleet::add_port(Editor_Game_Base & /* egbase */, PortDock * port)
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

void Fleet::remove_port(Editor_Game_Base & egbase, PortDock * port)
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
	container_iterate_const(std::vector<PortDock *>, m_ports, portit) {
		if (&(*portit.current)->base_flag() == &flag)
			return *portit.current;
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
void Fleet::update(Editor_Game_Base & egbase)
{
	if (m_act_pending)
		return;

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

	container_iterate_const(std::vector<Ship *>, m_ships, shipit) {
		Ship & ship = **shipit.current;
		if (ship.get_nritems() > 0 && !ship.get_destination(game)) {
			molog("Ship %u has items\n", ship.serial());
			bool found_dst = false;
			container_iterate(std::vector<ShippingItem>, ship.m_items, it) {
				PortDock * dst = it->get_destination(game);
				if (dst) {
					molog("... sending to portdock %u\n", dst->serial());
					ship.set_destination(game, *dst);
					found_dst = true;
					break;
				}
			}
			// If we end here, we just send the ship to the first port - maybe the old port got destroyed
			if (!found_dst) {
				assert(!m_ports.empty());
				ship.set_destination(game, *m_ports[0]);
			}
		}
	}

	for (uint32_t i = 0; i < m_ports.size(); ++i) {
		PortDock & pd = *m_ports[i];

		if (pd.get_need_ship()) {
			molog("Port %u needs ship\n", pd.serial());

			bool success = false;
			container_iterate_const(std::vector<Ship *>, m_ships, shipit) {
				Ship & ship = **shipit.current;
				// Check whether ship is in TRANSPORT state
				if (ship.get_ship_state() != Ship::TRANSPORT)
					continue;

				PortDock * dst = ship.get_destination(game);
				// Check if ship has currently a different destination
				if (dst && dst != &pd)
					continue;
				if (ship.get_nritems() >= ship.get_capacity())
					continue;

				molog("... ship %u takes care of it\n", ship.serial());

				if (!dst)
					ship.set_destination(game, pd);

				success = true;
				break;
			}

			if (!success) {
				schedule_act(game, 5000); // retry in the next time
				m_act_pending = true;
				break;
			}
		}
	}
}

void Fleet::log_general_info(const Editor_Game_Base & egbase)
{
	Map_Object::log_general_info(egbase);

	molog
		("%" PRIuS " ships and %" PRIuS " ports\n",  m_ships.size(), m_ports.size());
}

#define FLEET_SAVEGAME_VERSION 4

Fleet::Loader::Loader()
{
}

void Fleet::Loader::load(FileRead & fr, uint8_t version)
{
	Map_Object::Loader::load(fr);

	Fleet & fleet = get<Fleet>();

	uint32_t nrships = fr.Unsigned32();
	m_ships.resize(nrships);
	for (uint32_t i = 0; i < nrships; ++i)
		m_ships[i] = fr.Unsigned32();

	uint32_t nrports = fr.Unsigned32();
	m_ports.resize(nrports);
	for (uint32_t i = 0; i < nrports; ++i)
		m_ports[i] = fr.Unsigned32();

	if (version >= 2) {
		fleet.m_act_pending = fr.Unsigned8();
		if (version < 3)
			fleet.m_act_pending = false;
		if (version < 4)
			fr.Unsigned32(); // m_roundrobin
	}
}

void Fleet::Loader::load_pointers()
{
	Map_Object::Loader::load_pointers();

	Fleet & fleet = get<Fleet>();

	// Act commands created during loading are not persistent, so we need to undo any
	// changes to the pending state.
	bool save_act_pending = fleet.m_act_pending;

	container_iterate_const(std::vector<uint32_t>, m_ships, it) {
		fleet.m_ships.push_back(&mol().get<Ship>(*it));
		fleet.m_ships.back()->set_fleet(&fleet);
	}
	container_iterate_const(std::vector<uint32_t>, m_ports, it) {
		fleet.m_ports.push_back(&mol().get<PortDock>(*it));
		fleet.m_ports.back()->set_fleet(&fleet);
	}

	fleet.m_portpaths.resize((fleet.m_ports.size() * (fleet.m_ports.size() - 1)) / 2);

	fleet.m_act_pending = save_act_pending;
}

void Fleet::Loader::load_finish()
{
	Map_Object::Loader::load_finish();

	Fleet & fleet = get<Fleet>();

	if (!fleet.m_ports.empty()) {
		if (!fleet.m_ships.empty())
			fleet.check_merge_economy();

		fleet.set_economy(fleet.m_ports[0]->get_economy());
	}
}

Map_Object::Loader * Fleet::load
		(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const version = fr.Unsigned8();
		if (1 <= version && version <= FLEET_SAVEGAME_VERSION) {
			Player_Number owner_number = fr.Unsigned8();
			if (!owner_number || owner_number > egbase.map().get_nrplayers())
				throw game_data_error
					("owner number is %u but there are only %u players",
					 owner_number, egbase.map().get_nrplayers());

			Player * owner = egbase.get_player(owner_number);
			if (!owner)
				throw game_data_error("owning player %u does not exist", owner_number);

			loader->init(egbase, mol, *(new Fleet(*owner)));
			loader->load(fr, version);
		} else
			throw game_data_error("unknown/unhandled version %u", version);
	} catch (const std::exception & e) {
		throw wexception("loading portdock: %s", e.what());
	}

	return loader.release();
}

void Fleet::save(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Fleet);
	fw.Unsigned8(FLEET_SAVEGAME_VERSION);

	fw.Unsigned8(m_owner.player_number());

	Map_Object::save(egbase, mos, fw);

	fw.Unsigned32(m_ships.size());
	container_iterate_const(std::vector<Ship *>, m_ships, it) {
		fw.Unsigned32(mos.get_object_file_index(**it));
	}
	fw.Unsigned32(m_ports.size());
	container_iterate_const(std::vector<PortDock *>, m_ports, it) {
		fw.Unsigned32(mos.get_object_file_index(**it));
	}

	fw.Unsigned8(m_act_pending);
}

} // namespace Widelands
