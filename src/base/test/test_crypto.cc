/*
 * Copyright (C) 2021-2024 by the Widelands Development Team
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

#include <cstring>

#include "base/crypto.h"
#include "base/test.h"

TESTSUITE_START(md5)

TESTCASE(ASCII) {
	check_equal(crypto::md5_str("Hello World! This is an ASCII-only string."),
	            "7c8e4f06895ac16461bfa37f7f43ebd2");
}

TESTCASE(Unicode) {
	check_equal(crypto::md5_str("Hello World! This is a string with Ûñīcøđȩ Bÿtèş."),
	            "d4e32b0d5b4fc7b10c7c46fafabf1e17");
}

TESTSUITE_END()

TESTSUITE_START(sha1)

TESTCASE(ASCII) {
	check_equal(crypto::sha1("Hello World! This is an ASCII-only string."),
	            "eae56c3734fed035bd30c796ab282a6657f05a34");
}

TESTCASE(Unicode) {
	check_equal(crypto::sha1("Hello World! This is a string with Ûñīcøđȩ Bÿtèş."),
	            "1d8fd65bc996238a6795db335eab8adbce523d28");
}

TESTSUITE_END()
