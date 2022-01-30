/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "logic/field.h"

#include "wui/mapviewpixelconstants.h"

namespace Widelands {

/**
 * Set the field's brightness based upon the slopes.
 * Slopes are calulated as this field's height - neighbour's height.
 */
void Field::set_brightness(int32_t const l,
                           int32_t const r,
                           int32_t const tl,
                           int32_t const tr,
                           int32_t const bl,
                           int32_t const br) {

	auto calc_brightness = [](int32_t const left, int32_t const right, int32_t const topleft,
	                          int32_t const topright, int32_t const bottomleft,
	                          int32_t const bottomright) {
		constexpr float kVectorThird = 0.57735f;  // sqrt(1/3)
		constexpr float kCos60 = 0.5f;
		constexpr float kSin60 = 0.86603f;
		constexpr float kLightFactor = -75.0f;

		static Vector3f sun_vect =
		   Vector3f(kVectorThird, -kVectorThird, -kVectorThird);  //  |sun_vect| = 1

		// find normal
		// more guessed than thought about
		// but hey, results say I am good at guessing :)
		// perhaps I will paint an explanation for this someday
		// florian
		Vector3f normal(0, 0, kTriangleWidth);
		normal.x -= left * kHeightFactor;
		normal.x += right * kHeightFactor;
		normal.x -= topleft * kHeightFactorFloat * kCos60;
		normal.y -= topleft * kHeightFactorFloat * kSin60;
		normal.x += topright * kHeightFactorFloat * kCos60;
		normal.y -= topright * kHeightFactorFloat * kSin60;
		normal.x -= bottomleft * kHeightFactorFloat * kCos60;
		normal.y += bottomleft * kHeightFactorFloat * kSin60;
		normal.x += bottomright * kHeightFactorFloat * kCos60;
		normal.y += bottomright * kHeightFactorFloat * kSin60;
		normal.normalize();

		return normal.dot(sun_vect) * kLightFactor;
	};

	// HACK to normalize flat terrain to zero brightness
	static float flatbrightness = calc_brightness(0, 0, 0, 0, 0, 0);

	float b = calc_brightness(l, r, tl, tr, bl, br) - flatbrightness;

	if (b > 0) {
		b *= 1.5;
	}

	if (b < -128) {
		b = -128;
	} else if (b > 127) {
		b = 127;
	}
	brightness = static_cast<int8_t>(b);
}
}  // namespace Widelands
