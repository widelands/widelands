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

#ifndef WL_IO_FILESYSTEM_ILLEGAL_FILENAME_CHARACTERS_H
#define WL_IO_FILESYSTEM_ILLEGAL_FILENAME_CHARACTERS_H

namespace {
// Characters that are allowed in filenames, but not at the beginning
static const std::vector<std::string> illegal_filename_starting_characters{
   ".", "-",
   " ",  // Keep the blank last
};

// Characters that are disallowed anywhere in a filename
// No potential file separators or other potentially illegal characters
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
// http://www.linfo.org/file_name.html
// https://support.apple.com/en-us/HT202808
// We can't just regex for word & digit characters here because of non-Latin scripts.
static const std::vector<std::string> illegal_filename_characters{
   "<", ">", ":", "\"", "|", "?", "*", "/", "\\",
};

}  // namespace

#endif  // end of include guard: WL_IO_FILESYSTEM_ILLEGAL_FILENAME_CHARACTERS_H
