/*
 * Copyright (C) 2004-2018 by the Widelands Development Team
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

#include "economy/roadbase.h"

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

/**
 * Most of the actual work is done in init.
 */
RoadBase::RoadBase(const RoadBaseDescr& d, RoadType type, uint8_t nb_carriers, ...)
   : PlayerImmovable(d), idle_index_(0), type_(type) {
	flags_[0] = flags_[1] = nullptr;
	flagidx_[0] = flagidx_[1] = -1;

	// Initialize the worker slots for the road
	CarrierSlot slot;
	va_list types;
	va_start(types, nb_carriers);
	for (uint8_t i = 0; i < nb_carriers; i++) {
	    carrier_slots_.push_back(slot);
    	carrier_slots_[i++].carrier_type = static_cast<uint8_t>(va_arg(types, int));
	}
}

RoadBase::CarrierSlot::CarrierSlot() : carrier(nullptr), carrier_request(nullptr), carrier_type(0) {
}

/**
 * Most of the actual work is done in cleanup.
 */
RoadBase::~RoadBase() {
	for (CarrierSlot& slot : carrier_slots_) {
		delete slot.carrier_request;
	}
}

int32_t RoadBase::get_size() const {
	return SMALL;
}

bool RoadBase::get_passable() const {
	return true;
}

BaseImmovable::PositionList RoadBase::get_positions(const EditorGameBase& egbase) const {
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

Flag& RoadBase::base_flag() {
	return *flags_[FlagStart];
}

/**
 * Return the cost of getting from fromflag to the other flag.
 */
int32_t RoadBase::get_cost(FlagId fromflag) {
	return cost_[fromflag];
}

/**
 * Set the new path, calculate costs.
 * You have to set start and end flags before calling this function.
 */
void RoadBase::set_path(EditorGameBase& egbase, const Path& path) {
	assert(path.get_nsteps() >= 2);
	assert(path.get_start() == flags_[FlagStart]->get_position());
	assert(path.get_end() == flags_[FlagEnd]->get_position());

	path_ = path;
	egbase.map().calc_cost(path, &cost_[FlagStart], &cost_[FlagEnd]);

	// Figure out where carriers should idle
	idle_index_ = path.get_nsteps() / 2;
}

/**
 * Add road/waterway markings to the map
 */
void RoadBase::mark_map(EditorGameBase& egbase) {
	const Map& map = egbase.map();
	FCoords curf = map.get_fcoords(path_.get_start());

	const Path::StepVector::size_type nr_steps = path_.get_nsteps();
	for (Path::StepVector::size_type steps = 0; steps < nr_steps + 1; ++steps) {
		if (steps > 0 && steps < path_.get_nsteps())
			set_position(egbase, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir = get_reverse_dir(path_[steps - 1]);
			if (dir == WALK_SW || dir == WALK_SE || dir == WALK_E)
				egbase.set_road(curf, dir, type_);
		}

		// mark the road that leads away from this field
		if (steps < path_.get_nsteps()) {
			const Direction dir = path_[steps];
			if (dir == WALK_SW || dir == WALK_SE || dir == WALK_E)
				egbase.set_road(curf, dir, type_);
			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Remove road markings from the map
 */
void RoadBase::unmark_map(EditorGameBase& egbase) {
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

			if (rdir == WALK_SW || rdir == WALK_SE || rdir == WALK_E)
				egbase.set_road(curf, rdir, RoadType::kNone);
		}

		// mark the road that leads away from this field
		if (steps < path_.get_nsteps()) {
			const Direction dir = path_[steps];
			Direction const rdir = 2 * (dir - WALK_E);

			if (rdir == WALK_SW || rdir == WALK_SE || rdir == WALK_E)
				egbase.set_road(curf, rdir, RoadType::kNone);

			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Initialize the RoadBase.
 */
bool RoadBase::init(EditorGameBase& egbase) {
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
void RoadBase::link_into_flags(EditorGameBase& egbase) {
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
				// This happens after a road split. Tell the carrier what's going on.
				carrier->set_location(this);
				carrier->update_task_road(*game);
			} else if (!slot.carrier_request && (slot.carrier_type == 0 || slot.carrier_type == 1 ||
					(slot.carrier_type == 2 && get_roadtype() == RoadType::kBusy))) {
				// Normal carriers and ferries are requested at once, second carriers only for busy roads
				request_carrier(slot);
			}
		}
	}
}

/**
 * Cleanup the road
 */
void RoadBase::cleanup(EditorGameBase& egbase) {

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
void RoadBase::set_economy(Economy* const e) {
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
void RoadBase::request_carrier(CarrierSlot& slot) {
	DescriptionIndex di = INVALID_INDEX;
	switch (slot.carrier_type) {
		case 0:
			di = owner().tribe().ferry();
			break;
		case 1:
			di = owner().tribe().carrier();
			break;
		case 2:
			di = owner().tribe().carrier2();
			break;
		default:
			throw wexception("Invalid carrier type index %i", slot.carrier_type);
	}
	slot.carrier_request = new Request(*this, di, RoadBase::request_carrier_callback, wwWORKER);
}

/**
 * The carrier has arrived successfully.
 */
void RoadBase::request_carrier_callback(
   Game& game, Request& rq, DescriptionIndex, Worker* const w, PlayerImmovable& target) {
	assert(w);

	RoadBase& road = dynamic_cast<RoadBase&>(target);

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
	log("RoadBase(%u): got a request_callback but do not have the request\n", road.serial());
	delete &rq;
	w->start_task_gowarehouse(game);
}
uint8_t RoadBase::carriers_count() const {
	return (carrier_slots_[1].carrier == nullptr) ? 1 : 2;
}

/**
 * If we lost our carrier, re-request it.
 */
void RoadBase::remove_worker(Worker& w) {
	EditorGameBase& egbase = get_owner()->egbase();

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
void RoadBase::assign_carrier(Carrier& c, uint8_t slot) {
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
 * Try to pick up a ware from the given flag.
 * \return true if a carrier has been sent on its way, false otherwise.
 */
bool RoadBase::notify_ware(Game& game, FlagId const flagid) {
	// Iterate over all carriers and try to find one which will take the ware
	for (CarrierSlot& slot : carrier_slots_) {
		if (Carrier* const carrier = slot.carrier.get(game)) {
			if (carrier->notify_ware(game, flagid)) {
				// The carrier took the ware, so we're done
				return true;
			}
		}
	}
	// No carrier took the ware
	return false;
}

}  // namespace Widelands
