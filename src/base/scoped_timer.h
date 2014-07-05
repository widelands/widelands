/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#ifndef SCOPED_TIMER_H
#define SCOPED_TIMER_H

#include <string>
#include <stdint.h>

#include <boost/noncopyable.hpp>

/**
 * A cheap timer class that can be queried for timings and will print out
 * it's total time in existence on destruction.
 */
class ScopedTimer : boost::noncopyable {
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
};

#endif /* end of include guard: SCOPED_TIMER_H */
