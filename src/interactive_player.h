/*
 * Copyright (C) 2002, 2003 by The Widelands Development Team
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

#ifndef __S__INTPLAYER_H
#define __S__INTPLAYER_H

#include "game.h"
#include "interactive_base.h"

class CoordPath;
class MiniMap;
class Map_View;
class Player;
class UITextarea;
class UIWindow;

/** class Interactive_Player
 *
 * This is the interactive player. this one is
 * responsible to show the correct map
 * to the player and draws the user interface,
 * cares for input and so on.
 */
class Interactive_Player : public Interactive_Base {
   friend class Game_Interactive_Player_Data_Packet;
	
   public:
		Interactive_Player(Game *g, uchar pln);
		~Interactive_Player(void);

		virtual void think();

		void start();

		void exit_game_btn();
		void main_menu_btn();
		void toggle_buildhelp();

		void field_action();

		bool handle_key(bool down, int code, char c);

		inline Game *get_game() { return m_game; }
		inline uchar get_player_number(void) { return m_player_number; }
		inline Player *get_player() { assert(m_game); return m_game->get_player(m_player_number) ; }

      // for savegames
      void set_player_number(uint plrn);
      
      // Visibility for drawing
      std::vector<bool>* get_visibility(void);
     
      // For ware production statistics (only needed for the interactive player)
      void ware_produced(uint id);
      void next_ware_production_period( void );
      const std::vector<uint>* get_ware_production_statistics( int ware );
      
   private:
      Game*		m_game;
      uchar		m_player_number;

      UITextarea*	m_label_speed;

      UIUniqueWindowRegistry	m_mainmenu;
      UIUniqueWindowRegistry	m_fieldaction;

      std::vector<uint> m_current_statistics;
      std::vector< std::vector<uint> > m_ware_productions; 
      uint  m_last_stats_update;    
};


#endif // __S__INTPLAYER_H
