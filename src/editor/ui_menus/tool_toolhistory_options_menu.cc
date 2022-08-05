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

#include "editor/ui_menus/tool_toolhistory_options_menu.h"

#include <memory>
#include <vector>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/history.h"
#include "editor/tools/tool.h"
#include "editor/tools/toolhistory_tool.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/terrain_description.h"

inline EditorInteractive& EditorToolhistoryOptionsMenu::eia() const {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
 */
EditorToolhistoryOptionsMenu::EditorToolhistoryOptionsMenu(EditorInteractive& parent,
                                                           EditorHistoryTool& history_tool,
                                                           UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 370, 100, _("Tool History"), history_tool),
     history_tool_(history_tool),
     margin_(4),
     box_width_(get_inner_w() - 2 * margin_),
     box_(this, UI::PanelStyle::kWui, hmargin(), vmargin(), UI::Box::Vertical, 0, 0, vspacing()),
     list_(&box_, 0, 0, box_width_, 330, UI::PanelStyle::kWui) {

	box_.set_size(100, 20);
	box_.add(&list_);
	set_center_panel(&box_);

	list_.clicked.connect([this] { list_item_clicked(list_.get_selected()); });

	update();

	initialization_complete();
}

void EditorToolhistoryOptionsMenu::list_item_clicked(const std::string& selected) {

	if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
		history_tool_.remove_configuration(selected);
		update();
	} else if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		history_tool_.toggle_sticky(selected);
		update();
	} else {
		const ToolConf* conf = history_tool_.get_configuration_for(selected);
		assert(conf != nullptr);
		eia().restore_tool_configuration(*conf);
	}
}

std::string EditorToolhistoryOptionsMenu::make_tooltip(const ToolConf& conf,
                                                       const std::string& title) {
	LuaInterface* lua = &eia().egbase().lua();
	std::unique_ptr<LuaTable> table(lua->run_script("scripting/editor/toolhistory_tooltip.lua"));
	std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
	const Widelands::Descriptions& descriptions = eia().egbase().descriptions();

	if (conf.primary->get_window_id() == WindowID::Terrain) {
		cr->push_arg("terrain");
		for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
			const Widelands::TerrainDescription* descr = descriptions.get_terrain_descr(idx);
			cr->push_arg(descr->name());
		}
	} else if (conf.primary->get_window_id() == WindowID::Critters) {
		cr->push_arg("critter");
		for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
			const Widelands::CritterDescr* descr = descriptions.get_critter_descr(idx);
			cr->push_arg(descr->name());
		}
	} else if (conf.primary->get_window_id() == WindowID::Immovables) {
		cr->push_arg("immovable");
		for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
			const Widelands::ImmovableDescr* descr = descriptions.get_immovable_descr(idx);
			cr->push_arg(descr->name());
		}
	}

	cr->resume();

	std::string help =
	   /** TRANSLATORS: Help tooltip in tool history window */
	   _("Click to restore, Shift + Click to delete, and Ctrl + Click to pin an item.");

	return help + "<vspace gap=1>" + title + cr->pop_table()->get_string("text");
}

void EditorToolhistoryOptionsMenu::update() {
	list_.clear();

	int count = 0;
	for (const auto& it : history_tool_) {
		const ToolConf* conf = history_tool_.get_configuration_for(it.key);
		std::string tooltip = make_tooltip(*conf, it.title);
		if (it.sticky) {
			list_.add(it.title, it.key, g_image_cache->get("images/wui/editor/sticky_list_item.png"),
			          false, tooltip);
		} else {
			list_.add(it.title, it.key, nullptr, false, tooltip);
		}

		count++;
	}
}
