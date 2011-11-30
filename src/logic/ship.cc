/*
 * Copyright (C) 2010-2011 by the Widelands Development Team
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

#include "ship.h"

#include "economy/fleet.h"
#include "economy/portdock.h"
#include "findbob.h"
#include "game.h"
#include "game_data_error.h"
#include "map.h"
#include "mapastar.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "path.h"
#include "tribe.h"
#include "warehouse.h"

namespace Widelands {

Ship_Descr::Ship_Descr
	(const char * given_name, const char * descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Widelands::Tribe_Descr & tribe)
: Descr(given_name, descname, directory, prof, global_s, &tribe)
{
	m_sail_anims.parse
		(*this,
		 directory,
		 prof,
		 (name() + "_sail_??").c_str(),
		 prof.get_section("sail"));

	m_capacity = global_s.get_natural("capacity", 20);
}

uint32_t Ship_Descr::movecaps() const throw ()
{
	return MOVECAPS_SWIM;
}

Bob & Ship_Descr::create_object() const
{
	return *new Ship(*this);
}


Ship::Ship(const Ship_Descr & descr) :
	Bob(descr),
	m_window(0),
	m_fleet(0),
	m_economy(0)
{
}

Ship::~Ship()
{
	close_window();
}

Bob::Type Ship::get_bob_type() const throw ()
{
	return SHIP;
}

PortDock * Ship::get_destination(Editor_Game_Base & egbase)
{
	return m_destination.get(egbase);
}

void Ship::init_auto_task(Game & game)
{
	start_task_ship(game);
}

void Ship::init(Editor_Game_Base & egbase)
{
	Bob::init(egbase);

	init_fleet(egbase);
}

/**
 * Create the initial singleton @ref Fleet to which we belong.
 * The fleet code will take automatically merge us into a larger
 * fleet, if one is reachable.
 */
void Ship::init_fleet(Editor_Game_Base & egbase)
{
	assert(get_owner() != 0);
	Fleet * fleet = new Fleet(*get_owner());
	fleet->add_ship(this);
	fleet->init(egbase);
	// fleet calls the set_fleet function appropriately
}

void Ship::cleanup(Editor_Game_Base & egbase)
{
	if (m_fleet)
		m_fleet->remove_ship(egbase, this);

	while (!m_items.empty()) {
		m_items.back().remove(egbase);
		m_items.pop_back();
	}

	Bob::cleanup(egbase);
}

/**
 * This function is to be called only by @ref Fleet.
 */
void Ship::set_fleet(Fleet * fleet)
{
	m_fleet = fleet;
}

void Ship::wakeup_neighbours(Game & game)
{
	FCoords position = get_position();
	Area<FCoords> area(position, 1);
	std::vector<Bob *> ships;
	game.map().find_bobs(area, &ships, FindBobShip());

	for
		(std::vector<Bob *>::const_iterator it = ships.begin();
		 it != ships.end(); ++it)
	{
		if (*it == this)
			continue;

		static_cast<Ship *>(*it)->ship_wakeup(game);
	}
}


/**
 * Standard behaviour of ships.
 *
 * ivar1 = helper flag for coordination of mutual evasion of ships
 */
const Bob::Task Ship::taskShip = {
	"ship",
	static_cast<Bob::Ptr>(&Ship::ship_update),
	0,
	0,
	true // unique task
};

void Ship::start_task_ship(Game & game)
{
	push_task(game, taskShip);
	top_state().ivar1 = 0;
}

void Ship::ship_wakeup(Game & game)
{
	if (get_state(taskShip))
		send_signal(game, "wakeup");
}

