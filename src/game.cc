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

#include "widelands.h"
#include "cursor.h"
#include "map.h"
#include "cmd_queue.h"
#include "bob.h"
#include "ware.h"
#include "worker.h"
#include "tribe.h"
#include "game.h"
#include "player.h"
#include "ui.h"
#include "graphic.h"
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
	m_mapname = 0;

	memset(m_players, 0, sizeof(m_players));
	
   m_objects = new Object_Manager;
   cmdqueue = new Cmd_Queue(this);
   map=0;

	m_realtime = Sys_GetTime();
}

/** Game::~Game(void)
 *
 * cleanup
 */
Game::~Game(void)
{
	int i;
	
	delete m_objects;
	delete cmdqueue;
	if (map)
		delete map;
	
	for(i = 1; i <= MAX_PLAYERS; i++)
		if (m_players[i-1])
			remove_player(i);
	
	if (m_mapname)
		free(m_mapname);
}

/** void Game::set_mapname(const char *mapname)
 *
 * Set the mapname, but don't load the map yet (this is done when the
 * game starts)
 *
 * Args: mapname	name of the map
 */
void Game::set_mapname(const char* mapname)
{
	assert(!map);

	if (m_mapname)
		free(m_mapname);
	if (mapname)
		m_mapname = strdup(mapname);
	else
		m_mapname = 0;

	// Networking updates here?
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

/** Game::remove_player(int plnum)
 *
 + Remove the player with the given number
 */
void Game::remove_player(int plnum)
{
	assert(plnum >= 1 && plnum <= MAX_PLAYERS);
	assert(m_state != gs_running);
	
	if (m_players[plnum-1]) {
		delete m_players[plnum-1];
		m_players[plnum-1] = 0;
	}		
}

/*
===============
Game::add_player

Create the player structure for the given plnum.
Note that AI player structures and the Interactive_Player are created when
the game starts. Similar for remote players.
===============
*/
void Game::add_player(int plnum, int type, const uchar *playercolor)
{
	assert(plnum >= 1 && plnum <= MAX_PLAYERS);
	assert(m_state != gs_running);
	
	if (m_players[plnum-1])
		remove_player(plnum);
	
	m_players[plnum-1] = new Player(this, type, plnum, playercolor);
}

/** Game::can_start()
 *
 + Returns true if the game settings are valid.
 */
bool Game::can_start()
{
	int local_num;
	int i;

	if (!m_mapname)
		return false;
	
	// we need exactly one local player
	local_num = -1;
	for(i = 0; i < MAX_PLAYERS; i++) {
		if (!m_players[i])
			continue;
		
		if (m_players[i]->get_type() == Player::playerLocal) {
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

/** void Game::run(void)
 *
 * This runs a game, including game creation phase.
 * Returns true if a game actually took place.
 */
bool Game::run(void)
{
	bool played = false;

	m_state = gs_menu;

	if (launch_game_menu(this))
	{
		assert(m_mapname);

		m_state = gs_running;

		// TEMP
		tribe= new Tribe_Descr(); 
	   tribe->load("romans");
		// TEMP

      // Load the map
		map = new Map();
		if (RET_OK != map->load_map(m_mapname, this)) {
			critical_error("Couldn't load map.");
			m_state = gs_none;
			return false;
		}

      // TEMP: player number
		ipl = new Interactive_Player(this, 1);
	  
		// Prepare the players (i.e. place HQs)
		for(int i = 1; i <= map->get_nrplayers(); i++) {
			Player* player = get_player(i);
			if (!player)
				continue;

			player->setup();

			const Coords &c = map->get_starting_pos(i);
			if (player->get_type() == Player::playerLocal)
				ipl->move_view_to(c.x, c.y);
		}
		ipl->run();
		
		m_objects->cleanup(this);
	   delete ipl;
	   delete tribe;
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

	// Networking: check socket here

	if (m_state == gs_running)
	{
		cmdqueue->run_queue(frametime);
		
		map->get_world()->animate(get_gametime()); // update texture animation states
	}
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


/** Game::warp_building(int x, int y, char owner, int idx)
 *
 * Instantly create a building at the given x/y location. There is no build time.
 *
 * owner is the player number of the building's owner.
 * idx is the building type index.
 */
void Game::warp_building(int x, int y, char owner, int idx)
{
	Building_Descr *descr;
	Player *player = get_player(owner);
	
	assert(player);
   
	descr = player->get_tribe()->get_building_descr(idx);
	assert(descr);

	descr->create(this, get_player(owner), Coords(x, y));
}

/** Game::create_bob(int x, int y, int idx)
 *
 * Instantly create a bob at the given x/y location.
 *
 * idx is the bob type.
 */
void Game::create_bob(int x, int y, int idx)
{
	Bob_Descr *descr;

	descr = map->get_world()->get_bob_descr(idx);
	assert(descr);
	
	descr->create(this, 0, Coords(x, y));
}

/*
===============
Game::create_immovable

Create an immovable at the given location.
Does not perform any placability checks.
===============
*/
void Game::create_immovable(int x, int y, int idx)
{
	Immovable_Descr *descr;

	descr = map->get_world()->get_immovable_descr(idx);
	assert(descr);
	
	descr->create(this, Coords(x, y));
}

/*
===============
Game::conquer_area

Conquers the given area for that player.
Additionally, it updates the visible area for that player.
===============
*/
void Game::conquer_area(uchar playernr, Coords coords, int radius)
{
	Map_Region m(coords, radius, map);
	Field* f;

	while((f = m.next()))
	{
		if (f->get_owned_by() == playernr)
			continue;
		if (!f->get_owned_by()) {
			f->set_owned_by(playernr);
			continue;
		}
		
      // TODO: add support here what to do if some fields are already
      // occupied by another player
		// Probably the best thing to just don't grab it. Players should fight
		// for their land.
      //cerr << "warning: already occupied field is claimed by another user!" << endl;
   }
	
	Player *player = get_player(playernr);
	
	player->set_area_seen(coords.x, coords.y, radius+4, true);
}
