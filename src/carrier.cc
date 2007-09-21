/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
#include "game.h"
#include <stdint.h>
#include "wexception.h"


/**
 * Signal "road" on road split.
 * Signal "ware" when a ware has arrived.
 */
Bob::Task Carrier::taskRoad = {
	"road",

	(Bob::Ptr)&Carrier::road_update,
	(Bob::Ptr)&Carrier::road_signal,
	0,
};


/**
 * Work on the given road, assume the location is correct.
*/
void Carrier::start_task_road()
{
	push_task(taskRoad);

	get_state()->ivar1 = 0;

	m_acked_ware = -1;
}


/**
 * Called by Road code when the road is split.
*/
void Carrier::update_task_road(Game* g)
{
	send_signal(g, "road");
}


void Carrier::road_update(Game* g, State* state)
{
	Road* road = (Road*)get_location(g);

	assert(road); // expect 'location' signal

	// Check for pending items
	if (m_acked_ware < 0)
		find_pending_item(g);

	if (m_acked_ware >= 0) {
		if (state->ivar1) {
			state->ivar1 = 0;
			start_task_transport(m_acked_ware);
		} else {
			// Short delay before we move to pick up
			molog("[road]: delay (acked for %i)\n", m_acked_ware);
			state->ivar1 = 1;

			set_animation(g, descr().get_animation("idle"));
			schedule_act(g, 50);
		}

		return;
	}

	// Move into idle position if necessary
	if (start_task_movepath(g->map(), road->get_path(), road->get_idle_index(),
	                        descr().get_right_walk_anims(does_carry_ware())))
		return;

	// Be bored. There's nothing good on TV, either.
	// TODO: idle animations
	set_animation(g, descr().get_animation("idle"));
	skip_act(); // wait until signal
	state->ivar1 = 1; // we're available immediately after an idle phase
}


void Carrier::road_signal(Game * g, State *)
{
	std::string signal = get_signal();

	if (signal == "road" || signal == "ware") {
		set_signal(""); // update() will do the rest
		schedule_act(g, 10);
		return;
	}

	molog("[road]: Terminated by signal '%s'\n", signal.c_str());
	pop_task();
}


/**
 * Fetch an item from a flag, drop it on the other flag.
 * ivar1 is the flag we fetch from, or -1 when we're in the target building.
 *
 * Signal "update" when the road has been split etc.
 */
Bob::Task Carrier::taskTransport = {
	"transport",

	(Bob::Ptr)&Carrier::transport_update,
	(Bob::Ptr)&Carrier::transport_signal,
	0,
};


/**
 * Begin the transport task.
 */
void Carrier::start_task_transport(int fromflag)
{
	State* state;

	push_task(taskTransport);

	state = get_state();
	state->ivar1 = fromflag;
}


void Carrier::transport_update(Game* g, State* state)
{
	Road* road = (Road*)get_location(g);
	assert(road); // expect 'location' signal

	if (state->ivar1 == -1)
		// If we're "in" the target building, special code applies
		deliver_to_building(g, state);

	else if (!does_carry_ware())
		// If we don't carry something, walk to the flag
		pickup_from_flag(g, state);

	else {
		// If the item should go to the building attached to our flag, walk
		// directly into said building
		Flag* flag = road->get_flag((Road::FlagId)(state->ivar1 ^ 1));

		WareInstance* item = get_carried_item(g);
		assert(item->get_location(g) == this);

		// A sanity check is necessary, in case the building has been destroyed
		PlayerImmovable* next = item->get_next_move_step(g);

		if (next && next != flag && next->get_base_flag() == flag)
			enter_building(g, state);

		// If the flag is overloaded we are allowed to drop items as
		// long as we can pick another up. Otherwise we have to wait.
		else if ((flag->has_capacity() || !swap_or_wait(g, state))
				 && !start_task_walktoflag(g, state->ivar1 ^ 1))
			// Drop the item, possible exchanging it with another one
			drop_item(g, state);
	}

	return;
}


/**
 * Deliver all wares addressed to the building the carrier is already into
 *
 * \param g Game the carrier lives on
 * \param state UNDOCUMENTED
 *
 * \todo Upgrade this function to really support many-wares-at-a-time
 * \todo Document parameter state
 */
