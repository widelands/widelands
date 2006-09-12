/*
 * Copyright (C) 2002-2003, 2006 by the Widelands Development Team
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

#include <vector>
#include "game.h"
#include "interactive_base.h"
#include "network.h" // For chat

class Player;
class UIMultiline_Textarea;
class UITextarea;

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
      struct Building_Stats {
         bool is_constructionsite;
         Coords pos;
      };
      typedef std::vector<std::vector< Building_Stats > > BuildingStats ;
      struct General_Stats {
         std::vector< uint > land_size;
         std::vector< uint > nr_workers;
         std::vector< uint > nr_buildings;
         std::vector< uint > nr_wares;
         std::vector< uint > productivity;
         std::vector< uint > nr_kills;
         std::vector< uint > miltary_strength;
      };

      struct Game_Main_Menu_Windows {
         UIUniqueWindowRegistry loadgame;
         UIUniqueWindowRegistry savegame;
         UIUniqueWindowRegistry readme;
         UIUniqueWindowRegistry keys;
         UIUniqueWindowRegistry authors;
         UIUniqueWindowRegistry licence;
         UIUniqueWindowRegistry options;

         UIUniqueWindowRegistry building_stats;
         UIUniqueWindowRegistry general_stats;
         UIUniqueWindowRegistry ware_stats;
         UIUniqueWindowRegistry stock;

         UIUniqueWindowRegistry mission_objectives;
         UIUniqueWindowRegistry chat;
         UIUniqueWindowRegistry objectives;
      };

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

		Game * get_game() const {return m_game;}
		uchar get_player_number() const {return m_player_number;}
		Player * get_player() const
		{assert(m_game); return m_game->get_player(m_player_number);}

      // for savegames
      void set_player_number(uint plrn);

      // Visibility for drawing
      std::vector<bool>* get_visibility(void);

      // For ware production statistics (only needed for the interactive player)
      void ware_produced(const Descr_Maintainer<Item_Ware_Descr>::Index id);
      void next_ware_production_period( void );
      const std::vector<uint>* get_ware_production_statistics( int ware );

      // For statistics mainly, we keep track of buildings
      void gain_immovable(PlayerImmovable* );
      void lose_immovable(PlayerImmovable* );
      const std::vector< Building_Stats >&  get_building_statistics( int i ) { return m_building_stats[i]; }
      const std::vector< General_Stats>& get_general_statistics( void ) { return m_general_stats; }

      // For load
      virtual void cleanup_for_load( void );

      // Chat messages
      bool show_chat_overlay( void ) { return m_do_chat_overlays; }
      void set_show_chat_overlay( bool t ) { m_do_chat_overlays = t; }
      const std::vector< NetGame::Chat_Message >* get_chatmsges( void ) { return &m_chatmsges; }

   private:
      struct Overlay_Chat_Messages {
         NetGame::Chat_Message msg;
         uint starttime;
      };

   private:
      Game*		m_game;
      uchar		m_player_number;

      UITextarea*	m_label_speed;
      UIMultiline_Textarea*	m_chat_messages;
      UITextarea*	m_type_message;

      UIUniqueWindowRegistry	m_mainmenu;
      UIUniqueWindowRegistry	m_fieldaction;
      Game_Main_Menu_Windows  m_mainm_windows;

      std::vector<uint> m_current_statistics;
      std::vector< std::vector<uint> > m_ware_productions;
      uint  m_last_stats_update;

      BuildingStats m_building_stats;
      std::vector< General_Stats > m_general_stats;

      // Chat message stack
      std::vector< NetGame::Chat_Message > m_chatmsges;
      std::vector< Overlay_Chat_Messages > m_show_chatmsg;
      bool m_do_chat_overlays;
      bool m_is_typing_msg; // Is the user typing a chat message
      std::string m_typed_message;

   private:
      void sample_statistics( void );
};


#endif // __S__INTPLAYER_H
