/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#ifndef WL_BASE_SCOPED_TIMER_H
#define WL_BASE_SCOPED_TIMER_H

#include <stdint.h>
#include <string>

#include "base/macros.h"

/**
 * A cheap timer class that can be queried for timings and will print out
 * it's total time in existence on destruction.
 */
class ScopedTimer {
public:
	// Takes the output message that will be boost::format() with the total time
	// this object existed (in ms, use %u).
	explicit ScopedTimer(const std::string& message);

	~ScopedTimer();

	// Returns the milliseconds since this object was constructed or since this
	// method was called the last time.
	uint32_t ms_since_last_query();

private:
	std::string message_;
	uint32_t startime_, lasttime_;

	DISALLOW_COPY_AND_ASSIGN(ScopedTimer);
};

#endif  // end of include guard: WL_BASE_SCOPED_TIMER_H