void Ship::ship_update(Game & game, Bob::State & state)
{
	// Handle signals
	std::string signal = get_signal();
	if (!signal.empty()) {
		if (signal == "wakeup") {
			signal_handled();
		} else {
			send_signal(game, "fail");
			pop_task(game);
			return;
		}
	}

	Map & map = game.map();

	if (PortDock * dst = get_destination(game)) {
		FCoords position = map.get_fcoords(get_position());
		if (position.field->get_immovable() == dst) {
			molog("ship_update: Arrived at dock %u\n", dst->serial());
			m_lastdock = dst;
			m_destination = 0;
			dst->ship_arrived(game, *this);
			start_task_idle(game, descr().main_animation(), 250);
			return;
		}

		molog("ship_update: Go to dock %u\n", dst->serial());

		PortDock * lastdock = m_lastdock.get(game);
		if (lastdock && lastdock != dst) {
			molog("ship_update: Have lastdock %u\n", lastdock->serial());

			Path path;
			if (m_fleet->get_path(*lastdock, *dst, path)) {
				uint32_t closest_idx = std::numeric_limits<uint32_t>::max();
				uint32_t closest_dist = std::numeric_limits<uint32_t>::max();
				Coords closest_target(Coords::Null());

				Coords cur(path.get_start());
				for (uint32_t idx = 0; idx <= path.get_nsteps(); ++idx) {
					uint32_t dist = map.calc_distance(get_position(), cur);

					if (dist == 0) {
						molog("Follow pre-computed path from (%i,%i)  [idx = %u]\n", cur.x, cur.y, idx);

						Path subpath(cur);
						while (idx < path.get_nsteps()) {
							subpath.append(map, path[idx]);
							idx++;
						}

						start_task_movepath(game, subpath, descr().get_sail_anims());
						return;
					}

					if (dist < closest_dist) {
						closest_dist = dist;
						closest_idx = idx;
					}

					if (idx == closest_idx + closest_dist)
						closest_target = cur;

					if (idx < path.get_nsteps())
						map.get_neighbour(cur, path[idx], &cur);
				}

				if (closest_target) {
					molog("Closest target en route is (%i,%i)\n", closest_target.x, closest_target.y);
					if (start_task_movepath(game, closest_target, 0, descr().get_sail_anims()))
						return;

					molog("  Failed to find path!!! Retry full search\n");
				}
			}

			m_lastdock = 0;
		}

		start_task_movetodock(game, *dst);
		return;
	}

	ship_update_idle(game, state);
}

void Ship::ship_update_idle(Game & game, Bob::State & state)
{
	if (state.ivar1) {
		// We've just completed one step, so give neighbours
		// a chance to move away first
		wakeup_neighbours(game);
		state.ivar1 = 0;
		schedule_act(game, 25);
		return;
	}

	// Check if we should move away from ships and shores
	FCoords position = get_position();
	Map & map = game.map();
	unsigned int dirs[LAST_DIRECTION + 1];
	unsigned int dirmax = 0;

	for (Direction dir = 0; dir <= LAST_DIRECTION; ++dir) {
		FCoords node = dir ? map.get_neighbour(position, dir) : position;
		dirs[dir] = node.field->nodecaps() & MOVECAPS_WALK ? 10 : 0;

		Area<FCoords> area(node, 0);
		std::vector<Bob *> ships;
		game.map().find_bobs(area, &ships, FindBobShip());

		for (std::vector<Bob *>::const_iterator it = ships.begin(); it != ships.end(); ++it) {
			if (*it == this)
				continue;

			dirs[dir] += 3;
		}

		dirmax = std::max(dirmax, dirs[dir]);
	}

	if (dirmax) {
		unsigned int prob[LAST_DIRECTION + 1];
		unsigned int totalprob = 0;

		// The probability for moving into a given direction is also
		// affected by the "close" directions.
		for (Direction dir = 0; dir <= LAST_DIRECTION; ++dir) {
			prob[dir] = 10 * dirmax - 10 * dirs[dir];

			if (dir > 0) {
				unsigned int delta = std::min(prob[dir], dirs[(dir % 6) + 1] + dirs[1 + ((dir - 1) % 6)]);
				prob[dir] -= delta;
			}

			totalprob += prob[dir];
		}

		unsigned int rnd = game.logic_rand() % totalprob;
		Direction dir = 0;
		while (rnd >= prob[dir]) {
			rnd -= prob[dir];
			++dir;
		}

		if (dir == 0 || dir > LAST_DIRECTION) {
			start_task_idle(game, descr().main_animation(), 1500);
			return;
		}

		FCoords neighbour = map.get_neighbour(position, dir);
		if (!(neighbour.field->nodecaps() & MOVECAPS_SWIM)) {
			start_task_idle(game, descr().main_animation(), 1500);
			return;
		}

		state.ivar1 = 1;
		start_task_move(game, dir, &descr().get_sail_anims(), false);
		return;
	}

	// No desire to move around, so sleep
	start_task_idle(game, descr().main_animation(), -1);
}

void Ship::set_economy(Game & game, Economy * e)
{
	m_economy = e;
	container_iterate(std::vector<ShippingItem>, m_items, it) {
		it->set_economy(game, e);
	}
}

/**
 * Enter a new destination port for the ship.
 *
 * @note This is supposed to be called only from the scheduling code of @ref Fleet.
 */
void Ship::set_destination(Game & game, PortDock & pd)
{
	molog("set_destination to %u (currently %zu items)\n", pd.serial(), m_items.size());
	m_destination = &pd;
	send_signal(game, "wakeup");
}

void Ship::add_item(Game & game, const ShippingItem & item)
{
	assert(m_items.size() < get_capacity());

	m_items.push_back(item);
	m_items.back().set_location(game, this);
}

