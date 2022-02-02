/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

// Unittests related to Genetic Algorithm

#include "ai/ai_help_structs.h"
#include "base/test.h"

TESTSUITE_START(ai_ga)

// AI::Neuron presents a curve, of integers in range (0-20) (21 values)
TESTCASE(neuron) {
	AI::Neuron n1 = AI::Neuron(-50, 0, 0);
	check_equal(n1.get_id(), 0);
	check_equal(n1.get_weight(), -50);
	check_equal(n1.get_result(10), -25);
	check_equal(n1.get_result(20), -50);
	check_equal(n1.get_result_safe(100), -50);
}

TESTCASE(neuron_updated_weight) {
	AI::Neuron n1 = AI::Neuron(-50, 0, 0);
	n1.set_weight(50);
	n1.recalculate();
	check_equal(n1.get_id(), 0);
	check_equal(n1.get_weight(), 50);
	check_equal(n1.get_result(10), 25);
	check_equal(n1.get_result_safe(100), 50);
}

// AI::FNeuron is uint32_t that serves as 32 bools, that can be set and get independently
TESTCASE(fneuron_position) {
	AI::FNeuron fn = AI::FNeuron(0, 0);
	check_equal(fn.get_int(), 0u);  // Initialized as 0, so must be still 0
	const bool val0 = fn.get_position(0);
	const bool val1 = fn.get_position(1);
	check_equal(fn.get_position(0), val0);
	fn.flip_bit(0);
	check_equal(!fn.get_position(0), val0);
	check_equal(fn.get_position(1), val1);  // should not be changed
	check_equal(fn.get_int() != 0, true);   // Initialized as 0, so now must be different
	fn.flip_bit(0);                         // reverting back
	check_equal(fn.get_int(), 0);
}

TESTSUITE_END()
