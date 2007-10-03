/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_event_menu_new_trigger.h"

#include "constants.h"
#include "i18n.h"
#include "map_trigger_manager.h"
#include "editorinteractive.h"
#include "trigger/trigger.h"
#include "trigger/trigger_factory.h"
#include "map.h"

#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_window.h"


Editor_Event_Menu_New_Trigger::Editor_Event_Menu_New_Trigger(Editor_Interactive* parent) :
UI::Window(parent, 0, 0, 400, 240, _("New Trigger").c_str()),
m_parent(parent)
{

   const int32_t offsx=5;
   const int32_t offsy=25;
   const int32_t spacing=5;
   int32_t posx=offsx;
   int32_t posy=offsy;

   // Trigger List
   new UI::Textarea(this, spacing, offsy, _("Available Triggers: "), Align_Left);
   m_trigger_list=new UI::Listselect<Trigger_Descr &>(this, spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55);
   m_trigger_list->selected.set(this, &Editor_Event_Menu_New_Trigger::selected);
   m_trigger_list->double_clicked.set(this, &Editor_Event_Menu_New_Trigger::double_clicked);
   uint32_t i=0;
   for (i=0; i<Trigger_Factory::get_nr_of_available_triggers(); i++) {
		Trigger_Descr & d = *Trigger_Factory::get_trigger_descr(i);
		m_trigger_list->add(i18n::translate(d.name).c_str(), d);
	}
   m_trigger_list->sort();

   // Descr List
   new UI::Textarea(this, (get_inner_w()/2)+spacing, offsy, _("Description: "), Align_Left);
   m_description=new UI::Multiline_Textarea(this, (get_inner_w()/2)+spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55, "", Align_Left);

   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-80-spacing;

	m_ok_button = new UI::Button<Editor_Event_Menu_New_Trigger>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_New_Trigger::clicked_ok, this,
		 _("Ok"),
		 std::string(),
		 false);

	posx=(get_inner_w()/2)+spacing;

	new UI::IDButton<Editor_Event_Menu_New_Trigger, int32_t>
		(this,
		 posx, posy, 80, 20,
		 1,
		 &Editor_Event_Menu_New_Trigger::end_modal, this, 0,
		 _("Cancel"));

   center_to_parent();
}

/*
 * cleanup
 */
Editor_Event_Menu_New_Trigger::~Editor_Event_Menu_New_Trigger() {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Editor_Event_Menu_New_Trigger::handle_mousepress
(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Editor_Event_Menu_New_Trigger::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}

/*
 * a button has been clicked
 */
void Editor_Event_Menu_New_Trigger::clicked_ok() {
   // Create new trigger
	Trigger * const trig = Trigger_Factory::make_trigger_with_option_dialog
		(m_trigger_list->get_selected().id.c_str(), m_parent, 0);
	if (not trig) return; //  None created, don't close. Let user choose other.
	m_parent->egbase().map().get_mtm().register_new_trigger(trig);
   end_modal(1);
   return;
}

/*
 * the listbox got selected
 */
void Editor_Event_Menu_New_Trigger::selected(uint32_t) {
	m_description->set_text(_(m_trigger_list->get_selected().descr));
   m_ok_button->set_enabled(true);
}

/*
 * listbox got double clicked
 */
void Editor_Event_Menu_New_Trigger::double_clicked(uint32_t) {clicked_ok();}
