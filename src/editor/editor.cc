/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

