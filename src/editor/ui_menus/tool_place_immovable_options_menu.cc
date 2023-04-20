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

#include "editor/ui_menus/tool_place_immovable_options_menu.h"

#include <memory>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/place_immovable_tool.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"

namespace {

UI::Checkbox* create_immovable_checkbox(UI::Panel* parent,
                                        LuaInterface* lua,
                                        const Widelands::ImmovableDescr& immovable_descr) {
	const Image* pic = immovable_descr.representative_image();

	std::string tooltip;

	// Get information about preferred terrains
	if (immovable_descr.has_terrain_affinity()) {
		std::unique_ptr<LuaTable> table(lua->run_script("scripting/editor/tree_tooltip.lua"));
		std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
		cr->push_arg(immovable_descr.name());
		cr->resume();
		tooltip =
		   format(_("%1$s %2$s"), immovable_descr.descname(), cr->pop_table()->get_string("text"));
	} else {
		switch (immovable_descr.get_size()) {
		case Widelands::BaseImmovable::NONE:
			tooltip = format(_("%1$s (removable by e.g. placing roads)"), immovable_descr.descname());
			break;
		case Widelands::BaseImmovable::SMALL:
			tooltip = format(_("%1$s (needs a small building plot)"), immovable_descr.descname());
			break;
		case Widelands::BaseImmovable::MEDIUM:
			tooltip = format(_("%1$s (needs a medium building plot)"), immovable_descr.descname());
			break;
		case Widelands::BaseImmovable::BIG:
			tooltip = format(_("%1$s (needs a big building plot)"), immovable_descr.descname());
			break;
		}
	}

	UI::Checkbox* cb =
	   new UI::Checkbox(parent, UI::PanelStyle::kWui, Vector2i::zero(), pic, tooltip);
	const int kMinClickableArea = 24;
	cb->set_desired_size(std::max<int>(pic->width(), kMinClickableArea),
	                     std::max<int>(pic->height(), kMinClickableArea));
	return cb;
}

}  // namespace

EditorToolPlaceImmovableOptionsMenu::EditorToolPlaceImmovableOptionsMenu(
   EditorInteractive& parent, EditorPlaceImmovableTool& tool, UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Immovables"), tool) {
	const Widelands::Descriptions& descriptions = parent.egbase().descriptions();
	LuaInterface* lua = &parent.egbase().lua();
	multi_select_menu_.reset(
	   new CategorizedItemSelectionMenu<Widelands::ImmovableDescr, EditorPlaceImmovableTool>(
	      this, parent.editor_categories(Widelands::MapObjectType::IMMOVABLE),
	      descriptions.immovables(),
	      [lua](UI::Panel* cb_parent, const Widelands::ImmovableDescr& immovable_descr) {
		      return create_immovable_checkbox(cb_parent, lua, immovable_descr);
	      },
	      [this, &tool] {
		      auto_trees_button_->set_perm_pressed(false);
		      tool.enable(kAutoTreesIndex, false);
		      select_correct_tool();
	      },
	      &tool, {{kAutoTreesIndex, _("Automatic Trees")}}));

	UI::Box* auto_immovables_box =
	   new UI::Box(&multi_select_menu_->tabs(), UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	auto_trees_button_ = new UI::Button(auto_immovables_box, "auto_trees", 0, 0, 0, 0,
	                                    UI::ButtonStyle::kWuiSecondary, _("Automatic Trees"),
	                                    _("Automatically place the trees which "
	                                      "grow best on the terrain"));
	auto_trees_button_->sigclicked.connect([this, &tool]() {
		auto_trees_button_->toggle();
		if (auto_trees_button_->style() == UI::Button::VisualState::kPermpressed) {
			tool.disable_all();
			select_correct_tool();
			tool.enable(kAutoTreesIndex, true);
		} else {
			tool.enable(kAutoTreesIndex, false);
		}
		multi_select_menu_->update_label();
	});
	auto_immovables_box->add(auto_trees_button_, UI::Box::Resizing::kFullSize);
	multi_select_menu_->tabs().add("auto",
	                               g_image_cache->get("images/wui/editor/tools/immovables.png"),
	                               auto_immovables_box, _("Automatic Immovable Placement"), 0);
	multi_select_menu_->tabs().activate(0);

	set_center_panel(multi_select_menu_.get());

	initialization_complete();
}

void EditorToolPlaceImmovableOptionsMenu::update_window() {
	multi_select_menu_->update_selection();
}

void EditorToolPlaceImmovableOptionsMenu::think() {
	EditorToolOptionsMenu::think();
	auto_trees_button_->set_perm_pressed(
	   parent_.tools()->current_pointer == &current_tool_ &&
	   dynamic_cast<EditorPlaceImmovableTool&>(current_tool_).is_enabled(kAutoTreesIndex));
}
