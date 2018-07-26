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
   : RoadBase(g_waterway_descr, RoadType::kWaterway, 1, 0) {
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

}  // namespace Widelands
