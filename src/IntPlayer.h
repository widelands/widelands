/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#ifndef __S__INTPLAYER_H
#define __S__INTPLAYER_H

#include "mapview.h"
#include "ui.h"

#define MAIN_WINDOW_WIDTH 136
#define MAIN_WINDOW_HEIGHT 34

class Game;
class MiniMap;

class Interactive_Player : public Panel {
		friend class MiniMap;

		 Interactive_Player(const Interactive_Player&);
		 Interactive_Player operator=(const Interactive_Player&);

		  public:
					 Interactive_Player(Game *g, uchar pln);
					 ~Interactive_Player(void);

					 void start();
					 void exit_game_btn();
					 void main_menu_btn();
					 void minimap_btn();
					 void toggle_buildhelp();

					 void field_action(int fx, int fy);
					 void move_view_to(int fx, int fy);

					 void think();

					 inline Game *get_game() { return game; }

					 // Set the in-game resolution
					 static void set_resolution(uint x, uint y) { xresolution=x; yresolution=y; }
					 static inline uint get_xres() { return xresolution; }
					 static inline uint get_yres() { return yresolution; }
         
                inline uchar get_player_number(void) { return player_number; } // for watchwindow
					 
		  private:
					 void mainview_move(int x, int y);
					 void minimap_warp(int x, int y);

					 static uint xresolution, yresolution;
					 Game *game;
					 Map_View* main_mapview;
					 MiniMap *minimap;
					 Window *fieldaction;
                uchar player_number;
};


#endif // __S__INTPLAYER_H
