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
#include "player.h"
#include "map.h"


/*
==============================================================================

GameMainMenu IMPLEMENTATION

==============================================================================
*/

// The GameMainMenu is a rather dumb window with lots of buttons
class GameMainMenu : public Window {
public:
	GameMainMenu(Interactive_Player *plr, UniqueWindow *registry);
	virtual ~GameMainMenu();

private:
	Interactive_Player	*m_player;
	UniqueWindow			*m_registry;
};

/*
===============
GameMainMenu::GameMainMenu

Create all the buttons etc...
===============
*/
GameMainMenu::GameMainMenu(Interactive_Player *plr, UniqueWindow *registry)
	: Window(plr, (plr->get_w()-102)/2, (plr->get_h()-136)/2, 102, 136, "Menu")
{
	m_registry = registry;
	if (m_registry) {
		if (m_registry->window)
			delete m_registry->window;
		
		m_registry->window = this;
		if (m_registry->x >= 0)
			set_pos(m_registry->x, m_registry->y);
	}
}

/*
===============
GameMainMenu::~GameMainMenu

Unregister from the registry pointer
===============
*/
GameMainMenu::~GameMainMenu()
{
	if (m_registry) {
		m_registry->x = get_x();
		m_registry->y = get_y();
		m_registry->window = 0;
	}
}


/*
==============================================================================

Interactive_Player IMPLEMENTATION

==============================================================================
*/

AutoPic Interactive_Player::pic_buildhelp("menu_toggle_buildhelp.bmp", 0, 0, 255, 34, 34);
AutoPic Interactive_Player::pic_minimap("menu_toggle_minimap.bmp", 0, 0, 255, 34, 34);
AutoPic Interactive_Player::pic_menu("menu_toggle_menu.bmp", 0, 0, 255, 34, 34);
// temp
AutoPic Interactive_Player::pic_exit("menu_exit_game.bmp", 0, 0, 255, 34, 34);
// temp ends


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
	
	m_buildroad = false;
	
	// user interface buttons
	int x = (get_w() - (4*34)) >> 1;
	int y = get_h() - 34;
	Button *b;

	b = new Button(this, x, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::exit_game_btn);
	b->set_pic(&pic_exit);

	b = new Button(this, x+34, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::main_menu_btn);
	b->set_pic(&pic_menu);

	b = new Button(this, x+68, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::minimap_btn);
	b->set_pic(&pic_minimap);

	b = new Button(this, x+102, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_buildhelp);
	b->set_pic(&pic_buildhelp);
}

/*
===============
Interactive_Player::~Interactive_Player

cleanups
===============
*/
Interactive_Player::~Interactive_Player(void)
{
	if (m_buildroad)
		abort_build_road();
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
	return g_options.pull_section("global")->get_int("xres", 640);
}

int Interactive_Player::get_yres()
{
	return g_options.pull_section("global")->get_int("yres", 480);
}


/*
===============
Interactive_Player::start

Set the resolution just before going modal
===============
*/
void Interactive_Player::start()
{
	Section *s = g_options.pull_section("global");
	
	Sys_InitGraphics(GFXSYS_SW16, get_xres(), get_yres(), s->get_bool("fullscreen", false));
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

/*
===============
Interactive_Player::main_menu_btn

Bring up or close the main menu
===============
*/
void Interactive_Player::main_menu_btn()
{
	if (m_mainmenu.window)
		delete m_mainmenu.window;
	else
		new GameMainMenu(this, &m_mainmenu);
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
	int x = MULTIPLY_WITH_FIELD_WIDTH(fx);
	int y = MULTIPLY_WITH_HALF_FIELD_HEIGHT(fy);

	if (m_minimap.window)
		((MiniMap *)m_minimap.window)->set_view_pos(x, y);
	
	x -= main_mapview->get_w()>>1;
	if (x < 0) x += MULTIPLY_WITH_FIELD_WIDTH(m_game->get_map()->get_width());
	y -= main_mapview->get_h()>>1;
	if (y < 0) y += MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_game->get_map()->get_height());
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
	if (!get_ignore_shadow() && !get_player()->is_field_seen(m_fieldsel))
		return;			

	// Special case for buildings
	BaseImmovable *imm = m_game->get_map()->get_immovable(m_fieldsel);
	
	if (imm && imm->get_type() == Map_Object::BUILDING) {
		Building *building = (Building *)imm;
		
		if (building->get_owner()->get_player_number() == get_player_number()) {
			building->show_options(this);
			return;
		}
	}
	
	// everything else can bring up the temporary dialog
	show_field_action(this, &m_fieldaction);
}

