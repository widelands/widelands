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
#include "base/utf8.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

BOOST_AUTO_TEST_SUITE(utf8)

BOOST_AUTO_TEST_CASE(utf8_unicode) {
	const std::string str = "aäü?";

	BOOST_CHECK_EQUAL(Utf8::is_utf8_extended(str.at(0)), false);
	BOOST_CHECK_EQUAL(Utf8::is_utf8_extended(str.at(1)), false);
	BOOST_CHECK_EQUAL(Utf8::is_utf8_extended(str.at(2)), true);
	BOOST_CHECK_EQUAL(Utf8::is_utf8_extended(str.at(3)), false);
	BOOST_CHECK_EQUAL(Utf8::is_utf8_extended(str.at(4)), true);
	BOOST_CHECK_EQUAL(Utf8::is_utf8_extended(str.at(5)), false);

	size_t pos = 0;
	BOOST_CHECK_EQUAL(Utf8::utf8_to_unicode(str, pos), 0x61);
	BOOST_CHECK_EQUAL(pos, 1);
	BOOST_CHECK_EQUAL(Utf8::utf8_to_unicode(str, pos), 0xe4);
	BOOST_CHECK_EQUAL(pos, 3);
	BOOST_CHECK_EQUAL(Utf8::utf8_to_unicode(str, pos), 0xfc);
	BOOST_CHECK_EQUAL(pos, 5);
	BOOST_CHECK_EQUAL(Utf8::utf8_to_unicode(str, pos), 0x3f);
	BOOST_CHECK_EQUAL(pos, 6);
}

BOOST_AUTO_TEST_SUITE_END()
