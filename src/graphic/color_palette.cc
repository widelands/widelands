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

#include "graphic/color_palette.h"

#include <algorithm>
#include <cassert>

#include "base/log.h"
#include "base/wexception.h"

ColorPalette kOceanColors;

constexpr uint8_t kMaxValue = 0xff;
constexpr uint8_t kHalfValue = 0x7f;
constexpr uint8_t kInitStep = 0x80;
constexpr uint8_t kMinContrast = 0x20;

ColorPalette::ColorPalette()
   : colors_({
        // Initialise with main colors.

        // base
        RGBColor(0xff0000),  // red
        RGBColor(0x00ff00),  // green
        RGBColor(0xffff00),  // yellow
        RGBColor(0xff00ff),  // magenta
        RGBColor(0x7f7f7f),  // gray

        // light
        RGBColor(0xff7f7f),  // red
        RGBColor(0x7fff7f),  // green
        RGBColor(0xffff7f),  // yellow
        RGBColor(0xff7fff),  // magenta
        RGBColor(0xbfbfbf),  // gray

        // dark
        RGBColor(0xbf0000),  // red
        RGBColor(0x00bf00),  // green
        RGBColor(0xbfbf00),  // yellow
        RGBColor(0xbf00bf),  // magenta
        RGBColor(0x5f5f5f),  // gray (3f is too dark for now)
     }),
     values_sequence_({0, kMaxValue, kHalfValue}),
     value_step_(kInitStep) {
	for (auto color : colors_) {
		rgb_cache_.insert((color.r << 16) + (color.g << 8) + color.b);
	}
}

const RGBColor& ColorPalette::at(const size_t i) {
	while (i >= colors_.size()) {
		generate_more_colors();
	}
	return colors_.at(i);
}

// A Hue contains the indices within a {max, mid, min} vector to be assigned to each color
// component, i.e. the component with index 0 will get the highest value, 1 the middle and
// 2 the lowest value from the ordered vector of values.
struct Hue {
	size_t index_red;
	size_t index_green;
	size_t index_blue;
};
static const std::vector<Hue> hue_sequence{
   {0, 1, 2}, {2, 0, 1}, {1, 2, 0}, {0, 2, 1}, {2, 1, 0}, {1, 0, 2}};

void ColorPalette::generate_more_colors() {
	generate_more_values();

	// Let's shuffle them as much as we can
	for (const uint8_t mid : values_sequence_) {
		for (const uint8_t min : values_sequence_) {
			if (min > mid) {
				continue;
			}
			for (auto max_it = values_sequence_.rbegin(); max_it != values_sequence_.rend();
			     ++max_it) {
				const uint8_t max = *max_it;
				if (mid > max) {
					continue;
				}
				std::vector<uint8_t> current_values{max, mid, min};
				for (const Hue& hue : hue_sequence) {
					const uint32_t rgb = (current_values.at(hue.index_red) << 16) +
					                     (current_values.at(hue.index_green) << 8) +
					                     current_values.at(hue.index_blue);
					if (rgb_cache_.count(rgb) > 0) {
						continue;
					}
					const RGBColor next_color(current_values.at(hue.index_red),
					                          current_values.at(hue.index_green),
					                          current_values.at(hue.index_blue));
					if (check_color(next_color)) {
						colors_.emplace_back(next_color);
						// We must only cache added colors in the main cache, because previously
						// filtered colors may become allowed when value_step_ decreases!
						rgb_cache_.insert(rgb);
					}
				}
			}
		}
	}
	verb_log_dbg("We have %" PRIuS " ocean colors with %" PRIuS " palette steps per channel.",
	             colors_.size(), values_sequence_.size());
}

void ColorPalette::generate_more_values() {
	if (value_step_ < 2) {
		throw wexception("Too many ocean colors are requested. This shouldn't be possible.");
	}
	if (value_step_ <= kMinContrast) {
		log_warn("More than %" PRIuS " ocean colors are requested. Contrast will be very low.",
		         colors_.size());
	}
	value_step_ /= 2;

	const size_t previously_added_values_start = values_sequence_.size() / 2 + 1;
	const size_t previously_added_values_end = values_sequence_.size() - 1;

	// Let's shuffle them as much as we can
	for (size_t i = previously_added_values_start; i <= previously_added_values_end; ++i) {
		values_sequence_.emplace_back(values_sequence_.at(i) - value_step_);
	}
	for (size_t i = previously_added_values_start; i <= previously_added_values_end; ++i) {
		values_sequence_.emplace_back(values_sequence_.at(i) + value_step_);
	}
}

bool ColorPalette::check_color(const RGBColor& color) const {
	const uint32_t red = color.r;
	const uint32_t green = color.g;
	const uint32_t blue = color.b;

	// Don't allow shades of blue, except very dark ones

	constexpr uint32_t kDecreaseGreen = 3;
	constexpr uint32_t kAllowedBlue = 0x5f;
	constexpr uint32_t kAllowedBrightness = 0x7f;

	const uint32_t brightness = red + green + blue;

	if (blue > red + green - green / kDecreaseGreen) {
		if (blue > kAllowedBlue) {
			return false;
		}
		if (brightness > kAllowedBrightness) {
			return false;
		}
	}

	// Don't allow colors that don't have enough contrast to neighbors,
	// but only if value_step_ itself is big enough for contrasts.
	if (value_step_ < kMinContrast) {
		return true;
	}

	constexpr uint32_t kLowValue = kMinContrast - 1;
	constexpr uint32_t kStrongRed = 0xdf;
	constexpr uint32_t kStrongBlue = 0xdf;
	constexpr uint32_t kStrongGreen = 0x5f;
	constexpr uint32_t kSecondStep = 0x3f;

	if (green == kLowValue && red >= kStrongRed && blue >= kStrongBlue) {
		return false;
	}

	if (green >= kStrongGreen && (red == kLowValue || blue == kLowValue)) {
		return false;
	}

	if (green == kMaxValue) {
		if (red == 0 && blue == kSecondStep) {
			return false;
		}
		if (red == kSecondStep && blue == 0) {
			return false;
		}
	}

	constexpr uint32_t kMaxBrightness = 3 * kMaxValue;

	// Allow pure black and white, but not colors too close to them
	if (brightness == value_step_ || brightness == 2 * value_step_) {
		return false;
	}

	const uint32_t below_max = kMaxBrightness - brightness;
	if (below_max == value_step_ || below_max == 2 * value_step_) {
		// NOLINTNEXTLINE(readability-simplify-boolean-expr)
		return false;
	}

	// Tell clang-tidy that this is the common else clause, not just for the last condition
	// NOLINTNEXTLINE(readability-simplify-boolean-expr)
	return true;
}
