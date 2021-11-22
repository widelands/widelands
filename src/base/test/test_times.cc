/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#include "base/test.h"
#include "base/times.h"

UNLOGGABLE_VALUE(Time)
UNLOGGABLE_VALUE(Duration)

TESTSUITE_START(times)

TESTCASE(times_and_durations) {
	const Time a(1000);
	Time b(10000);
	const Time c;
	const Duration d;

	check_equal(b - a, Duration(9000));
	check_equal(a + Duration(b.get()), Time(11000));
	check_equal(b.is_valid(), true);
	check_equal(c.is_valid(), false);
	check_equal(d.is_valid(), false);
	check_equal((b - a).is_valid(), true);
	check_equal(a <= b, true);
	check_equal(b > a, true);

	b.increment();
	check_equal(b.get(), 10001);
	b.increment(b - a);
	check_equal(b.get(), 19002);
	check_equal(((b - a) * 10).get(), 180020);
	check_equal(((b - a) / 100).get(), 180);
}

TESTSUITE_END()
