/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "wui/mapviewpixelfunctions.h"

#include "base/vector.h"

using namespace Widelands;

/**
 * Calculate brightness based upon the slopes.
 */
float MapviewPixelFunctions::calc_brightness(int32_t const l,
                                             int32_t const r,
                                             int32_t const tl,
                                             int32_t const tr,
                                             int32_t const bl,
                                             int32_t const br) {
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
	normal.x -= l * kHeightFactor;
	normal.x += r * kHeightFactor;
	normal.x -= tl * kHeightFactorFloat * kCos60;
	normal.y -= tl * kHeightFactorFloat * kSin60;
	normal.x += tr * kHeightFactorFloat * kCos60;
	normal.y -= tr * kHeightFactorFloat * kSin60;
	normal.x -= bl * kHeightFactorFloat * kCos60;
	normal.y += bl * kHeightFactorFloat * kSin60;
	normal.x += br * kHeightFactorFloat * kCos60;
	normal.y += br * kHeightFactorFloat * kSin60;
	normal.normalize();

	return normal.dot(sun_vect) * kLightFactor;
}

/**
 * Compute a - b, taking care to handle wrap-around effects properly.
 */
Vector2f MapviewPixelFunctions::calc_pix_difference(const Map& map, Vector2f a, Vector2f b) {
	normalize_pix(map, &a);
	normalize_pix(map, &b);

	Vector2f diff = a - b;

	int32_t map_end_screen_x = get_map_end_screen_x(map);
	if (diff.x > map_end_screen_x / 2.f)
		diff.x -= map_end_screen_x;
	else if (diff.x < -map_end_screen_x / 2.f)
		diff.x += map_end_screen_x;

	int32_t map_end_screen_y = get_map_end_screen_y(map);
	if (diff.y > map_end_screen_y / 2.f)
		diff.y -= map_end_screen_y;
	else if (diff.y < -map_end_screen_y / 2.f)
		diff.y += map_end_screen_y;

	return diff;
}

/**
 * Calculate the pixel (currently Manhattan) distance between the two points,
 * taking wrap-arounds into account.
 */
float MapviewPixelFunctions::calc_pix_distance(const Map& map, Vector2f a, Vector2f b) {
	normalize_pix(map, &a);
	normalize_pix(map, &b);
	uint32_t dx = std::abs(a.x - b.x), dy = std::abs(a.y - b.y);
	{
		const uint32_t map_end_screen_x = get_map_end_screen_x(map);
		if (dx > map_end_screen_x / 2)
			dx = -(dx - map_end_screen_x);
	}
	{
		const uint32_t map_end_screen_y = get_map_end_screen_y(map);
		if (dy > map_end_screen_y / 2)
			dy = -(dy - map_end_screen_y);
	}
	return dx + dy;
}

