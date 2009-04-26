/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
#include "graphic/graphic.h"
#include "map.h"
#include "trigger/trigger_time.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_listselect.h"
#include "ui_messagebox.h"
#include "ui_multilineeditbox.h"
#include "ui_textarea.h"
#include "ui_window.h"


inline Editor_Interactive & Event_Message_Box_Option_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


Event_Message_Box_Option_Menu::Event_Message_Box_Option_Menu
	(Editor_Interactive & parent, Widelands::Event_Message_Box & event)
:
UI::Window(&parent, 0, 0, 430, 400, _("Message Box Event Options")),
m_event   (event)
{
	int32_t const spacing =  5;
	Point         pos       (spacing, 25);
	m_nr_buttons  = m_event.get_nr_buttons();
	m_ls_selected = 0;

	m_buttons[0].name    = _("Continue");
	m_buttons[1].name    = _("Button 1");
	m_buttons[2].name    = _("Button 2");
	m_buttons[3].name    = _("Button 3");
	m_buttons[0].trigger = -1;
	m_buttons[1].trigger = -1;
	m_buttons[2].trigger = -1;
	m_buttons[3].trigger = -1;


	new UI::Textarea(this, pos.x, pos.y, 50, 20, _("Name:"), Align_CenterLeft);
	m_name =
		new UI::EditBox
			(this,
			 spacing + 60, pos.y, get_inner_w() / 2 - 60 - 2 * spacing, 20,
			 0, 0);
	m_name->setText(event.name());
	pos.y += 20 + spacing;

	new UI::Textarea
		(this,
		 get_inner_w() / 2 + spacing, pos.y, 150, 20,
		 _("Is Modal: "), Align_CenterLeft);
	pos.x = get_inner_w() - STATEBOX_WIDTH - spacing;
	m_is_modal = new UI::Checkbox(this, pos);
	m_is_modal->set_state(m_event.get_is_modal());

	pos.x  =      spacing;
	pos.y += 20 + spacing;

	new UI::Textarea
		(this, pos.x, pos.y, 50, 20, _("Window Title:"), Align_CenterLeft);
	pos.x += 100;
	m_window_title =
		new UI::EditBox
			(this, pos.x, pos.y, get_inner_w() - 100 - 2 * spacing, 20, 0, 2);
	m_window_title->setText(m_event.get_window_title());

	pos.x  =      spacing;
	pos.y += 20 + spacing;
	new UI::Textarea(this, pos.x, pos.y, 50, 20, _("Text:"), Align_CenterLeft);
	pos.y += 20 + spacing;
	m_text =
		new UI::Multiline_Editbox
			(this,
			 pos.x, pos.y, get_inner_w() - 2 * spacing, 80,
			 event.get_text());

	pos.y += 80 + spacing;

	new UI::Textarea
		(this,
		 pos.x, pos.y, 130, 20,
		 _("Number of Buttons: "), Align_CenterLeft);

	pos.x += 140;
	new UI::Callback_Button<Event_Message_Box_Option_Menu>
		(this,
		 pos.x, pos.y, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Event_Message_Box_Option_Menu::clicked_number_of_buttons_decrease,
		 *this);

	m_nr_buttons_ta =
		new UI::Textarea(this, spacing + 160 + spacing, pos.y, 15, 20, "1", Align_CenterLeft);

	pos.x += 30 + spacing;
	new UI::Callback_Button<Event_Message_Box_Option_Menu>
		(this,
		 pos.x, pos.y, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Event_Message_Box_Option_Menu::clicked_number_of_buttons_increase,
		 *this);

	pos.x  =      spacing;
	pos.y += 20 + spacing;
	new UI::Textarea
		(this, pos.x, pos.y, 100, 20, _("Button Name: "), Align_CenterLeft);
	pos.x += 110;
	m_button_name = new UI::EditBox(this, pos.x, pos.y, 100, 20, 0, 3);
	m_button_name->changedid.set
		(this, &Event_Message_Box_Option_Menu::edit_box_edited);

	//  Listbox for buttons
	pos.x = get_inner_w() / 2 + spacing;
	m_buttons_ls =
		new UI::Listselect<void *>
		(this,
		 pos.x, pos.y, get_inner_w() / 2 - 2 * spacing, 80,
		 Align_Left);
	m_buttons_ls->selected.set
		(this, &Event_Message_Box_Option_Menu::ls_selected);

	pos.x  =      spacing;
	pos.y += 20 + spacing;
	new UI::Textarea
		(this, pos.x, pos.y, 100, 20, _("Select Trigger: "), Align_CenterLeft);

	pos.x += 110;
	new UI::Callback_Button<Event_Message_Box_Option_Menu>
		(this,
		 pos.x, pos.y, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Event_Message_Box_Option_Menu::clicked_trigger_sel_decrease, *this);

	pos.x += 20 + spacing;
	new UI::Callback_Button<Event_Message_Box_Option_Menu>
		(this,
		 pos.x, pos.y, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Event_Message_Box_Option_Menu::clicked_trigger_sel_increase, *this);

	pos.x  =      spacing;
	pos.y += 20 + spacing;
	new UI::Textarea
		(this, pos.x, pos.y, 100, 20, _("Current: "), Align_CenterLeft);
	m_current_trigger_ta =
		new UI::Textarea
			(this,
			 pos.x + 15, pos.y + 15 + spacing, get_inner_w() / 2, 20,
			 _("No trigger selected!"), Align_CenterLeft);

	pos = Point(get_inner_w() / 2 - 60 - spacing, get_inner_h() - 30);
	new UI::Callback_Button<Event_Message_Box_Option_Menu>
		(this,
		 pos.x, pos.y, 60, 20, 0,
		 &Event_Message_Box_Option_Menu::clicked_ok, *this,
		 _("Ok"));
	pos.x = get_inner_w() / 2 + spacing;
	new UI::Callback_IDButton<Event_Message_Box_Option_Menu, int32_t>
		(this,
		 pos.x, pos.y, 60, 20,
		 1,
		 &Event_Message_Box_Option_Menu::end_modal, *this, 0,
		 _("Cancel"));

	Manager<Widelands::Trigger> const & mtm = parent.egbase().map().mtm();
	Manager<Widelands::Trigger>::Index const nr_triggs = mtm.size();
	for (Manager<Widelands::Trigger>::Index i = 0; i < nr_triggs; ++i) {
		if (dynamic_cast<Widelands::Trigger_Time const *>(&mtm[i]))
			m_button_triggers.push_back(i);
	}

	for (int32_t i = 0; i < m_event.get_nr_buttons(); ++i) {
		m_buttons[i].name = m_event.get_button_name(i);
		for (size_t j = 0; j < m_button_triggers.size(); ++j) {
			//  Get this trigger's index.
			int32_t foundidx = -1;
			for (Manager<Widelands::Trigger>::Index x = 0; x < nr_triggs; ++x)
				if (&mtm[x] == m_event.get_button_trigger(i)) {
					foundidx = x;
					break;
				}

			if (foundidx == m_button_triggers[j])
				m_buttons[i].trigger = j;
		}
	}

	center_to_parent();
	update();
}


