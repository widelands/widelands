/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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

struct Editor : public Editor_Game_Base {
         Editor();
         ~Editor();

         void think();

         void run();

	Player * get_safe_player(const int n);
	virtual void player_immovable_notification (PlayerImmovable*, losegain_t) {}
	virtual void player_field_notification (const FCoords&, losegain_t) {}

private:
         int m_realtime;
};

#endif // __S__EDITOR_H
