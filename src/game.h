/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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
class Computer_Player;
class Map_Loader;
class BaseCommand;
class PlayerCommand;
class NetGame;

class Game : public Editor_Game_Base {
	friend class Cmd_Queue; // this class handles the commands

public:
	Game(void);
	~Game(void);

	// life cycle
	bool run_single_player ();
	bool run_multi_player (NetGame*);

	void load_map (const char*);

	void think(void);


	bool can_start();
	
	// in-game logic
	inline Cmd_Queue *get_cmdqueue() { return cmdqueue; }

	// Start using logic_rand() for the actual gamelogic (e.g. critter).
	// Do NOT use for random events in the UI or other display code.
	// This will allow us to plug another PRNG in here for game playbacks
	// and other fancy stuff.
	inline int logic_rand() { return rand(); }

	int get_speed() const { return m_speed; }
	void set_speed(int speed);

	bool get_allow_cheats();
	
	virtual void player_immovable_notification (PlayerImmovable*, losegain_t);
	virtual void player_field_notification (const FCoords&, losegain_t);

	void enqueue_command (BaseCommand*);

	void send_player_command (PlayerCommand*);

	void send_player_bulldoze (PlayerImmovable*);
	void send_player_build (int, const Coords&, int);
	void send_player_build_flag (int, const Coords&);
	void send_player_build_road (int, Path*);
	void send_player_flagaction (Flag*, int);
	void send_player_start_stop_building (Building*);
	void send_player_enhance_building (Building*, int);

	// is this base a game
	inline bool is_game() { return true; }

private:
	void init_player_controllers ();
	bool run ();

	Map_Loader*	m_maploader;
	
	NetGame*	m_netgame;
	
	int		m_state;
	int		m_speed;		// frametime multiplier

	Interactive_Player*			ipl;
	std::vector<Computer_Player*>		cpl;
 	Cmd_Queue*				cmdqueue;

	int m_realtime; // the real time (including) pauses in milliseconds
};

#endif // __S__GAME_H
