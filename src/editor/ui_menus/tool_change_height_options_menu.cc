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

#include "editor/ui_menus/tool_change_height_options_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/increase_height_tool.h"
#include "editor/tools/set_height_tool.h"
#include "logic/widelands_geometry.h"

EditorToolChangeHeightOptionsMenu::EditorToolChangeHeightOptionsMenu(
   EditorInteractive& parent,
   EditorIncreaseHeightTool& increase_tool,
   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 350, 100, _("Height Tools Options"), increase_tool),
     increase_tool_(increase_tool),
     box_(this, UI::PanelStyle::kWui, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     change_by_(&box_,
                0,
                0,
                get_inner_w() - 2 * hmargin(),
                80,
                increase_tool_.get_change_by(),
                1,
                MAX_FIELD_HEIGHT_DIFF,
                UI::PanelStyle::kWui,
                _("Increase/Decrease height by:"),
                UI::SpinBox::Units::kNone,
                UI::SpinBox::Type::kSmall),
     set_to_(&box_,
             0,
             0,
             get_inner_w() - 2 * hmargin(),
             80,
             increase_tool_.set_tool().get_interval().min,
             0,
             MAX_FIELD_HEIGHT,
             UI::PanelStyle::kWui,
             _("Set height to:"),
             UI::SpinBox::Units::kNone,
             UI::SpinBox::Type::kSmall) {
	change_by_.set_tooltip(
	   /** TRANSLATORS: Editor change height access keys. **/
	   _("Click on the map to increase, Shift + Click on the map to decrease terrain height"));
	set_to_.set_tooltip(
	   /** TRANSLATORS: Editor set height access key. **/
	   _("Ctrl + Click on the map to set terrain height"));

	change_by_.changed.connect([this]() { update_change_by(); });
	set_to_.changed.connect([this]() { update_set_to(); });

	box_.add(&change_by_);
	box_.add(&set_to_);
	box_.set_size(get_inner_w() - 2 * hmargin(), change_by_.get_h() + set_to_.get_h() + vspacing());
	set_inner_size(box_.get_w() + 2 * hmargin(), box_.get_h() + 2 * vspacing());

	initialization_complete();
}

void EditorToolChangeHeightOptionsMenu::update_change_by() {
	int32_t change_by = change_by_.get_value();
	assert(change_by > 0);
	assert(change_by <= MAX_FIELD_HEIGHT_DIFF);

	increase_tool_.set_change_by(change_by);
	increase_tool_.decrease_tool().set_change_by(change_by);
	select_correct_tool();
}

void EditorToolChangeHeightOptionsMenu::update_set_to() {
	int32_t set_to = set_to_.get_value();
	assert(set_to >= 0);
	assert(set_to <= MAX_FIELD_HEIGHT);
	increase_tool_.set_tool().set_interval(Widelands::HeightInterval(set_to, set_to));
	select_correct_tool();
}
