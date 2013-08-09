/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "logic/warehouse.h"

#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "container_iterate.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/ware_instance.h"
#include "economy/wares_queue.h"
#include "log.h"
#include "logic/battle.h"
#include "logic/carrier.h"
#include "logic/editor_game_base.h"
#include "logic/findbob.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/headquarters.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/requirements.h"
#include "logic/soldier.h"
#include "logic/storagehandler.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "profile/profile.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

/// Warehouse Descr
Warehouse_Descr::Warehouse_Descr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Tribe_Descr & _tribe)
:  Building_Descr(_name, _descname, directory, prof, global_s, _tribe)
{
	Section* garrison_s = prof.get_section("garrison");
	if (garrison_s) {
		m_has_garrison = true;
		m_conquers = garrison_s->get_safe_int("conquers");
		m_maxsoldiers = garrison_s->get_safe_int("max_soldiers");
		m_heal_per_second = garrison_s->get_safe_int("heal_per_second");
		if (m_conquers > 0)
			m_workarea_info[m_conquers].insert(descname() + _(" conquer"));
		m_preferheroes = garrison_s->get_safe_bool("prefer_heroes");
	} else {
		m_heal_per_second = global_s.get_safe_int("heal_per_second");
		m_has_garrison = false;
		m_conquers = global_s.get_int("conquers");
	}
}

Building& Warehouse_Descr::create_object() const
{
	if (!m_has_garrison) {
		return *new Warehouse(*this);
	} else {
		return *new Headquarters(*this);
	}
}


/*
==============================
IMPLEMENTATION
==============================
*/

Warehouse::Warehouse(const Warehouse_Descr & warehouse_descr) :
	Building(warehouse_descr),
	m_portdock(0)
{
	m_storage.reset(new StorageHandler(*this));
}


Warehouse::~Warehouse()
{
	for (uint8_t i = 0; i < m_expedition_wares.size(); ++i) {
		if (m_expedition_wares.at(i)) {
			m_expedition_wares.at(i)->cleanup();
			delete m_expedition_wares.at(i);
		}
	}
}

void Warehouse::load_finish(Editor_Game_Base & egbase) {
	Building::load_finish(egbase);
	m_storage->load_finish(egbase);
}

void Warehouse::init(Editor_Game_Base & egbase)
{
	Building::init(egbase);
	m_storage->init(egbase);

	Player & player = owner();
	if (upcast(Game, game, &egbase)) {
		// Even though technically, a warehouse might be completely empty,
		// we let warehouse see always for simplicity's sake (since there's
		// almost always going to be a carrier inside, that shouldn't hurt).
		player.see_area
			(Area<FCoords>
			 (egbase.map().get_fcoords(get_position()), vision_range()));

		schedule_act(*game, 1000);
		// Send a message
		std::string message =
			(boost::format(_("A new %s was added to your economy.")) % descname().c_str())
			.str();
		send_message
			(ref_cast<Game, Editor_Game_Base>(egbase),
			"warehouse", descname(), message);
	}

	// Conquer area
	if (uint32_t const conquer_radius = get_conquers()) {
		egbase.conquer_area
			(Player_Area<Area<FCoords> >
			 	(player.player_number(),
			 	 Area<FCoords>
			 	 	(egbase.map().get_fcoords(get_position()), conquer_radius)));
	}

	// Init port stuff
	// FIXME: move in a port class
	if (descr().get_isport())
		init_portdock(egbase);
}

/**
 * Find a contiguous set of water fields close to the port for docking
 * and initialize the @ref PortDock instance.
 */
void Warehouse::init_portdock(Editor_Game_Base & egbase)
{
	molog("Setting up port dock fields\n");

	Map & map = egbase.map();
	std::vector<Coords> dock = map.find_portdock(get_position());
	if (dock.empty()) {
		log("Attempting to setup port without neighboring water.\n");
		return;
	}

	molog("Found %" PRIuS " fields for the dock\n", dock.size());

	m_portdock = new PortDock;
	m_portdock->set_owner(get_owner());
	m_portdock->set_warehouse(this);
	m_portdock->set_economy(get_economy());
	container_iterate_const(std::vector<Coords>, dock, it) {
		m_portdock->add_position(*it.current);
	}
	m_portdock->init(egbase);

	if (get_economy() != nullptr) {
		m_portdock->set_economy(get_economy());
		m_storage->set_economy(get_economy());
	}
}


