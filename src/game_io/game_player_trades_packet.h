/*
 * Copyright (C) 2024-2025 by the Widelands Development Team
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

#ifndef WL_GAME_IO_GAME_PLAYER_TRADES_PACKET_H
#define WL_GAME_IO_GAME_PLAYER_TRADES_PACKET_H

#include "game_io/game_data_packet.h"

namespace Widelands {

/** Saves all the trade agreements and proposals. */
struct GamePlayerTradesPacket : public GameDataPacket {
	void read(FileSystem&, Game&, MapObjectLoader* = nullptr) override;
	void write(FileSystem&, Game&, MapObjectSaver* = nullptr) override;
};
}  // namespace Widelands

#endif  // end of include guard: WL_GAME_IO_GAME_PLAYER_TRADES_PACKET_H
