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

#include "event_message_box_message_box.h"
#include "event_message_box.h"
#include "graphic.h"
#include "editorinteractive.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "system.h"
#include "constants.h"
#include "trigger_null.h"

/*
 * The message box himself
 */
Message_Box_Event_Message_Box::Message_Box_Event_Message_Box(Editor_Game_Base* egbase, Event_Message_Box* event) :
  UIWindow(egbase->get_iabase(), 0, 0, 600, 400, event->get_window_title()) {
  
     m_event=event;
     
     UIMultiline_Textarea* m_text=0;
     UITextarea* m_caption=0;
       
     int spacing=5;
     int offsy=5;
     int offsx=spacing;
     int posx=offsx;
     int posy=offsy;
     
     if(static_cast<int>(m_event->get_pic_id())==-1) {
        // No picture, so we can optimaly assign all things its place
        set_inner_size(400,300);
        m_caption=new UITextarea(this, posx, posy, get_inner_w()-2*spacing, 50, m_event->get_caption(), Align_Center);
        posy+=50;
        m_text=new UIMultiline_Textarea(this, posx, posy, get_inner_w()-posx-spacing, get_inner_h()-posy-2*spacing-50, "", Align_Left);
     } else {
        // Picture there
        int picw, pich;
        g_gr->get_picture_size(m_event->get_pic_id(), &picw, &pich);
        UIButton* b;
        switch(event->get_pic_position()) {
           case Event_Message_Box::Right:
              {
                 set_inner_size(picw*2+2*spacing+50, pich+110);
                 m_caption=new UITextarea(this, posx, posy, get_inner_w()-2*spacing, 50, m_event->get_caption(), Align_Center);
                 posy+=50;
                 b=new UIButton(this, get_inner_w()/2+spacing, posy, get_inner_w()/2-2*spacing, pich, 0, 0, 1); 
                 b->set_enabled(false);
                 b->set_pic(event->get_pic_id());
                 m_text=new UIMultiline_Textarea(this, spacing, posy, get_inner_w()/2-2*spacing, pich, "", Align_Left);
              }
              break;

            case Event_Message_Box::Left:
              {
                 set_inner_size(picw*2+2*spacing+50, pich+110);
                 m_caption=new UITextarea(this, posx, posy, get_inner_w()-2*spacing, 50, m_event->get_caption(), Align_Center);
                 posy+=50;
                 b=new UIButton(this, spacing, posy, get_inner_w()/2-2*spacing, pich, 0, 0, 1); 
                 b->set_enabled(false);
                 b->set_pic(event->get_pic_id());
                 m_text=new UIMultiline_Textarea(this, get_inner_w()/2+spacing, posy, get_inner_w()/2-2*spacing, pich, "", Align_Left);
              }
              break;

            case Event_Message_Box::Center_under:
              {
                 set_inner_size(picw+2*15, pich+300);
                 m_caption=new UITextarea(this, posx, posy, get_inner_w()-2*spacing, 50, m_event->get_caption(), Align_Center);
                 posy+=50;
                 b=new UIButton(this, 15, posy, picw, pich, 0, 0, 1); 
                 b->set_enabled(false);
                 b->set_pic(event->get_pic_id());
                 posy+=pich+spacing;
                 m_text=new UIMultiline_Textarea(this, spacing, posy, get_inner_w()-2*spacing, get_inner_h()-2*spacing-posy, "", Align_Left);
              }
              break;

            case Event_Message_Box::Center_over:
              {
                 set_inner_size(picw+2*15, pich+300);
                 b=new UIButton(this, 15, posy, picw, pich, 0, 0, 1); 
                 b->set_enabled(false);
                 b->set_pic(event->get_pic_id());
                 posy+=pich+spacing;
                 m_caption=new UITextarea(this, posx, posy, get_inner_w()-2*spacing, 50, m_event->get_caption(), Align_Center);
                 posy+=50;
                 m_text=new UIMultiline_Textarea(this, spacing, posy, get_inner_w()-2*spacing, get_inner_h()-2*spacing-posy, "", Align_Left);
              }
              break;
        }
     }
     if(m_caption)
        m_caption->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
     if(m_text)
        m_text->set_text(m_event->get_text());


     // Buttons
     int but_width=80;
     int space=get_inner_w()-2*spacing;
     space-=but_width*m_event->get_nr_buttons();
     space/=m_event->get_nr_buttons()+1;
     UIButton* b;
     posx=spacing;
     posy=get_inner_h()-30;
     m_trigger.resize(m_event->get_nr_buttons());
     for(int i=0; i<m_event->get_nr_buttons(); i++) {
        posx+=space;
        b=new UIButton(this, posx, posy, but_width, 20, 0, i);
        posx+=but_width;
        b->clickedid.set(this, &Message_Box_Event_Message_Box::clicked);
        b->set_title(m_event->get_button_name(i));
        m_trigger[i]=m_event->get_button_trigger(i);
     }

     
     center_to_parent();
}


/*
 * Handle mouseclick
 *
 * we might be a modal, therefore, if we are, we delete ourself through end_modal()
 * otherwise through die()
 */
bool Message_Box_Event_Message_Box::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(-1);
      return true;
   } else
      return false; // we're not dragable 
}

/*
 * clicked
 */
void Message_Box_Event_Message_Box::clicked(int i) {
   if(i==-1) {
      // we should end this dialog
      if(m_event->get_is_modal()) {
         end_modal(0);
         return;
      } else {
         die();
         return;
      }
   } else {
      // One of the buttons has been pressed
//      NoLog("Button %i has been pressed, nr of buttons: %i!\n", i, m_event->get_nr_buttons());
      Trigger_Null* t=m_trigger[i];
      if(t)
         t->set_trigger_manually(true);
      clicked(-1);
      return;
   }
}
