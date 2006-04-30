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

#include "i18n.h"
#include "trigger_null_option_menu.h"
#include "trigger_null.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "editorinteractive.h"
#include "error.h"
#include "util.h"

Trigger_Null_Option_Menu::Trigger_Null_Option_Menu(Editor_Interactive* parent, Trigger_Null* trigger) :
   UIWindow(parent, 0, 0, 164, 100, _("Trigger Option Menu").c_str()) {
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Null Trigger Options"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

   m_trigger=trigger;

   new UITextarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+50, posy, get_inner_w()-50-2*spacing, 20, 0, 0);
   m_name->set_text( trigger->get_name() );

   // Buttons
   posx=(get_inner_w()/2)-60-spacing;
   posy+=20+spacing;
   UIButton* b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title(_("Ok").c_str());
   b->clickedid.set(this, &Trigger_Null_Option_Menu::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title(_("Cancel").c_str());
   b->clickedid.set(this, &Trigger_Null_Option_Menu::clicked);

   set_inner_size(get_inner_w(), posy+20+spacing);
   center_to_parent();
}

/*
 * cleanup
 */
Trigger_Null_Option_Menu::~Trigger_Null_Option_Menu(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Trigger_Null_Option_Menu::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * a button has been clicked
 */
void Trigger_Null_Option_Menu::clicked(int i) {
   if(!i) {
      // Cancel has been clicked
      end_modal(0);
      return;
   } else if(i==1) {
      // ok button
      if(m_name->get_text())
         m_trigger->set_name(m_name->get_text() );
      end_modal(1);
      return;
   }
}
