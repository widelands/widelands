/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
#ifndef WL_UI_FSMENU_TRAINING_WHEEL_OPTIONS_H
#define WL_UI_FSMENU_TRAINING_WHEEL_OPTIONS_H

#include "logic/training_wheels.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/window.h"

#include <memory>
#include <set>

class LuaInterface;

namespace FsMenu {

/**
 * @brief The TrainingWheelOptions class presents buttons to the user to reset the progress for
 * individual training wheels.
 */
class TrainingWheelOptions : public UI::Window {
public:
	explicit TrainingWheelOptions(Panel* parent);

	bool handle_key(bool, SDL_Keysym) override;

private:
	struct Entry {
		explicit Entry(const Widelands::TrainingWheels::TrainingWheel& init_training_wheel,
		               UI::Checkbox* init_checkbox)
		   : training_wheel(init_training_wheel),
		     initial_state(init_training_wheel.solved),
		     checkbox(init_checkbox) {
		}
		const Widelands::TrainingWheels::TrainingWheel training_wheel;
		const bool initial_state;
		UI::Checkbox* checkbox;
	};

	void clicked_ok();

	void toggle_mark_unmark_all_button();

	std::unique_ptr<LuaInterface> lua_;
	std::set<UI::Button*> reset_buttons_;
	std::map<std::string, Entry> checkboxes_;
	std::unique_ptr<Widelands::TrainingWheels> training_wheels_;
	UI::Button* mark_unmark_button_;
	bool mark_unmark_state_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_TRAINING_WHEEL_OPTIONS_H
#endif
