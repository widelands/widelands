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

#include "interactive_player.h"
#include "computer_player.h"
#include "cmd_queue.h"
#include "fullscreen_menu_launchgame.h"
#include "fullscreen_menu_loadgame.h"
#include "game.h"
#include "game_loader.h"
#include "graphic.h"
#include "player.h"
#include "soldier.h"
#include "system.h"
#include "tribe.h"
#include "map_loader.h"
#include "playercommand.h"
#include "trigger.h"
#include "network.h"
#include "widelands_map_loader.h"


/** Game::Game(void)
 *
 * init
 */
Game::Game(void)
{
	m_state = gs_none;
	m_speed = 1;
	
	rng = new RNG();

	cmdqueue = new Cmd_Queue(this);

	m_realtime = Sys_GetTime();

	ipl = 0;
}

/** Game::~Game(void)
 *
 * cleanup
 */
Game::~Game(void)
{
	delete cmdqueue;
	delete rng;
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

bool Game::run_splayer_map_direct(const char* mapname, bool scenario) {
   m_netgame = 0;
   
   assert(!get_map());

   Map *map = new Map();
   set_map(map);
   
   m_maploader = new Widelands_Map_Loader(mapname, map);
   m_maploader->preload_map(scenario);
   
   m_state = gs_running;

   // We have to create the players here
   for( uint i = 1; i <= map->get_nrplayers(); i++) 
      add_player(i, i==1 ? Player::playerLocal : Player::playerAI, 
            map->get_scenario_player_tribe(i).c_str(), map->get_scenario_player_name(i).c_str());

   init_player_controllers ();

	m_maploader->load_map_complete(this, scenario); // if code==2 is a scenario
	delete m_maploader;
	m_maploader=0;

	return run();
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

/*
 * Load a game
 * argument defines if this is a single player game (true) 
 * or networked (false)
 */
bool Game::run_load_game(bool is_splayer) {
   assert(is_splayer); // TODO: net game saving not supported
   
   Fullscreen_Menu_LoadGame* ssg = new Fullscreen_Menu_LoadGame(this, true);
   int code = ssg->run();

   if(code) {
   // We have to create an empty map, otherwise nothing will load properly
   Map* map = new Map;
   set_map(map);
   
   Game_Loader* gl=new Game_Loader(ssg->get_gamename(), this);
   gl->load_game();
   delete gl;

 
   }
   
   delete ssg;

   if(code==0) 
      return false;

   m_state = gs_running;

   make_influence_map ();  // This is needed to call after load a map :)
   
   return run(true);
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
	    
	g_gr->flush(PicMod_Menu);

	m_state = gs_running;
	
	init_player_controllers ();

	// Now first, completly load the map
	m_maploader->load_map_complete(this, false); // if code==2 is a scenario
	delete m_maploader;
	m_maploader=0;

	m_netgame->begin_game();

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
		if (get_player(i)!=0 && get_player(i)->get_type()==Player::playerLocal) {
		    ipl = new Interactive_Player(this, i);
		    break;
		}
	
	assert (ipl!=0);

	// inform base, that we have something interactive
	set_iabase(ipl);

	// set up computer controlled players
	for (int i=1; i<=get_map()->get_nrplayers(); i++)
		if (get_player(i)!=0 && get_player(i)->get_type()==Player::playerAI)
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
bool Game::run(bool is_savegame)
{
   postload();
   
   if(!is_savegame) {
      // Prepare the players (i.e. place HQs)
      for (int i = 1; i <= get_map()->get_nrplayers(); i++) {
         Player* player = get_player(i);
         if (!player)
            continue;

         player->init(this, true);

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

      // Finally, set the scenario names and tribes to represent
      // the correct names of the players
      int curplr;
      for(curplr=1; curplr <= get_map()->get_nrplayers(); curplr++) {
         Player* plr=get_player(curplr);

         if(plr) { 
            get_map()->set_scenario_player_tribe(curplr, plr->get_tribe()->get_name());
            get_map()->set_scenario_player_name(curplr, plr->get_name()); 
         } else {
            get_map()->set_scenario_player_tribe(curplr, "");
            get_map()->set_scenario_player_name(curplr, "");
         }
      }



      // Everything prepared, send the first trigger event
      // We lie about the sender here. Hey, what is one lie in a lifetime?
      enqueue_command (new Cmd_CheckTrigger(get_gametime(), -1));
   } 
   
   load_graphics();

   sound_handler->change_music("ingame", 1000, 0);

	ipl->run();
	
	sound_handler->change_music("menu", 1000, 0);

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
      
   if(get_ipl()->get_player_number()==pi->get_owner()->get_player_number())
      if (lg==GAIN)
         get_ipl()->gain_immovable (pi);
      else
         get_ipl()->lose_immovable (pi);
}

void Game::player_field_notification (const FCoords& fc, losegain_t lg)
{
	for (unsigned int i=0;i<cpl.size();i++)
		if (cpl[i]->get_player_number()==fc.field->get_owned_by())
			if (lg==GAIN)
				cpl[i]->gain_field (fc);
			else
				cpl[i]->lose_field (fc);
}

/*
 * Cleanup for load
 */
void Game::cleanup_for_load(bool t1, bool t2) {
   Editor_Game_Base::cleanup_for_load(t1,t2);

   while(cpl.size()) {
      delete cpl[cpl.size()-1];
      cpl.pop_back();
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

void Game::send_player_change_training_options(Building* b, int atr, int val)
{

	send_player_command (new Cmd_ChangeTrainingOptions(get_gametime(), b->get_owner()->get_player_number(), b, atr, val));
}

void Game::send_player_drop_soldier (Building* b, int ser)
{
	assert(ser != -1);
	send_player_command (new Cmd_DropSoldier(get_gametime(), b->get_owner()->get_player_number(), b, ser));
}

void Game::send_player_change_soldier_capacity (Building* b, int val)
{
	send_player_command (new Cmd_ChangeSoldierCapacity(get_gametime(), b->get_owner()->get_player_number(), b, val));
}

/////////////////////// TESTING STUFF
void Game::send_player_enemyflagaction (Flag* flag, int action, int who_attacks, int num_soldiers, int type)
{
	send_player_command (new Cmd_EnemyFlagAction(get_gametime(), who_attacks, flag, action, who_attacks, num_soldiers, type));
}

