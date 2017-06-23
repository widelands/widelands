/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "editor/ui_menus/tool_change_resources_options_menu.h"

#include <cstdio>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "editor/tools/increase_resources_tool.h"
#include "editor/tools/set_resources_tool.h"
#include "graphic/graphic.h"
#include "logic/map.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"
#include "wui/field_overlay_manager.h"

constexpr int kMaxValue = 63;

inline EditorInteractive& EditorToolChangeResourcesOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorToolChangeResourcesOptionsMenu::EditorToolChangeResourcesOptionsMenu(
   EditorInteractive& parent,
   EditorIncreaseResourcesTool& increase_tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 350, 120, _("Resources")),
     increase_tool_(increase_tool),
     box_(this, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     change_by_(&box_,
                0,
                0,
                get_inner_w() - 2 * hmargin(),
                80,
                increase_tool_.get_change_by(),
                1,
                kMaxValue,
                _("Increase/Decrease Value:"),
                UI::SpinBox::Units::kNone,
                g_gr->images().get("images/ui_basic/but1.png"),
                UI::SpinBox::Type::kSmall),
     set_to_(&box_,
             0,
             0,
             get_inner_w() - 2 * hmargin(),
             80,
             increase_tool_.set_tool().get_set_to(),
             0,
             kMaxValue,
             _("Set Value:"),
             UI::SpinBox::Units::kNone,
             g_gr->images().get("images/ui_basic/but1.png"),
             UI::SpinBox::Type::kSmall),
     resources_box_(&box_, 0, 0, UI::Box::Horizontal, 0, 0, 1),
     cur_selection_(&box_, 0, 0, "", UI::Align::kCenter) {
	// Configure spin boxes
	change_by_.set_tooltip(
	   /** TRANSLATORS: Editor change rseources access keys. **/
	   _("Click on the map to increase, "
	     "Shift + Click on the map to decrease the amount of the selected resource"));
	set_to_.set_tooltip(
	   /** TRANSLATORS: Editor set rseources access key. **/
	   _("Ctrl + Click on the map to set the amount of the selected resource"));

	change_by_.changed.connect(
	   boost::bind(&EditorToolChangeResourcesOptionsMenu::update_change_by, boost::ref(*this)));
	set_to_.changed.connect(
	   boost::bind(&EditorToolChangeResourcesOptionsMenu::update_set_to, boost::ref(*this)));

	box_.add(&change_by_);
	box_.add(&set_to_);
	box_.set_size(get_inner_w() - 2 * hmargin(), change_by_.get_h() + set_to_.get_h() + vspacing());

	// Add resource buttons
	const Widelands::World& world = parent.egbase().world();
	const Widelands::DescriptionIndex nr_resources = world.get_nr_resources();

	for (Widelands::DescriptionIndex i = 0; i < nr_resources; ++i) {
		const Widelands::ResourceDescription& resource = *world.get_resource(i);
		radiogroup_.add_button(&resources_box_, Vector2i::zero(),
		                       g_gr->images().get(resource.representative_image()),
		                       resource.descname());
		resources_box_.add(radiogroup_.get_first_button(), UI::Box::Resizing::kFillSpace);
	}

	box_.add_space(vspacing());
	box_.add(&resources_box_, UI::Box::Resizing::kFullSize);
	box_.set_size(box_.get_w(), box_.get_h() + 4 * vspacing() + resources_box_.get_h());

	radiogroup_.set_state(increase_tool_.get_cur_res());

	radiogroup_.changed.connect(
	   boost::bind(&EditorToolChangeResourcesOptionsMenu::change_resource, this));
	radiogroup_.clicked.connect(
	   boost::bind(&EditorToolChangeResourcesOptionsMenu::change_resource, this));

	// Add label
	cur_selection_.set_fixed_width(box_.get_inner_w());
	box_.add(&cur_selection_);

	box_.set_size(box_.get_w(), box_.get_h() + vspacing() + cur_selection_.get_h());
	set_inner_size(get_inner_w(), box_.get_h() + 1 * vmargin());
	update();
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

	Widelands::EditorGameBase& egbase = eia().egbase();
	Widelands::Map& map = egbase.map();
	eia().mutable_field_overlay_manager()->register_overlay_callback_function(
	   [resource_index, &map,
	    &egbase](const Widelands::TCoords<Widelands::FCoords>& coords) -> uint32_t {
		   if (map.is_resource_valid(egbase.world(), coords, resource_index)) {
			   return coords.field->nodecaps();
		   }
		   return 0;
		});

	map.recalc_whole_map(egbase.world());
	select_correct_tool();
	update();
}

/**
 * Update all the textareas, so that they represent the correct values
*/
void EditorToolChangeResourcesOptionsMenu::update() {
	cur_selection_.set_text(
	   (boost::format(_("Current: %s")) %
	    eia().egbase().world().get_resource(increase_tool_.set_tool().get_cur_res())->descname())
	      .str());
}
