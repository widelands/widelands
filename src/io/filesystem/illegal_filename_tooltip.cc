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
// this originally comes from Return to the Shadows (http://www.rtts.org/)
// files.cc: provides all the OS abstraction to access files

#include "io/filesystem/illegal_filename_tooltip.h"

#include <cstdlib>
#include <list>
#include <string>

#include "base/i18n.h"
#include "base/log.h"
#include "base/string.h"
#include "graphic/text_layout.h"
#include "io/filesystem/illegal_filename_characters.h"

namespace FileSystemHelper {

std::string illegal_filename_tooltip() {
	std::vector<std::string> starting_characters;
	for (const std::string& character : illegal_filename_starting_characters) {
		if (character == " ") {
			/** TRANSLATORS: Part of tooltip entry for characters in illegal filenames. replaces the
			 * blank space in a list of illegal characters */
			starting_characters.push_back(pgettext("illegal_filename_characters", "blank space"));
		} else {
			starting_characters.push_back(character);
		}
	}
	const std::string illegal_start(as_listitem(
	   /** TRANSLATORS: Tooltip entry for characters in illegal filenames.
	    *  %s is a list of illegal characters */
	   bformat(pgettext("illegal_filename_characters", "%s at the start of the filename"),
	           richtext_escape(i18n::localize_list(starting_characters, i18n::ConcatenateWith::OR))),
	   UI::FontStyle::kWuiMessageParagraph));

	const std::string illegal(as_listitem(
	   /** TRANSLATORS: Tooltip entry for characters in illegal filenames.
	    * %s is a list of illegal characters */
	   bformat(pgettext("illegal_filename_characters", "%s anywhere in the filename"),
	           richtext_escape(
	              i18n::localize_list(illegal_filename_characters, i18n::ConcatenateWith::OR))),
	   UI::FontStyle::kWuiMessageParagraph));

	return bformat(
	   "%s%s%s",
	   /** TRANSLATORS: Tooltip header for characters in illegal filenames.
	    * This is followed by a list of bullet points */
	   pgettext("illegal_filename_characters", "The following characters are not allowed:"),
	   illegal_start, illegal);
}

}  // namespace FileSystemHelper
