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

#include "event_message_box_option_menu.h"
#include "event_message_box.h"
#include "i18n.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_multilineeditbox.h"
#include "ui_listselect.h"
#include "editorinteractive.h"
#include "error.h"
#include "map.h"
#include "map_trigger_manager.h"
#include "graphic.h"
#include "trigger_null.h"
#include "util.h"

Event_Message_Box_Option_Menu::Event_Message_Box_Option_Menu
(Editor_Interactive* parent, Event_Message_Box* event)
:
UIWindow(parent, 0, 0, 430, 400, _("Event Option Menu").c_str()),
m_event(event),
m_parent(parent)
{

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Message Box Event Options"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
   m_nr_buttons=m_event->get_nr_buttons();
   m_ls_selected=0;

   m_buttons[0].name=_("Continue");
   m_buttons[1].name=_("Button 1");
   m_buttons[2].name=_("Button 2");
   m_buttons[3].name=_("Button 3");
   m_buttons[0].trigger=m_buttons[1].trigger=m_buttons[2].trigger=m_buttons[3].trigger=-1;


   // Name editbox
   new UITextarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+60, posy, get_inner_w()/2-60-2*spacing, 20, 0, 0);
   m_name->set_text( event->get_name() );
   posy+= 20+spacing;

   // Modal cb
   new UITextarea(this, get_inner_w()/2+spacing, posy, 150, 20, _("Is Modal: "), Align_CenterLeft);
   m_is_modal=new UICheckbox(this, get_inner_w()-STATEBOX_WIDTH-spacing, posy);
   m_is_modal->set_state(m_event->get_is_modal());

   posy+=20+spacing;

   // Window Title
   new UITextarea(this, spacing, posy, 50, 20, _("Window Title:"), Align_CenterLeft);
   m_window_title=new UIEdit_Box(this, spacing+100, posy, get_inner_w()-100-2*spacing, 20, 0, 2);
   m_window_title->set_text( m_event->get_window_title() );

   // Text
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 50, 20, _("Text:"), Align_CenterLeft);
   posy+=20+spacing;
   m_text=new UIMultiline_Editbox(this, spacing, posy, get_inner_w()-2*spacing, 80, event->get_text() );

   posy+=80+spacing;

   // Nur Buttons
   new UITextarea(this, spacing, posy, 130, 20, _("Number of Buttons: "), Align_CenterLeft);
   UIButton* b=new UIButton(this, spacing+140, posy, 20, 20, 0, 2);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   m_nr_buttons_ta=new UITextarea(this, spacing+160+spacing, posy,15,20,"1", Align_CenterLeft);
   b=new UIButton(this, spacing+175+spacing, posy, 20, 20, 0, 3);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);

   // Button name
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, _("Button Name: "), Align_CenterLeft);
   m_button_name=new UIEdit_Box(this, spacing+110, posy, 100, 20, 0, 3);
   m_button_name->changedid.set(this, &Event_Message_Box_Option_Menu::edit_box_edited);

   // Listbox for buttons
   m_buttons_ls=new UIListselect(this, get_inner_w()/2+spacing, posy, get_inner_w()/2-2*spacing, 80, Align_Left);
   m_buttons_ls->selected.set(this, &Event_Message_Box_Option_Menu::ls_selected);

   // Select trigger
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, _("Select Trigger: "), Align_CenterLeft);
   b=new UIButton(this, spacing+110, posy, 20, 20, 0, 4);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   b=new UIButton(this, spacing+130+spacing, posy, 20, 20, 0, 5);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);

   // Current Trigger
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, _("Current: "), Align_CenterLeft);
   m_current_trigger_ta=new UITextarea(this, spacing+15, posy+15+spacing, get_inner_w()/2, 20, _("No trigger selected!"), Align_CenterLeft);

   // Ok/Cancel Buttons
   posx=(get_inner_w()/2)-60-spacing;
   posy=get_inner_h()-30;
   b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title(_("Ok").c_str());
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title(_("Cancel").c_str());
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);

	const MapTriggerManager & mtm = m_parent->get_map()->get_mtm();
	const MapTriggerManager::Index nr_triggers = mtm.get_nr_triggers();
	for (MapTriggerManager::Index i = 0; i < nr_triggers; ++i) {
		if (strcmp(mtm.get_trigger_by_nr(i).get_id(), "null") == 0)
         m_null_triggers.push_back(i);
   }

   for(int i=0; i<m_event->get_nr_buttons(); i++) {
      m_buttons[i].name=m_event->get_button_name(i);
      for(int j=0; j<((int)m_null_triggers.size()); j++) {
         // Get this triggers index
         int foundidx = -1;
         for (MapTriggerManager::Index x = 0; x < nr_triggers; ++x)
            if (&mtm.get_trigger_by_nr(x) == m_event->get_button_trigger(i)) {
               foundidx = x;
               break;
            }

         if(foundidx==m_null_triggers[j])
            m_buttons[i].trigger=j;
      }
   }

   center_to_parent();
   update();
}

