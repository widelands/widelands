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

	m_map = 0;
   m_objects = new Object_Manager;
   cmdqueue = new Cmd_Queue(this);

	memset(m_players, 0, sizeof(m_players));
	
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
	if (m_map)
		delete m_map;
	
	for(i = 1; i <= MAX_PLAYERS; i++)
		if (m_players[i-1])
			remove_player(i);
	
	for(i = 0; i < (int)m_tribes.size(); i++)
		delete m_tribes[i];
	m_tribes.resize(0);
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

/*
===============
Game::get_safe_ware_id

Return the corresponding ware id. Throws an exception if ware can't be found.
===============
*/
int Game::get_safe_ware_id(const char *name)
{
	int id = m_wares.get_index(name);
	if (id < 0)
		throw wexception("Ware '%s' not found", name);
	return id;
}


/*
===============
Game::set_map

Replaces the current map with the given one. Ownership of the map is transferred
to the Game object.
===============
*/
void Game::set_map(Map* map)
{
	assert(m_state <= gs_menu);

	if (m_map)
		delete m_map;
	
	m_map = map;
}


/*
===============
Game::remove_player

Remove the player with the given number
===============
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
void Game::add_player(int plnum, int type, const char* tribe, const uchar *playercolor)
{
	assert(plnum >= 1 && plnum <= MAX_PLAYERS);
	assert(m_state != gs_running);
	
	if (m_players[plnum-1])
		remove_player(plnum);

	// Get the player's tribe
	uint i;
	
	for(i = 0; i < m_tribes.size(); i++)
		if (!strcmp(m_tribes[i]->get_name(), tribe))
			break;
	
	if (i == m_tribes.size())
		m_tribes.push_back(new Tribe_Descr(tribe));
	
	m_players[plnum-1] = new Player(this, type, plnum, m_tribes[i], playercolor);
}


/** Game::can_start()
 *
 + Returns true if the game settings are valid.
 */
bool Game::can_start()
{
	int local_num;
	int i;

	if (!m_map)
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


/*
===============
Game::postload

Load and prepare detailled game data.
This happens once just after the host has started the game and before the 
graphics are loaded.
===============
*/
void Game::postload()
{
	uint id;
	int pid;

	// Postload the map
	m_map->postload(this);
	
	// Postload tribes
	id = 0;
	while(id < m_tribes.size()) {
		for(pid = 1; pid <= MAX_PLAYERS; pid++) {
			Player* plr = get_player(pid);
			
			if (plr && plr->get_tribe() == m_tribes[id])
				break;
		}
		
		if (pid <= MAX_PLAYERS) {
			// the tribe is used, postload it
			m_tribes[id]->postload(this);
			id++;
		} else {
			delete m_tribes[id]; // the tribe is no longer used, remove it
			m_tribes.erase(m_tribes.begin() + id);
		}
	}
	
	// TODO: postload players? (maybe)

	// Postload wares
	init_wares();
}


/*
===============
Game::load_graphics

Load all graphics.
This function needs to be called once at startup when the graphics system
is ready.
If the graphics system is to be replaced at runtime, the function must be
called after that has happened.
===============
*/
void Game::load_graphics()
{
	int i;

	m_map->load_graphics(); // especially loads world data
	
	// TODO: load tribe graphics (buildings, units)
	// TODO: load player graphics? (maybe)
	
	for(i = 0; i < m_wares.get_nitems(); i++)
		m_wares.get(i)->load_graphics();
	
	g_gr->load_animations();
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
		assert(m_map);

		m_state = gs_running;

		g_gr->flush(PicMod_Menu);
		
		ipl = new Interactive_Player(this, 1);

		postload();
		
		// Prepare the players (i.e. place HQs)
		for(int i = 1; i <= m_map->get_nrplayers(); i++) {
			Player* player = get_player(i);
			if (!player)
				continue;

			player->setup();

			const Coords &c = m_map->get_starting_pos(i);
			if (player->get_type() == Player::playerLocal)
				ipl->move_view_to(c.x, c.y);
		}
		
		load_graphics();
		
		ipl->run();
		
		m_objects->cleanup(this);
	   delete ipl;
		
		g_gr->flush(PicMod_Game);
		g_anim.flush();

		played = true;
	}

	m_state = gs_none;
	
	return played;
}


/*
===============
Game::init_wares

Called during postload.
Collects all wares from world and tribes and puts them into a global list
===============
*/
void Game::init_wares()
{
	World *world = m_map->get_world();
	
	world->parse_wares(&m_wares);
	
	for(int pid = 1; pid <= MAX_PLAYERS; pid++) {
		Player *plr = get_player(pid);
		if (!plr)
			continue;
		
		Tribe_Descr *tribe = plr->get_tribe();
		
		for(int i = 0; i < tribe->get_nrworkers(); i++) {
			Worker_Descr *worker = tribe->get_worker_descr(i);
			if (!worker)
				continue;
			
			int idx = m_wares.get_index(worker->get_name());
			if (idx < 0)
				idx = m_wares.add(new Worker_Ware_Descr(worker->get_name()));
			
			worker->set_ware_id(idx);
				
			Worker_Ware_Descr *descr = (Worker_Ware_Descr*)m_wares.get(idx);
			descr->add_worker(tribe, worker);
		}
	}
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
		
		g_gr->animate_maptextures(get_gametime());
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
Building *Game::warp_building(int x, int y, char owner, int idx)
{
	Building_Descr *descr;
	Player *player = get_player(owner);
	
	assert(player);
   
	descr = player->get_tribe()->get_building_descr(idx);
	assert(descr);

	return descr->create(this, get_player(owner), Coords(x, y));
}

/** Game::create_bob(int x, int y, int idx)
 *
 * Instantly create a bob at the given x/y location.
 *
 * idx is the bob type.
 */
Bob *Game::create_bob(int x, int y, int idx)
{
	Bob_Descr *descr;

	descr = m_map->get_world()->get_bob_descr(idx);
	assert(descr);
	
	return descr->create(this, 0, Coords(x, y));
}

/*
===============
Game::create_immovable

Create an immovable at the given location.
Does not perform any placability checks.
===============
*/
Immovable *Game::create_immovable(int x, int y, int idx)
{
	Immovable_Descr *descr;

	descr = m_map->get_world()->get_immovable_descr(idx);
	assert(descr);
	
	return descr->create(this, Coords(x, y));
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
	Map_Region m(coords, radius, m_map);
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
	
	recalc_for_field(coords, radius);
}


/*
===============
Game::recalc_for_field

Call this function whenever the field at fx/fy has changed in one of the ways:
 - height has changed
 - robust Map_Object has been added or removed
 
This performs the steps outlined in the comment above Map::recalc_brightness()
and recalcs the interactive player's overlay.
===============
*/
void Game::recalc_for_field(Coords coords, int radius)
{
	Map_Region_Coords mrc;
	int x, y;
	Field *f;
	
	// First pass
	mrc.init(coords, 2+radius, m_map);

	while(mrc.next(&x, &y)) {
		f = m_map->get_field(x, y);
		m_map->recalc_brightness(x, y, f);
		m_map->recalc_fieldcaps_pass1(x, y, f);
	}
	
	// Second pass
	mrc.init(coords, 2+radius, m_map);
	
	while(mrc.next(&x, &y)) {
		f = m_map->get_field(x, y);
		m_map->recalc_fieldcaps_pass2(x, y, f);
		
		if (ipl)
			ipl->recalc_overlay(FCoords(x, y, f));
	}
}

