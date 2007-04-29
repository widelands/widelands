/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "cmd_check_eventchain.h"
#include "computer_player.h"
#include "event_chain.h"
#include "interactive_player.h"
#include "fullscreen_menu_launchgame.h"
#include "fullscreen_menu_loadgame.h"
#include "game.h"
#include "game_loader.h"
#include "graphic.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "map_event_manager.h"
#include "map_trigger_manager.h"
#include "network.h"
#include "player.h"
#include "playercommand.h"
#include "soldier.h"
#include "sound_handler.h"
#include <string>
#include "tribe.h"
#include "widelands_map_loader.h"
#include "wlapplication.h"

/** Game::Game(void)
 *
 * init
 */
Game::Game() :
m_state   (gs_none),
m_speed   (1),
ipl       (0),
cmdqueue  (this),
m_realtime(WLApplication::get()->get_time())
{g_sound_handler.m_the_game = this;}

/** Game::~Game(void)
 *
 * cleanup
 */
Game::~Game() {g_sound_handler.m_the_game = NULL;}


/**
 * Returns true if cheat codes have been activated (single-player only)
 */
bool Game::get_allow_cheats()
{
	return true;
}

/** Game::can_start()
 *
 * Returns true if the game settings are valid.
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

		if (get_player(i)->get_type() == Player::Local) {
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

   Map *m = new Map();
   set_map(m);

   FileSystem* fs = g_fs->MakeSubFileSystem( mapname );
	m_maploader = new Widelands_Map_Loader(*fs, m);

    // Loading the locals for the campaign
        if( scenario )
            {
            std::string textdomain("");
            textdomain.append(mapname);
	    i18n::grab_textdomain(textdomain.c_str());
            m_maploader->preload_map(scenario);
            log("Loading the locals for scenario. file: %s.mo\n", mapname);
            }

        m_state = gs_running;

	m_maploader->preload_map(scenario);
	m_maploader->load_world();

    // We have to create the players here
   for( uint i = 1; i <= m->get_nrplayers(); i++)
		add_player
		(i,
		 i == 1 ? Player::Local : Player::AI,
		 m->get_scenario_player_tribe(i),
		 m->get_scenario_player_name(i));

        // Reload the textdomain "widelands", so buttons can be translated.
        if( scenario )
        i18n::grab_textdomain("widelands");

   init_player_controllers ();

        // Unload the textdomain, to make campaign textdomain available, again.
        if( scenario )
        i18n::release_textdomain();

	m_maploader->load_map_complete(this, scenario); // if code==2 is a scenario
	delete m_maploader;
	m_maploader=0;

   if( scenario )
	   i18n::release_textdomain();

	return run();
}


bool Game::run_single_player ()
{
	m_state = gs_menu;
	m_netgame=0;

	m_maploader=0;
	Fullscreen_Menu_LaunchGame lgm(this, 0, &m_maploader);
	const int code = lgm.run();

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

/**
 * Load a game
 * argument defines if this is a single player game (true)
 * or networked (false)
 */
bool Game::run_load_game(const bool is_splayer, std::string filename) {
   assert(is_splayer); // TODO: net game saving not supported

	if (filename.empty()) {
		Fullscreen_Menu_LoadGame ssg(*this);
		if (ssg.run()) filename = ssg.filename(); else return false;
	}
	// We have to create an empty map, otherwise nothing will load properly
	set_map(new Map);

	FileSystem * const fs = g_fs->MakeSubFileSystem(filename.c_str());

	Game_Loader gl(*fs, this);
	gl.load_game();
   delete fs;

   m_state = gs_running;

   return run(true);
}

//extern uchar g_playercolors[MAX_PLAYERS][12];
bool Game::run_multi_player (NetGame* ng)
{
	m_state = gs_menu;
	m_netgame=ng;

	m_maploader=0;
	Fullscreen_Menu_LaunchGame lgm(this, m_netgame, &m_maploader);
	m_netgame->set_launch_menu (&lgm);
	const int code = lgm.run();
	m_netgame->set_launch_menu (0);

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
	m_maploader = (new Map())->get_correct_loader(filename);
	assert (m_maploader!=0);
	m_maploader->preload_map(0);
	set_map (m_maploader->get_map());
}


void Game::init_player_controllers ()
{
	ipl=0;
	const Player_Number nr_players = map().get_nrplayers();
	for (Player_Number i = 1; i <= nr_players; ++i)
		if (const Player * const p = get_player(i))
			if (p->get_type() == Player::Local) {
				ipl = new Interactive_Player(*this, i);
				break;
			}

	assert (ipl!=0);

	// inform base, that we have something interactive
	set_iabase(ipl);

	// set up computer controlled players
	for (Player_Number i = 1; i <= nr_players; ++i)
		if (get_player(i) and get_player(i)->get_type() == Player::AI)
			cpl.push_back (new Computer_Player(*this, i));

}

