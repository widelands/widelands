/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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

// Unittests related to Genetic Algorithm

#include <boost/test/unit_test.hpp>

#include "ai/ai_help_structs.h"
#include "base/macros.h"

// Triggered by BOOST_AUTO_TEST_CASE
CLANG_DIAG_OFF("-Wdisabled-macro-expansion")
CLANG_DIAG_OFF("-Wused-but-marked-unused")

using namespace Widelands;

BOOST_AUTO_TEST_SUITE(ai_ga)

// Neuron presents a curve, of integers in range (0-20) (21 values)
BOOST_AUTO_TEST_CASE(neuron) {
	Neuron n1 = Neuron(-50, 0, 0);
	BOOST_CHECK_EQUAL(n1.get_id(), 0);
	BOOST_CHECK_EQUAL(n1.get_weight(), -50);
	BOOST_CHECK_EQUAL(n1.get_result(10), -25);
	BOOST_CHECK_EQUAL(n1.get_result(20), -50);
	BOOST_CHECK_EQUAL(n1.get_result_safe(100), -50);
}

BOOST_AUTO_TEST_CASE(neuron_updated_weight) {
	Neuron n1 = Neuron(-50, 0, 0);
	n1.set_weight(50);
	n1.recalculate();
	BOOST_CHECK_EQUAL(n1.get_id(), 0);
	BOOST_CHECK_EQUAL(n1.get_weight(), 50);
	BOOST_CHECK_EQUAL(n1.get_result(10), 25);
	BOOST_CHECK_EQUAL(n1.get_result_safe(100), 50);
}

// FNeuron is uint32_t that serves as 32 bools, that can be set and get independently
BOOST_AUTO_TEST_CASE(fneuron_position) {
	FNeuron fn = FNeuron(0, 0);
	BOOST_CHECK_EQUAL(fn.get_int(), 0);  // Initialized as 0, so must be still 0
	const bool val0 = fn.get_position(0);
	const bool val1 = fn.get_position(1);
	BOOST_CHECK_EQUAL(fn.get_position(0), val0);
	fn.flip_bit(0);
	BOOST_CHECK_EQUAL(!fn.get_position(0), val0);
	BOOST_CHECK_EQUAL(fn.get_position(1), val1);  // should not be changed
	BOOST_CHECK(fn.get_int() != 0);               // Initialized as 0, so now must be different
	fn.flip_bit(0);                               // reverting back
	BOOST_CHECK_EQUAL(fn.get_int(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
