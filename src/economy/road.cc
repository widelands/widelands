/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#include "economy/road.h"

#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

namespace Widelands {

// dummy instance because MapObject needs a description
namespace {
const RoadDescr g_road_descr("road", "Road");
}

const RoadDescr& Road::descr() const {
	return g_road_descr;
}

bool Road::is_road_descr(MapObjectDescr const* const descr) {
	return descr == &g_road_descr;
}

/**
 * Most of the actual work is done in init.
*/
Road::Road()
   : PlayerImmovable(g_road_descr),
     busyness_(0),
     busyness_last_update_(0),
     type_(0),
     idle_index_(0) {
	flags_[0] = flags_[1] = nullptr;
	flagidx_[0] = flagidx_[1] = -1;

	// Initialize the worker slots for the road
	// TODO(unknown): make this configurable
	CarrierSlot slot;
	carrier_slots_.push_back(slot);
	carrier_slots_.push_back(slot);
	carrier_slots_[0].carrier_type = 1;
	carrier_slots_[1].carrier_type = 2;
}

Road::CarrierSlot::CarrierSlot() : carrier(nullptr), carrier_request(nullptr), carrier_type(0) {
}

/**
 * Most of the actual work is done in cleanup.
 */
Road::~Road() {
	for (CarrierSlot& slot : carrier_slots_) {
		delete slot.carrier_request;
	}
}

/**
 * Create a road between the given flags, using the given path.
*/
Road& Road::create(EditorGameBase& egbase, Flag& start, Flag& end, const Path& path) {
	assert(start.get_position() == path.get_start());
	assert(end.get_position() == path.get_end());
	assert(start.get_owner() == end.get_owner());

	Player& owner = start.owner();
	Road& road = *new Road();
	road.set_owner(&owner);
	road.type_ = RoadType::kNormal;
	road.flags_[FlagStart] = &start;
	road.flags_[FlagEnd] = &end;
	// flagidx_ is set when attach_road() is called, i.e. in init()
	road.set_path(egbase, path);

	road.init(egbase);

	return road;
}

int32_t Road::get_size() const {
	return SMALL;
}

bool Road::get_passable() const {
	return true;
}

BaseImmovable::PositionList Road::get_positions(const EditorGameBase& egbase) const {
	const Map& map = egbase.map();
	Coords curf = map.get_fcoords(path_.get_start());

	PositionList rv;
	const Path::StepVector::size_type nr_steps = path_.get_nsteps();
	for (Path::StepVector::size_type steps = 0; steps < nr_steps + 1; ++steps) {
		if (steps > 0 && steps < path_.get_nsteps())
			rv.push_back(curf);

		if (steps < path_.get_nsteps())
			map.get_neighbour(curf, path_[steps], &curf);
	}
	return rv;
}

Flag& Road::base_flag() {
	return *flags_[FlagStart];
}

/**
 * Return the cost of getting from fromflag to the other flag.
*/
int32_t Road::get_cost(FlagId fromflag) {
	return cost_[fromflag];
}

/**
 * Set the new path, calculate costs.
 * You have to set start and end flags before calling this function.
*/
void Road::set_path(EditorGameBase& egbase, const Path& path) {
	assert(path.get_nsteps() >= 2);
	assert(path.get_start() == flags_[FlagStart]->get_position());
	assert(path.get_end() == flags_[FlagEnd]->get_position());

	path_ = path;
	egbase.map().calc_cost(path, &cost_[FlagStart], &cost_[FlagEnd]);

	// Figure out where carriers should idle
	idle_index_ = path.get_nsteps() / 2;
}

/**
 * Add road markings to the map
*/
void Road::mark_map(EditorGameBase& egbase) {
	const Map& map = egbase.map();
	FCoords curf = map.get_fcoords(path_.get_start());

	const Path::StepVector::size_type nr_steps = path_.get_nsteps();
	for (Path::StepVector::size_type steps = 0; steps < nr_steps + 1; ++steps) {
		if (steps > 0 && steps < path_.get_nsteps())
			set_position(egbase, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir = get_reverse_dir(path_[steps - 1]);
			Direction const rdir = 2 * (dir - WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, type_);
		}

		// mark the road that leads away from this field
		if (steps < path_.get_nsteps()) {
			const Direction dir = path_[steps];
			Direction const rdir = 2 * (dir - WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, type_);

			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Remove road markings from the map
*/
void Road::unmark_map(EditorGameBase& egbase) {
	const Map& map = egbase.map();
	FCoords curf(path_.get_start(), &map[path_.get_start()]);

	const Path::StepVector::size_type nr_steps = path_.get_nsteps();
	for (Path::StepVector::size_type steps = 0; steps < nr_steps + 1; ++steps) {
		if (steps > 0 && steps < path_.get_nsteps())
			unset_position(egbase, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir = get_reverse_dir(path_[steps - 1]);
			Direction const rdir = 2 * (dir - WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, RoadType::kNone);
		}

		// mark the road that leads away from this field
		if (steps < path_.get_nsteps()) {
			const Direction dir = path_[steps];
			Direction const rdir = 2 * (dir - WALK_E);

			if (rdir <= 4)
				egbase.set_road(curf, rdir, RoadType::kNone);

			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Initialize the road.
*/
bool Road::init(EditorGameBase& egbase) {
	PlayerImmovable::init(egbase);

	if (2 <= path_.get_nsteps())
		link_into_flags(egbase);
	return true;
}

/**
 * This links into the flags, calls a carrier
 * and so on. This was formerly done in init (and
 * still is for normal games). But for save game loading
 * we needed to have this road already registered
 * as Map Object, thats why this is moved
 */
void Road::link_into_flags(EditorGameBase& egbase) {
	assert(path_.get_nsteps() >= 2);

	// Link into the flags (this will also set our economy)
	{
		const Direction dir = path_[0];
		flags_[FlagStart]->attach_road(dir, this);
		flagidx_[FlagStart] = dir;
	}

	const Direction dir = get_reverse_dir(path_[path_.get_nsteps() - 1]);
	flags_[FlagEnd]->attach_road(dir, this);
	flagidx_[FlagEnd] = dir;

	Economy::check_merge(*flags_[FlagStart], *flags_[FlagEnd]);

	// Mark Fields
	mark_map(egbase);

	/*
	 * Iterate over all Carrierslots
	 * If a carrier is set assign it to this road, else
	 * request a new carrier
	 */
	if (upcast(Game, game, &egbase)) {
		for (CarrierSlot& slot : carrier_slots_) {
			if (Carrier* const carrier = slot.carrier.get(*game)) {
				//  This happens after a road split. Tell the carrier what's going on.
				carrier->set_location(this);
				carrier->update_task_road(*game);
			} else if (!slot.carrier_request && (slot.carrier_type == 1 || type_ == RoadType::kBusy)) {
				request_carrier(slot);
			}
		}
	}
}

/**
 * Cleanup the road
*/
void Road::cleanup(EditorGameBase& egbase) {

	for (CarrierSlot& slot : carrier_slots_) {
		delete slot.carrier_request;
		slot.carrier_request = nullptr;

		// carrier will be released via PlayerImmovable::cleanup
		slot.carrier = nullptr;
	}

	// Unmark Fields
	unmark_map(egbase);

	// Unlink from flags (also clears the economy)
	flags_[FlagStart]->detach_road(flagidx_[FlagStart]);
	flags_[FlagEnd]->detach_road(flagidx_[FlagEnd]);

	Economy::check_split(*flags_[FlagStart], *flags_[FlagEnd]);

	if (upcast(Game, game, &egbase)) {
		flags_[FlagStart]->update_wares(*game, flags_[FlagEnd]);
		flags_[FlagEnd]->update_wares(*game, flags_[FlagStart]);
	}

	PlayerImmovable::cleanup(egbase);
}

/**
 * Workers' economies are fixed by PlayerImmovable, but we need to handle
 * any requests ourselves.
*/
void Road::set_economy(Economy* const e) {
	PlayerImmovable::set_economy(e);

	for (CarrierSlot& slot : carrier_slots_) {
		if (slot.carrier_request) {
			slot.carrier_request->set_economy(e);
		}
	}
}

/**
 * Request a new carrier.
 *
 * Only call this if the road can handle a new carrier, and if no request has
 * been issued.
*/
void Road::request_carrier(CarrierSlot& slot) {
	if (slot.carrier_type == 1)
		slot.carrier_request =
		   new Request(*this, owner().tribe().carrier(), Road::request_carrier_callback, wwWORKER);
	else
		slot.carrier_request =
		   new Request(*this, owner().tribe().carrier2(), Road::request_carrier_callback, wwWORKER);
}

/**
 * The carrier has arrived successfully.
*/
void Road::request_carrier_callback(
   Game& game, Request& rq, DescriptionIndex, Worker* const w, PlayerImmovable& target) {
	assert(w);

	Road& road = dynamic_cast<Road&>(target);

	for (CarrierSlot& slot : road.carrier_slots_) {
		if (slot.carrier_request == &rq) {
			Carrier& carrier = dynamic_cast<Carrier&>(*w);
			slot.carrier_request = nullptr;
			slot.carrier = &carrier;

			carrier.start_task_road(game);
			delete &rq;
			return;
		}
	}

	/*
	 * Oops! We got a request_callback but don't have the request.
	 * Try to send him home.
	 */
	log("Road(%u): got a request_callback but do not have the request\n", road.serial());
	delete &rq;
	w->start_task_gowarehouse(game);
}

/**
 * If we lost our carrier, re-request it.
*/
void Road::remove_worker(Worker& w) {
	EditorGameBase& egbase = owner().egbase();

	for (CarrierSlot& slot : carrier_slots_) {
		Carrier const* carrier = slot.carrier.get(egbase);

		if (carrier == &w) {
			slot.carrier = nullptr;
			carrier = nullptr;
			request_carrier(slot);
		}
	}

	PlayerImmovable::remove_worker(w);
}

/**
 * A carrier was created by someone else (e.g. Scripting Engine)
 * and should now be assigned to this road.
 */
void Road::assign_carrier(Carrier& c, uint8_t slot) {
	assert(slot <= 1);

	// Send the worker home if it occupies our slot
	CarrierSlot& s = carrier_slots_[slot];

	delete s.carrier_request;
	s.carrier_request = nullptr;
	if (Carrier* const current_carrier = s.carrier.get(owner().egbase()))
		current_carrier->set_location(nullptr);

	carrier_slots_[slot].carrier = &c;
	carrier_slots_[slot].carrier_request = nullptr;
}

/**
 * A flag has been placed that splits this road. This function is called before
 * the new flag initializes. We remove markings to avoid interference with the
 * flag.
*/
void Road::presplit(Game& game, Coords) {
	unmark_map(game);
}

/**
 * The flag that splits this road has been initialized. Perform the actual
 * splitting.
 *
 * After the split, this road will span [start...new flag]. A new road will
 * be created to span [new flag...end]
*/
// TODO(SirVer): This needs to take an EditorGameBase as well.
void Road::postsplit(Game& game, Flag& flag) {
	Flag& oldend = *flags_[FlagEnd];

	// detach from end
	oldend.detach_road(flagidx_[FlagEnd]);

	// build our new path and the new road's path
	const Map& map = game.map();
	CoordPath path(map, path_);
	CoordPath secondpath(path);
	int32_t const index = path.get_index(flag.get_position());

	assert(index > 0);
	assert(static_cast<uint32_t>(index) < path.get_nsteps() - 1);

	path.truncate(index);
	secondpath.trim_start(index);

	molog("splitting road: first part:\n");
	for (const Coords& coords : path.get_coords()) {
		molog("* (%i, %i)\n", coords.x, coords.y);
	}
	molog("                second part:\n");
	for (const Coords& coords : secondpath.get_coords()) {
		molog("* (%i, %i)\n", coords.x, coords.y);
	}

	// change road size and reattach
	flags_[FlagEnd] = &flag;
	set_path(game, path);

	const Direction dir = get_reverse_dir(path_[path_.get_nsteps() - 1]);
	flags_[FlagEnd]->attach_road(dir, this);
	flagidx_[FlagEnd] = dir;

	// recreate road markings
	mark_map(game);

	// create the new road
	Road& newroad = *new Road();
	newroad.set_owner(get_owner());
	newroad.type_ = type_;
	newroad.flags_[FlagStart] = &flag;  //  flagidx will be set on init()
	newroad.flags_[FlagEnd] = &oldend;
	newroad.set_path(game, secondpath);

	// Find workers on this road that need to be reassigned
	// The algorithm is pretty simplistic, and has a bias towards keeping
	// the worker around; there's obviously nothing wrong with that.

	std::vector<Worker*> const workers = get_workers();
	std::vector<Worker*> reassigned_workers;

	for (Worker* w : workers) {
		int32_t idx = path.get_index(w->get_position());

		// Careful! If the worker is currently inside the building at our
		// starting flag, we *must not* reassign him.
		// If he is in the building at our end flag or at the other road's
		// end flag, he can be reassigned to the other road.
		if (idx < 0) {
			if (dynamic_cast<Building const*>(map.get_immovable(w->get_position()))) {
				Coords pos;
				map.get_brn(w->get_position(), &pos);
				if (pos == path.get_start())
					idx = 0;
			}
		}

		if (idx < 0) {
			reassigned_workers.push_back(w);

			/*
			 * The current worker is not on this road. Search him
			 * in this road and remove him. Than add him to the new road
			 */
			for (CarrierSlot& old_slot : carrier_slots_) {
				Carrier const* const carrier = old_slot.carrier.get(game);

				if (carrier == w) {
					old_slot.carrier = nullptr;
					for (CarrierSlot& new_slot : newroad.carrier_slots_) {
						if (!new_slot.carrier.get(game) && !new_slot.carrier_request &&
						    new_slot.carrier_type == old_slot.carrier_type) {
							upcast(Carrier, new_carrier, w);
							new_slot.carrier = new_carrier;
							break;
						}
					}
				}
			}
		}

		// Cause a worker update in any case
		w->send_signal(game, "road");
	}

	// Initialize the new road
	newroad.init(game);

	// Actually reassign workers after the new road has initialized,
	// so that the reassignment is safe
	for (Worker*& w : reassigned_workers) {
		w->set_location(&newroad);
	}

	//  Request a new carrier for this road if necessary. This must be done
	//  _after_ the new road initializes, otherwise request routing might not
	//  work correctly
	for (CarrierSlot& slot : carrier_slots_) {
		if (!slot.carrier.get(game) && !slot.carrier_request &&
		    (slot.carrier_type == 1 || type_ == RoadType::kBusy)) {
			request_carrier(slot);
		}
	}

	//  Make sure wares waiting on the original endpoint flags are dealt with.
	flags_[FlagStart]->update_wares(game, &oldend);
	oldend.update_wares(game, flags_[FlagStart]);
}

/**
 * Called by Flag code: an ware should be picked up from the given flag.
 * \return true if a carrier has been sent on its way, false otherwise.
 */
bool Road::notify_ware(Game& game, FlagId const flagid) {
	uint32_t const gametime = game.get_gametime();
	assert(busyness_last_update_ <= gametime);
	uint32_t const tdelta = gametime - busyness_last_update_;

	//  Iterate over all carriers and try to find one which takes the ware.
	for (CarrierSlot& slot : carrier_slots_) {
		if (Carrier* const carrier = slot.carrier.get(game))
			if (carrier->notify_ware(game, flagid)) {
				//  notify_ware returns false if the carrier currently can not take
				//  the ware. If we get here, the carrier took the ware. So we
				//  decrement the usage busyness.
				if (500 < tdelta) {
					busyness_last_update_ = gametime;
					if (busyness_) {
						--busyness_;

						// If busyness_ drops below a limit, release the donkey.
						// remember that every time a ware is waiting at the flag
						// busyness_ increase by 10 but every time a ware is immediatly
						// acked by a carrier busyness_ is decreased by 1 only.
						// so the limit is not so easy to reach
						if (busyness_ < 350) {
							Carrier* const second_carrier = carrier_slots_[1].carrier.get(game);
							if (second_carrier && second_carrier->top_state().task == &Carrier::taskRoad) {
								second_carrier->send_signal(game, "cancel");
								// this signal is not handled in any special way
								// so it simply pop the task off the stack
								// the string "cancel" has been used to make clear
								// the final goal we want to achieve
								// ie: cancelling current task
								carrier_slots_[1].carrier = nullptr;
								carrier_slots_[1].carrier_request = nullptr;
								type_ = RoadType::kNormal;
								mark_map(game);
							}
						}
					}
				}
				return true;
			}
	}

	//  If we get here, no carrier took the ware. So we check if we should
	//  increment the usage counter. busyness_last_update_ prevents that the
	//  counter is incremented too often.
	if (100 < tdelta) {
		busyness_last_update_ = gametime;
		if (500 < (busyness_ += 10)) {
			type_ = RoadType::kBusy;
			mark_map(game);
			for (CarrierSlot& slot : carrier_slots_) {
				if (!slot.carrier.get(game) && !slot.carrier_request && slot.carrier_type != 1) {
					request_carrier(slot);
				}
			}
		}
	}
	return false;
}

void Road::log_general_info(const EditorGameBase& egbase) {
	PlayerImmovable::log_general_info(egbase);
	molog("busyness_: %i\n", busyness_);
}
}
