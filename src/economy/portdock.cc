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

#include "economy/portdock.h"

#include <boost/foreach.hpp>

#include "container_iterate.h"
#include "economy/fleet.h"
#include "economy/ware_instance.h"
#include "economy/wares_queue.h"
#include "log.h"
#include "logic/expedition_bootstrap.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/warehouse.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

Map_Object_Descr g_portdock_descr("portdock", "Port Dock");

PortDock::PortDock(Warehouse* wh) :
	PlayerImmovable(g_portdock_descr),
	m_fleet(nullptr),
	m_warehouse(wh),
	m_need_ship(false),
	m_expedition_ready(false)
{
}

PortDock::~PortDock() {
	assert(m_expedition_bootstrap.get() == nullptr);
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

Warehouse* PortDock::get_warehouse() const {
	return m_warehouse;
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

int32_t PortDock::get_size() const
{
	return SMALL;
}

bool PortDock::get_passable() const
{
	return true;
}

int32_t PortDock::get_type() const
{
	return PORTDOCK;
}

char const * PortDock::type_name() const
{
	return "portdock";
}

PortDock::PositionList PortDock::get_positions
	(const Editor_Game_Base &) const
{
	return m_dockpoints;
}

Flag & PortDock::base_flag()
{
	return m_warehouse->base_flag();
}

/**
 * Return the dock that has the given flag as its base, or 0 if no dock of our fleet
 * has the given flag.
 */
PortDock * PortDock::get_dock(Flag & flag) const
{
	if (m_fleet)
		return m_fleet->get_dock(flag);
	return nullptr;
}

/**
 * Signal to the dock that it now belongs to the given economy.
 *
 * Called by @ref Warehouse::set_economy, and responsible for forwarding the
 * change to @ref Fleet.
 */
void PortDock::set_economy(Economy * e)
{
	if (e == get_economy())
		return;

	PlayerImmovable::set_economy(e);
	if (m_fleet)
		m_fleet->set_economy(e);

	if (upcast(Game, game, &owner().egbase())) {
		container_iterate(std::vector<ShippingItem>, m_waiting, it) {
			it->set_economy(*game, e);
		}
	}

	if (m_expedition_bootstrap)
		m_expedition_bootstrap->set_economy(e);
}


void PortDock::draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&)
{
	// do nothing
}

const std::string & PortDock::name() const
{
	static const std::string name_("portdock");
	return name_;
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
	fleet->add_port(egbase, this);
	fleet->init(egbase);
	// Note: the Fleet calls our set_fleet automatically
}

void PortDock::cleanup(Editor_Game_Base & egbase)
{
	if (egbase.objects().object_still_available(m_warehouse)) {
		// Transfer all our wares into the warehouse.
		if (upcast(Game, game, &egbase)) {
			BOOST_FOREACH(ShippingItem & shipping_item, m_waiting) {
				WareInstance* ware;
				shipping_item.get(*game, &ware, nullptr);
				if (ware) {
					ware->cancel_moving();
					m_warehouse->incorporate_ware(*game, ware);
				} else {
					shipping_item.set_location(*game, m_warehouse);
					shipping_item.end_shipping(*game);
				}
			}
		}
		m_waiting.clear();
		m_warehouse->m_portdock = nullptr;
	}

	if (upcast(Game, game, &egbase)) {
		container_iterate(std::vector<ShippingItem>, m_waiting, it) {
			it.current->remove(*game);
		}
	}

	if (m_fleet)
		m_fleet->remove_port(egbase, this);

	container_iterate_const(PositionList, m_dockpoints, it) {
		unset_position(egbase, *it.current);
	}

	if (m_expedition_bootstrap) {
		m_expedition_bootstrap->cleanup(egbase);
		m_expedition_bootstrap.reset(nullptr);
	}

	PlayerImmovable::cleanup(egbase);
}

/**
 * Add the flags of all ports that can be reached via this dock.
 */
void PortDock::add_neighbours(std::vector<RoutingNodeNeighbour> & neighbours)
{
	if (m_fleet && m_fleet->active())
		m_fleet->add_neighbours(*this, neighbours);
}

/**
 * The given @p ware enters the dock, waiting to be transported away.
 */
void PortDock::add_shippingitem(Game & game, WareInstance & ware)
{
	m_waiting.push_back(ShippingItem(ware));
	ware.set_location(game, this);
	ware.update(game);
}

/**
 * The given @p ware, which is assumed to be inside the dock, has updated
 * its route.
 */
void PortDock::update_shippingitem(Game & game, WareInstance & ware)
{
	container_iterate(std::vector<ShippingItem>, m_waiting, it) {
		if (it.current->m_object.serial() == ware.serial()) {
			_update_shippingitem(game, it.current);
			return;
		}
	}
}

/**
 * The given @p worker enters the dock, waiting to be transported away.
 */
void PortDock::add_shippingitem(Game & game, Worker & worker)
{
	m_waiting.push_back(ShippingItem(worker));
	worker.set_location(this);
	update_shippingitem(game, worker);
}

/**
 * The given @p worker, which is assumed to be inside the dock, has
 * updated its route.
 */
void PortDock::update_shippingitem(Game & game, Worker & worker)
{
	container_iterate(std::vector<ShippingItem>, m_waiting, it) {
		if (it.current->m_object.serial() == worker.serial()) {
			_update_shippingitem(game, it.current);
			return;
		}
	}
}

void PortDock::_update_shippingitem(Game & game, std::vector<ShippingItem>::iterator it)
{
	it->update_destination(game, *this);

	PortDock * dst = it->get_destination(game);
	assert(dst != this);

	// Destination might have vanished or be in another economy altogether.
	if (dst && dst->get_economy() == get_economy()) {
		set_need_ship(game, true);
	} else {
		it->set_location(game, m_warehouse);
		it->end_shipping(game);
		*it = m_waiting.back();
		m_waiting.pop_back();

		if (m_waiting.empty())
			set_need_ship(game, false);
	}
}

/**
 * A ship has arrived at the dock. Clear all items designated for this dock,
 * and load the ship.
 */
void PortDock::ship_arrived(Game & game, Ship & ship)
{
	std::vector<ShippingItem> items_brought_by_ship;
	ship.withdraw_items(game, *this, items_brought_by_ship);

	container_iterate(std::vector<ShippingItem>, items_brought_by_ship, it) {
		it->set_location(game, m_warehouse);
		it->end_shipping(game);
	}

	if (m_expedition_ready) {
		assert(m_expedition_bootstrap.get() != nullptr);

		// Only use an empty ship.
		if (ship.get_nritems() < 1) {
			// Load the ship
			std::vector<Worker*> workers;
			std::vector<WareInstance*> wares;
			m_expedition_bootstrap->get_waiting_workers_and_wares(game, owner().tribe(), &workers, &wares);

			BOOST_FOREACH(Worker* worker, workers) {
				ship.add_item(game, ShippingItem(*worker));
			}
			BOOST_FOREACH(WareInstance* ware, wares) {
				ship.add_item(game, ShippingItem(*ware));
			}

			ship.start_task_expedition(game);

			// The expedition goods are now on the ship, so from now on it is independent from the port
			// and thus we switch the port to normal, so we could even start a new expedition,
			cancel_expedition(game);
			if (upcast(Interactive_GameBase, igb, game.get_ibase()))
				ship.refresh_window(*igb);
			return m_fleet->update(game);
		}
	}

	if (ship.get_nritems() < ship.get_capacity() && !m_waiting.empty()) {
		uint32_t nrload = std::min<uint32_t>(m_waiting.size(), ship.get_capacity() - ship.get_nritems());

		while (nrload--) {
			// Check if the item has still a valid destination
			if (m_waiting.back().get_destination(game)) {
				// Destination is valid, so we load the item onto the ship
				ship.add_item(game, m_waiting.back());
			} else {
				// The item has no valid destination anymore, so we just carry it
				// back in the warehouse
				m_waiting.back().set_location(game, m_warehouse);
				m_waiting.back().end_shipping(game);
			}
			m_waiting.pop_back();
		}

		if (m_waiting.empty())
			set_need_ship(game, false);
	}

	m_fleet->update(game);
}

void PortDock::set_need_ship(Game & game, bool need)
{
	molog("set_need_ship(%s)\n", need ? "true" : "false");

	if (need == m_need_ship)
		return;

	m_need_ship = need;

	if (m_fleet) {
		molog("... trigger fleet update\n");
		m_fleet->update(game);
	}
}

/**
 * Return the number of wares or workers of the given type that are waiting at the dock.
 */
uint32_t PortDock::count_waiting(WareWorker waretype, Ware_Index wareindex)
{
	uint32_t count = 0;

	container_iterate(std::vector<ShippingItem>, m_waiting, it) {
		WareInstance * ware;
		Worker * worker;
		it.current->get(owner().egbase(), &ware, &worker);

		if (waretype == wwWORKER) {
			if (worker && worker->worker_index() == wareindex)
				count++;
		} else {
			if (ware && ware->descr_index() == wareindex)
				count++;
		}
	}

	return count;
}


/// \returns whether an expedition was started or is even ready
bool PortDock::expedition_started() {
	return (m_expedition_bootstrap.get() != nullptr) || m_expedition_ready;
}

/// Start an expedition
void PortDock::start_expedition() {
	assert(!m_expedition_bootstrap);
	m_expedition_bootstrap.reset(new ExpeditionBootstrap(this));
	m_expedition_bootstrap->start();

}

ExpeditionBootstrap* PortDock::expedition_bootstrap() {
	return m_expedition_bootstrap.get();
}

void PortDock::expedition_bootstrap_complete(Game & game) {
	m_expedition_ready = true;
	get_fleet()->update(game);
}

void PortDock::cancel_expedition(Game & game) {
	// Reset
	m_expedition_ready = false;

	m_expedition_bootstrap->cancel(game);
	m_expedition_bootstrap.reset(nullptr);
}


void PortDock::log_general_info(const Editor_Game_Base & egbase)
{
	PlayerImmovable::log_general_info(egbase);

	Coords pos(m_warehouse->get_position());
	molog
		("PortDock for warehouse %u (at %i,%i) in fleet %u, need_ship: %s, waiting: %" PRIuS "\n",
		 m_warehouse ? m_warehouse->serial() : 0, pos.x, pos.y,
		 m_fleet ? m_fleet->serial() : 0,
		 m_need_ship ? "true" : "false",
		 m_waiting.size());

	container_iterate(std::vector<ShippingItem>, m_waiting, it) {
		molog
			("  IT %u, destination %u\n",
			 it.current->m_object.serial(),
			 it.current->m_destination_dock.serial());
	}
}

#define PORTDOCK_SAVEGAME_VERSION 3

PortDock::Loader::Loader() : m_warehouse(0)
{
}

void PortDock::Loader::load(FileRead & fr, uint8_t version)
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

	if (version >= 2) {
		pd.m_need_ship = fr.Unsigned8();

		m_waiting.resize(fr.Unsigned32());
		container_iterate(std::vector<ShippingItem::Loader>, m_waiting, it) {
			it->load(fr);
		}

		if (version >= 3) {
			// All the other expedition specific stuff is saved in the warehouse.
			if (fr.Unsigned8()) {  // Do we have an expedition?
				pd.m_expedition_bootstrap.reset(new ExpeditionBootstrap(&pd));
			}
			pd.m_expedition_ready = (fr.Unsigned8() == 1) ? true : false;
		} else {
			pd.m_expedition_ready = false;
		}
	}
}

void PortDock::Loader::load_pointers()
{
	PlayerImmovable::Loader::load_pointers();

	PortDock & pd = get<PortDock>();
	pd.m_warehouse = &mol().get<Warehouse>(m_warehouse);

	pd.m_waiting.resize(m_waiting.size());
	for (uint32_t i = 0; i < m_waiting.size(); ++i) {
		pd.m_waiting[i] = m_waiting[i].get(mol());
	}
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
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (1 <= version && version <= PORTDOCK_SAVEGAME_VERSION) {
			loader->init(egbase, mol, *new PortDock(nullptr));
			loader->load(fr, version);
		} else
			throw game_data_error("unknown/unhandled version %u", version);
	} catch (const std::exception & e) {
		throw wexception("loading portdock: %s", e.what());
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

	fw.Unsigned8(m_need_ship);

	fw.Unsigned32(m_waiting.size());
	container_iterate(std::vector<ShippingItem>, m_waiting, it) {
		it->save(egbase, mos, fw);
	}

	// Expedition specific stuff
	fw.Unsigned8(m_expedition_bootstrap.get() != nullptr ? 1 : 0);
	fw.Unsigned8(m_expedition_ready ? 1 : 0);
}

} // namespace Widelands
