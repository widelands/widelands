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

Journalfile_error::Journalfile_error(const std::string filename) throw():
		std::runtime_error("Problem with journal file \""+filename+"\"!"),
		filename(filename)
{}

BadMagic_error::BadMagic_error(const std::string filename) throw():
		Journalfile_error("Journal file \""+filename+"\" starts with "
		                  "bad magic number!"),
		filename(filename)
{}

BadRecord_error::BadRecord_error(const std::string filename, const unsigned char code, const unsigned char expectedcode) throw():
		Journalfile_error("Journal file \""+filename+"\" contains "
		                  "record with unknown type "),
		filename(filename), code(code), expectedcode(expectedcode)
{}

BadEvent_error::BadEvent_error(const std::string filename, const unsigned char type) throw():
		filename(filename), type(type),
		Journalfile_error("Journal file \""+filename+"\" contains "
		                  "record with unknown event type ")
{}
