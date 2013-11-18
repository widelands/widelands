/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef GAME_LOADER_H
#define GAME_LOADER_H

#include <string>

#include <stdint.h>

class FileSystem;

namespace Widelands {

class Game;
struct Game_Preload_Data_Packet;

/*
 * This class reads a complete state
 * of a game out to a file.
 */
struct Game_Loader {
	Game_Loader(const std::string & path, Game &);
	~Game_Loader();

	int32_t preload_game(Game_Preload_Data_Packet &);
	int32_t    load_game(bool multiplayer = false);

private:
	FileSystem & m_fs;
	Game       & m_game;
};

}

#endif
