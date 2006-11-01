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

#ifndef FILESYSTEM_EXCEPTIONS_H
#define FILESYSTEM_EXCEPTIONS_H

#include <stdexcept>

class FileNotFound_error : public std::runtime_error {
public:
	explicit FileNotFound_error(std::string s, std::string _filename) throw();
	virtual ~FileNotFound_error() throw() {}

	virtual const char *what() const throw();

	char *text;
	std::string filename;
};

class FileType_error : public std::runtime_error {
public:
	explicit FileType_error(std::string s, std::string _filename) throw();
	virtual ~FileType_error() throw() {}

	virtual const char *what() const throw();

	char *text;
	std::string filename;
};

#endif
