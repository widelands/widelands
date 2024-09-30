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

#include "editor/ocean_colors.h"

#include <algorithm>
#include <cassert>

#include "base/log.h"
#include "base/wexception.h"

OceanColors kOceanColors;

constexpr uint32_t kMaxValue = 0xff;
constexpr uint32_t kHalfValue = 0x7f;
constexpr uint32_t kInitStep = 0x80;
constexpr uint32_t kMinContrast = 0x20;

OceanColors::OceanColors()
   : colors_({
        // Initialise with main colors.
        // Colors in ARGB notation.

        // base
        0xffff0000,  // red
        0xff00ff00,  // green
        0xffffff00,  // yellow
        0xffff00ff,  // magenta
        0xff7f7f7f,  // gray

        // light
        0xffff7f7f,  // red
        0xff7fff7f,  // green
        0xffffff7f,  // yellow
        0xffff7fff,  // magenta
        0xffbfbfbf,  // gray

        // dark
        0xffbf0000,  // red
        0xff00bf00,  // green
        0xffbfbf00,  // yellow
        0xffbf00bf,  // magenta
        0xff5f5f5f,  // gray (3f is too dark for now)
     }),
     values_sequence_({0, kMaxValue, kHalfValue}),
     value_step_(kInitStep) {
}

uint32_t OceanColors::at(const size_t i) {
	if (i < colors_.size()) {
		return colors_.at(i);
	}
	if (i > colors_.size()) {
		throw wexception("Unallocated ocean color was requested!");
	}
	generate_more_colors();
	assert(colors_.size() > i);
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

void OceanColors::generate_more_colors() {
	generate_more_values();

	// Let's shuffle them as much as we can
	for (const uint32_t mid : values_sequence_) {
		for (const uint32_t min : values_sequence_) {
			if (min > mid) {
				continue;
			}
			for (auto max_it = values_sequence_.rbegin(); max_it != values_sequence_.rend();
			     ++max_it) {
				const uint32_t max = *max_it;
				if (mid > max) {
					continue;
				}
				std::vector<uint32_t> current_values{max, mid, min};
				for (const Hue& hue : hue_sequence) {
					add_color(current_values.at(hue.index_red), current_values.at(hue.index_green),
					          current_values.at(hue.index_blue));
				}
			}
		}
	}
	verb_log_dbg("We have %" PRIuS " ocean colors with %" PRIuS " palette steps per channel.",
	             colors_.size(), values_sequence_.size());
}

void OceanColors::generate_more_values() {
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

void OceanColors::add_color(const uint32_t red, const uint32_t green, const uint32_t blue) {
	assert(red <= kMaxValue);
	assert(green <= kMaxValue);
	assert(blue <= kMaxValue);

	if (!check_color(red, green, blue)) {
		return;
	}

	constexpr uint32_t kAlpha = kMaxValue << 24;
	constexpr uint32_t kRedShift = 16;
	constexpr uint32_t kGreenShift = 8;

	uint32_t argb_color = kAlpha + (red << kRedShift) + (green << kGreenShift) + blue;
	if (std::find(colors_.begin(), colors_.end(), argb_color) == colors_.end()) {
		colors_.emplace_back(argb_color);
	}
}

bool OceanColors::check_color(const uint32_t red, const uint32_t green, const uint32_t blue) {
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
		return false;
	}

	return true;
}
