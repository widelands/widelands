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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_MODULE BaseGeometry
#include <boost/test/unit_test.hpp>

#include "base/rect.h"

// NOCOM(#sirver): into the header.
void report(std::vector<FloatRect> H);

// Example from the paper.
// NOCOM(#sirver): bring back.
// BOOST_AUTO_TEST_CASE(test_rectangle_intersection) {
	// const FloatRect r1(107, 103, 175 - 107, 151 - 103);
	// const FloatRect r2(184, 111, 274 - 184, 197 - 111);
	// const FloatRect r3(213, 128, 249 - 213, 176 - 128);

	// report({r1, r2, r3});
	// BOOST_CHECK_EQUAL(0, 1);
// }

BOOST_AUTO_TEST_CASE(test_rectangle_intersection) {
	const FloatRect r1(3, 2, 3, 2);
	const FloatRect r2(2, 1, 7, 7);
	const FloatRect r3(6, 5, 2, 2);
	const FloatRect r4(5.5f, 3.5f, 1, 1);
	const FloatRect r5(2, 9, 1, 1);

	report({r1, r2, r3, r4, r5});
	BOOST_CHECK_EQUAL(0, 1);
}
