/*
 * Copyright (C) 2002-2004 by The Widelands Development Team
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

#include "IntPlayer.h"
#include "building.h"
#include "cmd_queue.h"
#include "fieldaction.h"
#include "font_handler.h"
#include "keycodes.h"
#include "immovable.h"
#include "mapview.h"
#include "player.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"
#include "overlay_manager.h"

/*
==============================================================================

GameMainMenu IMPLEMENTATION

==============================================================================
*/

// The GameMainMenu is a rather dumb window with lots of buttons
class GameMainMenu : public UIUniqueWindow {
public:
	GameMainMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry);
	virtual ~GameMainMenu();

private:
	Interactive_Player	*m_player;
};

/*
===============
GameMainMenu::GameMainMenu

Create all the buttons etc...
===============
*/
GameMainMenu::GameMainMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(plr, registry, 102, 136, "Menu")
{
	if (get_usedefaultpos())
		center_to_parent();
}


/*
===============
GameMainMenu::~GameMainMenu
===============
*/
GameMainMenu::~GameMainMenu()
{
}


/*
==============================================================================

Interactive_Player IMPLEMENTATION

==============================================================================
*/

// This function is the callback for recalculation of field overlays
static int Int_Player_overlay_callback_function(FCoords& fc, void* data, int) {
   Interactive_Player* plr=static_cast<Interactive_Player*>(data);

   return plr->get_player()->get_buildcaps(fc);
}


/*
===============
Interactive_Player::Interactive_Player

Initialize
===============
*/
Interactive_Player::Interactive_Player(Game *g, uchar plyn) : Interactive_Base(g)
{


   // Setup all screen elements
	m_game = g;
	m_player_number = plyn;

	Map_View* mview;
   mview = new Map_View(this, 0, 0, get_w(), get_h(), this);
	mview->warpview.set(this, &Interactive_Player::mainview_move);
	mview->fieldclicked.set(this, &Interactive_Player::field_action);
   set_mapview(mview);

	m_buildroad = false;

	// user interface buttons
	int x = (get_w() - (4*34)) >> 1;
	int y = get_h() - 34;
	UIButton *b;

	// temp (should be toggle messages)
	b = new UIButton(this, x, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::exit_game_btn);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.png", RGBColor(0,0,255)));
	// temp

	b = new UIButton(this, x+34, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::main_menu_btn);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.png", RGBColor(0,0,255)));

	b = new UIButton(this, x+68, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_minimap);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.png", RGBColor(0,0,255)));

	b = new UIButton(this, x+102, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_buildhelp);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.png", RGBColor(0,0,255)));

	// Speed info
	m_label_speed = new UITextarea(this, get_w(), 0, 0, 0, "", Align_TopRight);

   m_road_buildhelp_overlay_jobid=0;
   m_jobid=0;
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
Interactive_Player::think

Update the speed display.
===============
*/
void Interactive_Player::think()
{
	Interactive_Base::think();

	// Draw speed display
	int speed = m_game->get_speed();
	char buf[32] = "";

	if (!speed)
		strcpy(buf, "PAUSE");
	else if (speed > 1)
		snprintf(buf, sizeof(buf), "%ix", speed);

	m_label_speed->set_text(buf);
}


/*
===============
Interactive_Player::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Interactive_Player::start()
{
	int mapw;
	int maph;

   m_maprenderinfo.egbase = m_game;
	m_maprenderinfo.visibility = get_player()->get_visibility();

	mapw = 0;
	maph = 0; 

   get_map()->get_overlay_manager()->show_buildhelp(false);
   get_map()->get_overlay_manager()->register_overlay_callback_function(&Int_Player_overlay_callback_function, static_cast<void*>(this));

   // Recalc whole map for changed owner stuff
   get_map()->recalc_whole_map();
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
   get_map()->get_overlay_manager()->toggle_buildhelp();
}

/*
===============
Interactive_Player::field_action

Player has clicked on the given field; bring up the context menu.
===============
*/
void Interactive_Player::field_action()
{
	if (m_maprenderinfo.visibility && !get_player()->is_field_seen(get_fieldsel_pos()))
		return;

	// Special case for buildings
	BaseImmovable *imm = m_game->get_map()->get_immovable(get_fieldsel_pos());

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
			toggle_minimap();
		return true;

	case KEY_c:
		if (down)
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
		return true;

	case KEY_s:
		if (down)
			set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
		return true;

	case KEY_PAGEUP:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(speed + 1);
		}
		return true;

	case KEY_PAGEDOWN:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(std::max(0, speed-1));
		}
		return true;

	case KEY_F5:
		if (down) {
			if (!m_maprenderinfo.visibility)
				m_maprenderinfo.visibility = get_player()->get_visibility();
			else if (get_game()->get_allow_cheats())
				m_maprenderinfo.visibility = 0;
		}
		return true;

	case KEY_UP:
		if (down)
			get_mapview()->set_rel_viewpoint(Point(0, -20));
		return true;

	case KEY_DOWN:
		if (down)
			get_mapview()->set_rel_viewpoint(Point(0, 20));
		return true;

	case KEY_LEFT:
		if (down)
			get_mapview()->set_rel_viewpoint(Point(-20, 0));
		return true;

	case KEY_RIGHT:
		if (down)
			get_mapview()->set_rel_viewpoint(Point(20, 0));
		return true;
	}

	return false;
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

	roadb_add_overlay();
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

	roadb_remove_overlay();

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

	roadb_remove_overlay();

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
		roadb_remove_overlay();
		m_buildroad->truncate(idx);
		roadb_add_overlay();

		return true;
	}

	// Find a path to the clicked-on field
	Map *map = m_game->get_map();
	Path path;
	CheckStepRoad cstep(get_player(), MOVECAPS_WALK, &m_buildroad->get_coords());

	if (map->findpath(m_buildroad->get_end(), field, 0, &path, &cstep, Map::fpBidiCost) < 0)
		return false; // couldn't find a path

	roadb_remove_overlay();
	m_buildroad->append(path);
	roadb_add_overlay();

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

