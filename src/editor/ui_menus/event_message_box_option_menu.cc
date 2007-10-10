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

#include "event_message_box_option_menu.h"

#include "events/event_message_box.h"
#include "i18n.h"
#include "editorinteractive.h"
#include "graphic.h"
#include "map.h"
#include "map_trigger_manager.h"
#include "trigger/trigger_null.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_multilineeditbox.h"
#include "ui_textarea.h"
#include "ui_window.h"


Event_Message_Box_Option_Menu::Event_Message_Box_Option_Menu
(Editor_Interactive* parent, Event_Message_Box* event)
:
UI::Window(parent, 0, 0, 430, 400, _("Message Box Event Options").c_str()),
m_event(event),
m_parent(parent)
{

   const int32_t offsx=5;
   const int32_t offsy=25;
   const int32_t spacing=5;
   int32_t posx=offsx;
   int32_t posy=offsy;
   m_nr_buttons=m_event->get_nr_buttons();
   m_ls_selected=0;

   m_buttons[0].name=_("Continue");
   m_buttons[1].name=_("Button 1");
   m_buttons[2].name=_("Button 2");
   m_buttons[3].name=_("Button 3");
   m_buttons[0].trigger=m_buttons[1].trigger=m_buttons[2].trigger=m_buttons[3].trigger=-1;


   // Name editbox
   new UI::Textarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UI::Edit_Box(this, spacing+60, posy, get_inner_w()/2-60-2*spacing, 20, 0, 0);
   m_name->set_text(event->get_name());
   posy+= 20+spacing;

   // Modal cb
   new UI::Textarea(this, get_inner_w()/2+spacing, posy, 150, 20, _("Is Modal: "), Align_CenterLeft);
   m_is_modal=new UI::Checkbox(this, get_inner_w()-STATEBOX_WIDTH-spacing, posy);
   m_is_modal->set_state(m_event->get_is_modal());

   posy+=20+spacing;

   // Window Title
   new UI::Textarea(this, spacing, posy, 50, 20, _("Window Title:"), Align_CenterLeft);
   m_window_title=new UI::Edit_Box(this, spacing+100, posy, get_inner_w()-100-2*spacing, 20, 0, 2);
   m_window_title->set_text(m_event->get_window_title());

   // Text
   posy+=20+spacing;
   new UI::Textarea(this, spacing, posy, 50, 20, _("Text:"), Align_CenterLeft);
   posy+=20+spacing;
   m_text=new UI::Multiline_Editbox(this, spacing, posy, get_inner_w()-2*spacing, 80, event->get_text());

   posy+=80+spacing;

   // Nur Buttons
   new UI::Textarea(this, spacing, posy, 130, 20, _("Number of Buttons: "), Align_CenterLeft);

	new UI::Button<Event_Message_Box_Option_Menu>
		(this,
		 spacing + 140, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Event_Message_Box_Option_Menu::clicked_number_of_buttons_decrease,
		 this);

	m_nr_buttons_ta=new UI::Textarea(this, spacing+160+spacing, posy, 15, 20, "1", Align_CenterLeft);

	new UI::Button<Event_Message_Box_Option_Menu>
		(this,
		 spacing + 175 + spacing, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Event_Message_Box_Option_Menu::clicked_number_of_buttons_increase,
		 this);

   // Button name
   posy+=20+spacing;
   new UI::Textarea(this, spacing, posy, 100, 20, _("Button Name: "), Align_CenterLeft);
   m_button_name=new UI::Edit_Box(this, spacing+110, posy, 100, 20, 0, 3);
   m_button_name->changedid.set(this, &Event_Message_Box_Option_Menu::edit_box_edited);

   // Listbox for buttons
   m_buttons_ls=new UI::Listselect<void *>(this, get_inner_w()/2+spacing, posy, get_inner_w()/2-2*spacing, 80, Align_Left);
   m_buttons_ls->selected.set(this, &Event_Message_Box_Option_Menu::ls_selected);

   // Select trigger
   posy+=20+spacing;
   new UI::Textarea(this, spacing, posy, 100, 20, _("Select Trigger: "), Align_CenterLeft);

	new UI::Button<Event_Message_Box_Option_Menu>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Event_Message_Box_Option_Menu::clicked_trigger_sel_decrease, this);

	new UI::Button<Event_Message_Box_Option_Menu>
		(this,
		 spacing + 130 + spacing, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Event_Message_Box_Option_Menu::clicked_trigger_sel_increase, this);

   // Current Trigger
   posy+=20+spacing;
   new UI::Textarea(this, spacing, posy, 100, 20, _("Current: "), Align_CenterLeft);
   m_current_trigger_ta=new UI::Textarea(this, spacing+15, posy+15+spacing, get_inner_w()/2, 20, _("No trigger selected!"), Align_CenterLeft);

   // Ok/Cancel Buttons
   posx=(get_inner_w()/2)-60-spacing;
   posy=get_inner_h()-30;
	new UI::Button<Event_Message_Box_Option_Menu>
		(this,
		 posx, posy, 60, 20, 0,
		 &Event_Message_Box_Option_Menu::clicked_ok, this,
		 _("Ok"));
   posx=(get_inner_w()/2)+spacing;
	new UI::IDButton<Event_Message_Box_Option_Menu, int32_t>
		(this,
		 posx, posy, 60, 20,
		 1,
		 &Event_Message_Box_Option_Menu::end_modal, this, 0,
		 _("Cancel"));

	const MapTriggerManager & mtm = m_parent->egbase().map().get_mtm();
	const MapTriggerManager::Index nr_triggers = mtm.get_nr_triggers();
	for (MapTriggerManager::Index i = 0; i < nr_triggers; ++i) {
		if (strcmp(mtm.get_trigger_by_nr(i).get_id(), "null") == 0)
         m_null_triggers.push_back(i);
	}

	for (int32_t i = 0; i < m_event->get_nr_buttons(); ++i) {
      m_buttons[i].name=m_event->get_button_name(i);
		for (size_t j = 0; j < m_null_triggers.size(); ++j) {
         // Get this triggers index
         int32_t foundidx = -1;
			for (MapTriggerManager::Index x = 0; x < nr_triggers; ++x)
				if (&mtm.get_trigger_by_nr(x) == m_event->get_button_trigger(i)) {
               foundidx = x;
               break;
				}

         if (foundidx==m_null_triggers[j])
            m_buttons[i].trigger=j;
		}
	}

   center_to_parent();
   update();
}

