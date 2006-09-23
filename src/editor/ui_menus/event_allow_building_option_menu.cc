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

#include <stdio.h>
#include "editor.h"
#include "editorinteractive.h"
#include "error.h"
#include "event_allow_building.h"
#include "event_allow_building_option_menu.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "player.h"
#include "tribe.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_textarea.h"
#include "ui_window.h"
#include "util.h"

Event_Allow_Building_Option_Menu::Event_Allow_Building_Option_Menu(Editor_Interactive* parent, Event_Allow_Building* event) :
   UIWindow(parent, 0, 0, 200, 280, _("Event Option Menu").c_str()) {
   m_parent=parent;
   m_event=event;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Allow Building Event Options"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   m_player=m_event->get_player();
   m_building=-1;
   const int offsx=5;
   const int offsy=25;
   int spacing=5;
   int posx=offsx;
   int posy=offsy;

   if(m_player<1) m_player=1;

   // Fill the building infos
   Tribe_Descr* tribe = m_parent->get_editor()->get_tribe(m_parent->get_map()->get_scenario_player_tribe(m_player).c_str());
   int i=0;
   if(tribe) {
      for(i=0; i<tribe->get_nrbuildings(); i++) {
         Building_Descr* b=tribe->get_building_descr(i);
         if(!b->get_buildable() && !b->get_enhanced_building()) continue;
         std::string name=b->get_name();
         std::string event_name=m_event->get_building();
         if(name==event_name) m_building=m_buildings.size();
         m_buildings.push_back(name);
      }
   }

   // Name editbox
   new UITextarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+60, posy, get_inner_w()-2*spacing-60, 20, 0, 0);
   m_name->set_text( event->get_name() );
   posy+=20+spacing;

   // Player
   new UITextarea(this, spacing, posy, 70, 20, _("Player: "), Align_CenterLeft);
   m_player_ta=new UITextarea(this, spacing+70, posy, 20, 20, "2", Align_Center);
   UIButton* b=new UIButton(this, spacing+90, posy, 20, 20, 0, 15);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Allow_Building_Option_Menu::clicked);
   b=new UIButton(this, spacing+110, posy, 20, 20, 0, 16);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Allow_Building_Option_Menu::clicked);
   posy+=20+spacing;

   // Building
   new UITextarea(this, spacing, posy, 70, 20, _("Building: "), Align_CenterLeft);
   b=new UIButton(this, spacing+70, posy, 20, 20, 0, 23);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Allow_Building_Option_Menu::clicked);
   b=new UIButton(this, spacing+90, posy, 20, 20, 0, 24);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Allow_Building_Option_Menu::clicked);
   posy+=20+spacing;
   m_building_ta=new UITextarea(this, 0, posy, get_inner_w(), 20, _("Headquarters"), Align_Center);
   posy+=20+spacing;

   // Enable
   new UITextarea(this, spacing, posy, 150, 20, _("Allow Building: "), Align_CenterLeft);
   m_allow=new UICheckbox(this, spacing+150, posy);
   m_allow->set_state(m_event->get_allow());
   posy+=20+spacing;

   // Ok/Cancel Buttons
   posy+=spacing; // Extra space
   posx=(get_inner_w()/2)-60-spacing;
   b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title(_("Ok").c_str());
   b->clickedid.set(this, &Event_Allow_Building_Option_Menu::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title(_("Cancel").c_str());
   b->clickedid.set(this, &Event_Allow_Building_Option_Menu::clicked);

   set_inner_size(get_inner_w(), posy+20+spacing);
   center_to_parent();
   update();
}

/*
 * cleanup
 */
Event_Allow_Building_Option_Menu::~Event_Allow_Building_Option_Menu(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Event_Allow_Building_Option_Menu::handle_mouseclick
(uint btn, bool down, int, int)
{
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable

}

/*
 * a button has been clicked
 */
void Event_Allow_Building_Option_Menu::clicked(int i) {
   switch(i) {
      case 0:
         {
            // Cancel has been clicked
            end_modal(0);
            return;
         }
         break;

      case 1:
         {
            // ok button
            if(m_name->get_text())
               m_event->set_name( m_name->get_text() );
            if(m_event->get_player()!=m_player && m_event->get_player()!=-1)
               m_parent->unreference_player_tribe(m_event->get_player(), m_event);
            if(m_event->get_player()!=m_player) {
               m_event->set_player(m_player);
               m_parent->reference_player_tribe(m_player, m_event);
            }
            m_event->set_building(m_buildings[m_building].c_str());
            m_event->set_allow(m_allow->get_state());
            m_parent->set_need_save(true);
            end_modal(1);
            return;
         }
         break;

      case 15: m_player++; break;
      case 16: m_player--; break;

      case 23: m_building++; break;
      case 24: m_building--; break;
   }
   update();
}

/*
 * update function: update all UI elements
 */
void Event_Allow_Building_Option_Menu::update(void) {
   if(m_player<=0) m_player=1;
   if(m_player>m_parent->get_map()->get_nrplayers()) m_player=m_parent->get_map()->get_nrplayers();

   if(m_building<0) m_building=0;
   if(m_building>=static_cast<int>(m_buildings.size())) m_building=m_buildings.size()-1;

   std::string curbuild=_("<invalid player tribe>");
   if(!m_buildings.size()) {
      m_player=-1;
      m_building=-1;
   } else {
      curbuild=m_buildings[m_building];
   }

   char buf[200];
   sprintf(buf, "%i", m_player);
   m_player_ta->set_text(buf);

   sprintf(buf, "\"%s\"", curbuild.c_str());
   m_building_ta->set_text(buf);
}
