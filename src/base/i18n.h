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

#ifndef WL_BASE_I18N_H
#define WL_BASE_I18N_H

#include <string>
#include <vector>

#include "third_party/gettext/gettext.h"  // For ngettext and pgettext.

// prevent defining sprintf / snprintf / vsnprintf to libintl_sprintf / libintl_snprintf /
// libintl_vsnprintf in libintl.h, libintl.h is included by getext.h
#ifdef sprintf
#undef sprintf
#endif
#ifdef snprintf
#undef snprintf
#endif
#ifdef vsnprintf
#undef vsnprintf
#endif

#include "base/macros.h"
#include "base/multithreading.h"
#include "config.h"

/// Some macros to make i18n more readable and aid in tagging strings for translation
#define _(str) i18n::translate(str)

#undef pgettext
#define pgettext(c, m) i18n::pgettext_wrapper(c, m)
#ifndef BASE_I18N_CC
#undef ngettext
#undef npgettext
#define ngettext(s, p, n) i18n::ngettext_wrapper(s, p, n)
#define npgettext(c, s, p, n) i18n::npgettext_wrapper(c, s, p, n)
#endif

namespace i18n {

// Make every call to translate log the string for debugging.
void enable_verbose_i18n();

char const* translate(char const*) __attribute__((format_arg(1)));
char const* translate(const std::string&);
char const* ngettext_wrapper(const char* singular, const char* plural, int n);
char const* pgettext_wrapper(const char* msgctxt, const char* msgid);
char const* npgettext_wrapper(const char* msgctxt, const char* singular, const char* plural, int n);

void grab_textdomain(const std::string&, const char* ldir);
void release_textdomain();

void init_locale();
void set_locale(const std::string&);
const std::string& get_locale();

void set_localedir(const std::string&);
const std::string& get_localedir();

const std::string& get_addon_locale_dir();

void set_homedir(const std::string&);
const std::string& get_homedir();

/// Create an object of this type to grab a textdomain and make sure that it is
/// released when the object goes out of scope. This is exception-safe, unlike
/// calling grab_textdomain and release_textdomain directly.
struct GenericTextdomain {
public:
	virtual ~GenericTextdomain();

protected:
	explicit GenericTextdomain();

private:
	MutexLock lock_;
};
struct Textdomain : GenericTextdomain {
	// For all common purposes
	explicit Textdomain(const std::string& name);
};
struct AddOnTextdomain : GenericTextdomain {
	// For strings defined in an add-on
	explicit AddOnTextdomain(std::string addon, int i18n_version);
};

enum class ConcatenateWith { AND, OR, AMPERSAND, COMMA };
/**
 * Localize a list of 'items'. The last 2 items are concatenated with "and" or
 * "or" etc, depending on 'concatenate_with'.
 */
std::string localize_list(const std::vector<std::string>& items, ConcatenateWith listtype);

/**
 * Joins 2 sentences together. Use this rather than manually concatenating
 * a blank space, because some languages don't use blank spaces.
 */
std::string join_sentences(const std::string& sentence1, const std::string& sentence2);

}  // namespace i18n

#endif  // end of include guard: WL_BASE_I18N_H