/*
 * cleanup
 */
Event_Message_Box_Option_Menu::~Event_Message_Box_Option_Menu() {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Event_Message_Box_Option_Menu::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Event_Message_Box_Option_Menu::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}


void Event_Message_Box_Option_Menu::clicked_ok() {
            if (m_name->get_text())
               m_event->set_name(m_name->get_text());
            if (m_text->get_text().c_str())
               m_event->set_text(m_text->get_text().c_str());
            if (m_window_title->get_text())
               m_event->set_window_title(m_window_title->get_text());
            m_event->set_is_modal(m_is_modal->get_state());
            m_event->set_nr_buttons(m_nr_buttons);
	const MapTriggerManager & mtm = m_parent->egbase().map().get_mtm();
	for (uint32_t b = 0; b < m_nr_buttons; ++b) {
               m_event->set_button_name(b, m_buttons[b].name);
		if (m_buttons[b].trigger != -1) {
                  m_event->set_button_trigger
                     (b,
                      static_cast<Trigger_Null*>
                      (&mtm.get_trigger_by_nr
                       (m_null_triggers[m_buttons[b].trigger])));
		} else m_event->set_button_trigger(b, 0);
	}
	end_modal(1);
}


void Event_Message_Box_Option_Menu::clicked_number_of_buttons_decrease() {
            m_nr_buttons--;
            if (m_nr_buttons<1) m_nr_buttons=1;
            update();
}


void Event_Message_Box_Option_Menu::clicked_number_of_buttons_increase() {
            m_nr_buttons++;
            if (m_nr_buttons>MAX_BUTTONS) m_nr_buttons=MAX_BUTTONS;
            update();
}


void Event_Message_Box_Option_Menu::clicked_trigger_sel_decrease() {
            m_buttons[m_ls_selected].trigger--;
            if (m_buttons[m_ls_selected].trigger<-1) m_buttons[m_ls_selected].trigger=m_null_triggers.size()-1;
            update();
}


void Event_Message_Box_Option_Menu::clicked_trigger_sel_increase() {
            m_buttons[m_ls_selected].trigger++;
            if (m_buttons[m_ls_selected].trigger>=static_cast<int32_t>(m_null_triggers.size()))
               m_buttons[m_ls_selected].trigger=-1;
            update();
}


/*
 * update function: update all UI elements
 */
void Event_Message_Box_Option_Menu::update() {
   if (m_ls_selected>=m_nr_buttons) m_buttons_ls->select(0);

   if (!m_null_triggers.size()) {
      // No triggers, no other buttons
      m_nr_buttons=1;
	}

   m_buttons_ls->clear();
   for (uint32_t i=0; i<m_nr_buttons; i++)
		m_buttons_ls->add(m_buttons[i].name.c_str(), 0);

   std::string text;
   text.append(1, static_cast<uint8_t>(m_nr_buttons+0x30));
   m_nr_buttons_ta->set_text(text.c_str());


   m_button_name->set_text(m_buttons[m_ls_selected].name.c_str());

   if (m_nr_buttons && m_null_triggers.size()) {
		m_current_trigger_ta->set_text
			(m_buttons[m_ls_selected].trigger == -1 ?
			 "none"
			 :
			 m_parent->egbase().map().get_mtm().get_trigger_by_nr
			 (m_null_triggers[m_buttons[m_ls_selected].trigger]).get_name());
	} else {
      m_current_trigger_ta->set_text("---");
      m_buttons[0].trigger=-1;
	}
}

/*
 * Listbox got selected
 */
void Event_Message_Box_Option_Menu::ls_selected(uint32_t i) {
   m_ls_selected=i;
   update();
}

/*
 * Button name edit box edited
 */
void Event_Message_Box_Option_Menu::edit_box_edited(int32_t) {
   m_buttons[m_ls_selected].name= m_button_name->get_text() ;
   update();
}
