/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_BASE_MATH_H
#define WL_BASE_MATH_H

#include <string>

namespace math {

// Returns 1 for positive and -1 for negative numbers.
template <typename T> T sign(const T& val) {
	return val < T(0.) ? T(-1.) : T(1.);
}

// Clamps 'val' to 'min' and 'max'.
template <typename T> T clamp(const T& val, const T& low, const T& high) {
	if (val < low) {
		return low;
	}
	if (val > high) {
		return high;
	}
	return val;
}

// A simple square function.
template <typename T> T sqr(const T& a) {
	return a * a;
}

static constexpr unsigned k100PercentAsInt = 10000U;

/**
 * @brief Reads a percentage
 * @param input A percentage in the format 12%, 12.5% or 12.53%.
 * @return Scaled percentage as integer, where 100% corresponds to k100PercentAsInt.
 * */
unsigned read_percent_to_int(const std::string& input);

// Wrappers around stoi/stoll with more verbose exceptions
int to_int(const std::string&);
long long to_long(const std::string&);

}  // namespace math

#endif  // end of include guard: WL_BASE_MATH_H
