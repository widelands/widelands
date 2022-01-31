/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_PROGRAM_RESULT_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_PROGRAM_RESULT_H

#include "base/wexception.h"

namespace Widelands {
// Don't change these values, they are used as hardcoded array indices
enum class ProgramResult { kNone = 0, kFailed = 1, kCompleted = 2, kSkipped = 3 };
inline int program_result_index(ProgramResult result) {
	switch (result) {
	case ProgramResult::kFailed:
		return 0;
	case ProgramResult::kCompleted:
		return 1;
	case ProgramResult::kSkipped:
		return 2;
	case ProgramResult::kNone:
		NEVER_HERE();
	}
	return 0;
}

enum class ProgramResultHandlingMethod { kFail, kComplete, kSkip, kContinue, kRepeat };
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_PROGRAM_RESULT_H
