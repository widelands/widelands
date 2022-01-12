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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_GRAPHIC_TEXT_BIDI_H
#define WL_GRAPHIC_TEXT_BIDI_H

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include "graphic/text/font_set.h"

// BiDi support for RTL languages
namespace i18n {

std::string make_ligatures(const char* input);
std::string line2bidi(const char* input);
std::vector<std::string> split_cjk_word(const char* input);
bool has_rtl_character(const char* input, int32_t limit = std::numeric_limits<int32_t>::max());
bool has_rtl_character(std::vector<std::string> input);
// True if a string contains a character from the script's code blocks
bool has_script_character(const char* input, UI::FontSets::Selector script);
UI::FontSet const* find_fontset(const char* word, const UI::FontSets& fontsets);
std::string icustring2string(const icu::UnicodeString& convertme);
bool cannot_start_line(const UChar& c);
bool cannot_end_line(const UChar& c);
bool is_diacritic(const UChar& c);

}  // namespace i18n

#endif  // end of include guard: WL_GRAPHIC_TEXT_BIDI_H
