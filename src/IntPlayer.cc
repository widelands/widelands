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

#include "ui.h"
#include "input.h"
#include "graphic.h"
#include "IntPlayer.h"
#include "cursor.h"
#include "game.h"


/** class Interactive_Player
 *
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 *
 * Depends: g_ip, g_ui, g_gr, g_cur
 */

/** Interactive_Player::Interactive_Player(Game *g)
 *
 * Init
 *
 * Args: g	the game to be played
 */
Interactive_Player::Interactive_Player(Game *g)
	: Panel(0, 0, 0, Game::get_xres(), Game::get_yres())
{
	main_mapview = new Map_View(this, 0, 0, get_w(), get_h(), g->get_map());

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
	g_gr.set_mode(Game::get_xres(), Game::get_yres(), g_gr.get_mode());
	g_ip.set_max_cords(Game::get_xres()-g_cur.get_w(), Game::get_yres()-g_cur.get_h());
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
	new Window(this, 100, 100, 150, 250, "Test");
}

/** Interactive_Player::think()
 *
 * Called once per frame by the UI code
 */
void Interactive_Player::think()
{
	// Call game logic here

	// The entire screen needs to be redrawn (unit movement, tile animation, etc...)
	g_gr.needs_fs_update();
}
