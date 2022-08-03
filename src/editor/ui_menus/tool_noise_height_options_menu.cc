/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
#include "logic/widelands_geometry.h"
#include "ui_basic/textarea.h"

EditorToolNoiseHeightOptionsMenu::EditorToolNoiseHeightOptionsMenu(
   EditorInteractive& parent,
   EditorNoiseHeightTool& noise_tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 300, 120, _("Random Height Options"), noise_tool),
     noise_tool_(noise_tool),
     box_(this, UI::PanelStyle::kWui, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     lower_(&box_,
            0,
            0,
            get_inner_w() - 2 * hmargin(),
            80,
            noise_tool_.get_interval().min,
            1,
            MAX_FIELD_HEIGHT,
            UI::PanelStyle::kWui,
            _("Minimum height:"),
            UI::SpinBox::Units::kNone,
            UI::SpinBox::Type::kSmall),
     upper_(&box_,
            0,
            0,
            get_inner_w() - 2 * hmargin(),
            80,
            noise_tool_.get_interval().max,
            0,
            MAX_FIELD_HEIGHT,
            UI::PanelStyle::kWui,
            _("Maximum height:"),
            UI::SpinBox::Units::kNone,
            UI::SpinBox::Type::kSmall),
     set_to_(&box_,
             0,
             0,
             get_inner_w() - 2 * hmargin(),
             80,
             noise_tool_.set_tool().get_interval().min,
             0,
             MAX_FIELD_HEIGHT,
             UI::PanelStyle::kWui,
             _("Set height to:"),
             UI::SpinBox::Units::kNone,
             UI::SpinBox::Type::kSmall) {
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

	UI::Textarea* label = new UI::Textarea(&box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0,
	                                       0, 0, 0, _("Random height"), UI::Align::kCenter);
	label->set_fixed_width(get_inner_w() - 2 * hmargin());
	box_.add(label);
	box_.add(&upper_);
	box_.add(&lower_);

	box_.add_space(2 * vspacing());
	label = new UI::Textarea(&box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0, 0, 0,
	                         _("Fixed height"), UI::Align::kCenter);
	label->set_fixed_width(get_inner_w() - 2 * hmargin());
	box_.add(label);
	box_.add(&set_to_);

	box_.set_size(get_inner_w() - 2 * hmargin(), upper_.get_h() + lower_.get_h() + set_to_.get_h() +
	                                                2 * label->get_h() + 7 * vspacing());

	set_inner_size(box_.get_w() + 2 * hmargin(), box_.get_h() + 2 * vspacing());

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

	lower_.set_value(height_interval.min);
	upper_.set_value(height_interval.max);
	noise_tool_.set_interval(height_interval);
	select_correct_tool();
}

void EditorToolNoiseHeightOptionsMenu::update_set_to() {
	int32_t set_to = set_to_.get_value();
	assert(set_to >= 0);
	assert(set_to <= MAX_FIELD_HEIGHT);
	noise_tool_.set_tool().set_interval(Widelands::HeightInterval(set_to, set_to));
	select_correct_tool();
}

void EditorToolNoiseHeightOptionsMenu::update_window() {
	lower_.set_value(noise_tool_.get_interval().min);
	upper_.set_value(noise_tool_.get_interval().max);
}
