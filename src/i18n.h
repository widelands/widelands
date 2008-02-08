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

#ifndef I18N_H
#define I18N_H

#include <string>
#include <vector>

///A macro to make i18n more readable and aid in tagging strings for translation
#define _(str) i18n::translate(str)

/**
 * Encapsulates i18n-handling
 */
struct i18n {
	static std::string translate      (std::string const &);
	static void        grab_textdomain(std::string const &);
	static void release_textdomain();
	static void        set_locale     (std::string const & = std::string());
	static const std::string get_locale() {return m_locale;}

private:
	///The current locale
	static std::string m_locale;

	///A stack of textdomains. On entering a new textdomain, the old one gets
	///pushed on the stack. On leaving the domain again it is popped back.
	///\see grab_texdomain()
	static std::vector<std::string> m_textdomains;
};

#endif
