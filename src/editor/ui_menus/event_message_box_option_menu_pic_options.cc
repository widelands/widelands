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
#include "event_message_box_option_menu_pic_options.h"
#include "event_message_box_option_menu.h"
#include "event_message_box.h"
#include "ui_radiobutton.h"
#include "ui_textarea.h"
#include "ui_editbox.h"
#include "ui_checkbox.h"
#include "ui_modal_messagebox.h"
#include "system.h"

Event_Message_Box_Option_Menu_Picture_Options::Event_Message_Box_Option_Menu_Picture_Options(Editor_Interactive* parent, 
      bool* clrkey, uint* picid, int* position) :
   UIWindow(parent, 0, 0, 460, 350, "Event Option Menu") {
   
   m_parent=parent;
   m_clrkey_var=clrkey;
   m_picid_var=picid;
   m_pos_var=position;
   m_picid=*picid;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Message Box Event Picture Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
  

   // Only run once CB
   new UITextarea(this, spacing, posy, 150, 20, "Uses Clrkey: ", Align_CenterLeft);
   m_clrkey=new UICheckbox(this, get_inner_w()/2-STATEBOX_WIDTH-spacing, posy);
   m_clrkey->set_state(*clrkey);
   UITextarea* ta=new UITextarea(this, get_inner_w()/2+spacing, posy, get_inner_w()/2-2*spacing, 40, "Colorkeys (Clrkey) define the"
         " transparent color which is not visible. If you enable this checkbox, the upper left pixel of you picture"
         " is used as the transparent color", Align_Left, true);


   // Name
   posy+=ta->get_h()+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Filename: ", Align_CenterLeft);
   m_filename=new UIEdit_Box(this, spacing+100+spacing, posy, get_inner_w()-100-3*spacing, 20, 0,0);
   m_filename->set_text("noname.png");

   // Load button, save button
   posy+=20+spacing;
   UIButton* b=new UIButton(this, get_inner_w()/2-80-spacing, posy, 80, 20, 0, 2);
   b->clickedid.set(this, &Event_Message_Box_Option_Menu_Picture_Options::clicked);
   b->set_title("Load");
/*   b=new UIButton(this, get_inner_w()/2+spacing, posy, 80, 20, 0, 3);
   b->clickedid.set(this, &Event_Message_Box_Option_Menu_Picture_Options::clicked);
   b->set_title("Save");
*/
   // Picture currently assigned
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 300, 20, "Picture currently assigned: ", Align_CenterLeft);
   m_pic_assigned=new UITextarea(this, spacing+300, posy, 50, 20, "No", Align_CenterLeft);
   if(static_cast<int>(*picid)!=-1) 
      m_pic_assigned->set_text("Yes");

   // Picture positions
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 100, 20, "Picture Position (rel. to text): ", Align_CenterLeft);

   // Radiogroup
   posy+=20+spacing;
   m_radiogroup=new UIRadiogroup();
   new UITextarea(this, spacing, posy, 300, 20, "Right: ", Align_CenterLeft);
   m_radiogroup->add_button(this, spacing+300, posy);
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 300, 20, "Left: ", Align_CenterLeft);
   m_radiogroup->add_button(this, spacing+300, posy);
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 300, 20, "Center/Top (under Caption): ", Align_CenterLeft);
   m_radiogroup->add_button(this, spacing+300, posy);
   posy+=20+spacing;
   new UITextarea(this, spacing, posy, 300, 20, "Center/Top (over Caption): ", Align_CenterLeft);
   m_radiogroup->add_button(this, spacing+300, posy);
   m_radiogroup->set_state(*m_pos_var);

   // Ok/Cancel Buttons
   posx=(get_inner_w()/2)-60-spacing;
   posy=get_inner_h()-30;
   b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title("Ok");
   b->clickedid.set(this, &Event_Message_Box_Option_Menu_Picture_Options::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title("Cancel");
   b->clickedid.set(this, &Event_Message_Box_Option_Menu_Picture_Options::clicked);

   center_to_parent();
   update();
}

/*
 * cleanup
 */
Event_Message_Box_Option_Menu_Picture_Options::~Event_Message_Box_Option_Menu_Picture_Options(void) {
   delete m_radiogroup;
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself 
 * on close (the caller must do this) instead 
 * we simulate a cancel click
 */
bool Event_Message_Box_Option_Menu_Picture_Options::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable 
}
  
/*
 * a button has been clicked
 */
void Event_Message_Box_Option_Menu_Picture_Options::clicked(int i) {
   switch(i) {
      case 0:
         {
            // Cancel has been clicked
            if(static_cast<int>(*m_picid_var)!=m_picid) 
               if(static_cast<int>(m_picid)!=-1)
                  g_gr->flush_picture(m_picid);
               end_modal(0);
            return;
         }
         break;
      
      case 1:
         {
            // ok button
            *m_clrkey_var=m_clrkey->get_state();
            *m_picid_var=m_picid;
            *m_pos_var=m_radiogroup->get_state();
            end_modal(1);
            return;
         }
         break;

      case 2:
         {
            // Load button
            if((m_picid=g_gr->get_picture(PicMod_Game, m_filename->get_text(), true))) {
               m_pic_assigned->set_text("yes");
            } else {
               m_picid=*m_picid_var;
               std::string msg="Couldn't load file ";
               msg+=m_filename->get_text();
               msg+=", check file!";
               UIModal_Message_Box* mb=new UIModal_Message_Box(m_parent, "Load Error!!", msg, UIModal_Message_Box::OK);
               mb->run();
               delete mb;
            }
         }
         break;

      case 3:
         {
            // Save button
            // not currently supported (copyright issues)
         }
         break;
   }
}

/*
 * update function: update all UI elements
 */
void Event_Message_Box_Option_Menu_Picture_Options::update(void) {

}