/**
 * This runs a game, including game creation phase.
 *
 * The setup and loading of a game happens (or rather: will happen) in three
 * stages.
 * 1.  First of all, the host (or single player) configures the game. During this
 *     time, only short descriptions of the game data (such as map headers )are
 *     loaded to minimize loading times.
 * 2a. Once the game is about to start and the configuration screen is finished,
 *     all logic data (map, tribe information, building information) is loaded
 *     during postload.
 * 2b. If a game is created, initial player positions are set. This step is
 *     skipped when a game is loaded.
 * 3.  After this has happened, the game graphics are loaded.
 *
 * \return true if a game actually took place, false otherwise
 */
bool Game::run(bool is_savegame)
{
   postload();

	if (not is_savegame) {
      // Prepare the players (i.e. place HQs)
		const Player_Number nr_players = map().get_nrplayers();
		for (Player_Number i = 1; i <= nr_players; ++i) if
			(Player * const plr = get_player(i))
		{
         plr->init(true);
			if (plr->get_type() == Player::Local)
				ipl->move_view_to(map().get_starting_pos(i));
      }

      // Prepare the map, set default textures
		map().recalc_default_resources();
		map().get_mem().delete_unreferenced_events  ();
		map().get_mtm().delete_unreferenced_triggers();

      // Finally, set the scenario names and tribes to represent
      // the correct names of the players
		for (Player_Number curplr = 1; curplr <= nr_players; ++curplr) {
			const Player * const plr = get_player(curplr);
			const std::string                                             no_name;
			const std::string &  tribe_name = plr ? plr->tribe().name() : no_name;
			const std::string & player_name = plr ? plr->    get_name() : no_name;
			map().set_scenario_player_tribe(curplr,  tribe_name);
			map().set_scenario_player_name (curplr, player_name);
      }



      // Everything prepared, send the first trigger event
      // We lie about the sender here. Hey, what is one lie in a lifetime?
      enqueue_command (new Cmd_CheckEventChain(get_gametime(), -1));
   }

   load_graphics();

   g_sound_handler.change_music("ingame", 1000, 0);

	ipl->run();

	g_sound_handler.change_music("menu", 1000, 0);

	cleanup_objects();
	delete ipl;

	for (unsigned int i=0; i<cpl.size(); i++)
		delete cpl[i];

	g_gr->flush(PicMod_Game);
	g_anim.flush();

	m_state = gs_none;

	return true;
}


/**
 * think() is called by the UI objects initiated during Game::run()
 * during their modal loop.
 * Depending on the current state we advance game logic and stuff,
 * running the cmd queue etc.
 */
void Game::think(void)
{
	if (m_netgame!=0)
	    m_netgame->handle_network ();

	if (m_state == gs_running) {
		for (unsigned int i=0;i<cpl.size();i++)
			cpl[i]->think();

		int frametime = -m_realtime;
		m_realtime =  WLApplication::get()->get_time();
		frametime += m_realtime;

		if (m_netgame!=0) {
			int max_frametime=m_netgame->get_max_frametime();

			if (frametime>max_frametime)
				frametime = max_frametime; //  wait for the next server message
			else if (max_frametime-frametime>500)
				//  we are too long behind network time, so hurry a little
				frametime += (max_frametime - frametime) / 2;
		}
		else
			frametime *= get_speed();

		// maybe we are too fast...
		if (frametime==0)
			return;

		// prevent frametime escalation in case the game logic is the performance bottleneck
		if (frametime > 1000)
			frametime = 1000;

		cmdqueue.run_queue(frametime, get_game_time_pointer());

		g_gr->animate_maptextures(get_gametime());
        
		// check if autosave is needed, but only if that is not a network game
		if (NULL == m_netgame)
			m_savehandler.think(this, m_realtime);
	}
}


/**
 * Change the game speed.
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

/**
 * Cleanup for load
 */
void Game::cleanup_for_load
(const bool flush_graphics, const bool flush_animations)
{
	Editor_Game_Base::cleanup_for_load(flush_graphics, flush_animations);
	for
		(std::vector<Tribe_Descr*>::iterator it = m_tribes.begin();
		 it != m_tribes.end();
		 ++it)
		delete *it;
	m_tribes.resize(0);
	get_cmdqueue()->flush();
   while(cpl.size()) {
      delete cpl[cpl.size()-1];
      cpl.pop_back();
   }
}

/**
 * All player-issued commands must enter the queue through this function.
 * It takes the appropriate action, i.e. either add to the cmd_queue or send
 * across the network.
 */
void Game::send_player_command (PlayerCommand* pc)
{
	if (m_netgame and get_player(pc->get_sender())->get_type() == Player::Local)
		m_netgame->send_player_command (pc);
	else
		enqueue_command (pc);
}

void Game::enqueue_command (BaseCommand * const cmd) {cmdqueue.enqueue(cmd);}

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

void Game::send_player_build_road (int pid, Path & path)
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
void Game::send_player_enemyflagaction
(const Flag * const flag,
 const int action,
 const Player_Number who_attacks,
 const int num_soldiers,
 const int type)
{
	send_player_command (new Cmd_EnemyFlagAction(get_gametime(), who_attacks, flag, action, who_attacks, num_soldiers, type));
}