void Carrier::deliver_to_building(Game* g, State* state)
{
	BaseImmovable* pos = g->get_map()->get_immovable(get_position());

	// tough luck, the building has disappeared
	if (!pos || (pos->get_type() != BUILDING && pos->get_type() != FLAG)) {
		molog("[Carrier]: Building disappeared while in building.\n");
		set_location(0);

	} else if (pos->get_type() == Map_Object::BUILDING) {
		molog("[Carrier]: Arrived at building.\n");

		// Drop all items addresed to this building
		while (WareInstance* item = get_carried_item(g)) {
			// If the building has disappeared and immediately been replaced
			// with another building, we might have to return without dropping
			// the item.
			PlayerImmovable* next = item->get_next_move_step(g);

			if (next == pos) {
				fetch_carried_item(g);
				item->set_location(g, (Building*)pos);
				item->update(g);

				molog("[Carrier]: Delivered item inside building.\n");
			} else {
				molog("[Carrier]: Building switch from under us, return to road.\n");

				Road* road = (Road*)get_location(g);
				Flag* flag = road->get_flag((Road::FlagId)0);

				if (((Building*)pos)->get_base_flag() == flag)
					state->ivar1 = 1;
				else
					state->ivar1 = 0;
				break;
			}
		}

		// No more deliverable items. Walk out to the flag.
		molog("[Carrier]: Move out of building.\n");
		start_task_forcemove(WALK_SE,
				descr().get_right_walk_anims(does_carry_ware()));
	} else {
		// We're done
		pop_task();
	}

	return;
}


/**
 * Walks to the queued flag and picks up one acked ware
 *
 * \param g Game the carrier lives on
 * \param s Flags sent to the task
 */

/// \todo Upgrade this function to support many-wares-at-a-time (TM)
void Carrier::pickup_from_flag(Game* g, State* s)
{
	if (!start_task_walktoflag(g, s->ivar1)) {

		m_acked_ware = -1;

		Road* road = (Road*)get_location(g);
		Flag* flag = road->get_flag((Road::FlagId)s->ivar1);
		Flag* otherflag = road->get_flag((Road::FlagId)(s->ivar1 ^ 1));

		// Are there items to move between our flags?
		WareInstance* item = flag->fetch_pending_item(g, otherflag);
		if (!item) {
			molog("[Carrier]: Nothing suitable on flag.\n");
			pop_task();
		} else {
			set_carried_item(g, item);

			set_animation(g, descr().get_animation("idle"));
			schedule_act(g, 20);
		}
	}

	return;
}


/**
 * Drop one item in a flag, and pick up a new one if we acked it
 *
 * \param g Game the carrier lives on.
 * \param s Flags sent to the task
 */
/// \todo Upgrade this function to really support many-wares-at-a-time
void Carrier::drop_item(Game* g, State* s)
{
	WareInstance* other = 0;
	Road* road = (Road*)get_location(g);
	Flag* flag = road->get_flag((Road::FlagId)(s->ivar1 ^ 1));

	if (m_acked_ware == (s->ivar1 ^ 1)) {
		// If there's an item we acked, we can drop ours even if the flag is
		// flooded
		other = flag->fetch_pending_item(g,
				road->get_flag((Road::FlagId)s->ivar1));

		if (!other && !flag->has_capacity()) {
			molog("[Carrier]: strange: acked ware from busy flag no longer present.\n");

			m_acked_ware = -1;
			set_animation(g, descr().get_animation("idle"));
			schedule_act(g, 20);
			return;
		}

		s->ivar1 = m_acked_ware;
		m_acked_ware = -1;
	}

	// Drop our item
	WareInstance* item = fetch_carried_item(g);
	flag->add_item(g, item);

	// Pick up new load, if any
	if (other) {
		molog("[transport]: return trip.\n");

		set_carried_item(g, other);

		set_animation(g, descr().get_animation("idle"));
		schedule_act(g, 20);
	} else {
		molog("[Carrier]: back to idle.\n");
		pop_task();
	}

	return;
}


/**
 * When picking up items, if some of them is targeted to the building attached
 * to target flag walk straight into it and deliver.
 *
 * \param g Game the carrier lives on.
 * \param s Flags sent to the task.
 */
