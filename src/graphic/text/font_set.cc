/*
 * Copyright (C) 2006, 2008-2010 by the Widelands Development Team
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

#include "graphic/text/font_set.h"

#include <cstdlib>
#include <memory>

#include <boost/algorithm/string.hpp>

#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"

namespace UI {

FontSet::FontSet(const std::string& localename) {
	parse_font_for_locale(localename);
	assert(!serif_.empty());
	assert(!serif_bold_.empty());
	assert(!serif_italic_.empty());
	assert(!serif_bold_italic_.empty());
	assert(!sans_.empty());
	assert(!sans_bold_.empty());
	assert(!sans_italic_.empty());
	assert(!sans_bold_italic_.empty());
	assert(!condensed_.empty());
	assert(!condensed_bold_.empty());
	assert(!condensed_italic_.empty());
	assert(!condensed_bold_italic_.empty());
}

const std::string& FontSet::serif() const {return serif_;}
const std::string& FontSet::serif_bold() const {return serif_bold_;}
const std::string& FontSet::serif_italic() const {return serif_italic_;}
const std::string& FontSet::serif_bold_italic() const {return serif_bold_italic_;}
const std::string& FontSet::sans() const {return sans_;}
const std::string& FontSet::sans_bold() const {return sans_bold_;}
const std::string& FontSet::sans_italic() const {return sans_italic_;}
const std::string& FontSet::sans_bold_italic() const {return sans_bold_italic_;}
const std::string& FontSet::condensed() const {return condensed_;}
const std::string& FontSet::condensed_bold() const {return condensed_bold_;}
const std::string& FontSet::condensed_italic() const {return condensed_italic_;}
const std::string& FontSet::condensed_bold_italic() const {return condensed_bold_italic_;}
const FontSet::Direction& FontSet::direction() const {return direction_;}

// Loads font info from config files, depending on the localename
void FontSet::parse_font_for_locale(const std::string& localename) {
	std::string fontsetname = "default";
	std::string actual_localename = localename;
	std::string direction_string;
	LuaInterface lua;

	// Read default fontset. It defines the fallback fonts and needs to always be there and complete.
	// This way, we will always have fonts, even if we run into an exception further down.
	std::unique_ptr<LuaTable> fonts_table(lua.run_script("i18n/fonts.lua"));
	fonts_table->do_not_warn_about_unaccessed_keys();  // We are only reading partial information as needed

	std::unique_ptr<LuaTable> default_font_table = fonts_table->get_table("default");
	serif_ = default_font_table->get_string("serif");
	serif_bold_ = default_font_table->get_string("serif_bold");
	serif_italic_ = default_font_table->get_string("serif_italic");
	serif_bold_italic_ = default_font_table->get_string("serif_bold_italic");
	sans_ = default_font_table->get_string("sans");
	sans_bold_ = default_font_table->get_string("sans_bold");
	sans_italic_ = default_font_table->get_string("sans_italic");
	sans_bold_italic_ = default_font_table->get_string("sans_bold_italic");
	condensed_ = default_font_table->get_string("condensed");
	condensed_bold_ = default_font_table->get_string("condensed_bold");
	condensed_italic_ = default_font_table->get_string("condensed_italic");
	condensed_bold_italic_ = default_font_table->get_string("condensed_bold_italic");
	direction_string = default_font_table->get_string("dir");

	try  {
		std::unique_ptr<LuaTable> all_locales(lua.run_script(("i18n/locales.lua")));
		all_locales->do_not_warn_about_unaccessed_keys();

		// Locale identifiers can look like this: ca_ES@valencia.UTF-8
		if (localename.empty()) {
			std::vector<std::string> parts;
			boost::split(parts, i18n::get_locale(), boost::is_any_of("."));
			actual_localename = parts[0];

			if (!all_locales->has_key(actual_localename.c_str())) {
				boost::split(parts, parts[0], boost::is_any_of("@"));
				actual_localename = parts[0];

				if (!all_locales->has_key(actual_localename.c_str())) {
					boost::split(parts, parts[0], boost::is_any_of("_"));
					actual_localename = parts[0];
				}
			}
		}

		// Find out which fontset to use from the locale
		if (all_locales->has_key(actual_localename)) {
			try  {
				std::unique_ptr<LuaTable> locale_table = all_locales->get_table(actual_localename);
				locale_table->do_not_warn_about_unaccessed_keys();
				fontsetname = locale_table->get_string("font");
			} catch (const LuaError& err) {
				log("Error loading locale '%s' from file: %s\n", actual_localename.c_str(), err.what());
			}
		}

		// Read the fontset for the current locale.
		// Each locale needs to define a font face for serif and sans.
		// For everything else, there's a fallback font.
		try {
			std::unique_ptr<LuaTable> font_set_table = fonts_table->get_table(fontsetname.c_str());
			font_set_table->do_not_warn_about_unaccessed_keys();

			serif_ = font_set_table->get_string("serif");
			serif_bold_ = get_string_with_default(*font_set_table, "serif_bold", serif_);
			serif_italic_ = get_string_with_default(*font_set_table, "serif_italic", serif_);
			serif_bold_italic_ = get_string_with_default(*font_set_table, "serif_bold_italic", serif_bold_);

			// NOCOM(#codereview): you can pull out more methods here: void set_font_values("sans", &sans_bold, &sans_italic, &sans_bold_italic) and reuse that for all others.
			// NOCOM(#gunchleoc): I don't see how that would make things easier to read
			// - I would need to give the method interface the key names as well as the variable names, and add a special case for condensed.
			// Maybe that wasn't obvious before with all the conditional statements I had.
			sans_ = font_set_table->get_string("sans");
			sans_bold_ = get_string_with_default(*font_set_table, "serif_bold", sans_);
			sans_italic_ = get_string_with_default(*font_set_table, "sans_italic", sans_);
			sans_bold_italic_ = get_string_with_default(*font_set_table, "sans_bold_italic", sans_bold_);

			condensed_ = get_string_with_default(*font_set_table, "condensed", sans_);
			condensed_bold_ = get_string_with_default(*font_set_table, "condensed_bold", condensed_);
			condensed_italic_ = get_string_with_default(*font_set_table, "condensed_italic", condensed_);
			condensed_bold_italic_ = get_string_with_default(*font_set_table, "condensed", condensed_);

			direction_string = get_string_with_default(*font_set_table, "dir", "ltr");

		} catch (LuaError& err) {
			log("Could not read font set '%s': %s\n", fontsetname.c_str(), err.what());
		}

	} catch (const LuaError& err) {
		log("Could not read locales information from file: %s\n", err.what());
	}

	if (direction_string == "rtl") {
		direction_ = FontSet::Direction::kRightToLeft;
	} else {
		direction_ = FontSet::Direction::kLeftToRight;
	}
}

}
