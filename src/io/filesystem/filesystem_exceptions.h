/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_IO_FILESYSTEM_FILESYSTEM_EXCEPTIONS_H
#define WL_IO_FILESYSTEM_FILESYSTEM_EXCEPTIONS_H

#include <stdexcept>

/**
 * Generic problem when dealing with a file or directory
 */
class FileError : public std::runtime_error {
public:
	explicit FileError(const std::string& thrower,
	                   const std::string& filename,
	                   const std::string& message = "problem with file/directory")

	   : std::runtime_error(thrower + ": " + message + ": " + filename) {
	}
};

/**
 * A file/directory could not be found. Either it really does not exist or there
 * are problems with the path, e.g. loops or nonexistent path components
 */
class FileNotFoundError : public FileError {
public:
	explicit FileNotFoundError(const std::string& thrower,
	                           const std::string& filename,
	                           const std::string& message = "could not find file or directory")

	   : FileError(thrower, filename, message) {
	}
};

/**
 * The file/directory is of an unexpected type. Reasons can be given via message
 */
class FileTypeError : public FileError {
public:
	explicit FileTypeError(const std::string& thrower,
	                       const std::string& filename,
	                       const std::string& message = "file or directory has wrong type")

	   : FileError(thrower, filename, message) {
	}
};

/**
 * The operating system denied access to the file/directory in question
 */
class FileAccessDeniedError : public FileError {
public:
	explicit FileAccessDeniedError(const std::string& thrower,
	                               const std::string& filename,
	                               const std::string& message = "access denied on file or directory")

	   : FileError(thrower, filename, message) {
	}
};

#endif  // end of include guard: WL_IO_FILESYSTEM_FILESYSTEM_EXCEPTIONS_H
