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

#include "trigger_time_option_menu.h"
#include "trigger_time.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "editorinteractive.h"
#include "system.h"
#include "error.h"

Trigger_Time_Option_Menu::Trigger_Time_Option_Menu(Editor_Interactive* parent, Trigger_Time* trigger) :
   UIWindow(parent, 0, 0, 164, 180, "Trigger Option Menu") {
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Time Trigger Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   const int width=20;
   const int height=20;
   int posx=offsx;
   int posy=offsy;

   m_trigger=trigger;
   int wait_time=trigger->get_wait_time();
   m_values[0]=(wait_time/3600)/10; // hours
   m_values[1]=(wait_time/3600)%10;
   wait_time-=(wait_time/3600)*3600;
   m_values[2]=(wait_time/60)/10; // minutes
   m_values[3]=(wait_time/60)%10;
   wait_time-=(wait_time/60)*60;
   m_values[4]=(wait_time/3600)/10; // seconds
   m_values[5]=(wait_time/3600)%10;

   new UITextarea(this, spacing, offsy, "Only triggers once: ", Align_Left);
   UICheckbox* cb=new UICheckbox(this, spacing+130, offsy-3);
   m_is_one_time_trigger=trigger->is_one_time_trigger();
   cb->set_state(m_is_one_time_trigger);
   cb->changedto.set(this, &Trigger_Time_Option_Menu::cb_changed);
   posy+=20+spacing;

   new UITextarea(this, spacing, posy, 50, 20, "Name:", Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+50, posy, get_inner_w()-50-2*spacing, 20, 0, 0);
   m_name->set_text(trigger->get_name());
   
   posy+=20+spacing;

   UIButton* b;
   // hours, first cipher
   b=new UIButton(this, posx, posy, width, height, 1, 2);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   m_textareas[0]=new UITextarea(this, posx, posy+20, width, height, "9", Align_Center);
   b=new UIButton(this, posx, posy+40, width, height, 1, 3);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   posx+=width+spacing/2;
   // hours second cipher
   b=new UIButton(this, posx, posy, width, height, 1, 4);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   m_textareas[1]=new UITextarea(this, posx, posy+20, width, height, "9", Align_Center);
   b=new UIButton(this, posx, posy+40, width, height, 1, 5);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   posx+=width+spacing;

   // two points
   tt=new UITextarea(this, posx, posy+23, ":", Align_Left);
   posx+=spacing+tt->get_w();

   // minutes, first cipher
   b=new UIButton(this, posx, posy, width, height, 1, 6);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   m_textareas[2]=new UITextarea(this, posx, posy+20, width, height, "9", Align_Center);
   b=new UIButton(this, posx, posy+40, width, height, 1, 7);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   posx+=width+spacing/2;
   // minutes second cipher
   b=new UIButton(this, posx, posy, width, height, 1, 8);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   m_textareas[3]=new UITextarea(this, posx, posy+20, width, height, "9", Align_Center);
   b=new UIButton(this, posx, posy+40, width, height, 1, 9);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   posx+=width+spacing;

   // two points
   tt=new UITextarea(this, posx, posy+23, ":", Align_Left);
   posx+=spacing+tt->get_w();

   // seconds, first cipher
   b=new UIButton(this, posx, posy, width, height, 1, 10);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   m_textareas[4]=new UITextarea(this, posx, posy+20, width, height, "9", Align_Center);
   b=new UIButton(this, posx, posy+40, width, height, 1, 11);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   posx+=width+spacing/2;
   // seconds, second cipher
   b=new UIButton(this, posx, posy, width, height, 1, 12);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", RGBColor(0,0,255)));
   m_textareas[5]=new UITextarea(this, posx, posy+20, width, height, "9", Align_Center);
   b=new UIButton(this, posx, posy+40, width, height, 1, 13);
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", RGBColor(0,0,255)));
   posx+=width+spacing;

   posy+=60;

   posy+=2*spacing;

   // Buttons
   posx=(get_inner_w()/2)-60-spacing;
   b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title("Ok");
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title("Cancel");
   b->clickedid.set(this, &Trigger_Time_Option_Menu::clicked);

   center_to_parent();
   update();
}

/*
 * cleanup
 */
Trigger_Time_Option_Menu::~Trigger_Time_Option_Menu(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself 
 * on close (the caller must do this) instead 
 * we simulate a cancel click
 */
bool Trigger_Time_Option_Menu::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable 
}
  
/*
 * a button has been clicked
 */
void Trigger_Time_Option_Menu::clicked(int i) {
   if(!i) {
      // Cancel has been clicked
      end_modal(0);
      return;
   } else if(i==1) {
      int hours=m_values[0]*10+m_values[1];
      int minutes=m_values[2]*10+m_values[3];
      int seconds=m_values[4]*10+m_values[5];
      int total=hours*3600+minutes*60+seconds;   
      // ok button
      log("Setting trigger to: %i seconds and one time trigger: %i\n", total, m_is_one_time_trigger);
      m_trigger->set_is_one_time_trigger(m_is_one_time_trigger);
      m_trigger->set_wait_time(total);
      if(m_name->get_text())
         m_trigger->set_name(m_name->get_text());
      end_modal(1);
      return;
   }
   // one of the buttons have been pressed
   int id=(i-2)/2;
   m_values[id]= i%2 ?  m_values[id]-1 :  m_values[id]+1;
   if(m_values[id]<0) m_values[id]=0;
   if(!m_values[0] && !m_values[1] && !m_values[2] && !m_values[3] && !m_values[4] && !m_values[5]) m_values[5]=1;
   if(m_values[id]>9) m_values[id]=9;
   if( (id==2 || id==4)  && m_values[id]>6) m_values[id]=6;
   update();
}

/*
 * checkbox has been changed
 */
void Trigger_Time_Option_Menu::cb_changed(bool t) {
   m_is_one_time_trigger=t;
}

/*
 * Update the Textareas
 */
void Trigger_Time_Option_Menu::update(void) {
   int i;
   for(i=0; i<6; i++) {
      std::string str;
      str.append(1, static_cast<char>(m_values[i]+0x30));
      m_textareas[i]->set_text(str);
   }
}
