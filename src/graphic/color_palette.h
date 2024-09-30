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

#ifndef WL_GRAPHIC_COLOR_PALETTE_H
#define WL_GRAPHIC_COLOR_PALETTE_H

#include <cstddef>
#include <cstdint>
#include <set>
#include <vector>

#include "graphic/color.h"

// TODO(tothxa): This is specific to the needs of ocean colors for now. If any other use arises,
//               then it has to be generalised: at least allow specifying the initial colors and
//               changing check_color(), but possibly also the sequences in generate_more_colors().
class ColorPalette {
public:
	// Takes care of generating colors as needed
	[[nodiscard]] const RGBColor& at(size_t i);

	ColorPalette();

private:
	void generate_more_colors();

	std::vector<RGBColor> colors_;

	// Internals for generate_more_colors()
	[[nodiscard]] bool check_color(const RGBColor& color) const;
	void generate_more_values();
	std::vector<uint8_t> values_sequence_;
	uint8_t value_step_;
	std::set<uint32_t> rgb_cache_;
};

extern ColorPalette kOceanColors;

#endif  // end of include guard: WL_GRAPHIC_COLOR_PALETTE_H
