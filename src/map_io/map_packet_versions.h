/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#ifndef WL_MAP_IO_MAP_PACKET_VERSIONS_H
#define WL_MAP_IO_MAP_PACKET_VERSIONS_H

namespace Widelands {
constexpr uint8_t kCurrentPacketVersionMapObject = 2;
// Responsible for warehouses and expedition bootstraps
constexpr uint16_t kCurrentPacketVersionWarehouseAndExpedition = 8;
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_PACKET_VERSIONS_H
