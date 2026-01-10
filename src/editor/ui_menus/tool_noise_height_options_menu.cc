/*
 * Copyright (C) 2002-2026 by the Widelands Development Team
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

#include "editor/ui_menus/tool_noise_height_options_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/noise_height_tool.h"
#include "graphic/text_layout.h"
#include "logic/widelands_geometry.h"
#include "ui_basic/textarea.h"

EditorToolNoiseHeightOptionsMenu::EditorToolNoiseHeightOptionsMenu(
   EditorInteractive& parent,
   EditorNoiseHeightTool& noise_tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Random Height Options"), noise_tool),
     noise_tool_(noise_tool),
     box_(this, UI::PanelStyle::kWui, "main_box", hmargin(), vmargin(), UI::Box::Vertical),
     lower_box_(&box_, UI::PanelStyle::kWui, "lower_box", 0, 0, UI::Box::Horizontal),
     upper_box_(&box_, UI::PanelStyle::kWui, "upper_box", 0, 0, UI::Box::Horizontal),
     set_to_box_(&box_, UI::PanelStyle::kWui, "set_box", 0, 0, UI::Box::Horizontal),
     lower_(&lower_box_,
            "lower",
            0,
            0,
            330,
            80,
            noise_tool_.get_interval().min,
            1,
            MAX_FIELD_HEIGHT,
            UI::PanelStyle::kWui,
            _("Minimum height:"),
            UI::SpinBox::Units::kNone,
            UI::SpinBox::Type::kSmall),
     upper_(&upper_box_,
            "upper",
            0,
            0,
            330,
            80,
            noise_tool_.get_interval().max,
            0,
            MAX_FIELD_HEIGHT,
            UI::PanelStyle::kWui,
            _("Maximum height:"),
            UI::SpinBox::Units::kNone,
            UI::SpinBox::Type::kSmall),
     set_to_(&set_to_box_,
             "set_to",
             0,
             0,
             330,
             80,
             noise_tool_.set_tool().get_interval().min,
             0,
             MAX_FIELD_HEIGHT,
             UI::PanelStyle::kWui,
             _("Set height to:"),
             UI::SpinBox::Units::kNone,
             UI::SpinBox::Type::kSmall) {
	box_.set_size(330, 50);

	lower_.set_tooltip(
	   /** TRANSLATORS: Editor random height access key. **/
	   _("Click on the map to set terrain height to a random value within the specified range"));
	upper_.set_tooltip(
	   /** TRANSLATORS: Editor random height access key. **/
	   _("Click on the map to set terrain height to a random value within the specified range"));
	set_to_.set_tooltip(
	   /** TRANSLATORS: Editor set height access key. **/
	   _("Ctrl + Click on the map to set terrain height"));

	lower_.changed.connect([this]() { update_lower(); });
	upper_.changed.connect([this]() { update_upper(); });
	set_to_.changed.connect([this]() { update_set_to(); });

	UI::Textarea* label =
	   new UI::Textarea(&box_, UI::PanelStyle::kWui, "label_random", UI::FontStyle::kWuiLabel, 0, 0,
	                    0, 0, _("Random height"), UI::Align::kCenter);
	box_.add(label, UI::Box::Resizing::kFullSize);
	upper_box_.add(&upper_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	lower_box_.add(&lower_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	box_.add(&upper_box_, UI::Box::Resizing::kFullSize);
	box_.add_space(vspacing());
	box_.add(&lower_box_, UI::Box::Resizing::kFullSize);

	box_.add_space(2 * vspacing());
	label = new UI::Textarea(&box_, UI::PanelStyle::kWui, "label_fixed", UI::FontStyle::kWuiLabel, 0,
	                         0, 0, 0, _("Fixed height"), UI::Align::kCenter);
	box_.add(label, UI::Box::Resizing::kFullSize);
	set_to_box_.add(&set_to_);
	box_.add(&set_to_box_, UI::Box::Resizing::kFullSize);

	for (UI::Box* box : {&upper_box_, &lower_box_, &set_to_box_}) {
		UI::SpinBox* spinbox = dynamic_cast<UI::SpinBox*>(box->get_first_child());
		assert(spinbox != nullptr);

		std::string tooltip = _("Select the height of a field by clicking on it on the map");
		if (box == &set_to_box_) {
			tooltip = as_tooltip_text_with_hotkey(
			   tooltip, shortcut_string_for(KeyboardShortcut::kEditorPicker, true),
			   UI::PanelStyle::kWui);
		}

		UI::Button* picker = new UI::Button(
		   box, "picker", 0, 0, 0, 0, UI::ButtonStyle::kWuiSecondary, _("Pick …"), tooltip);
		box->add_space(hmargin());
		box->add(picker, UI::Box::Resizing::kAlign, UI::Align::kCenter);

		picker->sigclicked.connect([this, spinbox]() {
			if (picker_is_active()) {
				if (spinbox == picker_target_) {
					deactivate_picker();
				}
			} else {
				activate_picker();
			}
			picker_target_ = spinbox;
		});
	}

	set_center_panel(&box_);
	initialization_complete();
}

void EditorToolNoiseHeightOptionsMenu::update_lower() {
	int32_t upper = upper_.get_value();
	int32_t lower = lower_.get_value();
	// Make sure that upper increases if necessary
	upper = std::max(lower, upper);
	lower = std::min(lower, upper);
	update_interval(lower, upper);
}

void EditorToolNoiseHeightOptionsMenu::update_upper() {
	int32_t upper = upper_.get_value();
	int32_t lower = lower_.get_value();
	// Make sure that lower decreases if necessary
	lower = std::min(lower, upper);
	upper = std::max(lower, upper);
	update_interval(lower, upper);
}

void EditorToolNoiseHeightOptionsMenu::update_interval(int32_t lower, int32_t upper) {
	assert(lower >= 0);
	assert(lower <= MAX_FIELD_HEIGHT);
	assert(upper >= 0);
	assert(upper <= MAX_FIELD_HEIGHT);

	Widelands::HeightInterval height_interval(lower, upper);
	assert(height_interval.valid());

	lower_.set_value(height_interval.min, false);
	upper_.set_value(height_interval.max, false);
	noise_tool_.set_interval(height_interval);

	if (!picker_is_active()) {
		select_correct_tool();
	}
}

void EditorToolNoiseHeightOptionsMenu::update_set_to() {
	int32_t set_to = set_to_.get_value();
	assert(set_to >= 0);
	assert(set_to <= MAX_FIELD_HEIGHT);
	noise_tool_.set_tool().set_interval(Widelands::HeightInterval(set_to, set_to));
	if (!picker_is_active()) {
		select_correct_tool();
	}
}

void EditorToolNoiseHeightOptionsMenu::update_window() {
	lower_.set_value(noise_tool_.get_interval().min, false);
	upper_.set_value(noise_tool_.get_interval().max, false);
}

void EditorToolNoiseHeightOptionsMenu::toggle_picker() {
	EditorToolOptionsMenu::toggle_picker();
	picker_target_ = &set_to_;
}

bool EditorToolNoiseHeightOptionsMenu::pick_from_field(const Widelands::Map& map,
                                                       const Widelands::NodeAndTriangle<>& center,
                                                       const bool multiselect) {
	const Widelands::Field& field = map[center.triangle.node];

	UI::Panel::play_click();

	picker_target_->set_value(field.get_height());

	if (!multiselect) {
		select_correct_tool();
	}
	update_window();

	return !multiselect;
}
