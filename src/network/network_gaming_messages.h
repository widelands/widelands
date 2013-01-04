/*
 * Copyright (C) 2012-2013 by the Widelands Development Team
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

#ifndef INTERNET_GAMING_MESSAGES_H
#define INTERNET_GAMING_MESSAGES_H

#include <string>

struct NetworkGamingMessages {
public:
	// Before calling this function the first time, keep in mind to first fill_map()
	static const std::string get_message(const std::string & code);
	static const std::string get_message
		(const std::string & code,
		 const std::string & arg1, const std::string & arg2 = "", const std::string & arg3 = "");

	static void fill_map();
};

#endif
