/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "editor/ui_menus/toolsize_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"

inline EditorInteractive& EditorToolsizeMenu::eia() const {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

constexpr int kMargin = 5;
constexpr int kSpinboxWidth = 300;
constexpr int kSpinboxUnitW = 180;

/**
 * Create all the buttons etc...
 */
EditorToolsizeMenu::EditorToolsizeMenu(EditorInteractive& parent,
                                       UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "toolsize_menu", &registry, 250, 30, _("Tool Size")),
     box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, kMargin),
     spinbox_radius_(&box_,
                     0,
                     0,
                     kSpinboxWidth,
                     kSpinboxUnitW,
                     1,
                     1,
                     MAX_TOOL_AREA + 1,
                     UI::PanelStyle::kWui,
                     _("Size:"),
                     UI::SpinBox::Units::kNone,
                     UI::SpinBox::Type::kSmall),
     spinbox_gap_(&box_,
                  0,
                  0,
                  kSpinboxWidth,
                  kSpinboxUnitW,
                  0,
                  0,
                  100,
                  UI::PanelStyle::kWui,
                  _("Gaps:"),
                  UI::SpinBox::Units::kPercent,
                  UI::SpinBox::Type::kBig) {
	spinbox_radius_.set_tooltip(_("Radius of the active tool"));
	spinbox_gap_.set_tooltip(_("Amount of random gaps in the tool’s area"));

	spinbox_radius_.changed.connect([this]() { changed(); });
	spinbox_gap_.changed.connect([this]() { changed(); });

	box_.set_size(kSpinboxWidth, 0);  // to prevent spinbox asserts during layouting
	box_.add(&spinbox_radius_, UI::Box::Resizing::kFullSize);
	box_.add(&spinbox_gap_, UI::Box::Resizing::kFullSize);
	set_center_panel(&box_);

	update(parent.get_sel_radius(), parent.get_sel_gap_percent());

	if (eia().tools()->current().has_size_one()) {
		set_buttons_enabled(false);
	}

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	initialization_complete();
}

void EditorToolsizeMenu::changed() {
	if (is_updating_) {
		return;
	}
	radius_ = spinbox_radius_.get_value() - 1;
	gap_percent_ = spinbox_gap_.get_value();
	eia().set_sel_radius(radius_, gap_percent_);
}

void EditorToolsizeMenu::update(uint32_t radius, uint32_t gap) {
	radius_ = radius;
	gap_percent_ = gap;
	set_buttons_enabled(true);
}

void EditorToolsizeMenu::set_buttons_enabled(bool enable) {
	is_updating_ = true;
	int32_t sbval = radius_ + 1;
	if (enable) {
		spinbox_radius_.set_interval(1, MAX_TOOL_AREA + 1);
		spinbox_radius_.set_value(sbval);

		spinbox_gap_.set_interval(0, 100);
		spinbox_gap_.set_value(gap_percent_);

		eia().set_sel_radius(radius_, gap_percent_);
	} else {
		spinbox_radius_.set_interval(sbval, sbval);
		spinbox_gap_.set_interval(gap_percent_, gap_percent_);
	}
	is_updating_ = false;
}
