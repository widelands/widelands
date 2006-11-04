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

#include "zip_exceptions.h"

ZipOperation_error::ZipOperation_error(const std::string thrower,
                                       const std::string filename,
                                       const std::string zipfilename,
                                       const std::string message) throw()
		: std::logic_error(""),
		m_thrower(thrower), m_filename(filename), m_zipfilename(zipfilename),
		m_message(message)
{
	m_what_message=m_thrower+": "+m_message+" (working on '"+m_filename+
	               "' in zipfile '"+m_zipfilename+"')";
}

const char *ZipOperation_error::what() const throw()
{
	return m_what_message.c_str();
}
