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

#include <boost/test/unit_test.hpp>

#include "base/macros.h"
#include "base/math.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

BOOST_AUTO_TEST_SUITE(math)

BOOST_AUTO_TEST_CASE(math_sign) {
	BOOST_CHECK_EQUAL(math::sign(5), 1);
	BOOST_CHECK_EQUAL(math::sign(-5), -1);
	BOOST_CHECK_EQUAL(math::sign(0), 1);
	BOOST_CHECK_EQUAL(math::sign(0.0001), 1);
	BOOST_CHECK_EQUAL(math::sign(-0.0001), -1);
}

BOOST_AUTO_TEST_CASE(math_clamp) {
	BOOST_CHECK_EQUAL(math::clamp(100, 50, 200), 100);
	BOOST_CHECK_EQUAL(math::clamp(0, 50, 200), 50);
	BOOST_CHECK_EQUAL(math::clamp(1000, 50, 200), 200);
	BOOST_CHECK_EQUAL(math::clamp(0, -200, -50), -50);
	BOOST_CHECK_EQUAL(math::clamp(-1000, -200, -50), -200);
}

BOOST_AUTO_TEST_CASE(math_misc) {
	BOOST_CHECK_EQUAL(math::sqr(9), 81);
	BOOST_CHECK_EQUAL(math::sqr(-8), 64);
	BOOST_CHECK_EQUAL(math::to_int("5"), 5);
	BOOST_CHECK_EQUAL(math::to_int("+0"), 0);
	BOOST_CHECK_EQUAL(math::to_long("-100"), -100);
	BOOST_CHECK_EQUAL(math::read_percent_to_int("9%"), 900);
	BOOST_CHECK_EQUAL(math::read_percent_to_int("12.34%"), 1234);
}

BOOST_AUTO_TEST_SUITE_END()
