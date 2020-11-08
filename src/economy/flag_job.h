/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FLAG_JOB_H
#define WL_ECONOMY_FLAG_JOB_H

#include <string>

namespace Widelands {

class Request;

struct FlagJob {
	enum class Type : uint8_t { kGeologist = 0, kScout = 1 };  // stored in savegames
	Type type;
	std::string program;
	Request* request;  // only valid for geologists
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_FLAG_JOB_H
