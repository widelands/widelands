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
#include "base/utf8.h"

TESTSUITE_START(utf8)

TESTCASE(utf8_unicode) {
	const std::string str = "aäü?";

	check_equal(Utf8::is_utf8_extended(str.at(0)), false);
	check_equal(Utf8::is_utf8_extended(str.at(1)), false);
	check_equal(Utf8::is_utf8_extended(str.at(2)), true);
	check_equal(Utf8::is_utf8_extended(str.at(3)), false);
	check_equal(Utf8::is_utf8_extended(str.at(4)), true);
	check_equal(Utf8::is_utf8_extended(str.at(5)), false);

	size_t pos = 0;
	check_equal(Utf8::utf8_to_unicode(str, pos), 0x61);
	check_equal(pos, 1);
	check_equal(Utf8::utf8_to_unicode(str, pos), 0xe4);
	check_equal(pos, 3);
	check_equal(Utf8::utf8_to_unicode(str, pos), 0xfc);
	check_equal(pos, 5);
	check_equal(Utf8::utf8_to_unicode(str, pos), 0x3f);
	check_equal(pos, 6);
}

TESTSUITE_END()
