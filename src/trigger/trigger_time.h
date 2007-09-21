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

#ifndef __S__TRIGGER_TIME_H
#define __S__TRIGGER_TIME_H

#include <stdint.h>
#include "trigger.h"

/*
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
struct Trigger_Time : public Trigger {
      Trigger_Time();

      // one liner functions
	const char * get_id() const {return "time";}

      void check_set_conditions(Game*);
      void reset_trigger(Game*);

      // File Functions
	void Write(Section &) const;
      void Read(Section*, Editor_Game_Base*);

      inline void set_wait_time(int i) {m_wait_time=i;}
      inline void set_last_start_time(int i) {m_last_start_time = i;}
	int get_wait_time() const {return m_wait_time;}

private:
      uint32_t m_wait_time; // in seconds
      uint32_t m_last_start_time;
};

#endif
