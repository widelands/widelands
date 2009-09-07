/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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

#ifndef EVENT_FACTORY_H
#define EVENT_FACTORY_H

#include "event.h"

#include <string>
#include <cstring>

namespace Widelands {

/// Functions that create an event of some type and returns a reference to it.
/// The caller is given ownership of the created object and is therefore
/// responsible for that it is deallocated with operator delete. The event's
/// name and state are initialized to the given values.
namespace Event_Factory {

/// Creates an event of the type with the given number. A default name is given
/// and the state is set to INIT.
///
/// Assumes that the given index is less than the number of event types.
Event & create(size_t);

/// Creates an event of the type with the given number.
///
/// Assumes that the given index is less than the number of event types.
Event & create(size_t, char const * name, Event::State state);

/// Creates an event of the type with the given type name.
///
/// \Throws _wexception if there is no event type with the found name.
Event & create(Section &, Editor_Game_Base &);

struct Type_Descr {
	bool         const has_options_window;
	char const * const id;       /// The identifier that is written to files.
	std::string  const name;     /// Descriptive name for the user (localized).
	std::string  const helptext; /// Help text for the user (localized).
};

/// \Returns the description of the event type with the given index. The return
/// value is a reference to an item in the static array of event type
/// descriptions.
///
/// Assumes that the given index is less than the number of event types.
Type_Descr const & type_descr(size_t);

/// \Returns the number of event types, which is a compile-time constant.
size_t nr_event_types();
};

};

#endif
