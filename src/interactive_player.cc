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

#include "building.h"
#include "building_statistics_menu.h"
#include "cmd_queue.h"
#include "constructionsite.h"
#include "fieldaction.h"
#include "font_handler.h"
#include "game_loader.h"
#include "game_main_menu.h"
#include "general_statistics_menu.h"
#include "i18n.h"
#include "interactive_player.h"
#include "keycodes.h"
#include "immovable.h"
#include "network.h"
#include "mapview.h"
#include "player.h"
#include "productionsite.h"
#include "ui_editbox.h"
#include "ui_button.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"
#include "overlay_manager.h"
#include "soldier.h"
#include "stock_menu.h"
#include "transport.h"
#include "tribe.h"
#include "util.h"
#include "ware_statistics_menu.h"
#include "wlapplication.h"

#define CHAT_DISPLAY_TIME 5000 // Show chat messages as overlay for 5 seconds

/*
==============================================================================

Interactive_Player IMPLEMENTATION

==============================================================================
*/

// This function is the callback for recalculation of field overlays
int Int_Player_overlay_callback_function(const TCoords c, void* data, int) {
	return
		static_cast<const Interactive_Player * const>(data)->get_player()->
		get_buildcaps(c);
}


/*
===============
Interactive_Player::Interactive_Player

Initialize
===============
*/
Interactive_Player::Interactive_Player(Game *g, uchar plyn) : Interactive_Base(g)
{

   // Setup all screen elements
	m_game = g;
	set_player_number(plyn);

	Map_View* mview;
   mview = new Map_View(this, 0, 0, get_w(), get_h(), this);
	mview->warpview.set(this, &Interactive_Player::mainview_move);
	mview->fieldclicked.set(this, &Interactive_Player::field_action);
   set_mapview(mview);

	// user interface buttons
	int x = (get_w() - (4*34)) >> 1;
	int y = get_h() - 34;
	UIButton *b;

	/* temp (should be toggle messages)
	b = new UIButton(this, x, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::exit_game_btn);
	b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_exit_game.png" ));
	b->set_tooltip(_("Exit game").c_str());
	// temp */

	b = new UIButton(this, x+34, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::main_menu_btn);
	b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_menu.png" ));
	b->set_tooltip(_("Menu").c_str());

	b = new UIButton(this, x+68, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_minimap);
	b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_minimap.png" ));
	b->set_tooltip(_("Minimap").c_str());

	b = new UIButton(this, x+102, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_buildhelp);
	b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_toggle_buildhelp.png" ));
	b->set_tooltip(_("Buildhelp").c_str());

	// Speed info
	m_label_speed = new UITextarea(this, get_w(), 0, 0, 0, "", Align_TopRight);

   // Chat Messages
   m_chat_messages = new UIMultiline_Textarea(this, 10, 25, get_inner_w(), get_inner_h(), "", Align_TopLeft);
   m_type_message = new UITextarea(this, 10, get_inner_h()-50, get_inner_w(), 50, "", Align_TopLeft);
   m_last_stats_update = 0;

   m_is_typing_msg = false;
   m_do_chat_overlays = true;
}

/*
===============
Interactive_Player::~Interactive_Player

cleanups
===============
*/
Interactive_Player::~Interactive_Player(void)
{
}

/*
 * Sample all satistics data
 */
