/*
 * Copyright (C) 2004, 2006-2010 by the Widelands Development Team
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

#include "road.h"

// Package includes
#include "economy.h"
#include "flag.h"

#include "logic/carrier.h"
#include "logic/instances.h"
#include "logic/player.h"
#include "request.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/tribe.h"

#include "upcast.h"

namespace Widelands {

// dummy instance because Map_Object needs a description
Map_Object_Descr g_road_descr("road", "Road");

/**
 * Most of the actual work is done in init.
*/
Road::Road() :
	PlayerImmovable  (g_road_descr),
	m_type           (0)
{
	m_flags[0] = m_flags[1] = 0;
	m_flagidx[0] = m_flagidx[1] = -1;
}

Road::CarrierSlot::CarrierSlot() :
	carrier (0),
	carrier_request(0),
	carrier_type(0)
	{}


/**
 * Most of the actual work is done in cleanup.
 */
Road::~Road()
{
	container_iterate_const(SlotVector, m_carrier_slots, i)
		delete i.current->carrier_request;
}

/**
 * Create a road between the given flags, using the given path.
*/
void Road::create
	(Editor_Game_Base & egbase,
	 Flag & start, Flag & end, Path const & path,
	 bool    const create_carrier,
	 int32_t const type)
{
	assert(start.get_position() == path.get_start());
	assert(end  .get_position() == path.get_end  ());
	assert(start.get_owner   () == end .get_owner());

	Player & owner          = start.owner();
	Road & road             = *new Road();
	road.set_owner(&owner);
	road.m_type             = type;
	road.m_flags[FlagStart] = &start;
	road.m_flags[FlagEnd]   = &end;
	// m_flagidx is set when attach_road() is called, i.e. in init()
	road._set_path(egbase, path);
	if (create_carrier) {
		Coords idle_position = start.get_position();
		{
			Map const & map = egbase.map();
			Path::Step_Vector::size_type idle_index = road.get_idle_index();
			for (Path::Step_Vector::size_type i = 0; i < idle_index; ++i)
				map.get_neighbour(idle_position, path[i], &idle_position);
		}
		Tribe_Descr const & tribe = owner.tribe();
		Carrier & carrier =
			ref_cast<Carrier, Worker>
				(tribe.get_worker_descr(tribe.worker_index("carrier"))->create
				 	(egbase, owner, &start, idle_position));
		carrier.start_task_road(ref_cast<Game, Editor_Game_Base>(egbase));

		CarrierSlot slot;
		slot.carrier = &carrier;
		slot.carrier_type = 1;
		road.m_carrier_slots.push_back(slot);
	}
	log("Road::create: &road = %p\n", &road);
	road.init(egbase);
}

int32_t Road::get_type() const throw ()
{
	return ROAD;
}

int32_t Road::get_size() const throw ()
{
	return SMALL;
}

bool Road::get_passable() const throw ()
{
	return true;
}


static std::string const road_name = "road";
std::string const & Road::name() const throw () {return road_name;}


Flag & Road::base_flag()
{
	return *m_flags[FlagStart];
}

/**
 * Return the cost of getting from fromflag to the other flag.
*/
int32_t Road::get_cost(FlagId fromflag)
{
	return m_cost[fromflag];
}

/**
 * Set the new path, calculate costs.
 * You have to set start and end flags before calling this function.
*/
void Road::_set_path(Editor_Game_Base & egbase, Path const & path)
{
	assert(path.get_nsteps() >= 2);
	assert(path.get_start() == m_flags[FlagStart]->get_position());
	assert(path.get_end() == m_flags[FlagEnd]->get_position());

	m_path = path;
	egbase.map().calc_cost(path, &m_cost[FlagStart], &m_cost[FlagEnd]);

	// Figure out where carriers should idle
	m_idle_index = path.get_nsteps() / 2;
}

