/*
 * Copyright (C) 2006-2007 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef ZIP_EXCEPTIONS_H
#define ZIP_EXCEPTIONS_H

#include <stdexcept>

/**
 * Zip specific problems when working \e inside a zipfile.
 *
 * Problems with the zipfile itself or normal file operations should throw
 * File_error or one of it's descendants with an appropriate message. E.g.:
 * throw FileNotFound_error("ZipFilesystem::Load", fname,
 * "couldn't open file (from zipfile "+m_zipfilename+")");
 */
struct ZipOperation_error : public std::logic_error {
	explicit ZipOperation_error
		(const std::string & thrower,
		 const std::string & filename,
		 const std::string & zipfilename,
		 const std::string & message = "problem during zipfile operation")

		:
		std::logic_error
			(thrower + ": " + message + " (working on '" + filename +
			 "' in zipfile '" + zipfilename + "')"),
		m_thrower(thrower), m_filename(filename), m_zipfilename(zipfilename)
	{}

	virtual ~ZipOperation_error() throw () {}

	std::string m_thrower;
	std::string m_filename;
	std::string m_zipfilename;
};

#endif
