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

#ifndef WL_GRAPHIC_TEXT_FONT_SET_H
#define WL_GRAPHIC_TEXT_FONT_SET_H

#include <string>

namespace UI {

// Contains font information for a locale
struct FontSet {

	// Writing direction of a script
	enum class Direction : int {
		kLeftToRight,
		kRightToLeft
	};

	/// Create the fontset for a locale from configuration file
	FontSet(const std::string& localename);

	const std::string& serif() const;
	const std::string& serif_bold() const;
	const std::string& serif_italic() const;
	const std::string& serif_bold_italic() const;
	const std::string& sans() const;
	const std::string& sans_bold() const;
	const std::string& sans_italic() const;
	const std::string& sans_bold_italic() const;
	const std::string& condensed() const;
	const std::string& condensed_bold() const;
	const std::string& condensed_italic() const;
	const std::string& condensed_bold_italic() const;
	const FontSet::Direction& direction() const;

private:
	/// Parses font information for the given locale from Lua files.
	/// Each locale in i18n/locales.lua defines which fontset to use.
	/// The fontset definitions are in i18n/fonts.lua
	void parse_font_for_locale(const std::string& localename);

	std::string serif_;
	std::string serif_bold_;
	std::string serif_italic_;
	std::string serif_bold_italic_;
	std::string sans_;
	std::string sans_bold_;
	std::string sans_italic_;
	std::string sans_bold_italic_;
	std::string condensed_;
	std::string condensed_bold_;
	std::string condensed_italic_;
	std::string condensed_bold_italic_;
	FontSet::Direction direction_;
};


/// Singleton object that contains the FontSet for the currently active locale.
/// It also has a parse function to get a FontSet for a locale that isn't currently active.
class LocaleFonts {
public:

	/// Returns the one existing instance of this object.
	/// Creates a new instance if none exists yet.
	// NOCOM(#codereview): In other places we use instance(). WLApplication uses get(). We should consolidate (in trunk and merge here). I prefer instance() as it gives a clear indication that we are dealing with a Singleton.
	static LocaleFonts* get();

	/// Returns the FontSet for the currently active locale
	// NOCOM(#codereview): drop the get? fontset().
	const UI::FontSet& get_fontset() const {return *fontset_;}

	/// Sets the FontSet for the currently active locale.
	void set_fontset(const std::string& localename) {
		fontset_ = new FontSet(localename);
	}

private:
	 LocaleFonts() = default;

	 UI::FontSet* fontset_; // The currently active FontSet
};

}

#endif  // end of include guard: WL_GRAPHIC_TEXT_FONT_SET_H