/*
===============
Interactive_Player::think

Called once per frame by the UI code
===============
*/
void Interactive_Player::think()
{
	// Call game logic here
   // The game advances
	m_game->think();
   
	// The entire screen needs to be redrawn (unit movement, tile animation, etc...)
	g_gr->update_fullscreen();
	
	// some of the UI windows need to think()
	Panel::think();
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
		int maxx = MULTIPLY_WITH_FIELD_WIDTH(m_game->get_map()->get_width());
		int maxy = MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_game->get_map()->get_height());

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
	if (x < 0) x += MULTIPLY_WITH_FIELD_WIDTH(m_game->get_map()->get_width());
	y -= main_mapview->get_h()>>1;
	if (y < 0) y += MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_game->get_map()->get_height());
	main_mapview->set_viewpoint(x, y);
}


/*
===============
Interactive_Player::start_build_road

Begin building a road
===============
*/
void Interactive_Player::start_build_road(Coords start)
{
	// create an empty path
	m_buildroad = new CoordPath(m_game->get_map(), start);
}


/*
===============
Interactive_Player::abort_build_road

Stop building the road
===============
*/
void Interactive_Player::abort_build_road()
{
	assert(m_buildroad);
	
	delete m_buildroad;
	m_buildroad = 0;
}
	

/*
===============
Interactive_Player::finish_build_road

Finally build the road
===============
*/
void Interactive_Player::finish_build_road()
{
	assert(m_buildroad);

	if (m_buildroad->get_nsteps()) {
		// awkward... path changes ownership
		Path *path = new Path(*m_buildroad);
		m_game->send_player_command(get_player_number(), CMD_BUILD_ROAD, (int)path, 0, 0);
	}
	
	delete m_buildroad;
	m_buildroad = 0;
}


/*
===============
Interactive_Player::append_build_road

If field is on the path, remove tail of path. 
Otherwise append if possible or return false.
===============
*/
bool Interactive_Player::append_build_road(Coords field)
{
	assert(m_buildroad);

	int idx = m_buildroad->get_index(field);
	
	if (idx >= 0) {
		m_buildroad->truncate(idx);
		return true;
	}
	
	// Find a path to the clicked-on field
	Map *map = m_game->get_map();
	Path path;
	
	if (map->findpath(m_buildroad->get_end(), field, MOVECAPS_WALK, 0, &path, 
	                  get_player(), true, &m_buildroad->get_coords()) < 0)
		return false; // couldn't find a path
	
	m_buildroad->append(path);
	
	return true;
}

/*
===============
Interactive_Player::get_build_road_start

Return the current road-building startpoint
===============
*/
const Coords &Interactive_Player::get_build_road_start()
{
	assert(m_buildroad);
	
	return m_buildroad->get_start();
}

/*
===============
Interactive_Player::get_build_road_end

Return the current road-building endpoint
===============
*/
const Coords &Interactive_Player::get_build_road_end()
{
	assert(m_buildroad);
	
	return m_buildroad->get_end();
}

/*
===============
Interactive_Player::get_build_road_end_dir

Return the direction of the last step
===============
*/
int Interactive_Player::get_build_road_end_dir()
{
	assert(m_buildroad);
	
	if (!m_buildroad->get_nsteps())
		return 0;
	
	return m_buildroad->get_step(m_buildroad->get_nsteps()-1);
}
