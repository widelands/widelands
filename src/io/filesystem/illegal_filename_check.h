/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

// illegal_filename_tooltip() is a helper function for the GUI, that needs
// other GUI functions. We need to separate it from the rest because we don't
// want the low level io/filesystem code to depend on all the GUI code it
// would pull in.
// is_legal_filename() does not have such heavy dependencies, but it is only
// used by the same GUI dialogs. It is included here because the list of
// illegal characters is shared by these 2 functions, and not used elsewhere.

#ifndef WL_IO_FILESYSTEM_ILLEGAL_FILENAME_CHECK_H
#define WL_IO_FILESYSTEM_ILLEGAL_FILENAME_CHECK_H

#include <string>

namespace FileSystemHelper {

// Returns true if the filename is legal in all operating systems
bool is_legal_filename(const std::string& filename);

// Generates a richtext helptext from the (constant) list of characters that
// are not allowed in filenames
std::string illegal_filename_tooltip();

}  // namespace FileSystemHelper

#endif  // end of include guard: WL_IO_FILESYSTEM_ILLEGAL_FILENAME_CHECK_H
