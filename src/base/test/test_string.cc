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

TESTCASE(string_formatting) {
	check_equal("Hello World", format("%s", "Hello World"));
	check_equal("Hello World", format("%s %s", "Hello", "World"));
	check_equal("Hello World", format("%1$s %2%", "Hello", "World"));
	check_equal("Hello World", format("%2% %1%", "World", "Hello"));
	check_equal("   Hello World", format("%1$14s", "Hello World"));
	check_equal("Hello World   ", format("%-14s", "Hello World"));
	check_equal("         Hello", format("%14.5s", "Hello World"));
	check_equal("Hello         ", format("%1$-14.5s", "Hello World"));

	check_equal("A123X", format("A%dX", 123));
	check_equal("A-1X", format("A%dX", -1));
	check_equal("A+123X", format("A%0+2dX", 123));
	check_equal("A+123    X", format("A%+-8dX", 123));
	check_equal("A+123    X", format("A%-+8dX", 123));
	check_equal("A123     X", format("A%-8dX", 123));
	check_equal("A     123X", format("A%8dX", 123));
	check_equal("A00000123X", format("A%08dX", 123));
	check_equal("A    +123X", format("A%+8dX", 123));
	check_equal("A+0000123X", format("A%+08dX", 123));
	check_equal("A    -123X", format("A%+8dX", -123));
	check_equal("A      +0X", format("A%+8dX", 0));
	check_equal("A0123X", format("A%d%u%d%uX", 0, 1, 2, 3));

	check_equal("AfalsetrueX", format("A%b%bX", 0, 1));
	check_equal("Aw77X", format("A%2$c%1$iX", 77, 'w'));
	check_equal("A^@X", format("A%1$cX", '\0'));
	check_equal("A^MX", format("A%cX", '\r'));
	check_equal("A^[X", format("A%1%X", '\x1b'));

	check_equal("AnullptrX", format("A%PX", nullptr));
	check_equal("A0x123abcX", format("A%pX", reinterpret_cast<int*>(0x123abc)));
	check_equal("A0x123abcX", format("A%1%X", reinterpret_cast<int*>(0x123abc)));
	check_equal("A+0xABC123X", format("A%+PX", 0xabc123));
	check_equal("A-ABC123X", format("A%XX", -0xabc123));
	check_equal("A1194684X", format("A%1%X", 0x123abc));

	check_equal("A123.456X", format("A%.3fX", 123.456));
	check_equal("A-0.45600000X", format("A%2.8fX", -0.456));
	check_equal("A-0.300X", format("A%2.3fX", -0.2999));
	check_equal("A      12.3X", format("A%10.1fX", 12.34567));
	check_equal("A123      X", format("A%-9.0fX", 123.456));
	check_equal("A+123.5   X", format("A%+-9.1fX", 123.456));
	check_equal("A+00123.46X", format("A%0+9.2fX", 123.456));

	format_impl::ArgsPair p1, p2;
	p1.first = p2.first = format_impl::AbstractNode::ArgType::kString;
	p1.second.string_val = "World";
	p2.second.string_val = "Hello";
	check_equal("Hello World", format("%2% %1%", format_impl::ArgsVector{p1, p2}));
	check_equal("World Hello", format("%2% %1%", format_impl::ArgsVector{p2, p1}));

	check_error("invalid placeholder", []() { format("%q", 1); });
	check_error("invalid placeholder", []() { format("%lf", 1); });
	check_error("invalid character after %", []() { format("%|1$d|", 1); });
	check_error("end of string", []() { format("%02.7", 4); });
	check_error("missing placeholder", []() { format("%2% %3$i", 2, 3); });
	check_error("duplicate placeholder", []() { format("%1% %1$d", 1, 1); });
	check_error("mixed placeholders", []() { format("%4d %2%", 123, 123); });
	check_error("too many args", []() { format("%u %li %lld", 1, 2, 3, 4); });
	check_error("too few args", []() { format("%lu %llu %lli", 1, 2); });
	check_error("wrong arg type", []() { format("%s", 1); });
	check_error("wrong arg type", []() { format("%i", "foo"); });
	check_error("float too large", []() { format("%f", 12345678901234567890.f); });
	check_error("int too large", []() { format("%ld", 0x876543210fedcba9); });
	check_error("invalid flag combination", []() { format("%-05d", 123); });
	check_error("repeated flag", []() { format("%+0+5d", 123); });
}

TESTSUITE_END()
