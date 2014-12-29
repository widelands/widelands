/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <vector>

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE BaseGeometry
#include <boost/test/unit_test.hpp>

#include "base/rect.h"

namespace {

void check_equal_vectors(const std::vector<int>& a, const std::vector<int>& b) {
	BOOST_CHECK_EQUAL(a.size(), b.size());
	for (size_t i = 0; i < a.size(); ++i) {
		BOOST_CHECK_EQUAL(a[i], b[i]);
	}
}

}  // namespace
// NOCOM(#sirver): into the header.
std::vector<std::vector<int>> report(const std::vector<FloatRect>& H);

// Example from the paper.
// BOOST_AUTO_TEST_CASE(test_rectangle_paper_example) {
	// const FloatRect r0(107, 103, 175 - 107, 151 - 103);
	// const FloatRect r1(184, 111, 274 - 184, 197 - 111);
	// const FloatRect r2(213, 128, 249 - 213, 176 - 128);

	// const auto rv = report({r0, r1, r2});
	// check_equal_vectors({0}, rv[0]);
	// check_equal_vectors({1, 2}, rv[1]);
	// check_equal_vectors({2}, rv[2]);
// }

BOOST_AUTO_TEST_CASE(test_rectangle_intersection) {
	const FloatRect r0(3, 0, 3, 4);
	const FloatRect r1(2, 1, 7, 7);
	const FloatRect r2(6, 5, 2, 2);
	const FloatRect r3(5.5f, 3.5f, 1, 1);
	const FloatRect r4(2, 9, 1, 1);

	const auto rv = report({r0, r1, r2, r3, r4});
	check_equal_vectors({0, 1, 3}, rv[0]);
	check_equal_vectors({1, 2, 3}, rv[1]);
	check_equal_vectors({2}, rv[2]);
	check_equal_vectors({3}, rv[3]);
	check_equal_vectors({4}, rv[4]);
}
