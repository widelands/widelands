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

#include "trigger_null_option_menu.h"

#include "i18n.h"
#include "trigger/trigger_null.h"
#include "editorinteractive.h"

#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"


Trigger_Null_Option_Menu::Trigger_Null_Option_Menu
(Editor_Interactive & parent, Trigger_Null & trigger)
:
UI::Window(&parent, 0, 0, 164, 100, _("Null Trigger Options").c_str()),
m_trigger (trigger)
{
   const int32_t offsx=5;
   const int32_t offsy=25;
   const int32_t spacing=5;
   int32_t posx=offsx;
   int32_t posy=offsy;

   new UI::Textarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UI::Edit_Box(this, spacing+50, posy, get_inner_w()-50-2*spacing, 20, 0, 0);
	m_name->set_text(trigger.get_name());

   // Buttons
   posx=(get_inner_w()/2)-60-spacing;
   posy+=20+spacing;
	new UI::Button<Trigger_Null_Option_Menu>
		(this,
		 posx, posy, 60, 20,
		 0,
		 &Trigger_Null_Option_Menu::clicked_ok, this,
		 _("Ok"));

   posx=(get_inner_w()/2)+spacing;

	new UI::IDButton<Trigger_Null_Option_Menu, int32_t>
		(this,
		 posx, posy, 60, 20,
		 1,
		 &Trigger_Null_Option_Menu::end_modal, this, 0,
		 _("Cancel"));

   set_inner_size(get_inner_w(), posy+20+spacing);
   center_to_parent();
}


/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
*/
bool Trigger_Null_Option_Menu::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Trigger_Null_Option_Menu::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}

/*
 * a button has been clicked
 */
void Trigger_Null_Option_Menu::clicked_ok() {
      if (m_name->get_text())
         m_trigger.set_name(m_name->get_text());
      end_modal(1);
}
