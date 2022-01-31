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

#include "io/filesystem/illegal_filename_check.h"

#include <cstdlib>
#include <list>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/string.h"
#include "graphic/text_layout.h"

namespace FileSystemHelper {

// Characters that are allowed in filenames, but not at the beginning
const std::vector<std::string> illegal_filename_starting_characters{
   ".", "-",
   " ",  // Keep the blank last
};

// Characters that are disallowed anywhere in a filename
// No potential file separators or other potentially illegal characters
// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
// http://www.linfo.org/file_name.html
// https://support.apple.com/en-us/HT202808
// We can't just regex for word & digit characters here because of non-Latin scripts.
const std::vector<std::string> illegal_filename_characters{
   "<", ">", ":", "\"", "|", "?", "*", "/", "\\",
};

bool is_legal_filename(const std::string& filename) {
	if (filename.empty()) {
		return false;
	}
	for (const std::string& illegal_start : illegal_filename_starting_characters) {
		if (starts_with(filename, illegal_start)) {
			return false;
		}
	}
	for (const std::string& illegal_char : illegal_filename_characters) {
		if (contains(filename, illegal_char)) {
			return false;
		}
	}
	return true;
}

std::string illegal_filename_tooltip() {
	std::vector<std::string> starting_characters;
	for (const std::string& character : illegal_filename_starting_characters) {
		if (character == " ") {
			/** TRANSLATORS: Part of tooltip entry for characters in illegal filenames. replaces the
			 * blank space in a list of illegal characters */
			starting_characters.emplace_back(pgettext("illegal_filename_characters", "blank space"));
		} else {
			starting_characters.push_back(character);
		}
	}
	const std::string illegal_start(as_listitem(
	   /** TRANSLATORS: Tooltip entry for characters in illegal filenames.
	    *  %s is a list of illegal characters */
	   format(pgettext("illegal_filename_characters", "%s at the start of the filename"),
	          richtext_escape(i18n::localize_list(starting_characters, i18n::ConcatenateWith::OR))),
	   UI::FontStyle::kWuiMessageParagraph));

	const std::string illegal(as_listitem(
	   /** TRANSLATORS: Tooltip entry for characters in illegal filenames.
	    * %s is a list of illegal characters */
	   format(pgettext("illegal_filename_characters", "%s anywhere in the filename"),
	          richtext_escape(
	             i18n::localize_list(illegal_filename_characters, i18n::ConcatenateWith::OR))),
	   UI::FontStyle::kWuiMessageParagraph));

	return format(
	   "%s%s%s",
	   /** TRANSLATORS: Tooltip header for characters in illegal filenames.
	    * This is followed by a list of bullet points */
	   pgettext("illegal_filename_characters", "The following characters are not allowed:"),
	   illegal_start, illegal);
}

}  // namespace FileSystemHelper
