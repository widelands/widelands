/*
 * Copyright (C) 2002 by Holger Rapp 
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

#include "game.h"
#include "map.h"
#include "input.h"
#include "cursor.h"
#include "criterr.h"

/** class Game
 *
 * This game handels one game. This class is not a all portabel,
 * it depends on nearly everything else in widelands
 */

uint Game::xresolution, Game::yresolution;

/** Game::Game(void)
 *
 * init
 */
Game::Game(void) {
}


/** Game::~Game(void)
 *
 * cleanup
 */
Game::~Game(void) {

}

/** void Game::run(const char* mapname, uint nipl)
 *
 * This runs a game with the given map.
 *
 * Args:	mapname	file name of map to load
 * 		nipl	the player number of the interactive player
 * Returns: nothing
 */
#include "ui.h"
#include "fileloc.h"
#include "worldfiletypes.h"
void Game::run(const char* mapname, uint nipl) {

		  map = new Map();
		  if(map->load_map(mapname)) {
					 // TODO: make this better
					 assert(0) ;
		  }

		  ipl = new Interactive_Player(this);
		  ipl->run();
		  delete ipl;

		  delete map;
}
