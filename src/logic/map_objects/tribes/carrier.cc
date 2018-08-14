/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#include "logic/map_objects/tribes/carrier.h"

#include <memory>

#include "base/macros.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/ware_instance.h"
#include "graphic/rendertarget.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"

namespace Widelands {

/**
 * Signal "road" on road split.
 * Signal "ware" when a ware has arrived.
 */
Bob::Task const Carrier::taskRoad = {"road", static_cast<Bob::Ptr>(&Carrier::road_update), nullptr,
                                     static_cast<Bob::Ptr>(&Carrier::road_pop), true};

/**
 * Called by road code when the carrier has arrived successfully.
 * Work on the given road, assume the location is correct.
 */
void Carrier::start_task_road(Game& game) {
	push_task(game, taskRoad);

	top_state().ivar1 = 0;

	operation_ = INIT;
}

/**
 * Called by Road code when the road is split.
*/
void Carrier::update_task_road(Game& game) {
	send_signal(game, "road");
}

void Carrier::road_update(Game& game, State& state) {
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

	if (operation_ == INIT) {
		operation_ = find_source_flag(game);
	}

	if (operation_ > NO_OPERATION) {
		if (state.ivar1) {
			state.ivar1 = 0;
			return start_task_transport(game, operation_);
		} else {
			// Short delay before we move to pick up
			state.ivar1 = 1;

			set_animation(game, descr().get_animation("idle"));
			return schedule_act(game, 50);
		}
	}

	Road& road = dynamic_cast<Road&>(*get_location(game));

	// Move into idle position if necessary
	if (start_task_movepath(game, road.get_path(), road.get_idle_index(),
	                        descr().get_right_walk_anims(does_carry_ware())))
		return;

	// Be bored. There's nothing good on TV, either.
	// TODO(unknown): idle animations
	set_animation(game, descr().get_animation("idle"));
	state.ivar1 = 1;  //  we are available immediately after an idle phase
	// subtract maintenance cost and check for road demotion
	road.charge_wallet(game);
	// if road still promoted then schedule demotion, otherwise go fully idle waiting until signal
	return road.get_roadtype() == RoadType::kBusy ? schedule_act(game, (road.wallet() + 2) * 500) :
	                                                skip_act();
}

/**
 * We are released, most likely because the road is no longer busy and we are
 * a second carrier (ox or something). If we promised a flag that we would pick up
 * a ware there, we have to make sure that they do not count on us anymore.
 */
void Carrier::road_pop(Game& game, State& /* state */) {
	if (operation_ > NO_OPERATION && get_location(game)) {
		Road& road = dynamic_cast<Road&>(*get_location(game));
		Flag& flag = road.get_flag(static_cast<Road::FlagId>(operation_));
		Flag& otherflag = road.get_flag(static_cast<Road::FlagId>(operation_ ^ 1));

		flag.cancel_pickup(game, otherflag);
	}
}

/**
 * Fetch a ware from a flag, drop it on the other flag.
 * ivar1 is the flag we fetch from, or -1 when we're in the target building.
 *
 * Signal "update" when the road has been split etc.
 */
Bob::Task const Carrier::taskTransport = {
   "transport", static_cast<Bob::Ptr>(&Carrier::transport_update), nullptr, nullptr, true};

/**
 * Begin the transport task.
 */
void Carrier::start_task_transport(Game& game, int32_t const fromflag) {
	push_task(game, taskTransport);
	top_state().ivar1 = fromflag;
}

void Carrier::transport_update(Game& game, State& state) {
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

	int32_t const ivar1 = state.ivar1;
	if (ivar1 == -1) {
		// If we're "in" the target building, special code applies
		return deliver_to_building(game, state);
	}

	WareInstance* ware = get_carried_ware(game);
	if (ware) {
		assert(ware->get_location(game) == this);
	}

	Road& road = dynamic_cast<Road&>(*get_location(game));
	int32_t const dest = ware ? ivar1 ^ 1 : ivar1;
	Flag& flag = road.get_flag(static_cast<Road::FlagId>(dest));

	if (ware) {
		// If the ware should go to the building attached to our flag,
		// walk directly into said building
		// A sanity check is necessary, in case the building has been destroyed

		PlayerImmovable* const next = ware->get_next_move_step(game);
		if (next && next != &flag && &next->base_flag() == &flag) {
			// pay some coins before entering the building,
			// to compensate for the time to be spent in its street-segment
			road.pay_for_building();

			if (!start_task_walktoflag(game, dest)) {
				// Enter building
				state.ivar1 = -1;
				start_task_move(game, WALK_NW, descr().get_right_walk_anims(does_carry_ware()), true);
			}
			return;
		}
	}

	if (!start_task_walktoflag(game, dest, operation_ == WAIT)) {
		// If the flag is overloaded we are allowed to drop wares,
		// as long as we can pick another up. Otherwise we have to wait.

		Flag& otherflag = road.get_flag(static_cast<Road::FlagId>(dest ^ 1));
		int32_t otherware_idx =
		   ware ? flag.find_swappable_ware(*ware, otherflag) : flag.find_pending_ware(otherflag);
		if (operation_ == WAIT) {
			if (otherware_idx < kNotFoundAppropriate) {
				return start_task_waitforcapacity(game, flag);  // join flag's wait queue
			} else {
				operation_ = dest ^ 1;  // resume transport without joining flag's wait queue
				set_animation(game, descr().get_animation("idle"));
				return schedule_act(game, 20);
			}
		} else if (otherware_idx < kNotFoundAppropriate) {
			operation_ = WAIT;  // move one node away
			set_animation(game, descr().get_animation("idle"));
			return schedule_act(game, 20);
		}

		WareInstance* otherware = flag.fetch_pending_ware(game, otherware_idx);

		if (ware) {
			// Drop our ware
			flag.add_ware(game, *fetch_carried_ware(game));
		}

		// Pick up new load, if any
		if (otherware) {
			// pay before getting the ware, while checking for road promotion
			road.pay_for_road(game, flag.count_wares_in_queue(otherflag));

			set_carried_ware(game, otherware);
			flag.ware_departing(game);

			operation_ = state.ivar1 = dest;
			set_animation(game, descr().get_animation("idle"));
			schedule_act(game, 20);
		} else {
			Flag::PendingWare* pw = otherflag.get_ware_for_flag(flag, kPendingOnly);
			if (pw) {
				pw->pending = false;

				operation_ = state.ivar1 = dest ^ 1;
				set_animation(game, descr().get_animation("idle"));
				schedule_act(game, 20);
			} else {
				operation_ = NO_OPERATION;
				pop_task(game);
			}
		}
	}
}

/**
 * Deliver all wares addressed to the building the carrier is already into
 *
 * \param state UNDOCUMENTED
 */
// TODO(unknown):  Upgrade this function to really support many-wares-at-a-time
// TODO(unknown):  Document parameter state
void Carrier::deliver_to_building(Game& game, State& state) {
	BaseImmovable* const pos = game.map()[get_position()].get_immovable();

	if (dynamic_cast<Flag const*>(pos)) {
		operation_ = INIT;
		return pop_task(game);  //  we are done
	} else if (upcast(Building, building, pos)) {
		// Drop all wares addressed to this building
		while (WareInstance* const ware = get_carried_ware(game)) {
			// If the building has disappeared and immediately been replaced
			// with another building, we might have to return without dropping
			// the ware.
			PlayerImmovable const* const next = ware->get_next_move_step(game);

			if (next == pos) {
				fetch_carried_ware(game);
				ware->enter_building(game, *building);
			} else {
				molog("[Carrier]: Building switch from under us, return to road.\n");

				state.ivar1 =
				   &building->base_flag() ==
				   &dynamic_cast<Road&>(*get_location(game)).get_flag(static_cast<Road::FlagId>(0));
				break;
			}
		}

		// No more deliverable wares. Walk out to the flag.
		return start_task_move(game, WALK_SE, descr().get_right_walk_anims(does_carry_ware()), true);
	} else {
		//  tough luck, the building has disappeared
		molog("[Carrier]: Building disappeared while in building.\n");
		set_location(nullptr);
	}
}

/**
 * Called by road code to indicate that the given flag
 * (0 = start, 1 = end) has a ware ready for transfer.
 * \return true if the carrier is going to fetch it.
 */
bool Carrier::notify_ware(Game& game, int32_t const flag) {
	State& state = top_state();

	if (operation_ == WAIT) {
		if (state.objvar1.get(game) ==
		    &dynamic_cast<Road&>(*get_location(game)).get_flag(static_cast<Road::FlagId>(flag))) {
			operation_ = flag;
			send_signal(game, "wakeup");
			return true;
		}
	} else if (operation_ == NO_OPERATION) {
		operation_ = flag;
		send_signal(game, "ware");
		return true;
	}

	return false;
}

/**
 * Find a pending ware meant for our road,
 * remove its pending status, and
 * \return the flag it is on.
 */
int32_t Carrier::find_source_flag(Game& game) {
	assert(operation_ == INIT);

	Road& road = dynamic_cast<Road&>(*get_location(game));
	int32_t near = find_closest_flag(game);
	Flag& nearflag = road.get_flag(static_cast<Road::FlagId>(near));
	Flag& farflag = road.get_flag(static_cast<Road::FlagId>(near ^ 1));

	Flag::PendingWare* pw;
	if ((pw = nearflag.get_ware_for_flag(farflag))) {
		pw->pending = false;
		return near;
	} else if ((pw = farflag.get_ware_for_flag(nearflag, kPendingOnly))) {
		pw->pending = false;
		return near ^ 1;
	} else {
		return NO_OPERATION;
	}
}

/**
 * Find the flag we are closest to (in walking time).
 */
int32_t Carrier::find_closest_flag(Game& game) {
	const Map& map = game.map();
	CoordPath startpath(map, dynamic_cast<Road&>(*get_location(game)).get_path());

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
			throw wexception("MO(%u): Carrier::find_closest_flag: not on road, not on "
			                 "building",
			                 serial());
	}

