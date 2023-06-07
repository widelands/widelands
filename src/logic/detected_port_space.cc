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

bool DetectedPortSpace::has_dockpoint(const Coords& c) const {
	return std::find(dockpoints.begin(), dockpoints.end(), c) != dockpoints.end();
}

std::string DetectedPortSpace::to_long_string(const EditorGameBase& egbase) const {
	std::string direction;
	switch (direction_from_portdock) {
	case CompassDir::kNorthWest:
		direction = format(_("northwest of %s"), nearest_portdock);
		break;
	case CompassDir::kNorthEast:
		direction = format(_("northeast of %s"), nearest_portdock);
		break;
	case CompassDir::kSouthWest:
		direction = format(_("southwest of %s"), nearest_portdock);
		break;
	case CompassDir::kSouthEast:
		direction = format(_("southeast of %s"), nearest_portdock);
		break;
	case CompassDir::kNorth:
		direction = format(_("north of %s"), nearest_portdock);
		break;
	case CompassDir::kSouth:
		direction = format(_("south of %s"), nearest_portdock);
		break;
	case CompassDir::kWest:
		direction = format(_("west of %s"), nearest_portdock);
		break;
	case CompassDir::kEast:
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

std::string DetectedPortSpace::to_short_string(const EditorGameBase& egbase) const {
	std::string direction;
	switch (direction_from_portdock) {
	case CompassDir::kNorthWest:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("NW %s"), nearest_portdock);
		break;
	case CompassDir::kNorthEast:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("NE %s"), nearest_portdock);
		break;
	case CompassDir::kSouthWest:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("SW %s"), nearest_portdock);
		break;
	case CompassDir::kSouthEast:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("SE %s"), nearest_portdock);
		break;
	case CompassDir::kNorth:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("N %s"), nearest_portdock);
		break;
	case CompassDir::kSouth:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("S %s"), nearest_portdock);
		break;
	case CompassDir::kWest:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("W %s"), nearest_portdock);
		break;
	case CompassDir::kEast:
		/** TRANSLATORS: This is a direction from a port. Keep this short and abbreviated. */
		direction = format(_("E %s"), nearest_portdock);
		break;
	default:
		break;
	}

	if (owner == 0) {
		if (direction.empty()) {
			/** TRANSLATORS: Placeholders are gametime and shipname. Keep this short. */
			return format(pgettext("detected_port_space", "Unowned / %1$s / %2$s"),
			              gametimestring(time_discovered.get()), discovering_ship);
		}
		/** TRANSLATORS: Placeholders are gametime, shipname, and "<direction> <port name>".
		 *  Keep this short. */
		return format(pgettext("detected_port_space", "Unowned / %1$s / %2$s / %3$s"),
		              gametimestring(time_discovered.get()), discovering_ship, direction);
	}
	if (direction.empty()) {
		/** TRANSLATORS: Placeholders are owner, gametime, and shipname. Keep this short. */
		return format(pgettext("detected_port_space", "%1$s / %2$s / %3$s"),
		              egbase.player(owner).get_name(), gametimestring(time_discovered.get()),
		              discovering_ship);
	}
	/** TRANSLATORS: Placeholders are owner, gametime, shipname, and "<direction> <port name>".
	 *  Keep this short. */
	return format(pgettext("detected_port_space", "%1$s / %2$s / %3$s / %4$s"),
	              egbase.player(owner).get_name(), gametimestring(time_discovered.get()),
	              discovering_ship, direction);
}

}  // namespace Widelands
