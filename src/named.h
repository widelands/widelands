/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef NAMED_H
#define NAMED_H

#include <string>

struct Named {
	Named(char        const * const Name = "") : m_name(Name) {}
	Named(std::string const &       Name)      : m_name(Name) {}
	void set_name(std::string const & new_name) {m_name = new_name;}
	std::string const & name() const throw () {return m_name;}
private:
	std::string m_name;
};

#endif
