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

#include <SDL.h>
#include "editor.h"
#include "error.h"
#include "filesystem.h"
#include "font_handler.h"
#include "fullscreen_menu_fileview.h"
#include "fullscreen_menu_intro.h"
#include "fullscreen_menu_main.h"
#include "fullscreen_menu_netsetup.h"
#include "fullscreen_menu_options.h"
#include "fullscreen_menu_singleplayer.h"
#include "fullscreen_menu_tutorial_select_map.h"
#include "fullscreen_menu_inet_server_options.h"
#include "fullscreen_menu_inet_lobby.h"
#include "game.h"
#include "game_server_connection.h"
#include "game_server_proto.h"
#include "graphic.h"
#include "network.h"
#include "network_ggz.h"
#include "options.h"
#include "setup.h"
#include "system.h"
#include "util.h"
#include "sound_handler.h"

LayeredFileSystem *g_fs;
Sound_Handler* sound_handler;

static void g_shutdown();

/*
===============
g_init

Initialize all subsystems
===============
*/
static void g_init(int argc, char **argv)
{
	try
	{
		// Create filesystem
		g_fs = LayeredFileSystem::Create();
		setup_searchpaths(argc, argv);

		// Handle options
		options_init(argc, argv);

		// Create all subsystems after config has been read
		Sys_Init();

		g_fh = new Font_Handler();

		// Initialize graphics
		Section *s = g_options.pull_section("global");

      Sys_InitGraphics(640, 480, s->get_int("depth",16), s->get_bool("fullscreen", false));

		// complain about unknown options in the configuration file and on the
		// command line

		// KLUDGE!
		// Without this, xres, yres and workareapreview get dropped by
		// check_used().
		// Profile needs support for a Syntax definition to solve this in a sensible way
		s->get_string("xres");
		s->get_string("yres");
		s->get_bool("workareapreview");
		s->get_bool("nozip");
		// KLUDGE!

		// Create global sound handler
      sound_handler=new Sound_Handler();

		g_options.check_used();
	}
	catch(std::exception &e) {
		critical_error("Initialization error: %s", e.what());
		g_shutdown();
		exit(-1);
	}

#define USE_LOCALE 1
#if USE_LOCALE
#include <libintl.h>

   
	// Load message catalog
	setlocale(LC_ALL, "");
	bind_textdomain_codeset ("widelands", "UTF-8"); 
   bindtextdomain("widelands", "locale");
	textdomain("widelands");
#endif
}

/*
===============
g_shutdown

Shutdown all subsystems
===============
*/
static void g_shutdown()
{
	// Shutdown subsystems
	Sys_InitGraphics(0, 0, 0, false);

	if (g_fh) {
		delete g_fh;
      g_fh = 0;
	}

	Sys_Shutdown();

	// Save options
	options_shutdown();

	// Destroy filesystem
	if (g_fs) {
		delete g_fs;
		g_fs = 0;
	}
}

