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

#include "IntPlayer.h"
#include "CompPlayer.h"
#include "cmd_queue.h"
#include "fullscreen_menu_launchgame.h"
#include "game.h"
#include "graphic.h"
#include "player.h"
#include "system.h"
#include "map_loader.h"
#include "playercommand.h"
#include "trigger.h"
#include "network.h"


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


bool Game::run_single_player ()
{
	m_state = gs_menu;
	m_netgame=0;

	m_maploader=0;
	Fullscreen_Menu_LaunchGame *lgm = new Fullscreen_Menu_LaunchGame(this, 0, &m_maploader);
	int code = lgm->run();
	delete lgm;
	
	if (code==0 || get_map()==0)
	    return false;

	g_gr->flush(PicMod_Menu);

	m_state = gs_running;

	init_player_controllers ();

	// Now first, completly load the map
	m_maploader->load_map_complete(this, code==2); // if code==2 is a scenario
	delete m_maploader;
	m_maploader=0;

	return run();
}


//extern uchar g_playercolors[MAX_PLAYERS][12];
bool Game::run_multi_player (NetGame* ng)
{
	m_state = gs_menu;
	m_netgame=ng;

	m_maploader=0;
	Fullscreen_Menu_LaunchGame *lgm = new Fullscreen_Menu_LaunchGame(this, m_netgame, &m_maploader);
	m_netgame->set_launch_menu (lgm);
	int code = lgm->run();
	m_netgame->set_launch_menu (0);
	delete lgm;
	
	if (code==0 || get_map()==0)
	    return false;

	m_netgame->begin_game();
	    
	g_gr->flush(PicMod_Menu);

	m_state = gs_running;
	
	init_player_controllers ();

	// Now first, completly load the map
	m_maploader->load_map_complete(this, false); // if code==2 is a scenario
	delete m_maploader;
	m_maploader=0;

	return run();
}


void Game::load_map (const char* filename)
{
	Map* map=new Map();
	m_maploader = map->get_correct_loader(filename);
	assert (m_maploader!=0);
	m_maploader->preload_map(0);
	set_map (m_maploader->get_map());
}


void Game::init_player_controllers ()
{
	ipl=0;
	for (int i=1; i<=get_map()->get_nrplayers(); i++)
		if (get_player(i)->get_type()==Player::playerLocal) {
		    ipl = new Interactive_Player(this, i);
		    break;
		}
	
	assert (ipl!=0);

	// inform base, that we have something interactive
	set_iabase(ipl);

	// set up computer controlled players
	for (int i=1; i<=get_map()->get_nrplayers(); i++)
		if (get_player(i)->get_type()==Player::playerAI)
			cpl.push_back (new Computer_Player(this,i));

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
bool Game::run()
{
	postload();

	// Prepare the players (i.e. place HQs)
	for (int i = 1; i <= get_map()->get_nrplayers(); i++) {
		Player* player = get_player(i);
		if (!player)
			continue;

		player->init_for_game(this);

		const Coords &c = get_map()->get_starting_pos(i);
		if (player->get_type() == Player::playerLocal)
			ipl->move_view_to(c.x, c.y);
	}

	// Prepare the map, set default textures
	get_map()->recalc_default_resources();
	get_map()->delete_unreferenced_triggers();
	get_map()->delete_events_without_trigger();

	// Now let all triggers check once, if they are in the right state
	for (int i=0; i<get_map()->get_number_of_triggers(); i++)
		get_map()->get_trigger(i)->check_set_conditions(this);

	load_graphics();

	// Everything prepared, send the first trigger event
	// We lie about the sender here. Hey, what is one lie in a lifetime?
	enqueue_command (new Cmd_CheckTrigger(get_gametime(), -1));

	ipl->run();

	get_objects()->cleanup(this);
	delete ipl;

	for (unsigned int i=0; i<cpl.size(); i++)
		delete cpl[i];

	g_gr->flush(PicMod_Game);
	g_anim.flush();

	m_state = gs_none;

	return true;
}



//
// think() is called by the UI objects initiated during Game::run()
// during their modal loop.
// Depending on the current state we advance game logic and stuff,
// running the cmd queue etc.
//
void Game::think(void)
{
	if (m_netgame!=0)
	    m_netgame->handle_network ();

	if (m_state == gs_running) {
		for (unsigned int i=0;i<cpl.size();i++)
			cpl[i]->think();
	
		int frametime = -m_realtime;
		m_realtime = Sys_GetTime();
		frametime += m_realtime;
		
		if (m_netgame!=0) {
			int max_frametime=m_netgame->get_max_frametime();
			
			if (frametime>max_frametime)
			    frametime=max_frametime;	// wait for the next server message
			else if (max_frametime-frametime>500)
			    frametime+=(max_frametime-frametime)/2;	// we are too long behind network time, so hurry a little
		}
		else
			frametime *= get_speed();
		    
		// maybe we are too fast...
		if (frametime==0)
			return;

		// prevent frametime escalation in case the game logic is the performance bottleneck
		if (frametime > 1000)
			frametime = 1000;

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


void Game::player_immovable_notification (PlayerImmovable* pi, losegain_t lg)
{
	for (unsigned int i=0;i<cpl.size();i++)
		if (cpl[i]->get_player_number()==pi->get_owner()->get_player_number())
			if (lg==GAIN)
				cpl[i]->gain_immovable (pi);
			else
				cpl[i]->lose_immovable (pi);
}

/*
===============
Game::send_player_command

All player-issued commands must enter the queue through this function.
It takes the appropriate action, i.e. either add to the cmd_queue or send
across the network.
===============
*/
void Game::send_player_command (PlayerCommand* pc)
{
	if (m_netgame!=0 && get_player(pc->get_sender())->get_type()==Player::playerLocal)
		m_netgame->send_player_command (pc);
	else
		enqueue_command (pc);
}

void Game::enqueue_command (BaseCommand* cmd)
{
	cmdqueue->enqueue (cmd);
}

// we might want to make these inlines:
void Game::send_player_bulldoze (PlayerImmovable* pi)
{
	send_player_command (new Cmd_Bulldoze(get_gametime(), pi->get_owner()->get_player_number(), pi));
}

void Game::send_player_build (int pid, const Coords& coords, int id)
{
	send_player_command (new Cmd_Build(get_gametime(), pid, coords, id));
}

void Game::send_player_build_flag (int pid, const Coords& coords)
{
	send_player_command (new Cmd_BuildFlag(get_gametime(), pid, coords));
}

void Game::send_player_build_road (int pid, Path* path)
{
	send_player_command (new Cmd_BuildRoad(get_gametime(), pid, path));
}

void Game::send_player_flagaction (Flag* flag, int action)
{
	send_player_command (new Cmd_FlagAction(get_gametime(), flag->get_owner()->get_player_number(), flag, action));
}

void Game::send_player_start_stop_building (Building* b)
{
	send_player_command (new Cmd_StartStopBuilding(get_gametime(), b->get_owner()->get_player_number(), b));
}

void Game::send_player_enhance_building (Building* b, int id)
{
	assert(id!=-1);

	send_player_command (new Cmd_EnhanceBuilding(get_gametime(), b->get_owner()->get_player_number(), b, id));
}

