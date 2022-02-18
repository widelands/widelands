/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "base/scoped_timer.h"

#include <SDL_timer.h>

#include "base/log.h"
#include "base/string.h"

ScopedTimer::ScopedTimer(const std::string& message, bool only_verbose)
   : message_(message), only_verbose_(only_verbose) {
	startime_ = SDL_GetTicks();
	lasttime_ = startime_;
}

ScopedTimer::~ScopedTimer() {
	if ((!only_verbose_ || g_verbose) && !message_.empty()) {
		uint32_t ms_in_existance = SDL_GetTicks() - startime_;
		log_info("%s", format(message_, ms_in_existance).c_str());
	}
}

uint32_t ScopedTimer::ms_since_last_query() {
	uint32_t current_time = SDL_GetTicks();
	assert(current_time >= lasttime_);

	uint32_t delta = current_time - lasttime_;
	lasttime_ = current_time;
	return delta;
}
