/*
 * Copyright (C) 2006, 2008 by the Widelands Development Team
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

#include "config.h"

#include <libintl.h>

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
std::string translate(char const * const str) {
	return gettext(str);
}
std::string translate(std::string const & str) {
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
void set_locale(char const * const name) {
	// Somehow setlocale doesn't behave same on
	// some systems.
#ifdef __BEOS__
	setenv ("LANG",   name, 1);
	setenv ("LC_ALL", name, 1);
#endif
#ifdef __APPLE__
	setenv ("LANGUAGE", name, 1);
	setenv ("LC_ALL",   name, 1);
#endif

#ifdef _WIN32
	putenv((std::string("LANG=") + name).c_str());
#endif

	setlocale(LC_ALL, name); //  call to libintl
	locale = name;

	if (textdomains.size()) {
		char const * const domain = textdomains.back().c_str();
		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain(domain, INSTALL_LOCALEDIR);
		textdomain(domain);
	}
}

std::string const & get_locale() {return locale;}

};
