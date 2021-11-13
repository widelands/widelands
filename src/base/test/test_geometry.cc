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
#include "base/rect.h"
#include "base/vector.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")
BOOST_TEST_DONT_PRINT_LOG_VALUE(Vector2i)
BOOST_TEST_DONT_PRINT_LOG_VALUE(Recti)

BOOST_AUTO_TEST_SUITE(geometry)

BOOST_AUTO_TEST_CASE(rect_and_vector) {
	Vector2i v(1, 5);
	const Recti r1(1, 5, 100, 800);
	const Recti r2(v, 20, 3);

	BOOST_CHECK_EQUAL(r1.origin(), v);
	BOOST_CHECK_EQUAL(r2.origin(), v);
	BOOST_CHECK_EQUAL(r2.opposite_of_origin(), Vector2i(21, 8));
	BOOST_CHECK_CLOSE(r1.center().x, 51.f, 0.001f);
	BOOST_CHECK_CLOSE(r1.center().y, 405.f, 0.001f);

	v += Vector2i(3, -8);
	BOOST_CHECK_EQUAL(v.x, 4);
	BOOST_CHECK_EQUAL(v.y, -3);
	BOOST_CHECK_EQUAL(-v, Vector2i(-4, 3));
	v -= Vector2i(4, -3);
	BOOST_CHECK_EQUAL(v, Vector2i::zero());
}

BOOST_AUTO_TEST_SUITE_END()
