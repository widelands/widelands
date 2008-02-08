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

#include "config.h"
#include "i18n.h"
#include <libintl.h>


std::string i18n::m_locale="";
std::vector<std::string> i18n::m_textdomains=std::vector<std::string>();

/**
 * Translate a string with gettext
 * \todo Implement a workaround if gettext was not found
 */
std::string i18n::translate(std::string const & str) {
	return std::string(gettext(str.c_str()));
}

/**
 * Grab a given TextDomain. If a new one is grabbed, it is pushed on the stack.
 * On release, it is dropped and the previous one is re-grabbed instead.
 *
 * So when a tribe loads, it grabs it's textdomain, loads all data and releases
 * it -> we're back in widelands domain. Negative: We can't translate error
 * messages. Who cares?
 */
void i18n::grab_textdomain(std::string const & domain) {
	char const * const dom = domain.c_str();

	bind_textdomain_codeset(dom, "UTF-8");
	bindtextdomain(dom, INSTALL_LOCALEDIR);
	textdomain(dom);
	m_textdomains.push_back(dom);
}

/**
 * See grab_textdomain()
 */
void i18n::release_textdomain()
{
	m_textdomains.pop_back();

	//don't try to get the previous TD when the very first one ('widelands')
	//just got dropped
	if (!m_textdomains.empty()) {
		const char * const domain=m_textdomains.back().c_str();

		bind_textdomain_codeset(domain, "UTF-8");
		bindtextdomain(domain, INSTALL_LOCALEDIR);
		textdomain(domain);
	}
}

/**
 * Set the locale to the given string
 */
void i18n::set_locale(std::string const & str) {
	// Somehow setlocale doesn't behave same on
	// some systems.
#ifdef __BEOS__
	setenv ("LANG", str.c_str(), 1);
	setenv ("LC_ALL", str.c_str(), 1);
#endif
#ifdef __APPLE__
	setenv ("LANGUAGE", str.c_str(), 1);
	setenv ("LC_ALL", str.c_str(), 1);
#endif

#ifdef _WIN32
	const std::string env = std::string("LANG=") + str;
	putenv(env.c_str());
#endif

	setlocale(LC_ALL, str.c_str()); //call to libintl
	m_locale=str;

	if (m_textdomains.size()) {
		const char * const domain = m_textdomains.back().c_str();
		bind_textdomain_codeset (domain, "UTF-8");
		bindtextdomain(domain, INSTALL_LOCALEDIR);
		textdomain(domain);
	}
}
