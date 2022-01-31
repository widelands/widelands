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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "base/math.h"
#include "base/test.h"

TESTSUITE_START(math)

TESTCASE(math_sign) {
	check_equal(math::sign(5), 1);
	check_equal(math::sign(-5), -1);
	check_equal(math::sign(0), 1);
	check_equal(math::sign(0.0001), 1);
	check_equal(math::sign(-0.0001), -1);
}

TESTCASE(math_clamp) {
	check_equal(math::clamp(100, 50, 200), 100);
	check_equal(math::clamp(0, 50, 200), 50);
	check_equal(math::clamp(1000, 50, 200), 200);
	check_equal(math::clamp(0, -200, -50), -50);
	check_equal(math::clamp(-1000, -200, -50), -200);
}

TESTCASE(math_misc) {
	check_equal(math::sqr(9), 81);
	check_equal(math::sqr(-8), 64);
	check_equal(math::to_int("5"), 5);
	check_equal(math::to_int("+0"), 0);
	check_equal(math::to_long("-100"), -100);
	check_equal(math::read_percent_to_int("9%"), 900);
	check_equal(math::read_percent_to_int("12.34%"), 1234);
}

TESTSUITE_END()
