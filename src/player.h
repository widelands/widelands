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

#include "game.h"
#include "map.h"


/** class Player
 *
 * Pleease... no more pure virtual nonsense.
 * What we really need is a Player class that stores e.g. score
 * and diplomacy of a player.
 *
 * These Player classes should be controlled via the cmd queue.
 * Commands are inserted by:
 *  - local player
 *  - network packets
 *  - AI code which is invoked from Game, _not_ from Player
 * So basically the Game knows whether a player is controlled
 * locally, remotely or by AI.
 *                      -- Nicolai
 */
class Player {
   friend class Game;
   
   public:
		enum {
			playerLocal = 0,
			//playerRemote,
			playerAI
		};
		
      Player(Game* g, int type);
      ~Player(void);

      inline int get_type() { return m_type; }
      
		// get functions
      inline bool is_field_seen(int i) { return (*seen_fields)[i]; }
      inline bool is_field_seen(int x, int y) { return (*seen_fields)[y*game->get_map()->get_w() + x]; }
		
   private:
      // set functions
      inline void set_field_seen(int i, bool t) { (*seen_fields)[i]=t; }
      inline void set_field_seen(int x, int y, bool t) { 
         (*seen_fields)[y*game->get_map()->get_w() + x]=t; 
      }
      
      std::bit_vector* seen_fields; 
      Game* game; 
		int m_type;
      
		// regent data: name, pics so on
      Tribe_Descr* tribe; // buildings, wares, workers, sciences
};

#endif 
