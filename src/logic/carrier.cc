/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "carrier.h"
#include "logic/game.h"
#include "wexception.h"

#include "economy/road.h"
#include "economy/ware_instance.h"
#include "economy/flag.h"

#include "upcast.h"

namespace Widelands {

/**
 * Signal "road" on road split.
 * Signal "ware" when a ware has arrived.
 */
Bob::Task Carrier::taskRoad = {
	"road",
	static_cast<Bob::Ptr>(&Carrier::road_update),
	0,
	0
};


/**
 * Work on the given road, assume the location is correct.
*/
void Carrier::start_task_road(Game & game)
{
	push_task(game, taskRoad);

	top_state().ivar1 = 0;

	m_acked_ware = -1;
}


/**
 * Called by Road code when the road is split.
*/
void Carrier::update_task_road(Game & game)
{
	send_signal(game, "road");
}


void Carrier::road_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal == "road" || signal == "ware") {
		// The road changed under us or we're supposed to pick up some ware
		signal_handled();
	} else if (signal.size()) {
		// Something else happened (probably a location signal)
		molog("[road]: Terminated by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	Road & road = dynamic_cast<Road &>(*get_location(game));

	// Check for pending items
	if (m_acked_ware < 0)
		find_pending_item(game);

	if (m_acked_ware >= 0) {
		if (state.ivar1) {
			state.ivar1 = 0;
			return start_task_transport(game, m_acked_ware);
		} else {
			// Short delay before we move to pick up
			state.ivar1 = 1;

			set_animation(game, descr().get_animation("idle"));
			return schedule_act(game, 50);
		}
	}

	// Move into idle position if necessary
	if
		(start_task_movepath
		 	(game,
		 	 road.get_path(),
		 	 road.get_idle_index(),
		 	 descr().get_right_walk_anims(does_carry_ware())))
		return;

	// Be bored. There's nothing good on TV, either.
	// TODO: idle animations
	set_animation(game, descr().get_animation("idle"));
	state.ivar1 = 1; //  we are available immediately after an idle phase
	return skip_act(); //  wait until signal
}


/**
 * Fetch an item from a flag, drop it on the other flag.
 * ivar1 is the flag we fetch from, or -1 when we're in the target building.
 *
 * Signal "update" when the road has been split etc.
 */
Bob::Task Carrier::taskTransport = {
	"transport",
	static_cast<Bob::Ptr>(&Carrier::transport_update),
	0,
	0
};


/**
 * Begin the transport task.
 */
void Carrier::start_task_transport(Game & game, int32_t const fromflag)
{
	push_task(game, taskTransport);
	top_state().ivar1 = fromflag;
}


void Carrier::transport_update(Game & game, State & state)
{
	std::string signal = get_signal();

	if (signal == "road") {
		signal_handled();
	} else if (signal.size()) {
		molog("[transport]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	Road & road = dynamic_cast<Road &>(*get_location(game));

	if (state.ivar1 == -1)
		// If we're "in" the target building, special code applies
		deliver_to_building(game, state);

	else if (!does_carry_ware())
		// If we don't carry something, walk to the flag
		pickup_from_flag(game, state);

	else {
		// If the item should go to the building attached to our flag, walk
		// directly into said building
		Flag & flag = road.get_flag(static_cast<Road::FlagId>(state.ivar1 ^ 1));

		WareInstance & item = *get_carried_item(game);
		assert(item.get_location(game) == this);

		// A sanity check is necessary, in case the building has been destroyed
		PlayerImmovable * const next = item.get_next_move_step(game);

		if (next && next != &flag && &next->base_flag() == &flag)
			enter_building(game, state);

		// If the flag is overloaded we are allowed to drop items as
		// long as we can pick another up. Otherwise we have to wait.
		else if
			((flag.has_capacity() || !swap_or_wait(game, state))
			 &&
			 !start_task_walktoflag(game, state.ivar1 ^ 1))
			// Drop the item, possible exchanging it with another one
			drop_item(game, state);
	}
}


/**
 * Deliver all wares addressed to the building the carrier is already into
 *
 * \param state UNDOCUMENTED
 *
 * \todo Upgrade this function to really support many-wares-at-a-time
 * \todo Document parameter state
 */
void Carrier::deliver_to_building(Game & game, State & state)
{
	BaseImmovable * const pos = game.map()[get_position()].get_immovable();

	if (dynamic_cast<Flag const *>(pos))
		return pop_task(game); //  we are done
	else if (upcast(Building, building, pos)) {
		// Drop all items addresed to this building
		while (WareInstance * const item = get_carried_item(game)) {
			// If the building has disappeared and immediately been replaced
			// with another building, we might have to return without dropping
			// the item.
			PlayerImmovable const * const next = item->get_next_move_step(game);

			if (next == pos) {
				fetch_carried_item(game);
				item->set_location(game, building);
				item->update      (game);
			} else {
				molog
					("[Carrier]: Building switch from under us, return to road.\n");

				state.ivar1 =
					&building->base_flag()
					==
					&dynamic_cast<Road &>(*get_location(game)).get_flag
						(static_cast<Road::FlagId>(0));
				break;
			}
		}

		// No more deliverable items. Walk out to the flag.
		return
			start_task_move
				(game,
				 WALK_SE,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);
	} else {
		//  tough luck, the building has disappeared
		molog("[Carrier]: Building disappeared while in building.\n");
		set_location(0);
	}
}


/**
 * Walks to the queued flag and picks up one acked ware
 *
 * \param g Game the carrier lives on
 * \param s Flags sent to the task
 */
void Carrier::pickup_from_flag(Game & game, State & state)
{
	int32_t const ivar1 = state.ivar1;
	if (!start_task_walktoflag(game, ivar1)) {

		m_acked_ware = -1;

		Road & road = dynamic_cast<Road &>(*get_location(game));
		Flag & flag      = road.get_flag(static_cast<Road::FlagId>(ivar1));
		Flag & otherflag = road.get_flag(static_cast<Road::FlagId>(ivar1 ^ 1));

		// Are there items to move between our flags?
		if (WareInstance * const item = flag.fetch_pending_item(game, otherflag))
		{
			set_carried_item(game, item);

			set_animation(game, descr().get_animation("idle"));
			return schedule_act(game, 20);
		} else {
			molog("[Carrier]: Nothing suitable on flag.\n");
			return pop_task(game);
		}
	}
}


/**
 * Drop one item in a flag, and pick up a new one if we acked it
 *
 * \param g Game the carrier lives on.
 * \param s Flags sent to the task
 */
void Carrier::drop_item(Game & game, State & state)
{
	WareInstance * other = 0;
	Road & road = dynamic_cast<Road &>(*get_location(game));
	Flag & flag = road.get_flag(static_cast<Road::FlagId>(state.ivar1 ^ 1));

	if (m_acked_ware == (state.ivar1 ^ 1)) {
		// If there's an item we acked, we can drop ours even if the flag is
		// flooded
		other =
			flag.fetch_pending_item
				(game, road.get_flag(static_cast<Road::FlagId>(state.ivar1)));

		if (!other && !flag.has_capacity()) {
			molog
				("[Carrier]: strange: acked ware from busy flag no longer "
				 "present.\n");

			m_acked_ware = -1;
			set_animation(game, descr().get_animation("idle"));
			return schedule_act(game, 20);
		}

		state.ivar1 = m_acked_ware;
		m_acked_ware = -1;
	}

	// Drop our item
	flag.add_item(game, *fetch_carried_item(game));

	// Pick up new load, if any
	if (other) {
		set_carried_item(game, other);

		set_animation(game, descr().get_animation("idle"));
		return schedule_act(game, 20);
	} else
		return pop_task(game);
}


/**
 * When picking up items, if some of them is targeted to the building attached
 * to target flag walk straight into it and deliver.
 *
 * \param g Game the carrier lives on.
 * \param s Flags sent to the task.
 */
void Carrier::enter_building(Game & game, State & state)
{
	if (!start_task_walktoflag(game, state.ivar1 ^ 1)) {
		state.ivar1 = -1;
		return
			start_task_move
				(game,
				 WALK_NW,
				 &descr().get_right_walk_anims(does_carry_ware()),
				 true);
	}
}


/**
 * Swaps items from an overloaded flag for as long as the carrier can pick
 * up new items from it. Otherwise, changes the carrier state to wait.
 *
 * \param g Game the carrier lives on.
 * \param s Flags sent to the task.
 *
 * \return true if the carrier must wait before delivering his wares.
 */
bool Carrier::swap_or_wait(Game & game, State & state)
{
	// Road that employs us
	Road & road = dynamic_cast<Road &>(*get_location(game));
	// Flag we are delivering to
	Flag & flag = road.get_flag(static_cast<Road::FlagId>(state.ivar1 ^ 1));
	// The other flag of our road
	Flag & otherflag = road.get_flag(static_cast<Road::FlagId>(state.ivar1));


	if (m_acked_ware == (state.ivar1 ^ 1)) {
		// All is well, we already acked an item that we can pick up
		// from this flag
		return false;
	} else if (flag.has_pending_item(game, otherflag)) {
		if (!flag.ack_pending_item(game, otherflag))
			throw wexception
				("MO(%u): transport: overload exchange: flag %u is fucked up",
				 serial(), flag.serial());

		m_acked_ware = state.ivar1 ^ 1;
		return false;
	} else if (!start_task_walktoflag(game, state.ivar1 ^ 1, true))
		start_task_waitforcapacity(game, flag); //  wait one node away

	return true;
}


/**
 * Called by Road code to indicate that a new item has arrived on a flag
 * (0 = start, 1 = end).
 * \return true if the carrier is going to fetch it.
 */
bool Carrier::notify_ware(Game & game, int32_t const flag)
{
	State & state = top_state();

	// Check if we've already acked something
	if (m_acked_ware >= 0)
		return false;

	// If we are currently in a transport.
	// Explanation:
	//  a) a different carrier / road may be better suited for this ware
	//     (the transport code does not have priorities for the actual
	//     carrier that is notified)
	//  b) the transport task has logic that allows it to
	//     drop an item on an overloaded flag iff it can pick up an item
	//     at the same time.
	//     We should ack said item to avoid more confusion before we move
	//     onto the flag, but we can't do that if we have already acked
	//     something.
	//  c) we might ack for a flag that we are actually moving away from;
	//     this will get us into trouble if items have arrived on the other
	//     flag while we couldn't ack them.
	//
	// (Maybe the need for this lengthy explanation is proof that the
	// ack system needs to be reworked.)
	if (State const * const transport = get_state(taskTransport))
		if
			((transport->ivar1 == -1 && find_closest_flag(game) != flag) ||
			 flag == transport->ivar1)
			return false;

	// Ack it if we haven't
	m_acked_ware = flag;

	if      (state.task == &taskRoad)
		send_signal(game, "ware");
	else if (state.task == &taskWaitforcapacity)
		send_signal(game, "wakeup");

	return true;
}


/**
 * Find a pending item on one of the road's flags, ack it and set m_acked_ware
 * accordingly.
 */
void Carrier::find_pending_item(Game & game)
{
	Road & road = dynamic_cast<Road &>(*get_location(game));
	uint32_t haveitembits = 0;

	assert(m_acked_ware < 0);

	if
		(road.get_flag(Road::FlagStart).has_pending_item
		 	(game, road.get_flag(Road::FlagEnd)))
		haveitembits |= 1;

	if
		(road.get_flag(Road::FlagEnd).has_pending_item
		 	(game, road.get_flag(Road::FlagStart)))
		haveitembits |= 2;

	//  If both flags have an item, we pick the one closer to us.
	if (haveitembits == 3)
		haveitembits = 1 << find_closest_flag(game);

	// Ack our decision
	if (haveitembits == 1) {
		m_acked_ware = 0;
		if
			(!
			 road.get_flag(Road::FlagStart).ack_pending_item
			 	(game, road.get_flag(Road::FlagEnd)))
			throw wexception
				("Carrier::find_pending_item: start flag is messed up");

	} else if (haveitembits == 2) {
		m_acked_ware = 1;
		if
			(!
			 road.get_flag(Road::FlagEnd).ack_pending_item
			 	(game, road.get_flag(Road::FlagStart)))
			throw wexception("Carrier::find_pending_item: end flag is messed up");
	}
}


/**
 * Find the flag we are closest to (in walking time).
 */
int32_t Carrier::find_closest_flag(Game & game)
{
	Map & map = game.map();
	CoordPath startpath
		(map, dynamic_cast<Road &>(*get_location(game)).get_path());

	CoordPath endpath;
	int32_t startcost, endcost;
	int32_t curidx = -1;

	curidx = startpath.get_index(get_position());

	// Apparently, we're in a building
	if (curidx < 0) {
		Coords pos = get_position();

		map.get_brn(pos, &pos);

		if (pos == startpath.get_start())
			curidx = 0;
		else if (pos == startpath.get_end())
			curidx = startpath.get_nsteps();
		else
			throw wexception
				("MO(%u): Carrier::find_closest_flag: not on road, not on "
				 "building",
				 serial());
	}

	// Calculate the paths and their associated costs
	endpath = startpath;

	startpath.truncate(curidx);
	startpath.reverse();

	endpath.starttrim(curidx);

	map.calc_cost(startpath, &startcost, 0);
	map.calc_cost(endpath,   &endcost,   0);

	return endcost < startcost;
}


/**
 * Walk to the given flag, or one field before it if offset is true.
 *
 * \return true if a move task has been started, or false if we're already on
 * the target field.
 */
bool Carrier::start_task_walktoflag
	(Game & game, int32_t const flag, bool const offset)
{
	Path const & path = dynamic_cast<Road &>(*get_location(game)).get_path();
	int32_t idx;

	if (!flag) {
		idx = 0;
		if (offset)
			++idx;
	} else {
		idx = path.get_nsteps();
		if (offset)
			--idx;
	}

	return
		start_task_movepath
			(game, path, idx, descr().get_right_walk_anims(does_carry_ware()));
}

};