void Interactive_Player::sample_statistics( void ) {
   // Update ware stats
   next_ware_production_period();

   // Update general stats
   Map* map = get_game()->get_map();
   std::vector< uint > land_size; land_size.resize( map->get_nrplayers() );
   std::vector< uint > nr_buildings; nr_buildings.resize( map->get_nrplayers() );
   std::vector< uint > nr_kills; nr_kills.resize( map->get_nrplayers() );
   std::vector< uint > miltary_strength; miltary_strength.resize( map->get_nrplayers() );
   std::vector< uint > nr_workers; nr_workers.resize( map->get_nrplayers() );
   std::vector< uint > nr_wares; nr_wares.resize( map->get_nrplayers() );
   std::vector< uint > productivity; productivity.resize( map->get_nrplayers() );

   std::vector< uint > nr_production_sites; nr_production_sites.resize( map->get_nrplayers() );

   // We walk the map, to gain all needed informations
   for( ushort y = 0; y < map->get_height(); y++) {
      for(ushort x = 0; x < map->get_width(); x++) {
         Field* f = map->get_field( Coords( x, y ) );

         // First, ownership of this field
         if( f->get_owned_by() )
            land_size[ f->get_owned_by()-1 ]++;

         // Get the immovable
         BaseImmovable* imm = f->get_immovable();
         if(imm && imm->get_type() == Map_Object::BUILDING) {
            Building* build = static_cast<Building*>(imm);
            if( build->get_position() == Coords(x,y)) { // only main location is intresting

               // Ok, count the building
               nr_buildings[ build->get_owner()->get_player_number() - 1 ]++;

               // If it is a productionsite, add it's productivity
               if( build->get_building_type() == Building::PRODUCTIONSITE ) {
                  nr_production_sites[  build->get_owner()->get_player_number() - 1 ]++;
                  productivity[  build->get_owner()->get_player_number() - 1 ] += static_cast<ProductionSite*>( build )->get_statistics_percent();
               }
            }
         }


         // Now, walk the bobs
         if( f->get_first_bob() ) {
            Bob* b = f->get_first_bob();
            do {
               if( b->get_bob_type() == Bob::WORKER ) {
                  Worker* w = static_cast<Worker*>(b);

                  switch( w->get_worker_type() ) {
                     case Worker_Descr::SOLDIER:
                        {
                           Soldier* s = static_cast<Soldier*>(w);
                           uint calc_level = s->get_level(atrTotal) + 1; // So that level 0 loosers also count something
                           miltary_strength[ s->get_owner()->get_player_number() -1 ] += calc_level;
                        }
                        break;

                     default: break;
                  }

               }
            } while( (b = b->get_next_bob() ) );
         }
      }
   }

   // Number of workers / wares
   for( uint i = 0; i < map->get_nrplayers(); i++) {
      Player* plr = get_game()->get_player(i+1);

      uint wostock = 0;
      uint wastock = 0;

      for( uint j = 0; plr && j < plr->get_nr_economies(); j++) {
         Economy* eco = plr->get_economy_by_number( j );

         for( int wareid = 0; wareid < plr->get_tribe()->get_nrwares(); wareid++)
            wastock += eco->stock_ware( wareid );
         for( int workerid = 0; workerid < plr->get_tribe()->get_nrworkers(); workerid++) {
            if( plr->get_tribe()->get_worker_descr( workerid )->get_worker_type() == Worker_Descr::CARRIER)
               continue;
            wostock += eco->stock_worker( workerid );
         }
      }
      nr_wares[ i ] = wastock;
      nr_workers[ i ] = wostock;
   }

   // Now, divide the statistics
   for( uint i = 0; i < map->get_nrplayers(); i++) {
      if( productivity[ i ] )
         productivity[ i ] /= nr_production_sites[ i ];
   }

   // Now, push this on the general statistics
   m_general_stats.resize( map->get_nrplayers() );
   for( uint i = 0; i < map->get_nrplayers(); i++) {
      m_general_stats[i].land_size.push_back( land_size[i] );
      m_general_stats[i].nr_buildings.push_back( nr_buildings[i] );
      m_general_stats[i].nr_kills.push_back( nr_kills[i] );
      m_general_stats[i].miltary_strength.push_back( miltary_strength[i] );
      m_general_stats[i].nr_workers.push_back( nr_workers[i] );
      m_general_stats[i].nr_wares.push_back( nr_wares[i]  );
      m_general_stats[i].productivity.push_back( productivity[i] );
   }

   // Update last stats time
   m_last_stats_update = m_game->get_gametime();
}

/*
===============
Interactive_Player::think

Update the speed display, check for chatmessages.
===============
*/
void Interactive_Player::think()
{
   Interactive_Base::think();

   // Draw speed display
   int speed = m_game->get_speed();
   char buf[32] = "";

   if (!speed)
	   strcpy(buf, _("PAUSE").c_str());
   else if (speed > 1)
      snprintf(buf, sizeof(buf), "%ix", speed);

   m_label_speed->set_text(buf);

   // Reset our statistics counting
   if(m_game->get_gametime()-m_last_stats_update > STATISTICS_SAMPLE_TIME)
      sample_statistics();

   // Check for chatmessages
   NetGame* ng = m_game->get_netgame();
   if(ng && ng->have_chat_message()) {
      NetGame::Chat_Message t = ng->get_chat_message();
      m_chatmsges.push_back( t );

      Overlay_Chat_Messages ov;
      ov.msg =  t;
      ov.starttime = WLApplication::get()->get_time();
      m_show_chatmsg.push_back( ov );
   }

   // If we have chat messages to overlay, show them now
   m_chat_messages->set_text("");
   if( m_show_chatmsg.size() && m_do_chat_overlays ) {
      std::string str;
      for( uint i = 0; i < m_show_chatmsg.size(); i++) {
         const NetGame::Chat_Message& t = m_show_chatmsg[i].msg;
         str += get_game()->get_player(t.plrnum)->get_name();
         str += ": ";
         str += t.msg;
         str += "\n";

	 if( WLApplication::get()->get_time() - m_show_chatmsg[i].starttime > CHAT_DISPLAY_TIME ) {
            m_show_chatmsg.erase( m_show_chatmsg.begin() + i);
            i--;
         }
      }

      m_chat_messages->set_text( str.c_str() );
   }

   // Is the user typing a message?
   m_type_message->set_text("");
   if( m_is_typing_msg ) {
      std::string text = _("Message: ");
      text += m_typed_message;
      m_type_message->set_text( text.c_str() );
   }
}


