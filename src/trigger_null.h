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

#ifndef __S__TRIGGER_NULL_H
#define __S__TRIGGER_NULL_H

#include "trigger.h"

/*
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
class Trigger_Null : public Trigger {
   public:
      Trigger_Null();
      ~Trigger_Null();

      // one liner functions
      const char* get_id(void) { return "null"; }

      void check_set_conditions(Game*);
      void reset_trigger(Game*);

      // File Functions
      void Write(Section*);
      void Read(Section*, Editor_Game_Base*);

      void set_trigger_manually(bool t) { m_should_toggle=true; m_value=t; }

   private:
      bool m_should_toggle;
      bool m_value;
};

#endif

