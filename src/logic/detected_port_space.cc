/*
 * Copyright (C) 2023-2024 by the Widelands Development Team
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
		/** TRANSLATORS: "12 steps northwest of Port1" */
		direction = _("northwest");
		break;
	case CompassDir::kNorthEast:
		/** TRANSLATORS: "12 steps northeast of Port1" */
		direction = _("northeast");
		break;
	case CompassDir::kSouthWest:
		/** TRANSLATORS: "12 steps southwest of Port1" */
		direction = _("southwest");
		break;
	case CompassDir::kSouthEast:
		/** TRANSLATORS: "12 steps southwest of Port1" */
		direction = _("southeast");
		break;
	case CompassDir::kNorth:
		/** TRANSLATORS: "12 steps north of Port1" */
		direction = _("north");
		break;
	case CompassDir::kSouth:
		/** TRANSLATORS: "12 steps south of Port1" */
		direction = _("south");
		break;
	case CompassDir::kWest:
		/** TRANSLATORS: "12 steps west of Port1" */
		direction = _("west");
		break;
	case CompassDir::kEast:
		/** TRANSLATORS: "12 steps east of Port1" */
		direction = _("east");
		break;
	default:
		break;
	}

	if (owner == 0) {
		if (direction.empty()) {
			return format(_("Unowned port space discovered at %1$s by %2$s"),
			              gametimestring(time_discovered.get()), discovering_ship);
		}
		return format(
		   /** TRANSLATORS: "Unowned port space discovered at 1:11 by Ship2 33 steps north of
		       Port5" */
		   ngettext("Unowned port space discovered at %1$s by %2$s %3$u step %4$s of %5$s",
		            "Unowned port space discovered at %1$s by %2$s %3$u steps %4$s of %5$s",
		            distance_to_portdock),
		   gametimestring(time_discovered.get()), discovering_ship, distance_to_portdock, direction,
		   nearest_portdock);
	}
	if (direction.empty()) {
		return format(_("Port space of %1$s discovered at %2$s by %3$s"),
		              egbase.player(owner).get_name(), gametimestring(time_discovered.get()),
		              discovering_ship);
	}
	return format(
	   /** TRANSLATORS: "Port space of Player1 discovered at 2:22 by Ship3 44 steps north
	       of Port6" */
	   ngettext("Port space of %1$s discovered at %2$s by %3$s %4$u step %5$s of %6$s",
	            "Port space of %1$s discovered at %2$s by %3$s %4$u steps %5$s of %6$s",
	            distance_to_portdock),
	   egbase.player(owner).get_name(), gametimestring(time_discovered.get()), discovering_ship,
	   distance_to_portdock, direction, nearest_portdock);
}

std::string DetectedPortSpace::to_short_string(const EditorGameBase& egbase) const {
	std::string direction;
	switch (direction_from_portdock) {
	case CompassDir::kNorthWest:
		/** TRANSLATORS: This is an abbreviated direction: Northwest. */
		direction = pgettext("direction", "NW");
		break;
	case CompassDir::kNorthEast:
		/** TRANSLATORS: This is an abbreviated direction: Northeast. */
		direction = pgettext("direction", "NE");
		break;
	case CompassDir::kSouthWest:
		/** TRANSLATORS: This is an abbreviated direction: Southwest. */
		direction = pgettext("direction", "SW");
		break;
	case CompassDir::kSouthEast:
		/** TRANSLATORS: This is an abbreviated direction: Southeast. */
		direction = pgettext("direction", "SE");
		break;
	case CompassDir::kNorth:
		/** TRANSLATORS: This is an abbreviated direction: North. */
		direction = pgettext("direction", "N");
		break;
	case CompassDir::kSouth:
		/** TRANSLATORS: This is an abbreviated direction: South. */
		direction = pgettext("direction", "S");
		break;
	case CompassDir::kWest:
		/** TRANSLATORS: This is an abbreviated direction: West. */
		direction = pgettext("direction", "W");
		break;
	case CompassDir::kEast:
		/** TRANSLATORS: This is an abbreviated direction: East. */
		direction = pgettext("direction", "E");
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
		/** TRANSLATORS: Placeholders are gametime, shipname, and distance direction port_name.
		 *  Keep this short. */
		return format(pgettext("detected_port_space", "Unowned / %1$s / %2$s / %3$u %4$s %5$s"),
		              gametimestring(time_discovered.get()), discovering_ship, distance_to_portdock,
		              direction, nearest_portdock);
	}
	if (direction.empty()) {
		/** TRANSLATORS: Placeholders are owner, gametime, and shipname. Keep this short. */
		return format(pgettext("detected_port_space", "%1$s / %2$s / %3$s"),
		              egbase.player(owner).get_name(), gametimestring(time_discovered.get()),
		              discovering_ship);
	}
	/** TRANSLATORS: Placeholders are owner, gametime, shipname, and "distance direction port_name.
	 *  Keep this short. */
	return format(pgettext("detected_port_space", "%1$s / %2$s / %3$s / %4$u %5$s %6$s"),
	              egbase.player(owner).get_name(), gametimestring(time_discovered.get()),
	              discovering_ship, distance_to_portdock, direction, nearest_portdock);
}

}  // namespace Widelands
