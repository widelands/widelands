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

#include "event_message_box.h"

#include "editor_game_base.h"
#include "event_message_box_message_box.h"
#include "filesystem.h"
#include "game.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "profile.h"
#include "trigger/trigger_null.h"
#include "wexception.h"

#include "upcast.h"

#define EVENT_VERSION 2

namespace Widelands {

Event_Message_Box::Event_Message_Box(char const * const Name, State const S)
	: Event(Name, S) {
	set_text(_("No text defined").c_str());
	set_window_title(_("Window Title").c_str());
   set_is_modal(false);
   set_dimensions(400, 300);
   set_pos(-1, -1);

   set_nr_buttons(1);
   m_buttons[0].name=_("Continue");
   m_buttons[0].trigger=0;

   m_window = 0;
}

Event_Message_Box::~Event_Message_Box() {
	for (uint32_t i = 0; i < m_buttons.size(); ++i)
		if (m_buttons[i].trigger)
         set_button_trigger(i, 0);
}


/*
 * functions for button handling
 */
void Event_Message_Box::set_nr_buttons(int32_t i) {
   int32_t oldsize=m_buttons.size();
   m_buttons.resize(i);
	for (uint32_t j = oldsize; j < m_buttons.size(); ++j)
      m_buttons[j].trigger=0;
}


void Event_Message_Box::set_button_trigger
	(uint8_t button_number, Trigger_Null * const new_trigger)
{
	assert(button_number < get_nr_buttons());
	Trigger * const old_trigger = m_buttons[button_number].trigger;
	if (new_trigger != old_trigger) {
		if (old_trigger) Referencer<Trigger>::unreference(*old_trigger);
		if (new_trigger) Referencer<Trigger>::  reference(*new_trigger);
		m_buttons[button_number].trigger = new_trigger;
	}
}
Trigger_Null* Event_Message_Box::get_button_trigger(int32_t i) {
   assert(i<get_nr_buttons());
   return m_buttons[i].trigger;
}
void Event_Message_Box::set_button_name(int32_t i, std::string str) {
   assert(i<get_nr_buttons());
   m_buttons[i].name=str;
}
const char* Event_Message_Box::get_button_name(int32_t i) {
   assert(i<get_nr_buttons());
   return m_buttons[i].name.c_str();
}


void Event_Message_Box::Read(Section & s, Editor_Game_Base & egbase) {
	m_buttons.clear();
	try {
		int32_t const packet_version = s.get_safe_int("version");
		if (1 <= packet_version and packet_version <= EVENT_VERSION) {
			set_name        (s.get_name());
			set_text        (s.get_safe_string("text"));
			set_window_title(s.get_safe_string("window_title"));
			set_is_modal    (s.get_safe_bool  ("is_modal"));

			m_posx   =       s.get_int        ("posx", -1);
			m_posy   =       s.get_int        ("posy", -1);
			m_width  =       s.get_int        ("width", 400);
			m_height =       s.get_int        ("height", 300);

			Manager<Trigger> & mtm = egbase.map().mtm();
			char key[] = "button_00\0trigger"; //  tailed string
			for (;;) {
				if (packet_version == 1) { //  modify part of tail for old version
					key [9] = '_';
					key[10] = 'n';
					key[11] = 'a';
					key[12] = 'm';
					key[13] = 'e';
					key[14] = '\0';
				}
				char const * const button_name = s.get_string(key);
				if (packet_version == 1) { //  restore part of tail
					key[10] = 't';
					key[11] = 'r';
					key[12] = 'i';
					key[13] = 'g';
					key[14] = 'g';
				}
				if (not button_name) {
					if (key[8] == '0' and key[7] == '0')
						throw wexception
							("there are no buttons, at least one is required");
					break;
				}
				Button_Descr descr;
				descr.name    = button_name;

				key [9] = '_'; //  Enable the tail of the key string.
				if (char const * const trigger_name = s.get_string(key)) {
					if (upcast(Trigger_Null, trigger, mtm[trigger_name])) {
						if (not m_is_modal)
							throw wexception
								("is not modal although %s=%s", key, trigger_name);
						descr.trigger = trigger;
						Referencer<Trigger>::reference(*trigger);
					} else
						throw wexception
							("%s refers to \"%s\", " "which is not a null trigger",
							 key, trigger_name);
				} else
					descr.trigger = 0;
				key [9] = '\0'; //  Cut off the tail again.
				m_buttons.push_back(descr);

				//  Increment the number in the key string.
				if (key[8] == '9') {key[8] = '0'; ++key[7];} else ++key[8];
			}
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (std::exception const & e) {
		throw wexception
			("(message box): %s", e.what());
	}
}

void Event_Message_Box::Write(Section & s, Editor_Game_Base const &) const {
	s.set_string("type",         "message_box");
	s.set_int   ("version",      EVENT_VERSION);
	s.set_string("text",         m_text);
	s.set_string("window_title", m_window_title);
	s.set_bool  ("is_modal",     get_is_modal());
	s.set_int   ("width",        get_w());
	s.set_int   ("height",       get_h());
	if (get_posx() != -1)
		s.set_int("posx",         get_posx());
	if (get_posy() != -1)
		s.set_int("posy",         get_posy());

	char key[] = "button_00\0trigger"; //  tailed string
	for (int32_t i=0; i < get_nr_buttons(); ++i) {
		s.set_string(key, m_buttons[i].name);
		if (m_buttons[i].trigger) {
			key[9] = '_'; //  Enable the "_trigger" tail of the key string.
			s.set_string(key, m_buttons[i].trigger->name());
			key[9] = '\0'; //  Cut off the tail again.
		}

		//  Increment the number in the key string.
		if (key[8] == '9') {key[8] = '0'; ++key[7];} else ++key[8];
	}
}

/*
 * check if trigger conditions are done
 */
Event::State Event_Message_Box::run(Game* game) {

   Message_Box_Event_Message_Box* mb=new Message_Box_Event_Message_Box(game, this, get_posx(), get_posy(), get_w(), get_h());
	if (get_is_modal()) {
      mb->run();
      delete mb;
	}

   m_state = DONE;
   return m_state;
}

};
