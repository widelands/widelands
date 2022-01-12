/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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
#include "base/time_string.h"

TESTSUITE_START(time_string)

TESTCASE(time_string_formatting) {
	check_equal(gametimestring(123456789L, false), "34:17");
	check_equal(gametimestring(123456789L, true), "34:17:36");
	check_equal(gametimestring(90000, false), "0:01");
	check_equal(gametimestring(90000, true), "01:30");
	check_equal(gametimestring(5000, false), "0:00");
	check_equal(gametimestring(5000, true), "00:05");
	check_equal(gametimestring(100, false), "0:00");
	check_equal(gametimestring(100, true), "00:00");

	check_equal(gamestring_with_leading_zeros(123456789L), "034:17:36");
	check_equal(gamestring_with_leading_zeros(2000), "000:00:02");
}

TESTSUITE_END()