/*
===============
Interactive_Player::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Interactive_Player::start()
{
	int mapw;
	int maph;

	mapw = 0;
	maph = 0;

   get_map()->get_overlay_manager()->show_buildhelp(false);
   get_map()->get_overlay_manager()->register_overlay_callback_function(&Int_Player_overlay_callback_function, static_cast<void*>(this));

   // Recalc whole map for changed owner stuff
   get_map()->recalc_whole_map();

   sample_statistics();
}

/*
 * A ware was produced
 */
void Interactive_Player::ware_produced( uint wareid ) {
   if( m_ware_productions.size() != (uint)get_player()->get_tribe()->get_nrwares() ) {
      m_ware_productions.resize( get_player()->get_tribe()->get_nrwares() );
      m_current_statistics.resize( get_player()->get_tribe()->get_nrwares() );
   }

   assert( wareid < (uint)get_player()->get_tribe()->get_nrwares() );

   m_current_statistics[wareid]++;
}

/*
 * void next_ware_production_period()
 *
 * Set the next production period
 */
void Interactive_Player::next_ware_production_period( void ) {
   if( m_ware_productions.size() != (uint)get_player()->get_tribe()->get_nrwares() ) {
      m_ware_productions.resize( get_player()->get_tribe()->get_nrwares() );
      m_current_statistics.resize( get_player()->get_tribe()->get_nrwares() );
   }

   for(uint i = 0; i < m_ware_productions.size(); i++) {
      m_ware_productions[i].push_back( m_current_statistics[i] );
      m_current_statistics[i] = 0;
   }
}

/*
 * Get current ware production statistics
 */
const std::vector<uint> * Interactive_Player::get_ware_production_statistics
(const int ware) const
{
   assert( ware < (int)m_ware_productions.size()) ;

   return &m_ware_productions[ware];
}

/** Interactive_Player::exit_game_btn(void *a)
 *
 * Handle exit button
 */
void Interactive_Player::exit_game_btn()
{
	end_modal(0);
}

/*
===============
Interactive_Player::main_menu_btn

Bring up or close the main menu
===============
*/
void Interactive_Player::main_menu_btn()
{
	if (m_mainmenu.window) delete m_mainmenu.window;
	else new GameMainMenu(*this, m_mainmenu, m_mainm_windows);
}

//
// Toggles buildhelp rendering in the main MapView
//
void Interactive_Player::toggle_buildhelp(void)
{
   get_map()->get_overlay_manager()->toggle_buildhelp();
}

/*
===============
Interactive_Player::field_action

Player has clicked on the given field; bring up the context menu.
===============
*/
void Interactive_Player::field_action()
{
	if (!get_player()->is_field_seen(get_fieldsel_pos()))
		return;

	// Special case for buildings
	BaseImmovable *imm = m_game->get_map()->get_immovable(get_fieldsel_pos());

	if (imm && imm->get_type() == Map_Object::BUILDING) {
		Building *building = (Building *)imm;

		if (building->get_owner()->get_player_number() == get_player_number()) {
			building->show_options(this);
			return;
		}
	}

	// everything else can bring up the temporary dialog
	show_field_action(this, get_player(), &m_fieldaction);
}

