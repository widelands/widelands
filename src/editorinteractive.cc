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
#include "e_ui.h"
#include "options.h"
#include "editor.h"
#include "map.h"
#include "player.h"
#include "minimap.h"

/**********************************************
 *
 * class EditorInteractive
 *
 **********************************************/

/*
==========
Editor_Interactive::Editor_Interactive()

construct editor sourroundings
==========
*/
Editor_Interactive::Editor_Interactive(Editor *e) : Interactive_Base(e) {
   m_editor = e;

   // The mapview. watch the map!!!
   m_mapview = new Map_View(this, 0, 0, get_w(), get_h(), this);
   m_mapview->warpview.set(this, &Editor_Interactive::mainview_move);
   //     main_mapview->fieldclicked.set(this, &Interactive_Player::field_action);

     
   // The panel. Tools, infos and gimmicks
   m_panel = new ToolPanel(this, 0, get_h()-PANEL_HEIGHT, get_w(), PANEL_HEIGHT);
		

   // user interface buttons
   int x = (get_w() - (4*34)) >> 1;
   int y = get_h() - 34;
   Button *b;

	m_minimap= new MiniMapView(m_panel, m_panel->get_w()-PANEL_HEIGHT, 0, this, PANEL_HEIGHT, PANEL_HEIGHT);
   m_minimap->warpview.set(this, &Editor_Interactive::minimap_warp);

		// make sure the viewpos marker is at the right pos to start with
  // new MiniMapView(this, 50, 50, this, 0, 0);

   // temp (should be toggle messages)
   b = new Button(this, x, y, 34, 34, 2);
   b->clicked.set(this, &Editor_Interactive::exit_game_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.bmp", RGBColor(0,0,255)));
   // temp

   b = new Button(this, x+34, y, 34, 34, 2);
   //      b->clicked.set(this, &Interactive_Player::main_menu_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.bmp", RGBColor(0,0,255)));

   b = new Button(this, x+68, y, 34, 34, 2);
   //      b->clicked.set(this, &Interactive_Player::minimap_btn);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.bmp", RGBColor(0,0,255)));

   b = new Button(this, x+102, y, 34, 34, 2);
   //      b->clicked.set(this, &Interactive_Player::toggle_buildhelp);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.bmp", RGBColor(0,0,255)));
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
Editor_Interactive::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Editor_Interactive::start()
{
   int mapw;
	int maph;

   m_maprenderinfo.egbase = m_editor; 
	m_maprenderinfo.map = m_editor->get_map();
	m_maprenderinfo.visibility = 0; 
	m_maprenderinfo.show_buildhelp = true;
	
	mapw = m_maprenderinfo.map->get_width();
	maph = m_maprenderinfo.map->get_height();
	m_maprenderinfo.overlay_basic = (uchar*)malloc(mapw*maph);
	m_maprenderinfo.overlay_roads = (uchar*)malloc(mapw*maph);
	memset(m_maprenderinfo.overlay_roads, 0, mapw*maph);
	
	for(int y = 0; y < maph; y++)
		for(int x = 0; x < mapw; x++) {
			FCoords coords(x, y, m_maprenderinfo.map->get_field(x,y));
			
			recalc_overlay(coords);
		}
}

/*
===============
Editor_Interactive::recalc_overlay

Recalculate build help and borders for the given field
===============
*/
void Editor_Interactive::recalc_overlay(FCoords fc)
{
   Map* map = m_maprenderinfo.map;

   // Only do recalcs after maprenderinfo has been setup
   if (!map)
      return;

   uchar code = 0;
   int owner = fc.field->get_owned_by();

   // A border is on every field that is owned by a player and has
   // neighbouring fields that are not owned by that player
   for(int dir = 1; dir <= 6; dir++) {
      FCoords neighb;

      map->get_neighbour(fc, dir, &neighb);

      if (neighb.field->get_owned_by() != owner)
         code = Overlay_Frontier_Base + owner;
   }

   int buildcaps=fc.field->get_caps();
   if(owner) {
      // Determine the buildhelp icon for that field		
      buildcaps = m_editor->get_player(owner)->get_buildcaps(fc);
   }	

   if (buildcaps & BUILDCAPS_MINE)
      code = Overlay_Build_Mine;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_BIG)
      code = Overlay_Build_Big;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_MEDIUM)
      code = Overlay_Build_Medium;
   else if ((buildcaps & BUILDCAPS_SIZEMASK) == BUILDCAPS_SMALL)
      code = Overlay_Build_Small;
   else if (buildcaps & BUILDCAPS_FLAG)
      code = Overlay_Build_Flag;

   m_maprenderinfo.overlay_basic[fc.y*map->get_width() + fc.x] = code;
}


/** Editor_Interactive::exit_game_btn(void *a)
 *
 * Handle exit button
 */
void Editor_Interactive::exit_game_btn()
{
	end_modal(0);
}


/** Editor_Interactive::minimap_warp(int x, int y)
 *
 * Called whenever the player clicks on a location on the minimap.
 * Warps the main mapview position to the clicked location.
 */
void Editor_Interactive::minimap_warp(int x, int y)
{
	x -= m_mapview->get_w()>>1;
	if (x < 0) x += MULTIPLY_WITH_FIELD_WIDTH(m_editor->get_map()->get_width());
	y -= m_mapview->get_h()>>1;
	if (y < 0) y += MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_editor->get_map()->get_height());
	m_mapview->set_viewpoint(x, y);
}


/** Editor_Interactive::mainview_move(int x, int y)
 *
 * Signal handler for the main view's warpview updates the mini map's
 * viewpos marker position
 */
void Editor_Interactive::mainview_move(int x, int y)
{
   int maxx = MULTIPLY_WITH_FIELD_WIDTH(m_editor->get_map()->get_width());
   int maxy = MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_editor->get_map()->get_height());

   x += m_mapview->get_w()>>1;
   if (x >= maxx) x -= maxx;
   y += m_mapview->get_h()>>1;
   if (y >= maxy) y -= maxy;

   m_minimap->set_view_pos(x, y);

}
