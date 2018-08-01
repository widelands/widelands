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

#include "economy/waterway.h"

#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/fleet.h"
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
const WaterwayDescr g_waterway_descr("waterway", "Waterway");
}

bool Waterway::is_waterway_descr(MapObjectDescr const* const descr) {
	return descr == &g_waterway_descr;
}

/**
 * Most of the actual work is done in init.
 */
Waterway::Waterway()
   : RoadBase(g_waterway_descr, RoadType::kWaterway), ferry_(nullptr), fleet_(nullptr) {
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

void Waterway::link_into_flags(EditorGameBase& egbase) {
	RoadBase::link_into_flags(egbase);
	if (upcast(Game, game, &egbase))
		request_ferry();
}

bool Waterway::notify_ware(Game& game, FlagId const flagid) {
	if (ferry_)
		if (ferry_->notify_ware(game, flagid))
			return true;
	return false;
}

void Waterway::remove_worker(Worker& w) {
	if (ferry_ != &w)
		return;

	ferry_ = nullptr;
	request_ferry();

	PlayerImmovable::remove_worker(w);
}

void Waterway::request_ferry() {
	Fleet* fleet = new Fleet(get_owner());
	fleet->request_ferry(this);
	fleet->init(get_owner()->egbase());
}

void Waterway::request_ferry_callback(Game& game, Ferry* f) {
	ferry_ = f;
	ferry_->set_economy(game, get_economy());
	ferry_->set_location(this);
	ferry_->start_task_row(game, this);
}

void Waterway::assign_carrier(Carrier& c, uint8_t) {
	ferry_->set_location(nullptr);
	ferry_ = &dynamic_cast<Ferry&>(c);
}

Fleet* Waterway::get_fleet() const {
	return fleet_;
}

void Waterway::set_fleet(Fleet* fleet) {
	fleet_ = fleet;
}

}  // namespace Widelands
