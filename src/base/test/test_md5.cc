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

#include <cstring>

#include <boost/test/unit_test.hpp>

#include "base/macros.h"
#include "base/md5.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

BOOST_AUTO_TEST_SUITE(md5)

BOOST_AUTO_TEST_CASE(checksum) {
	const char* const text = "Hello World! This is a string with Ûñīcøđȩ Bÿtèş.";
	SimpleMD5Checksum md5sum;
	md5sum.data(text, strlen(text));
	md5sum.finish_checksum();

	BOOST_CHECK_EQUAL(md5sum.get_checksum().str(), "d4e32b0d5b4fc7b10c7c46fafabf1e17");
}

BOOST_AUTO_TEST_SUITE_END()