/**
 * Add road markings to the map
*/
void Road::_mark_map(Editor_Game_Base & egbase)
{
	Map & map = egbase.map();
	FCoords curf = map.get_fcoords(m_path.get_start());

	const Path::Step_Vector::size_type nr_steps = m_path.get_nsteps();
	for (Path::Step_Vector::size_type steps = 0; steps <  nr_steps + 1; ++steps)
	{
		if (steps > 0 && steps < m_path.get_nsteps())
			set_position(egbase, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir  = get_reverse_dir(m_path[steps - 1]);
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, m_type);
		}

		// mark the road that leads away from this field
		if (steps < m_path.get_nsteps()) {
			const Direction dir  = m_path[steps];
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, m_type);

			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Remove road markings from the map
*/
void Road::_unmark_map(Editor_Game_Base & egbase) {
	Map & map = egbase.map();
	FCoords curf(m_path.get_start(), &map[m_path.get_start()]);

	const Path::Step_Vector::size_type nr_steps = m_path.get_nsteps();
	for (Path::Step_Vector::size_type steps = 0; steps < nr_steps + 1; ++steps)
	{
		if (steps > 0 && steps < m_path.get_nsteps())
			unset_position(egbase, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir  = get_reverse_dir(m_path[steps - 1]);
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, Road_None);
		}

		// mark the road that leads away from this field
		if (steps < m_path.get_nsteps()) {
			const Direction  dir = m_path[steps];
			const Direction rdir = 2 * (dir - Map_Object::WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, Road_None);

			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Initialize the road.
*/
void Road::init(Editor_Game_Base & egbase)
{
	PlayerImmovable::init(egbase);

	if (2 <= m_path.get_nsteps())
		_link_into_flags(ref_cast<Game, Editor_Game_Base>(egbase));
}

/**
 * This links into the flags, calls a carrier
 * and so on. This was formerly done in init (and
 * still is for normal games). But for save game loading
 * we needed to have this road already registered
 * as Map Object, thats why this is moved
 */
void Road::_link_into_flags(Game & game) {
	assert(m_path.get_nsteps() >= 2);

	/*
	 * Initialize the worker slots for the road
	 * TODO: make this configurable
	 */
	CarrierSlot slot;
	m_carrier_slots.push_back(slot);
	m_carrier_slots.push_back(slot);
	//m_carrier_slots.push_back(slot);
	m_carrier_slots[0].carrier_type = 1;
	m_carrier_slots[1].carrier_type = 2;
	//m_carrier_slots[2].carrier_type = 2;

	// Link into the flags (this will also set our economy)
	{
		const Direction dir = m_path[0];
		m_flags[FlagStart]->attach_road(dir, this);
		m_flagidx[FlagStart] = dir;
	}


	const Direction dir =
		get_reverse_dir(m_path[m_path.get_nsteps() - 1]);
	m_flags[FlagEnd]->attach_road(dir, this);
	m_flagidx[FlagEnd] = dir;

	Economy::check_merge(*m_flags[FlagStart], *m_flags[FlagEnd]);

	// Mark Fields
	_mark_map(game);

	/*
	 * Iterate over all Carrierslots
	 * If a carrier is set assign it to this road, else
	 * request a new carrier
	 */
	container_iterate(SlotVector, m_carrier_slots, i)
		if (Carrier * const carrier = i.current->carrier.get(game)) {
			//  This happens after a road split. Tell the carrier what's going on.
			carrier->set_location    (this);
			carrier->update_task_road(game);
		} else if (not i.current->carrier_request)
			_request_carrier(game, *i.current);
}

/**
 * Cleanup the road
*/
void Road::cleanup(Editor_Game_Base & egbase)
{
	Game & game = ref_cast<Game, Editor_Game_Base>(egbase);

	container_iterate(SlotVector, m_carrier_slots, i) {
		delete i.current->carrier_request;
		i.current->carrier_request = 0;

		// carrier will be released via PlayerImmovable::cleanup
		i.current->carrier = 0;
	}

	// Unmark Fields
	_unmark_map(game);

	// Unlink from flags (also clears the economy)
	m_flags[FlagStart]->detach_road(m_flagidx[FlagStart]);
	m_flags[FlagEnd]->detach_road(m_flagidx[FlagEnd]);

	Economy::check_split(*m_flags[FlagStart], *m_flags[FlagEnd]);

	m_flags[FlagStart]->update_items(game, m_flags[FlagEnd]);
	m_flags[FlagEnd]->update_items(game, m_flags[FlagStart]);

	PlayerImmovable::cleanup(game);
}

/**
 * Workers' economies are fixed by PlayerImmovable, but we need to handle
 * any requests ourselves.
*/
void Road::set_economy(Economy * const e)
{
	PlayerImmovable::set_economy(e);

	container_iterate_const(SlotVector, m_carrier_slots, i)
		if (i.current->carrier_request)
			i.current->carrier_request->set_economy(e);
}

/**
 * Request a new carrier.
 *
 * Only call this if the road can handle a new carrier, and if no request has
 * been issued.
*/
void Road::_request_carrier(Game &, CarrierSlot & slot)
{

	if (slot.carrier_type == 1 or owner().tribe().name() == "atlanteans") {
		slot.carrier_request =
			new Request
				(*this,
				 owner().tribe().safe_worker_index("carrier"),
				 Road::_request_carrier_callback,
				 Request::WORKER);
	} else {
		std::string donkey_name =
			owner().tribe().name() == "empire" ? "donkey" : "wisent";
		slot.carrier_request =
		new Request
			(*this,
			 owner().tribe().safe_worker_index(donkey_name),
			 Road::_request_carrier_callback,
			 Request::WORKER);

	}
}

/**
 * The carrier has arrived successfully.
*/
void Road::_request_carrier_callback
	(Game            &       game,
	 Request         &       rq,
	 Ware_Index,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	assert(w);

	Road    & road    = ref_cast<Road,    PlayerImmovable>(target);
	Carrier & carrier = ref_cast<Carrier, Worker>         (*w);

	container_iterate(SlotVector, road.m_carrier_slots, i)
		if (i.current->carrier_request == &rq) {
			i.current->carrier_request = 0;
			i.current->carrier = &carrier;

			carrier.start_task_road(game);
			delete &rq;
			return;
		}

	delete &rq;
	carrier.start_task_gowarehouse(game);
}

/**
 * If we lost our carrier, re-request it.
*/
void Road::remove_worker(Worker & w)
{
	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());

	container_iterate(SlotVector, m_carrier_slots, i) {
		Carrier const * carrier = i.current->carrier.get(game);

		if (carrier == &w) {
			i.current->carrier = 0;
			carrier            = 0;
		}

		if (not carrier and not i.current->carrier_request)
			_request_carrier(game, *i.current);
	}

	PlayerImmovable::remove_worker(w);
}


/**
 * A flag has been placed that splits this road. This function is called before
 * the new flag initializes. We remove markings to avoid interference with the
 * flag.
*/
void Road::presplit(Game & game, Coords) {_unmark_map(game);}

/**
 * The flag that splits this road has been initialized. Perform the actual
 * splitting.
 *
 * After the split, this road will span [start...new flag]. A new road will
 * be created to span [new flag...end]
*/
void Road::postsplit(Game & game, Flag & flag)
{

	Flag & oldend = *m_flags[FlagEnd];

	// detach from end
	oldend.detach_road(m_flagidx[FlagEnd]);

	// build our new path and the new road's path
	Map & map = game.map();
	CoordPath path(map, m_path);
	CoordPath secondpath(path);
	int32_t const index = path.get_index(flag.get_position());

	assert(index > 0);
	assert(static_cast<uint32_t>(index) < path.get_nsteps() - 1);

	path.truncate(index);
	secondpath.starttrim(index);

	molog("splitting road: first part:\n");
	container_iterate_const(std::vector<Coords>, path.get_coords(), i)
		molog("* (%i, %i)\n", i.current->x, i.current->y);
	molog("                second part:\n");
	container_iterate_const(std::vector<Coords>, secondpath.get_coords(), i)
		molog("* (%i, %i)\n", i.current->x, i.current->y);

	// change road size and reattach
	m_flags[FlagEnd] = &flag;
	_set_path(game, path);

	const Direction dir = get_reverse_dir(m_path[m_path.get_nsteps() - 1]);
	m_flags[FlagEnd]->attach_road(dir, this);
	m_flagidx[FlagEnd] = dir;

	// recreate road markings
	_mark_map(game);

	// create the new road
	Road & newroad = *new Road();
	newroad.set_owner(get_owner());
	newroad.m_type = m_type;
	newroad.m_flags[FlagStart] = &flag; //  flagidx will be set on init()
	newroad.m_flags[FlagEnd] = &oldend;
	newroad._set_path(game, secondpath);

	// Find workers on this road that need to be reassigned
	// The algorithm is pretty simplistic, and has a bias towards keeping
	// the worker around; there's obviously nothing wrong with that.


	//Carrier * const carrier = m_carrier.get(game);

	//SlotVector carrierslots = m_carrier_slots;
	std::vector<Worker *> const workers = get_workers();

	container_iterate(SlotVector, m_carrier_slots, i) {
		delete i.current->carrier_request;
		i.current->carrier_request = 0;
	}


	std::vector<Worker *> reassigned_workers;

	container_iterate_const(std::vector<Worker *>, workers, i) {
		Worker & w = **i.current;
		int32_t idx = path.get_index(w.get_position());

		// Careful! If the worker is currently inside the building at our
		// starting flag, we *must not* reassign him.
		// If he is in the building at our end flag or at the other road's
		// end flag, he can be reassigned to the other road.
		if (idx < 0) {
			if
				(dynamic_cast<Building const *>
				 	(map.get_immovable(w.get_position())))
			{
				Coords pos;
				map.get_brn(w.get_position(), &pos);
				if (pos == path.get_start())
					idx = 0;
			}
		}

		if (idx < 0) {
			reassigned_workers.push_back(&w);

			/*
			 * The current worker is not on this road. Search him
			 * in this road and remove him. Than add him to the new road
			 */
			container_iterate(SlotVector, m_carrier_slots, j) {
				Carrier const * const carrier = j.current->carrier.get(game);

				if (carrier == &w) {
					j.current->carrier = 0;
					container_iterate(SlotVector, newroad.m_carrier_slots, k)
						if
							(not k.current->carrier.get(game) and
							 not k.current->carrier_request)
						{
							k.current->carrier = &ref_cast<Carrier, Worker> (w);
							break;
						}
				}
			}
		}

		// Cause a worker update in any case
		w.send_signal(game, "road");
	}

	// Initialize the new road
	newroad.init(game);

	// Actually reassign workers after the new road has initialized,
	// so that the reassignment is safe
	container_iterate_const(std::vector<Worker *>, reassigned_workers, i)
		(*i.current)->set_location(&newroad);

	//  Request a new carrier for this road if necessary. This must be done
	//  _after_ the new road initializes, otherwise request routing might not
	//  work correctly
	container_iterate(SlotVector, m_carrier_slots, i)
		if (not i.current->carrier.get(game) and not i.current->carrier_request)
			_request_carrier(game, *i.current);

	//  Make sure items waiting on the original endpoint flags are dealt with.
	m_flags[FlagStart]->update_items(game, &oldend);
	oldend.update_items(game, m_flags[FlagStart]);
}

/**
 * Called by Flag code: an item should be picked up from the given flag.
 * \return true if a carrier has been sent on its way, false otherwise.
 */
bool Road::notify_ware(Game & game, FlagId const flagid)
{
	container_iterate(SlotVector, m_carrier_slots, i)
		if (Carrier * const carrier = i.current->carrier.get(game))
			if (carrier->notify_ware(game, flagid))
				return true;
	return false;
}

}
