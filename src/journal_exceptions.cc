/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#include "journal_exceptions.h"

#include "helper.h"

Journalfile_error::Journalfile_error(const std::string _filename) throw ()
: std::runtime_error("Problem with journal file."), filename(_filename)
{
	text="Problem with journal file "+_filename;
}

///\todo Say _which_ magic number was found and which was expected
BadMagic_error::BadMagic_error(const std::string _filename) throw ()
: Journalfile_error(_filename)
{
	text="Journal file "+_filename+" starts with bad magic number";
}

BadRecord_error::BadRecord_error
(const std::string _filename,
 const uint8_t     _code,
 const uint8_t     _expectedcode)
throw ()
: Journalfile_error(_filename), code(_code), expectedcode(_expectedcode)
{
	text="Journal file "+_filename+" contains record with type "+
	     toString(_code)+" instead of the expected type "+
	     toString(_expectedcode);
}

BadEvent_error::BadEvent_error(const std::string _filename,
                               const uint8_t _type) throw()
		: Journalfile_error(_filename), type(_type)
{
	text="Journal file '"+_filename+"' contains record with unknown event type "+
	     toString(_type);
}
