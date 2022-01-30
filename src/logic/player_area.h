/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_PLAYER_AREA_H
#define WL_LOGIC_PLAYER_AREA_H

#include "logic/widelands_geometry.h"

namespace Widelands {

template <typename AreaType = Area<>> struct PlayerArea : public AreaType {
	PlayerArea() : player_number(0) {
	}
	PlayerArea(const PlayerNumber pn, const AreaType area) : AreaType(area), player_number(pn) {
	}
	PlayerNumber player_number;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PLAYER_AREA_H
