/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#ifndef WL_BASE_I18N_H
#define WL_BASE_I18N_H

#include <cstring>
#include <string>
#include <vector>

#include "third_party/gettext/gettext.h"  // For ngettext and pgettext.

#include "base/macros.h"
#include "config.h"

/// A macro to make i18n more readable and aid in tagging strings for translation
#define _(str) i18n::translate(str)

namespace i18n {
char const* translate(char const*) __attribute__((format_arg(1)));
char const* translate(const std::string&);

void grab_textdomain(const std::string&);
void release_textdomain();

/// Create an object of this type to grab a textdomain and make sure that it is
/// released when the object goes out of scope. This is exception-safe, unlike
/// calling grab_textdomain and release_textdomain directly.
struct Textdomain {
	explicit Textdomain(const std::string& name) {
		grab_textdomain(name);
	}
	~Textdomain() {
		release_textdomain();
	}
};

void init_locale();
void set_locale(const std::string&);
const std::string& get_locale();

void set_localedir(const std::string&);
const std::string& get_localedir();

enum class ConcatenateWith { AND, OR, AMPERSAND, COMMA };
/**
 * Localize a list of 'items'. The last 2 items are concatenated with "and" or
 * "or" etc, depending on 'concatenate_with'.
 */
std::string localize_list(const std::vector<std::string>& items, ConcatenateWith concatenate_with);

/**
 * Joins 2 sentences together. Use this rather than manually concatenating
 * a blank space, because some languages don't use blank spaces.
 */
std::string join_sentences(const std::string& sentence1, const std::string& sentence2);

}  // namespace i18n

#endif  // end of include guard: WL_BASE_I18N_H