NodeAndTriangle<>
MapviewPixelFunctions::calc_node_and_triangle(const Map& map, uint32_t x, uint32_t y) {
	const uint16_t mapwidth = map.get_width();
	const uint16_t mapheight = map.get_height();
	const uint32_t map_end_screen_x = get_map_end_screen_x(map);
	const uint32_t map_end_screen_y = get_map_end_screen_y(map);
	while (x >= map_end_screen_x)
		x -= map_end_screen_x;
	while (y >= map_end_screen_y)
		y -= map_end_screen_y;
	Coords result_node;

	const uint16_t col_number = x / (kTriangleWidth / 2);
	uint16_t row_number = y / kTriangleHeight, next_row_number;
	assert(row_number < mapheight);
	const uint32_t left_col = col_number / 2;
	uint16_t right_col = (col_number + 1) / 2;
	if (right_col == mapwidth)
		right_col = 0;
	bool slash = (col_number + row_number) & 1;

	//  Find out which two nodes the mouse is between in the y-dimension, taking
	//  the height factor into account.
	//  There are 2 cases. One where the node in the left col has greater map
	//  y-coordinate than the right node. This is called slash because the edge
	//  between them goes in the direction of the '/' character. When slash is
	//  false, the edge goes in the direction of the '\' character.
	uint16_t screen_y_base = row_number * kTriangleHeight;
	int32_t upper_screen_dy,
	   lower_screen_dy =
	      screen_y_base -
	      map[Coords(slash ? right_col : left_col, row_number)].get_height() * kHeightFactor - y;
	for (;;) {
		screen_y_base += kTriangleHeight;
		next_row_number = row_number + 1;
		if (next_row_number == mapheight)
			next_row_number = 0;
		upper_screen_dy = lower_screen_dy;
		lower_screen_dy =
		   screen_y_base -
		   map[Coords(slash ? left_col : right_col, next_row_number)].get_height() * kHeightFactor -
		   y;
		if (lower_screen_dy < 0) {
			row_number = next_row_number;
			slash = !slash;
		} else
			break;
	}

	{  //  Calculate which of the 2 nodes (x, y) is closest to.
		uint16_t upper_x, lower_x, upper_screen_dx, lower_screen_dx;
		if (slash) {
			upper_x = right_col;
			lower_x = left_col;
			lower_screen_dx = x - col_number * (kTriangleWidth / 2);
			upper_screen_dx = (kTriangleWidth / 2) - lower_screen_dx;
		} else {
			upper_x = left_col;
			lower_x = right_col;
			upper_screen_dx = x - col_number * (kTriangleWidth / 2);
			lower_screen_dx = (kTriangleWidth / 2) - upper_screen_dx;
		}
		if (upper_screen_dx * upper_screen_dx + upper_screen_dy * upper_screen_dy <
		    lower_screen_dx * lower_screen_dx + lower_screen_dy * lower_screen_dy)
			result_node = Coords(upper_x, row_number);
		else
			result_node = Coords(lower_x, next_row_number);
	}

	// This will be overwritten in all cases below.
	TCoords<> result_triangle(Coords::null(), TriangleIndex::D);

	//  Find out which of the 4 possible triangles (x, y) is in.
	if (slash) {
		int32_t Y_a =
		   screen_y_base - kTriangleHeight -
		   map[Coords((right_col == 0 ? mapwidth : right_col) - 1, row_number)].get_height() *
		      kHeightFactor;
		int32_t Y_b = screen_y_base - kTriangleHeight -
		              map[Coords(right_col, row_number)].get_height() * kHeightFactor;
		int32_t ldy = Y_b - Y_a, pdy = Y_b - y;
		int32_t pdx = (col_number + 1) * (kTriangleWidth / 2) - x;
		assert(pdx > 0);
		if (pdy * kTriangleWidth > ldy * pdx) {
			//  (x, y) is in the upper triangle.
			result_triangle = TCoords<>(
			   Coords(left_col, (row_number == 0 ? mapheight : row_number) - 1), TriangleIndex::D);
		} else {
			Y_a = screen_y_base - map[Coords(left_col, next_row_number)].get_height() * kHeightFactor;
			ldy = Y_b - Y_a;
			if (pdy * (kTriangleWidth / 2) > ldy * pdx) {
				//  (x, y) is in the second triangle.
				result_triangle = TCoords<>(
				   Coords((right_col == 0 ? mapwidth : right_col) - 1, row_number), TriangleIndex::R);
			} else {
				Y_b = screen_y_base -
				      map[Coords(left_col + 1 == mapwidth ? 0 : left_col + 1, next_row_number)]
				            .get_height() *
				         kHeightFactor;
				ldy = Y_b - Y_a;
				pdy = Y_b - y;
				pdx = (col_number + 2) * (kTriangleWidth / 2) - x;
				if (pdy * kTriangleWidth > ldy * pdx) {
					//  (x, y) is in the third triangle.
					result_triangle = TCoords<>(Coords(right_col, row_number), TriangleIndex::D);
				} else {
					//  (x, y) is in the lower triangle.
					result_triangle = TCoords<>(Coords(left_col, next_row_number), TriangleIndex::R);
				}
			}
		}
	} else {
		int32_t Y_a = screen_y_base - kTriangleHeight -
		              map[Coords(left_col, row_number)].get_height() * kHeightFactor;
		int32_t Y_b =
		   screen_y_base - kTriangleHeight -
		   map[Coords(left_col + 1 == mapwidth ? 0 : left_col + 1, row_number)].get_height() *
		      kHeightFactor;
		int32_t ldy = Y_b - Y_a, pdy = Y_b - y;
		int32_t pdx = (col_number + 2) * (kTriangleWidth / 2) - x;
		assert(pdx > 0);
		if (pdy * kTriangleWidth > ldy * pdx) {
			//  (x, y) is in the upper triangle.
			result_triangle = TCoords<>(
			   Coords(right_col, (row_number == 0 ? mapheight : row_number) - 1), TriangleIndex::D);
		} else {
			Y_b = screen_y_base - map[Coords(right_col, next_row_number)].get_height() * kHeightFactor;
			ldy = Y_b - Y_a;
			pdy = Y_b - y;
			pdx = (col_number + 1) * (kTriangleWidth / 2) - x;
			if (pdy * (kTriangleWidth / 2) > ldy * pdx) {
				//  (x, y) is in the second triangle.
				result_triangle = TCoords<>(Coords(left_col, row_number), TriangleIndex::R);
			} else {
				Y_a = screen_y_base -
				      map[Coords((right_col == 0 ? mapwidth : right_col) - 1, next_row_number)]
				            .get_height() *
				         kHeightFactor;
				ldy = Y_b - Y_a;
				if (pdy * kTriangleWidth > ldy * pdx) {
					//  (x, y) is in the third triangle.
					result_triangle = TCoords<>(Coords(left_col, row_number), TriangleIndex::D);
				} else {
					//  (x, y) is in the lower triangle.
					result_triangle =
					   TCoords<>(Coords((right_col == 0 ? mapwidth : right_col) - 1, next_row_number),
					             TriangleIndex::R);
				}
			}
		}
	}
	assert(result_triangle.node != Coords::null());
	return NodeAndTriangle<>{result_node, result_triangle};
}

/**
 * Normalize pixel points of the map.
 */
void MapviewPixelFunctions::normalize_pix(const Map& map, Vector2f* p) {
	const float map_end_screen_x = get_map_end_screen_x(map);
	while (p->x >= map_end_screen_x) {
		p->x -= map_end_screen_x;
	}
	while (p->x < 0) {
		p->x += map_end_screen_x;
	}
	const float map_end_screen_y = get_map_end_screen_y(map);
	while (p->y >= map_end_screen_y) {
		p->y -= map_end_screen_y;
	}
	while (p->y < 0) {
		p->y += map_end_screen_y;
	}
}
