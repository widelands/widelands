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


class Road;

/** class Player
 *
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
   friend class Editor_Game_Base;
   
   public:
		enum {
			playerLocal = 0,
			//playerRemote,
			playerAI
		};
		
      Player(Editor_Game_Base* g, int type, int plnum, Tribe_Descr* tribe, const uchar *playercolor);
      ~Player(void);

		inline Editor_Game_Base *get_game() const { return m_egbase; }
      inline int get_type() const { return m_type; }
		inline int get_player_number() const { return m_plnum; }
		inline const RGBColor* get_playercolor() const { return m_playercolor; }
		inline Tribe_Descr *get_tribe() const { return m_tribe; }
		
		void init_for_game(Game*);
		
		int get_buildcaps(Coords coords);
		
		// See area
		inline bool is_field_seen(int i) { return (*seen_fields)[i]; }
		inline bool is_field_seen(Coords c) { return (*seen_fields)[c.y*m_egbase->get_map()->get_width() + c.x]; }
		inline bool is_field_seen(int x, int y) { return is_field_seen(Coords(x, y)); }
		inline std::vector<bool>* get_visibility() { return seen_fields; }
		
		void set_area_seen(int x, int y, uint area, bool on);
		
		// Player commands
		void build_flag(Coords c);
		void rip_flag(Coords c);
		void build_road(const Path *path);
		void remove_road(Road *road);

   private:
      // set functions
      inline void set_field_seen(int i, bool t) { (*seen_fields)[i]=t; }
      inline void set_field_seen(int x, int y, bool t) { 
			(*seen_fields)[y*m_egbase->get_map()->get_width() + x]=t; 
      }
      
      Editor_Game_Base*				m_egbase; 
		int				m_type;
		int				m_plnum;
      Tribe_Descr*	m_tribe; // buildings, wares, workers, sciences
		RGBColor			m_playercolor[4];
		
      std::vector<bool>* seen_fields; 
      
		// regent data: name, pics so on
};

#endif 
