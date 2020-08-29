/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "editor/ui_menus/scenario_tool_road_options_menu.h"

#include "editor/editorinteractive.h"

inline EditorInteractive& ScenarioToolRoadOptionsMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

constexpr uint16_t kButtonSize = 34;

ScenarioToolRoadOptionsMenu::ScenarioToolRoadOptionsMenu(EditorInteractive& parent,
                                                         ScenarioPlaceRoadTool& tool,
                                                         UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 250, 200, _("Roads and Waterways"), tool),
     tool_(tool),
     main_box_(this, 0, 0, UI::Box::Vertical),
     buttons_(&main_box_,
              0,
              0,
              (parent.egbase().map().get_waterway_max_length() < 2 ? 2 : 3) * kButtonSize,
              kButtonSize),
     force_(&main_box_,
            Vector2i(0, 0),
            _("Force road"),
            _("Allow placing roads where they can normally not be built, conquer the path for the "
              "building player, and destroy flags that are too close to the end flag")),
     create_primary_(&main_box_,
                     Vector2i(0, 0),
                     _("Create worker"),
                     _("Create a carrier/ferry for the road/waterway")),
     create_secondary_(&main_box_,
                       Vector2i(0, 0),
                       _("Create second carrier"),
                       _("Create a second carrier for busy roads")),
     place_flags_(&main_box_, Vector2i(0, 0), _("Place flags"), _("Place flags along the road")),
     info_(&main_box_,
           0,
           0,
           0,
           0,
           UI::PanelStyle::kWui,
           "",
           UI::Align::kCenter,
           UI::MultilineTextarea::ScrollMode::kNoScrolling) {
	force_.set_state(tool_.get_force());
	place_flags_.set_state(tool_.get_place_flags());
	create_primary_.set_state(tool_.get_create_primary_worker());
	create_secondary_.set_state(tool_.get_create_secondary_worker());
	type_.set_state(static_cast<int32_t>(tool_.get_mode()));
	create_secondary_.set_enabled(tool_.get_mode() == EditorActionArgs::RoadMode::kBusy);

	type_.changedto.connect([this](int32_t i) {
		tool_.set_mode(static_cast<EditorActionArgs::RoadMode>(i));
		create_secondary_.set_enabled(tool_.get_mode() == EditorActionArgs::RoadMode::kBusy);
		select_correct_tool();
	});
	force_.changedto.connect([this](bool f) {
		tool_.set_force(f);
		select_correct_tool();
	});
	place_flags_.changedto.connect([this](bool f) {
		tool_.set_place_flags(f);
		select_correct_tool();
	});
	create_primary_.changedto.connect([this](bool c) {
		tool_.set_create_primary_worker(c);
		select_correct_tool();
	});
	create_secondary_.changedto.connect([this](bool c) {
		tool_.set_create_secondary_worker(c);
		select_correct_tool();
	});

	type_.add_button(&buttons_, Vector2i(0, 0),
	                 g_image_cache->get("images/wui/fieldaction/menu_build_way.png"),
	                 _("Normal road"));
	type_.add_button(&buttons_, Vector2i(kButtonSize, 0),
	                 g_image_cache->get("images/wui/fieldaction/menu_tab_buildroad.png"),
	                 _("Busy road"));
	if (parent.egbase().map().get_waterway_max_length() >= 2) {
		type_.add_button(&buttons_, Vector2i(2 * kButtonSize, 0),
		                 g_image_cache->get("images/wui/fieldaction/menu_tab_buildwaterway.png"),
		                 _("Waterway"));
	}
	main_box_.add(&buttons_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	main_box_.add(&create_primary_, UI::Box::Resizing::kFullSize);
	main_box_.add(&create_secondary_, UI::Box::Resizing::kFullSize);
	main_box_.add(&place_flags_, UI::Box::Resizing::kFullSize);
	main_box_.add(&force_, UI::Box::Resizing::kFullSize);
	main_box_.add(&info_, UI::Box::Resizing::kFullSize);
	set_center_panel(&main_box_);
	think();

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void ScenarioToolRoadOptionsMenu::think() {
	EditorToolOptionsMenu::think();
	const EditorInteractive& e = eia();
	info_.set_text(e.in_road_building_mode(RoadBuildingType::kRoad) ?
	                  e.get_build_road_start() == e.get_build_road_end() ?
	                  _("Click on fields to determine the waterway’s path, or click the start flag "
	                    "again to cancel.") :
	                  _("Click on fields to determine the waterway’s path. Click on the end field "
	                    "again to build a flag there. Double-click the start flag to cancel.") :
	                  e.in_road_building_mode(RoadBuildingType::kWaterway) ?
	                  e.get_build_road_start() == e.get_build_road_end() ?
	                  _("Click on fields to determine the road’s path, or click the start flag "
	                    "again to cancel.") :
	                  _("Click on fields to determine the road’s path. Click on the end field again "
	                    "to build a flag there. Double-click the start flag to cancel.") :
	                  static_cast<EditorActionArgs::RoadMode>(type_.get_state()) ==
	                           EditorActionArgs::RoadMode::kWaterway ?
	                  _("Click on a flag to start building a waterway.") :
	                  _("Click on a flag to start building a road."));
}
