/*
 * Copyright (C) 2002-2006 by the Widelands Development Team
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

#ifndef __S__EVENT_REFERENCER_H
#define __S__EVENT_REFERENCER_H

#include <string>

namespace Widelands {

struct Event;

/*
 * An Event referencer is a class, that uses an Event
 */
struct EventReferencer {
      virtual ~EventReferencer() {}
	virtual const char * get_type() const = 0;
	virtual const std::string & name() const throw () = 0;

      void reference_event(Event*);
      void unreference_event(Event*);
};

};

#endif // __S__EVENT_REFERENCER_H
