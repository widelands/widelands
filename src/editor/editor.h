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

#ifndef __S__EDITOR_H
#define __S__EDITOR_H

#include "editor_game_base.h"

class Editor_Interactive;

class Editor : public Editor_Game_Base {
      public:
         Editor();
         ~Editor();

         bool is_game() { return false; }
         void think();

         void run();

	virtual void player_immovable_notification (PlayerImmovable*, losegain_t) {}
	virtual void player_field_notification (const FCoords&, losegain_t) {}
	
      private:
         int m_realtime;
         Editor_Interactive* m_eia;
};

#endif // __S__EDITOR_H

