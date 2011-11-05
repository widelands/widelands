/*
 * Copyright (C) 2011 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "fleet.h"

#include "container_iterate.h"
#include "portdock.h"
#include "logic/game.h"
#include "logic/mapastar.h"
#include "logic/player.h"
#include "logic/ship.h"
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
	m_owner(player)
{
}

int32_t Fleet::get_type() const throw ()
{
	return FLEET;
}

char const * Fleet::type_name() const throw ()
{
	return "fleet";
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
}

struct StepEvalFindFleet {
	int32_t estimate(FCoords pos) const {return 0;}
	int32_t stepcost(FCoords from, int32_t fromcost, FCoords to) const {
		if (!(to.field->nodecaps() & MOVECAPS_SWIM))
			return -1;

		if (from.field->nodecaps() & MOVECAPS_WALK) {
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

		for (Bob * bob = cur.field->get_first_bob(); bob != 0; bob = bob->get_next_bob()) {
			if (bob->get_bob_type() != Bob::SHIP)
				continue;

			if (upcast(Ship, ship, bob)) {
				if (ship->get_fleet() != this && ship->get_owner() == get_owner()) {
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
	while (!other->m_ships.empty()) {
		Ship * ship = other->m_ships.back();
		other->m_ships.pop_back();
		add_ship(ship);
	}
	while (!other->m_ports.empty()) {
		PortDock * port = other->m_ports.back();
		other->m_ports.pop_back();
		add_port(port);
	}
	other->remove(egbase);
}

void Fleet::cleanup(Editor_Game_Base & egbase)
{
	while (!m_ports.empty()) {
		m_ports.back()->set_fleet(0);
		m_ports.pop_back();
	}
	while (!m_ships.empty()) {
		m_ships.back()->set_fleet(0);
		m_ships.pop_back();
	}

	Map_Object::cleanup(egbase);
}

void Fleet::add_ship(Ship * ship)
{
	m_ships.push_back(ship);
	ship->set_fleet(this);
}

void Fleet::remove_ship(Editor_Game_Base & egbase, Ship * ship)
{
	std::vector<Ship *>::iterator it = std::find(m_ships.begin(), m_ships.end(), ship);
	if (it != m_ships.end()) {
		*it = m_ships.back();
		m_ships.pop_back();
	}
	ship->set_fleet(0);

	if (m_ships.empty() && m_ports.empty())
		remove(egbase);
}

void Fleet::add_port(PortDock * port)
{
	m_ports.push_back(port);
	port->set_fleet(this);
}

void Fleet::remove_port(Editor_Game_Base & egbase, PortDock * port)
{
	std::vector<PortDock *>::iterator it = std::find(m_ports.begin(), m_ports.end(), port);
	if (it != m_ports.end()) {
		*it = m_ports.back();
		m_ports.pop_back();
	}
	port->set_fleet(0);

	if (m_ships.empty() && m_ports.empty())
		remove(egbase);
}

void Fleet::log_general_info(Editor_Game_Base const & egbase)
{
	Map_Object::log_general_info(egbase);

	molog("%zu ships and %zu ports\n", m_ships.size(), m_ports.size());
}

#define FLEET_SAVEGAME_VERSION 1

Fleet::Loader::Loader()
{
}

void Fleet::Loader::load(FileRead & fr)
{
	Map_Object::Loader::load(fr);

	uint32_t nrships = fr.Unsigned32();
	m_ships.resize(nrships);
	for (uint32_t i = 0; i < nrships; ++i)
		m_ships[i] = fr.Unsigned32();

	uint32_t nrports = fr.Unsigned32();
	m_ports.resize(nrports);
	for (uint32_t i = 0; i < nrports; ++i)
		m_ports[i] = fr.Unsigned32();
}

void Fleet::Loader::load_pointers()
{
	Map_Object::Loader::load_pointers();

	Fleet & fleet = get<Fleet>();

	container_iterate_const(std::vector<uint32_t>, m_ships, it) {
		fleet.add_ship(&mol().get<Ship>(*it));
	}
	container_iterate_const(std::vector<uint32_t>, m_ports, it) {
		fleet.add_port(&mol().get<PortDock>(*it));
	}
}

void Fleet::Loader::load_finish()
{
	Map_Object::Loader::load_finish();
}

Map_Object::Loader * Fleet::load
		(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::auto_ptr<Loader> loader(new Loader);

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
			loader->load(fr);
		} else
			throw game_data_error(_("unknown/unhandled version %u"), version);
	} catch (const std::exception & e) {
		throw wexception(_("loading portdock: %s"), e.what());
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
}

} // namespace Widelands
