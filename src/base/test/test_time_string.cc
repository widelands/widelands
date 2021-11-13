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
#include "base/time_string.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

BOOST_AUTO_TEST_SUITE(time_string)

BOOST_AUTO_TEST_CASE(time_string_formatting) {
	BOOST_CHECK_EQUAL(gametimestring(123456789L, false), "34:17");
	BOOST_CHECK_EQUAL(gametimestring(123456789L, true), "34:17:36");
	BOOST_CHECK_EQUAL(gametimestring(90000, false), "0:01");
	BOOST_CHECK_EQUAL(gametimestring(90000, true), "01:30");
	BOOST_CHECK_EQUAL(gametimestring(5000, false), "0:00");
	BOOST_CHECK_EQUAL(gametimestring(5000, true), "00:05");
	BOOST_CHECK_EQUAL(gametimestring(100, false), "0:00");
	BOOST_CHECK_EQUAL(gametimestring(100, true), "00:00");

	BOOST_CHECK_EQUAL(strcmp(gamestring_with_leading_zeros(123456789L), "034:17:36"), 0);
	BOOST_CHECK_EQUAL(strcmp(gamestring_with_leading_zeros(2000), "000:00:02"), 0);
}

BOOST_AUTO_TEST_SUITE_END()
