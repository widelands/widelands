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
#include <stdio.h> //TODO: find a replacement for sprintf

Journalfile_error::Journalfile_error(const std::string _filename) throw():
		std::runtime_error("Problem with journal file."),
		filename(_filename)
{
	sprintf(text, "Problem with journal file '%s'.", _filename.c_str());
}

const char *Journalfile_error::what() const throw()
{
	return text;
}

///\todo Say _which_ magic number was found and which was expected
BadMagic_error::BadMagic_error(const std::string _filename) throw():
		Journalfile_error(_filename)
{
	sprintf(text, "Journal file '%s' starts with bad magic number.",
	        _filename.c_str());
}

BadRecord_error::BadRecord_error(const std::string _filename,
                                 const unsigned char _code,
                                 const unsigned char _expectedcode) throw():
		Journalfile_error(_filename),
		code(_code), expectedcode(_expectedcode)
{
	sprintf(text, "Journal file '%s' contains record with type %i instead "
	        "of the expected type %i.",
	        _filename.c_str(), _code, _expectedcode);
}

BadEvent_error::BadEvent_error(const std::string _filename, const unsigned char _type) throw():
		Journalfile_error(_filename),
		type(_type)
{
	sprintf(text, "Journal file '%s' contains record with unknown event "
	        "type %i.",
	        _filename.c_str(), _type);
}
