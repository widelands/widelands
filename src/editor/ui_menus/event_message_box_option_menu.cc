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

#include "event_message_box_option_menu.h"
#include "event_message_box.h"
#include "event_message_box_option_menu_pic_options.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_multilineeditbox.h"
#include "ui_listselect.h"
#include "editorinteractive.h"
#include "system.h"
#include "error.h"
#include "map.h"
#include "graphic.h"
#include "trigger_null.h"

Event_Message_Box_Option_Menu::Event_Message_Box_Option_Menu(Editor_Interactive* parent, Event_Message_Box* event) :
   UIWindow(parent, 0, 0, 430, 400, "Event Option Menu") {
   m_parent=parent;
   m_event=event;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Message Box Event Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
   m_nr_buttons=m_event->get_nr_buttons();
   m_ls_selected=0;
   m_picid=m_event->get_pic_id();
   m_position=m_event->get_pic_position();
   if(static_cast<int>(m_picid)!=-1) 
      m_clrkey=g_gr->has_clrkey(m_picid);
   else
      m_clrkey=false;

   m_buttons[0].name="Continue";
   m_buttons[1].name="Button 1";
   m_buttons[2].name="Button 2";
   m_buttons[3].name="Button 3";
   m_buttons[0].trigger=m_buttons[1].trigger=m_buttons[2].trigger=m_buttons[3].trigger=-1;
  
   for(int i=0; i<m_event->get_nr_buttons(); i++) {
      m_buttons[i].name=m_event->get_button_name(i);
      m_buttons[i].trigger=m_parent->get_map()->get_trigger_index(m_event->get_button_trigger(i));
   }
   
   // Name editbox 
   new UITextarea(this, spacing, posy, 50, 20, "Name:", Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+60, posy, get_inner_w()/2-60-2*spacing, 20, 0, 0);
   m_name->set_text(event->get_name());
  
   // Only run once CB
   new UITextarea(this, get_inner_w()/2+spacing, posy, 150, 20, "Only run once: ", Align_CenterLeft);
   m_is_one_time_event=new UICheckbox(this, get_inner_w()-STATEBOX_WIDTH-spacing, posy);
   m_is_one_time_event->set_state(m_event->is_one_time_event());

   posy+=20+spacing;
   
   // Caption 
   new UITextarea(this, spacing, posy, 60, 20, "Caption:", Align_CenterLeft);
   m_caption=new UIEdit_Box(this, spacing+60, posy, get_inner_w()/2-60-2*spacing, 20, 0, 1);
   m_caption->set_text(m_event->get_caption());

   // Modal cb
   new UITextarea(this, get_inner_w()/2+spacing, posy, 150, 20, "Is Modal: ", Align_CenterLeft);
   m_is_modal=new UICheckbox(this, get_inner_w()-STATEBOX_WIDTH-spacing, posy);
   m_is_modal->set_state(m_event->get_is_modal());
   
   posy+=20+spacing;
  
   // Window Title
   new UITextarea(this, spacing, posy, 50, 20, "Window Title:", Align_CenterLeft);
   m_window_title=new UIEdit_Box(this, spacing+100, posy, get_inner_w()-100-2*spacing, 20, 0, 2);
   m_window_title->set_text(m_event->get_window_title());
   
   // Text
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 50, 20, "Text:", Align_CenterLeft);
   posy+=20+spacing;
   m_text=new UIMultiline_Editbox(this, spacing, posy, get_inner_w()-2*spacing, 80, event->get_text());
  
   posy+=80+spacing;

   // Nur Buttons
   new UITextarea(this, spacing, posy, 130, 20, "Number of Buttons: ", Align_CenterLeft);
   UIButton* b=new UIButton(this, spacing+140, posy, 20, 20, 0, 2); 
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", true));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   m_nr_buttons_ta=new UITextarea(this, spacing+160+spacing, posy,15,20,"1", Align_CenterLeft);
   b=new UIButton(this, spacing+175+spacing, posy, 20, 20, 0, 3);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", true));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
  
   // Button name
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Button Name: ", Align_CenterLeft);
   m_button_name=new UIEdit_Box(this, spacing+110, posy, 100, 20, 0, 3);
   m_button_name->changedid.set(this, &Event_Message_Box_Option_Menu::edit_box_edited);

   // Listbox for buttons
   m_buttons_ls=new UIListselect(this, get_inner_w()/2+spacing, posy, get_inner_w()/2-2*spacing, 80, Align_Left); 
   m_buttons_ls->selected.set(this, &Event_Message_Box_Option_Menu::ls_selected);

   // Select trigger
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Select Trigger: ", Align_CenterLeft);
   b=new UIButton(this, spacing+110, posy, 20, 20, 0, 4); 
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", true));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   b=new UIButton(this, spacing+130+spacing, posy, 20, 20, 0, 5);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", true));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
  
   // Current Trigger
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Current: ", Align_CenterLeft);
   m_current_trigger_ta=new UITextarea(this, spacing+15, posy+15+spacing, get_inner_w()/2, 20, "Keine Trigger gewaehlt!", Align_CenterLeft); 


   // Uses picture checkbox
   posy=get_inner_h()-60;
   new UITextarea(this, spacing, posy, 200, 20, "Uses Picture: ", Align_CenterLeft);
   m_uses_picture=new UICheckbox(this, get_inner_w()/2-STATEBOX_HEIGHT-2*spacing, posy);
   m_uses_picture->set_state(static_cast<int>(m_event->get_pic_id())!=-1);
   m_uses_picture->changedto.set(this, &Event_Message_Box_Option_Menu::pic_checkbox_enabled);

   // Picture options Button
   b=new UIButton(this, get_inner_w()/2+spacing, posy, 120, 20, 0, 6);
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   b->set_title("Picture options");
   b->set_enabled(m_uses_picture->get_state());
   m_btn_picture_options=b;
   
   // Ok/Cancel Buttons
   posx=(get_inner_w()/2)-60-spacing;
   posy=get_inner_h()-30;
   b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title("Ok");
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title("Cancel");
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);

   int i=0;
   for(i=0; i<m_parent->get_map()->get_number_of_triggers(); i++) {
      Trigger* trig=m_parent->get_map()->get_trigger(i);
      if(trig->get_id()==TRIGGER_NULL) 
         m_null_triggers.push_back(i);
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
            if(static_cast<int>(m_picid)!=-1 && m_picid!=m_event->get_pic_id())
               g_gr->flush_picture(m_picid);
            end_modal(0);
            return;
         }
         break;

      case 1:
         {
            // ok button
            m_event->set_is_one_time_event(m_is_one_time_event->get_state());
            if(m_name->get_text())
               m_event->set_name(m_name->get_text());
            if(m_text->get_text().c_str())
               m_event->set_text(m_text->get_text().c_str());
            if(m_caption->get_text())
               m_event->set_caption(m_caption->get_text());
            if(m_window_title->get_text())
               m_event->set_window_title(m_window_title->get_text());
            m_event->set_is_modal(m_is_modal->get_state());
            m_event->set_nr_buttons(m_nr_buttons);
            for(int i=0; i<m_nr_buttons; i++) {
               m_event->set_button_name(i, m_buttons[i].name.c_str());
               if(m_buttons[i].trigger!=-1) {
                  m_event->set_button_trigger(i, static_cast<Trigger_Null*>(m_parent->get_map()->get_trigger(m_buttons[i].trigger)), m_parent->get_map());
               } else {
                  m_event->set_button_trigger(i, 0, m_parent->get_map());
               }
            }
            if(m_uses_picture->get_state()) {
               if(m_picid!=m_event->get_pic_id() && static_cast<int>(m_event->get_pic_id())!=-1) {
                  log("Flushing picture: %i\n", m_event->get_pic_id());
                  g_gr->flush_picture(m_event->get_pic_id());
               }
               log("Setting pic id to: %i\n", m_picid);
               m_event->set_pic_id(m_picid);
               m_event->set_pic_position(m_position);
               g_gr->use_clrkey(m_event->get_pic_id(), m_clrkey);
            } else {
               int picid=m_event->get_pic_id();
               if(static_cast<int>(picid)!=-1) { 
                  log("Flushing picture unten: %i\n", m_event->get_pic_id());
                  g_gr->flush_picture(picid);
               }
               m_event->set_pic_id(-1);
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
            if(m_buttons[m_ls_selected].trigger<0) m_buttons[m_ls_selected].trigger=m_null_triggers.size()-1;
            update();
         }
         break;


      case 5:
         {
            // Trigger sel, up
            m_buttons[m_ls_selected].trigger++;
            if(m_buttons[m_ls_selected].trigger>=static_cast<int>(m_null_triggers.size()))
               m_buttons[m_ls_selected].trigger=0;
            update();
         }
         break;

      case 6:
         {
            // Picture options
            Event_Message_Box_Option_Menu_Picture_Options* epo=new Event_Message_Box_Option_Menu_Picture_Options(m_parent,&m_clrkey, &m_picid, &m_position);
            if(epo->run()) 
               update();
            delete epo;
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
      m_buttons_ls->add_entry(m_buttons[i].name.c_str(), 0);

   std::string text;
   text.append(1,static_cast<uchar>(m_nr_buttons+0x30));
   m_nr_buttons_ta->set_text(text.c_str());

   
   m_button_name->set_text(m_buttons[m_ls_selected].name.c_str());
   
   if(m_nr_buttons!=1) {
      for(int i=0; i<m_nr_buttons; i++) 
         if(m_buttons[i].trigger==-1) 
            m_buttons[i].trigger=0;
      
      m_current_trigger_ta->set_text(m_parent->get_map()->get_trigger(m_buttons[m_ls_selected].trigger)->get_name());
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
   m_buttons[m_ls_selected].name=m_button_name->get_text();
   update();
}

/*
 * The picture checkbox was enabled
 */
void Event_Message_Box_Option_Menu::pic_checkbox_enabled(bool t) {
   m_btn_picture_options->set_enabled(t);
   update();
}
