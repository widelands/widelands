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

#include "logic/carrier.h"

#include "economy/flag.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

/**
 * Signal "road" on road split.
 * Signal "ware" when a ware has arrived.
 */
Bob::Task const Carrier::taskRoad = {
	"road",
	static_cast<Bob::Ptr>(&Carrier::road_update),
	nullptr,
	static_cast<Bob::Ptr>(&Carrier::road_pop),
	true
};


/**
 * Work on the given road, assume the location is correct.
*/
void Carrier::start_task_road(Game & game)
{
	push_task(game, taskRoad);

	top_state().ivar1 = 0;

	m_promised_pickup_to = NOONE;
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
	} else if (signal == "blocked") {
		// Blocked by an ongoing battle
		signal_handled();
		set_animation(game, descr().get_animation("idle"));
		return schedule_act(game, 250);
	} else if (signal.size()) {
		// Something else happened (probably a location signal)
		molog("[road]: Terminated by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	Road & road = ref_cast<Road, PlayerImmovable>(*get_location(game));

	// Check for pending wares
	if (m_promised_pickup_to == NOONE)
		find_pending_ware(game);

	if (m_promised_pickup_to != NOONE) {
		if (state.ivar1) {
			state.ivar1 = 0;
			return start_task_transport(game, m_promised_pickup_to);
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
 * We are released, most likely because the road is no longer busy and we are
 * a second carrier (ox or something). If we promised a flag that we would pick up
 * a ware there, we have to make sure that they do not count on us anymore.
 */
void Carrier::road_pop(Game & game, State & /* state */)
{
	if (m_promised_pickup_to != NOONE && get_location(game)) {
		Road & road      = ref_cast<Road, PlayerImmovable>(*get_location(game));
		Flag & flag      = road.get_flag(static_cast<Road::FlagId>(m_promised_pickup_to));
		Flag & otherflag = road.get_flag(static_cast<Road::FlagId>(m_promised_pickup_to ^ 1));

		flag.cancel_pickup(game, otherflag);
	}
}

/**
 * Fetch an ware from a flag, drop it on the other flag.
 * ivar1 is the flag we fetch from, or -1 when we're in the target building.
 *
 * Signal "update" when the road has been split etc.
 */
Bob::Task const Carrier::taskTransport = {
	"transport",
	static_cast<Bob::Ptr>(&Carrier::transport_update),
	nullptr,
	nullptr,
	true
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
	} else if (signal == "blocked") {
		// Blocked by an ongoing battle
		signal_handled();
		set_animation(game, descr().get_animation("idle"));
		return schedule_act(game, 250);
	} else if (signal.size()) {
		molog("[transport]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	Road & road = ref_cast<Road, PlayerImmovable>(*get_location(game));

	if (state.ivar1 == -1)
		// If we're "in" the target building, special code applies
		deliver_to_building(game, state);

	else if (!does_carry_ware())
		// If we don't carry something, walk to the flag
		pickup_from_flag(game, state);

	else {
		// If the ware should go to the building attached to our flag, walk
		// directly into said building
		Flag & flag = road.get_flag(static_cast<Road::FlagId>(state.ivar1 ^ 1));

		WareInstance & ware = *get_carried_ware(game);
		assert(ware.get_location(game) == this);

		// A sanity check is necessary, in case the building has been destroyed
		PlayerImmovable * const next = ware.get_next_move_step(game);

		if (next && next != &flag && &next->base_flag() == &flag)
			enter_building(game, state);

		// If the flag is overloaded we are allowed to drop wares as
		// long as we can pick another up. Otherwise we have to wait.
		else if
			((flag.has_capacity() || !swap_or_wait(game, state))
			 &&
			 !start_task_walktoflag(game, state.ivar1 ^ 1))
			// Drop the ware, possible exchanging it with another one
			drop_ware(game, state);
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
		// Drop all wares addressed to this building
		while (WareInstance * const ware = get_carried_ware(game)) {
			// If the building has disappeared and immediately been replaced
			// with another building, we might have to return without dropping
			// the ware.
			PlayerImmovable const * const next = ware->get_next_move_step(game);

			if (next == pos) {
				fetch_carried_ware(game);
				ware->enter_building(game, *building);
			} else {
				molog
					("[Carrier]: Building switch from under us, return to road.\n");

				state.ivar1 =
					&building->base_flag()
					==
					&ref_cast<Road, PlayerImmovable>(*get_location(game)).get_flag
						(static_cast<Road::FlagId>(0));
				break;
			}
		}

		// No more deliverable wares. Walk out to the flag.
		return
			start_task_move
				(game,
				 WALK_SE,
				 descr().get_right_walk_anims(does_carry_ware()),
				 true);
	} else {
		//  tough luck, the building has disappeared
		molog("[Carrier]: Building disappeared while in building.\n");
		set_location(nullptr);
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

		m_promised_pickup_to = NOONE;

		Road & road      = ref_cast<Road, PlayerImmovable>(*get_location(game));
		Flag & flag      = road.get_flag(static_cast<Road::FlagId>(ivar1));
		Flag & otherflag = road.get_flag(static_cast<Road::FlagId>(ivar1 ^ 1));

		// Are there wares to move between our flags?
		if (WareInstance * const ware = flag.fetch_pending_ware(game, otherflag))
		{
			set_carried_ware(game, ware);

			set_animation(game, descr().get_animation("idle"));
			return schedule_act(game, 20);
		} else {
			molog("[Carrier]: Nothing suitable on flag.\n");
			return pop_task(game);
		}
	}
}


/**
 * Drop one ware in a flag, and pick up a new one if we acked it
 *
 * \param g Game the carrier lives on.
 * \param s Flags sent to the task
 */
void Carrier::drop_ware(Game & game, State & state)
{
	WareInstance * other = nullptr;
	Road & road = ref_cast<Road, PlayerImmovable>(*get_location(game));
	Flag & flag = road.get_flag(static_cast<Road::FlagId>(state.ivar1 ^ 1));

	if (m_promised_pickup_to == (state.ivar1 ^ 1)) {
		// If there's an ware we acked, we can drop ours even if the flag is
		// flooded
		other =
			flag.fetch_pending_ware
				(game, road.get_flag(static_cast<Road::FlagId>(state.ivar1)));

		if (!other && !flag.has_capacity()) {
			molog
				("[Carrier]: strange: acked ware from busy flag no longer "
				 "present.\n");

			m_promised_pickup_to = NOONE;
			set_animation(game, descr().get_animation("idle"));
			return schedule_act(game, 20);
		}

		state.ivar1 = m_promised_pickup_to;
		m_promised_pickup_to = NOONE;
	}

	// Drop our ware
	flag.add_ware(game, *fetch_carried_ware(game));

	// Pick up new load, if any
	if (other) {
		set_carried_ware(game, other);

		set_animation(game, descr().get_animation("idle"));
		return schedule_act(game, 20);
	} else
		return pop_task(game);
}


/**
 * When picking up wares, if some of them is targeted to the building attached
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
				 descr().get_right_walk_anims(does_carry_ware()),
				 true);
	}
}


/**
 * Swaps wares from an overloaded flag for as long as the carrier can pick
 * up new wares from it. Otherwise, changes the carrier state to wait.
 *
 * \param g Game the carrier lives on.
 * \param s Flags sent to the task.
 *
 * \return true if the carrier must wait before delivering his wares.
 */
bool Carrier::swap_or_wait(Game & game, State & state)
{
	// Road that employs us
	Road & road = ref_cast<Road, PlayerImmovable>(*get_location(game));
	// Flag we are delivering to
	Flag & flag = road.get_flag(static_cast<Road::FlagId>(state.ivar1 ^ 1));
	// The other flag of our road
	Flag & otherflag = road.get_flag(static_cast<Road::FlagId>(state.ivar1));


	if (m_promised_pickup_to == (state.ivar1 ^ 1)) {
		// All is well, we already acked an ware that we can pick up
		// from this flag
		return false;
	} else if (flag.has_pending_ware(game, otherflag)) {
		if (!flag.ack_pickup(game, otherflag))
			throw wexception
				("MO(%u): transport: overload exchange: flag %u is fucked up",
				 serial(), flag.serial());

		m_promised_pickup_to = state.ivar1 ^ 1;
		return false;
	} else if (!start_task_walktoflag(game, state.ivar1 ^ 1, true))
		start_task_waitforcapacity(game, flag); //  wait one node away

	return true;
}


/**
 * Called by Road code to indicate that a new ware has arrived on a flag
 * (0 = start, 1 = end).
 * \return true if the carrier is going to fetch it.
 */
bool Carrier::notify_ware(Game & game, int32_t const flag)
{
	State & state = top_state();

	// Check if we've already acked something
	if (m_promised_pickup_to != NOONE)
		return false;

	// If we are currently in a transport.
	// Explanation:
	//  a) a different carrier / road may be better suited for this ware
	//     (the transport code does not have priorities for the actual
	//     carrier that is notified)
	//  b) the transport task has logic that allows it to
	//     drop an ware on an overloaded flag iff it can pick up an ware
	//     at the same time.
	//     We should ack said ware to avoid more confusion before we move
	//     onto the flag, but we can't do that if we have already acked
	//     something.
	//  c) we might ack for a flag that we are actually moving away from;
	//     this will get us into trouble if wares have arrived on the other
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
	m_promised_pickup_to = flag;

	if      (state.task == &taskRoad)
		send_signal(game, "ware");
	else if (state.task == &taskWaitforcapacity)
		send_signal(game, "wakeup");

	return true;
}


/**
 * Find a pending ware on one of the road's flags, ack it and set m_promised_pickup_to
 * accordingly.
 */
void Carrier::find_pending_ware(Game & game)
{
	Road & road = ref_cast<Road, PlayerImmovable>(*get_location(game));
	uint32_t havewarebits = 0;

	assert(m_promised_pickup_to == NOONE);

	if
		(road.get_flag(Road::FlagStart).has_pending_ware
		 	(game, road.get_flag(Road::FlagEnd)))
		havewarebits |= 1;

	if
		(road.get_flag(Road::FlagEnd).has_pending_ware
		 	(game, road.get_flag(Road::FlagStart)))
		havewarebits |= 2;

	//  If both flags have an ware, we pick the one closer to us.
	if (havewarebits == 3)
		havewarebits = 1 << find_closest_flag(game);

	// Ack our decision
	if (havewarebits == 1) {
		m_promised_pickup_to = START_FLAG;
		if
			(!
			 road.get_flag(Road::FlagStart).ack_pickup
			 	(game, road.get_flag(Road::FlagEnd)))
			throw wexception
				("Carrier::find_pending_ware: start flag is messed up");

	} else if (havewarebits == 2) {
		m_promised_pickup_to = END_FLAG;
		if
			(!
			 road.get_flag(Road::FlagEnd).ack_pickup
			 	(game, road.get_flag(Road::FlagStart)))
			throw wexception("Carrier::find_pending_ware: end flag is messed up");
	}
}


/**
 * Find the flag we are closest to (in walking time).
 */
int32_t Carrier::find_closest_flag(Game & game)
{
	Map & map = game.map();
	CoordPath startpath
		(map, ref_cast<Road, PlayerImmovable>(*get_location(game)).get_path());

	CoordPath endpath;
	int32_t startcost, endcost;
	int32_t curidx = startpath.get_index(get_position());

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

	map.calc_cost(startpath, &startcost, nullptr);
	map.calc_cost(endpath,   &endcost,   nullptr);

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
	const Path & path =
		ref_cast<Road, PlayerImmovable>(*get_location(game)).get_path();
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

void Carrier::log_general_info(const Widelands::Editor_Game_Base & egbase)
{
	molog("Carrier at %i,%i\n", get_position().x, get_position().y);

	Worker::log_general_info(egbase);

	molog("m_promised_pickup_to = %i\n", m_promised_pickup_to);
}

/*
==============================

Load/save support

==============================
*/

#define CARRIER_SAVEGAME_VERSION 1

Carrier::Loader::Loader()
{
}

void Carrier::Loader::load(FileRead & fr)
{
	Worker::Loader::load(fr);

	uint8_t version = fr.Unsigned8();
	if (version != CARRIER_SAVEGAME_VERSION)
		throw game_data_error("unknown/unhandled version %u", version);

	Carrier & carrier = get<Carrier>();
	carrier.m_promised_pickup_to = fr.Signed32();
}

const Bob::Task * Carrier::Loader::get_task(const std::string & name)
{
	if (name == "road") return &taskRoad;
	if (name == "transport") return &taskTransport;
	return Worker::Loader::get_task(name);
}

Carrier::Loader * Carrier::create_loader()
{
	return new Loader;
}

void Carrier::do_save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	Worker::do_save(egbase, mos, fw);

	fw.Unsigned8(CARRIER_SAVEGAME_VERSION);
	fw.Signed32(m_promised_pickup_to);
}

}