/// PortDock's expedition waresqueue
WaresQueue & Warehouse::waresqueue(Ware_Index const wi) {
	container_iterate_const(std::vector<WaresQueue *>, m_expedition_wares, i)
		if ((*i.current)->get_ware() == wi)
			return **i.current;
	throw wexception
		("%s (%u) (building %s) has no WaresQueue for %u",
		 name().c_str(), serial(), name().c_str(), wi.value());
}

/// keeps track of arriving expedition workers
void Warehouse::handle_expedition_worker_callback(Game & g, Request & r, Worker * w) {
	for (uint8_t i = 0; i < m_expedition_workers.size(); ++i)
		if (m_expedition_workers.at(i)->worker_request == &r) {
			m_expedition_workers.at(i)->worker_request = 0;
			m_expedition_workers.at(i)->worker = w;
			delete &r;
			// Check if this one was the last one we waited for
			return m_portdock->check_expedition_wares_and_workers(g);
		}
	assert(false);
}


void Warehouse::destroy(Editor_Game_Base & egbase)
{
	// Launch all workers in stock
	if (upcast(Game, game, &egbase)) {
		m_storage->launch_all_workers(*game);
	}

	Building::destroy(egbase);
}

void Warehouse::cleanup(Editor_Game_Base & egbase)
{
	if (m_portdock) {
		m_portdock->remove(egbase);
		m_portdock = 0;
		// Wares are deleted later on, after Building::cleanup was run
		for (uint8_t i = 0; i < m_expedition_workers.size(); ++i) {
			if (m_expedition_workers.at(i)->worker_request)
				delete m_expedition_workers.at(i)->worker_request;
			if (m_expedition_workers.at(i)->worker) {
				m_expedition_workers.at(i)->worker->cleanup(egbase);
				delete m_expedition_workers.at(i)->worker;
			}
			delete m_expedition_workers.at(i);
		}
	}

	// Launch workers with some experience
	// This happen on dismantle for instance
	if (upcast(Game, game, &egbase)) {
		m_storage->launch_all_workers(*game, true);
	}
	m_storage->cleanup(egbase);

	// Unconquer area
	Map & map = egbase.map();
	if (const uint32_t conquer_radius = get_conquers()) {
		egbase.unconquer_area
			(Player_Area<Area<FCoords> >
			 	(owner().player_number(),
			 	 Area<FCoords>(map.get_fcoords(get_position()), conquer_radius)),
			 m_defeating_player);
	}

	// Unsee the area that we started seeing in init()
	Player & player = owner();
	player.unsee_area
			(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));

	Building::cleanup(egbase);
}


/// Act regularly to create workers of buildable types without cost. According
/// to intelligence, this is some highly advanced technology. Not only do the
/// settlers have no problems with birth control, they do not even need anybody
/// to procreate. They must have built-in DNA samples in those warehouses. And
/// what the hell are they doing, killing useless tribesmen! The Borg? Or just
/// like Soylent Green? Or maybe I should just stop writing comments that late
/// at night ;-)
void Warehouse::act(Game & game, uint32_t const data)
{
	uint32_t act_time = m_storage->act(game);
	Building::act(game, data);
	schedule_act(game, act_time);
}


/// Transfer our registration to the new economy.
void Warehouse::set_economy(Economy * const e)
{
	Economy* old = get_economy();
	m_storage->set_economy(e);

	// Take care about the expeditions WaresQueues
	for (uint8_t i = 0; i < m_expedition_wares.size(); ++i) {
		if (old)
			m_expedition_wares.at(i)->remove_from_economy(*old);
		if (e)
			m_expedition_wares.at(i)->add_to_economy(*e);
	}
	Building::set_economy(e);
}

/// Launch a carrier to fetch an item from our flag.
bool Warehouse::fetch_from_flag(Game & game)
{
	Ware_Index const carrierid = tribe().safe_worker_index("carrier");

	if (!m_storage->count_workers(game, carrierid)) {
		m_storage->insert_workers(carrierid, 1);
	}
	Worker& w = m_storage->launch_worker(game, carrierid);
	w.start_task_fetchfromflag(game);
	return true;
}


void Warehouse::log_general_info(const Editor_Game_Base & egbase)
{
	Building::log_general_info(egbase);

	if (descr().get_isport())
		molog("Port dock: %u\n", m_portdock ? m_portdock->serial() : 0);
}

void Warehouse::receive_ware(Game&, Ware_Index ware)
{
	m_storage->insert_wares(ware, 1);
}

void Warehouse::receive_worker(Game& game, Worker& worker)
{
	m_storage->incorporate_worker(game, worker);
}

Building* Warehouse::get_building()
{
	return this;
}

Storage* Warehouse::get_storage() const
{
	return m_storage.get();
}

}
