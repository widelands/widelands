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

#ifndef WL_UI_BASIC_INFINITE_SPINNER_H
#define WL_UI_BASIC_INFINITE_SPINNER_H

#include "ui/basic/box.h"
#include "ui/basic/button.h"
#include "ui/basic/spinbox.h"

namespace UI {

/**
 * A box that combines a regular spinbox with a separate button to toggle infinity on/off.
 */
struct InfiniteSpinner : public Box {
	static constexpr const char* kIconEndInfinity = "images/wui/menus/end_infinity.png";
	static constexpr const char* kIconInfinity = "images/wui/menus/infinity.png";

	InfiniteSpinner(Panel* parent,
	                const std::string& name,
	                PanelStyle style,
	                const std::string& button_tooltip,
	                const std::string& spinner_label,
	                int32_t spinner_val,
	                int32_t spinner_min,
	                int32_t spinner_max,
	                SpinBox::Units spinner_units = SpinBox::Units::kNone);

	Notifications::Signal<> changed;

	[[nodiscard]] bool is_infinite() const;
	[[nodiscard]] int32_t get_value() const;

	void set_infinite() const;
	void set_value(int32_t value, bool trigger_signal = true);

private:
	UI::SpinBox spinner_;
	UI::Button button_;
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_INFINITE_SPINNER_H
