/*
 * Copyright (C) 2012-2022 by the Widelands Development Team
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

#ifndef WL_WLAPPLICATION_MESSAGES_H
#define WL_WLAPPLICATION_MESSAGES_H

#include <string>
#include <vector>

struct Parameter {
	std::string title_;
	std::string key_;
	std::string hint_;
	std::string help_;
	bool is_verbose_;
};

enum class CmdLineVerbosity { None, Normal, All };

void fill_parameter_vector();
bool is_parameter(const std::string&);
const std::vector<std::string> get_all_parameters();
void show_usage(const std::string&, const std::string&, CmdLineVerbosity verbosity);

#endif  // end of include guard: WL_WLAPPLICATION_MESSAGES_H