/*
 * cleanup
 */
Event_Message_Box_Option_Menu::~Event_Message_Box_Option_Menu(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Event_Message_Box_Option_Menu::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * a button has been clicked
 */
void Event_Message_Box_Option_Menu::clicked(int i) {
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
            if(m_text->get_text().c_str())
               m_event->set_text( m_text->get_text().c_str() );
            if(m_window_title->get_text())
               m_event->set_window_title( m_window_title->get_text() );
            m_event->set_is_modal(m_is_modal->get_state());
            m_event->set_nr_buttons(m_nr_buttons);
            const MapTriggerManager & mtm =
               m_parent->get_map()->get_mtm();
            for(int i=0; i<m_nr_buttons; i++) {
               m_event->set_button_name(i, m_buttons[i].name);
               if(m_buttons[i].trigger!=-1) {
                  m_event->set_button_trigger
                     (i,
                      static_cast<Trigger_Null*>
                      (&mtm.get_trigger_by_nr
                       (m_null_triggers[m_buttons[i].trigger])));
               } else {
                  m_event->set_button_trigger(i, 0);
               }
            }
            end_modal(1);
            return;
         }
         break;

      case 2:
         {
            // Number of buttons: down
            m_nr_buttons--;
            if(m_nr_buttons<1) m_nr_buttons=1;
            update();
         }
         break;

      case 3:
         {
            // Number of buttons up
            m_nr_buttons++;
            if(m_nr_buttons>MAX_BUTTONS) m_nr_buttons=MAX_BUTTONS;
            update();
         }
         break;

      case 4:
         {
            // Trigger sel, down
            m_buttons[m_ls_selected].trigger--;
            if(m_buttons[m_ls_selected].trigger<-1) m_buttons[m_ls_selected].trigger=m_null_triggers.size()-1;
            update();
         }
         break;


      case 5:
         {
            // Trigger sel, up
            m_buttons[m_ls_selected].trigger++;
            if(m_buttons[m_ls_selected].trigger>=static_cast<int>(m_null_triggers.size()))
               m_buttons[m_ls_selected].trigger=-1;
            update();
         }
         break;
   }
}

/*
 * update function: update all UI elements
 */
void Event_Message_Box_Option_Menu::update(void) {
   if(m_ls_selected>=m_nr_buttons) m_buttons_ls->select(0);

   if(!m_null_triggers.size()) {
      // No triggers, no other buttons
      m_nr_buttons=1;
   }

   m_buttons_ls->clear();
   int i;
   for(i=0; i<m_nr_buttons; i++)
      m_buttons_ls->add_entry( m_buttons[i].name.c_str(), 0);

   std::string text;
   text.append(1,static_cast<uchar>(m_nr_buttons+0x30));
   m_nr_buttons_ta->set_text(text.c_str());


   m_button_name->set_text( m_buttons[m_ls_selected].name.c_str());

   if(m_nr_buttons && m_null_triggers.size()) {
      if(m_buttons[m_ls_selected].trigger==-1)
         m_current_trigger_ta->set_text("none");
      else
         m_current_trigger_ta->set_text( m_parent->get_map()->get_mtm().get_trigger_by_nr(m_null_triggers[m_buttons[m_ls_selected].trigger]).get_name());
   } else {
      m_current_trigger_ta->set_text("---");
      m_buttons[0].trigger=-1;
   }
}

/*
 * Listbox got selected
 */
void Event_Message_Box_Option_Menu::ls_selected(int i) {
   m_ls_selected=i;
   update();
}

/*
 * Button name edit box edited
 */
void Event_Message_Box_Option_Menu::edit_box_edited(int i) {
   m_buttons[m_ls_selected].name= m_button_name->get_text() ;
   update();
}
