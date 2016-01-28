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
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

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
uint16_t FontSet::size_offset() const {return size_offset_;}
bool FontSet::is_rtl() const {return is_rtl_;}


// Loads font info from config files, depending on the localename
void FontSet::parse_font_for_locale(const std::string& localename) {
	std::string fontsetname = "default";
	std::string actual_localename = localename;
	std::string direction_string;
	size_offset_ = 0;
	LuaInterface lua;

	// Read default fontset. It defines the fallback fonts and needs to always be there and complete.
	// This way, we will always have fonts, even if we run into an exception further down.
	std::unique_ptr<LuaTable> fonts_table(lua.run_script("i18n/fonts.lua"));
	fonts_table->do_not_warn_about_unaccessed_keys();  // We are only reading partial information as needed

	std::unique_ptr<LuaTable> default_font_table = fonts_table->get_table("default");

	set_fonts(*default_font_table, kFallbackFont);
	direction_string = default_font_table->get_string("direction");
	size_offset_ = default_font_table->get_int("size_offset");

	// Now try to get the fontset for the actual locale.
	try  {
		std::unique_ptr<LuaTable> all_locales(lua.run_script(("i18n/locales.lua")));
		all_locales->do_not_warn_about_unaccessed_keys();

		// Locale identifiers can look like this: ca_ES@valencia.UTF-8
		if (localename.empty()) {
			std::vector<std::string> parts;
			boost::split(parts, i18n::get_locale(), boost::is_any_of("."));
			actual_localename = parts[0];

			if (!all_locales->has_key(actual_localename)) {
				boost::split(parts, parts[0], boost::is_any_of("@"));
				actual_localename = parts[0];

				if (!all_locales->has_key(actual_localename)) {
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

				// Read the fontset for the current locale.
				try {
					if (!fonts_table->has_key(fontsetname)) {
						log("Font set '%s' for locale '%s' does not exist; using default instead.\n",
							 fontsetname.c_str(), actual_localename.c_str());
						fontsetname = "default";
					}
					std::unique_ptr<LuaTable> font_set_table = fonts_table->get_table(fontsetname);
					font_set_table->do_not_warn_about_unaccessed_keys();

					set_fonts(*font_set_table, serif_);
					direction_string = get_string_with_default(*font_set_table, "direction", "ltr");
					if (font_set_table->has_key("size_offset")) {
						size_offset_ = font_set_table->get_int("size_offset");
					}
				} catch (LuaError& err) {
					log("Could not read font set '%s': %s\n", fontsetname.c_str(), err.what());
				}
			} catch (const LuaError& err) {
				log("Error loading locale '%s' from file: %s\n", actual_localename.c_str(), err.what());
			}
		}
	} catch (const LuaError& err) {
		log("Could not read locales information from file: %s\n", err.what());
	}

	is_rtl_ = false;
	if (direction_string == "rtl") {
		is_rtl_ = true;
	} else if (direction_string != "ltr") {
		log("Unknown script direction '%s'. Using to left-to-right rendering.\n", direction_string.c_str());
	}
}


// The documentation on the fonts fallback scheme is in the 'data/i18n/fonts.lua' data file.
void FontSet::set_fonts(const LuaTable& table, const std::string& fallback) {
	set_font_group(table, "serif", fallback,
						&serif_, &serif_bold_, &serif_italic_, &serif_bold_italic_);

	set_font_group(table, "sans", serif_,
						&sans_, &sans_bold_, &sans_italic_, &sans_bold_italic_);

	set_font_group(table, "condensed", sans_,
						&condensed_, &condensed_bold_, &condensed_italic_, &condensed_bold_italic_);
}

void FontSet::set_font_group(const LuaTable& table, const std::string& key, const std::string& fallback,
									  std::string* basic, std::string* bold,
									  std::string* italic, std::string* bold_italic) {
	*basic = get_string_with_default(table, key, fallback);
	*bold = get_string_with_default(table, (boost::format("%s_bold") % key).str(), *basic);
	*italic = get_string_with_default(table, (boost::format("%s_italic") % key).str(), *basic);
	*bold_italic = get_string_with_default(table, (boost::format("%s_bold_italic") % key).str(), *bold);
}

}
