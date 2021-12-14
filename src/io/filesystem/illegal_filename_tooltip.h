/*
 * Copyright (C) 2021 by the Widelands Development Team
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

// This is a helper function for the GUI, that needs other GUI functions.
// We need to separate it from the rest because we don't want the low level
// io/filesystem code to depend on all the GUI code this would pull in.

#ifndef WL_IO_FILESYSTEM_ILLEGAL_FILENAME_TOOLTIP_H
#define WL_IO_FILESYSTEM_ILLEGAL_FILENAME_TOOLTIP_H

#include <string>

namespace FileSystemHelper {
extern std::string illegal_filename_tooltip();
}

#endif  // end of include guard: WL_IO_FILESYSTEM_ILLEGAL_FILENAME_TOOLTIP_H
