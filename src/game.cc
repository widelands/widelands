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
#include "descr_maintainer.h"
#include "bob.h"
#include "ware.h"
#include "worker.h"
#include "tribe.h"
#include "game.h"
#include "ui.h"
#include "fileloc.h"
#include "myfile.h"
#include "graphic.h"
#include "mapselectmenue.h"
#include "IntPlayer.h"



/** Game::Game(void)
 *
 * init
 */
Game::Game(void)
{
	m_state = gs_none;
	m_mapname = 0;

   hinst = new Instance_Handler(MAX_OBJS);
   cmdqueue = new Cmd_Queue(this);
   map=0;

	m_realtime = 0;
}

/** Game::~Game(void)
 *
 * cleanup
 */
Game::~Game(void) {
   delete hinst; 
	delete cmdqueue;
	if (map)
		delete map;
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

/** void Game::run(void)
 *
 * This runs a game 
 */
void Game::run(void)
{
   counter.start();

	m_state = gs_menu;

	if (launch_game_menu(this))
	{
		assert(m_mapname);

		m_state = gs_running;

		// TEMP
		tribe= new Tribe_Descr(); 
	   const char* str=g_fileloc.locate_file("testtribe.wtf", TYPE_TRIBE);
		assert(str);
	   if(tribe->load(str)) {
		   assert(0);
	   }
		// TEMP

		// Load the map
		map = new Map();
      if (RET_OK != map->load_map(m_mapname, this)) {
			critical_error("Couldn't load map.");
			return;
		}

	   ipl = new Interactive_Player(this);
	   ipl->run();
	   delete ipl;
		delete map;
		map = 0;
	   delete tribe;
	}

	m_state = gs_none;
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
	m_realtime = counter.get_ticks();
	frametime = m_realtime - lasttime;

	// Networking: check socket here

	if (m_state == gs_running)
	{
		cmdqueue->run_queue(frametime);
		
		map->get_world()->animate(get_gametime()); // update texture animation states
	}
}

/** Game::warp_building(int x, int y, uchar owner, int idx)
 *
 * Instantly create a building at the given x/y location. There is no build time.
 *
 * owner is the player number of the building's owner.
 * idx is the building type index.
 */
void Game::warp_building(int x, int y, uchar owner, int idx)
{
	Instance *inst;
	Building_Descr *descr;
	int i;

	i = hinst->get_free_inst_id();
	inst = hinst->get_inst(i);

	descr = get_player_tribe(owner)->get_building_descr(idx);

	inst->create(this, descr);
	inst->hook_field(x, y, map->get_field(x, y));
	inst->set_owned_by(owner);
}

/** Game::create_bob(int x, int y, int idx)
 *
 * Instantly create a bob at the given x/y location.
 *
 * idx is the bob type.
 */
void Game::create_bob(int x, int y, int idx)
{
	Instance *inst;
	Logic_Bob_Descr *descr;
	int i;
            
	i = hinst->get_free_inst_id();
	inst = hinst->get_inst(i);
	
	descr = map->get_world()->get_bob_descr(idx);
	
	inst->create(this, descr);
	inst->hook_field(x, y, map->get_field(x, y));
}

