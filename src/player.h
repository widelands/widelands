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

#ifndef __S__PLAYER_H
#define __S__PLAYER_H

//
// This defines a player class. 
// A player is 
//  a) the one who has the track of all it's buildings, units and wares
//  b) the logic (e.g network code or AI code) 
//  c) the one who triggers commands to the game
//
class Player {
   public:
		enum {
			playerLocal = 0,
			//playerRemote,
			playerAI
		};
		
		Player(int type) { m_type = type; }
		~Player() { }
	
		inline int get_type() { return m_type; }
		
   private:
		int m_type;
      
		// regent data: name, pics so on
      Tribe_Descr* tribe; // buildings, wares, workers, sciences
};

#endif 
