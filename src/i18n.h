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
#include <cstring>
#include <vector>

///A macro to make i18n more readable and aid in tagging strings for translation
#define _(str) i18n::translate(str)

namespace i18n {
char const * translate(char        const *);
char const * translate(std::string const &);

void    grab_textdomain(std::string const &);
void release_textdomain();

/// Create an object of this type to grab a textdomain and make sure that it is
/// released when the object goes out of scope. This is exception-safe, unlike
/// calling grab_textdomain and release_textdomain directly.
struct Textdomain {
	Textdomain (std::string const & name) {grab_textdomain   (name);}
	~Textdomain()                         {release_textdomain();}
};

void                set_locale(std::string);
std::string const & get_locale();
};

#endif
