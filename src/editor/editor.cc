/*
 * Copyright (C) 2002-2003, 2006-2007 by the Widelands Development Team
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

Editor::Editor() : m_realtime(WLApplication::get()->get_time()) {}


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
   Map* m=new Map;
   m->create_empty_map();


   set_map(m);

   g_gr->flush(PicMod_Menu);

	Editor_Interactive eia(*this);
   // inform base, that we have something interactive
   set_iabase(&eia);

   postload();
   load_graphics();

	eia.start();
	eia.run();


   get_objects()->cleanup(this);

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
			 Player::Local,
			 get_map()->get_scenario_player_tribe(n),
			 get_map()->get_scenario_player_name(n));
		result->init(false);
	}
	return result;
}
