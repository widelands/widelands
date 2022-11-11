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

#include "editor/ui_menus/tool_change_resources_options_menu.h"

#include "base/i18n.h"
#include "base/string.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/tools/increase_resources_tool.h"
#include "editor/tools/set_resources_tool.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/resource_description.h"

constexpr int kMaxValue = 63;

inline EditorInteractive& EditorToolChangeResourcesOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorToolChangeResourcesOptionsMenu::EditorToolChangeResourcesOptionsMenu(
   EditorInteractive& parent,
   EditorIncreaseResourcesTool& increase_tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 370, 120, _("Resources"), increase_tool),
     increase_tool_(increase_tool),
     box_(this, UI::PanelStyle::kWui, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     change_by_(&box_,
                0,
                0,
                get_inner_w() - 2 * hmargin(),
                80,
                increase_tool_.get_change_by(),
                1,
                kMaxValue,
                UI::PanelStyle::kWui,
                _("Increase/Decrease amount by:"),
                UI::SpinBox::Units::kNone,
                UI::SpinBox::Type::kSmall),
     set_to_(&box_,
             0,
             0,
             get_inner_w() - 2 * hmargin(),
             80,
             increase_tool_.set_tool().get_set_to(),
             0,
             kMaxValue,
             UI::PanelStyle::kWui,
             _("Set amount to:"),
             UI::SpinBox::Units::kNone,
             UI::SpinBox::Type::kSmall),
     resources_box_(&box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, 0, 0, 1),
     cur_selection_(
        &box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0, 0, 0, "", UI::Align::kCenter) {
	// Configure spin boxes
	change_by_.set_tooltip(
	   /** TRANSLATORS: Editor change rseources access keys. **/
	   _("Click on the map to increase, "
	     "Shift + Click on the map to decrease the amount of the selected resource"));
	set_to_.set_tooltip(
	   /** TRANSLATORS: Editor set rseources access key. **/
	   _("Ctrl + Click on the map to set the amount of the selected resource. This will replace "
	     "already set resources."));

	change_by_.changed.connect([this]() { update_change_by(); });
	set_to_.changed.connect([this]() { update_set_to(); });

	box_.add(&change_by_);
	box_.add(&set_to_);
	box_.set_size(get_inner_w() - 2 * hmargin(), change_by_.get_h() + set_to_.get_h() + vspacing());

	// Add resource buttons
	resources_box_.add_inf_space();
	const Widelands::Descriptions& descriptions = parent.egbase().descriptions();
	for (Widelands::DescriptionIndex i = 0; i < descriptions.nr_resources(); ++i) {
		const Widelands::ResourceDescription& resource = *descriptions.get_resource_descr(i);
		radiogroup_.add_button(&resources_box_, UI::PanelStyle::kWui, Vector2i::zero(),
		                       g_image_cache->get(resource.representative_image()),
		                       resource.descname());
		resources_box_.add(radiogroup_.get_first_button(), UI::Box::Resizing::kFillSpace);
	}
	resources_box_.add_inf_space();

	box_.add_space(vspacing());
	box_.add(&resources_box_, UI::Box::Resizing::kFullSize);
	box_.set_size(box_.get_w(), box_.get_h() + 4 * vspacing() + resources_box_.get_h());

	radiogroup_.set_state(increase_tool_.get_cur_res(), false);

	radiogroup_.changed.connect([this]() { change_resource(); });
	radiogroup_.clicked.connect([this]() { change_resource(); });

	// Add label
	cur_selection_.set_fixed_width(box_.get_inner_w());
	box_.add(&cur_selection_);

	box_.set_size(box_.get_w(), box_.get_h() + vspacing() + cur_selection_.get_h());
	set_inner_size(get_inner_w(), box_.get_h() + 1 * vmargin());
	update();

	initialization_complete();
}

void EditorToolChangeResourcesOptionsMenu::update_change_by() {
	int32_t change_by = change_by_.get_value();
	assert(change_by > 0);
	assert(change_by <= kMaxValue);
	increase_tool_.set_change_by(change_by);
	increase_tool_.decrease_tool().set_change_by(change_by);
	select_correct_tool();
}

void EditorToolChangeResourcesOptionsMenu::update_set_to() {
	int32_t set_to = set_to_.get_value();
	assert(set_to >= 0);
	assert(set_to <= kMaxValue);
	increase_tool_.set_tool().set_set_to(set_to);
	select_correct_tool();
}

/**
 * called when a resource has been selected
 */
void EditorToolChangeResourcesOptionsMenu::change_resource() {
	const int32_t resource_index = radiogroup_.get_state();

	increase_tool_.set_tool().set_cur_res(resource_index);
	increase_tool_.set_cur_res(resource_index);
	increase_tool_.decrease_tool().set_cur_res(resource_index);

	select_correct_tool();
	update();
}

/**
 * Update all the textareas, so that they represent the correct values
 */
void EditorToolChangeResourcesOptionsMenu::update() {
	cur_selection_.set_text(
	   format(_("Current: %s"), eia()
	                               .egbase()
	                               .descriptions()
	                               .get_resource_descr(increase_tool_.set_tool().get_cur_res())
	                               ->descname()));
}

void EditorToolChangeResourcesOptionsMenu::update_window() {
	radiogroup_.set_state(increase_tool_.get_cur_res(), false);
	change_by_.set_value(increase_tool_.get_change_by());
	set_to_.set_value(static_cast<int>(increase_tool_.set_tool().get_set_to()));
}
