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

#ifndef __S__EVENT_MESSAGEBOX_H
#define __S__EVENT_MESSAGEBOX_H

#include <vector>
#include <string>
#include "event.h"
#include "trigger_referencer.h"

class Trigger_Null;
class Editor_Game_Base;
class UIPanel;

/*
 * This event shows a message box
 */
class Event_Message_Box : public Event, public TriggerReferencer {
   public:
     Event_Message_Box();
      ~Event_Message_Box();

      // For trigger referenecer
      virtual const char* get_type( void ) { return "Event:MessageBox"; }
      virtual const char* get_name( void ) { return Event::get_name(); }

      // one liner functions
      const char* get_id(void) { return "message_box"; }

      State run(Game*);
      virtual void reinitialize(Game*);

      // File Functions
      void Write(Section*, Editor_Game_Base*);
      void Read(Section*, Editor_Game_Base*);

      inline void set_text(const char* str) { m_text=str; }
      inline const char* get_text(void) { return m_text.c_str(); }
      inline void set_window_title(const char* str) { m_window_title=str; }
      inline const char* get_window_title(void) { return m_window_title.c_str(); }
      inline void set_is_modal(bool t) {  m_is_modal=t; }
      inline bool get_is_modal(void) { return m_is_modal; }
      inline void set_pos( int posx, int posy ) { m_posx=posx; m_posy=posy; }
      inline int get_posx( void ) { return m_posx; }
      inline int get_posy( void ) { return m_posy; }
      inline void set_dimensions( int w, int h ) { m_width = w; m_height = h; }
      inline int get_w( void ) { return m_width; }
      inline int get_h( void ) { return m_height; }
      void set_button_trigger(int i, Trigger_Null* t);
      Trigger_Null* get_button_trigger(int i);
      void set_button_name(int i, std::string);
      const char* get_button_name(int i);
      void set_nr_buttons(int i);
      int get_nr_buttons(void);

      enum {
         Right = 0,
         Left,
         Center_under,
         Center_over,
      };

   protected:

   private:
      struct Button_Descr {
         std::string name;
         Trigger_Null *trigger;
      };

      std::string m_text;
      std::string m_window_title;
      bool m_is_modal;

      std::vector<Button_Descr> m_buttons;
      UIPanel*      m_window;
      int  m_posx, m_posy;
      int  m_width, m_height;
};



#endif
