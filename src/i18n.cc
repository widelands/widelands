/*
 * Copyright (C) 2006, 2008-2009 by the Widelands Development Team
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

#include "i18n.h"
#include "log.h"

#include <config.h>

#include <libintl.h>

#include <cstdlib>

extern int _nl_msg_cat_cntr;

namespace i18n {

/// A stack of textdomains. On entering a new textdomain, the old one gets
/// pushed on the stack. On leaving the domain again it is popped back.
/// \see grab_texdomain()
std::vector<std::string> textdomains;

std::string locale;

/**
 * Translate a string with gettext
 * \todo Implement a workaround if gettext was not found
 */
char const * translate(char const * const str) {
	return gettext(str);
}
char const * translate(std::string const & str) {
	return gettext(str.c_str());
}

/**
 * Grab a given TextDomain. If a new one is grabbed, it is pushed on the stack.
 * On release, it is dropped and the previous one is re-grabbed instead.
 *
 * So when a tribe loads, it grabs it's textdomain, loads all data and releases
 * it -> we're back in widelands domain. Negative: We can't translate error
 * messages. Who cares?
 */
void grab_textdomain(std::string const & domain) {
	char const * const dom = domain.c_str();

	bind_textdomain_codeset(dom, "UTF-8");
	bindtextdomain(dom, INSTALL_LOCALEDIR);
	textdomain(dom);
	textdomains.push_back(dom);
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
	if (textdomains.size()) {
		char const * const domain = textdomains.back().c_str();

		bind_textdomain_codeset(domain, "UTF-8");
		bindtextdomain(domain, INSTALL_LOCALEDIR);
		textdomain(domain);
	}
}

/**
 * Set the locale to the given string
 */
void set_locale(std::string name) {
	std::string lang(name);

	// Somehow setlocale doesn't behave same on
	// some systems.
#ifdef __BEOS__
	setenv ("LANG",   lang.c_str(), 1);
	setenv ("LC_ALL", lang.c_str(), 1);
#endif
#ifdef __APPLE__
	setenv ("LANGUAGE", lang.c_str(), 1);
	setenv ("LC_ALL",   lang.c_str(), 1);
#endif

#ifdef _WIN32
	putenv(const_cast<char *>((std::string("LANG=") + lang).c_str()));
#endif

#ifdef linux
	// hopefully this gets gettext to run on all linux distributions - some like
	// ubuntu are very problematic with setting language variables.
	// If this doesn't solve the problem on your linux-distribution, here comes
	// a quote from
	// http://www.gnu.org/software/automake/manual/
	//  gettext/Locale-Environment-Variables.html
	//
	//   Some systems, unfortunately, set LC_ALL in /etc/profile or in similar
	//   initialization files. As a user, you therefore have to unset this
	//   variable if you want to set LANG and optionally some of the other LC_xxx
	//   variables.

	/* If lang is empty, fill it with $LANG */
	if (lang.size() < 1)
		if (char const * const l = getenv("LANG"))
			lang = l;
	/* Than set the variables */
	setenv ("LANG",     lang.c_str(), 1);
	setenv ("LANGUAGE", (lang + ":" + lang.substr(0, 2)).c_str(), 1);
	/* Finally make changes known.  */
	++_nl_msg_cat_cntr;
#endif
	setlocale(LC_ALL, ""); //  call to libintl
	locale = lang.c_str();

	if (textdomains.size()) {
		char const * const domain = textdomains.back().c_str();
		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain(domain, INSTALL_LOCALEDIR);
		textdomain(domain);
	}
}

std::string const & get_locale() {return locale;}

};
