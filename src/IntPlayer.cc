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

#include <vector>
#include "widelands.h"
#include "ui.h"
#include "input.h"
#include "graphic.h"
#include "IntPlayer.h"
#include "cursor.h"
#include "game.h"
#include "minimap.h"
#include "fieldaction.h"


/** class Interactive_Player
 *
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 *
 * Depends: g_ip, g_ui, g_gr, g_cur
 */

uint Interactive_Player::xresolution;
uint Interactive_Player::yresolution;

/** Interactive_Player::Interactive_Player(Game *g)
 *
 * Init
 *
 * Args: g	the game to be played
 */
Interactive_Player::Interactive_Player(Game *g, uchar plyn)
	: Panel(0, 0, 0, get_xres(), get_yres())
{
	game = g;
   player_number=plyn;

	main_mapview = new Map_View(this, 0, 0, get_w(), get_h(), g, player_number);
	main_mapview->warpview.set(this, &Interactive_Player::mainview_move);
	main_mapview->fieldclicked.set(this, &Interactive_Player::field_action);
	minimap = 0;
	fieldaction = 0;

	// user interface buttons
	int x = (get_w() - (4*34)) >> 1;
	int y = get_h() - 34;
	Button *b;

	b = new Button(this, x, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::exit_game_btn);
	b->set_pic(g_fh.get_string("EXIT", 0));

	b = new Button(this, x+34, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::main_menu_btn);
	b->set_pic(g_fh.get_string("MENU", 0));

	b = new Button(this, x+68, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::minimap_btn);
	b->set_pic(g_fh.get_string("MAP", 0));

	b = new Button(this, x+102, y, 34, 34, 2);
}

/** Interactive_Player::~Interactive_Player(void)
 *
 * cleanups
 */
Interactive_Player::~Interactive_Player(void)
{
}

/** Interactive_Player::start()
 *
 * Set the resolution
 */
void Interactive_Player::start()
{
	g_gr.set_mode(get_xres(), get_yres(), g_gr.get_mode());
	g_ip.set_max_cords(get_xres()-g_cur.get_w(), get_yres()-g_cur.get_h());
}

/** Interactive_Player::exit_game_btn(void *a)
 *
 * Handle exit button
 */
void Interactive_Player::exit_game_btn()
{
	end_modal(0);
}

/** Interactive_Player::main_menu_btn()
 *
 * Bring up the main menu
 */
void Interactive_Player::main_menu_btn()
{
	new Window(this, 100, 100, 150, 250, "Menu");
}

/** Interactive_Player::minimap_btn()
 *
 * Handle minimap button by opening the minimap (or closing it if it's
 * currently open).
 */
void Interactive_Player::minimap_btn()
{
	if (minimap)
		delete minimap;
	else
	{
		new MiniMap(this, 200, 150, game->get_map(), &minimap, game->get_player(player_number));
		minimap->warpview.set(this, &Interactive_Player::minimap_warp);

		// make sure the viewpos marker is at the right pos to start with
		mainview_move(main_mapview->get_vpx(), main_mapview->get_vpy());
	}
}

/** Interactive_Player::field_action(int fx, int fy)
 *
 * Player has clicked on the given field; bring up the context menu.
 *
 * Args: fx		field coordinates
 *       fy
 */
void Interactive_Player::field_action(int fx, int fy)
{
	// note: buildings owned by the player must be treated differently
	// (i.e bring up dialog specific to the building)

   // TEMP
   cerr << fx << ":" << fy << "=" << game->get_map()->get_field(fx,fy)->get_terr() << "|" << game->get_map()->get_field(fx,fy)->get_terd() << endl;
   cerr << fx << ":" << fy << "=" << hex << (int) game->get_map()->get_field(fx,fy)->get_terr()->get_is() << "|" << 
      (int) game->get_map()->get_field(fx,fy)->get_terd()->get_is() << endl << dec ;
   cerr << "Bau symbol: " << (int) game->get_map()->get_build_symbol(fx,fy) << endl;
   cerr << "Hoehe: " << (int) game->get_map()->get_field(fx,fy)->get_height() << endl;

   // TEMP ENDS
   
	show_field_action(this, fx, fy, &fieldaction);
}

/** Interactive_Player::think()
 *
 * Called once per frame by the UI code
 */
void Interactive_Player::think()
{
	// Call game logic here
   // The game advances
   game->think();
   
	// The entire screen needs to be redrawn (unit movement, tile animation, etc...)
	g_gr.needs_fs_update();
}

/** Interactive_Player::mainview_move(int x, int y)
 *
 * Signal handler for the main view's warpview updates the mini map's
 * viewpos marker position
 */
void Interactive_Player::mainview_move(int x, int y)
{
	if (minimap) {
		int maxx = game->get_map()->get_w() * FIELD_WIDTH;
		int maxy = game->get_map()->get_h() * (FIELD_HEIGHT>>1);

		x += main_mapview->get_w()>>1;
		if (x >= maxx) x -= maxx;
		y += main_mapview->get_h()>>1;
		if (y >= maxy) y -= maxy;

		minimap->set_view_pos(x, y);
	}
}

/** Interactive_Player::minimap_warp(int x, int y)
 *
 * Called whenever the player clicks on a location on the minimap.
 * Warps the main mapview position to the clicked location.
 */
void Interactive_Player::minimap_warp(int x, int y)
{
	x -= main_mapview->get_w()>>1;
	if (x < 0) x += game->get_map()->get_w() * FIELD_WIDTH;
	y -= main_mapview->get_h()>>1;
	if (y < 0) y += game->get_map()->get_h() * (FIELD_HEIGHT>>1);
	main_mapview->set_viewpoint(x, y);
}
