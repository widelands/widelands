/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#include "editorinteractive.h"
#include "ui.h"
#include "options.h"

/**********************************************
 *
 * class EditorInteractive
 *
 **********************************************/

/***********************************************
 * Editor_Interactive::Editor_Interactive()
 *
 * construct editor sourroundings
 ***********************************************/
Editor_Interactive::Editor_Interactive(Editor* editor) :
   Panel(0, 0, 0, get_xres(), get_yres()) {
// Switch to the new graphics system now, if necessary
	Section *s = g_options.pull_section("global");
	
	Sys_InitGraphics(GFXSYS_SW16, get_xres(), get_yres(), s->get_bool("fullscreen", false));
	
//	memset(&m_maprenderinfo, 0, sizeof(m_maprenderinfo));
	
	// Setup all screen elements
	m_editor = editor;
	
/*	main_mapview = new Map_View(this, 0, 0, get_w(), get_h(), this);
	main_mapview->warpview.set(this, &Interactive_Player::mainview_move);
	main_mapview->fieldclicked.set(this, &Interactive_Player::field_action);

	m_fieldsel_freeze = false;
	
	m_buildroad = false;
	
	// user interface buttons
	int x = (get_w() - (4*34)) >> 1;
	int y = get_h() - 34;
	Button *b;

	// temp (should be toggle messages)
	b = new Button(this, x, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::exit_game_btn);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.bmp", RGBColor(0,0,255)));
	// temp

	b = new Button(this, x+34, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::main_menu_btn);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.bmp", RGBColor(0,0,255)));

	b = new Button(this, x+68, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::minimap_btn);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.bmp", RGBColor(0,0,255)));

	b = new Button(this, x+102, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_buildhelp);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.bmp", RGBColor(0,0,255)));*/
}

/****************************************
 * Editor_Interactive::~EditorInteractive()
 *
 * cleanup
 */
Editor_Interactive::~Editor_Interactive() {
}

/*
===============
Editor_Interactive::get_xres [static]
Editor_Interactive::get_yres [static]

Retrieve in-game resolution from g_options.
===============
*/
int Editor_Interactive::get_xres()
{
	return g_options.pull_section("global")->get_int("xres", 640);
}

int Editor_Interactive::get_yres()
{
	return g_options.pull_section("global")->get_int("yres", 480);
}
