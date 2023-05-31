/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#include "logic/detected_port_space.h"

#include <algorithm>

#include "base/time_string.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"

namespace Widelands {

static Serial last_serial(0U);

DetectedPortSpace::DetectedPortSpace() : serial(++last_serial) {
}

void DetectedPortSpace::initialize_serial() {
	last_serial = 0;
}

void DetectedPortSpace::set_serial(Serial s) {
	serial = s;
	last_serial = std::max(last_serial, s);
}

bool DetectedPortSpace::has_dockpoint(const Coords& coords) const {
	return std::find(dockpoints.begin(), dockpoints.end(), coords) != dockpoints.end();
}

std::string DetectedPortSpace::to_string(const EditorGameBase& egbase) const {
	std::string direction;
	switch (direction_from_portdock) {
	case WALK_NW:
		direction = format(_("northwest of %s"), nearest_portdock);
		break;
	case WALK_NE:
		direction = format(_("northeast of %s"), nearest_portdock);
		break;
	case WALK_SW:
		direction = format(_("southwest of %s"), nearest_portdock);
		break;
	case WALK_SE:
		direction = format(_("southeast of %s"), nearest_portdock);
		break;
	case WALK_W:
		direction = format(_("west of %s"), nearest_portdock);
		break;
	case WALK_E:
		direction = format(_("east of %s"), nearest_portdock);
		break;
	default:
		break;
	}

	if (owner == 0) {
		if (direction.empty()) {
			return format(_("Unowned port space discovered at %1$s by %2$s"),
			              gametimestring(time_discovered.get()), discovering_ship);
		}
		/** TRANSLATORS: Last placeholder is "northwest/southeast/... of <port name>" */
		return format(_("Unowned port space discovered at %1$s by %2$s %3$s"),
		              gametimestring(time_discovered.get()), discovering_ship, direction);
	}
	if (direction.empty()) {
		return format(_("Port space of %1$s discovered at %2$s by %3$s"),
		              egbase.player(owner).get_name(), gametimestring(time_discovered.get()),
		              discovering_ship);
	}
	/** TRANSLATORS: Last placeholder is "northwest/southeast/... of <port name>" */
	return format(_("Port space of %1$s discovered at %2$s by %3$s %4$s"),
	              egbase.player(owner).get_name(), gametimestring(time_discovered.get()),
	              discovering_ship, direction);
}

}  // namespace Widelands
