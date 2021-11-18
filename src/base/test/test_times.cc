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
#include "base/times.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")
BOOST_TEST_DONT_PRINT_LOG_VALUE(Time)
BOOST_TEST_DONT_PRINT_LOG_VALUE(Duration)

BOOST_AUTO_TEST_SUITE(times)

BOOST_AUTO_TEST_CASE(times_and_durations) {
	const Time a(1000);
	Time b(10000);
	const Time c;
	const Duration d;

	BOOST_CHECK_EQUAL(b - a, Duration(9000));
	BOOST_CHECK_EQUAL(a + Duration(b.get()), Time(11000));
	BOOST_CHECK_EQUAL(b.is_valid(), true);
	BOOST_CHECK_EQUAL(c.is_valid(), false);
	BOOST_CHECK_EQUAL(d.is_valid(), false);
	BOOST_CHECK_EQUAL((b - a).is_valid(), true);
	BOOST_CHECK_EQUAL(a <= b, true);
	BOOST_CHECK_EQUAL(b > a, true);

	b.increment();
	BOOST_CHECK_EQUAL(b.get(), 10001);
	b.increment(b - a);
	BOOST_CHECK_EQUAL(b.get(), 19002);
	BOOST_CHECK_EQUAL(((b - a) * 10).get(), 180020);
	BOOST_CHECK_EQUAL(((b - a) / 100).get(), 180);
}

BOOST_AUTO_TEST_SUITE_END()
