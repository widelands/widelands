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

#include "event_message_box.h"

#include "editor_game_base.h"
#include "event_message_box_message_box.h"
#include "filesystem.h"
#include "game.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "map_trigger_manager.h"
#include "profile.h"
#include "trigger/trigger_null.h"
#include "wexception.h"

namespace Widelands {

static const int32_t EVENT_VERSION = 1;

/*
 * Init and cleanup
 */
Event_Message_Box::Event_Message_Box() : Event(_("Message Box")) {
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
      if (m_buttons[i].trigger) {
         set_button_trigger(i, 0);
		}
   m_buttons.resize(0);

}

/*
 * reinitialize
 */
void Event_Message_Box::reinitialize(Game *) {}

/*
 * functions for button handling
 */
void Event_Message_Box::set_nr_buttons(int32_t i) {
   int32_t oldsize=m_buttons.size();
   m_buttons.resize(i);
	for (uint32_t j = oldsize; j < m_buttons.size(); ++j)
      m_buttons[j].trigger=0;
}


void Event_Message_Box::set_button_trigger(int32_t i, Trigger_Null* t) {
   assert(i<get_nr_buttons());
   if (m_buttons[i].trigger==t) return;

   if (m_buttons[i].trigger)
      unreference_trigger(m_buttons[i].trigger) ;

   if (t)
      reference_trigger(t);
   m_buttons[i].trigger=t;
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

/*
 * File Read, File Write
 */
void Event_Message_Box::Read(Section* s, Editor_Game_Base* egbase) {
	const int32_t version = s->get_safe_int("version");
	if (version == EVENT_VERSION) {
      set_name(s->get_name());
      set_text(s->get_safe_string("text"));
      set_window_title(s->get_safe_string("window_title"));
      set_is_modal(s->get_safe_bool("is_modal"));

      m_posx = s->get_int("posx", -1);
      m_posy = s->get_int("posy", -1);
      m_width = s->get_int("width", 400);
      m_height = s->get_int("height", 300);

      uint32_t nr_buttons = s->get_safe_int("number_of_buttons");
      set_nr_buttons(nr_buttons);
      char buf[256];
		for (uint32_t i = 0; i < nr_buttons; ++i) {
         sprintf(buf, "button_%02i_name", i);
         set_button_name(i, s->get_safe_string(buf));

         sprintf(buf, "button_%02i_has_trigger", i);
         bool trigger = s->get_safe_bool(buf);

			if (trigger) {
            sprintf(buf, "button_%02i_trigger", i);
            Trigger * const t = egbase->get_map()->get_mtm().get_trigger(s->get_safe_string(buf)); // Hopefully it is a null trigger
            set_button_trigger(i, static_cast<Trigger_Null*>(t));
			} else
            set_button_trigger(i, 0);
		}
      return;
	}
   throw wexception("Unknown Version in Event_Message_Box::Read: %i", version);
}

void Event_Message_Box::Write(Section & s, const Editor_Game_Base &) const {
	s.set_int   ("version",           EVENT_VERSION);
	s.set_string("text",              m_text.c_str());
	s.set_string("window_title",      m_window_title.c_str());
	s.set_bool  ("is_modal",          get_is_modal());
	s.set_int   ("number_of_buttons", get_nr_buttons());
	s.set_int   ("width",             get_w());
	s.set_int   ("height",            get_h());
	s.set_int   ("posx",              get_posx());
	s.set_int   ("posy",              get_posy());

   char buf[256];
	for (int32_t i=0; i < get_nr_buttons(); ++i) {
      sprintf(buf, "button_%02i_name", i);
		s.set_string(buf, m_buttons[i].name.c_str());


      sprintf(buf, "button_%02i_has_trigger", i);
		s.set_bool(buf, m_buttons[i].trigger == 0 ? 0 : 1);
      if (m_buttons[i].trigger) {
         sprintf(buf, "button_%02i_trigger", i);
			s.set_string(buf, m_buttons[i].trigger->get_name());
		}
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
