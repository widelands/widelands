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

#include "descr_maintainer.h"
#include "instances.h"
#include "tribe.h"
#include "cmd_queue.h"
#include "ware.h"
#include "editor_game_base.h"

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

class Player;
class Interactive_Player;

class Game : public Editor_Game_Base {
	friend class Cmd_Queue; // this class handles the commands 

public:
	Game(void);
	~Game(void);

	// life cycle
   bool run(void);
	void think(void);
	

	bool can_start();
	
	// in-game logic
	inline Cmd_Queue *get_cmdqueue() { return cmdqueue; }

	// Start using logic_rand() for the actual gamelogic (e.g. critter).
	// Do NOT use for random events in the UI or other display code.
	// This will allow us to plug another PRNG in here for game playbacks
	// and other fancy stuff.
	inline int logic_rand() { return rand(); }
	
	bool get_allow_cheats();
	

	void send_player_command(int pid, int cmd, int arg1=0, int arg2=0, int arg3=0);

  // is this base a game
   inline bool is_game() { return true; }
   
   // TEMP
   inline Interactive_Player* get_ipl(void) { return ipl; } 
	
private:
	int		m_state;

	Interactive_Player*			ipl;
 	Cmd_Queue*						cmdqueue;

	int m_realtime; // the real time (including) pauses in milliseconds
};

#endif // __S__GAME_H
