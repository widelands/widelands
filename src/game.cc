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

#include <assert.h>
#include "game.h"
#include "map.h"


/** class Game
 *
 * This game handels one game. This class is not a all portabel, 
 * it depends on nearly everything else in widelands
 */

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

/** void Game::run(const char* map, uint nipl) 
 *
 * This runs a game with the given map. 
 *
 * Args:	map	file name of map to load
 * 		nipl	the player number of the interactive player
 * Returns: nothing
 */
#include "ui.h"
void Game::run(const char* map, uint nipl) {	
		  Map* c;
		  Interactive_Player* ip;
		 
		  // TEMP
		  g_ui.create_window(0, 0, 640, 480, Window::FLAT);
		  
		  c=new Map();
		  if(c->load_map(map)) {
					 // TODO: make this better
					 assert(0) ;
		  }

		  ip=new Interactive_Player(c);

		  while(!ip->should_end_game()) {
					 ip->interact();
		  }

		
		  delete ip;
		  delete c;
}
