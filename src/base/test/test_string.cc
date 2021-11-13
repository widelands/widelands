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
#include "base/string.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

BOOST_AUTO_TEST_SUITE(strings)

static const std::string str1 = "Hello World";
static const std::string str2 = "hello world";
static const std::string str3 = "helloworld";
static const std::string str4 = "Hello";
static const std::string str5 = "World";
static const std::string str6 = "xyz";

BOOST_AUTO_TEST_CASE(chars) {
	BOOST_CHECK_EQUAL(as_string(70), "70");
	BOOST_CHECK_EQUAL(as_string("xyz"), str6);
	/* Boost does not allow comparing the strings directly in this one case for some reason… */
	BOOST_CHECK_EQUAL(strcmp(as_string('w').c_str(), "w"), 0);

	BOOST_CHECK_EQUAL(to_lower(str2), str2);
	BOOST_CHECK_EQUAL(to_lower(str1), str2);
	BOOST_CHECK_EQUAL(to_lower(str3) == str2, false);
}

BOOST_AUTO_TEST_CASE(equality) {
	BOOST_CHECK_EQUAL(str1 == str2, false);
	BOOST_CHECK_EQUAL(iequals(str1, str2), true);
	BOOST_CHECK_EQUAL(str1 == str3, false);
	BOOST_CHECK_EQUAL(iequals(str1, str3), false);
}

BOOST_AUTO_TEST_CASE(contain_start_end) {
	BOOST_CHECK_EQUAL(contains(str1, str4, true), true);
	BOOST_CHECK_EQUAL(contains(str2, str4, true), false);
	BOOST_CHECK_EQUAL(contains(str1, str4, false), true);
	BOOST_CHECK_EQUAL(contains(str2, str4, false), true);
	BOOST_CHECK_EQUAL(contains(str1, str6, true), false);
	BOOST_CHECK_EQUAL(contains(str1, str6, false), false);
	BOOST_CHECK_EQUAL(contains(str6, str1, true), false);
	BOOST_CHECK_EQUAL(contains(str6, str1, false), false);

	BOOST_CHECK_EQUAL(starts_with(str1, str4, true), true);
	BOOST_CHECK_EQUAL(starts_with(str1, str4, false), true);
	BOOST_CHECK_EQUAL(starts_with(str2, str4, true), false);
	BOOST_CHECK_EQUAL(starts_with(str2, str4, false), true);
	BOOST_CHECK_EQUAL(starts_with(str6, str1, true), false);
	BOOST_CHECK_EQUAL(starts_with(str6, str1, false), false);

	BOOST_CHECK_EQUAL(ends_with(str1, str5, true), true);
	BOOST_CHECK_EQUAL(ends_with(str1, str5, false), true);
	BOOST_CHECK_EQUAL(ends_with(str2, str5, true), false);
	BOOST_CHECK_EQUAL(ends_with(str2, str5, false), true);
	BOOST_CHECK_EQUAL(ends_with(str6, str1, true), false);
	BOOST_CHECK_EQUAL(ends_with(str6, str1, false), false);
}

BOOST_AUTO_TEST_CASE(join_strings) {
	BOOST_CHECK_EQUAL(join(std::vector<std::string>{"foo", "bar", "baz"}, " "), "foo bar baz");
	BOOST_CHECK_EQUAL(join(std::set<std::string>{"foo", "bar", "baz"}, "HelloWorld"),
	                  "barHelloWorldbazHelloWorldfoo");
}

BOOST_AUTO_TEST_CASE(trim_split_replace) {
	const std::string prefix = "          ";
	const std::string middle = "foo bar baz";
	const std::string suffix = "    ";

	std::string str = prefix + middle + suffix;
	trim(str, false, false);
	BOOST_CHECK_EQUAL(str, prefix + middle + suffix);
	trim(str, true, false);
	BOOST_CHECK_EQUAL(str, middle + suffix);
	trim(str, false, true);
	BOOST_CHECK_EQUAL(str, middle);
	trim(str);
	BOOST_CHECK_EQUAL(str, middle);

	std::vector<std::string> v;
	split(v, str, {' '});
	BOOST_CHECK_EQUAL(v.size(), 3);
	BOOST_CHECK_EQUAL(v[0], "foo");
	BOOST_CHECK_EQUAL(v[1], "bar");
	BOOST_CHECK_EQUAL(v[2], "baz");
	BOOST_CHECK_EQUAL(str, middle);

	split(v, str, {'a'});
	BOOST_CHECK_EQUAL(v.size(), 3);
	BOOST_CHECK_EQUAL(v[0], "foo b");
	BOOST_CHECK_EQUAL(v[1], "r b");
	BOOST_CHECK_EQUAL(v[2], "z");
	BOOST_CHECK_EQUAL(str, middle);

	replace_first(str, "bar", "word");
	BOOST_CHECK_EQUAL(str, "foo word baz");
	replace_last(str, "word", "bar");
	BOOST_CHECK_EQUAL(str, middle);
	replace_all(str, " ba", "/word");
	BOOST_CHECK_EQUAL(str, "foo/wordr/wordz");
}

BOOST_AUTO_TEST_SUITE_END()
