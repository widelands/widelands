/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include "constants.h"
#include "descr_maintainer.h"
#include "geometry.h"
#include "types.h"

class Battle;
class Bob;
class Building;
class Building_Descr;
class Immovable;
class Interactive_Base;
class Map;
class Object_Manager;
class Player;
class Tribe_Descr;

class Editor_Game_Base {
   friend class Interactive_Base;
   friend class Game_Game_Class_Data_Packet;

   public:
      Editor_Game_Base();
      virtual ~Editor_Game_Base();

      void set_map(Map* map);
      inline Map *get_map() { return m_map; }
      inline Object_Manager* get_objects() { return m_objects; }

      void unconquer_area(uchar playernr, Coords coords);
      void conquer_area(uchar playernr, Coords coords, Building_Descr*);
      void conquer_area_no_building(uchar playernr, Coords coords, int radius);

      // logic handler func
      virtual void think() = 0;

      // Player commands
      void remove_player(int plnum);
      void add_player(int plnum, int type, const char* tribe, const char* name);
	Player * get_player(const int n) const
	{assert(n >= 1 and n <= MAX_PLAYERS); return m_players[n-1];}
      Player* get_safe_player(int n);

      virtual bool is_game() = 0;

      // loading stuff
      void postload();
      void load_graphics();
      virtual void cleanup_for_load(bool flush_graphics=true, bool flush_animations=true);

      // warping stuff. instantly creating map_objects
      Building* warp_building(Coords, char owner, int idx);
		Building* warp_constructionsite(Coords c, char owner, int idx, int oldid=-1);
      Bob *create_bob(Coords c, int idx, Tribe_Descr* = 0);
      Immovable* create_immovable(Coords c, int idx, Tribe_Descr*);
		Immovable* create_immovable(Coords c, std::string name, Tribe_Descr*);
      Battle*    create_battle ();

      std::vector<int> get_battle_serials() { return m_battle_serials; }
      inline int get_gametime(void) { return m_gametime; }
		Interactive_Base* get_iabase() { return m_iabase; }

		// safe system for storing pointers to non-Map_Object C++ objects
		// unlike objects in the Object_Manager, these pointers need not be
		// synchronized across the network, and they are not saved in savegames
		uint add_trackpointer(void* ptr);
		void* get_trackpointer(uint serial);
		void remove_trackpointer(uint serial);

      // Manually load a tribe into memory. Used by the editor
      void manually_load_tribe(const char* tribe);
      // Get a tribe from the loaded list, when available
      Tribe_Descr * get_tribe(const char * const tribe) const;

	enum losegain_t { LOSE=0, GAIN };
	virtual void player_immovable_notification (PlayerImmovable*, losegain_t)=0;
	virtual void player_field_notification (const FCoords&, losegain_t)=0;

      // Military stuff
   std::vector<Coords>* get_attack_points(uchar player);

   virtual void make_influence_map ();

      /// Returns the influence value of one position (a) with the radius (radius) about (b)
   virtual int calc_influence (Coords a, Coords b, int radius);

   protected:
      // next function is used to update the current gametime,
      // for queue runs e.g.
      inline int* get_game_time_pointer(void) { return &m_gametime; }
      inline void set_iabase(Interactive_Base* b) { m_iabase=b; }

   private:
      struct Conquer_Info {
         uchar  player;
         Coords middle_point;
         ushort    area;
      };
      std::vector<Conquer_Info> m_conquer_info;
      void do_conquer_area(uchar playernr, Coords coords, int radius, bool conquer);

   private:
		void cleanup_playerimmovables_area(Coords coords, int radius);

      int m_gametime;
      Player*							m_players[MAX_PLAYERS];
      Object_Manager*				m_objects;
      std::vector<Tribe_Descr*>	m_tribes;
      Interactive_Base*          m_iabase;
      Map*								m_map;

		uint								m_lasttrackserial;
		std::map<uint, void*>		m_trackpointers;
      // I know that this fucks, ideas ?
#define MAX_X     512
#define MAX_Y     512
public:
      int  m_conquer_map[MAX_PLAYERS+1][MAX_X][MAX_Y]; // m_conquer_map[playernr][x][y] = [quantity of influence]
                                                         // The playernr 0 is the REAL OWNER
      std::vector<int>           m_battle_serials;    // The serials of the battles only used to load/save
};

extern const uchar g_playercolors[MAX_PLAYERS][12];

#endif // __S__EDITOR_GAME_BASE_H
