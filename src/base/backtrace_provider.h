/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#ifndef WL_BASE_BACKTRACE_PROVIDER_H
#define WL_BASE_BACKTRACE_PROVIDER_H

#include <string>

class BacktraceProvider {
public:
	class DefaultAbortGuard {
	public:
		~DefaultAbortGuard();
	};

	static void register_signal_handler();
	static std::string get_signal_description(int sig);
};

#endif  // end of include guard: WL_BASE_BACKTRACE_PROVIDER_H
