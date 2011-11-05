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

#include "portdock.h"

#include "container_iterate.h"
#include "fleet.h"
#include "log.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/warehouse.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"

namespace Widelands {

Map_Object_Descr portdock_descr("portdock", "Port Dock");

PortDock::PortDock() :
	PlayerImmovable(portdock_descr),
	m_fleet(0),
	m_warehouse(0)
{
}

/**
 * Add a position where ships may dock.
 *
 * The caller is responsible for ensuring that the positions are connected
 * by water, i.e. ships can move freely between the positions.
 *
 * @param where must be a field that is entirely water
 *
 * @note This only works properly when called before @ref init
 */
void PortDock::add_position(Coords where)
{
	m_dockpoints.push_back(where);
}

/**
 * Set the @ref Warehouse buddy of this dock. May only be called once.
 */
void PortDock::set_warehouse(Warehouse * wh)
{
	assert(!m_warehouse);

	m_warehouse = wh;
}

/**
 * Update which @ref Fleet we belong to.
 *
 * @warning This should only be called via @ref Fleet itself.
 */
void PortDock::set_fleet(Fleet * fleet)
{
	m_fleet = fleet;
}

int32_t PortDock::get_size() const throw ()
{
	return SMALL;
}

bool PortDock::get_passable() const throw ()
{
	return true;
}

int32_t PortDock::get_type() const throw ()
{
	return PORTDOCK;
}

char const * PortDock::type_name() const throw ()
{
	return "portdock";
}

PortDock::PositionList PortDock::get_positions
	(const Editor_Game_Base &) const throw ()
{
	return m_dockpoints;
}

Flag & PortDock::base_flag()
{
	return m_warehouse->base_flag();
}

void PortDock::draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point)
{
	// do nothing
}

std::string const & PortDock::name() const throw ()
{
	static std::string name("portdock");
	return name;
}

void PortDock::init(Editor_Game_Base & egbase)
{
	PlayerImmovable::init(egbase);

	container_iterate_const(PositionList, m_dockpoints, it) {
		set_position(egbase, *it.current);
	}

	init_fleet(egbase);
}

/**
 * Create our initial singleton @ref Fleet. The fleet code ensures
 * that we merge with a larger fleet when possible.
 */
void PortDock::init_fleet(Editor_Game_Base & egbase)
{
	Fleet * fleet = new Fleet(owner());
	fleet->add_port(this);
	fleet->init(egbase);
	// Note: the Fleet calls our set_fleet automatically
}

void PortDock::cleanup(Editor_Game_Base & egbase)
{
	if (m_fleet)
		m_fleet->remove_port(egbase, this);

	container_iterate_const(PositionList, m_dockpoints, it) {
		unset_position(egbase, *it.current);
	}

	if (m_warehouse)
		m_warehouse->m_portdock = 0;

	PlayerImmovable::cleanup(egbase);
}

void PortDock::log_general_info(Editor_Game_Base const & egbase)
{
	PlayerImmovable::log_general_info(egbase);

	Coords pos(m_warehouse->get_position());
	molog
		("PortDock for warehouse %u (at %i,%i)\n",
		 m_warehouse ? m_warehouse->serial() : 0, pos.x, pos.y);
	molog("Fleet: %u\n", m_fleet ? m_fleet->serial() : 0);
	molog("%zu dock points\n", m_dockpoints.size());
}

#define PORTDOCK_SAVEGAME_VERSION 1

PortDock::Loader::Loader()
{
}

void PortDock::Loader::load(FileRead & fr)
{
	PlayerImmovable::Loader::load(fr);

	PortDock & pd = get<PortDock>();

	m_warehouse = fr.Unsigned32();
	uint16_t nrdockpoints = fr.Unsigned16();

	pd.m_dockpoints.resize(nrdockpoints);
	for (uint16_t i = 0; i < nrdockpoints; ++i) {
		pd.m_dockpoints[i] = fr.Coords32(egbase().map().extent());
		pd.set_position(egbase(), pd.m_dockpoints[i]);
	}
}

void PortDock::Loader::load_pointers()
{
	PlayerImmovable::Loader::load_pointers();

	PortDock & pd = get<PortDock>();
	pd.m_warehouse = &mol().get<Warehouse>(m_warehouse);
}

void PortDock::Loader::load_finish()
{
	PlayerImmovable::Loader::load_finish();

	PortDock & pd = get<PortDock>();

	if (pd.m_warehouse->get_portdock() != &pd) {
		log("Inconsistent PortDock <> Warehouse link\n");
		if (upcast(Game, game, &egbase()))
			pd.schedule_destroy(*game);
	}

	// This shouldn't be necessary, but let's check just in case
	if (!pd.m_fleet)
		pd.init_fleet(egbase());
}

Map_Object::Loader * PortDock::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::auto_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (1 <= version && version <= PORTDOCK_SAVEGAME_VERSION) {
			loader->init(egbase, mol, *new PortDock);
			loader->load(fr);
		} else
			throw game_data_error(_("unknown/unhandled version %u"), version);
	} catch (const std::exception & e) {
		throw wexception(_("loading portdock: %s"), e.what());
	}

	return loader.release();
}

void PortDock::save(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_PortDock);
	fw.Unsigned8(PORTDOCK_SAVEGAME_VERSION);

	PlayerImmovable::save(egbase, mos, fw);

	fw.Unsigned32(mos.get_object_file_index(*m_warehouse));
	fw.Unsigned16(m_dockpoints.size());
	container_iterate_const(PositionList, m_dockpoints, it) {
		fw.Coords32(*it);
	}
}

} // namespace Widelands
