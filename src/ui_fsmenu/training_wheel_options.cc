/*
 * Copyright (C) 2020 by the Widelands Development Team
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

#include "ui_fsmenu/training_wheel_options.h"

#include <memory>

#include "base/i18n.h"
#include "logic/training_wheels.h"
#include "scripting/lua_interface.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

constexpr int kPadding = 4;

TrainingWheelOptions::TrainingWheelOptions(Panel* parent)
   : UI::Window(parent,
                "training_wheel_options",
                parent->get_x(),
                parent->get_y(),
                parent->get_w(),
                parent->get_h(),
                _("Reset Teaching Progress")),
     lua_(new LuaInterface()),
     training_wheels_(new Widelands::TrainingWheels(*lua_)) {
	UI::Box* main_box = new UI::Box(this, 0, 0, UI::Box::Vertical, 0, 0, kPadding);
	set_center_panel(main_box);

	main_box->add_space(0);

	// Some horizontal padding for the title label
	UI::Box* label_box = new UI::Box(main_box, 0, 0, UI::Box::Horizontal, 0, 0, kPadding);
	main_box->add(label_box, UI::Box::Resizing::kFullSize);

	label_box->add_space(0);

	const std::map<std::string, std::string>& objectives = training_wheels_->solved_objectives();
	UI::Textarea* title_label =
	   new UI::Textarea(label_box, objectives.empty() ?
	                                  /** TRANSLATORS: Title above a list */
	                                  _("There is no teaching progress yet") :
	                                  /** TRANSLATORS: Title above a list */
	                                  _("Completed teaching objectives"));
	label_box->add(title_label, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	label_box->add_space(0);

	// Add a row for each solved training wheel with a reset button.
	// Box should scroll if there are many.
	if (!objectives.empty()) {
		main_box->add_space(kPadding);
		title_label->set_style(g_style_manager->font_style(UI::FontStyle::kWuiInfoPanelHeading));

		UI::Box* list_box =
		   new UI::Box(main_box, 0, 0, UI::Box::Vertical, 0, parent->get_h(), kPadding);
		list_box->set_scrolling(true);
		main_box->add(list_box, UI::Box::Resizing::kExpandBoth);

		for (const auto& objective : objectives) {
			UI::Box* entry_box = new UI::Box(list_box, 0, 0, UI::Box::Horizontal, 0, 0, kPadding);
			list_box->add(entry_box, UI::Box::Resizing::kFullSize);
			entry_box->add_space(0);

			UI::Textarea* entry_label = new UI::Textarea(entry_box, objective.second);
			entry_box->add(entry_label, UI::Box::Resizing::kFullSize);
			entry_label->set_style(g_style_manager->font_style(UI::FontStyle::kWuiInfoPanelParagraph));

			entry_box->add_inf_space();

			UI::Button* reset_button = new UI::Button(
			   entry_box, "reset_progress", 0, 0, 0, 0, UI::ButtonStyle::kWuiSecondary, _("Reset"));
			entry_box->add(reset_button);
			reset_button->sigclicked.connect([this, reset_button, &objective]() {
				reset_button->set_enabled(false);
				training_wheels_->mark_as_unsolved(objective.first);
			});

			entry_box->add_space(0);
		}
	}

	main_box->add_space(kPadding);

	UI::Button* ok_button =
	   new UI::Button(main_box, "ok", 0, 0, 0, 0, UI::ButtonStyle::kWuiPrimary, _("Done"));
	main_box->add(ok_button, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	ok_button->sigclicked.connect(
	   [this]() { end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk); });

	main_box->add_space(0);
}
