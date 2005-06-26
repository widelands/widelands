/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __S__GAME_LOADER_H
#define __S__GAME_LOADER_H

#include <string>
#include "wexception.h"

class Game;
class FileSystem;
class Widelands_Map_Map_Object_Loader;

class Game_Preload_Data_Packet;

/*
 * This class reads a complete state
 * of a game out to a file. 
 */
class Game_Loader {
   public:
      Game_Loader(FileSystem*, Game* game);
      ~Game_Loader(void);

      int preload_game(Game_Preload_Data_Packet*);
      int load_game(void);
     
   private:
      FileSystem* m_fs;
      Game* m_game;
};

#endif
