/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef UI_OBJECT_H
#define UI_OBJECT_H

namespace UI {
/**
 * Serves as a base class for UI related objects. The only purpose is
 * to provide the base class for signal function pointers.
 */
struct Object {
	Object() {}

	// Yeah right... force a VMT so that MSVC++ gets the pointers-to-members
	// right *sigh*
	// OTOH, looking at the gcc assembly, gcc seems to use a less efficient
	// pointer representation. Can anyone clear this up? -- Nicolai
	virtual ~Object() {}

private:
	Object & operator= (Object const &);
	explicit Object    (Object const &);
};
};

#endif
