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

#ifndef WL_LOGIC_DETECTED_PORT_SPACE_H
#define WL_LOGIC_DETECTED_PORT_SPACE_H

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/times.h"
#include "logic/map_compassdir.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {
class EditorGameBase;

struct DetectedPortSpace {
	DetectedPortSpace();
	static void initialize_serial();
	void set_serial(Serial s);

	Serial serial;
	Coords coords;
	std::vector<Coords> dockpoints;
	PlayerNumber owner;
	Time time_discovered;
	std::string discovering_ship;
	std::string nearest_portdock;
	CompassDir direction_from_portdock;

	[[nodiscard]] bool has_dockpoint(const Coords& c) const;
	[[nodiscard]] std::string to_short_string(const EditorGameBase& egbase) const;
	[[nodiscard]] std::string to_long_string(const EditorGameBase& egbase) const;

	DISALLOW_COPY_AND_ASSIGN(DetectedPortSpace);
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_DETECTED_PORT_SPACE_H
