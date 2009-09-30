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

#include "trigger_time_option_menu.h"

#include "graphic/graphic.h"
#include "i18n.h"
#include "trigger/trigger_time.h"
#include "editor/editorinteractive.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/textarea.h"


int32_t Widelands::Trigger_Time::option_menu(Editor_Interactive & eia) {
	Trigger_Time_Option_Menu m(eia, *this); return m.run();
}


inline Editor_Interactive & Trigger_Time_Option_Menu::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


Trigger_Time_Option_Menu::Trigger_Time_Option_Menu
	(Editor_Interactive & parent, Widelands::Trigger_Time & trigger)
	:
	UI::Window(&parent, 0, 0, 200, 150, _("Trigger Option Menu")),
	m_trigger (trigger)
{
	int32_t const offsx   = 20;
	int32_t const offsy   =  5;
	int32_t const spacing =  5;
	int32_t const width   = 20;
	int32_t const height  = 20;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	new UI::Textarea
		(this, spacing, posy, 50, 20, _("Name:"), UI::Align_CenterLeft);
	m_name = new UI::EditBox
		(this, spacing + 50, posy, get_inner_w() - 50 - 2 * spacing, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"), 0);
	m_name->setText(trigger.name());

	posy += 20 + spacing;

	m_absolute_time =
		new UI::Checkbox
			(this, Point(spacing, posy),
			 g_gr->get_no_picture(),
			 _
			 	("Trigger at an absolute time from the beginning of the game "
			 	 "(otherwise, only trigger if activated by an event set timer)."));
	m_absolute_time->clickedto.set
		(this, &Trigger_Time_Option_Menu::absolute_time_clickedto);
	new UI::Textarea(this, 30, posy, _("Trigger at absolute time"));

	posy += 25;

	Widelands::Time time = trigger.time();
	if (time == Widelands::Never()) {
		time = 0;
		m_absolute_time->set_state(false);
	} else
		m_absolute_time->set_state(true);
	m_values[0] = time / 3600 /   10; //  hours
	m_values[1] = time / 3600 %   10;
	time       -= time / 3600 * 3600;
	assert(0 <= time);
	m_values[2] = time /   60 /   10; //  minutes
	m_values[3] = time /   60 %   10;
	time       -= time /   60 *   60;
	assert(0 <= time);
	m_values[4] = time        /   10; //  seconds
	m_values[5] = time        %   10;

	//  hours, first cipher

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 2);

	m_textareas[0] =
		new UI::Textarea
			(this, posx, posy + 20, width, height, "9", UI::Align_Center);

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy + 40, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 3);

	posx += width + spacing / 2;

	//  hours second cipher

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 4);

	m_textareas[1] =
		new UI::Textarea
			(this, posx, posy + 20, width, height, "9", UI::Align_Center);

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy + 40, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 5);

	posx += width + spacing + spacing;
	posx +=
		(new UI::Textarea
		 	(this, posx, posy + 20, 0, height, ":", UI::Align_Center))
		->get_w();

	//  minutes, first cipher

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 6);

	m_textareas[2] =
		new UI::Textarea
			(this, posx, posy + 20, width, height, "9", UI::Align_Center);

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy + 40, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 7);

	posx += width + spacing / 2;

	//  minutes second cipher

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 8);

	m_textareas[3] =
		new UI::Textarea
			(this, posx, posy + 20, width, height, "9", UI::Align_Center);

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy + 40, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 9);

	posx += width + spacing + spacing;
	posx +=
		(new UI::Textarea
		 	(this, posx, posy + 20, 0, height, ":", UI::Align_Center))
		->get_w();
	// seconds, first cipher

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 10);

	m_textareas[4] =
		new UI::Textarea
			(this, posx, posy + 20, width, height, "9", UI::Align_Center);

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy + 40, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 11);

	posx += width + spacing / 2;
	//  seconds, second cipher

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 12);

	m_textareas[5] =
		new UI::Textarea
			(this, posx, posy + 20, width, height, "9", UI::Align_Center);

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy + 40, width, height,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Time_Option_Menu::clicked, *this, 13);

	posx += width + spacing;

	posy += 60;

	posy += 2 * spacing;

	posx = get_inner_w() / 2 - 80 - spacing;

	new UI::Callback_Button<Trigger_Time_Option_Menu>
		(this,
		 posx, posy, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Trigger_Time_Option_Menu::clicked_ok, *this,
		 _("Ok"));

	posx = get_inner_w() / 2 + spacing;

	new UI::Callback_IDButton<Trigger_Time_Option_Menu, int32_t>
		(this,
		 posx, posy, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Trigger_Time_Option_Menu::end_modal, *this, 0,
		 _("Cancel"));

	center_to_parent();
	update();
}


void Trigger_Time_Option_Menu::absolute_time_clickedto(bool const on)
{
	if (not on)
		for (uint8_t i = 6; i;)
			m_values[--i] = 0;
	update();
}

/**
 * Handle mousepress/-release
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Trigger_Time_Option_Menu::handle_mousepress
	(Uint8 const btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_RIGHT) {
		end_modal(0);
		return true;
	}
	return false;
}
bool Trigger_Time_Option_Menu::handle_mouserelease(Uint8, int32_t, int32_t)
{
	return false;
}


void Trigger_Time_Option_Menu::clicked_ok() {
	m_trigger.set_time
		(m_absolute_time->get_state() ?
		 (m_values[0] * 10 + m_values[1]) * 3600
		 +
		 (m_values[2] * 10 + m_values[3]) * 60
		 +
		 (m_values[4] * 10 + m_values[5])
		 :
		 Widelands::Never());
	std::string const & name = m_name->text();
	if (name.size()) {
		if
			(Widelands::Trigger * const registered_trigger =
			 eia().egbase().map().mtm()[name])
			if (registered_trigger != & m_trigger) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another trigger registered with the name "
					 	 "\"%s\". Choose another name."),
					 name.c_str());
				UI::WLMessageBox mb
					(get_parent(),
					 _("Name in use"), buffer,
					 UI::WLMessageBox::OK);
				mb.run();
				return;
			}
		m_trigger.set_name(name);
	}
	end_modal(1);
}


void Trigger_Time_Option_Menu::clicked(int32_t const i) {
	int32_t id = (i - 2) / 2;
	assert(0 <= m_values[id]);
	assert     (m_values[id] <= 9);
	m_values[id] = i % 2 ?  m_values[id] - 1 :  m_values[id] + 1;
	if (m_values[id] < 0)
		m_values[id] = 0;
	if (m_values[id] > 9)
		m_values[id] = 9;
	if ((id == 2 || id == 4) && m_values[id] >= 6)
		m_values[id] = 5;
	assert(0 <= m_values[id]);
	assert     (m_values[id] <= 9);
	m_absolute_time->set_state(true);
	update();
}

/**
 * Update the Textareas
 */
void Trigger_Time_Option_Menu::update() {
	for (size_t i = 0; i < 6; ++i) {
		char str[2];
		assert(0 <= m_values[i]);
		assert     (m_values[i] <= 9);
		str[0] = '0' + m_values[i];
		str[1] = '\0';
		m_textareas[i]->set_text(str);
	}
}
