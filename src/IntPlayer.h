/*
 * Copyright (C) 2002 by Holger Rapp
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

#include "mytypes.h"
#include "mapview.h"
#include "ui.h"

#define MAIN_WINDOW_WIDTH 136
#define MAIN_WINDOW_HEIGHT 34

class Game;

class Interactive_Player : public Panel {
		 Interactive_Player(const Interactive_Player&);
		 Interactive_Player operator=(const Interactive_Player&);

		  public:
					 Interactive_Player(Game *g);
					 ~Interactive_Player(void);

					 void start();
					 void exit_game_btn();
					 void main_menu_btn();

					 void think();

					 // Set the in-game resolution
					 static void set_resolution(uint x, uint y) { xresolution=x; yresolution=y; }
					 static inline uint get_xres() { return xresolution; }
					 static inline uint get_yres() { return yresolution; }

		  private:
					 static uint xresolution, yresolution;
					 Game *game;
					 Map_View* main_mapview;
};


#endif // __S__INTPLAYER_H
