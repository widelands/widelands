/*
 * Copyright (C) 2012-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_NETWORK_NETWORK_GAMING_MESSAGES_H
#define WL_NETWORK_NETWORK_GAMING_MESSAGES_H

#include <string>

struct NetworkGamingMessages {
public:
	static const std::string get_message(const std::string& code);
	static const std::string get_message(const std::string& code,
	                                     const std::string& arg1,
	                                     const std::string& arg2 = "",
	                                     const std::string& arg3 = "");
};

#endif  // end of include guard: WL_NETWORK_NETWORK_GAMING_MESSAGES_H
