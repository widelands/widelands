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

#include <cstring>

#include "base/md5.h"
#include "base/test.h"

TESTSUITE_START(md5)

TESTCASE(checksum) {
	const char* const text = "Hello World! This is a string with Ûñīcøđȩ Bÿtèş.";
	SimpleMD5Checksum md5sum;
	md5sum.data(text, strlen(text));
	md5sum.finish_checksum();

	check_equal(md5sum.get_checksum().str(), "d4e32b0d5b4fc7b10c7c46fafabf1e17");
}

TESTSUITE_END()
