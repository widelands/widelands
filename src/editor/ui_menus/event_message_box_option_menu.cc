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

Event_Message_Box_Option_Menu::Event_Message_Box_Option_Menu(Editor_Interactive* parent, Event_Message_Box* event) :
   UIWindow(parent, 0, 0, 430, 400, "Event Option Menu") {
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Message Box Event Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

   m_event=event;
   
   // Name editbox 
   new UITextarea(this, spacing, posy, 50, 20, "Name:", Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+60, posy, get_inner_w()/2-60-2*spacing, 20, 0, 0);
   m_name->set_text(event->get_name());
  
   // Only run once CB
   new UITextarea(this, get_inner_w()/2+spacing, posy, 150, 20, "Only run once: ", Align_CenterLeft);
   UICheckbox* m_is_one_time_event=new UICheckbox(this, get_inner_w()-STATEBOX_WIDTH-spacing, posy);
   m_is_one_time_event->set_state(m_is_one_time_event);

   posy+=20+spacing;
   
   // Caption 
   new UITextarea(this, spacing, posy, 60, 20, "Caption:", Align_CenterLeft);
   m_caption=new UIEdit_Box(this, spacing+60, posy, get_inner_w()/2-60-2*spacing, 20, 0, 1);
   m_caption->set_text("Caption");

   // Modal cb
   new UITextarea(this, get_inner_w()/2+spacing, posy, 150, 20, "Is Modal: ", Align_CenterLeft);
   UICheckbox* m_is_modal=new UICheckbox(this, get_inner_w()-STATEBOX_WIDTH-spacing, posy);
   m_is_modal->set_state(false);
   
   posy+=20+spacing;
  
   // Window Title
   new UITextarea(this, spacing, posy, 50, 20, "Window Title:", Align_CenterLeft);
   m_window_title=new UIEdit_Box(this, spacing+100, posy, get_inner_w()-100-2*spacing, 20, 0, 2);
   m_window_title->set_text("Window Title");
   
   // Text
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 50, 20, "Text:", Align_CenterLeft);
   posy+=20+spacing;
   m_text=new UIMultiline_Editbox(this, spacing, posy, get_inner_w()-2*spacing, 80, event->get_string());
  
   posy+=80+spacing;

   // Nur Buttons
   new UITextarea(this, spacing, posy, 130, 20, "Number of Buttons: ", Align_CenterLeft);
   UIButton* b=new UIButton(this, spacing+140, posy, 20, 20, 0, 2); 
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   m_nr_buttons_ta=new UITextarea(this, spacing+160+spacing, posy,15,20,"1", Align_CenterLeft);
   b=new UIButton(this, spacing+175+spacing, posy, 20, 20, 0, 3);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
  
   // Button name
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Button Name: ", Align_CenterLeft);
   m_button_name=new UIEdit_Box(this, spacing+110, posy, 100, 20, 0, 3);

   // Listbox for buttons
   m_buttons_ls=new UIListselect(this, get_inner_w()/2+spacing, posy, get_inner_w()/2-2*spacing, 80, Align_Left); 

   // Select trigger
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Select Trigger: ", Align_CenterLeft);
   b=new UIButton(this, spacing+110, posy, 20, 20, 0, 4); 
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   b=new UIButton(this, spacing+130+spacing, posy, 20, 20, 0, 5);
   b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
  
   // Current Trigger
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Current: ", Align_CenterLeft);
   m_current_trigger_ta=new UITextarea(this, spacing+5, posy+20+spacing, get_inner_w()/2, 20, "Keine Trigger gewaehlt!", Align_CenterLeft); 


   // Uses picture checkbox
   posy=get_inner_h()-60;
   new UITextarea(this, spacing, posy, 200, 20, "Uses Picture: ", Align_CenterLeft);
   m_uses_picture=new UICheckbox(this, get_inner_w()/2-STATEBOX_HEIGHT-2*spacing, posy);
   // Picture options Button
   b=new UIButton(this, get_inner_w()/2+spacing, posy, 120, 20, 0, 6);
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   b->set_title("Picture options");
//   b->set_enabled(false);

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

   center_to_parent();
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
            m_event->set_is_one_time_event(m_is_one_time_event);
            if(m_name->get_text())
               m_event->set_name(m_name->get_text());
            if(m_text->get_text().c_str())
               m_event->set_string(m_text->get_text().c_str());
            end_modal(1);
            return;
         }
         break;

      case 2:
         {
            // Number of buttons: down
         }
         break;

      case 3:
         {
            // Number of buttons up
         }
         break;

      case 4:
         {
            // Trigger sel, down
         }
         break;


      case 5:
         {
            // Trigger sel, up
         }
         break;

      case 6:
         {
            int picid=-1;
            bool clrkey=true;
            int position=2;
            // Picture options
            Event_Message_Box_Option_Menu_Picture_Options* epo=new Event_Message_Box_Option_Menu_Picture_Options(m_parent,&clrkey, &picid, &position);
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

}

