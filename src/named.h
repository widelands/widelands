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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef NAMED_H
#define NAMED_H

#include <cstring>
#include <string>

struct Named {
	Named(char        const * const Name = "") : m_name(Name) {}
	Named(const std::string &       Name)      : m_name(Name) {}
	void set_name(const std::string & new_name) {m_name = new_name;}
	const std::string & name() const {return m_name;}
private:
	std::string m_name;
};

#endif
