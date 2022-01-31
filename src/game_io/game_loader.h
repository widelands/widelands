/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_GAME_IO_GAME_LOADER_H
#define WL_GAME_IO_GAME_LOADER_H

#include <string>

class FileSystem;

namespace Widelands {

class Game;
struct GamePreloadPacket;

/*
 * This class reads a complete state
 * of a game out to a file.
 */
struct GameLoader {
	GameLoader(const std::string& path, Game&);
	~GameLoader();

	int32_t preload_game(GamePreloadPacket&);
	int32_t load_game(bool multiplayer = false);

private:
	FileSystem& fs_;
	Game& game_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_GAME_IO_GAME_LOADER_H
