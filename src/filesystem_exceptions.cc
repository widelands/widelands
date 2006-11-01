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
#include <stdio.h>

FileNotFound_error::FileNotFound_error(std::string s, std::string _filename) throw()
	: std::runtime_error(s), filename(_filename)
{
	sprintf(text, "Could not find '%s'.", _filename.c_str());
}

const char *FileNotFound_error::what() const throw()
{
	return text;
}

FileType_error::FileType_error(std::string s, std::string _filename) throw()
	: std::runtime_error(s), filename(_filename)
{
	sprintf(text, "Wrong filetype: '%s'.", _filename.c_str());
}

const char *FileType_error::what() const throw()
{
	return text;
}
