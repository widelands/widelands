/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "map.h"
#include "rgbcolor.h"

class Game;
class Path;
class PlayerImmovable;
class Road;
class Soldier;
class Flag;
class Tribe_Descr;

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
	friend class Game_Player_Info_Data_Packet;
	friend class Game_Player_Economies_Data_Packet;

	public:
		enum {
			playerLocal = 0,
			playerRemote,
			playerAI
		};

		Player(Editor_Game_Base* g, int type, int plnum, Tribe_Descr* tribe, const char* name, const uchar *playercolor);
		~Player(void);

		inline Editor_Game_Base *get_game() const { return m_egbase; }
		inline int get_type() const { return m_type; }
		inline int get_player_number() const { return m_plnum; }
		inline const RGBColor* get_playercolor() const { return m_playercolor; }
		inline Tribe_Descr *get_tribe() const { return m_tribe; }
      
      const char* get_name(void) { return m_name.c_str(); }
      void set_name(const char* str) { m_name=str; }

		void init(Editor_Game_Base*, bool);

		bool is_field_owned(Coords coords);
		int get_buildcaps(Coords coords);

      // For cheating
      void set_see_all(bool t) { m_see_all=t; }
      bool get_see_all(void) { return m_see_all; }
      
		// See area
		inline bool is_field_seen(int i) { if(m_see_all) return true; return seen_fields[i]; }
		inline bool is_field_seen(Coords c) { if(m_see_all) return true; return seen_fields[c.y*m_egbase->get_map()->get_width() + c.x]; }
		inline bool is_field_seen(int x, int y) { if(m_see_all) return true; return is_field_seen(Coords(x, y)); }
		inline std::vector<bool>* get_visibility() { if(m_see_all) return 0; return &seen_fields; }

		void set_area_seen(Coords c, uint area, bool on);

      // Allowed buildings
      inline bool is_building_allowed(int i) { return m_allowed_buildings[i]; }
      void allow_building(int i, bool t);

		// Player commands
		// Only to be called indirectly via CmdQueue
		void build_flag(Coords c);
		void build_road(const Path *path);
		void build(Coords c, int idx);
		void bulldoze(PlayerImmovable* imm);
		void flagaction(Flag* flag, int action);
		void start_stop_building(PlayerImmovable* imm);
      void enhance_building(PlayerImmovable* imm, int id);

      // Economy stuff
      void add_economy(Economy*);
      void remove_economy(Economy*);
      bool has_economy(Economy*);
      int get_economy_number(Economy*); // for savegames
      inline Economy* get_economy_by_number(int i) { return m_economies[i]; } // for loading
      inline uint get_nr_economies( void ) { return m_economies.size(); } 

      // Military stuff
      void drop_soldier(PlayerImmovable* imm, Soldier* worker);
      void change_soldier_capacity (PlayerImmovable* , int val);
      void change_training_options(PlayerImmovable* imm, int atr, int val);
         // Launch an attack
      void enemyflagaction(Flag* flag, int action, int param, int param2, int param3);
	private:
		bool m_see_all;
      
      // set functions
		inline void set_field_seen(int i, bool t) { seen_fields[i]=t; }
		inline void set_field_seen(Coords c, bool t) {
			seen_fields[c.y*m_egbase->get_map()->get_width() + c.x]=t;
		}

		Editor_Game_Base*				m_egbase;
		int				m_type;
		int				m_plnum;
		Tribe_Descr*	m_tribe; // buildings, wares, workers, sciences
		RGBColor			m_playercolor[4];

		std::vector<bool> seen_fields;
      std::vector<bool> m_allowed_buildings;
      std::vector<Economy*> m_economies;
      std::string    m_name; // Player name
};

#endif
