/*
 * Copyright (C) 2006, 2008, 2010 by the Widelands Development Team
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

#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include <libintl.h>  // for ngettext.

#include "base/macros.h"
#include "config.h"

///A macro to make i18n more readable and aid in tagging strings for translation
#define _(str) i18n::translate(str)

namespace i18n {
char const * translate(char        const *) __attribute__ ((format_arg (1)));
char const * translate(const std::string &);

void  grab_textdomain(const std::string &);
void release_textdomain();

/// Create an object of this type to grab a textdomain and make sure that it is
/// released when the object goes out of scope. This is exception-safe, unlike
/// calling grab_textdomain and release_textdomain directly.
struct Textdomain {
	Textdomain (const std::string & name) {grab_textdomain   (name);}
	~Textdomain()                         {release_textdomain();}
};

void init_locale();
void set_locale(std::string);
const std::string & get_locale();

void set_localedir(std::string);


// Localize a list of 'items'. The last 2 items are concatenated with "and" or
// "or", depending on 'concatenate_with'.
enum class ConcatenateWith {AND, OR, AMPERSAND, COMMA};
std::string localize_item_list(const std::vector<std::string>& items, ConcatenateWith concatenate_with);


// Contains font information for a locale
struct FontSet {
public:
	// Writing diection of a script
	enum class Direction: uint8_t {
		kLeftToRight,
		kRightToLeft
	};

	FontSet(const std::string& serif_, const std::string& serif_bold_,
			  const std::string& serif_italic_, const std::string& serif_bold_italic_,
			  const std::string& sans_, const std::string& sans_bold_,
			  const std::string& sans_italic_, const std::string& sans_bold_italic_,
			  const std::string& condensed_, const std::string& condensed_bold_,
			  const std::string& condensed_italic_, const std::string& condensed_bold_italic_,
			  const std::string& direction_) :
		m_serif(serif_),
		m_serif_bold(serif_bold_),
		m_serif_italic(serif_italic_),
		m_serif_bold_italic(serif_bold_italic_),
		m_sans(sans_),
		m_sans_bold(sans_bold_),
		m_sans_italic(sans_italic_),
		m_sans_bold_italic(sans_bold_italic_),
		m_condensed(condensed_),
		m_condensed_bold(condensed_bold_),
		m_condensed_italic(condensed_italic_),
		m_condensed_bold_italic(condensed_bold_italic_) {

		assert(!m_serif.empty());
		assert(!m_serif_bold.empty());
		assert(!m_serif_italic.empty());
		assert(!m_serif_bold_italic.empty());
		assert(!m_sans.empty());
		assert(!m_sans_bold.empty());
		assert(!m_sans_italic.empty());
		assert(!m_sans_bold_italic.empty());
		assert(!m_condensed.empty());
		assert(!m_condensed_bold.empty());
		assert(!m_condensed_italic.empty());
		assert(!m_condensed_bold_italic.empty());

		if (direction_ == "rtl") {
			m_direction = FontSet::Direction::kRightToLeft;
		} else {
			m_direction = FontSet::Direction::kLeftToRight;
		}
	}

	const std::string& serif() const {return m_serif;}
	const std::string& serif_bold() const {return m_serif_bold;}
	const std::string& serif_italic() const {return m_serif_italic;}
	const std::string& serif_bold_italic() const {return m_serif_bold_italic;}
	const std::string& sans() const {return m_sans;}
	const std::string& sans_bold() const {return m_sans_bold;}
	const std::string& sans_italic() const {return m_sans_italic;}
	const std::string& sans_bold_italic() const {return m_sans_bold_italic;}
	const std::string& condensed() const {return m_condensed;}
	const std::string& condensed_bold() const {return m_condensed_bold;}
	const std::string& condensed_italic() const {return m_condensed_italic;}
	const std::string& condensed_bold_italic() const {return m_condensed_bold_italic;}
	FontSet::Direction direction() {return m_direction;}

private:
	std::string m_serif;
	std::string m_serif_bold;
	std::string m_serif_italic;
	std::string m_serif_bold_italic;
	std::string m_sans;
	std::string m_sans_bold;
	std::string m_sans_italic;
	std::string m_sans_bold_italic;
	std::string m_condensed;
	std::string m_condensed_bold;
	std::string m_condensed_italic;
	std::string m_condensed_bold_italic;
	FontSet::Direction m_direction;
};


/// Singleton object that contains the FontSet for the currently active locale.
/// It also has a parse function to get a FontSet for a locale that isn't currently active.
class LocaleFonts {
public:

	/// Returns the one existing instance of this object.
	/// Creates a new instance if none exists yet.
	static LocaleFonts* get();

	/// Parses font information for the given locale from Lua files.
	/// Each locale in i18n/locales/ISO.lua defines which fontset to use.
	/// The fontset definitions are in i18n/fonts
	i18n::FontSet* parse_font_for_locale(const std::string& locale);

	/// Returns the FontSet for the currently active locale
	const i18n::FontSet& get_fontset() const {return *m_fontset;}

	/// Sets the FontSet for the currently active locale.
	void set_fontset() {
		m_fontset = parse_font_for_locale(get_locale());
	}

private:
	 static LocaleFonts* m_singleton;
	 LocaleFonts() {}

	 i18n::FontSet* m_fontset; // The currently active FontSet
};

}

#endif  // end of include guard: WL_BASE_I18N_H
