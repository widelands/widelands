/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "error.h"
#include "event_move_view.h"
#include "filesystem.h"
#include "game.h"
#include "interactive_base.h"
#include "map.h"
#include "profile.h"

static const int EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Move_View::Event_Move_View(void) {
   set_name(_("Move View"));
   set_coords(Coords(0,0));
}

Event_Move_View::~Event_Move_View(void) {
}

/*
 * reinitialize
 */
void Event_Move_View::reinitialize(Game* g) {
}

/*
 * File Read, File Write
 */
void Event_Move_View::Read(Section* s, Editor_Game_Base* egbase) {
   int version=s->get_safe_int("version");
   if(version == EVENT_VERSION) {
      m_pt.x=s->get_safe_int("point_x");
      m_pt.y=s->get_safe_int("point_y");
      return;
   }
   throw wexception("Move View Event with unknown/unhandled version %i in map!\n", version);
}

void Event_Move_View::Write(Section* s, Editor_Game_Base *egbase) {
   // the version
   s->set_int("version", EVENT_VERSION);

   // Point
   s->set_int("point_x", m_pt.x);
   s->set_int("point_y", m_pt.y);
   // done
}

/*
 * check if trigger conditions are done
 */
Event::State Event_Move_View::run(Game* game) {
   assert(m_pt.x!=-1 && m_pt.y!=-1);

   Interactive_Base* iab=game->get_iabase();
   iab->move_view_to(m_pt.x, m_pt.y);

   m_state = DONE;
   return m_state;
}


