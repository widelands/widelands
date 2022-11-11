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

#include "editor/ui_menus/tool_place_critter_options_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/place_critter_tool.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/critter.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"

namespace {
UI::Checkbox* create_critter_checkbox(UI::Panel* parent,
                                      const Widelands::CritterDescr& critter_descr) {
	const Image* pic = critter_descr.representative_image();
	UI::Checkbox* cb = new UI::Checkbox(
	   parent, UI::PanelStyle::kWui, Vector2i::zero(), pic, critter_descr.descname());
	const int kMinClickableArea = 24;
	cb->set_desired_size(std::max<int>(pic->width(), kMinClickableArea),
	                     std::max<int>(pic->height(), kMinClickableArea));
	return cb;
}

}  // namespace

EditorToolPlaceCritterOptionsMenu::EditorToolPlaceCritterOptionsMenu(
   EditorInteractive& parent, EditorPlaceCritterTool& tool, UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Animals"), tool) {
	const Widelands::Descriptions& descriptions = parent.egbase().descriptions();
	multi_select_menu_.reset(
	   new CategorizedItemSelectionMenu<Widelands::CritterDescr, EditorPlaceCritterTool>(
	      this, parent.editor_categories(Widelands::MapObjectType::CRITTER), descriptions.critters(),
	      [](UI::Panel* cb_parent, const Widelands::CritterDescr& critter_descr) {
		      return create_critter_checkbox(cb_parent, critter_descr);
	      },
	      [this] { select_correct_tool(); }, &tool));
	set_center_panel(multi_select_menu_.get());

	initialization_complete();
}

void EditorToolPlaceCritterOptionsMenu::update_window() {
	multi_select_menu_->update_selection();
}
