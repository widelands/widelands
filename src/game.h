/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "counter.h"
#include "descr_maintainer.h"
#include "instances.h"
#include "tribe.h"
#include "cmd_queue.h"

/** class Player
 *
 * Pleease... no more pure virtual nonsense.
 * What we really need is a Player class that stores e.g. score
 * and diplomacy of a player.
 *
 * These Player classes should be controlled via the cmd queue.
 * Commands are inserted by:
 *  - local player
 *  - network packets
 *  - AI code which is invoked from Game, _not_ from Player
 * So basically the Game knows whether a player is controlled
 * locally, remotely or by AI.
 *                      -- Nicolai
 */
class Player {

};

/** class Game
 *
 * This class manages the entire lifetime of a game session, from creating the
 * game and setting options, selecting maps to the actual playing phase and the
 * final statistics screen(s).
 */
enum {
	gs_none = 0,	// not connected, nothing
	gs_menu,			// in the setup menu(s)
	gs_paused,		// in-game but paused
	gs_running		// in-game
};

class Interactive_Player;

class Game {
	friend class Cmd_Queue; // this class handles the commands 

public:
	Game(void);
	~Game(void);

	inline Cmd_Queue *get_cmdqueue() { return cmdqueue; }
   inline int get_gametime(void) { return cmdqueue->get_time(); }

	// Start using logic_rand() for the actual gamelogic (e.g. critter).
	// Do NOT use for random events in the UI or other display code.
	// This will allow us to plug another PRNG in here for game playbacks
	// and other fancy stuff.
	inline int logic_rand() { return rand(); }
	
	inline const char *get_mapname() { return m_mapname; }
	void set_mapname(const char* mapname);

   inline Map *get_map() { return map; }

	void think(void);
   void run(void);
	
public:
	void warp_building(int x, int y, uchar owner, int idx);
	void create_bob(int x, int y, int idx);

private:
	int m_state;
	char* m_mapname;

	Interactive_Player *ipl;
	Tribe_Descr* tribe;
	Map *map;
	Player** pls;
   Cmd_Queue* cmdqueue;
   Instance_Handler* hinst;
   Counter counter; // used to obtain realtime

	int m_realtime; // # of ms passed since game was created (including pauses etc..)

public: // items to reconsider

   // TEMP
   Tribe_Descr* get_player_tribe(uint n) { return tribe; }
   // TEMP END
};

#endif // __S__GAME_H
