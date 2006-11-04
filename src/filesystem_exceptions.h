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

/**
 * Generic problem when dealing with a file or directory
 */
class File_error : public std::runtime_error {
public:
	explicit File_error(const std::string thrower,
	                    const std::string filename,
	                    const std::string message="problem with file/directory")
	throw();
	virtual ~File_error() throw() {}

	virtual const char *what() const throw();

	std::string m_thrower;
	std::string m_filename;
	std::string m_message;

protected:
	std::string m_what_message;
};

/**
 * A file/directory could not be found. Either it really does not exist or there
 * are problems with the path, e.g. loops or nonexistent path components
 */
class FileNotFound_error : public File_error {
public:
	explicit FileNotFound_error(const std::string thrower,
	                            const std::string filename,
	                            const std::string message="could not find file or directory")
	throw();
};

/**
 * The file/directory is of an unexpected type. Reasons can be given via message
 */
class FileType_error : public File_error {
public:
	explicit FileType_error(const std::string thrower,
	                        const std::string filename,
	                        const std::string message="file or directory has wrong type")
	throw();
};

/**
 * The operating system denied access to the file/directory in question
 */
class FileAccessDenied_error : public File_error {
public:
	explicit FileAccessDenied_error(const std::string thrower,
	                                const std::string filename,
	                                const std::string message="access denied on file or directory")
	throw();
};

/**
 * Generic problem when working \e inside a zipfile.
 * Problems with the zipfile itself should throw File*_error
 */
class ZipFile_error : public File_error {
public:
	explicit ZipFile_error(const std::string thrower,
	                       const std::string filename,
	                       const std::string zipfilename,
	                       const std::string message="problem with file or directory")
	throw();
	virtual ~ZipFile_error() throw() {}

	std::string m_zipfilename;
};

#endif
