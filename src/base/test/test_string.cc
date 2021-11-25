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

#include "base/string.h"
#include "base/test.h"

TESTSUITE_START(strings)

static const std::string str1 = "Hello World";
static const std::string str2 = "hello world";
static const std::string str3 = "helloworld";
static const std::string str4 = "Hello";
static const std::string str5 = "World";
static const std::string str6 = "xyz";

TESTCASE(chars) {
	check_equal(as_string(70), "70");
	check_equal(as_string("xyz"), str6);
	check_equal(as_string('w'), "w");

	check_equal(to_lower(str2), str2);
	check_equal(to_lower(str1), str2);
	check_equal(to_lower(str3) == str2, false);
}

TESTCASE(equality) {
	check_equal(str1 == str2, false);
	check_equal(iequals(str1, str2), true);
	check_equal(str1 == str3, false);
	check_equal(iequals(str1, str3), false);
}

TESTCASE(contain_start_end) {
	check_equal(contains(str1, str4, true), true);
	check_equal(contains(str2, str4, true), false);
	check_equal(contains(str1, str4, false), true);
	check_equal(contains(str2, str4, false), true);
	check_equal(contains(str1, str6, true), false);
	check_equal(contains(str1, str6, false), false);
	check_equal(contains(str6, str1, true), false);
	check_equal(contains(str6, str1, false), false);

	check_equal(starts_with(str1, str4, true), true);
	check_equal(starts_with(str1, str4, false), true);
	check_equal(starts_with(str2, str4, true), false);
	check_equal(starts_with(str2, str4, false), true);
	check_equal(starts_with(str6, str1, true), false);
	check_equal(starts_with(str6, str1, false), false);

	check_equal(ends_with(str1, str5, true), true);
	check_equal(ends_with(str1, str5, false), true);
	check_equal(ends_with(str2, str5, true), false);
	check_equal(ends_with(str2, str5, false), true);
	check_equal(ends_with(str6, str1, true), false);
	check_equal(ends_with(str6, str1, false), false);
}

TESTCASE(join_strings) {
	check_equal(join(std::vector<std::string>{"foo", "bar", "baz"}, " "), "foo bar baz");
	check_equal(join(std::set<std::string>{"foo", "bar", "baz"}, "HelloWorld"),
	            "barHelloWorldbazHelloWorldfoo");
}

TESTCASE(trim_split_replace) {
	const std::string prefix = "          ";
	const std::string middle = "foo bar baz";
	const std::string suffix = "    ";

	std::string str = prefix + middle + suffix;
	trim(str, false, false);
	check_equal(str, prefix + middle + suffix);
	trim(str, true, false);
	check_equal(str, middle + suffix);
	trim(str, false, true);
	check_equal(str, middle);
	trim(str);
	check_equal(str, middle);

	std::vector<std::string> v;
	split(v, str, {' '});
	check_equal(v.size(), 3);
	check_equal(v[0], "foo");
	check_equal(v[1], "bar");
	check_equal(v[2], "baz");
	check_equal(str, middle);

	split(v, str, {'a'});
	check_equal(v.size(), 3);
	check_equal(v[0], "foo b");
	check_equal(v[1], "r b");
	check_equal(v[2], "z");
	check_equal(str, middle);

	replace_first(str, "bar", "word");
	check_equal(str, "foo word baz");
	replace_last(str, "word", "bar");
	check_equal(str, middle);
	replace_all(str, " ba", "/word");
	check_equal(str, "foo/wordr/wordz");
}

TESTSUITE_END()
