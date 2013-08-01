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

#include "i18n.h"

#include <cstdlib>
#include <utility>

#include <config.h>
#include <libintl.h>

#include "log.h"
#include "profile/profile.h"

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


extern int _nl_msg_cat_cntr;

namespace i18n {

/// A stack of textdomains. On entering a new textdomain, the old one gets
/// pushed on the stack. On leaving the domain again it is popped back.
/// \see grab_texdomain()
std::vector<std::pair<std::string, std::string> > textdomains;

std::string env_locale;
std::string locale;
std::string localedir;

/**
 * Translate a string with gettext
 * \todo Implement a workaround if gettext was not found
 */
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
		alt_str = env_locale + ",";
	} else {
		// otherwise, read alternatives from file
		Profile loc("txts/locales");
		Section * s = &loc.pull_section("alternatives");
		alt_str = s->get_string(lang.c_str(), lang.c_str());
		alt_str += ",";
	}

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

		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain(domain, textdomains.back().second.c_str());
		textdomain(domain);
	}
}

const std::string & get_locale() {return locale;}

}
