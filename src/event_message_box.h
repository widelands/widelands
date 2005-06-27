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
      virtual const wchar_t* get_type( void ) { return L"Event:MessageBox"; }
      virtual const wchar_t* get_name( void ) { return Event::get_name(); }
      
      // one liner functions
      const char* get_id(void) { return "message_box"; }

      State run(Game*);
      virtual void reinitialize(Game*);

      // File Functions
      void Write(Section*, Editor_Game_Base*);
      void Read(Section*, Editor_Game_Base*);

      inline void set_text(const wchar_t* str) { m_text=str; }
      inline const wchar_t* get_text(void) { return m_text.c_str(); }
      inline void set_caption(const wchar_t* str) { m_caption=str; }
      inline const wchar_t* get_caption(void) { return m_caption.c_str(); }
      inline void set_window_title(const wchar_t* str) { m_window_title=str; }
      inline const wchar_t* get_window_title(void) { return m_window_title.c_str(); }
      inline void set_is_modal(bool t) {  m_is_modal=t; }
      inline bool get_is_modal(void) { return m_is_modal; }
      void set_button_trigger(int i, Trigger_Null* t);
      Trigger_Null* get_button_trigger(int i);
      void set_button_name(int i, std::wstring);
      const wchar_t* get_button_name(int i);
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
         std::wstring name;
         Trigger_Null *trigger;
      };

      std::wstring m_text;
      std::wstring m_caption;
      std::wstring m_window_title;
      bool m_is_modal;

      std::vector<Button_Descr> m_buttons;
      UIPanel*      m_window;
};



#endif

