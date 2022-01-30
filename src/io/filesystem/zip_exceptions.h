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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_IO_FILESYSTEM_ZIP_EXCEPTIONS_H
#define WL_IO_FILESYSTEM_ZIP_EXCEPTIONS_H

#include <stdexcept>

/**
 * Zip specific problems when working \e inside a zipfile.
 *
 * Problems with the zipfile itself or normal file operations should throw
 * FileError or one of it's descendants with an appropriate message. E.g.:
 * throw FileNotFoundError("ZipFilesystem::load", fname,
 * "couldn't open file (from zipfile "+zipfilename_+")");
 */
struct ZipOperationError : public std::logic_error {
	ZipOperationError(const std::string& thrower,
	                  const std::string& filename,
	                  const std::string& zipfilename,
	                  const std::string& message = "problem during zipfile operation")
	   : std::logic_error(thrower + ": " + message + " (working on '" + filename + "' in zipfile '" +
	                      zipfilename + "')"),
	     thrower_(thrower),
	     filename_(filename),
	     zipfilename_(zipfilename) {
	}

	std::string thrower_;
	std::string filename_;
	std::string zipfilename_;
};

#endif  // end of include guard: WL_IO_FILESYSTEM_ZIP_EXCEPTIONS_H
