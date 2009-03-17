/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#ifndef VARIABLE_H
#define VARIABLE_H

#include <cassert>
#include <string>
#include <cstring>
#include <vector>

namespace Widelands {

/**
 * A Map variable is a user defined type, which can be changed with
 * triggers or events and displayed in strings. Also triggers can be
 * run when variables are over a certain value.
 *
 * Variables are mostly for mission objectives - simple stuff (like how many
 * buildings of a type should be build before triggering) doesn't use variables
 */

/**
 * First, the variables
 */
struct Variable {
	Variable(bool const t) {m_delete_protected = t;}
	virtual ~Variable() {}

	bool is_delete_protected() const {return m_delete_protected;}
	void set_name(std::string const & new_name) {m_name = new_name;}
	std::string const & name() const throw () {return m_name;}

	virtual std::string get_string_representation() const = 0;

private:
	std::string m_name;
	bool        m_delete_protected;
};

struct Variable_Int : public Variable {
	Variable_Int(bool const t = 0) : Variable(t) {m_value = 0;}

	int32_t get_value() const {return m_value;}
	void set_value(int32_t const t) {m_value = t;}
	std::string get_string_representation() const {
		char buffer[256];
		sprintf(buffer, "%i", m_value);
		return buffer;
	}

private:
	int32_t m_value;
};

struct Variable_String : public Variable {
	Variable_String(bool const t = 0) : Variable(t) {}

	std::string const & get_value() const {return m_value;}
	void set_value(char const * const t) {m_value = t;}
	std::string get_string_representation() const {return m_value;}

private:
	std::string m_value;
};

};

#endif
