/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include "widelands.h"
#include "graphic.h"
#include "ui.h"
#include "game.h"
#include "map.h"
#include "cmd_queue.h"
#include "bob.h"
#include "ware.h"
#include "worker.h"
#include "tribe.h"
#include "player.h"
#include "mapselectmenue.h"
#include "IntPlayer.h"
#include "player.h"
#include "building.h"


/** Game::Game(void)
 *
 * init
 */
Game::Game(void)
{
	m_state = gs_none;
	m_speed = 1;

   cmdqueue = new Cmd_Queue(this);
   
	m_realtime = Sys_GetTime();
}

/** Game::~Game(void)
 *
 * cleanup
 */
Game::~Game(void)
{
   delete cmdqueue;
}


/*
===============
Game::get_allow_cheats

Returns true if cheat codes have been activated (single-player only)
===============
*/
bool Game::get_allow_cheats()
{
	return true;
}

/** Game::can_start()
 *
 + Returns true if the game settings are valid.
 */
bool Game::can_start()
{
	int local_num;
	int i;

	if (!get_map())
		return false;
	
	// we need exactly one local player
	local_num = -1;
	for(i = 1; i <= MAX_PLAYERS; i++) {
		if (!get_player(i))
			continue;
		
		if (get_player(i)->get_type() == Player::playerLocal) {
			if (local_num < 0)
				local_num = i;
			else
				return false;
		}
	}
	if (local_num < 0)
		return false;
	
	return true;
}

/*
===============
void Game::run(void)

This runs a game, including game creation phase.
Returns true if a game actually took place.

The setup and loading of a game happens (or rather: will happen) in three
stages.
1. First of all, the host (or single player) configures the game. During this
   time, only short descriptions of the game data (such as map headers )are
	loaded to minimize loading times.
2. Once the game is about to start and the configuration screen is finished,
   all logic data (map, tribe information, building information) is loaded
	during postload.
2b. If a game is created, initial player positions are set. This step is
    skipped when a game is loaded.
3. After this has happened, the game graphics are loaded.
===============
*/
bool Game::run(void)
{
	bool played = false;

	m_state = gs_menu;

	if (launch_game_menu(this))
	{
		assert(get_map());

		m_state = gs_running;

		g_gr->flush(PicMod_Menu);

		ipl = new Interactive_Player(this, 1);
      // inform base, that we have something interactive
      set_iabase(ipl);

		postload();

		// Prepare the players (i.e. place HQs)
		for(int i = 1; i <= get_map()->get_nrplayers(); i++) {
			Player* player = get_player(i);
			if (!player)
				continue;

			player->init_for_game(this);

			const Coords &c = get_map()->get_starting_pos(i);
			if (player->get_type() == Player::playerLocal)
				ipl->move_view_to(c.x, c.y);
		}

		load_graphics();

		ipl->run();

		get_objects()->cleanup(this);
	   delete ipl;

		g_gr->flush(PicMod_Game);
		g_anim.flush();

		played = true;
	}

	m_state = gs_none;

	return played;
}



//
// think() is called by the UI objects initiated during Game::run()
// during their modal loop.
// Depending on the current state we advance game logic and stuff,
// running the cmd queue etc.
// 
// think(), mmh, i don't know if i like the name
//
void Game::think(void)
{
	int lasttime = m_realtime;
	int frametime;
	m_realtime = Sys_GetTime();
	frametime = m_realtime - lasttime;

	// prevent frametime escalation in case the game logic is the performance bottleneck
	if (frametime > 1000)
		frametime = 1000;

	frametime *= get_speed();

	// Networking: check socket here

	if (m_state == gs_running)
	{
      cmdqueue->run_queue(frametime, get_game_time_pointer());

		g_gr->animate_maptextures(get_gametime());
   }
}


/*
===============
Game::set_speed

Change the game speed.
===============
*/
void Game::set_speed(int speed)
{
	assert(speed >= 0);

	m_speed = speed;
}


/*
===============
Game::send_player_command

All player-issued commands must enter the queue through this function.
It takes the appropriate action, i.e. either add to the cmd_queue or send
across the network.
===============
*/
void Game::send_player_command(int pid, int cmd, int arg1, int arg2, int arg3)
{
	cmdqueue->queue(get_gametime(), pid, cmd, arg1, arg2, arg3);
}



