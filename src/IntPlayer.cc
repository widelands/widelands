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

#include "widelands.h"
#include "options.h"
#include "ui.h"
#include "graphic.h"
#include "cursor.h"
#include "game.h"
#include "minimap.h"
#include "fieldaction.h"
#include "mapview.h"
#include "IntPlayer.h"
#include "map.h"


/*
==============================================================================

Interactive_Player IMPLEMENTATION

==============================================================================
*/

/*
===============
Interactive_Player::Interactive_Player
 
Initialize
===============
*/
Interactive_Player::Interactive_Player(Game *g, uchar plyn)
	: Panel(0, 0, 0, get_xres(), get_yres())
{
	m_game = g;
	m_player_number = plyn;

	main_mapview = new Map_View(this, 0, 0, get_w(), get_h(), this);
	main_mapview->warpview.set(this, &Interactive_Player::mainview_move);
	main_mapview->fieldclicked.set(this, &Interactive_Player::field_action);

	m_ignore_shadow = false;
	
	m_fieldsel.x = m_fieldsel.y = 0;
	m_fieldsel_freeze = false;
	
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
	b->clicked.set(this, &Interactive_Player::toggle_buildhelp);
	b->set_pic(g_fh.get_string("BHELP", 0));
}

/** Interactive_Player::~Interactive_Player(void)
 *
 * cleanups
 */
Interactive_Player::~Interactive_Player(void)
{
}

/*
===============
Interactive_Player::get_xres [static]
Interactive_Player::get_yres [static]

Retrieve in-game resolution from g_options.
===============
*/
int Interactive_Player::get_xres()
{
	return g_options.get_safe_section("global")->get_int("xres", 640);
}

int Interactive_Player::get_yres()
{
	return g_options.get_safe_section("global")->get_int("yres", 480);
}


/*
===============
Interactive_Player::start

Set the resolution just before going modal
===============
*/
void Interactive_Player::start()
{
	g_gr.set_mode(get_xres(), get_yres(), g_gr.get_mode());
}


/*
===============
Interactive_Player::get_player

Return the logic player that is controlled by this Interactive_Player
===============
*/
Player *Interactive_Player::get_player()
{
	return m_game->get_player(m_player_number);
}


/*
===============
Interactive_Player::set_fieldsel

Change the field selection. Does not honour the freeze!
===============
*/
void Interactive_Player::set_fieldsel(Coords c)
{
	m_fieldsel = c;
}


/*
===============
Interactive_Player::set_fieldsel_freeze

Field selection is frozen while the field action dialog is visible
===============
*/
void Interactive_Player::set_fieldsel_freeze(bool yes)
{
	m_fieldsel_freeze = yes;
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

//
// Toggles buildhelp rendering in the main MapView
//
void Interactive_Player::toggle_buildhelp(void)
{
   main_mapview->toggle_buildhelp();
}

/** Interactive_Player::minimap_btn()
 *
 * Handle minimap button by opening the minimap (or closing it if it's
 * currently open).
 */
void Interactive_Player::minimap_btn()
{
	if (m_minimap.window)
		delete m_minimap.window;
	else {
		MiniMap *mm = new MiniMap(this, &m_minimap);
		mm->warpview.set(this, &Interactive_Player::minimap_warp);

		// make sure the viewpos marker is at the right pos to start with
		mainview_move(main_mapview->get_vpx(), main_mapview->get_vpy());
	}
}

/** Interactive_Player::move_view_to(int fx, int fy)
 *
 * Move the mainview to the given position (in field coordinates)
 */
void Interactive_Player::move_view_to(int fx, int fy)
{
	int x = fx * FIELD_WIDTH;
	int y = fy * (FIELD_HEIGHT/2);

	if (m_minimap.window)
		((MiniMap *)m_minimap.window)->set_view_pos(x, y);
	
	x -= main_mapview->get_w()>>1;
	if (x < 0) x += m_game->get_map()->get_w() * FIELD_WIDTH;
	y -= main_mapview->get_h()>>1;
	if (y < 0) y += m_game->get_map()->get_h() * (FIELD_HEIGHT>>1);
	main_mapview->set_viewpoint(x, y);
}


/*
===============
Interactive_Player::warp_mouse_to_field

Move the mouse so that it's directly above the given field
===============
*/
void Interactive_Player::warp_mouse_to_field(Coords c)
{
	main_mapview->warp_mouse_to_field(c);
}


/*
===============
Interactive_Player::field_action

Player has clicked on the given field; bring up the context menu.
===============
*/
void Interactive_Player::field_action()
{
	// Special case for buildings
	std::vector<Map_Object*> objs;
	
	if (m_game->get_map()->find_objects(m_fieldsel, 0, Map_Object::BUILDING, &objs)) {
		Building *building = (Building *)objs[0];
		
		if (building->get_owned_by() == get_player_number()) {
			building->show_options(this);
			return;
		}
	}
	
	// everything else can bring up the temporary dialog
	show_field_action(this, &m_fieldaction);
}

/** Interactive_Player::think()
 *
 * Called once per frame by the UI code
 */
void Interactive_Player::think()
{
	// Call game logic here
   // The game advances
	m_game->think();
   
	// The entire screen needs to be redrawn (unit movement, tile animation, etc...)
	g_gr.needs_fs_update();
}

/*
===============
Interactive_Player::handle_key

Global in-game keypresses:
Space: toggles buildhelp
F5: reveal map
===============
*/
bool Interactive_Player::handle_key(bool down, int code, char c)
{
	switch(code) {
	case KEY_SPACE:
		if (down)
			toggle_buildhelp();
		return true;
	
	case KEY_m:
		if (down)
			minimap_btn();
		return true;
		
	case KEY_F5:
		if (down) {
			if (m_ignore_shadow)
				m_ignore_shadow = false;
			else if (get_game()->get_allow_cheats())
				m_ignore_shadow = true;
		}
		return true;
	}
	
	return false;
}

/** Interactive_Player::mainview_move(int x, int y)
 *
 * Signal handler for the main view's warpview updates the mini map's
 * viewpos marker position
 */
void Interactive_Player::mainview_move(int x, int y)
{
	if (m_minimap.window) {
		int maxx = m_game->get_map()->get_w() * FIELD_WIDTH;
		int maxy = m_game->get_map()->get_h() * (FIELD_HEIGHT>>1);

		x += main_mapview->get_w()>>1;
		if (x >= maxx) x -= maxx;
		y += main_mapview->get_h()>>1;
		if (y >= maxy) y -= maxy;

		((MiniMap*)m_minimap.window)->set_view_pos(x, y);
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
	if (x < 0) x += m_game->get_map()->get_w() * FIELD_WIDTH;
	y -= main_mapview->get_h()>>1;
	if (y < 0) y += m_game->get_map()->get_h() * (FIELD_HEIGHT>>1);
	main_mapview->set_viewpoint(x, y);
}