void Carrier::enter_building(Game* g, State* s)
{
	if (!start_task_walktoflag(g, s->ivar1 ^ 1)) {
		molog("[Carrier]: Move into building.\n");
		start_task_forcemove(WALK_NW,
		                     descr().get_right_walk_anims(does_carry_ware()));
		s->ivar1 = -1;
	}

	return;
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
/// \todo Upgrade this function to truly support many-wares-at-a-time
bool Carrier::swap_or_wait(Game *g, State* s)
{
	// Road that employs us
	Road* road = (Road*)get_location(g);
	// Flag we are delivering to
	Flag *flag = road->get_flag((Road::FlagId)(s->ivar1 ^ 1));
	// The other flag of our road
	Flag *otherflag = road->get_flag((Road::FlagId)(s->ivar1));


	if (m_acked_ware == (s->ivar1 ^ 1)) {
		// All is well, we already acked an item that we can pick up
		// from this flag
		return false;
	} else if (flag->has_pending_item(g, otherflag)) {
		if (!flag->ack_pending_item(g, otherflag))
			throw wexception("MO(%u): transport: overload exchange: flag %u is fucked up",
						get_serial(), flag->get_serial());

		m_acked_ware = s->ivar1 ^ 1;
		return false;
	} else if (!start_task_walktoflag(g, s->ivar1 ^ 1, true)) {
		// Wait one field away
		start_task_waitforcapacity(g, flag);
	}

	return true;
}


void Carrier::transport_signal(Game * g, State *)
{
	std::string signal = get_signal();

	if (signal == "road") {
		set_signal("");
		schedule_act(g, 10);
		return;
	}

	molog("[transport]: Interrupted by signal '%s'\n", signal.c_str());
	pop_task();
}


/**
 * Called by Road code to indicate that a new item has arrived on a flag
 * (0 = start, 1 = end).
 * \return true if the carrier is going to fetch it.
 */
bool Carrier::notify_ware(Game* g, int flag)
{
	State* state = get_state();

	// Check if we've already acked something
	if (m_acked_ware >= 0) {
		molog("notify_ware(%i): already acked %i\n", flag, m_acked_ware);
		return false;
	}

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
	State* transport = get_state(&taskTransport);

	if (transport) {
		if ((transport->ivar1 == -1 && find_closest_flag(g) != flag) || flag == transport->ivar1)
			return false;
	}

	// Ack it if we haven't
	molog("notify_ware(%i)\n", flag);
	m_acked_ware = flag;

	if (state->task == &taskRoad)
		send_signal(g, "ware");
	else if (state->task == &taskWaitforcapacity)
		send_signal(g, "wakeup");

	return true;
}


/**
 * Find a pending item on one of the road's flags, ack it and set m_acked_ware
 * accordingly.
 */
void Carrier::find_pending_item(Game* g)
{
	Road* road = (Road*)get_location(g);
	uint32_t haveitembits = 0;

	assert(m_acked_ware < 0);

	if (road->get_flag(Road::FlagStart)->has_pending_item(g,
												road->get_flag(Road::FlagEnd)))
		haveitembits |= 1;

	if (road->get_flag(Road::FlagEnd)->has_pending_item(g,
												road->get_flag(Road::FlagStart)))
		haveitembits |= 2;

	// If both roads have an item, we pick the one closer to us
	if (haveitembits == 3)
		haveitembits = (1 << find_closest_flag(g));

	// Ack our decision
	if (haveitembits == 1) {
		bool ok = false;

		molog("find_pending_item: flag %i\n", 0);
		m_acked_ware = 0;

		ok = road->get_flag(Road::FlagStart)->ack_pending_item(g, road->get_flag(Road::FlagEnd));
		if (!ok)
			throw wexception("Carrier::find_pending_item: start flag is messed up");

		return;
	}

	if (haveitembits == 2) {
		bool ok = false;

		molog("find_pending_item: flag %i\n", 1);
		m_acked_ware = 1;

		ok = road->get_flag(Road::FlagEnd)->ack_pending_item(g, road->get_flag(Road::FlagStart));
		if (!ok)
			throw wexception("Carrier::find_pending_item: end flag is messed up");

		return;
	}
}


/**
 * Find the flag we are closest to (in walking time).
 */
int Carrier::find_closest_flag(Game* g)
{
	Road* road = (Road*)get_location(g);
	CoordPath startpath(g->map(), road->get_path());
	CoordPath endpath;
	int startcost, endcost;
	int curidx = -1;

	curidx = startpath.get_index(get_position());

	// Apparently, we're in a building
	if (curidx < 0) {
		Coords pos = get_position();

		g->get_map()->get_brn(pos, &pos);

		if (pos == startpath.get_start())
			curidx = 0;
		else if (pos == startpath.get_end())
			curidx = startpath.get_nsteps();
		else
			throw wexception("MO(%u): Carrier::find_closest_flag: not on road, not on building",
						get_serial());
	}

	// Calculate the paths and their associated costs
	endpath = startpath;

	startpath.truncate(curidx);
	startpath.reverse();

	endpath.starttrim(curidx);

	g->get_map()->calc_cost(startpath, &startcost, 0);
	g->get_map()->calc_cost(endpath, &endcost, 0);

	if (startcost <= endcost)
		return 0;
	else
		return 1;
}


/**
 * Walk to the given flag, or one field before it if offset is true.
 *
 * \return true if a move task has been started, or false if we're already on
 * the target field.
 */
bool Carrier::start_task_walktoflag(Game* g, int flag, bool offset)
{
	Road* road = (Road*)get_location(g);
	const Path& path = road->get_path();
	int idx;

	if (!flag) {
		idx = 0;
		if (offset)
			idx++;
	} else {
		idx = path.get_nsteps();
		if (offset)
			idx--;
	}

	return start_task_movepath
		(g->map(),
		 path,
		 idx,
		 descr().get_right_walk_anims(does_carry_ware()));
}
