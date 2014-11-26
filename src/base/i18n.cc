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

#include "base/i18n.h"

#ifdef __FreeBSD__
#  include <clocale>
#endif

#include <cstdlib>
#include <map>
#include <memory>
#include <utility>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/log.h"
#include "config.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"

#ifdef __APPLE__
# if LIBINTL_VERSION >= 0x001201
// for older libintl versions, setlocale is just fine
#  define SETLOCALE libintl_setlocale
# endif // LIBINTL_VERSION
#else // __APPLE__
# if defined _WIN32
#  define SETLOCALE setlocale
# else
#  define SETLOCALE std::setlocale
# endif
#endif

// NOCOM(#codereview): For a lack of a better place: I love the lua scripts in i18n/locales/*.lua, but I do not understand why you are not simply putting everything in one file: i18n/locales.lua: return {
//    ast = { name = "A", ... }, de = { name = ... }
//  }
//  Stating and parsing that many files is potentially slow and one file is easier to understand.

extern int _nl_msg_cat_cntr;

namespace i18n {

/// A stack of textdomains. On entering a new textdomain, the old one gets
/// pushed on the stack. On leaving the domain again it is popped back.
/// \see grab_texdomain()
namespace  {

std::vector<std::pair<std::string, std::string> > textdomains;

std::string env_locale;
std::string locale;
std::string localedir;

}  // namespace

/**
 * Translate a string with gettext
 */
// TODO(unknown): Implement a workaround if gettext was not found
char const * translate(char const * const str) {
	return gettext(str);
}
char const * translate(const std::string & str) {
	return gettext(str.c_str());
}

/**
 * Set the localedir. This should usually only be done once
 */
void set_localedir(std::string dname) {
	localedir = dname;
}

/**
 * Grab a given TextDomain. If a new one is grabbed, it is pushed on the stack.
 * On release, it is dropped and the previous one is re-grabbed instead.
 *
 * So when a tribe loads, it grabs it's textdomain, loads all data and releases
 * it -> we're back in widelands domain. Negative: We can't translate error
 * messages. Who cares?
 */
void grab_textdomain(const std::string & domain)
{
	char const * const dom = domain.c_str();
	char const * const ldir = localedir.c_str();

	bindtextdomain(dom, ldir);
	bind_textdomain_codeset(dom, "UTF-8");
	// log("textdomain %s @ %s\n", dom, ldir);
	textdomain(dom);
	textdomains.push_back(std::make_pair(dom, ldir));
}

/**
 * See grab_textdomain()
 */
void release_textdomain() {
	if (textdomains.empty()) {
		log("ERROR: trying to pop textdomain from empty stack");
		return;
	}
	textdomains.pop_back();

	//don't try to get the previous TD when the very first one ('widelands')
	//just got dropped
	if (!textdomains.empty()) {
		char const * const domain = textdomains.back().first.c_str();

		bind_textdomain_codeset(domain, "UTF-8");
		bindtextdomain(domain, textdomains.back().second.c_str());
		textdomain(domain);
	}
}

/**
 * Initialize locale to English.
 * Save system language (for later selection).
 * Code inspired by wesnoth.org
 */
void init_locale() {
	env_locale = std::string();
#ifdef _WIN32
	locale = "English";
	SETLOCALE(LC_ALL, "English");
#else
	// first, save environment variable
	char * lang;
	lang = getenv("LANG");
	if (lang != nullptr) {
		env_locale = lang;
	}
	if (env_locale.empty()) {
		lang = getenv("LANGUAGE");
		if (lang != nullptr) {
			env_locale = lang;
		}
	}
	locale = "C";
	SETLOCALE(LC_ALL, "C");
	SETLOCALE(LC_MESSAGES, "");
#endif
}

/**
 * Set the locale to the given string.
 * Code inspired by wesnoth.org
 */
void set_locale(std::string name) {
	const std::map<std::string, std::string> kAlternatives = {
		{"ar", "ar,ar_AR,ar_AE,ar_BH,ar_DZ,ar_EG,ar_IN,ar_IQ,ar_JO,ar_KW,ar_LB,ar_LY,ar_MA,ar_OM,ar_"
		"QA,ar_SA,ar_SD,ar_SY,ar_TN,ar_YE"},
		{"ast", "ast,ast_ES"},
		{"ca", "ca,ca_ES,ca_ES@valencia,ca_FR,ca_IT"},
		{"cs", "cs,cs_CZ"},
		{"da", "da,da_DK"},
		{"de", "de,de_DE,de_AT,de_CH,de_LI,de_LU,de_BE"},
		{"el", "el,el_GR,el_CY"},
		{"en", "en,en_US,en_GB,en_AU,en_CA,en_AG,en_BW,en_DK,en_HK,en_IE,en_IN,en_NG,en_NZ,en_PH,en_"
		"SG,en_ZA,en_ZW"},
		{"en_AU", "en_AU,en,en_US,en_GB"},
		{"en_CA", "en_CA,en,en_US,en_GB"},
		{"en_GB", "en_GB,en,en_US"},
		{"eo", "eo,eo_XX"},
		{"es", "es,es_ES,es_MX,es_US"},
		{"et", "et,et_EE"},
		{"eu", "eu,eu_ES,eu_FR"},
		{"fa", "fa,fa_IR"},
		{"fi", "fi,fi_FI"},
		{"fr", "fr,fr_FR,fr_CH,fr_BE,fr_CA,fr_LU"},
		{"gd", "gd,gd_GB,gd_CA"},
		{"gl", "ga,gl_ES"},
		{"he", "he,he_IL"},
		{"hr", "hr,hr_HR,hr_RS,hr_BA,hr_ME,hr_HU"},
		{"hu", "hu,hu_HU"},
		{"ia", "ia"},
		{"id", "id,id_ID"},
		{"it", "it,it_IT,it_CH"},
		{"ja", "ja,ja_JP"},
		{"jv", "jv,jv_ID,jv_MY,jv_SR,jv_NC"},
		{"ka", "ka,ka_GE"},
		{"ko", "ko,ko_KR"},
		{"la", "la,la_AU,la_VA"},
		{"mr", "mr,mr_IN"},
		{"ms", "ms,ms_MY"},
		{"my", "my,my_MM"},
		{"nb", "nb,nb_NO"},
		{"nl", "nl,nl_NL,nl_BE,nl_AW"},
		{"nn", "nn,nn_NO"},
		{"oc", "oc,oc_FR"},
		{"pl", "pl,pl_PL"},
		{"pt", "pt,pt_PT,pt_BR"},
		{"pt_BR", "pt_BR,pt,pt_PT"},
		{"ru", "ru,ru_RU,ru_UA"},
		{"si", "si,si_LK"},
		{"sk", "sk,sk_SK"},
		{"sl", "sl,sl_SI"},
		{"sr", "sr,sr_RS,sr_ME"},
		{"sv", "sv,sv_FI,sv_SE"},
		{"tr", "tr,tr_TR,tr_CY"},
		{"uk", "uk,uk_UA"},
		{"vi", "vi,vi_VN"},
		{"zh", "zh,zh_CN,zh_TW,zh_HK,zh_MO"},
		{"zh_CN", "zh_CN,zh,zh_TW,zh_HK,zh_MO"},
		{"zh_TW", "zh_TW,zh_HK,zh_MO,zh,zh_CN"},
	};

	std::string lang(name);

	log("selected language: %s\n", lang.empty()?"(system language)":lang.c_str());

#ifndef _WIN32
#ifndef __AMIGAOS4__
#ifndef __APPLE__
	unsetenv ("LANGUAGE"); // avoid problems with this variable
#endif
#endif
#endif

	std::string alt_str;
	if (lang.empty()) {
		// reload system language, if selected
		lang = env_locale;
		alt_str = env_locale;
	} else {
		alt_str = lang;
		// otherwise, try alternatives.
		if (kAlternatives.count(lang)) {
			alt_str = kAlternatives.at(lang);
		}
	}
	alt_str += ",";

	// Somehow setlocale doesn't behave same on
	// some systems.
#ifdef __BEOS__
	setenv ("LANG",   lang.c_str(), 1);
	setenv ("LC_ALL", lang.c_str(), 1);
	locale = lang;
#endif
#ifdef __APPLE__
	setenv ("LANGUAGE", lang.c_str(), 1);
	setenv ("LANG",     lang.c_str(), 1);
	setenv ("LC_ALL",   lang.c_str(), 1);
	locale = lang;
#endif
#ifdef _WIN32
	putenv(const_cast<char *>((std::string("LANG=") + lang).c_str()));
	locale = lang;
#endif

#ifdef __linux__
	char * res = nullptr;
	char const * encoding[] = {"", ".utf-8", "@euro", ".UTF-8"};
	std::size_t found = alt_str.find(',', 0);
	bool leave_while = false;
	// try every possible combination of alternative and encoding
	while (found != std::string::npos) {
		std::string base_locale = alt_str.substr(0, int(found));
		alt_str = alt_str.erase(0, int(found) + 1);

		for (int j = 0; j < 4; ++j) {
			std::string try_locale = base_locale + encoding[j];
			res = SETLOCALE(LC_MESSAGES, try_locale.c_str());
			if (res) {
				locale = try_locale;
				log("using locale %s\n", try_locale.c_str());
				leave_while = true;
				break;
			} else {
				//log("locale is not working: %s\n", try_locale.c_str());
			}
		}
		if (leave_while) break;

		found = alt_str.find(',', 0);
	}
	if (leave_while) {
		setenv("LANG",     locale.c_str(), 1);
		setenv("LANGUAGE", locale.c_str(), 1);
	} else {
		log
			("No corresponding locale found - trying to set it via LANGUAGE=%s, LANG=%s\n",
			 lang.c_str(), lang.c_str());
		setenv("LANGUAGE", lang.c_str(), 1);
		setenv("LANG",     lang.c_str(), 1);
		SETLOCALE(LC_MESSAGES, "");    // set locale according to the env. variables
		                               // --> see  $ man 3 setlocale
		// assume that it worked
		// maybe, do another check with the return value (?)
		locale = lang;
	}

	/* Finally make changes known.  */
	++_nl_msg_cat_cntr;
#endif

	SETLOCALE(LC_ALL, ""); //  call to libintl

	if (!textdomains.empty()) {
		char const * const domain = textdomains.back().first.c_str();

		bind_textdomain_codeset(domain, "UTF-8");
		bindtextdomain(domain, textdomains.back().second.c_str());
		textdomain(domain);
	}
	LocaleFonts::get()->set_fontset();
}

const std::string & get_locale() {return locale;}


std::string localize_item_list(const std::vector<std::string>& items, ConcatenateWith listtype) {
	std::string result = "";
	for (std::vector<std::string>::const_iterator it = items.begin(); it != items.end(); ++it) {
		if (it == items.begin()) {
			result = *it;
		}
		else if (it == --items.end()) {
			if (listtype == ConcatenateWith::AMPERSAND) {
				/** TRANSLATORS: Concatenate the last 2 items on a list. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = (boost::format(_("%1$s & %2$s")) % result % (*it)).str();
			} else if (listtype == ConcatenateWith::OR) {
				/** TRANSLATORS: Join the last 2 items on a list with "or". */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = (boost::format(_("%1$s or %2$s")) % result % (*it)).str();
			} else if (listtype == ConcatenateWith::COMMA) {
				/** TRANSLATORS: Join the last 2 items on a list with a comma. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = (boost::format(_("%1$s, %2$s")) % result % (*it)).str();
			} else {
				/** TRANSLATORS: Concatenate the last 2 items on a list. */
				/** TRANSLATORS: RTL languages might want to change the word order here. */
				result = (boost::format(_("%1$s and %2$s")) % result % (*it)).str();
			}
		}
		else {
			/** TRANSLATORS: Concatenate 2 items at in the middle of a list. */
			/** TRANSLATORS: RTL languages might want to change the word order here. */
			result = (boost::format(_("%1$s, %2$s")) % result % (*it)).str();
		}
	}
	return result;
}


// NOCOM(#codereview): this is a preferable design to the static pointer in the class. a) you do not need a pointer and b) the class is actually deleted when the program exits.
LocaleFonts* LocaleFonts::get() {
	static LocaleFonts locale_fonts;
	return &locale_fonts;
}


// NOCOM(#codereview): This class is pretty complex and deserves it's own
// library. That also solves the issue that it drags dependency into base.

// NOCOM(#codereview): This being a singleton is problematic. Singleton is one
// of the most dangerous patterns to use, see
// http://geekswithblogs.net/AngelEyes/archive/2013/09/08/singleton-i-love-you-but-youre-bringing-me-down-re-uploaded.aspx
// and the links in that document.
//
// Where it smells really badly here is that tons of code now depend on this
// singleton being around. One example: The richtext renderer now doesn not
// work anymore without all the Widelands fonts being around.
// I think this class should not be a Singleton, but instead live in WLApplication or the
// FontHandler. Most places that use it right now already use the font handler
// and it can forward the API or expose the fontset. Classes that use FontSet explicitly should get
// it passed in.
//
// After looking some more at the code I find LocaleFonts unnecessary. Give
// FontSet the ability to parse itself, via new FontSet(localename) (remember
// deletion of this class tough) and let FontHandler own the current one and
// update it when needed - you can also introduce a notification
// "LocaleUpdated" and FontHandler updates the FontSet on that. This is quite
// clean and loosely coupled.

// Loads font info from config files, depending on the localename
// NOCOM(#codereview): return a unique_ptr<Fontset>, otherwise ownership is unclear. Who deletes that?
i18n::FontSet* LocaleFonts::parse_font_for_locale(const std::string& localename) {
	std::string fontsetname = "default";
	std::string actual_localename = localename;
	i18n::FontSet* fontset;
	LuaInterface lua;

	// Read default fontset. It defines the fallback fonts and needs to always be there and complete.
	// This way, we will always have fonts, even if we run into an exception further down.
	std::unique_ptr<LuaTable> fonts_table(lua.run_script("i18n/fonts.lua"));
	fonts_table->do_not_warn_about_unaccessed_keys();  // We are only reading partial information as needed

	std::unique_ptr<LuaTable> default_font_table = fonts_table->get_table("default");
	std::string serif = default_font_table->get_string("serif");
	std::string serif_bold = default_font_table->get_string("serif_bold");
	std::string serif_italic = default_font_table->get_string("serif_italic");
	std::string serif_bold_italic = default_font_table->get_string("serif_bold_italic");
	std::string sans = default_font_table->get_string("sans");
	std::string sans_bold = default_font_table->get_string("sans_bold");
	std::string sans_italic = default_font_table->get_string("sans_italic");
	std::string sans_bold_italic = default_font_table->get_string("sans_bold_italic");
	std::string condensed = default_font_table->get_string("condensed");
	std::string condensed_bold = default_font_table->get_string("condensed_bold");
	std::string condensed_italic = default_font_table->get_string("condensed_italic");
	std::string condensed_bold_italic = default_font_table->get_string("condensed_bold_italic");
	std::string direction = default_font_table->get_string("dir");

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

			// NOCOM(#sirver): Add a stand alone function
			// get_string_with_default(LuaTable, key, default) into the lua_table.h
			// file. do not put it into LuaTable though as it is not part of a
			// minimal interface.
			serif = font_set_table->get_string("serif");
			if (font_set_table->has_key("serif_bold")) {
				serif_bold = font_set_table->get_string("serif_bold");
			} else {
				serif_bold = serif;
			}
			if (font_set_table->has_key("serif_italic")) {
				serif_italic = font_set_table->get_string("serif_italic");
			} else {
				serif_italic = serif;
			}
			if (font_set_table->has_key("serif_bold_italic")) {
				serif_bold_italic = font_set_table->get_string("serif_bold_italic");
			} else {
				serif_bold_italic = serif_bold;
			}

			// NOCOM(#codereview): you can pull out more methods here: void set_font_values("sans", &sans_bold, &sans_italic, &sans_bold_italic) and reuse that for all others.
			sans = font_set_table->get_string("sans");
			if (font_set_table->has_key("sans_bold")) {
				sans_bold = font_set_table->get_string("sans_bold");
			} else {
				sans_bold = sans;
			}
			if (font_set_table->has_key("sans_italic")) {
				sans_italic = font_set_table->get_string("sans_italic");
			} else {
				sans_italic = sans;
			}
			if (font_set_table->has_key("sans_bold_italic")) {
				sans_bold_italic = font_set_table->get_string("sans_bold_italic");
			} else {
				sans_bold_italic = sans_bold;
			}

			if (font_set_table->has_key("condensed")) {
				condensed = font_set_table->get_string("condensed");
			} else {
				condensed = sans;
			}
			if (font_set_table->has_key("condensed_bold")) {
				condensed_bold = font_set_table->get_string("condensed_bold");
			} else {
				condensed_bold = condensed;
			}
			if (font_set_table->has_key("condensed_italic")) {
				condensed_italic = font_set_table->get_string("condensed_italic");
			} else {
				condensed_italic = condensed;
			}
			if (font_set_table->has_key("condensed_bold_italic")) {
				condensed_bold_italic = font_set_table->get_string("condensed_bold_italic");
			} else {
				condensed_bold_italic = condensed_bold;
			}

			if (font_set_table->has_key("dir")) {
				direction = font_set_table->get_string("dir");
			} else {
				direction = "ltr";
			}
		} catch (LuaError& err) {
			log("Could not read font set '%s': %s\n", fontsetname.c_str(), err.what());
		}

	} catch (const LuaError& err) {
		log("Could not read locales information from file: %s\n", err.what());
	}


	fontset = new i18n::FontSet(serif, serif_bold, serif_italic, serif_bold_italic,
									  sans, sans_bold, sans_italic, sans_bold_italic,
									  condensed, condensed_bold, condensed_italic, condensed_bold_italic,
									  direction);
	return fontset;
}


}
