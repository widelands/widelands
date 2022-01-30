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

#include "editor/ui_menus/toolsize_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"

inline EditorInteractive& EditorToolsizeMenu::eia() const {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
 */
EditorToolsizeMenu::EditorToolsizeMenu(EditorInteractive& parent,
                                       UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "toolsize_menu", &registry, 250, 30, _("Tool Size")),
     spinbox_(this,
              0,
              0,
              get_inner_w(),
              80,
              1,
              1,
              MAX_TOOL_AREA + 1,
              UI::PanelStyle::kWui,
              _("Current Size:"),
              UI::SpinBox::Units::kNone,
              UI::SpinBox::Type::kSmall),
     value_(0) {
	spinbox_.changed.connect([this]() { changed(); });

	set_inner_size(spinbox_.get_w() + 2 * kMargin, spinbox_.get_h() + 2 * kMargin);
	spinbox_.set_pos(Vector2i(kMargin, kMargin));

	update(parent.get_sel_radius());

	if (eia().tools()->current().has_size_one()) {
		set_buttons_enabled(false);
	}

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	initialization_complete();
}

void EditorToolsizeMenu::changed() {
	value_ = spinbox_.get_value() - 1;
	eia().set_sel_radius(value_);
}

void EditorToolsizeMenu::update(uint32_t const val) {
	value_ = val;
	set_buttons_enabled(true);
}

void EditorToolsizeMenu::set_buttons_enabled(bool enable) {
	int32_t sbval = value_ + 1;
	if (enable) {
		spinbox_.set_interval(1, MAX_TOOL_AREA + 1);
		spinbox_.set_value(sbval);
		eia().set_sel_radius(value_);
	} else {
		spinbox_.set_interval(sbval, sbval);
	}
}