/**
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
	std::string const & name = m_name->text();
	if (name.size()) {
		if
			(Widelands::Event * const registered_event =
			 eia().egbase().map().mem()[name])
			if (registered_event != & m_event) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another event registered with the name \"%s\". "
					 	 "Choose another name."),
					 name.c_str());
				UI::MessageBox mb
					(get_parent(),
					 _("Name in use"), buffer,
					 UI::MessageBox::OK);
				mb.run();
				return;
			}
		m_event.set_name(name);
	}
	if (m_text->get_text().c_str())
		m_event.set_text(m_text->get_text().c_str());
	m_event.set_window_title(m_window_title->text().c_str());
	m_event.set_is_modal(m_is_modal->get_state());
	m_event.set_nr_buttons(m_nr_buttons);
	Manager<Widelands::Trigger> & mtm = eia().egbase().map().mtm();
	for (uint32_t b = 0; b < m_nr_buttons; ++b) {
		m_event.set_button_name(b, m_buttons[b].name);
		m_event.set_button_trigger
			(b,
			 m_buttons[b].trigger == -1 ?
			 0
			 :
			 dynamic_cast<Widelands::Trigger_Time *>
			 	(&mtm[m_button_triggers[m_buttons[b].trigger]]));
	}
	end_modal(1);
}


void Event_Message_Box_Option_Menu::clicked_number_of_buttons_decrease() {
	--m_nr_buttons;
	if (m_nr_buttons < 1)
		m_nr_buttons = 1;
	update();
}


void Event_Message_Box_Option_Menu::clicked_number_of_buttons_increase() {
	++m_nr_buttons;
	if (m_nr_buttons > MAX_BUTTONS)
		m_nr_buttons = MAX_BUTTONS;
	update();
}


void Event_Message_Box_Option_Menu::clicked_trigger_sel_decrease() {
	--m_buttons[m_ls_selected].trigger;
	if (m_buttons[m_ls_selected].trigger < -1)
		m_buttons[m_ls_selected].trigger = m_button_triggers.size() - 1;
	update();
}


void Event_Message_Box_Option_Menu::clicked_trigger_sel_increase() {
	++m_buttons[m_ls_selected].trigger;
	if
		(m_buttons[m_ls_selected].trigger
		 >=
		 static_cast<int32_t>(m_button_triggers.size()))
		m_buttons[m_ls_selected].trigger = -1;
	update();
}


/**
 * Update function: update all UI elements
 */
void Event_Message_Box_Option_Menu::update() {
	if (m_ls_selected >= m_nr_buttons)
		m_buttons_ls->select(0);

	if (m_button_triggers.empty()) //  No triggers, no other buttons.
		m_nr_buttons = 1;

	m_buttons_ls->clear();
	for (uint32_t i = 0; i < m_nr_buttons; ++i)
		m_buttons_ls->add(m_buttons[i].name.c_str(), 0);

	{
		char text[2];
		text[0] = '0' + m_nr_buttons;
		text[1] = '\0';
		m_nr_buttons_ta->set_text(text);
	}


	m_button_name->setText(m_buttons[m_ls_selected].name);

	if (m_nr_buttons && m_button_triggers.size()) {
		m_current_trigger_ta->set_text
			(m_buttons[m_ls_selected].trigger == -1 ?
			 "none"
			 :
			 eia().egbase().map().mtm()
			 	[m_button_triggers[m_buttons[m_ls_selected].trigger]]
			 .name());
	} else {
		m_current_trigger_ta->set_text("---");
		m_buttons[0].trigger = -1;
	}
}

/**
 * Listbox got selected
 */
void Event_Message_Box_Option_Menu::ls_selected(uint32_t const i) {
	m_ls_selected = i;
	update();
}

/**
 * Button name edit box edited
 */
void Event_Message_Box_Option_Menu::edit_box_edited(int32_t) {
	m_buttons[m_ls_selected].name = m_button_name->text();
	update();
}
