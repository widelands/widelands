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

#include "event_message_box_message_box.h"

#include "event_message_box.h"
#include "game.h"
#include "graphic/graphic.h"
#include "editorinteractive.h"
#include "constants.h"
#include "trigger/trigger_time.h"

#include "ui_button.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"

/**
 * The message box itself
 */
Message_Box_Event_Message_Box::Message_Box_Event_Message_Box
	(Widelands::Game              & game,
	 Widelands::Event_Message_Box * event,
	 int32_t gposx, int32_t gposy, int32_t w, int32_t h)
:
UI::Window(game.get_ibase(), 0, 0, 600, 400, event->get_window_title()),
m_game    (game)
{


	UI::Multiline_Textarea * m_text = 0;
	int32_t const spacing = 5;
	int32_t       offsy   = 5;
	int32_t       offsx   = spacing;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	set_inner_size(w, h);
	m_text =
		new UI::Multiline_Textarea
			(this,
			 posx, posy,
			 get_inner_w() - posx -     spacing,
			 get_inner_h() - posy - 2 * spacing - 50,
			 "", Align_Left);

	if (m_text)
		m_text->set_text(event->get_text());

	int32_t const but_width = 80;
	int32_t space = get_inner_w() - 2 * spacing;
	space -= but_width * event->get_nr_buttons();
	space /= event->get_nr_buttons() + 1;
	posx = spacing;
	posy = get_inner_h() - 30;
	m_trigger.resize(event->get_nr_buttons());
	for (int32_t i = 0; i < event->get_nr_buttons(); ++i) {
		posx += space;
		new UI::Callback_IDButton<Message_Box_Event_Message_Box, int32_t>
			(this,
			 posx, posy, but_width, 20,
			 0,
			 &Message_Box_Event_Message_Box::clicked, *this, i,
			 event->get_button_name(i));
		posx += but_width;
		m_trigger[i] = event->get_button_trigger(i);
	}

	m_is_modal = event->get_is_modal();

	center_to_parent();

	if (gposx != -1) set_pos(Point(gposy, get_y()));
	if (gposy != -1) set_pos(Point(get_x(), gposy));
}


/**
 * Handle mouse button events
 *
 * If the dialog has only one button, then it can be closed by right-clicking,
 * which will perform the same thing as pressing the button.
 * If there is more than one button, the player has to make a choice by
 * explicitly clicking one of the buttons, and closing via right-click is not
 * possible.
 *
 * We are not draggable.
 */
bool Message_Box_Event_Message_Box::handle_mousepress
	(const Uint8 btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_RIGHT && m_trigger.size() == 1) {
		play_click();
		clicked(0);
		return true;
	} else
		return false;
}


bool Message_Box_Event_Message_Box::handle_mouserelease
	(Uint8, int32_t, int32_t)
{
	return false;
}

/**
 * Clicked
 */
void Message_Box_Event_Message_Box::clicked(int32_t const i) {
	if (i == -1) { // we should end this dialog
		if (m_is_modal) {
			end_modal(0);
			return;
		} else {
			die();
			return;
		}
	} else {
		//  One of the buttons has been pressed
		if (Widelands::Trigger_Time * const t = m_trigger[i]) {
			//  FIXME This is totally broken. Here the click directly affects the
			//  FIXME game state, whitout passing the command queue. This fails
			//  FIXME horribly with replays or network games. (bug #2326416)
			t->set_time(0);
			t->check_set_conditions(m_game); //  forcefully update this trigger
		}
		clicked(-1);
		return;
	}
}
