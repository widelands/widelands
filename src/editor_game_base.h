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

#include "descr_maintainer.h"
#include "instances.h"
#include "tribe.h"
#include "cmd_queue.h"
#include "ware.h"

class Interactive_Base;

class Editor_Game_Base {
   friend class Interactive_Base;

   public:
      Editor_Game_Base(); 
      virtual ~Editor_Game_Base(); 
	
      void set_map(Map* map);
      inline Map *get_map() { return m_map; }
      inline Object_Manager* get_objects() { return m_objects; }
	
	
      void conquer_area(uchar playernr, Coords coords, int radius);
      void recalc_for_field(Coords coords, int radius = 0);
	
      // logic handler func
      virtual void think() = 0;	
	
      // Player commands
      void remove_player(int plnum);
      void add_player(int plnum, int type, const char* tribe, const uchar *playercolor);
      inline Player* get_player(int n) { assert(n>=1 && n<=MAX_PLAYERS); return m_players[n-1]; }
  
      virtual bool is_game() = 0; 
 
      // Ware stuff
      inline int get_ware_id(const char *name) { return m_wares.get_index(name); }
      inline int get_nrwares() const { return m_wares.get_nitems(); }
      int get_safe_ware_id(const char *name);
      inline Ware_Descr *get_ware_description(int id) { return m_wares.get(id); }
	
      // loading stuff
      void postload();
      void load_graphics();

      // warping stuff. instantly creating map_objects
      Building* warp_building(int x, int y, char owner, int idx);
		Building* warp_constructionsite(int x, int y, char owner, int idx);
      Bob *create_bob(int x, int y, int idx);
      Immovable* create_immovable(int x, int y, int idx);
		Immovable* create_immovable(Coords c, std::string name);

      inline int get_gametime(void) { return m_gametime; }

   protected:
      // next function is used to update the current gametime,
      // for queue runs e.g.
      inline int* get_game_time_pointer(void) { return &m_gametime; }
      inline void set_iabase(Interactive_Base* b) { m_iabase=b; }

   private:
      void init_wares();

      int m_gametime;
      Player*							m_players[MAX_PLAYERS];
      Object_Manager*				m_objects;
      std::vector<Tribe_Descr*>	m_tribes;
      Descr_Maintainer<Ware_Descr>	m_wares;
      Interactive_Base*          m_iabase;
      Map*								m_map;
};

#endif // __S__EDITOR_GAME_BASE_H
