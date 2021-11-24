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

#ifndef WL_BASE_TEST_H
#define WL_BASE_TEST_H

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "base/string.h"
#include "base/wexception.h"

/**
 * This file provides a unit test framework which can be run automatically after every compilation.
 * Never include this file in anything that is meant to be part of another executable.
 *
 * Each test executable is structured in one or more testsuites, each of which consists or
 * one or more testcases. Each testcase runs one or more checks. A testcase is a static
 * function written by you which takes no arguments and returns void. The function is expected
 * to return normally if all checks go fine, and to throw a WException if the check fails.
 *
 * It is recommended to use the `check_equal` function for this purpose.
 * If it receives two values that are not equal, it logs both their values and throws an exception.
 * Otherwise it returns normally. Only booleans and values `T` for which a `as_string(T)` overload
 * exists can be logged; to be able to pass values that does not support this operator, call
 *
 *     UNLOGGABLE_VALUE(type)
 *
 * to log values of the given type as "<unloggable>".
 *
 * Declaring a testsuite is as easy as calling
 *
 *     TESTSUITE_START(name)
 *     // testcases here
 *     TESTSUITE_END()
 *
 * where `name` is the unique name of the testsuite.
 * A testcase is declared as
 *
 *     TESTCASE(name) {
 *         // your checks here
 *     }
 *
 * where `name` is the unique name of the testcase.
 * Testcases may only be declared within a testsuite, and testsuites may not be nested.
 *
 * In one file per executable, you need to call
 *
 *     TEST_EXECUTABLE(name)
 *
 * where `name` is the unique name of the executable. This macro will generate a main function
 * that calls and examines all testsuites and -cases in the executable.
 *
 * For executables, testsuites, and testcases, `name` must always be a valid C++ identifier.
 *
 * Everything else in this file is an implementation detail, please don't use directly.
 */

namespace WLTestsuite {
using Testcases = std::map<std::string, void (*)()>;
using Testsuite = std::map<std::string, Testcases>;
Testsuite& all_testsuites();

struct InternalTestcaseInserter {
	explicit InternalTestcaseInserter(Testcases& tc, const std::string& name, void (*fn)()) {
		std::cout << "Adding testcase " << name << std::endl;
		tc[name] = fn;
	}
};

template <typename T> inline void log_value(std::ostringstream& oss, T* t) {
	oss << as_string(reinterpret_cast<uint64_t>(t));
}
template <typename T> inline void log_value(std::ostringstream& oss, const T& t) {
	oss << as_string(t);
}
inline void log_value(std::ostringstream& oss, const bool t) {
	oss << (t ? "true" : "false");
}
inline void log_value(std::ostringstream& oss, std::nullptr_t) {
	oss << "<nullptr>";
}
inline void log_value(std::ostringstream& oss, const char* str) {
	oss << str;
}
inline void log_value(std::ostringstream& oss, char* str) {
	oss << str;
}

#define UNLOGGABLE_VALUE(T)                                                                        \
	namespace WLTestsuite {                                                                         \
	template <> inline void log_value<T>(std::ostringstream & oss, const T&) {                      \
		oss << "<unloggable>";                                                                       \
	}                                                                                               \
	}

template <typename T1, typename T2> inline bool compare(const T1 a, const T2 b) {
	return a == b;
}
template <typename T> inline bool compare(const char* a, const T b) {
	return std::string(a) == b;
}
template <typename T> inline bool compare(char* a, const T b) {
	return std::string(a) == b;
}
inline bool compare(const unsigned a, const int b) {
	return static_cast<long>(a) == b;
}
inline bool compare(const unsigned long a, const int b) {
	return static_cast<long long>(a) == b;
}
inline bool compare(const unsigned long long a, const int b) {
	return static_cast<long long>(a) == b;
}
template <typename T> inline bool compare(const float a, const T b) {
	return std::fabs(a - b) < 0.001f;
}
template <typename T> inline bool compare(const double a, const T b) {
	return std::fabs(a - b) < 0.001;
}

#define check_equal(a, b) do_check_equal(__FILE__, __LINE__, a, b)
template <typename T1, typename T2>
inline void do_check_equal(const char* f, uint32_t l, const T1& a, const T2& b) {
	if (!compare(a, b)) {
		std::ostringstream oss;
		oss << "Check failed: (";
		log_value(oss, a);
		oss << ") != (";
		log_value(oss, b);
		oss << ")";
		const std::string str = oss.str();
		throw WException(f, l, "%s", str.c_str());
	}
}
}  // namespace WLTestsuite

#define TEST_EXECUTABLE(name)                                                                      \
	namespace WLTestsuite {                                                                         \
	namespace WLTestsuite_##name {                                                                  \
		static int main() {                                                                          \
			bool errors = false;                                                                      \
			for (const auto& suite : all_testsuites()) {                                              \
				for (const auto& test : suite.second) {                                                \
					try {                                                                               \
						std::cout << "Running " << suite.first << "::" << test.first << std::endl;       \
						test.second();                                                                   \
					} catch (const std::exception& e) {                                                 \
						errors = true;                                                                   \
						std::cout << "Error in " << suite.first << "::" << test.first << ": "            \
						          << e.what() << std::endl;                                              \
					}                                                                                   \
				}                                                                                      \
			}                                                                                         \
			return errors ? 3 : 0;                                                                    \
		}                                                                                            \
	}                                                                                               \
	}                                                                                               \
	int main(int, char**) {                                                                         \
		return WLTestsuite::WLTestsuite_##name::main();                                              \
	}

#define TESTSUITE_START(name)                                                                      \
	namespace WLTestsuite {                                                                         \
	namespace WLTestsuite_##name {                                                                  \
		static Testcases& all_testcases_ = all_testsuites()[#name];

#define TESTCASE(name)                                                                             \
	static void testcase_##name();                                                                  \
	static const InternalTestcaseInserter _internal_testcase_inserter_##name(                       \
	   all_testcases_, #name, &testcase_##name);                                                    \
	static void testcase_##name()

#define TESTSUITE_END()                                                                            \
	}                                                                                               \
	}  // namespace WLTestsuite

#endif  // end of include guard: WL_BASE_TEST_H
