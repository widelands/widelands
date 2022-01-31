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

#ifndef WL_GAME_IO_GAME_SAVER_H
#define WL_GAME_IO_GAME_SAVER_H

#include "base/wexception.h"

class FileSystem;

namespace Widelands {

class Game;

/**
 * This class writes a complete state
 * of a game out to a file. The world in ONE file
 *
 * This might eventually evolve into the same kind of saving/loading
 * structure as was used for the map stuff - but currently, there's
 * so little to save, that everything is done by this class
 */

struct GameSaver {
	GameSaver(FileSystem&, Game&);

	void save();

private:
	FileSystem& fs_;
	Game& game_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_GAME_IO_GAME_SAVER_H
