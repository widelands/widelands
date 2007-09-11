/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#ifndef HELPER_H
#define HELPER_H

#include "wexception.h"

#include <sstream>
#include <string>
#include <vector>

/**
 * Convert std::string to any sstream-compatible type
 *
 * \see http://www.experts-exchange.com/Programming/Programming_Languages/Cplusplus/Q_20670737.html
 * \author AssafLavie on http://www.experts-exchange.com
 */
template<typename T> T stringTo(const std::string& s) {
	std::istringstream iss(s);
	T x;
	iss >> x;
	return x;
}

/**
 * Convert any sstream-compatible type to std::string
 *
 * \note In a just world, this would be implemented with gnu::autosprintf. But
 * many distributions don't carry that lib despite the fact that it is part of
 * glibc :-(
 *
 * \see http://www.experts-exchange.com/Programming/Programming_Languages/Cplusplus/Q_20670737.html
 * \author AssafLavie on http://www.experts-exchange.com
 */
template<typename T> std::string toString(const T& x) {
	std::ostringstream oss;
	oss << x;
	return oss.str();
}

std::vector<std::string> split_string
(const std::string &, const char * const separators);
void remove_spaces(std::string &);
void log(const char *fmt, ...);

#endif
