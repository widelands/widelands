/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "editorinteractive.h"
#include "editor_event_menu_edit_eventchain.h"
#include "editor_event_menu_edit_trigger_conditional.h"
#include "error.h"
#include "event.h"
#include "event_chain.h"
#include "map.h"
#include "map_event_manager.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "ui_window.h"
#include "system.h"
#include "trigger_conditional.h"
#include "util.h"
#include "wlapplication.h"

Editor_Event_Menu_Edit_EventChain::Editor_Event_Menu_Edit_EventChain(Editor_Interactive* parent, EventChain* chain) :
   UIWindow(parent, 0, 0, 505, 340, _("Edit Event Chain")) {

   m_parent=parent;
   m_event_chain = chain;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Edit Event Chain Menu"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
   const int ls_width = 200;

   // Name
   new UITextarea( this, posx, posy, 60, 20, _("Name: "), Align_CenterLeft);
   m_name = new UIEdit_Box( this, posx + 60, posy, get_inner_w()-2*spacing-60, 20, 0, 0);
   m_name->set_text( m_event_chain->get_name() );
   posy += 20 + spacing;

   // More then once
   new UITextarea( this, posx + STATEBOX_WIDTH + spacing, posy, 120, STATEBOX_HEIGHT, _("Runs multiple times"), Align_CenterLeft);
   m_morethanonce = new UICheckbox( this, posx, posy );
   m_morethanonce->set_state( m_event_chain->get_repeating() );
   posy += STATEBOX_HEIGHT + spacing;
   const int lsoffsy = posy;

   // Event List
   new UITextarea(this, posx, lsoffsy, _("Events: "), Align_Left);
   m_events= new UIListselect(this, spacing, lsoffsy+20, ls_width, get_inner_h()-lsoffsy-55);
   m_events->selected.set(this, &Editor_Event_Menu_Edit_EventChain::cs_selected);
   m_events->double_clicked.set(this, &Editor_Event_Menu_Edit_EventChain::cs_double_clicked);
   posx += ls_width + spacing;

   posy = 75;
   UIButton* b = new UIButton(this, posx, posy, 80, 20, 0, 10);
   b->set_title(_("Conditional"));
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);
   posy += 20 + spacing + spacing;
   b = new UIButton(this, posx, posy, 80, 20, 0, 11);
   b->set_title(_("New Event"));
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);
   posy += 20 + spacing + spacing;
   b = new UIButton(this, posx, posy, 80, 20, 0, 20);
   b->set_title("<-");
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);
   posy += 20 + spacing + spacing;
   b->set_enabled( false );
   m_insert_btn = b;
   b = new UIButton(this, posx, posy, 80, 20, 0, 21);
   b->set_title(_("Delete"));
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);
   b->set_enabled( false );
   m_delete_btn = b;
   posy += 20 + spacing + spacing + spacing;

   b = new UIButton(this, posx+5, posy, 24, 24, 0, 30);
   b->set_pic(g_gr->get_picture( PicMod_UI, "pics/scrollbar_up.png"));
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);
   b->set_enabled( false );
   m_mvup_btn = b;
   b = new UIButton(this, posx+51, posy, 24, 24, 0, 31);
   b->set_pic(g_gr->get_picture( PicMod_UI, "pics/scrollbar_down.png"));
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);
   b->set_enabled( false );
   m_mvdown_btn = b;
   posy += 24 + spacing + spacing;

   posx += 80 + spacing;
   new UITextarea(this, posx, lsoffsy, _("Available Events: "), Align_Left);
   m_available_events=new UIListselect(this, posx, lsoffsy+20, ls_width, get_inner_h()-lsoffsy-55);
   m_available_events->selected.set(this, &Editor_Event_Menu_Edit_EventChain::tl_selected);
   m_available_events->double_clicked.set(this, &Editor_Event_Menu_Edit_EventChain::tl_double_clicked);
   for(int i=0; i < parent->get_egbase()->get_map()->get_mem()->get_nr_events(); i++) {
      Event* ev = parent->get_egbase()->get_map()->get_mem()->get_event_by_nr(i);
      m_available_events->add_entry( ev->get_name(), ev);
   }
   m_available_events->sort();

   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-80-spacing;
   b=new UIButton(this, posx, posy, 80, 20, 0, 1);
   b->set_title(_("Ok"));
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 80, 20, 1, 0);
   b->set_title(_("Cancel"));
   b->clickedid.set(this, &Editor_Event_Menu_Edit_EventChain::clicked);

   for( uint i = 0; i < m_event_chain->get_nr_events(); i++ ) {
      Event* ev = m_event_chain->get_event( i );
      m_events->add_entry( ev->get_name(), ev );
   }

   m_edit_trigcond = m_event_chain->get_trigcond() ? false : true;

   center_to_parent();
}

