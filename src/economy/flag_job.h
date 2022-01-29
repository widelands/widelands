/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FLAG_JOB_H
#define WL_ECONOMY_FLAG_JOB_H

#include <string>

namespace Widelands {

class Request;

/**
 * Flags can have flag jobs associated with them. A flag job represents the task
 * that a worker of specified type needs to walk to this flag by roads, and will
 * start a specified worker program upon arriving at the flag.
 *
 * Currently geologists and scouts use this functionality to walk to a flag and
 * then explore their surroundings looking for resources (geologist) or exploring
 * the landscape and scouting the territory of enemies (scout).
 *
 * The flag job can contain additional data such as a Request which the flag
 * needs to maintain its pending jobs. Flag jobs are handled by the owning flag,
 * to which the job needs to be added by calling `Flag::add_flag_job()`.
 *
 * This system can be extended to implement any kind of action that involves a
 * worker walking to a flag and doing something after getting there.
 */
struct FlagJob {
	enum class Type : uint8_t { kGeologist = 0, kScout = 1 };  // stored in savegames
	Type type;
	std::string program;
	Request* request;  // only valid for geologists
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_FLAG_JOB_H
