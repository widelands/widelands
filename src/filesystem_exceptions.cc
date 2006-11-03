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

#include "filesystem_exceptions.h"

File_error::File_error(const std::string thrower, const std::string filename, const std::string message)
throw() : std::runtime_error(""),
		m_thrower(thrower), m_filename(filename), m_message(message)
{}

const char *File_error::what() const throw()
{
	std::string text=m_thrower+": "+m_message+": "+m_filename;

	return text.c_str();
}

FileNotFound_error::FileNotFound_error(const std::string thrower, const std::string filename, const std::string message)
throw() : File_error(thrower, filename, message)
{}

FileType_error::FileType_error(std::string thrower, std::string filename, const std::string message)
throw() : File_error(thrower, filename, message)
{}

FileAccessDenied_error::FileAccessDenied_error(std::string thrower, std::string filename, const std::string message)
throw() : File_error(thrower, filename, message)
{}

ZipFile_error::ZipFile_error(const std::string thrower, const std::string filename, const std::string zipfilename, const std::string message)
throw() : File_error(thrower, filename, message),
		m_zipfilename(zipfilename)
{}

const char *ZipFile_error::what() const throw()
{
	std::string text=m_thrower+": "+m_message+": "+m_filename+
	                 " (in zipfile "+m_zipfilename+")";

	return text.c_str();
}
