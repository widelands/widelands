/*
 * Copyright (C) 2025-2026 by the Widelands Development Team
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

#include "ui_basic/infinite_spinner.h"

#include "graphic/image_cache.h"

namespace UI {

InfiniteSpinner::InfiniteSpinner(Panel* parent,
                                 const std::string& name,
                                 const PanelStyle style,
                                 const std::string& button_tooltip,
                                 const std::string& spinner_label,
                                 const int32_t spinner_val,
                                 const int32_t spinner_min,
                                 const int32_t spinner_max,
                                 SpinBox::Units spinner_units)
   : Box(parent, style, name, 0, 0, Box::Horizontal),
     spinner_(this,
              "spinner",
              0,
              0,
              400,
              250,
              spinner_val,
              spinner_min,
              spinner_max,
              style,
              spinner_label,
              spinner_units,
              SpinBox::Type::kBig),
     button_(this,
             "infinity_button",
             0,
             0,
             34,
             34,
             style == PanelStyle::kWui ? ButtonStyle::kWuiSecondary : ButtonStyle::kFsMenuSecondary,
             g_image_cache->get(kIconInfinity),
             button_tooltip) {

	button_.sigclicked.connect([this, spinner_min, spinner_max]() {
		const bool now_infinite = button_.style() != Button::VisualState::kPermpressed;
		button_.set_perm_pressed(now_infinite);
		button_.set_pic(g_image_cache->get(now_infinite ? kIconEndInfinity : kIconInfinity));
		if (now_infinite) {
			spinner_.set_interval(spinner_.get_value(), spinner_.get_value());
		} else {
			spinner_.set_interval(spinner_min, spinner_max);
		}
		changed();
	});

	spinner_.changed.connect(changed);

	add(&button_, Box::Resizing::kAlign, Align::kCenter);
	add_space(4);
	add(&spinner_, Box::Resizing::kFillSpace, Align::kCenter);
}

bool InfiniteSpinner::is_infinite() const {
	return button_.style() == Button::VisualState::kPermpressed;
}

int32_t InfiniteSpinner::get_value() const {
	return spinner_.get_value();
}

void InfiniteSpinner::set_infinite() const {
	if (!is_infinite()) {
		button_.sigclicked();
		assert(is_infinite());
	}
}
void InfiniteSpinner::set_value(int32_t value, bool trigger_signal) {
	if (!is_infinite()) {
		spinner_.set_value(value, trigger_signal);
	}
}

}  // namespace UI