/*
===============
Interactive_Player::roadb_add_overlay

Add road building data to the road overlay
===============
*/
void Interactive_Player::roadb_add_overlay()
{
	assert(m_buildroad);

	//log("Add overlay\n");

	Map* map = m_game->get_map();

	// preview of the road
   assert(!m_jobid);
   m_jobid=get_map()->get_overlay_manager()->get_a_job_id();
	for(int idx = 0; idx < m_buildroad->get_nsteps(); idx++)	{
		uchar dir = m_buildroad->get_step(idx);
		Coords c = m_buildroad->get_coords()[idx];

		if (dir < Map_Object::WALK_E || dir > Map_Object::WALK_SW) {
			map->get_neighbour(c, dir, &c);
			dir = get_reverse_dir(dir);
		}

		int shift = 2*(dir - Map_Object::WALK_E);

      uchar set_to= get_map()->get_overlay_manager()->get_road_overlay(c);
      set_to|=  Road_Normal << shift;
      get_map()->get_overlay_manager()->register_road_overlay(c, set_to, m_jobid);
	}

	// build hints
	FCoords endpos = map->get_fcoords(m_buildroad->get_end());

   assert(!m_road_buildhelp_overlay_jobid);
   m_road_buildhelp_overlay_jobid= get_map()->get_overlay_manager()->get_a_job_id();
	for(int dir = 1; dir <= 6; dir++) {
		FCoords neighb;
		int caps;

		map->get_neighbour(endpos, dir, &neighb);
		caps = get_player()->get_buildcaps(neighb);

		if (!(caps & MOVECAPS_WALK))
			continue; // need to be able to walk there

		BaseImmovable *imm = map->get_immovable(neighb); // can't build on robusts
		if (imm && imm->get_size() >= BaseImmovable::SMALL) {
			if (!(imm->get_type() == Map_Object::FLAG ||
					(imm->get_type() == Map_Object::ROAD && caps & BUILDCAPS_FLAG)))
				continue;
		}

		if (m_buildroad->get_index(neighb) >= 0)
			continue; // the road can't cross itself

		int slope = abs(endpos.field->get_height() - neighb.field->get_height());
		int icon;

		if (slope < 2)
			icon = 1;
		else if (slope < 4)
			icon = 2;
		else
			icon = 3;

      std::string name="";
      switch(icon) {
         case 1: name="pics/roadb_green.png"; break;
         case 2: name="pics/roadb_yellow.png"; break;
         case 3: name="pics/roadb_red.png"; break;
      };

      assert(name!="");

      get_map()->get_overlay_manager()->register_overlay(neighb,  g_gr->get_picture(PicMod_Game, name.c_str(), RGBColor(0,0,255)),7, Coords(-1,-1), m_road_buildhelp_overlay_jobid);
	}
}

/*
===============
Interactive_Player::roadb_remove_overlay

Remove road building data from road overlay
===============
*/
void Interactive_Player::roadb_remove_overlay()
{
	assert(m_buildroad);

	//log("Remove overlay\n");
	
   // preview of the road
   if(m_jobid) 
       get_map()->get_overlay_manager()->remove_road_overlay(m_jobid);
   m_jobid=0;

	// build hints
   if(m_road_buildhelp_overlay_jobid) 
      get_map()->get_overlay_manager()->remove_overlay(m_road_buildhelp_overlay_jobid);
   m_road_buildhelp_overlay_jobid=0;
}