	// Calculate the paths and their associated costs
	endpath = startpath;

	startpath.truncate(curidx);
	startpath.reverse();

	endpath.trim_start(curidx);

	map.calc_cost(startpath, &startcost, nullptr);
	map.calc_cost(endpath, &endcost, nullptr);

	return endcost < startcost;
}

/**
 * Walk to the given flag, or one field before it if offset is true.
 *
 * \return true if a move task has been started, or false if we're already on
 * the target field.
 */
bool Carrier::start_task_walktoflag(Game& game, int32_t const flag, bool const offset) {
	const Path& path = dynamic_cast<Road&>(*get_location(game)).get_path();
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

	return start_task_movepath(game, path, idx, descr().get_right_walk_anims(does_carry_ware()));
}

void Carrier::log_general_info(const Widelands::EditorGameBase& egbase) {
	molog("Carrier at %i,%i\n", get_position().x, get_position().y);

	Worker::log_general_info(egbase);

	molog("operation = %i\n", operation_);
}

/*
==============================

Load/save support

==============================
*/
constexpr uint8_t kCurrentPacketVersion = 2;

Carrier::Loader::Loader() {
}

void Carrier::Loader::load(FileRead& fr) {
	Worker::Loader::load(fr);

	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			Carrier& carrier = get<Carrier>();
			carrier.operation_ = fr.signed_32();
		} else {
			throw UnhandledVersionError("Carrier", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw wexception("loading carrier: %s", e.what());
	}
}

const Bob::Task* Carrier::Loader::get_task(const std::string& name) {
	if (name == "road")
		return &taskRoad;
	if (name == "transport")
		return &taskTransport;
	return Worker::Loader::get_task(name);
}

Carrier::Loader* Carrier::create_loader() {
	return new Loader;
}

void Carrier::do_save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	Worker::do_save(egbase, mos, fw);

	fw.unsigned_8(kCurrentPacketVersion);
	fw.signed_32(operation_);
}

CarrierDescr::CarrierDescr(const std::string& init_descname,
                           const LuaTable& table,
                           const EditorGameBase& egbase)
   : WorkerDescr(init_descname, MapObjectType::CARRIER, table, egbase) {
}

/**
 * Create a new carrier
 */
Bob& CarrierDescr::create_object() const {
	return *new Carrier(*this);
}
}
