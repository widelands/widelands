/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#ifndef __S__EDITOR_GAME_BASE_H
#define __S__EDITOR_GAME_BASE_H

class Map;
class Object_Manager;
class Interactive_Player;
class Player;
class Tribe_Descr;

class Editor_Game_Base {
   public:
      Editor_Game_Base() { }
      virtual ~Editor_Game_Base() { }
	
      inline Map *get_map() { return m_map; }
      inline Object_Manager* get_objects() { return m_objects; }
	
      
      void conquer_area(uchar playernr, Coords coords, int radius);
      void recalc_for_field(Coords coords, int radius = 0);
	
      // Player commands
      void remove_player(int plnum);
      void add_player(int plnum, int type, const char* tribe, const uchar *playercolor);
      inline Player* get_player(int n) { assert(n>=1 && n<=MAX_PLAYERS); return m_players[n-1]; }
  
      virtual bool is_game() { cerr <<"ERR: Editor_Game_Base::is_game() called!"; return 0; };
 

      // TODO!!
      virtual int get_gametime(void) { cerr <<"ERR: Editor_Game_Base::get_gametime() called!"; return 0; };
      virtual Interactive_Player* get_ipl(void)  { cerr <<"ERR: Editor_Game_Base::get_ipl() called!"; return 0; };
; 
 
   
   protected:
      Player*							m_players[MAX_PLAYERS];
      Object_Manager*				m_objects;
      std::vector<Tribe_Descr*>	m_tribes;
      Map*								m_map;
};

#endif // __S__EDITOR_GAME_BASE_H
