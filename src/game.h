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

#ifndef __S__GAME_H
#define __S__GAME_H

// a frame is the time between two updates of the game logic
// this includes: AI, NETWORK, CMD_QUEUES
// this excludes: the whole User_Interface, this is updated independent from the 
//   logic frame length (more often)
#define FRAME_LENGTH 250   

/** class Player
 *
 * pure virtual class to define, what a player must be able to do
 *
 */
class Player {

};

#include "IntPlayer.h"

// TODO: NetworkPlayer
// TODO: AIPlayer

#include "counter.h"
#include "descr_maintainer.h"
#include "cmd_queue.h"

/** class Game
 *
 * This game handels one game. This class is not a all portabel,
 * it depends on nearly everything else in widelands
 */
class Game {
		  Game(const Game&);
		  Game& operator=(const Game&);

        friend class Cmd_Queue; // this class handles the commands 

		  public:
					 Game(void);
					 ~Game(void);

					 void think(void); 
                void run(void);

					 int set_map(const char* mapname);
                inline Map *get_map() { return map; }

		  private:
					 Map *map;
					 Player** pls;
					 Interactive_Player* ipl;
                Cmd_Queue* queue;
                Counter counter;
                ulong frame_count;
};




#endif // __S__GAME_H