/*
 * cleanup
 */
Editor_Event_Menu_Edit_EventChain::~Editor_Event_Menu_Edit_EventChain(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Editor_Event_Menu_Edit_EventChain::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * Think.
 *
 * Maybe we have to simulate a click
 */
void Editor_Event_Menu_Edit_EventChain::think( void ) {
   if( m_edit_trigcond )
      clicked( 10 );
}

/*
 * a button has been clicked
 */
void Editor_Event_Menu_Edit_EventChain::clicked(int i) {
   if(!i) {
      // Cancel has been clicked
      end_modal(0);
      return;
   }

   // ok
   if(i == 1) {
      // Name
      m_event_chain->set_name( m_name->get_text() );
      // Repeating
      m_event_chain->set_repeating( m_morethanonce->get_state() );
      // Trigger Conditional is always updated
      // Events
      m_event_chain->clear_events();
      for( int i = 0; i < m_events->get_nr_entries(); i++) {
         m_event_chain->add_event( static_cast<Event*>(m_events->get_entry( i )));
      }
      end_modal(1);
   }

   // new event
   if( i == 11 ) {
      // TODO
   }

   // Edit trigger condiditional
   if( i == 10 ) {
      Editor_Event_Menu_Edit_TriggerConditional* menu = new Editor_Event_Menu_Edit_TriggerConditional( m_parent, m_event_chain->get_trigcond(), m_event_chain );
      int code = menu->run();
      if( code ) {
         if( m_event_chain->get_trigcond() ) {
            m_event_chain->get_trigcond()->unreference_triggers( m_event_chain );
            delete m_event_chain->get_trigcond();
         }
         m_event_chain->set_trigcond( menu->get_trigcond() );
      }
      delete menu;
   }

   // Insert event
   if( i == 20 ) {
      Event* ev = static_cast<Event*>(m_available_events->get_selection());
      m_events->add_entry( ev->get_name(), ev, true);
   }

   // Delete a event
   if( i == 21 ) {
      m_events->remove_entry( m_events->get_selection_index() );
      m_mvup_btn->set_enabled( false );
      m_mvdown_btn->set_enabled( false );
      m_delete_btn->set_enabled( false );
   }

   // Move up
   if( i == 30) {
      int n = m_events->get_selection_index();
      if( n != 0)
         m_events->switch_entries( n, n - 1);
   }
   if( i == 31) {
      int n = m_events->get_selection_index();
      if( n != (m_events->get_nr_entries() - 1) )
         m_events->switch_entries( n, n + 1);
   }
   return;
}

/*
 * the listbox got selected
 */
void Editor_Event_Menu_Edit_EventChain::tl_selected(int i) {
   m_insert_btn->set_enabled( true );
}
void Editor_Event_Menu_Edit_EventChain::cs_selected(int i) {
   m_mvdown_btn->set_enabled( true );
   m_mvup_btn->set_enabled( true );
   m_delete_btn->set_enabled( true );
}

/*
 * listbox got double clicked
 */
void Editor_Event_Menu_Edit_EventChain::tl_double_clicked(int i) {
   // insert clicked
   clicked( 20 );
}
void Editor_Event_Menu_Edit_EventChain::cs_double_clicked(int i) {
   // Delete clicked
   clicked( 21 );
}