/*
===============
g_main

This is the OS Independant main function.

Control the life-cycle of the game
===============
*/
void g_main(int argc, char** argv)
{
	try
	{
		g_init(argc, argv);

		if(NetGGZ::ref()->used())
		{
			if(NetGGZ::ref()->connect())
			{
				NetGame *netgame;
				
				if(NetGGZ::ref()->host()) netgame = new NetHost();
				else
				{
					while(!NetGGZ::ref()->ip()) NetGGZ::ref()->data();

					IPaddress peer;
					SDLNet_ResolveHost (&peer, NetGGZ::ref()->ip(), WIDELANDS_PORT);
					netgame = new NetClient(&peer);
				}
				netgame->run();
				delete netgame;
			}
		}

		try {
                        sound_handler->start_music("intro");

         Fullscreen_Menu_Intro r;
         r.run();
         bool done=false;

         sound_handler->change_music("menu", 1000);

         while(!done) {
            Fullscreen_Menu_Main *mm = new Fullscreen_Menu_Main;
            int code = mm->run();
            delete mm;

            switch(code) {
               case Fullscreen_Menu_Main::mm_singleplayer:
                  {
                     bool done=false;
                     while(!done) {
                        Fullscreen_Menu_SinglePlayer *sp = new Fullscreen_Menu_SinglePlayer;
                        int code = sp->run();
                        delete sp;

                        switch(code) {
                           case Fullscreen_Menu_SinglePlayer::sp_skirmish:
                              {
                                 Game *g = new Game;
                                 bool ran = g->run_single_player();
                                 delete g;
                                 if (ran) {
                                    // game is over. everything's good. restart Main Menu
                                    done=true;
                                 }
                                 continue;
                              }

                           case Fullscreen_Menu_SinglePlayer::sp_loadgame:
                              {
                                 Game* g = new Game;
                                 bool ran = g->run_load_game(true);
                                 delete g;
                                 if (ran) {
                                    done=true;
                                 }
                                 continue;
                              }
                           
                           case Fullscreen_Menu_SinglePlayer::sp_tutorial:
                              {
                                 Fullscreen_Menu_TutorialSelectMap* sm = new Fullscreen_Menu_TutorialSelectMap;
                                 int code = sm->run();
                                 if(code) {
                                    std::string mapname = sm->get_mapname( code );
                                    delete sm;
                                    
                                    Game* g = new Game;
                                    bool run = g->run_splayer_map_direct( mapname.c_str(), true);
                                    delete g;
                                    if(run) 
                                       done = true;
                                    continue;
                                 }
                                 // Fallthrough if back was pressed
                              }
                           
                           default:
                           case Fullscreen_Menu_SinglePlayer::sp_back:
                              done = true;
                              break;
                        }
                     }
                  }
                  break;
	       
	       
	       case Fullscreen_Menu_Main::mm_multiplayer:
		  {
			Fullscreen_Menu_NetSetup* ns = new Fullscreen_Menu_NetSetup();
			if(NetGGZ::ref()->tables().size() > 0) ns->fill(NetGGZ::ref()->tables());
			int code=ns->run();
			
			NetGame* netgame = 0;
			
			if (code==Fullscreen_Menu_NetSetup::HOSTGAME)
			    netgame=new NetHost();
			else if (code==Fullscreen_Menu_NetSetup::JOINGAME) {
			    IPaddress peer;

//			    if (SDLNet_ResolveHost (&peer, ns->get_host_address(), WIDELANDS_PORT) < 0)
//				    throw wexception("Error resolving hostname %s: %s\n", ns->get_host_address(), SDLNet_GetError());
			    ulong addr;
			    ushort port;
			    
			    if (!ns->get_host_address(addr,port))
				    throw wexception("Address of game server is no good");

			    peer.host=addr;
			    peer.port=port;

			    netgame=new NetClient(&peer);
			} else if(code==Fullscreen_Menu_NetSetup::INTERNETGAME) {
            delete ns;
            Fullscreen_Menu_InetServerOptions* igo = new Fullscreen_Menu_InetServerOptions();
            int code=igo->run();
           
            // Get informations here
            std::string host = igo->get_server_name();
            std::string player = igo->get_player_name();
            delete igo;
            
            if(code) {
               Game_Server_Connection csc(host, GAME_SERVER_PORT);

               try {
                  csc.connect();
               } catch(...) {
                  // TODO: error handling here
                  throw;
               }

               csc.set_username(widen_string(player).c_str());
               
               // Wowi, we are connected. Let's start the lobby
               Fullscreen_Menu_InetLobby* il = new Fullscreen_Menu_InetLobby(&csc);
               il->run();
               delete il;
            }
            break;
         }
			else if((code == Fullscreen_Menu_NetSetup::JOINGGZGAME)
				|| (code == Fullscreen_Menu_NetSetup::HOSTGGZGAME)) {
				if(code == Fullscreen_Menu_NetSetup::HOSTGGZGAME) NetGGZ::ref()->launch();
				if(NetGGZ::ref()->host()) netgame = new NetHost();
				else
				{
					while(!NetGGZ::ref()->ip()) NetGGZ::ref()->data();

					IPaddress peer;
					SDLNet_ResolveHost (&peer, NetGGZ::ref()->ip(), WIDELANDS_PORT);
					netgame = new NetClient(&peer);
				}
			}
			else
			    break;
			
			delete ns;
			
			netgame->run();
			delete netgame;
		  }
		  break;


               case Fullscreen_Menu_Main::mm_options:
                  {
                     Section *s = g_options.pull_section("global");
                     Options_Ctrl *om = new Options_Ctrl(s);
                     delete om;
                  }
                  break;

               case Fullscreen_Menu_Main::mm_readme:
                  {
                     Fullscreen_Menu_FileView* ff=new Fullscreen_Menu_FileView("README","README");
                     ff->run();
                     delete ff;
                  }
                  break;

               case Fullscreen_Menu_Main::mm_license:
                  {
                     Fullscreen_Menu_FileView* ff=new Fullscreen_Menu_FileView("COPYING","COPYING");
                     ff->run();
                     delete ff;
                  }
                  break;

               case Fullscreen_Menu_Main::mm_editor:
                  {
                     Editor* e=new Editor();
                     e->run();
                     delete e;
                     break;
                  }

               default:
               case Fullscreen_Menu_Main::mm_exit:
                  done=true;
                  break;
            }
         }

        sound_handler->stop_music(500);
        delete sound_handler;
		} catch(std::exception &e) {
			critical_error("Unhandled exception: %s", e.what());
		}

		g_shutdown();
	}
	catch(std::exception &e) {
		g_gr = 0; // paranoia
		critical_error("Unhandled exception: %s", e.what());
	}
	catch(...) {
		g_gr = 0;
		critical_error("Unhandled exception");
	}
}


/*
==============
main

Cross-platform entry point for SDL applications.
==============
*/
int main(int argc, char** argv)
{
	g_main(argc, argv);
	return 0;
}

#ifdef __MINGW__
#undef main

// This is a hack needed for mingw under windows 
int main(int argc, char** argv) {
	g_main(argc,argv);
return 0;
}
#endif
