/*
 * Copyright (C) 2004 by The Widelands Development Team
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

#ifndef __COMPPLAYER_H
#define __COMPPLAYER_H

#include "game.h"

class Player;
class PlayerImmovable;

class Computer_Player {
	public:
		Computer_Player (Game *g, uchar pln);
		~Computer_Player ();

		void think ();
		
		void gain_immovable (PlayerImmovable*);
		void lose_immovable (PlayerImmovable*);

		inline Game *get_game() { return m_game; }
		inline uchar get_player_number(void) { return m_player_number; }
		inline Player *get_player() { assert(m_game); return m_game->get_player(m_player_number) ; }

	private:
		void gain_building (Building*);
		void lose_building (Building*);
		
		Game*		m_game;
		uchar		m_player_number;
};


#endif // __COMPPLAYER_H
