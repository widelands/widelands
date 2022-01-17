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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_GRAPHIC_TEXT_FONT_SET_H
#define WL_GRAPHIC_TEXT_FONT_SET_H

#include <map>
#include <memory>

#include "base/macros.h"
#include "scripting/lua_table.h"

namespace UI {

// Contains font information for a locale
struct FontSet {

	static constexpr const char* kFallbackFont = "DejaVu/DejaVuSans.ttf";

	/// Create a fontset from i18n/fonts.lua
	explicit FontSet(const std::string& fontset_name);

	// The fontset's name
	const std::string& name() const;

	/// All functions below return the path of the font file used for the given
	/// style.
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
	const std::string& representative_character() const;
	// Some scripts need more vertical space than the default font, e.g. Arabic
	uint16_t size_offset() const;
	// Returns true iff the fontset's script is written from right to left.
	bool is_rtl() const;

private:
	/// Parses font information for the given fontset name from Lua.
	/// The fontset definitions are in i18n/fonts.lua
	void parse_fontset(const std::string& fontset_name);

	/// Reads and sets the fonts from 'table', using 'fallback' as the fallback font file.
	void set_fonts(const LuaTable& table, const std::string& fallback);

	/// Helper function for set_fonts. key is "serif", "sans" or "condensed".
	void set_font_group(const LuaTable& table,
	                    const std::string& key,
	                    const std::string& fallback,
	                    std::string* basic,
	                    std::string* bold,
	                    std::string* italic,
	                    std::string* bold_italic);

	std::string name_;
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
	std::string representative_character_;
	uint16_t size_offset_;
	bool is_rtl_;
	DISALLOW_COPY_AND_ASSIGN(FontSet);
};

/// A repository of all available fontsets
struct FontSets {
	enum class Selector { kDefault, kArabic, kCJK, kDevanagari, kHebrew, kUnknown };

	FontSets();

	/// Get the fontset corresponding to the given selector.
	const FontSet* get_fontset(UI::FontSets::Selector selector) const;
	/// Get the fontset used by the given locale ISO code.
	const FontSet* get_fontset(const std::string& locale) const;

private:
	// Maps locale ISO codes to fontset selectors.
	std::map<std::string, UI::FontSets::Selector> locale_fontsets;
	// Contains all available fontsets, to be accessed by their selector.
	std::map<UI::FontSets::Selector, std::unique_ptr<UI::FontSet>> fontsets;

	DISALLOW_COPY_AND_ASSIGN(FontSets);
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_TEXT_FONT_SET_H
