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
      
      // File Functions
      void Write(FileWrite*, Editor_Game_Base*);
      void Read(FileRead*, Editor_Game_Base*, bool);
  
      inline void set_string(const char* str) { m_string=str; }
      inline const char* get_string(void) { return m_string.c_str(); }
      
   private:
      std::string m_string;
};



#endif

