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

#ifndef __S__MESSAGEBOX_H
#define __S__MESSAGEBOX_H

#include <string>
#include "event.h"

class Trigger_Null;
class Editor_Game_Base;

/*
 * This event shows a message box
 */
class Event_Message_Box : public Event {
   public:
     Event_Message_Box();
      ~Event_Message_Box();
      
      // one liner functions
      uint get_id(void) { return EVENT_MESSAGE_BOX; } 

      void run(Game*);
      virtual void reinitialize(Game*);
      virtual void cleanup(Editor_Game_Base* g);

      // File Functions
      void Write(FileWrite*, Editor_Game_Base*);
      void Read(FileRead*, Editor_Game_Base*, bool);
  
      inline void set_text(const char* str) { m_text=str; }
      inline const char* get_text(void) { return m_text.c_str(); }
      inline void set_caption(const char* str) { m_caption=str; }
      inline const char* get_caption(void) { return m_caption.c_str(); }
      inline void set_window_title(const char* str) { m_window_title=str; }
      inline const char* get_window_title(void) { return m_window_title.c_str(); }
      inline void set_pic_id(int i) { m_pic_id=i; }
      inline uint get_pic_id(void) { return m_pic_id; }
      inline void set_pic_position(int i) { m_pic_position=i; }
      inline int  get_pic_position(void) { return m_pic_position; }
      inline void set_is_modal(bool t) {  m_is_modal=t; }
      inline bool get_is_modal(void) { return m_is_modal; }
      void set_button_trigger(int i, Trigger_Null* t, Map*);
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
      std::string m_caption;
      std::string m_window_title;
      bool m_is_modal;
      uint m_pic_id;
      int m_pic_position;

      std::vector<Button_Descr> m_buttons;
      
};



#endif

