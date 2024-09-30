/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#ifndef WL_EDITOR_OCEAN_COLORS_H
#define WL_EDITOR_OCEAN_COLORS_H

#include <cstddef>
#include <cstdint>
#include <vector>

class OceanColors {
public:
	// Takes care of generating colors as needed
	uint32_t at(size_t i);

	OceanColors();

private:
	void generate_more_colors();

	std::vector<uint32_t> colors_;

	// Internals for generate_more_colors()
	void add_color(uint32_t red, uint32_t green, uint32_t blue);
	bool check_color(uint32_t red, uint32_t green, uint32_t blue) const;
	void generate_more_values();
	std::vector<uint8_t> values_sequence_;
	uint8_t value_step_;
};

extern OceanColors kOceanColors;

#endif  // end of include guard: WL_EDITOR_OCEAN_COLORS_H