/*
===============
Interactive_Player::handle_key

Global in-game keypresses:
Space: toggles buildhelp
F5: reveal map
===============
*/
bool Interactive_Player::handle_key(bool down, int code, char c)
{

   if( m_is_typing_msg && down ) {
      if(c & 0x7f) {
         m_typed_message.append(1, c);
         return true;
      }
   }

	switch(code) {
	case KEY_SPACE:
		if (down)
			toggle_buildhelp();
		return true;

	case KEY_m:
		if (down)
			toggle_minimap();
		return true;

	case KEY_c:
		if (down)
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
		return true;

	case KEY_s:
		if (down)
			set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
		return true;

   case KEY_f:
      if( down )
         g_gr->toggle_fullscreen();
      return true;

	case KEY_PAGEUP:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(speed + 1);
		}
		return true;

	case KEY_PAGEDOWN:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(std::max(0, speed-1));
		}
		return true;

   case KEY_BACKSPACE:
      if( down ) {
         if( m_is_typing_msg && m_typed_message.size() ) {
            m_typed_message.erase( m_typed_message.begin() + m_typed_message.size() - 1);
            return true;
         }
      }
      break;

   case KEY_ESCAPE:
      if( down ) {
         if( m_is_typing_msg ) {
            m_is_typing_msg = false;
            m_typed_message.clear();
            return true;
         }
      }
      break;

   case KEY_RETURN:
      if( down ) {
         if( m_is_typing_msg && m_typed_message.size() ) {
            if( m_game->get_netgame() ) {
               NetGame::Chat_Message t;

               t.plrnum = get_player_number();
               t.msg = m_typed_message;
               m_game->get_netgame()->send_chat_message( t );
            }
            m_typed_message.clear();
            m_is_typing_msg = false;
         } else {
            // Begin writing a message
            m_is_typing_msg = true;
         }
      }
      return true;

#ifdef DEBUG
   // Only in debug builds
	case KEY_F5:
		if (down) {
         get_player()->set_see_all(!get_player()->get_see_all());
		}
		return true;
#endif
	}

	return false;
}

/*
 * set the player and the visibility to this
 * player
 */
void Interactive_Player::set_player_number(uint n) {
   m_player_number=n;
}


/*
 * Return our players visibility
 */
std::vector<bool>* Interactive_Player::get_visibility(void) {
   return m_game->get_player(m_player_number)->get_visibility();
}

/*
 * Gain a immovable
 */
void Interactive_Player::gain_immovable( PlayerImmovable* imm ) {
   if( imm->get_type() != BaseImmovable::BUILDING ) return;
   Building* b = static_cast<Building*>(imm);

   std::string name;
   bool is_constructionsite = false;
   if(!strcmp(b->get_name(),"constructionsite")) {
      name = static_cast<ConstructionSite*>(b)->get_building()->get_name();
      is_constructionsite = true;
   } else
      name = b->get_name();

   // Get the valid vector for this
   if( (int)m_building_stats.size() < get_player()->get_tribe()->get_nrbuildings())
      m_building_stats.resize( get_player()->get_tribe()->get_nrbuildings());

   std::vector<Building_Stats>& stat = m_building_stats[ get_player()->get_tribe()->get_building_index(name.c_str()) ];

   Building_Stats new_building;
   new_building.is_constructionsite = is_constructionsite;
   new_building.pos = b->get_position();
   stat.push_back( new_building );
}

/*
 * Loose a immovable
 */
void Interactive_Player::lose_immovable( PlayerImmovable* imm ) {
   if( imm->get_type() != BaseImmovable::BUILDING ) return;
   Building* b = static_cast<Building*>(imm);

   std::string name;
   if(!strcmp(b->get_name(),"constructionsite")) {
      name = static_cast<ConstructionSite*>(b)->get_building()->get_name();
   } else
      name = b->get_name();

   // Get the valid vector for this
   if( (int)m_building_stats.size() < get_player()->get_tribe()->get_nrbuildings())
      m_building_stats.resize( get_player()->get_tribe()->get_nrbuildings());

   std::vector<Building_Stats>& stat = m_building_stats[ get_player()->get_tribe()->get_building_index(name.c_str()) ];

   for( uint i = 0; i < stat.size(); i++ ) {
      if( stat[i].pos == b->get_position() ) {
         stat.erase( stat.begin() + i );
         return;
      }
   }

   throw wexception("Interactive_Player::loose_immovable(): A building shoud be removed at the location %i, %i, but nothing"
         " is known about this building!\n", b->get_position().x, b->get_position().y);
}

/*
 * Cleanup, so that we can load
 */
void Interactive_Player::cleanup_for_load( void ) {
   m_building_stats.clear();
   m_general_stats.clear();
}
