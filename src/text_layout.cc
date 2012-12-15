/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#include <boost/format.hpp>

#include "text_layout.h"

using namespace std;
using namespace boost;

string as_uifont
	(const string & txt, int size, const string & face, RGBColor clr)
{
	// UI Text is always bold due to historic reasons
	return (format
		("<rt><p><font face=%s size=%i shadow=1 bold=1 color=%02x%02x%02x>%s</font></p></rt>")
		 % face.c_str() % size % int(clr.r) % int(clr.g) % int(clr.b) % txt.c_str()
		 ).str();
}

