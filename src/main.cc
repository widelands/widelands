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
#include "fullscreen_menu_options.h"
#include "fullscreen_menu_singleplayer.h"
#include "fullscreen_menu_netsetup.h"
#include "game.h"
#include "graphic.h"
#include "options.h"
#include "setup.h"
#include "system.h"
#include "network.h"

LayeredFileSystem *g_fs;

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
      // Load the standart fonts
      g_fh->load_font(UI_FONT_SMALL, UI_FONT_SMALL_CLR);
      g_fh->load_font(UI_FONT_BIG, UI_FONT_BIG_CLR);

		// Initialize graphics
		Section *s = g_options.pull_section("global");

		Sys_InitGraphics(Sys_GetGraphicsSystemFromString(s->get_string("gfxsys", "sw32")),
						640, 480, s->get_bool("fullscreen", false));

		// complain about unknown options in the configuration file and on the
		// command line

		// KLUDGE!
		// Without this, xres and yres get dropped by check_used().
		// Profile needs support for a Syntax definition to solve this in a sensible way
		s->get_string("xres");
		s->get_string("yres");
		// KLUDGE!

		g_options.check_used();
	}
	catch(std::exception &e) {
		critical_error("Initialization error: %s", e.what());
		g_shutdown();
		exit(-1);
	}
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
	Sys_InitGraphics(GFXSYS_NONE, 0, 0, false);

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

		try {
         Fullscreen_Menu_Intro r;
         r.run();
         bool done=false;

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
			int code=ns->run();
			
			NetGame* netgame;
			
			if (code==Fullscreen_Menu_NetSetup::HOSTGAME)
			    netgame=new NetHost();
			else if (code==Fullscreen_Menu_NetSetup::JOINGAME) {
			    IPaddress peer;

			    if (SDLNet_ResolveHost (&peer, ns->get_host_address(), WIDELANDS_PORT) < 0)
				    throw wexception("Error resolving hostname %s: %s\n", ns->get_host_address(), SDLNet_GetError());

			    netgame=new NetClient(&peer);
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
                     Fullscreen_Menu_Options *om = new Fullscreen_Menu_Options(s->get_int("xres", 640), s->get_int("yres", 640), s->get_bool("fullscreen", false), s->get_bool("inputgrab", false));
                     int code = om->run();

                     if (code == Fullscreen_Menu_Options::om_ok) {
                        Section *s = g_options.pull_section("global");

                        s->set_int("xres", om->get_xres());
                        s->set_int("yres", om->get_yres());
                        s->set_bool("fullscreen", om->get_fullscreen());
                        s->set_bool("inputgrab", om->get_inputgrab());

								switch(om->get_gfxsys()) {
								case GFXSYS_SW16: s->set_string("gfxsys", "sw16"); break;
								case GFXSYS_SW32: s->set_string("gfxsys", "sw32"); break;
                        default: wexception("Unknown graphic system is set!!");
								}

                        Sys_SetInputGrab(om->get_inputgrab());
                     }
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
