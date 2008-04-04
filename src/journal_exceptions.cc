/*
 * Copyright (C) 2006, 2008 by the Widelands Development Team
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

Journalfile_error::Journalfile_error(std::string const & _filename) throw ()
: std::runtime_error("Problem with journal file."), filename(_filename)
{
	text="Problem with journal file "+_filename;
}

///\todo Say _which_ magic number was found and which was expected
BadMagic_error::BadMagic_error(std::string const & _filename) throw ()
: Journalfile_error(_filename)
{
	text="Journal file "+_filename+" starts with bad magic number";
}

BadRecord_error::BadRecord_error
	(std::string const &       _filename,
	 uint8_t             const _code,
	 uint8_t             const _expectedcode)
throw ()
: Journalfile_error(_filename), code(_code), expectedcode(_expectedcode)
{
	text  = "Journal file ";
	text += _filename;
	text += " contains record with type ";
	text += toString(static_cast<int>(_code));
	text += " instead of the expected type ";
	text += toString(static_cast<int>(_expectedcode));
}

BadEvent_error::BadEvent_error
	(std::string const & _filename, uint8_t const _type)
throw ()
: Journalfile_error(_filename), type(_type)
{
	text  = "Journal file '";
	text += _filename;
	text += "' contains record with unknown event type ";
	text += toString(_type);
}
