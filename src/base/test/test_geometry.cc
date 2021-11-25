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

#include <cstdlib>

#include "base/rect.h"
#include "base/test.h"
#include "base/vector.h"

UNLOGGABLE_VALUE(Vector2i)
UNLOGGABLE_VALUE(Recti)

TESTSUITE_START(geometry)

TESTCASE(rect_and_vector) {
	Vector2i v(1, 5);
	const Recti r1(1, 5, 100, 800);
	const Recti r2(v, 20, 3);

	check_equal(r1.origin(), v);
	check_equal(r2.origin(), v);
	check_equal(r2.opposite_of_origin(), Vector2i(21, 8));
	check_equal(r1.center().x, 51.f);
	check_equal(r1.center().y, 405.f);

	v += Vector2i(3, -8);
	check_equal(v.x, 4);
	check_equal(v.y, -3);
	check_equal(-v, Vector2i(-4, 3));
	v -= Vector2i(4, -3);
	check_equal(v, Vector2i::zero());
}

TESTSUITE_END()
