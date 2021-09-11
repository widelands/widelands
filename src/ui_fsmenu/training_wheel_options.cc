/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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
#include "ui_fsmenu/training_wheel_options.h"

#include <memory>

#include "base/i18n.h"
#include "scripting/lua_interface.h"
#include "ui_basic/box.h"
#include "ui_basic/textarea.h"

namespace FsMenu {

constexpr int kPadding = 4;

TrainingWheelOptions::TrainingWheelOptions(Panel* parent)
   : UI::Window(parent,
                UI::WindowStyle::kFsMenu,
                "training_wheel_options",
                parent->get_x(),
                parent->get_y(),
                parent->get_w(),
                parent->get_h(),
                /** TRANSLATORS: Window Title in Options */
                pgettext("teaching_progress", "Edit Teaching Progress")),
     lua_(new LuaInterface()),
     training_wheels_(new Widelands::TrainingWheels(*lua_)),
     mark_unmark_state_(false) {

	UI::PanelStyle panel_style = UI::PanelStyle::kFsMenu;

	// Some horizontal padding
	UI::Box* wrapper_box = new UI::Box(this, panel_style, 0, 0, UI::Box::Horizontal, 0, 0, kPadding);
	set_center_panel(wrapper_box);

	wrapper_box->add_space(0);

	UI::Box* main_box =
	   new UI::Box(wrapper_box, panel_style, 0, 0, UI::Box::Vertical, 0, 0, kPadding);
	wrapper_box->add(main_box, UI::Box::Resizing::kExpandBoth);

	main_box->add_space(0);

	const auto& objectives = training_wheels_->all_objectives();
	UI::Textarea* title_label =
	   new UI::Textarea(main_box, panel_style, UI::FontStyle::kWuiInfoPanelHeading,
	                    /** TRANSLATORS: Title above a list */
	                    pgettext("teaching_progress", "Teaching objectives"));
	main_box->add(title_label, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box->add_space(0);

	// Add a checkbox for each training wheel.
	// Box should scroll if there are many.
	main_box->add_space(kPadding);

	UI::Box* list_box = new UI::Box(
	   main_box, panel_style, 0, 0, UI::Box::Vertical, 0, parent->get_h() * 3 / 4, kPadding);
	list_box->set_scrolling(true);
	main_box->add(list_box, UI::Box::Resizing::kExpandBoth);

	// If some but not all wheels are solved, the Mark/Unmark button should be labelled "Unmark All"
	// instead of "Mark All" initially.
	bool has_solved = false;
	for (const auto& objective : objectives) {
		if (objective.second.solved) {
			has_solved = true;
		}
		UI::Checkbox* checkbox = new UI::Checkbox(
		   list_box, UI::PanelStyle::kFsMenu, Vector2i::zero(), objective.second.descname);
		checkbox->set_state(objective.second.solved);
		list_box->add(checkbox, UI::Box::Resizing::kFullSize);
		checkboxes_.insert(
		   std::make_pair(objective.first, TrainingWheelOptions::Entry(objective.second, checkbox)));
		list_box->add_space(0);
	}

	mark_unmark_state_ = !has_solved;

	main_box->add_space(kPadding);

	// Bottom button row with actions
	UI::Box* horizontal_box =
	   new UI::Box(main_box, panel_style, 0, 0, UI::Box::Horizontal, 0, 0, kPadding);
	main_box->add(horizontal_box, UI::Box::Resizing::kFullSize);

	horizontal_box->add_space(0);
	horizontal_box->add_inf_space();

	// Close without any action
	UI::Button* cancel_button = new UI::Button(
	   horizontal_box, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Cancel"));
	horizontal_box->add(cancel_button, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	horizontal_box->add_inf_space();
	// Ensure some space between the buttons
	horizontal_box->add_space(0);

	cancel_button->sigclicked.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack); });

	// Reset states of the checkboxes
	UI::Button* reset_button = new UI::Button(
	   horizontal_box, "reset", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Reset"));
	horizontal_box->add(reset_button, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	horizontal_box->add_inf_space();
	horizontal_box->add_space(0);

	reset_button->sigclicked.connect([this]() {
		for (const auto& checkboxinfo : checkboxes_) {
			checkboxinfo.second.checkbox->set_state(checkboxinfo.second.initial_state);
		}
	});

	// Mark/Unmark all checkboxes
	mark_unmark_button_ = new UI::Button(
	   horizontal_box, "mark_unmark", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, "");
	horizontal_box->add(mark_unmark_button_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	horizontal_box->add_inf_space();
	horizontal_box->add_space(0);

	// Toggle twice to make it expand to both labels
	int desired_width = mark_unmark_button_->get_w();
	mark_unmark_button_->expand(0, 0);
	toggle_mark_unmark_all_button();
	mark_unmark_button_->expand(0, 0);
	desired_width = std::max(desired_width, mark_unmark_button_->get_w());
	toggle_mark_unmark_all_button();

	mark_unmark_button_->sigclicked.connect([this]() {
		for (const auto& checkboxinfo : checkboxes_) {
			checkboxinfo.second.checkbox->set_state(mark_unmark_state_);
		}
		toggle_mark_unmark_all_button();
	});

	// Apply settings from checkboxes and close
	UI::Button* ok_button =
	   new UI::Button(horizontal_box, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK"));
	horizontal_box->add(ok_button, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	horizontal_box->add_inf_space();
	horizontal_box->add_space(0);

	ok_button->sigclicked.connect([this]() { clicked_ok(); });

	main_box->add_space(0);

	wrapper_box->add_space(0);

	// Make all buttons the same width
	desired_width = std::max(std::max(cancel_button->get_w(), reset_button->get_w()),
	                         std::max(desired_width, ok_button->get_w()));

	cancel_button->set_desired_size(desired_width, cancel_button->get_h());
	reset_button->set_desired_size(desired_width, reset_button->get_h());
	mark_unmark_button_->set_desired_size(desired_width, mark_unmark_button_->get_h());
	ok_button->set_desired_size(desired_width, ok_button->get_h());

	center_to_parent();

	initialization_complete();
}

void TrainingWheelOptions::toggle_mark_unmark_all_button() {
	mark_unmark_state_ = !mark_unmark_state_;
	/** TRANSLATORS: Button label to mark or unmark all checkboxes in an options window */
	mark_unmark_button_->set_title(mark_unmark_state_ ? _("Mark All") : _("Unmark All"));
}

void TrainingWheelOptions::clicked_ok() {
	for (const auto& checkboxinfo : checkboxes_) {
		const bool solve = checkboxinfo.second.checkbox->get_state();
		if (solve) {
			training_wheels_->mark_as_solved(checkboxinfo.first, false);
		} else {
			training_wheels_->mark_as_unsolved(checkboxinfo.first);
		}
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
}

bool TrainingWheelOptions::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			clicked_ok();
			return true;
		case SDLK_ESCAPE:
			end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

}  // namespace FsMenu
#endif