void Ship::withdraw_items(Game & game, PortDock & pd, std::vector<ShippingItem> & items)
{
	uint32_t dst = 0;
	for (uint32_t src = 0; src < m_items.size(); ++src) {
		PortDock * destination = m_items[src].get_destination(game);
		if (!destination || destination == &pd) {
			items.push_back(m_items[src]);
		} else {
			m_items[dst++] = m_items[src];
		}
	}
	m_items.resize(dst);
}

/**
 * Find a path to the dock @p pd and follow it without using precomputed paths.
 */
void Ship::start_task_movetodock(Game & game, PortDock & pd)
{
	Map & map = game.map();
	StepEvalAStar se(pd.get_warehouse()->get_position());
	se.m_swim = true;
	se.m_conservative = false;
	se.m_estimator_bias = -5 * map.calc_cost(0);

	MapAStar<StepEvalAStar> astar(map, se);

	astar.push(get_position());

	int32_t cost;
	FCoords cur;
	while (astar.step(cur, cost)) {
		if (cur.field->get_immovable() == &pd) {
			Path path;
			astar.pathto(cur, path);
			start_task_movepath(game, path, descr().get_sail_anims());
			return;
		}
	}

	molog("start_task_movedock: Failed to find path!\n");
	start_task_idle(game, descr().main_animation(), 5000);
}

void Ship::log_general_info(const Editor_Game_Base & egbase)
{
	Bob::log_general_info(egbase);

	molog
		("Fleet: %u, destination: %u, lastdock: %u, carrying: %zu\n",
		 m_fleet? m_fleet->serial() : 0,
		 m_destination.serial(), m_lastdock.serial(),
		 m_items.size());

	container_iterate(std::vector<ShippingItem>, m_items, it) {
		molog
			("  IT %u, destination %u\n",
			 it.current->m_object.serial(),
			 it.current->m_destination_dock.serial());
	}
}


/*
==============================

Load / Save implementation

==============================
*/

#define SHIP_SAVEGAME_VERSION 2

Ship::Loader::Loader() :
	m_lastdock(0),
	m_destination(0)
{
}

const Bob::Task * Ship::Loader::get_task(const std::string & name)
{
	if (name == "shipidle" || name == "ship") return &taskShip;
	return Bob::Loader::get_task(name);
}

void Ship::Loader::load(FileRead & fr, uint8_t version)
{
	Bob::Loader::load(fr);

	if (version >= 2) {
		m_lastdock = fr.Unsigned32();
		m_destination = fr.Unsigned32();

		m_items.resize(fr.Unsigned32());
		container_iterate(std::vector<ShippingItem::Loader>, m_items, it) {
			it->load(fr);
		}
	}
}

void Ship::Loader::load_pointers()
{
	Bob::Loader::load_pointers();

	Ship & ship = get<Ship>();

	if (m_lastdock)
		ship.m_lastdock = &mol().get<PortDock>(m_lastdock);
	if (m_destination)
		ship.m_destination = &mol().get<PortDock>(m_destination);

	ship.m_items.resize(m_items.size());
	for (uint32_t i = 0; i < m_items.size(); ++i) {
		ship.m_items[i] = m_items[i].get(mol());
	}
}

void Ship::Loader::load_finish()
{
	Bob::Loader::load_finish();

	Ship & ship = get<Ship>();
	// For robustness, in case our fleet did not get restored from the savegame
	// for whatever reason
	if (!ship.m_fleet)
		ship.init_fleet(egbase());
}


Map_Object::Loader * Ship::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::auto_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (1 <= version && version <= SHIP_SAVEGAME_VERSION) {
			std::string owner = fr.CString();
			std::string name = fr.CString();
			const Ship_Descr * descr = 0;

			egbase.manually_load_tribe(owner);

			if (const Tribe_Descr * tribe = egbase.get_tribe(owner))
				descr = dynamic_cast<const Ship_Descr *>
					(tribe->get_bob_descr(name));

			if (!descr)
				throw game_data_error
					("undefined ship %s/%s", owner.c_str(), name.c_str());

			loader->init(egbase, mol, descr->create_object());
			loader->load(fr, version);
		} else
			throw game_data_error(_("unknown/unhandled version %u"), version);
	} catch (const std::exception & e) {
		throw wexception(_("loading ship: %s"), e.what());
	}

	return loader.release();
}

void Ship::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Ship);
	fw.Unsigned8(SHIP_SAVEGAME_VERSION);

	fw.CString(descr().get_owner_tribe()->name());
	fw.CString(descr().name());

	Bob::save(egbase, mos, fw);

	fw.Unsigned32(mos.get_object_file_index_or_zero(m_lastdock.get(egbase)));
	fw.Unsigned32(mos.get_object_file_index_or_zero(m_destination.get(egbase)));

	fw.Unsigned32(m_items.size());
	container_iterate(std::vector<ShippingItem>, m_items, it) {
		it->save(egbase, mos, fw);
	}
}

} // namespace Widelands
