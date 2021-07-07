/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
     toolsize_spinbox_(this,
                       5,
                       5,
                       get_inner_w() - 2 * 5,
                       80,
                       1,
                       1,
                       MAX_TOOL_AREA + 1,
                       UI::PanelStyle::kWui,
                       _("Current size:"),
                       UI::SpinBox::Units::kNone,
                       UI::SpinBox::Type::kSmall),
     value_(0) {
	toolsize_spinbox_.changed.connect([this]() { changed_(); });

	update(parent.get_sel_radius());

	if (eia().tools()->current().has_size_one()) {
		set_buttons_enabled(false);
	}

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	initialization_complete();
}

void EditorToolsizeMenu::changed_() {
	value_ = toolsize_spinbox_.get_value() - 1;
	eia().set_sel_radius(value_);
}

void EditorToolsizeMenu::update(uint32_t const val) {
	value_ = val;
	set_buttons_enabled(true);
}

void EditorToolsizeMenu::set_buttons_enabled(bool enable) {
	if (enable) {
		toolsize_spinbox_.set_interval(1, MAX_TOOL_AREA + 1);
		toolsize_spinbox_.set_value(value_ + 1);
		eia().set_sel_radius(value_);
	} else {
		toolsize_spinbox_.set_interval(1, 1);
		toolsize_spinbox_.set_value(value_ + 1);
	}
}
