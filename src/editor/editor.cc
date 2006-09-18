/*
 * Copyright (C) 2002-2003, 2006 by the Widelands Development Team
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

#include "editor.h"
#include "editorinteractive.h"
#include "graphic.h"
#include "map.h"
#include "player.h"
#include "world.h"
#include "wlapplication.h"

/*
===========
Editor::Editor()

inits
===========
*/

Editor::Editor() {
   m_eia=0;

   m_realtime=WLApplication::get()->get_time();
}


/*
==========
Editor::~Editor()

cleanups
*/
Editor::~Editor() {
}

/*
=========
Editor::run()

start the editor, start the Editor_Interactive
and go for it
=========
*/
void Editor::run() {

   // set empty map, defaults
   Map* map=new Map;
   map->create_empty_map();


   set_map(map);

   g_gr->flush(PicMod_Menu);

   m_eia = new Editor_Interactive(this);
   // inform base, that we have something interactive
   set_iabase(m_eia);

   postload();
   load_graphics();

   m_eia->start();
   m_eia->run();


   get_objects()->cleanup(this);
   delete m_eia;
   m_eia=0;

   g_gr->flush(PicMod_Game);
   g_anim.flush();

}

/*
==========
Editor::think()

logic function for editor. advances timecounter
==========
*/
void Editor::think() {
	int lasttime = m_realtime;
	int frametime;
	m_realtime = WLApplication::get()->get_time();
	frametime = m_realtime - lasttime;

   *get_game_time_pointer()+=frametime;

   g_gr->animate_maptextures(get_gametime());
}


Player * Editor::get_safe_player(const int n) {
	Player * result = get_player(n);
	if (not result) {
		// Create this player, but check that
		// we are in the editor. In the game, all
		// players are known from the beginning. In the
		// case of savegames, players must be set up
		// before this is ever called. Only in the editor
		// players are not always initialized
		result = add_player
			(n,
			 Player::playerLocal,
			 get_map()->get_scenario_player_tribe(n).c_str(),
			 get_map()->get_scenario_player_name(n).c_str());
		result->init(false);
	}
	return result;
}
