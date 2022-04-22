/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "economy/waterway.h"

#include "base/macros.h"
#include "economy/economy.h"
#include "economy/ferry_fleet.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

namespace Widelands {

// dummy instance because MapObject needs a description
namespace {
const WaterwayDescr g_waterway_descr("waterway", "Waterway");
}  // namespace

bool Waterway::is_waterway_descr(MapObjectDescr const* const descr) {
	return descr == &g_waterway_descr;
}

/**
 * Most of the actual work is done in init.
 */
Waterway::Waterway() : RoadBase(g_waterway_descr), ferry_(nullptr), fleet_(nullptr) {
}

/**
 * Create a waterway between the given flags, using the given path.
 */
Waterway& Waterway::create(EditorGameBase& egbase, Flag& start, Flag& end, const Path& path) {
	assert(start.get_position() == path.get_start());
	assert(end.get_position() == path.get_end());
	assert(start.get_owner() == end.get_owner());

	Waterway& waterway = *new Waterway();
	waterway.set_owner(start.get_owner());
	waterway.flags_[FlagStart] = &start;
	waterway.flags_[FlagEnd] = &end;
	waterway.set_path(egbase, path);

	waterway.init(egbase);

	return waterway;
}

void Waterway::link_into_flags(EditorGameBase& egbase, bool loading) {
	RoadBase::link_into_flags(egbase);
	Economy::check_merge(*flags_[FlagStart], *flags_[FlagEnd], wwWARE);
	if (!loading) {
		if (egbase.is_game()) {
			request_ferry(egbase);
		}
	}
}

bool Waterway::notify_ware(Game& game, FlagId flag) {
	Ferry* f = ferry_.get(game);
	return (f != nullptr) && f->notify_ware(game, flag);
}

void Waterway::remove_worker(Worker& w) {
	if (ferry_.get(owner().egbase()) == &w) {
		ferry_ = nullptr;
		// Since waterways cannot have any other worker than their own ferry and since a
		// ferry cannot be disassigned except by destroying its waterway, I assume that
		// this method is called only on cleanup, so we don't need to issue a new request
	}
	PlayerImmovable::remove_worker(w);
}

void Waterway::request_ferry(EditorGameBase& egbase) {
	FerryFleet* fleet = new FerryFleet(get_owner());
	fleet->init(egbase, this);
}

void Waterway::assign_carrier(Carrier& c, uint8_t /* slot */) {
	if (Ferry* f = ferry_.get(owner().egbase())) {
		f->set_location(nullptr);
	}
	ferry_ = dynamic_cast<Ferry*>(&c);
}

void Waterway::set_fleet(FerryFleet* fleet) {
	if (fleet_ == fleet) {
		return;
	}
	FerryFleet* old_fleet = fleet_.get(get_owner()->egbase());
	if ((old_fleet != nullptr) && (fleet != nullptr)) {
		// Only if the new fleet is non-null, to avoid problems in end-of-game cleanup
		if (upcast(Game, game, &get_owner()->egbase())) {
			old_fleet->cancel_ferry_request(*game, this);
		}
	}
	// This function should be called only by Fleet code, so we assume that the
	// new fleet (if non-null) already registered us for a ferry request
	fleet_ = fleet;
}

/**
 * The flag that splits this road has been initialized. Perform the actual
 * splitting.
 *
 * After the split, this road will span [start...new flag]. A new road will
 * be created to span [new flag...end]
 */
// TODO(SirVer): This needs to take an EditorGameBase as well.
void Waterway::postsplit(Game& game, Flag& flag) {
	Flag& oldend = *flags_[FlagEnd];

	// detach from end
	oldend.detach_road(flagidx_[FlagEnd]);

	// build our new path and the new waterway's path
	const Map& map = game.map();
	CoordPath path(map, path_);
	CoordPath secondpath(path);
	int32_t const index = path.get_index(flag.get_position());

	assert(index > 0);
	assert(static_cast<uint32_t>(index) < path.get_nsteps() - 1);

	path.truncate(index);
	secondpath.trim_start(index);

	molog(game.get_gametime(), "splitting waterway: first part:\n");
	for (const Coords& coords : path.get_coords()) {
		molog(game.get_gametime(), "* (%i, %i)\n", coords.x, coords.y);
	}
	molog(game.get_gametime(), "                    second part:\n");
	for (const Coords& coords : secondpath.get_coords()) {
		molog(game.get_gametime(), "* (%i, %i)\n", coords.x, coords.y);
	}

	// change waterway size and reattach
	flags_[FlagEnd] = &flag;
	set_path(game, path);

	const Direction dir = get_reverse_dir(path_[path_.get_nsteps() - 1]);
	flags_[FlagEnd]->attach_road(dir, this);
	flagidx_[FlagEnd] = dir;

	// recreate waterway markings
	mark_map(game);

	// create the new waterway
	Waterway& newww = *new Waterway();
	newww.set_owner(get_owner());
	newww.flags_[FlagStart] = &flag;  //  flagidx will be set on init()
	newww.flags_[FlagEnd] = &oldend;
	newww.set_path(game, secondpath);

	// Initialize the new waterway
	newww.init(game);

	if (Ferry* ferry = ferry_.get(game)) {
		assert(ferry->get_location(game) == this);
		// We assign the ferry to the waterway part it currently is on
		bool other = true;
		const Coords pos = ferry->get_position();
		Coords temp(flags_[FlagStart]->get_position());
		for (uint32_t i = 0; i < path_.get_nsteps(); i++) {
			if (temp == pos) {
				other = false;
				break;
			}
			map.get_neighbour(temp, path_[i], &temp);
		}

		if (other) {
			molog(game.get_gametime(), "Assigning the ferry to the NEW waterway\n");
			ferry->set_destination(game, &newww);
			request_ferry(game);
		} else {
			molog(game.get_gametime(), "Assigning the ferry to the OLD waterway\n");
			ferry->set_destination(game, this);
			newww.request_ferry(game);
		}
	} else {
		// this is needed to make sure the ferry finds the way correctly
		fleet_.get(game)->reroute_ferry_request(game, this, this);
		newww.request_ferry(game);
	}

	//  Make sure wares waiting on the original endpoint flags are dealt with.
	flags_[FlagStart]->update_wares(game, &oldend);
	oldend.update_wares(game, flags_[FlagStart]);
}

void Waterway::cleanup(EditorGameBase& egbase) {
	Economy::check_split(*flags_[FlagStart], *flags_[FlagEnd], wwWARE);
	if (upcast(Game, game, &egbase)) {
		if (Ferry* ferry = ferry_.get(egbase)) {
			ferry->set_destination(*game, nullptr);
		}
		if (FerryFleet* fleet = fleet_.get(egbase)) {
			// Always call this, in case the fleet can disband now
			fleet->cancel_ferry_request(*game, this);
		}
	}
	RoadBase::cleanup(egbase);
}

void Waterway::log_general_info(const EditorGameBase& egbase) const {
	RoadBase::log_general_info(egbase);

	molog(egbase.get_gametime(), "Ferry %u\n", ferry_.get(egbase) != nullptr ? ferry_.serial() : 0);
	molog(egbase.get_gametime(), "FerryFleet %u\n",
	      fleet_.get(egbase) != nullptr ? fleet_.serial() : 0);
}

}  // namespace Widelands
